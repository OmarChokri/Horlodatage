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

// Horloge scalaire
typedef struct {
    int value;
} ScalarClock;

// Horloge vectorielle
typedef struct {
    int vector[N_PROCESSES];
    int pid;
} VectorClock;

// Horloge matricielle
typedef struct {
    int matrix[N_PROCESSES][N_PROCESSES];
    int pid;
} MatrixClock;

// Initialisation des horloges
void init_scalar_clock(ScalarClock *clock) {
    clock->value = 0;
}

void init_vector_clock(VectorClock *clock, int pid) {
    for (int i = 0; i < N_PROCESSES; i++) {
        clock->vector[i] = 0;
    }
    clock->pid = pid;
}

void init_matrix_clock(MatrixClock *clock, int pid) {
    for (int i = 0; i < N_PROCESSES; i++) {
        for (int j = 0; j < N_PROCESSES; j++) {
            clock->matrix[i][j] = 0;
        }
    }
    clock->pid = pid;
}

// Mise à jour des horloges
void update_scalar_clock(ScalarClock *clock) {
    clock->value++;
    printf("Horloge scalaire mise à jour: %d\n", clock->value);
}

void update_vector_clock(VectorClock *clock) {
    clock->vector[clock->pid]++;
    printf("Horloge vectorielle mise à jour: [");
    for (int i = 0; i < N_PROCESSES; i++) {
        printf("%d", clock->vector[i]);
        if (i < N_PROCESSES - 1) printf(",");
    }
    printf("]\n");
}

void update_matrix_clock(MatrixClock *clock) {
    clock->matrix[clock->pid][clock->pid]++;
    printf("Horloge matricielle mise à jour:\n");
    for (int i = 0; i < N_PROCESSES; i++) {
        printf("[");
        for (int j = 0; j < N_PROCESSES; j++) {
            printf("%d", clock->matrix[i][j]);
            if (j < N_PROCESSES - 1) printf(",");
        }
        printf("]\n");
    }
}

// Fusion des horloges vectorielles
void merge_vector_clock(VectorClock *clock, int *received_vector) {
    for (int i = 0; i < N_PROCESSES; i++) {
        clock->vector[i] = (clock->vector[i] > received_vector[i]) ? clock->vector[i] : received_vector[i];
    }
    update_vector_clock(clock);
}

// Fusion des horloges matricielles
void merge_matrix_clock(MatrixClock *clock, int received_matrix[N_PROCESSES][N_PROCESSES]) {
    for (int i = 0; i < N_PROCESSES; i++) {
        for (int j = 0; j < N_PROCESSES; j++) {
            clock->matrix[i][j] = (clock->matrix[i][j] > received_matrix[i][j]) ? clock->matrix[i][j] : received_matrix[i][j];
        }
    }
    update_matrix_clock(clock);
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
    int clock_type = atoi(argv[2]); // 0: scalaire, 1: vectorielle, 2: matricielle

    ScalarClock scalar_clock;
    VectorClock vector_clock;
    MatrixClock matrix_clock;

    if (clock_type == 0) {
        init_scalar_clock(&scalar_clock);
    } else if (clock_type == 1) {
        init_vector_clock(&vector_clock, pid);
    } else {
        init_matrix_clock(&matrix_clock, pid);
    }

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
        if (clock_type == 0) {
            update_scalar_clock(&scalar_clock);
        } else if (clock_type == 1) {
            update_vector_clock(&vector_clock);
        } else {
            update_matrix_clock(&matrix_clock);
        }

        // Envoi d'un message
        char msg[MAX_MSG];
        if (clock_type == 0) {
            snprintf(msg, MAX_MSG, "Processus %d, Horloge scalaire: %d", pid, scalar_clock.value);
        } else if (clock_type == 1) {
            snprintf(msg, MAX_MSG, "Processus %d, Horloge vectorielle: [%d,%d,%d,%d]", 
                     pid, vector_clock.vector[0], vector_clock.vector[1], vector_clock.vector[2], vector_clock.vector[3]);
        } else {
            snprintf(msg, MAX_MSG, "Processus %d, Horloge matricielle", pid);
        }
        send(sock, msg, strlen(msg), 0);
        printf("Message envoyé: %s\n", msg);

        // Réception d'un message
        int valread = read(sock, buffer, MAX_MSG);
        if (valread > 0) {
            printf("Message reçu: %s\n", buffer);
            if (clock_type == 1) {
                int received_vector[N_PROCESSES];
                sscanf(buffer, "Processus %*d, Horloge vectorielle: [%d,%d,%d,%d]", 
                       &received_vector[0], &received_vector[1], &received_vector[2], &received_vector[3]);
                merge_vector_clock(&vector_clock, received_vector);
            } else if (clock_type == 2) {
                // Simulation de réception d'une matrice (simplifiée)
                int received_matrix[N_PROCESSES][N_PROCESSES] = {0};
                merge_matrix_clock(&matrix_clock, received_matrix);
            } else {
                update_scalar_clock(&scalar_clock);
            }
        }
    }

    close(sock);
    return 0;
}