#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// Structure pour stocker les données des registres
typedef struct {
    int coil;
    int discreteInput;
    int holdingRegister;
    int inputRegister;
} ModbusRegisters;

// Structure pour stocker les informations du thread
typedef struct {
    int clientSocket;
    ModbusRegisters* registers;
    pthread_mutex_t* registersMutex;
} ThreadData;

// Structure pour représenter une trame Modbus TCP
typedef struct {
    unsigned short transactionId;
    unsigned short protocolId;
    unsigned short length;
    unsigned char unitId;
    unsigned char functionCode;
    unsigned short address;
    unsigned short data;
} ModbusFrame;

// Fonction exécutée par chaque thread
void* handleClient(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int clientSocket = data->clientSocket;
    ModbusRegisters* registers = data->registers;
    pthread_mutex_t* registersMutex = data->registersMutex;

    // Gérer la communication Modbus avec le client
    ModbusFrame request;
    ssize_t bytesRead = recv(clientSocket, (void*)&request, sizeof(ModbusFrame), 0);
    if (bytesRead <= 0) {
        perror("Erreur lors de la réception de la trame de requête");
        close(clientSocket);
        free(data);
        pthread_exit(NULL);
    }

    // Analyser la trame de requête
    unsigned char unitId = request.unitId;
    unsigned char functionCode = request.functionCode;
    unsigned short address = ntohs(request.address);
    unsigned short requestData = ntohs(request.data);

    // Traiter la requête et effectuer les opérations appropriées sur les registres
    unsigned short responseData = 0;
    pthread_mutex_lock(registersMutex);
    switch (functionCode) {
        case 0x01: // Read Coil Status
            switch (address) {
                case 0x0000:
                    responseData = registers->coil;
                    break;
            }
            break;
        case 0x03: // Read Holding Registers
            switch (address) {
                case 0x0000:
                    responseData = registers->holdingRegister;
                    break;
            }
            break;
        case 0x04: // Read Input Registers
            switch (address) {
                case 0x0000:
                    responseData = registers->inputRegister;
                    break;
            }
            break;
        case 0x05: // Write Single Coil
            switch (address) {
                case 0x0000:
                    registers->coil = requestData;
                    responseData = requestData;
                    break;
            }
            break;
        case 0x06: // Write Single Register
            switch (address) {
                case 0x0000:
                    registers->holdingRegister = requestData;
                    responseData = requestData;
                    break;
            }
            break;
        default:
            break;
    }
    pthread_mutex_unlock(registersMutex);

    // Construire la trame de réponse Modbus
    ModbusFrame response;
    response.transactionId = request.transactionId;
    response.protocolId = request.protocolId;
    response.length = htons(sizeof(ModbusFrame) - sizeof(response.transactionId) - sizeof(response.protocolId));
    response.unitId = unitId;
    response.functionCode = functionCode;
    response.address = htons(address);
    response.data = htons(responseData);

    // Envoyer la trame de réponse au client
    ssize_t bytesSent = send(client
Socket, (void*)&response, sizeof(ModbusFrame), 0);
    if (bytesSent <= 0) {
        perror("Erreur lors de l'envoi de la trame de réponse");
    }

    // Fermer la socket du client
    close(clientSocket);

    // Libérer la mémoire
    free(data);
    pthread_exit(NULL);
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    unsigned short serverPort = 502;  // Port Modbus TCP

    // Créer la socket du serveur
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    // Préparer l'adresse du serveur
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(serverPort);

    // Lier la socket à l'adresse du serveur
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Erreur lors du bind");
        exit(EXIT_FAILURE);
    }

    // Écouter les connexions entrantes
    if (listen(serverSocket, 5) == -1) {
        perror("Erreur lors de l'écoute");
        exit(EXIT_FAILURE);
    }

    printf("Le serveur est en écoute sur le port %d...\n", serverPort);

    // Mutex pour verrouiller l'accès aux registres partagés
    pthread_mutex_t registersMutex;
    pthread_mutex_init(&registersMutex, NULL);

    // Boucle principale du serveur
    while (1) {
        socklen_t clientAddrLen = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            perror("Erreur lors de l'acceptation de la connexion");
            continue;
        }

        // Créer une structure de données pour le thread
        ThreadData* data = (ThreadData*)malloc(sizeof(ThreadData));
        data->clientSocket = clientSocket;
        data->registers = (ModbusRegisters*)malloc(sizeof(ModbusRegisters));
        // Initialiser les registres à des valeurs par défaut
        data->registers->coil = 0;
        data->registers->holdingRegister = 0;
        data->registers->inputRegister = 0;
        data->registersMutex = &registersMutex;

        // Créer un nouveau thread pour gérer la connexion client
        pthread_t thread;
        if (pthread_create(&thread, NULL, handleClient, (void*)data) != 0) {
            perror("Erreur lors de la création du thread");
            close(clientSocket);
            free(data);
            continue;
        }
    }

    // Fermer la socket du serveur
    close(serverSocket);

    // Détruire le mutex
    pthread_mutex_destroy(&registersMutex);

    return 0;
}
