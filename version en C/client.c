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

typedef struct {
    int scalar;
    int pid;
} ScalarClock;

typedef struct {
    int vector[N_PROCESSES];
    int pid;
} VectorClock;

typedef struct {
    int matrix[N_PROCESSES][N_PROCESSES];
    int pid;
} MatrixClock;

typedef struct {
    ScalarClock scalar;
    VectorClock vector;
    MatrixClock matrix;
    int pid;
    int clock_type;
} Clock;

void init_clock(Clock *clock, int pid, int clock_type) {
    clock->pid = pid;
    clock->clock_type = clock_type;

    clock->scalar.scalar = 0;
    clock->scalar.pid = pid;

    for (int i = 0; i < N_PROCESSES; i++) {
        clock->vector.vector[i] = 0;
    }
    clock->vector.pid = pid;

    for (int i = 0; i < N_PROCESSES; i++) {
        for (int j = 0; j < N_PROCESSES; j++) {
            clock->matrix.matrix[i][j] = 0;
        }
    }
    clock->matrix.pid = pid;
}

void update_scalar_clock(ScalarClock *clock) {
    clock->scalar++;
    printf("Horloge scalaire mise à jour: %d\n", clock->scalar);
}

void merge_scalar_clock(ScalarClock *clock, int received_scalar) {
    printf("Fusion de l'horloge scalaire locale %d avec reçue %d\n", clock->scalar, received_scalar);
    clock->scalar = (clock->scalar > received_scalar) ? clock->scalar : received_scalar;
    update_scalar_clock(clock);
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
        printf("Reçue (vecteur converti): [");
        for (int i = 0; i < N_PROCESSES; i++) {
            printf("%d", received_vector[i]);
            if (i < N_PROCESSES - 1) printf(",");
        }
        printf("] de Processus %d\n", sender_pid);

        for (int j = 0; j < N_PROCESSES; j++) {
            clock->matrix[sender_pid][j] = received_vector[j];
        }

        for (int i = 0; i < N_PROCESSES; i++) {
            if (i != sender_pid) {
                for (int j = 0; j < N_PROCESSES; j++) {
                    clock->matrix[i][j] = (clock->matrix[i][j] > clock->matrix[sender_pid][j]) ? clock->matrix[i][j] : clock->matrix[sender_pid][j];
                }
            }
        }
    } else {
        printf("Reçue (matrice):\n");
        for (int i = 0; i < N_PROCESSES; i++) {
            printf("[");
            for (int j = 0; j < N_PROCESSES; j++) {
                printf("%d", received_matrix[i][j]);
                if (j < N_PROCESSES - 1) printf(",");
            }
            printf("]\n");
        }

        for (int i = 0; i < N_PROCESSES; i++) {
            for (int j = 0; j < N_PROCESSES; j++) {
                clock->matrix[i][j] = (clock->matrix[i][j] > received_matrix[i][j]) ? clock->matrix[i][j] : received_matrix[i][j];
            }
        }

        for (int i = 0; i < N_PROCESSES; i++) {
            if (i != sender_pid) {
                for (int j = 0; j < N_PROCESSES; j++) {
                    clock->matrix[i][j] = (clock->matrix[i][j] > clock->matrix[sender_pid][j]) ? clock->matrix[i][j] : clock->matrix[sender_pid][j];
                }
            }
        }
    }

    update_matrix_clock(clock);
}

void local_operations(int pid) {
    printf("Processus %d: Exécution d'opérations locales...\n", pid);
    int a = rand() % 100;
    int b = rand() % 100;
    int c = a + b;
    int d = c * 2;
    int e = d - a;
    printf("Processus %d: Résultat des opérations locales: %d\n", pid, e);
}

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
            snprintf(msg, msg_size, "Processus %d, Horloge matricielle: [%s]", 
                     clock->pid, matrix_str);
            break;
        }
        default:
            snprintf(msg, msg_size, "Type d'horloge invalide");
    }
}

void process_received_message(Clock *clock, char *buffer) {
    int sender_pid;
    if (sscanf(buffer, "Processus %d", &sender_pid) != 1 || sender_pid == clock->pid) {
        printf("Message ignoré (même processus ou format invalide)\n");
        return;
    }

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
                int received_vector[N_PROCESSES] = {0};
                received_vector[sender_pid] = received_scalar;
                merge_vector_clock(&clock->vector, received_vector);
                break;
            }
            case 3: {
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
        int received_matrix[N_PROCESSES][N_PROCESSES] = {0};
        char *matrix_start = strstr(buffer, "Horloge matricielle: ");
        if (!matrix_start) {
            printf("Format de message matriciel invalide\n");
            return;
        }
        matrix_start += strlen("Horloge matricielle: ");

        // Ajuster pour gérer les crochets extérieurs
        if (*matrix_start == '[') matrix_start++; // Sauter le premier '['

        // Parser manuellement les valeurs de la matrice
        char *ptr = matrix_start;
        for (int i = 0; i < N_PROCESSES; i++) {
            // Sauter les caractères inutiles (crochets, virgules)
            while (*ptr && (*ptr == '[' || *ptr == ',' || *ptr == ' ')) ptr++;

            // Parser une ligne de la matrice
            for (int j = 0; j < N_PROCESSES; j++) {
                if (*ptr == '\0' || *ptr == ']') break;
                received_matrix[i][j] = atoi(ptr);
                
                // Avancer jusqu'au prochain nombre ou délimiteur
                while (*ptr && *ptr != ',' && *ptr != ']') ptr++;
                if (*ptr == ',') ptr++; // Sauter la virgule
            }

            // Sauter jusqu'à la fin de la ligne
            while (*ptr && *ptr != '[' && *ptr != '\0') ptr++;
        }

        // Vérifier si le parsing a réussi
        int valid = 1;
        for (int i = 0; i < N_PROCESSES; i++) {
            for (int j = 0; j < N_PROCESSES; j++) {
                if (received_matrix[i][j] < 0) { // Les valeurs doivent être positives ou nulles
                    valid = 0;
                    break;
                }
            }
            if (!valid) break;
        }

        if (!valid) {
            printf("Erreur de parsing de l'horloge matricielle\n");
            return;
        }

        switch (clock->clock_type) {
            case 1: {
                int received_scalar = received_matrix[sender_pid][sender_pid];
                merge_scalar_clock(&clock->scalar, received_scalar);
                break;
            }
            case 2: {
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

    srand(time(NULL) + pid);
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

    for (int i = 0; i < 4; i++) {
        local_operations(pid);
        update_clock(&clock);

        char msg[MAX_MSG];
        format_message(&clock, msg, MAX_MSG);
        if (send(sock, msg, strlen(msg), 0) < 0) {
            printf("Erreur lors de l'envoi du message\n");
            break;
        }
        printf("Message envoyé: %s\n", msg);

        usleep(500000);

        int valread = read(sock, buffer, MAX_MSG - 1);
        if (valread > 0) {
            buffer[valread] = '\0';
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
