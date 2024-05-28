#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define PORT 8080
#define BUF_SIZE 1024

int client_count = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *client_socket);

void signal_handler(int sig)
{
    if (sig == SIGUSR1)
    {
        printf("Server shutting down...\n");
        exit(0);
    }
    else if (sig == SIGUSR2)
    {
        pthread_mutex_lock(&count_mutex);
        printf("Current number of connected clients: %d\n", client_count);
        pthread_mutex_unlock(&count_mutex);
    }
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;
    pthread_t tid;

    // Setting up the signal handlers
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);

    // Creating socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuring server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Binding the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Binding failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listening for connections
    if (listen(server_socket, 10) == -1)
    {
        perror("Listening failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1)
    {
        client_addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_socket == -1)
        {
            perror("Accept failed");
            continue;
        }

        pthread_mutex_lock(&count_mutex);
        client_count++;
        pthread_mutex_unlock(&count_mutex);

        if (pthread_create(&tid, NULL, handle_client, (void *)&client_socket) != 0)
        {
            perror("Thread creation failed");
            close(client_socket);
        }
    }

    close(server_socket);
    return 0;
}

void *handle_client(void *client_socket)
{
    int sock = *(int *)client_socket;
    char buffer[BUF_SIZE];
    int bytes_read;

    while ((bytes_read = read(sock, buffer, BUF_SIZE - 1)) > 0)
    {
        buffer[bytes_read] = '\0';
        printf("Client says: %s\n", buffer);
        write(sock, buffer, strlen(buffer));
    }

    close(sock);

    pthread_mutex_lock(&count_mutex);
    client_count--;
    pthread_mutex_unlock(&count_mutex);

    return NULL;
}
