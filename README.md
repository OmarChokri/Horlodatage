# Horlodatage
Système Distribué avec Horodatage en C
Ce projet implémente un système distribué simple en C, où 4 processus communiquent via sockets TCP pour simuler des événements locaux et des échanges de messages. Chaque processus utilise un type d'horloge (scalaire, vectorielle ou matricielle) pour horodater les événements et gérer leur ordre.
Fonctionnalités

4 processus : Simulés par des clients (client.c) connectés à un serveur central (server.c).
Communication via sockets TCP : Les processus envoient des messages à un serveur qui les relaie aux autres processus.
3 types d'horloges :
Horloge scalaire : Simple compteur incrémenté à chaque événement.
Horloge vectorielle : Vecteur de taille 4 (un pour chaque processus) pour suivre les événements locaux et fusionner les horloges reçues.
Horloge matricielle : Matrice 4x4 représentant la vue de chaque processus sur les horloges des autres.


Instructions locales : Chaque processus effectue 5 instructions locales par itération (génération de nombres aléatoires, calculs simples).
Émissions de messages : Chaque processus envoie 4 messages au serveur, qui les relaie aux autres processus.

Prérequis

Système d'exploitation : Linux (ou WSL sur Windows).
Compilateur : gcc (GNU Compiler Collection).
Outils :
make (facultatif, pour automatiser la compilation).
Un terminal pour exécuter plusieurs processus simultanément (par exemple, via VS Code ou plusieurs fenêtres de terminal).



Installation des prérequis sur Linux/WSL

Mettre à jour les paquets :sudo apt update


Installer gcc :sudo apt install gcc


Vérifier l'installation :gcc --version



Structure du projet

server.c : Le serveur central qui écoute sur le port 8080, accepte les connexions des clients, reçoit leurs messages, et les relaie aux autres clients connectés.
client.c : Le client qui simule un processus. Chaque client :
Exécute des instructions locales.
Met à jour son horloge (scalaire, vectorielle ou matricielle).
Envoie des messages au serveur.
Reçoit des messages relayés par le serveur et met à jour son horloge.


README.md : Ce fichier.

Compilation

Compiler le serveur :gcc server.c -o server


Compiler le client :gcc client.c -o client



Avec make (facultatif)
Créez un fichier Makefile :
all: server client

server: server.c
	gcc server.c -o server

client: client.c
	gcc client.c -o client

clean:
	rm -f server client

Puis exécutez :
make

Exécution

Lancer le serveur :

Dans un terminal :./server


Le serveur écoute sur le port 8080 et affiche :Serveur en écoute sur le port 8080...




Lancer les clients :

Chaque client représente un processus (0 à 3) et utilise un type d'horloge spécifique.
Syntaxe : ./client <pid> <clock_type>
<pid> : Identifiant du processus (0, 1, 2, ou 3).
<clock_type> :
0 : Horloge scalaire.
1 : Horloge vectorielle.
2 : Horloge matricielle.




Configuration recommandée :
Processus 0 : Horloge scalaire.
Processus 1 : Horloge vectorielle.
Processus 2 : Horloge matricielle.
Processus 3 : Horloge vectorielle.


Dans des terminaux séparés :./client 0 0  # Processus 0, Horloge scalaire
./client 1 1  # Processus 1, Horloge vectorielle
./client 2 2  # Processus 2, Horloge matricielle
./client 3 1  # Processus 3, Horloge vectorielle





Exemple de sortie
Serveur
Serveur en écoute sur le port 8080...
Message reçu: Processus 0, Horloge scalaire: 1
Message reçu: Processus 1, Horloge vectorielle: [0,1,0,0]
Message reçu: Processus 2, Horloge matricielle: [0,0,0,0|0,0,0,0|0,0,1,0|0,0,0,0]
Message reçu: Processus 3, Horloge vectorielle: [0,0,0,1]
...

Client (Processus 0, Horloge scalaire)
Processus 0: Exécution d'opérations locales...
Processus 0: Résultat des opérations locales: 255
Horloge scalaire mise à jour: 1
Message envoyé: Processus 0, Horloge scalaire: 1
Message reçu: Processus 1, Horloge vectorielle: [0,1,0,0]
Horloge scalaire mise à jour: 2
...

