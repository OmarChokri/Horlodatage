#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 8080
#define MAX_MSG 1024
#define N_PROCESSES 4

// Horloge vectorielle
typedef struct {
    int vector[N_PROCESSES];
    int pid;
} VectorClock;

// Initialisation de l'horloge vectorielle
void init_vector_clock(VectorClock *clock, int pid) {
    for (int i = 0; i < N_PROCESSES; i++) {
        clock->vector[i] = 0;
    }
    clock->pid = pid;
}

// Mise à jour de l'horloge vectorielle
void update_vector_clock(VectorClock *clock) {
    clock->vector[clock->pid]++;
    printf("Horloge vectorielle mise à jour: [");
    for (int i = 0; i < N_PROCESSES; i++) {
        printf("%d", clock->vector[i]);
        if (i < N_PROCESSES - 1) printf(",");
    }
    printf("]\n");
}

// Fusion des horloges vectorielles
void merge_vector_clock(VectorClock *clock, int *received_vector) {
    printf("Fusion de l'horloge locale [");
    for (int i = 0; i < N_PROCESSES; i++) {
        printf("%d", clock->vector[i]);
        if (i < N_PROCESSES - 1) printf(",");
    }
    printf("] avec l'horloge reçue [");
    for (int i = 0; i < N_PROCESSES; i++) {
        printf("%d", received_vector[i]);
        if (i < N_PROCESSES - 1) printf(",");
    }
    printf("]\n");

    // Merge by taking the maximum of each component
    for (int i = 0; i < N_PROCESSES; i++) {
        clock->vector[i] = (clock->vector[i] > received_vector[i]) ? clock->vector[i] : received_vector[i];
    }

    // Increment the local component for the receive event
    update_vector_clock(clock);
}

// Instructions locales
void local_operations(int pid) {
    printf("Processus %d: Exécution d'opérations locales...\n", pid);
    int a = rand() % 100;
    int b = rand() % 100;
    int c = a + b;
    int d = c * 2;
    int e = d - a;
    printf("Processus %d: Résultat des opérations locales: %d\n", pid, e);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <pid> <clock_type>\n", argv[0]);
        return 1;
    }

    int pid = atoi(argv[1]);
    int clock_type = atoi(argv[2]); // We'll force clock_type to 1 (vector clock)

    if (clock_type != 1) {
        printf("Forcing vector clock (clock_type = 1) for synchronization.\n");
        clock_type = 1;
    }

    VectorClock vector_clock;
    init_vector_clock(&vector_clock, pid);

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[MAX_MSG] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Erreur de création de socket\n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Adresse invalide\n");
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connexion échouée\n");
        return 1;
    }

    // Boucle principale
    for (int i = 0; i < 4; i++) {
        // Instructions locales
        local_operations(pid);

        // Mise à jour de l'horloge
        update_vector_clock(&vector_clock);

        // Envoi d'un message avec l'horloge vectorielle
        char msg[MAX_MSG];
        snprintf(msg, MAX_MSG, "Processus %d, Horloge vectorielle: [%d,%d,%d,%d]", 
                 pid, vector_clock.vector[0], vector_clock.vector[1], vector_clock.vector[2], vector_clock.vector[3]);
        send(sock, msg, strlen(msg), 0);
        printf("Message envoyé: %s\n", msg);

        // Attendre un court instant pour permettre à l'autre processus d'envoyer un message
        usleep(500000); // 500ms delay to ensure message exchange

        // Réception d'un message
        int valread = read(sock, buffer, MAX_MSG);
        if (valread > 0) {
            buffer[valread] = '\0'; // Ensure the buffer is null-terminated
            printf("Message reçu: %s\n", buffer);

            // Extraire l'horloge vectorielle du message reçu
            int received_vector[N_PROCESSES];
            int sender_pid;
            int parsed = sscanf(buffer, "Processus %d, Horloge vectorielle: [%d,%d,%d,%d]", 
                                &sender_pid, &received_vector[0], &received_vector[1], 
                                &received_vector[2], &received_vector[3]);
            
            if (parsed == 5 && sender_pid != pid) { // Ensure we parsed correctly and it's not our own message
                // Fusionner l'horloge vectorielle reçue avec l'horloge locale
                merge_vector_clock(&vector_clock, received_vector);
            } else {
                printf("Aucun message pertinent reçu ou message envoyé par soi-même.\n");
            }
        } else {
            printf("Aucun message reçu.\n");
        }

        // Clear the buffer for the next iteration
        memset(buffer, 0, MAX_MSG);
    }

    close(sock);
    return 0;
}