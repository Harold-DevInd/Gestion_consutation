#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <signal.h> 
#include <pthread.h> 
#include <ctype.h>
#include "../protocol/TCP.h" 
#include "../protocol/CBP.h" 

void HandlerSIGINT(int s); 
void TraitementConnexion(int sService); 
void* FctThreadClient(void* p); 
int sEcoute; 
int NB_THREADS_POOL;
int PORT_ECOUTE;

int load_config(const char *filename, int *portE, int *nbrP);

// Gestion du pool de threads 
#define TAILLE_FILE_ATTENTE 20 
int socketsAcceptees[TAILLE_FILE_ATTENTE]; 
int indiceEcriture=0, indiceLecture=0; 
pthread_mutex_t mutexSocketsAcceptees; 
pthread_cond_t  condSocketsAcceptees; 

int main(int argc,char* argv[]) 
{ 
    // Initialisation socketsAcceptees 
    pthread_mutex_init(&mutexSocketsAcceptees,NULL); 
    pthread_cond_init(&condSocketsAcceptees,NULL); 
    for (int i=0 ; i<TAILLE_FILE_ATTENTE ; i++) 
        socketsAcceptees[i] = -1; 

    // Armement des signaux 
    struct sigaction A; 
    A.sa_flags = 0; 
    sigemptyset(&A.sa_mask); 
    A.sa_handler = HandlerSIGINT; 
    if (sigaction(SIGINT,&A,NULL) == -1) 
    { 
        perror("Erreur de sigaction"); 
        exit(1); 
    }

    //Lecture des parametre du serveur
    if(load_config("./serveur/server.conf", &PORT_ECOUTE, &NB_THREADS_POOL) == -1)
    {
        perror("Erreur de chargement du fichier de configuration"); 
        exit(1);
    } 

    // Creation de la socket d'écoute 
    if ((sEcoute = ServerSocket(PORT_ECOUTE)) == -1) 
    { 
        perror("Erreur de ServeurSocket"); 
        exit(1); 
    } 

    // Creation du pool de threads 
    printf("Création du pool de threads.\n"); 
    pthread_t th; 
    for (int i=0 ; i<NB_THREADS_POOL ; i++) 
    pthread_create(&th,NULL,FctThreadClient,NULL); 

    // Mise en boucle du serveur 
    int sService; 
    char ipClient[50]; 
    printf("Demarrage du serveur.\n"); 
    while(1) 
    { 
        printf("Attente d'une connexion...\n"); 
        if ((sService = Accept(sEcoute,ipClient)) == -1) 
        { 
            perror("Erreur de Accept"); 
            close(sEcoute); 
            CBP_Close(); 
            exit(1); 
        } 
        printf("Connexion acceptée : IP=%s socket=%d\n",ipClient,sService); 

        // Insertion en liste d'attente et réveil d'un thread du pool 
        // (Production d'une tâche) 
        pthread_mutex_lock(&mutexSocketsAcceptees); 
        socketsAcceptees[indiceEcriture] = sService; // !!! 
        indiceEcriture++; 

        if (indiceEcriture == TAILLE_FILE_ATTENTE) indiceEcriture = 0; 

        pthread_mutex_unlock(&mutexSocketsAcceptees); 
        pthread_cond_signal(&condSocketsAcceptees); 
    } 
} 

void* FctThreadClient(void* p) 
{ 
    int sService; 
    while(1) 
    { 
        printf("\t[THREAD %ld] Attente socket...\n",pthread_self()); 

        // Attente d'une tâche 
        pthread_mutex_lock(&mutexSocketsAcceptees); 

        while (indiceEcriture == indiceLecture) 
            pthread_cond_wait(&condSocketsAcceptees,&mutexSocketsAcceptees); 

        sService = socketsAcceptees[indiceLecture]; 
        socketsAcceptees[indiceLecture] = -1; 
        indiceLecture++; 

        if (indiceLecture == TAILLE_FILE_ATTENTE) indiceLecture = 0; 

        pthread_mutex_unlock(&mutexSocketsAcceptees); 

        // Traitement de la connexion (consommation de la tâche) 
        printf("\t[THREAD %ld] Je m'occupe de la socket %d\n", 
        pthread_self(),sService); 
        TraitementConnexion(sService); 
    } 
} 

void HandlerSIGINT(int s) 
{ 
    printf("\nArret du serveur.\n"); 
    close(sEcoute); 

    pthread_mutex_lock(&mutexSocketsAcceptees); 

    for (int i=0 ; i<TAILLE_FILE_ATTENTE ; i++) 
        if (socketsAcceptees[i] != -1) close(socketsAcceptees[i]); 

    pthread_mutex_unlock(&mutexSocketsAcceptees); 
    CBP_Close(); 
    exit(0); 
}

void TraitementConnexion(int sService) 
{ 
    char requete[200], reponse[200]; 
    int nbLus, nbEcrits; 
    bool onContinue = true; 

    while (onContinue) 
    { 
        printf("\t[THREAD %ld] Attente requete...\n",pthread_self()); 

        // ***** Reception Requete ****************** 
        if ((nbLus = Receive(sService,requete)) < 0) 
        { 
            perror("Erreur de Receive"); 
            close(sService); 
            HandlerSIGINT(0); 
        } 

        // ***** Fin de connexion ? ***************** 
        if (nbLus == 0) 
        { 
            printf("\t[THREAD %ld] Fin de connexion du client.\n",pthread_self()); 
            close(sService); 
            return; 
        } 
        requete[nbLus] = 0; 
        printf("\t[THREAD %ld] Requete recue = %s\n",pthread_self(),requete); 

        // ***** Traitement de la requete *********** 
        onContinue = CBP(requete,reponse,sService); 

        // ***** Envoi de la reponse **************** 
        if ((nbEcrits = Send(sService,reponse,strlen(reponse))) < 0) 
        { 
            perror("Erreur de Send"); 
            close(sService); 
            HandlerSIGINT(0); 
        } 

        printf("\t[THREAD %ld] Reponse envoyee = %s\n",pthread_self(),reponse); 
        
        if (!onContinue)  
            printf("\t[THREAD %ld] Fin de connexion de la socket %d\n",pthread_self(),sService); 
    }
}

/* Helper pour supprimer espaces et retours de ligne */
static void trim(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;      // trim left
    if (*str == 0) return;                           // empty string
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
}

/* Fonction de chargement de configuration */
int load_config(const char *filename, int *portE, int *nbrP)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Impossible d'ouvrir le fichier de configuration");
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        // Ignorer lignes vides et commentaires
        if (line[0] == '#' || strlen(line) < 3)
            continue;

        trim(line);
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "=");
        if (!key || !value)
            continue;

        trim(key);
        trim(value);

        if (strcmp(key, "PORT_RESERVATION") == 0) {
            *portE = atoi(value);
        } else if (strcmp(key, "NB_THREADS") == 0) {
            *nbrP = atoi(value);
        } else {
            printf("Clé inconnue dans config : %s\n", key);
        }
    }

    fclose(f);
    return 0;
}