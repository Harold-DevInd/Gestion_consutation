#ifndef CBP_H 
#define CBP_H 
#define NB_MAX_CLIENTS 100 

#include <mysql.h>
#include <string> 

typedef struct {
  int numero_patient;
  char last_name[30];
  char first_name[30];
  char  ip_client[30];
} CLIENT;


bool CBP(char* requete, char* reponse,int socket); 
bool CBP_Login(const char* last_name,const char* first_name, int numero, int* existe); 
bool CBP_Get_Specialities(int* nbr_Specialite, char* liste_Specialite);
bool CBP_Get_Doctors(int* nbr_Doctor, char* liste_Doctor);
bool CBP_Search_Consultations(char *Speciality, char *nomDoctor, char* prenomDoctor, char* date_Debut, char* date_Fin, int* nbr_Consultation, char* liste_Consultation);
bool CBP_Book_Consultation(int id_Consultation, int id_Patient, char* raison);
bool CBP_Liste_Clients(char* lc);

bool RechercheBD(char* requete, MYSQL_RES **resultat);
void CBP_Close(); 

#endif 