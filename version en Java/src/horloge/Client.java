package horloge;

import java.io.*;
import java.net.*;
import java.util.Random;

public class Client {
    private static final int PORT = 8080;
    private static final int N_PROCESSES = 4;

    // Implémentation des opérations locales
    static class BasicLocalOperation implements LocalOperation {
        @Override
        public int execute(int pid) {
            System.out.println("Processus " + pid + ": Exécution d'opérations locales...");
            Random rand = new Random();
            int a = rand.nextInt(100);
            int b = rand.nextInt(100);
            int c = a + b;
            int d = c * 2;
            int e = d - a;
            System.out.println("Processus " + pid + ": Résultat des opérations locales: " + e);
            return e;
        }
    }

    // Horloge scalaire
    static class ScalarClock implements Clock {
        int value;

        ScalarClock() {
            value = 0;
        }

        @Override
        public void update() {
            value++;
            System.out.println("Horloge scalaire mise à jour: " + value);
        }

        @Override
        public void merge(String receivedMessage) {
            update(); // Incrémente simplement lors de la réception
        }

        @Override
        public String getClockState() {
            return String.valueOf(value);
        }
    }

    // Horloge vectorielle
    static class VectorClock implements Clock {
        int[] vector;
        int pid;

        VectorClock(int pid) {
            vector = new int[N_PROCESSES];
            this.pid = pid;
        }

        @Override
        public void update() {
            vector[pid]++;
            System.out.print("Horloge vectorielle mise à jour: [");
            for (int i = 0; i < N_PROCESSES; i++) {
                System.out.print(vector[i]);
                if (i < N_PROCESSES - 1) System.out.print(",");
            }
            System.out.println("]");
        }

        @Override
        public void merge(String receivedMessage) {
            int[] receivedVector = MessageHandler.deserializeVector(receivedMessage);
            for (int i = 0; i < N_PROCESSES; i++) {
                vector[i] = Math.max(vector[i], receivedVector[i]);
            }
            update();
        }

        @Override
        public String getClockState() {
            StringBuilder sb = new StringBuilder("[");
            for (int i = 0; i < N_PROCESSES; i++) {
                sb.append(vector[i]);
                if (i < N_PROCESSES - 1) sb.append(",");
            }
            sb.append("]");
            return sb.toString();
        }
    }

    // Horloge matricielle
    static class MatrixClock implements Clock {
        int[][] matrix;
        int pid;

        MatrixClock(int pid) {
            matrix = new int[N_PROCESSES][N_PROCESSES];
            this.pid = pid;
        }

        @Override
        public void update() {
            matrix[pid][pid]++;
            System.out.println("Horloge matricielle mise à jour:");
            for (int i = 0; i < N_PROCESSES; i++) {
                System.out.print("[");
                for (int j = 0; j < N_PROCESSES; j++) {
                    System.out.print(matrix[i][j]);
                    if (j < N_PROCESSES - 1) System.out.print(",");
                }
                System.out.println("]");
            }
        }

        @Override
        public void merge(String receivedMessage) {
            if (receivedMessage.contains("Horloge matricielle")) {
                int[][] receivedMatrix = MessageHandler.deserializeMatrix(receivedMessage);
                for (int i = 0; i < N_PROCESSES; i++) {
                    for (int j = 0; j < N_PROCESSES; j++) {
                        matrix[i][j] = Math.max(matrix[i][j], receivedMatrix[i][j]);
                    }
                }
                update();
            }
        }

        @Override
        public String getClockState() {
            return MessageHandler.serializeMatrix(matrix);
        }
    }

    public static void main(String[] args) throws Exception {
        if (args.length != 2) {
            System.out.println("Usage: java Client <pid> <clock_type>");
            return;
        }

        int pid = Integer.parseInt(args[0]);
        int clockType = Integer.parseInt(args[1]); // 0: scalaire, 1: vectorielle, 2: matricielle

        // Initialisation de l'horloge
        Clock clock;
        if (clockType == 0) {
            clock = new ScalarClock();
        } else if (clockType == 1) {
            clock = new VectorClock(pid);
        } else {
            clock = new MatrixClock(pid);
        }

        // Initialisation des opérations locales
        LocalOperation localOp = new BasicLocalOperation();

        Socket socket = new Socket("localhost", PORT);
        PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
        BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

        // Boucle principale
        for (int i = 0; i < 4; i++) {
            // Instructions locales
            localOp.execute(pid);

            // Mise à jour de l'horloge
            clock.update();

            // Envoi d'un message
            String clockTypeStr = clockType == 0 ? "Horloge scalaire" : clockType == 1 ? "Horloge vectorielle" : "Horloge matricielle";
            String msg = "Processus " + pid + ", " + clockTypeStr + ": " + clock.getClockState();
            out.println(msg);
            System.out.println("Message envoyé: " + msg);

            // Réception d'un message
            String received = in.readLine();
            if (received != null) {
                System.out.println("Message reçu: " + received);
                clock.merge(received);
            }
        }

        socket.close();
    }
}