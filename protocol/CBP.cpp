#include "CBP.h" 
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h>
#include <mysql.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>      // Pour types comme socklen_t
#include <sys/socket.h>     // Pour socket(), bind(), accept(), etc.
#include <netinet/in.h>     // Pour sockaddr_in, htons(), etc.
#include <arpa/inet.h>  
using namespace std;

//***** Etat du protocole : liste des clients loggés **************** 
int clients[NB_MAX_CLIENTS];
CLIENT liste_clients[NB_MAX_CLIENTS]; 
int nbClients = 0; 
int  estPresent(int socket); 
void ajoute(int socket, CLIENT client); 
void retire(int socket);
pthread_mutex_t mutexClients = PTHREAD_MUTEX_INITIALIZER; 

//***** Parsing de la requete et creation de la reponse ************* 
bool CBP(char* requete, char* reponse,int socket) 
{ 
    // ***** Récupération nom de la requete ***************** 
    char *ptr = strtok(requete,"#"); 

    // ***** LOGIN ****************************************** 
    if (strcmp(ptr,"LOGIN") == 0)  
    { 
        char lastName[50], firstName[50];
        int numero, existe;
        CLIENT client;
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        getpeername(socket, (struct sockaddr*)&client_addr, &addr_len);

        strcpy(lastName,strtok(NULL,"#")); 
        strcpy(firstName,strtok(NULL,"#")); 
        numero = atoi(strtok(NULL,"#"));
        existe = atoi(strtok(NULL,"#"));
        printf("\t[THREAD %ld] LOGIN de %s %s\n",pthread_self(),lastName, firstName); 

        if (estPresent(socket) >= 0)  // client déjà loggé 
        { 
            sprintf(reponse,"LOGIN#ko#Client déjà loggé !"); 
        } 
        else 
        { 
            if (CBP_Login(lastName,firstName, numero, &existe)) 
            {   

                sprintf(reponse,"LOGIN#ok#%d", existe); 

                //Recuperation des infos sur le client
                strcpy(client.ip_client, inet_ntoa(client_addr.sin_addr));
                strcpy(client.ip_client, firstName);
                strcpy(client.ip_client, lastName);
                client.numero_patient = existe;

                ajoute(socket, client);
            }       
            else 
            { 
                sprintf(reponse,"LOGIN#ko#Mauvais identifiants !");
            } 
        } 
    } 

    // ***** LOGOUT ***************************************** 
    if (strcmp(ptr,"LOGOUT") == 0) 
    { 
        printf("\t[THREAD %ld] LOGOUT\n",pthread_self()); 
        retire(socket); 
        sprintf(reponse,"LOGOUT#ok");
    } 

    // ***** GET_SPECIALITES ******************************************* 
    if (strcmp(ptr,"GET_SPEC") == 0) 
    { 
        printf("\t\n[THREAD %ld] OPERATION %s\n",pthread_self(), ptr); 

        if (estPresent(socket) == -1) sprintf(reponse,"GET_SPEC#ko#Client non loggé !");  
        else 
        { 
            int nbr;
            char resultat[2048];

            if(CBP_Get_Specialities(&nbr, resultat)) 
            {
                sprintf(reponse,"GET_SPEC#ok|%d|%s", nbr, resultat);
                return true;
            }
            else
            {
                sprintf(reponse,"GET_SPEC#ko#Erreur lors de la recherche des specialites");
                return false;
            }
        } 
    }

    // ***** GET_DOCTORS ******************************************* 
    if (strcmp(ptr,"GET_DOC") == 0) 
    { 
        printf("\t\n[THREAD %ld] OPERATION %s\n",pthread_self(), ptr); 

        if (estPresent(socket) == -1) sprintf(reponse,"GET_DOC#ko#Client non loggé !");  
        else 
        { 
            int nbr;
            char resultat[2048];

            if(CBP_Get_Doctors(&nbr, resultat)) 
            {
                sprintf(reponse,"GET_DOC#ok#%d|%s", nbr, resultat);
                return true;
            }
            else
            {
                sprintf(reponse,"GET_DOC#ko#Erreur lors de la recherche des specialites");
                return false;
            }
        }
         
    }

    // ***** SEARCH_CONSULTATIONS ******************************************* 
    if (strcmp(ptr,"SEARCH_CONS") == 0) 
    { 
        printf("\t\n[THREAD %ld] OPERATION %s\n",pthread_self(), ptr); 

        char specialite[30], nomdoctor[30], prenomdoctor[30], startDate[20], endDate[20];
        int nbr;

        strcpy(specialite,strtok(NULL,"#")); 
        strcpy(nomdoctor,strtok(NULL,"#"));
        strcpy(prenomdoctor,strtok(NULL,"#"));
        strcpy(startDate,strtok(NULL,"#"));
        strcpy(endDate,strtok(NULL,"#"));

        if (estPresent(socket) == -1) sprintf(reponse,"SEARCH_CONS#ko#Client non loggé !");  
        else 
        { 
            char resultat[2048];

            if(CBP_Search_Consultations(specialite, nomdoctor, prenomdoctor, startDate, endDate, &nbr, resultat)) 
            {
                sprintf(reponse,"SEARCH_CONS#ok#%d|%s", nbr, resultat);
                return true;
            }
            else
            {
                sprintf(reponse,"SEARCH_CONS#ko#Erreur lors de la recherche des consultations");
                return false;
            }
        }
    }

    // ***** BOOK_CONSULTATION ******************************************* 
    if (strcmp(ptr,"BOOK_CONS") == 0) 
    { 
        printf("\t\n[THREAD %ld] OPERATION %s\n",pthread_self(), ptr); 

        if (estPresent(socket) == -1) sprintf(reponse,"BOOK_CONS#ko#Client non loggé !");  
        else 
        { 
            int idC, idP;
            char raison[100];

            idC = atoi(strtok(NULL,"#"));
            idP = atoi(strtok(NULL,"#"));
            strcpy(raison, strtok(NULL,"#"));

            if(CBP_Book_Consultation(idC, idP, raison)) 
            {
                sprintf(reponse,"BOOK_CONS#ok");
                return true;
            }
            else
            {
                sprintf(reponse,"BOOK_CONS#ko#Erreur lors de la reservation de la consultation");
                return false;
            }
        }
    }
    if(strcmp(ptr,"INFO_CLIENTS") == 0)
    {
        printf("\t\n[THREAD %ld] OPERATION %s\n",pthread_self(), ptr); 

        int idC, idP;
        char raison[100];

        idC = atoi(strtok(NULL,"#"));
        idP = atoi(strtok(NULL,"#"));
        strcpy(raison, strtok(NULL,"#"));

        if(CBP_Book_Consultation(idC, idP, raison)) 
        {
            sprintf(reponse,"BOOK_CONS#ok");
            return true;
        }
        else
        {
            sprintf(reponse,"BOOK_CONS#ko#Erreur lors de la reservation de la consultation");
            return false;
        }
        
    }

    return true; 
} 

