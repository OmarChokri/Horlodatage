package horloge;

import java.io.*;
import java.net.*;
import java.util.*;

public class Server {
    private static final int PORT = 8080;
    private static final int MAX_CLIENTS = 4;

    public static void main(String[] args) throws Exception {
        ServerSocket serverSocket = new ServerSocket(PORT);
        System.out.println("Serveur en écoute sur le port " + PORT + "...");
        List<Socket> clients = new ArrayList<>();

        while (true) {
            Socket clientSocket = serverSocket.accept();
            clients.add(clientSocket);
            new Thread(() -> handleClient(clientSocket, clients)).start();
        }
    }

    private static void handleClient(Socket clientSocket, List<Socket> clients) {
        try {
            BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            String msg;
            while ((msg = in.readLine()) != null) {
                System.out.println("Message reçu: " + msg);
                // Diffuser le message à tous les autres clients
                for (Socket otherClient : clients) {
                    if (otherClient != clientSocket && !otherClient.isClosed()) {
                        PrintWriter out = new PrintWriter(otherClient.getOutputStream(), true);
                        out.println(msg);
                    }
                }
            }
            clientSocket.close();
            clients.remove(clientSocket);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}