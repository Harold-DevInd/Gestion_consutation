#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <string.h>     // pour memset 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netdb.h> 
#include "TCP.h"
using namespace std;

int ServerSocket(int portServ)
{
    int sEcoute;
    printf("pid = %d\n",getpid()); 

    // Creation de la socket 
    if ((sEcoute = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    { 
        perror("Erreur de socket()"); 
        exit(1); 
    } 
    printf("socket creee = %d\n",sEcoute); 

    // Construction de l'adresse 
    struct addrinfo hints; 
    struct addrinfo *results; 
    memset(&hints,0,sizeof(struct addrinfo)); 
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;  

    // pour une connexion passive
    char PortServ[10];
    sprintf(PortServ, "%d",portServ);
    if (getaddrinfo(NULL, PortServ,&hints,&results) != 0) 
        exit(1); 

    // Affichage du contenu de l'adresse obtenue 
    char host[NI_MAXHOST]; 
    char port[NI_MAXSERV]; 
    getnameinfo(results->ai_addr,results->ai_addrlen, host,NI_MAXHOST,port,NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST); 
    //printf("Mon Adresse IP: %s -- Mon Port: %s\n",host,port); 

    // Liaison de la socket à l'adresse 
    int value = 1 ; 
    setsockopt(sEcoute,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(int));
    if (bind(sEcoute,results->ai_addr,results->ai_addrlen) < 0) 
    { 
        perror("Erreur de bind()"); 
        exit(1); 
    } 
    freeaddrinfo(results); 
    printf("bind() reussi !\n");

    // Mise à l'écoute de la socket 
    if (listen(sEcoute,SOMAXCONN) == -1) 
    { 
        perror("Erreur de listen()"); 
        exit(1); 
    } 
    printf("listen() reussi !\n"); 

    return sEcoute;
}

int Accept(int sEcoute,char *ipClient) 
{
    char host[NI_MAXHOST]; 
    char port[NI_MAXSERV];

    // Attente d'une connexion 
    int sService; 
    if ((sService = accept(sEcoute,NULL,NULL)) == -1) 
    { 
        perror("Erreur de accept()"); 
        exit(1); 
    } 
    printf("accept() reussi !\n"); 
    printf("socket de service = %d\n",sService);

    // Recuperation d'information sur le client connecte 
    struct sockaddr_in adrClient; 
    socklen_t adrClientLen = sizeof(struct sockaddr_in); // nécessaire 
    getpeername(sService,(struct sockaddr*)&adrClient,&adrClientLen); 
    getnameinfo((struct sockaddr*)&adrClient,adrClientLen,host,NI_MAXHOST,port,NI_MAXSERV,NI_NUMERICSERV | NI_NUMERICHOST); 
    //printf("Client connecte --> Adresse IP: %s -- Port: %s\n",host,port); 

    //Renvoi de l'addresse du client connecte via la variable ipClient 
    if(ipClient != NULL)
        sprintf(ipClient, "%s", host);

    return sService;
}

int ClientSocket(char* ipServeur,int portServeur)
{
    if(portServeur < 0)
        printf("\n Numero de port invalide ");

    int sClient;
    char PortServ[10];
    snprintf(PortServ, sizeof(PortServ), "%d",portServeur);
    printf("pid du Client = %d\n",getpid()); 

    // Creation de la socket 
    if ((sClient = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    { 
        perror("Erreur de socket()"); 
        exit(1); 
    } 
    printf("socket creee = %d\n",sClient); 
    
    // Construction de l'adresse du serveur 
    struct addrinfo hints; 
    struct addrinfo *results; 

    memset(&hints,0,sizeof(struct addrinfo)); 
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = AI_NUMERICSERV; 
    if (getaddrinfo(ipServeur, PortServ,&hints,&results) != 0) 
        exit(1); 
    
    // Demande de connexion 
    if (connect(sClient,results->ai_addr,results->ai_addrlen) == -1) 
    { 
        perror("Erreur de connect()"); 
        exit(1); 
    } 
    printf("connect() reussi !"); 

    return sClient;
} 

int Send(int sSocket,char* data,int taille)
{
    //Ajout d un caractere de fin
    int x;
    char entete[5];
    snprintf(entete, sizeof(entete), "%04d", taille);

    if ((x = write(sSocket, entete, 4)) != 4) 
    { 
        perror("Erreur de write() lors du passage de la taille du message"); 
        return -1;   
    } 

    // Ecriture sur la socket 
    char message[taille];
    int nb;

    sprintf(message, "%s", data); 
    if ((nb = write(sSocket,message, taille)) == -1) 
    { 
        perror("Erreur de write()"); 
        return -1;   
    } 
     
    return nb;
}

int Receive(int sSocket,char* data)
{
    // Lecture sur la socket 
    int nb = 0; 
    char entette[5];
    int taille;

    if((read(sSocket, entette, 4)) == 1){
        perror("Erreur de read() lors de lma lecture de la taille du message");
        return -1;
    }

    entette[4]= '\0';
    taille = atoi(entette);

    if((nb = read(sSocket, data, taille)) == -1)
    {
        perror("Erreur de read()");
        return -1;
    }

    /* terminaison de la chaîne */
    return nb;
} 