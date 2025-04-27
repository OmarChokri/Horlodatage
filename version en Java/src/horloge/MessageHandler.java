package horloge;

public class MessageHandler {
    private static final int N_PROCESSES = 4;

    // Sérialise l'état d'une horloge matricielle
    public static String serializeMatrix(int[][] matrix) {
        StringBuilder msgBuilder = new StringBuilder("[");
        for (int i = 0; i < N_PROCESSES; i++) {
            msgBuilder.append("[");
            for (int j = 0; j < N_PROCESSES; j++) {
                msgBuilder.append(matrix[i][j]);
                if (j < N_PROCESSES - 1) msgBuilder.append(",");
            }
            msgBuilder.append("]");
            if (i < N_PROCESSES - 1) msgBuilder.append(",");
        }
        msgBuilder.append("]");
        return msgBuilder.toString();
    }

    // Désérialise un vecteur à partir d'un message
    public static int[] deserializeVector(String message) {
        String vectorStr = message.split("\\[")[1].split("\\]")[0];
        String[] parts = vectorStr.split(",");
        int[] vector = new int[N_PROCESSES];
        for (int i = 0; i < N_PROCESSES; i++) {
            vector[i] = Integer.parseInt(parts[i]);
        }
        return vector;
    }

    // Désérialise une matrice à partir d'un message
    public static int[][] deserializeMatrix(String message) {
        String matrixStr = message.split(": ")[1];
        matrixStr = matrixStr.substring(1, matrixStr.length() - 1);
        String[] rows = matrixStr.split("\\],\\[");
        int[][] matrix = new int[N_PROCESSES][N_PROCESSES];
        for (int i = 0; i < N_PROCESSES; i++) {
            String row = rows[i].replaceAll("[\\[\\]]", "");
            String[] values = row.split(",");
            for (int j = 0; j < N_PROCESSES; j++) {
                matrix[i][j] = Integer.parseInt(values[j]);
            }
        }
        return matrix;
    }
}