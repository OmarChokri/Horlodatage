package horloge;

public interface Clock {
    void update(); // Mise à jour de l'horloge
    void merge(String receivedMessage); // Fusion avec un message reçu
    String getClockState(); // Retourne l'état de l'horloge pour l'envoi
}