//***** Traitement des requetes ************************************* 
bool CBP_Login(const char* last_name,const char* first_name, int numero, int* existe)
{
    MYSQL* connexion;
    MYSQL_RES  *resultat;
    MYSQL_ROW  Tuple;
    char requete[200];

    // Connexion à la base de donnée
    connexion = mysql_init(NULL);
    if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
    {
        fprintf(stderr,"(CBP) Erreur de connexion à la base de données...\n");
        return false;  
    }
    fprintf(stderr,"\n(CBP) apres connexion a la BD\n");

    // Construction de la requête SELECT
    if(*existe == -1)
        sprintf(requete, "INSERT INTO patients VALUES (NULL, '%s', '%s', NULL);", last_name, first_name);
    else
        sprintf(requete, "SELECT * FROM patients WHERE id = %d;", numero);

    // Exécution de la requête
    if (mysql_query(connexion, requete)) {
        fprintf(stderr, "Erreur lors de la requête SELECT : %s\n", mysql_error(connexion));
        mysql_close(connexion);
        return false;
    }

    if(*existe == -1)
    {
        int nb = mysql_affected_rows(connexion);
        *existe = mysql_insert_id(connexion);
        if (nb == 1)
            return true;
        else
            return false;
    }
    else
    {
        // Récupération des résultats
        resultat = mysql_store_result(connexion);
        if (resultat == NULL) {
            fprintf(stderr, "Erreur lors de la récupération des résultats : %s\n", mysql_error(connexion));
            mysql_close(connexion);
            return false;
        }

        if ((Tuple = mysql_fetch_row(resultat))) 
        {
            int id;
            char nom[50], prenom[50];
            id = atoi(Tuple[0]); //id
            strncpy(nom, Tuple[1], sizeof(nom)); //last name
            strncpy(prenom, Tuple[2], sizeof(prenom)); // first name
            *existe = id;

            if (strcmp(last_name, nom)==0 && strcmp(first_name, prenom)==0 && (id == numero)) 
                return true;
        }
        else
            return false;
    }
    

    return false;
} 