Client (Processus 1, Horloge vectorielle)
Processus 1: Exécution d'opérations locales...
Processus 1: Résultat des opérations locales: 255
Horloge vectorielle mise à jour: [0,1,0,0]
Message envoyé: Processus 1, Horloge vectorielle: [0,1,0,0]
Message reçu: Processus 0, Horloge scalaire: 1
Horloge vectorielle mise à jour: [1,1,0,0]
...

Client (Processus 2, Horloge matricielle)
Processus 2: Exécution d'opérations locales...
Processus 2: Résultat des opérations locales: 255
Horloge matricielle mise à jour:
[0,0,0,0]
[0,0,0,0]
[0,0,1,0]
[0,0,0,0]
Message envoyé: Processus 2, Horloge matricielle: [0,0,0,0|0,0,0,0|0,0,1,0|0,0,0,0]
Message reçu: Processus 0, Horloge scalaire: 1
...

Dépannage
1. Erreur : "Bind failed: Address already in use"

Cause : Le port 8080 est déjà utilisé par un autre processus.
Solution :
Trouver le processus utilisant le port :netstat -tulnp | grep 8080


Tuer le processus :kill -9 <pid>


Relancer le serveur.



2. Les clients semblent bloqués

Cause : Les sockets TCP sont bloquantes par défaut, et les clients attendent indéfiniment des messages.
Solution :
Le code inclut déjà des sockets non bloquantes (fcntl avec O_NONBLOCK) et des délais (usleep). Assurez-vous que ces modifications sont appliquées.
Relancez les clients et laissez-les s'exécuter complètement.



3. Type d'horloge invalide

Cause : Vous avez passé un clock_type non valide (par exemple, ./client 1 3).
Solution :
Utilisez uniquement 0, 1, ou 2 comme clock_type.
Le code inclut une validation pour afficher une erreur si clock_type est invalide.



Améliorations possibles

Horloge matricielle complète :
Actuellement, la matrice est sérialisée et envoyée dans les messages, mais la fusion pourrait être améliorée pour mieux refléter les dépendances entre processus.


Synchronisation :
Ajouter des délais supplémentaires ou des mécanismes de synchronisation pour mieux observer les interactions.


Logs :
Ajouter des horodatages réels aux messages pour suivre l'ordre des événements :time_t now;
time(&now);
printf("[%s] %s\n", ctime(&now), msg);




Gestion des erreurs :
Ajouter des vérifications pour gérer les erreurs de connexion ou de réception.



Auteurs

Ce projet a été développé dans le cadre d'un exercice académique sur les systèmes distribués et l'horodatage.

Licence
Ce projet est sous licence MIT. Vous êtes libre de l'utiliser, le modifier et le distribuer selon les termes de cette licence.
---------------------------------------------------------------------------------------------------------------------------------------------------
Horloge : Simulation d'Horloges Distribuées
Aperçu
Ce projet Java simule un système distribué avec plusieurs processus communiquant via un réseau, implémentant trois types d'horloges logiques : horloge scalaire, vectorielle et matricielle. Chaque processus effectue des opérations locales et échange des messages avec les autres, mettant à jour son horloge en fonction du type d'horloge et des messages reçus. Le système illustre des concepts de calcul distribué, tels que la synchronisation des horloges et l'ordonnancement des événements.
Structure du Projet
Le projet comprend les fichiers suivants dans le package horloge :

Client.java : Implémente la logique côté client, incluant les opérations locales et la gestion des horloges (scalaire, vectorielle ou matricielle). Chaque client se connecte au serveur, effectue des opérations et échange des états d'horloge.
Server.java : Exécute un serveur qui accepte les connexions des clients et diffuse les messages à tous les clients connectés sauf l'émetteur.
Clock.java : Définit l'interface Clock avec des méthodes pour mettre à jour, fusionner et récupérer l'état des horloges.
LocalOperation.java : Définit l'interface LocalOperation pour exécuter des calculs locaux sur chaque processus.
MessageHandler.java : Fournit des méthodes utilitaires pour sérialiser et désérialiser les états des horloges vectorielles et matricielles.

Prérequis

Kit de développement Java (JDK) 8 ou supérieur
Interface en ligne de commande ou IDE (par exemple, IntelliJ IDEA, Eclipse)

Compilation et Exécution

