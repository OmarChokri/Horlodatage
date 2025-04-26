#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_MSG 1024
#define MAX_CLIENTS 4

int main() {
    int server_fd, new_socket, client_sockets[MAX_CLIENTS] = {0};
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[MAX_MSG] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Erreur de création de socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Erreur de bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Erreur de listen");
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Erreur d'accept");
            exit(EXIT_FAILURE);
        }

        // Ajouter le nouveau client
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = new_socket;
                break;
            }
        }

        // Gérer les messages des clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] > 0) {
                int valread = read(client_sockets[i], buffer, MAX_MSG);
                if (valread > 0) {
                    printf("Message reçu: %s\n", buffer);
                    // Renvoyer le message à tous les clients
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (client_sockets[j] > 0 && client_sockets[j] != client_sockets[i]) {
                            send(client_sockets[j], buffer, strlen(buffer), 0);
                        }
                    }
                } else if (valread == 0) {
                    close(client_sockets[i]);
                    client_sockets[i] = 0;
                }
            }
        }
    }

    close(server_fd);
    return 0;
}