bool CBP_Get_Specialities(int* nbr_Specialite, char* liste_Specialite)
{
    MYSQL_RES  *resultat;
    MYSQL_ROW  Tuple;
    char requete[200];
    int nbr = 0;
    char nom[30];
    string listeSpec; 

    // Construction de la requête SELECT pour la liste de specialite
    sprintf(requete, "SELECT name FROM specialties;");

    // Exécution de la  requette pour la liste de specialite
    if(!RechercheBD(requete, &resultat))
        return false;
    
    *nbr_Specialite = mysql_num_rows(resultat);

    while(nbr < *nbr_Specialite)
    {
        if ((Tuple = mysql_fetch_row(resultat))) 
        {
            strncpy(nom, Tuple[0], sizeof(nom)); //nom

            listeSpec += string(nom);
            listeSpec += "#";

            nbr++;
        }
        else
            return false;
    }

    if(nbr == *nbr_Specialite)
    {
        strcpy(liste_Specialite, listeSpec.c_str());
        return true;
    }

    return false;
}

bool CBP_Get_Doctors(int* nbr_Doctor, char* liste_Doctor)
{
    MYSQL_RES  *resultat;
    MYSQL_ROW  Tuple;
    char requete[200];
    char nom[30], prenom[30];
    int nbr = 0;
    string listeDoc;

    // Construction de la requête SELECT pour la liste des docteurs
    sprintf(requete, "SELECT last_name, first_name FROM doctors;");

    // Exécution de la  requette pour a liste des docteurs
    if(!RechercheBD(requete, &resultat))
        return false;

    //Nombre de docteurs recupere
    *nbr_Doctor = mysql_num_rows(resultat);

    while(nbr < *nbr_Doctor)
    {
        if ((Tuple = mysql_fetch_row(resultat))) 
        {
            strncpy(nom, Tuple[0], sizeof(nom)); //nom
            strncpy(prenom, Tuple[1], sizeof(prenom)); //prenom

            listeDoc += string(nom);
            listeDoc += " ";
            listeDoc += string(prenom);
            listeDoc += "#";

            nbr++;
        }
        else
            return false;
    }

    if(nbr == *nbr_Doctor)
    {
        strcpy(liste_Doctor, listeDoc.c_str());
        return true;
    }

    return false;
}

