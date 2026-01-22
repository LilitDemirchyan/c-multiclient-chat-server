//client
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define PORT 3368
#define MAX_BUF_SIZE 1024
int socket_fd;

void* read_thread(void* argv){
    while(1){
    char buffer[MAX_BUF_SIZE];
    memset(buffer, 0, MAX_BUF_SIZE);
    read(socket_fd, buffer, MAX_BUF_SIZE);
    printf("%s\n", buffer);
    }
    return NULL;
}

void* write_thread(void* argv){
    while(1){
        char buffer[MAX_BUF_SIZE];
        memset(buffer, 0, MAX_BUF_SIZE);
        fgets(buffer, MAX_BUF_SIZE, stdin);
        write(socket_fd, buffer, strlen(buffer));
    }
    return NULL;
}

int main(){
    pthread_t th_read, th_write;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        perror("socket failed");
        exit(1);
    }
    struct sockaddr_in client;
    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(PORT);
    client.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    int con = connect(socket_fd, (struct sockaddr*)&client, sizeof(client));
    if(con < 0){
        perror("connection failed");
        exit(1);
    }
    printf("Enter your name : ");
    char name[20];
    fgets(name, 20, stdin);
    write(socket_fd, name, strlen(name));
    pthread_create(&th_read, NULL, read_thread, NULL);
    
    pthread_create(&th_write, NULL, write_thread, NULL);
    pthread_join(th_read, NULL);
    pthread_join(th_write, NULL);
    return 0;
}