Compiler le projet :Naviguez dans le répertoire source et compilez tous les fichiers Java :
javac horloge/*.java


Lancer le serveur :Démarrez le serveur, qui écoute sur le port 8080 :
java horloge.Server


Lancer les clients :Dans des fenêtres de terminal séparées, exécutez chaque client avec un identifiant de processus (pid) et un type d'horloge (clock_type) :
java horloge.Client <pid> <clock_type>


<pid> : Identifiant du processus (0 à 3, car le système supporte jusqu'à 4 processus).
<clock_type> :
0 : Horloge scalaire
1 : Horloge vectorielle
2 : Horloge matricielle



Exemples de commandes pour exécuter quatre clients avec différents types d'horloges :
java horloge.Client 0 1  # Processus 0 avec horloge vectorielle
java horloge.Client 1 2  # Processus 1 avec horloge matricielle
java horloge.Client 2 2  # Processus 2 avec horloge matricielle
java horloge.Client 3 0  # Processus 3 avec horloge scalaire



Flux du Programme

Serveur : Écoute les connexions des clients sur le port 8080. Lorsqu'un client envoie un message, le serveur le diffuse à tous les autres clients connectés.
Client :
Initialise une horloge selon le type spécifié (scalaire, vectorielle ou matricielle).
Exécute une boucle (4 itérations) :
Effectue une opération locale (calcul arithmétique aléatoire).
Met à jour son horloge.
Envoie l'état de son horloge au serveur.
Reçoit un message d'un autre client (via le serveur) et fusionne l'état de l'horloge reçue avec la sienne.




Horloges :
Horloge scalaire : Un entier unique incrémenté à chaque événement. Lors de la réception d'un message, elle s'incrémente sans tenir compte de la valeur reçue.
Horloge vectorielle : Un vecteur de taille N_PROCESSES (4), où chaque processus incrémente son propre indice. Lors de la réception d'un message, elle prend le maximum des éléments correspondants et incrémente son propre indice.
Horloge matricielle : Une matrice 4x4 où la diagonale représente le compte des événements locaux de chaque processus. Lors de la réception d'un message, elle fusionne en prenant le maximum des éléments correspondants et incrémente son propre élément diagonal.



Sortie

Serveur : Enregistre les messages reçus et les erreurs de connexion (par exemple, java.net.SocketException: Connection reset si un client se déconnecte).
Client : Affiche :
Les résultats des opérations locales.
Les mises à jour de l'horloge (état actuel de l'horloge scalaire, vectorielle ou matricielle).
Les messages envoyés et reçus avec leurs états d'horloge.



Exemple de sortie pour un client avec une horloge vectorielle :
Processus 0: Exécution d'opérations locales...
Processus 0: Résultat des opérations locales: 196
Horloge vectorielle mise à jour: [1,0,0,0]
Message envoyé: Processus 0, Horloge vectorielle: [1,0,0,0]
Message reçu: Processus 1, Horloge vectorielle: [0,1,0,0]
Horloge vectorielle mise à jour: [2,1,0,0]

Remarques

Le système suppose jusqu'à 4 processus (N_PROCESSES = 4).
Le serveur fonctionne indéfiniment jusqu'à ce qu'il soit arrêté manuellement.
Les clients s'exécutent pendant 4 itérations, puis ferment leurs connexions.
Des erreurs de réinitialisation de connexion peuvent survenir si les clients se déconnectent brusquement, mais le serveur continue de gérer les clients restants.
La classe MessageHandler suppose des formats de message spécifiques pour la désérialisation, ce qui peut provoquer des erreurs si les messages sont mal formés.

Dépannage

Erreurs de réinitialisation de connexion : Assurez-vous que les clients sont en cours d'exécution et correctement connectés. Vérifiez que le serveur est démarré avant de lancer les clients.
Erreurs de désérialisation : Vérifiez que les types d'horloges correspondent entre les clients s'ils échangent des messages (par exemple, les horloges vectorielles doivent uniquement fusionner avec des messages d'horloges vectorielles).
Conflits de port : Assurez-vous que le port 8080 est libre avant de démarrer le serveur.

Améliorations Futures

Ajouter une gestion d'erreurs pour les types d'horloges incompatibles lors de la fusion des messages.
Implémenter un mécanisme pour synchroniser les temps de démarrage des clients afin d'éviter les conditions de course.
Étendre le système pour supporter l'ajout/suppression dynamique de processus.
Enrichir les opérations locales avec des calculs plus complexes ou des scénarios réels.

