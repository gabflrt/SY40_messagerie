#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>

#define PORT 8080
#define BUF_SIZE 1024
#define MAX_CLIENTS 10

int client_sockets[MAX_CLIENTS] = {0};
int server_socket;
int active_connections = 0;

void signal_handler(int sig);

int main()
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    fd_set read_fds;
    int max_sd, new_socket, activity, valread;
    char buffer[BUF_SIZE];
    int opt = 1;

    // Gestion des signaux pour une extinction en douceur et le compte des connexions actives
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
    signal(SIGCHLD, SIG_IGN); // Pour éviter les processus zombies

    // Création du socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Échec de la création du socket");
        exit(EXIT_FAILURE);
    }

    // Définir les options du socket pour réutiliser l'adresse
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Configuration de la structure de l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Liaison du socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Échec de la liaison");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Écoute des connexions
    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        perror("Échec de l'écoute");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Le serveur écoute sur le port %d\n", PORT);

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        max_sd = server_socket;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_sockets[i] > 0)
            {
                FD_SET(client_sockets[i], &read_fds);
                if (client_sockets[i] > max_sd)
                    max_sd = client_sockets[i];
            }
        }

        activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0)
        {
            perror("Erreur de select");
            continue;
        }

        // Vérifier les nouvelles connexions
        if (FD_ISSET(server_socket, &read_fds))
        {
            new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
            if (new_socket == -1)
            {
                perror("Échec de l'acceptation");
                continue;
            }

            printf("Nouvelle connexion acceptée\n");
            active_connections++;
            printf("Nombre de connexions actives : %d\n", active_connections);

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = new_socket;
                    break;
                }
            }
        }

        // Vérifier les opérations d'E/S sur n'importe quel socket
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_sockets[i] > 0 && FD_ISSET(client_sockets[i], &read_fds))
            {
                valread = read(client_sockets[i], buffer, BUF_SIZE);
                if (valread == 0)
                {
                    // Client déconnecté
                    printf("Client %d déconnecté\n", i + 1);
                    close(client_sockets[i]);
                    client_sockets[i] = 0;
                    active_connections--;
                    printf("Nombre de connexions actives : %d\n", active_connections);
                }
                else
                {
                    buffer[valread] = '\0';

                    // Envoyer le message aux clients jumelés
                    for (int j = 0; j < MAX_CLIENTS; j++)
                    {
                        if (client_sockets[j] > 0 && j != i && j / 2 == i / 2)
                        {
                            if (write(client_sockets[j], buffer, strlen(buffer)) == -1)
                            {
                                perror("Échec de l'écriture au client");
                            }
                        }
                    }
                }
            }
        }
    }

    close(server_socket);
    return 0;
}

void signal_handler(int sig)
{
    if (sig == SIGUSR1)
    {
        printf("Arrêt du serveur...\n");
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_sockets[i] > 0)
            {
                close(client_sockets[i]);
            }
        }
        close(server_socket);
        exit(0);
    }
    else if (sig == SIGUSR2)
    {
        printf("Nombre de connexions actives : %d\n", active_connections);
    }
}
