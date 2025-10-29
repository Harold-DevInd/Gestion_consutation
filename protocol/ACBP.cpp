#include "CBP.h"
#include "ACBP.h" 
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h>
#include <mysql.h>
#include <stdbool.h>
#include <errno.h>
using namespace std;

CLIENT liste_client[NB_MAX_CLIENTS];
int socket_admin;

//***** Parsing de la requete et creation de la reponse ************* 
bool ACBP(char* requete, char* reponse,int socketAdmin)
{ 
    socket_admin = socketAdmin;
    // ***** Récupération nom de la requete ***************** 
    char *ptr = strtok(requete,"#"); 

    // ***** GET_SPECIALITES ******************************************* 
    if (strcmp(ptr,"LIST_CLIENT") == 0) 
    { 
        printf("\t\n[THREAD ADMIN %ld] OPERATION %s\n",pthread_self(), ptr); 

         
    }

    return true; 
} 

//***** Traitement des requetes ************************************* 
bool ACBP_ListClient()
{
    return true;
}

//***** Fin prématurée ********************************************** 
void ACBP_Close() 
{ 
    close(socket_admin); 
}