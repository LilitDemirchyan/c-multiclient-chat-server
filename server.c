//server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 3368
#define MAX_BUF_SIZE 1024

typedef struct init {
    char name[50];
    int fd;
} init;

init users[SOMAXCONN];
int i = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* thread_func(void* argv){
    int j = *(int*)argv;
    char name[20];
    memset(name, 0, sizeof(name));
    read(users[j].fd, name, 20);
    name[strcspn(name, "\n")] = 0;
    strncpy(users[j].name, name, strlen(name));
    strcat(name, " : ");
    char buffer[MAX_BUF_SIZE];
    while (1){
        memset(buffer, 0, MAX_BUF_SIZE);
        int bytes_read = read(users[j].fd, buffer, MAX_BUF_SIZE - 1);
        if (bytes_read <= 0) {
            close(users[j].fd);
            pthread_exit(NULL);
        }
        buffer[bytes_read] = '\0';  
        if (buffer[0] == '@'){
            int k = 1;
            char user_name[20];
            memset(user_name, 0, 20);
            while (buffer[k] != ' ' && buffer[k] != '\0'){
                user_name[k - 1] = buffer[k];
                ++k;
            }
            for (int t = 0; t < i; ++t){
                if (strncmp(user_name, users[t].name, strlen(user_name)) == 0){
                    write(users[t].fd, name, strlen(name));
                    write(users[t].fd, &buffer[k + 1], strlen(&buffer[k + 1]));
                    break;
                }
            }
        } else {
            pthread_mutex_lock(&mutex);
            for (int t = 0; t < i; ++t){
                if (users[t].fd != users[j].fd){
                    write(users[t].fd, name, strlen(name));
                    write(users[t].fd, buffer, strlen(buffer));
                }
            }
            pthread_mutex_unlock(&mutex);
        }
    }
    free(argv);
    return NULL;
}

int main(){
    pthread_t threads[SOMAXCONN];
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket failed");
        exit(1);
    }
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    socklen_t size = sizeof(struct sockaddr_in);
    int bind_s = bind(socket_fd, (struct sockaddr*)&server, sizeof(server));
    if (bind_s < 0){
        perror("bind failed");
        exit(1);
    }
    int lis = listen(socket_fd, SOMAXCONN);
    if (lis < 0){
        perror("listen failed");
        exit(1);
    }
    while (1){
        struct sockaddr_in client;
        int client_fd = accept(socket_fd, (struct sockaddr*)&client, &size);
        if (client_fd < 0){
            perror("accept failed");
            exit(1);
        }
        pthread_mutex_lock(&mutex);
        users[i].fd = client_fd;
        int* j = malloc(sizeof(int));
        *j = i;
        i++;
        pthread_mutex_unlock(&mutex);
        pthread_create(&threads[*j], NULL, thread_func, j);
    }
    for (int k = 0; k < i; ++k){
        pthread_join(threads[k], NULL);
    }
    return 0;
}
