package horloge;

import java.io.*;
import java.net.*;
import java.util.Random;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

public class Client {
    private static final int PORT = 8080;
    private static final int N_PROCESSES = 4;
    private static JTextArea outputArea;

    // Implémentation des opérations locales
    static class BasicLocalOperation implements LocalOperation {
        @Override
        public int execute(int pid) {
            String message = "Processus " + pid + ": Exécution d'opérations locales...\n";
            outputArea.append(message);
            Random rand = new Random();
            int a = rand.nextInt(100);
            int b = rand.nextInt(100);
            int c = a + b;
            int d = c * 2;
            int e = d - a;
            message = "Processus " + pid + ": Résultat des opérations locales: " + e + "\n";
            outputArea.append(message);
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
            outputArea.append("Horloge scalaire mise à jour: " + value + "\n");
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
            StringBuilder message = new StringBuilder("Horloge vectorielle mise à jour: [");
            for (int i = 0; i < N_PROCESSES; i++) {
                message.append(vector[i]);
                if (i < N_PROCESSES - 1) message.append(",");
            }
            message.append("]\n");
            outputArea.append(message.toString());
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
            outputArea.append("Horloge matricielle mise à jour:\n");
            for (int i = 0; i < N_PROCESSES; i++) {
                outputArea.append("[");
                for (int j = 0; j < N_PROCESSES; j++) {
                    outputArea.append(String.valueOf(matrix[i][j]));
                    if (j < N_PROCESSES - 1) outputArea.append(",");
                }
                outputArea.append("]\n");
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

    public static void main(String[] args) {
        JFrame frame = new JFrame("Horloge System");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(800, 400);
        frame.setLayout(new BorderLayout());

        // Panel for controls
        JPanel controlPanel = new JPanel();
        controlPanel.setLayout(new FlowLayout());

        // Process ID selection
        JLabel pidLabel = new JLabel("Processus ID (PID): ");
        JComboBox<String> pidComboBox = new JComboBox<>(new String[]{"0", "1", "2", "3"});
        controlPanel.add(pidLabel);
        controlPanel.add(pidComboBox);

        // Clock Type selection
        JLabel clockTypeLabel = new JLabel("Type d'Horloge: ");
        JComboBox<String> clockTypeComboBox = new JComboBox<>(new String[]{"Scalaire", "Vectorielle", "Matricielle"});
        controlPanel.add(clockTypeLabel);
        controlPanel.add(clockTypeComboBox);

        // Start button
        JButton startButton = new JButton("Lancer le Client");
        controlPanel.add(startButton);

        // Output area
        outputArea = new JTextArea(15, 70);
        outputArea.setEditable(false);
        JScrollPane scrollPane = new JScrollPane(outputArea);

        // Add components to frame
        frame.add(controlPanel, BorderLayout.NORTH);
        frame.add(scrollPane, BorderLayout.CENTER);

        // Action listener for start button
        startButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                outputArea.setText(""); // Clear previous output
                int pid = Integer.parseInt((String) pidComboBox.getSelectedItem());
                int clockType;
                String selectedClockType = (String) clockTypeComboBox.getSelectedItem();
                switch (selectedClockType) {
                    case "Scalaire":
                        clockType = 0;
                        break;
                    case "Vectorielle":
                        clockType = 1;
                        break;
                    default:
                        clockType = 2;
                        break;
                }

                // Run client in a separate thread to avoid freezing the GUI
                new Thread(() -> {
                    try {
                        runClient(pid, clockType);
                    } catch (Exception ex) {
                        outputArea.append("Erreur: " + ex.getMessage() + "\n");
                        ex.printStackTrace();
                    }
                }).start();
            }
        });

        frame.setVisible(true);
    }

    private static void runClient(int pid, int clockType) throws Exception {
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
            outputArea.append("Message envoyé: " + msg + "\n");

            // Réception d'un message
            String received = in.readLine();
            if (received != null) {
                outputArea.append("Message reçu: " + received + "\n");
                clock.merge(received);
            }
        }

        socket.close();
    }
}