bool CBP_Search_Consultations(char *Speciality, char *nomDoctor, char *prenomDoctor, char* date_Debut, char* date_Fin, int* nbr_Consultation, char* liste_Consultation)
{
    MYSQL_RES  *resultat;
    MYSQL_ROW  Tuple;
    char requete[524];
    int nbr;

    // Construction de la requête SELECT pour la liste des consultations
    if((strcmp(Speciality, "0") == 0) && (strcmp(nomDoctor, "0") == 0) && (strcmp(prenomDoctor, "0") == 0))
    {
        sprintf(requete, "SELECT c.id, sp.name AS specialty, d.last_name, d.first_name, c.date, c.hour \
                FROM consultations c \
                JOIN doctors d ON c.doctor_id = d.id \
                JOIN specialties sp ON d.specialty_id = sp.id \
                WHERE c.patient_id IS NULL AND c.date BETWEEN '%s' AND '%s' \
                ORDER BY c.id;", date_Debut, date_Fin);
    }
    else if((strcmp(Speciality, "0") == 0) && (strcmp(nomDoctor, "0") != 0) && (strcmp(prenomDoctor, "0") != 0))
    {
        sprintf(requete, "SELECT c.id, sp.name AS specialty, d.last_name, d.first_name, c.date, c.hour \
                FROM consultations c \
                JOIN doctors d ON c.doctor_id = d.id \
                JOIN specialties sp ON d.specialty_id = sp.id \
                WHERE c.patient_id IS NULL AND d.last_name LIKE '%s' AND d.first_name LIKE '%s' \
                AND c.date BETWEEN '%s' AND '%s' \
                ORDER BY c.id;", nomDoctor, prenomDoctor, date_Debut, date_Fin);
    }
    else if((strcmp(Speciality, "0") != 0) && (strcmp(nomDoctor, "0") == 0) && (strcmp(prenomDoctor, "0") == 0))
    {
        sprintf(requete, "SELECT c.id, sp.name AS specialty, d.last_name, d.first_name, c.date, c.hour \
                FROM consultations c \
                JOIN doctors d ON c.doctor_id = d.id \
                JOIN specialties sp ON d.specialty_id = sp.id \
                WHERE c.patient_id IS NULL AND sp.name LIKE '%s' AND c.date BETWEEN '%s' AND '%s' \
                ORDER BY c.id;", Speciality, date_Debut, date_Fin);
    }
    else
        sprintf(requete, "SELECT c.id, sp.name AS specialty, d.last_name, d.first_name, c.date, c.hour \
                FROM consultations c \
                JOIN doctors d ON c.doctor_id = d.id \
                JOIN specialties sp ON d.specialty_id = sp.id \
                WHERE c.patient_id IS NULL AND sp.name LIKE '%s' AND d.last_name LIKE '%s' AND d.first_name LIKE '%s' \
                AND c.date BETWEEN '%s' AND '%s' \
                ORDER BY c.id;", Speciality, nomDoctor, prenomDoctor, date_Debut, date_Fin); //

    // Exécution de la  requette pour la liste des consultations
    if(!RechercheBD(requete, &resultat))
        return false;

    //Nombre de docteurs recupere
    *nbr_Consultation = mysql_num_rows(resultat);

    int id;
    char nomDoc[50], prenomDoc[30], nomSpe[50], date[20], heure[20];
    string listeConsul;

    while(nbr < *nbr_Consultation)
    {
        if ((Tuple = mysql_fetch_row(resultat))) 
        {
            string doctor;
            id = atoi(Tuple[0]); //id
            strncpy(nomSpe, Tuple[1], sizeof(nomSpe)); //nom specialite
            strncpy(nomDoc, Tuple[2], sizeof(nomDoc)); //nom medecin
            strncpy(prenomDoc, Tuple[3], sizeof(prenomDoc)); //prenom medecin
            strncpy(date, Tuple[4], sizeof(date)); //date
            strncpy(heure, Tuple[5], sizeof(heure)); //heure

            doctor += string(nomDoc);
            doctor += " ";
            doctor += string(prenomDoc);

            listeConsul += to_string(id);
            listeConsul += "#";
            listeConsul += string(nomSpe);
            listeConsul += "#";
            listeConsul += doctor;
            listeConsul += "#";
            listeConsul += string(date);
            listeConsul += "#";
            listeConsul += string(heure);
            listeConsul += "#";

            nbr++;
        }
        else
            return false;
    }

    if(nbr == *nbr_Consultation)
    {
        strcpy(liste_Consultation, listeConsul.c_str());
        return true;
    }

    return false;
}

