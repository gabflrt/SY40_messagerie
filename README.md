
# Projet de Serveur et Client en C
## Description
Ce projet implémente un serveur et un client en C pour établir une communication en utilisant des sockets TCP. Le serveur peut gérer plusieurs connexions clients et permettre la transmission de messages entre les clients.

## Fichiers
- server.c: Implémente le serveur qui écoute sur un port spécifié, accepte les connexions clients, et transmet les messages entre clients jumelés.
- client.c: Implémente le client qui se connecte au serveur et permet à l'utilisateur d'envoyer et de recevoir des messages.
- Makefile: Fichier Makefile pour compiler et nettoyer les fichiers du projet.
### Prérequis
- Un compilateur C (par exemple, gcc)
- Un système Unix/Linux
### Compilation
Pour compiler le serveur et le client, utilisez les commandes suivantes `make`

## Utilisation
### Lancer le Serveur
Pour démarrer le serveur, exécutez :

```sh
./bin/server
```
Le serveur écoute sur le port 8080 par défaut. Vous pouvez modifier le port en changeant la valeur de la constante PORT dans le code source.

### Lancer le Client
Pour démarrer le client, exécutez :

```sh
./bin/client <client_name>
```
Remplacez <client_name> par le nom du client que vous souhaitez utiliser. Ce nom sera affiché avec les messages envoyés.

### Communication entre Clients
Les messages envoyés par un client seront transmis aux autres clients jumelés (client 0 avec client 1, client 2 avec client 3, etc.).

### Gestion des Signaux
Le serveur gère les signaux suivants :
- SIGUSR1 : Arrête le serveur et ferme toutes les connexions clients.
- SIGUSR2 : Affiche le nombre de connexions actives.

Pour envoyer un signal au serveur, vous pouvez utiliser la commande kill avec l'identifiant de processus (PID) du serveur. Par exemple :

```sh
kill -SIGUSR1 <server_pid>
```

## Fonctionnalités
### Serveur
- Accepte les connexions de plusieurs clients (jusqu'à 10 par défaut).
- Transmet les messages entre clients jumelés.
- Affiche le nombre de connexions actives.
- Gestion des signaux pour arrêter le serveur proprement et afficher le nombre de connexions actives.
### Client
- Se connecte au serveur et permet d'envoyer et de recevoir des messages.
- Affiche les messages reçus des autres clients.
- Réessaie de se connecter au serveur en cas d'échec (jusqu'à 5 fois par défaut).


## Nettoyage
Pour nettoyer les fichiers objets et les exécutables générés, utilisez la commande : `make clean`

