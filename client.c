#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define BUF_SIZE 1024
#define MAX_RETRIES 5
#define RETRY_DELAY 2 // seconds

int main()
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    int bytes_read;
    fd_set read_fds;
    int retries = 0;

    while (retries < MAX_RETRIES)
    {
        // Creating socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1)
        {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }
        printf("Socket created successfully\n");

        // Configuring server address structure
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        server_addr.sin_port = htons(PORT);

        // Connecting to the server
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        {
            perror("Connection failed");
            close(sock);
            retries++;
            printf("Retrying in %d seconds...\n", RETRY_DELAY);
            sleep(RETRY_DELAY);
            continue;
        }
        printf("Connected to the server. Type your messages...\n");
        break;
    }

    if (retries == MAX_RETRIES)
    {
        fprintf(stderr, "Failed to connect to the server after %d retries\n", MAX_RETRIES);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sock, &read_fds);

        // Using select for non-blocking I/O
        int max_sd = sock;
        int activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0)
        {
            perror("select error");
        }

        // Check if it's from the server
        if (FD_ISSET(sock, &read_fds))
        {
            bytes_read = read(sock, buffer, BUF_SIZE - 1);
            if (bytes_read <= 0)
            {
                printf("Server closed the connection.\n");
                break;
            }
            buffer[bytes_read] = '\0';
            printf("%s\n", buffer);
        }

        // Check if it's user input
        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            fgets(buffer, BUF_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
            if (write(sock, buffer, strlen(buffer)) == -1)
            {
                perror("Write to server failed");
                break;
            }
        }
    }

    close(sock);
    return 0;
}
