#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define BUFFER_SIZE 256

int server_fd;
volatile int server_disconnected = 0;

void* send_msg(void* arg)
{
    char* username = (char*)arg;

    char buffer[BUFFER_SIZE] = {0};
    while(1) {
        fgets(buffer, sizeof(buffer), stdin);

        size_t bufferlen = strlen(buffer);
        if(bufferlen > 0 && buffer[bufferlen - 1] == '\n') {
            buffer[bufferlen - 1] = '\0';
        }

        if(strcmp(buffer, "/exit") == 0) {
            printf("Exitting...\n");
            server_disconnected = 1;
            break;
        }

        char message[BUFFER_SIZE + 32];
        snprintf(message, sizeof(message), "%s: %s", username, buffer);

        write(server_fd, message, strlen(message));
    }

    return NULL;
}

int main(int argc, char* argv[])
{
    if(argc != 4) {
        printf("Usage: prog <HOSTIP> <PORT> <USERNAME>\n");
        exit(EXIT_SUCCESS);
    }

    char* host = argv[1];
    int port = atoi(argv[2]);
    char* username = argv[3];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server_fd == -1) {
        perror("Failed to create socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, host, &server_addr.sin_addr) == 0) {
        perror("Invalid host ip");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if(connect(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to connect");
        close(server_fd);
        exit(EXIT_SUCCESS);
    }

    printf("Connected to server at %s:%d\n", host, port);

    pthread_t msg_thread;
    if(pthread_create(&msg_thread, NULL, send_msg, (void*)username) != 0) {
        perror("Failed to create send message thread");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE] = {0};
    while(1) {
        if(server_disconnected)
            break;
        
        int bytes_received = read(server_fd, buffer, sizeof(buffer) - 1);
        if(bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("%s\n", buffer);
        }
        
        else if(bytes_received == 0) {
            printf("Server disconnected.\n");
            break;
        }
        
        else {
            perror("Failed to receive msg");
            break;
        }
    }

    pthread_cancel(msg_thread);
    close(server_fd);

    exit(EXIT_SUCCESS);
}