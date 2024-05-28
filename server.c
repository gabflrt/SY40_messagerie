#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT 8080
#define BUF_SIZE 1024

int client_count = 0;
int client_sockets[2] = {0};
pid_t child_pids[2] = {0};
int server_socket;

void signal_handler(int sig);

void handle_client(int client_socket, int other_socket);

int main()
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;
    int client_socket;
    pid_t pid;

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

        client_sockets[client_count] = client_socket;
        client_count++;

        if (client_count == 2)
        {
            pid = fork();
            if (pid == 0)
            { // Child process
                close(server_socket);
                handle_client(client_sockets[0], client_sockets[1]);
                handle_client(client_sockets[1], client_sockets[0]);
                exit(0);
            }
            else if (pid > 0)
            { // Parent process
                child_pids[0] = pid;
                close(client_sockets[0]);
                close(client_sockets[1]);
                client_count = 0;
            }
            else
            {
                perror("Fork failed");
                close(client_sockets[0]);
                close(client_sockets[1]);
                client_count = 0;
            }
        }
    }

    close(server_socket);
    return 0;
}

void handle_client(int client_socket, int other_socket)
{
    char buffer[BUF_SIZE];
    char message[BUF_SIZE];
    int bytes_read;

    while (1)
    {
        // Prompt the client to enter a response
        snprintf(message, sizeof(message), "Donnez votre réponse: ");
        write(client_socket, message, strlen(message));

        // Read the client's response
        bytes_read = read(client_socket, buffer, BUF_SIZE - 1);
        if (bytes_read <= 0)
        {
            break;
        }
        buffer[bytes_read] = '\0';

        // Format and send the response to the other client
        snprintf(message, sizeof(message), "Le client distant dit: %s", buffer);
        write(other_socket, message, strlen(message));
    }

    close(client_socket);
}

void signal_handler(int sig)
{
    if (sig == SIGUSR1)
    {
        printf("Server shutting down...\n");
        for (int i = 0; i < 2; i++)
        {
            if (child_pids[i] > 0)
            {
                kill(child_pids[i], SIGTERM);
            }
        }
        close(server_socket);
        exit(0);
    }
    else if (sig == SIGUSR2)
    {
        printf("Current number of connected clients: %d\n", client_count);
    }
}