bool CBP_Book_Consultation(int id_Consultation, int id_Patient, char* raison)
{
    char requette[100];

    MYSQL *connexion = mysql_init(NULL);
    if (!mysql_real_connect(connexion, "localhost", "Student", "PassStudent1_", "PourStudent", 0, NULL, 0)) {
        fprintf(stderr, "mysql_real_connect failed: %s\n", mysql_error(connexion));
        mysql_close(connexion);
        return false;
    }

    /* START TRANSACTION */
    if (mysql_query(connexion, "START TRANSACTION")) {
        fprintf(stderr, "START TRANSACTION failed: %s\n", mysql_error(connexion));
        mysql_close(connexion);
        return false;
    }

    /* LOCK the row with SELECT ... FOR UPDATE */
    sprintf(requette, "SELECT patient_id FROM consultations WHERE id = %d FOR UPDATE", id_Consultation);
    if (mysql_query(connexion, requette)) {
        fprintf(stderr, "SELECT FOR UPDATE failed: %s\n", mysql_error(connexion));
        mysql_query(connexion, "ROLLBACK");
        mysql_close(connexion);
        return false;
    }

    MYSQL_RES *res = mysql_store_result(connexion);
    if (!res && mysql_field_count(connexion) != 0) {
        fprintf(stderr, "mysql_store_result failed: %s\n", mysql_error(connexion));
        mysql_query(connexion, "ROLLBACK");
        mysql_close(connexion);
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row && row[0] != NULL) {
        /* déjà réservé */
        mysql_free_result(res);
        mysql_query(connexion, "ROLLBACK");
        mysql_close(connexion);
        return false;
    }
    mysql_free_result(res);


    /*UPDATE avec WHERE patient_id IS NULL */
    sprintf(requette,
             "UPDATE consultations SET patient_id = %d, reason = '%s' WHERE id = %d AND patient_id IS NULL",
             id_Patient, raison, id_Consultation);

    if (mysql_query(connexion, requette)) {
        fprintf(stderr, "UPDATE failed: %s\n", mysql_error(connexion));
        mysql_query(connexion, "ROLLBACK");
        mysql_close(connexion);
        return false;
    }

    /* vérifier affected_rows */
    my_ulonglong affected = mysql_affected_rows(connexion);
    if (affected != 1) {
        /* pas réussi à réserver (concurrence ou id inexistant) */
        mysql_query(connexion, "ROLLBACK");
        mysql_close(connexion);
        return false;
    }

    mysql_close(connexion);
    return true;
}


//***** Gestion de l'état du protocole ****************************** 
int estPresent(int socket) 
{ 
    int indice = -1; 

    pthread_mutex_lock(&mutexClients); 

    for(int i=0 ; i<nbClients ; i++) 
    {
        if (clients[i] == socket) 
        { 
            indice  = i; 
            break; 
        } 
    }
    
    pthread_mutex_unlock(&mutexClients); 

    return indice; 
} 

void ajoute(int socket, CLIENT client) 
{ 
    pthread_mutex_lock(&mutexClients); 

    clients[nbClients] = socket;
    liste_clients[nbClients] = client; 
    nbClients++; 

    pthread_mutex_unlock(&mutexClients); 
} 

void retire(int socket) 
{ 
    int pos = estPresent(socket); 

    if (pos == -1) return; 
        pthread_mutex_lock(&mutexClients); 

    for (int i=pos ; i<=nbClients-2 ; i++)
    {
        clients[i] = clients[i+1];
        liste_clients[i] = liste_clients[i+1];
    }

    nbClients--; 
    pthread_mutex_unlock(&mutexClients); 
}

bool RechercheBD(char* requete, MYSQL_RES **resultat)
{
    MYSQL* connexion;

    // Connexion à la base de donnée
    connexion = mysql_init(NULL);
    if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
    {
        fprintf(stderr,"\n(CBP) Erreur de connexion à la base de données...\n");
        return false;  
    }
    //fprintf(stderr,"\n(CBP) Connexion a la BD reussi\n");
    
    // Exécution de la requête
    if (mysql_query(connexion, requete)) {
        fprintf(stderr, "Erreur lors de la requête SELECT : %s\n", mysql_error(connexion));
        mysql_close(connexion);
        return false;
    }

    // Récupération des résultats
    *resultat = mysql_store_result(connexion);
    if (resultat == NULL) {
        fprintf(stderr, "Erreur lors de la récupération des résultats : %s\n", mysql_error(connexion));
        mysql_close(connexion);
        return false;
    }

    return true;
}


//***** Fin prématurée ********************************************** 
void CBP_Close() 
{ 
    pthread_mutex_lock(&mutexClients); 

    for (int i=0 ; i<nbClients ; i++) 
        close(clients[i]); 

    pthread_mutex_unlock(&mutexClients); 
}