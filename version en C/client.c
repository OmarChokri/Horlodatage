#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 8080
#define MAX_MSG 2048
#define N_PROCESSES 4

// Horloge scalaire
typedef struct {
    int scalar;
    int pid;
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

// Structure pour gérer toutes les horloges
typedef struct {
    ScalarClock scalar;
    VectorClock vector;
    MatrixClock matrix;
    int pid;
    int clock_type; // 1: scalaire, 2: vectorielle, 3: matricielle
} Clock;

// Initialisation des horloges
void init_clock(Clock *clock, int pid, int clock_type) {
    clock->pid = pid;
    clock->clock_type = clock_type;

    // Initialisation horloge scalaire
    clock->scalar.scalar = 0;
    clock->scalar.pid = pid;

    // Initialisation horloge vectorielle
    for (int i = 0; i < N_PROCESSES; i++) {
        clock->vector.vector[i] = 0;
    }
    clock->vector.pid = pid;

    // Initialisation horloge matricielle
    for (int i = 0; i < N_PROCESSES; i++) {
        for (int j = 0; j < N_PROCESSES; j++) {
            clock->matrix.matrix[i][j] = 0;
        }
    }
    clock->matrix.pid = pid;
}

// Mise à jour de l'horloge scalaire
void update_scalar_clock(ScalarClock *clock) {
    clock->scalar++;
    printf("Horloge scalaire mise à jour: %d\n", clock->scalar);
}

// Fusion des horloges scalaires
void merge_scalar_clock(ScalarClock *clock, int received_scalar) {
    printf("Fusion de l'horloge scalaire locale %d avec reçue %d\n", clock->scalar, received_scalar);
    clock->scalar = (clock->scalar > received_scalar) ? clock->scalar : received_scalar;
    update_scalar_clock(clock);
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
    printf("Fusion de l'horloge vectorielle locale [");
    for (int i = 0; i < N_PROCESSES; i++) {
        printf("%d", clock->vector[i]);
        if (i < N_PROCESSES - 1) printf(",");
    }
    printf("] avec reçue [");
    for (int i = 0; i < N_PROCESSES; i++) {
        printf("%d", received_vector[i]);
        if (i < N_PROCESSES - 1) printf(",");
    }
    printf("]\n");

    for (int i = 0; i < N_PROCESSES; i++) {
        clock->vector[i] = (clock->vector[i] > received_vector[i]) ? clock->vector[i] : received_vector[i];
    }
    update_vector_clock(clock);
}

// Mise à jour de l'horloge matricielle
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

// Fusion des horloges matricielles (pour messages matriciels ou vectoriels)
void merge_matrix_clock(MatrixClock *clock, int received_matrix[N_PROCESSES][N_PROCESSES], int sender_pid, int is_vector, int *received_vector) {
    printf("Fusion de l'horloge matricielle locale avec reçue (Processus %d):\n", sender_pid);
    printf("Locale:\n");
    for (int i = 0; i < N_PROCESSES; i++) {
        printf("[");
        for (int j = 0; j < N_PROCESSES; j++) {
            printf("%d", clock->matrix[i][j]);
            if (j < N_PROCESSES - 1) printf(",");
        }
        printf("]\n");
    }

    if (is_vector) {
        // Si le message est un vecteur (ou scalaire converti en vecteur), on met à jour la ligne correspondant au sender_pid
        printf("Reçue (vecteur converti): [");
        for (int i = 0; i < N_PROCESSES; i++) {
            printf("%d", received_vector[i]);
            if (i < N_PROCESSES - 1) printf(",");
        }
        printf("] de Processus %d\n", sender_pid);

        // Mettre à jour la ligne sender_pid dans la matrice
        for (int j = 0; j < N_PROCESSES; j++) {
            clock->matrix[sender_pid][j] = received_vector[j];
        }

        // Fusionner avec les autres lignes (prendre le max pour chaque élément)
        for (int i = 0; i < N_PROCESSES; i++) {
            if (i != sender_pid) {
                for (int j = 0; j < N_PROCESSES; j++) {
                    clock->matrix[i][j] = (clock->matrix[i][j] > clock->matrix[sender_pid][j]) ? clock->matrix[i][j] : clock->matrix[sender_pid][j];
                }
            }
        }
    } else {
        // Si le message est une matrice complète
        printf("Reçue (matrice):\n");
        for (int i = 0; i < N_PROCESSES; i++) {
            printf("[");
            for (int j = 0; j < N_PROCESSES; j++) {
                printf("%d", received_matrix[i][j]);
                if (j < N_PROCESSES - 1) printf(",");
            }
            printf("]\n");
        }

        // Fusionner en prenant le maximum de chaque élément
        for (int i = 0; i < N_PROCESSES; i++) {
            for (int j = 0; j < N_PROCESSES; j++) {
                clock->matrix[i][j] = (clock->matrix[i][j] > received_matrix[i][j]) ? clock->matrix[i][j] : received_matrix[i][j];
            }
        }
    }

    // Incrémenter l'élément diagonal pour l'événement de réception
    update_matrix_clock(clock);
}

// Instructions locales (5 opérations)
void local_operations(int pid) {
    printf("Processus %d: Exécution d'opérations locales...\n", pid);
    int a = rand() % 100;
    int b = rand() % 100;
    int c = a + b;
    int d = c * 2;
    int e = d - a;
    printf("Processus %d: Résultat des opérations locales: %d\n", pid, e);
}

// Mise à jour de l'horloge en fonction du type
void update_clock(Clock *clock) {
    switch (clock->clock_type) {
        case 1:
            update_scalar_clock(&clock->scalar);
            break;
        case 2:
            update_vector_clock(&clock->vector);
            break;
        case 3:
            update_matrix_clock(&clock->matrix);
            break;
        default:
            printf("Type d'horloge invalide\n");
    }
}

// Formatage du message à envoyer
void format_message(Clock *clock, char *msg, size_t msg_size) {
    switch (clock->clock_type) {
        case 1:
            snprintf(msg, msg_size, "Processus %d, Horloge scalaire: %d", 
                     clock->pid, clock->scalar.scalar);
            break;
        case 2:
            snprintf(msg, msg_size, "Processus %d, Horloge vectorielle: [%d,%d,%d,%d]", 
                     clock->pid, clock->vector.vector[0], clock->vector.vector[1], 
                     clock->vector.vector[2], clock->vector.vector[3]);
            break;
        case 3: {
            char matrix_str[512] = "";
            for (int i = 0; i < N_PROCESSES; i++) {
                char row[128];
                snprintf(row, sizeof(row), "[%d,%d,%d,%d]", 
                         clock->matrix.matrix[i][0], clock->matrix.matrix[i][1], 
                         clock->matrix.matrix[i][2], clock->matrix.matrix[i][3]);
                strcat(matrix_str, row);
                if (i < N_PROCESSES - 1) strcat(matrix_str, ",");
            }
            snprintf(msg, msg_size, "Processus %d, Horloge matricielle: %s", 
                     clock->pid, matrix_str);
            break;
        }
        default:
            snprintf(msg, msg_size, "Type d'horloge invalide");
    }
}

// Parsing et fusion des messages reçus
void process_received_message(Clock *clock, char *buffer) {
    int sender_pid;
    if (sscanf(buffer, "Processus %d", &sender_pid) != 1 || sender_pid == clock->pid) {
        printf("Message ignoré (même processus ou format invalide)\n");
        return;
    }

    // Déterminer le type d'horloge du message reçu
    if (strstr(buffer, "Horloge scalaire")) {
        int received_scalar;
        if (sscanf(buffer, "Processus %d, Horloge scalaire: %d", &sender_pid, &received_scalar) != 2) {
            printf("Erreur de parsing de l'horloge scalaire\n");
            return;
        }

        switch (clock->clock_type) {
            case 1:
                merge_scalar_clock(&clock->scalar, received_scalar);
                break;
            case 2: {
                // Convertir le scalaire en vecteur pour fusion
                int received_vector[N_PROCESSES] = {0};
                received_vector[sender_pid] = received_scalar;
                merge_vector_clock(&clock->vector, received_vector);
                break;
            }
            case 3: {
                // Convertir le scalaire en vecteur pour fusion dans la matrice
                int received_vector[N_PROCESSES] = {0};
                received_vector[sender_pid] = received_scalar;
                int dummy_matrix[N_PROCESSES][N_PROCESSES] = {0};
                merge_matrix_clock(&clock->matrix, dummy_matrix, sender_pid, 1, received_vector);
                break;
            }
        }
    } else if (strstr(buffer, "Horloge vectorielle")) {
        int received_vector[N_PROCESSES];
        if (sscanf(buffer, "Processus %d, Horloge vectorielle: [%d,%d,%d,%d]", 
                   &sender_pid, &received_vector[0], &received_vector[1], 
                   &received_vector[2], &received_vector[3]) != 5) {
            printf("Erreur de parsing de l'horloge vectorielle\n");
            return;
        }

        switch (clock->clock_type) {
            case 1: {
                // Utiliser la composante correspondant au sender_pid comme scalaire
                int received_scalar = received_vector[sender_pid];
                merge_scalar_clock(&clock->scalar, received_scalar);
                break;
            }
            case 2:
                merge_vector_clock(&clock->vector, received_vector);
                break;
            case 3: {
                int dummy_matrix[N_PROCESSES][N_PROCESSES] = {0};
                merge_matrix_clock(&clock->matrix, dummy_matrix, sender_pid, 1, received_vector);
                break;
            }
        }
    } else if (strstr(buffer, "Horloge matricielle")) {
        int received_matrix[N_PROCESSES][N_PROCESSES];
        char *matrix_start = strstr(buffer, "Horloge matricielle: ");
        if (!matrix_start) {
            printf("Format de message matriciel invalide\n");
            return;
        }
        matrix_start += strlen("Horloge matricielle: ");
        int parsed = sscanf(matrix_start, 
            "[[%d,%d,%d,%d],[%d,%d,%d,%d],[%d,%d,%d,%d],[%d,%d,%d,%d]]", 
            &received_matrix[0][0], &received_matrix[0][1], &received_matrix[0][2], &received_matrix[0][3],
            &received_matrix[1][0], &received_matrix[1][1], &received_matrix[1][2], &received_matrix[1][3],
            &received_matrix[2][0], &received_matrix[2][1], &received_matrix[2][2], &received_matrix[2][3],
            &received_matrix[3][0], &received_matrix[3][1], &received_matrix[3][2], &received_matrix[3][3]);
        if (parsed != 16) {
            printf("Erreur de parsing de l'horloge matricielle\n");
            return;
        }

        switch (clock->clock_type) {
            case 1: {
                // Utiliser la composante diagonale du sender_pid comme scalaire
                int received_scalar = received_matrix[sender_pid][sender_pid];
                merge_scalar_clock(&clock->scalar, received_scalar);
                break;
            }
            case 2: {
                // Utiliser la ligne sender_pid comme vecteur
                int received_vector[N_PROCESSES];
                for (int j = 0; j < N_PROCESSES; j++) {
                    received_vector[j] = received_matrix[sender_pid][j];
                }
                merge_vector_clock(&clock->vector, received_vector);
                break;
            }
            case 3:
                merge_matrix_clock(&clock->matrix, received_matrix, sender_pid, 0, NULL);
                break;
        }
    } else {
        printf("Type d'horloge du message inconnu\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <pid> <clock_type>\n", argv[0]);
        return 1;
    }

    int pid = atoi(argv[1]);
    int clock_type = atoi(argv[2]);
    if (clock_type < 1 || clock_type > 3) {
        printf("Type d'horloge invalide (1: scalaire, 2: vectorielle, 3: matricielle)\n");
        return 1;
    }

    srand(time(NULL) + pid); // Seed différent pour chaque processus
    Clock clock;
    init_clock(&clock, pid, clock_type);

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

    // Boucle principale : 4 itérations
    for (int i = 0; i < 4; i++) {
        // Instructions locales
        local_operations(pid);

        // Mise à jour de l'horloge
        update_clock(&clock);

        // Envoi d'un message
        char msg[MAX_MSG];
        format_message(&clock, msg, MAX_MSG);
        if (send(sock, msg, strlen(msg), 0) < 0) {
            printf("Erreur lors de l'envoi du message\n");
            break;
        }
        printf("Message envoyé: %s\n", msg);

        // Attendre pour permettre l'échange de messages
        usleep(500000);

        // Réception d'un message
        int valread = read(sock, buffer, MAX_MSG - 1);
        if (valread > 0) {
            buffer[valread] = '\0'; // Assurer que le buffer est terminé par un caractère nul
            printf("Message reçu: %s\n", buffer);
            process_received_message(&clock, buffer);
        } else if (valread == 0) {
            printf("Connexion fermée par le serveur\n");
            break;
        } else {
            printf("Erreur lors de la réception du message\n");
            break;
        }

        memset(buffer, 0, MAX_MSG);
    }

    close(sock);
    return 0;
}