#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CHATTERS 10
#define PORT 9300
#define BUFFER_SIZE 256

int server_fd;
int client_fd[MAX_CHATTERS] = {0};
fd_set read_fds;
volatile int server_shutdown = 0;

void* manage_server(void* arg)
{
    char buffer[32] = {0};
    while(1) {
        fgets(buffer, sizeof(buffer), stdin);

        size_t bufferlen = strlen(buffer);
        if(bufferlen > 0 && buffer[bufferlen - 1] == '\n') {
            buffer[bufferlen - 1] = '\0';
        }

        if(strcmp(buffer, "/shutdown") == 0) {
            printf("Shutting server down...\n");
            for(int i = 0; i < MAX_CHATTERS; i++)
                    close(client_fd[i]);
                
            server_shutdown = 1;
            break;
        }

        int client_id;
        int found = 0;
        if(sscanf(buffer, "/kick %d", &client_id) == 1) {
            for (int i = 0; i < MAX_CHATTERS; i++) {
                if (client_fd[i] == client_id) {
                    close(client_fd[i]);
                    client_fd[i] = 0;
                    printf("Kicking user with socket ID %d\n", client_id);
                    found = 1;
                    break;
                }
            }

            if(!found)
                printf("User %d not found.\n", client_id);
        }
    }

    return NULL;
}

int main()
{
    struct sockaddr_in server_addr;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server_fd == -1) {
        perror("Failed to create socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, MAX_CHATTERS) == -1) {
        perror("Failed to listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d.\n", PORT);
    pthread_t admin_thread;
    pthread_create(&admin_thread, NULL, manage_server, NULL);

    struct sockaddr_in client_addr;
    socklen_t client_addrsize = sizeof(client_addr);
    int client_newfd;
    int sock_count, max_sd;
    char buffer[BUFFER_SIZE] = {0};

    while(1) {
        if(server_shutdown)
            break;
        
        bzero(buffer, sizeof(buffer));
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        max_sd = server_fd;

        for(int i = 0; i < MAX_CHATTERS; i++) {
            if(client_fd[i] > 0)
                FD_SET(client_fd[i], &read_fds);

            if(client_fd[i] > max_sd)
                max_sd = client_fd[i];
        }

        struct timeval timeout;
        timeout.tv_sec = 1; // 1 second
        timeout.tv_usec = 0;
        sock_count = select(max_sd + 1, &read_fds, NULL, NULL, &timeout);

        if(sock_count < 0)
            perror("Failed to select");

        if(FD_ISSET(server_fd, &read_fds)) {
            if((client_newfd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addrsize)) == -1) {
                perror("Failed to accept");
                continue;
            }

            for(int i = 0; i < MAX_CHATTERS; i++) {
                if(client_fd[i] == 0) {
                    client_fd[i] = client_newfd;
                    printf("[S:%d] Chatter connected, %s:%d.\n", client_newfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    break;
                }
            }
        }

        for(int i = 0; i < MAX_CHATTERS; i++) {
            if(client_fd[i] > 0 && FD_ISSET(client_fd[i], &read_fds)) {
                int data_received = recv(client_fd[i], buffer, sizeof(buffer) -1, 0);

                if(data_received > 0) {
                    buffer[data_received] = '\0';
                    printf("[%d]%s\n", client_fd[i], buffer);
                    
                    for(int j = 0; j < MAX_CHATTERS; j++) {
                        if(client_fd[j] > 0 && client_fd[j] != client_fd[i]) {
                            send(client_fd[j], buffer, strlen(buffer), 0);
                        }
                    }
                }

                else if(data_received == 0) {
                    printf("[S:%d] Chatter disconnected.\n", client_fd[i]);
                    close(client_fd[i]);
                    client_fd[i] = 0;
                }

                else {
                    perror("Failed to receive");
                }
            }
        }
    }

    pthread_cancel(admin_thread);
    close(server_fd);
    
    exit(EXIT_SUCCESS);
}