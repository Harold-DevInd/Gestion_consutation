#ifndef CBP_H 
#define CBP_H 
#define NB_MAX_CLIENTS 100 

#include <mysql.h>
#include <string> 

typedef struct {
  int  id;
  char name[30];
} SPECIALTY;

typedef struct {
  int  id;
  int  specialty_id;
  char last_name[30];
  char first_name[30];
} DOCTOR;

typedef struct {
  int  id;
  char last_name[30];
  char first_name[30];
  char birth_date[20];
} PATIENT;

typedef struct {
  int  id;
  int  doctor_id;
  int  patient_id;
  char date[20];
  char hour[10];
  char reason[100];
} CONSULTATION;

bool CBP(char* requete, char* reponse,int socket); 
bool CBP_Login(const char* last_name,const char* first_name, int numero, int existe); 
bool CBP_Get_Specialities(int* nbr_Specialite, char* liste_Specialite);
bool CBP_Get_Doctors(int* nbr_Doctor, char* liste_Doctor);
bool CBP_Search_Consultations(char *Speciality, char *nomDoctor, char* prenomDoctor, char* date_Debut, char* date_Fin, int* nbr_Consultation, char* liste_Consultation);
bool CBP_Book_Consultation(int id_Consultation, int id_Patient, char* raison);

bool RechercheBD(char* requete, MYSQL_RES **resultat);
void CBP_Close(); 

#endif 