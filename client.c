#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 1024

int main()
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    int bytes_read;

    // Creating socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuring server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // Connecting to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server. Type your messages...\n");

    while (1)
    {
        // Read prompt from server
        bytes_read = read(sock, buffer, BUF_SIZE - 1);
        if (bytes_read <= 0)
        {
            break;
        }
        buffer[bytes_read] = '\0';
        printf("%s", buffer);

        // Get user input and send to server
        fgets(buffer, BUF_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        write(sock, buffer, strlen(buffer));

        // Read response from server
        bytes_read = read(sock, buffer, BUF_SIZE - 1);
        if (bytes_read <= 0)
        {
            break;
        }
        buffer[bytes_read] = '\0';
        printf("%s\n", buffer);
    }

    close(sock);
    return 0;
}
