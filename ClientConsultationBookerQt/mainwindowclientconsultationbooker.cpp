#include "mainwindowclientconsultationbooker.h"
#include "ui_mainwindowclientconsultationbooker.h"
#include <QInputDialog>
#include <QMessageBox>
#include <iostream>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <signal.h> 
#include <pthread.h> 
#include <ctype.h>
#include "../protocol/TCP.h" 
#include "../protocol/CBP.h" 
using namespace std;

int serverPort;


MainWindowClientConsultationBooker::MainWindowClientConsultationBooker(char *serverIp, int serverPort, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClientConsultationBooker)
{
    // stocke les valeurs dans des membres
    strcpy(this->m_serverIp, serverIp);
    this->m_serverPort = serverPort;

    ui->setupUi(this);
    logoutOk();

    // Connexion sur le serveur  
    if ((sClient = ClientSocket(m_serverIp, m_serverPort)) == -1) 
    { 
        perror("Erreur de ClientSocket"); 
        exit(1); 
    } 
    printf("Connecte sur le serveur.\n"); 

    // Configuration de la table des employes (Personnel Garage)
    ui->tableWidgetConsultations->setColumnCount(5);
    ui->tableWidgetConsultations->setRowCount(0);
    QStringList labelsTableConsultations;
    labelsTableConsultations << "Id" << "Spécialité" << "Médecin" << "Date" << "Heure";
    ui->tableWidgetConsultations->setHorizontalHeaderLabels(labelsTableConsultations);
    ui->tableWidgetConsultations->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetConsultations->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetConsultations->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidgetConsultations->horizontalHeader()->setVisible(true);
    ui->tableWidgetConsultations->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetConsultations->verticalHeader()->setVisible(false);
    ui->tableWidgetConsultations->horizontalHeader()->setStyleSheet("background-color: lightyellow");
    int columnWidths[] = {40, 150, 200, 150, 100};
    for (int col = 0; col < 5; ++col)
        ui->tableWidgetConsultations->setColumnWidth(col, columnWidths[col]);

    // Exemples d'utilisation (à supprimer)
    /*this->addTupleTableConsultations(1,"Neurologie","Martin Claire","2025-10-01", "09:00");
    this->addTupleTableConsultations(2,"Cardiologie","Lemoine Bernard","2025-10-06", "10:15");
    this->addTupleTableConsultations(3,"Dermatologie","Maboul Paul","2025-10-23", "14:30");

    //this->addComboBoxSpecialties("--- TOUTES ---");
    this->addComboBoxSpecialties("Dermatologie");
    this->addComboBoxSpecialties("Cardiologie");

    //this->addComboBoxDoctors("--- TOUS ---");
    this->addComboBoxDoctors("Martin Claire");
    this->addComboBoxDoctors("Maboul Paul");*/
}

MainWindowClientConsultationBooker::~MainWindowClientConsultationBooker()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table des livres encodés (ne pas modifier) ////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowClientConsultationBooker::addTupleTableConsultations(int id,
                                                                    string specialty,
                                                                    string doctor,
                                                                    string date,
                                                                    string hour)
{
    int nb = ui->tableWidgetConsultations->rowCount();
    nb++;
    ui->tableWidgetConsultations->setRowCount(nb);
    ui->tableWidgetConsultations->setRowHeight(nb-1,10);

    // id
    QTableWidgetItem *item = new QTableWidgetItem;
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(QString::number(id));
    ui->tableWidgetConsultations->setItem(nb-1,0,item);

    // specialty
    item = new QTableWidgetItem;
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(QString::fromStdString(specialty));
    ui->tableWidgetConsultations->setItem(nb-1,1,item);

    // doctor
    item = new QTableWidgetItem;
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(QString::fromStdString(doctor));
    ui->tableWidgetConsultations->setItem(nb-1,2,item);

    // date
    item = new QTableWidgetItem;
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(QString::fromStdString(date));
    ui->tableWidgetConsultations->setItem(nb-1,3,item);

    // hour
    item = new QTableWidgetItem;
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(QString::fromStdString(hour));
    ui->tableWidgetConsultations->setItem(nb-1,4,item);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowClientConsultationBooker::clearTableConsultations() {
    ui->tableWidgetConsultations->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MainWindowClientConsultationBooker::getSelectionIndexTableConsultations() const
{
    QModelIndexList list = ui->tableWidgetConsultations->selectionModel()->selectedRows();
    if (list.size() == 0) return -1;
    QModelIndex index = list.at(0);
    int ind = index.row();
    return ind;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles des comboboxes (ne pas modifier) //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowClientConsultationBooker::addComboBoxSpecialties(string specialty) {
    ui->comboBoxSpecialties->addItem(QString::fromStdString(specialty));
}

string MainWindowClientConsultationBooker::getSelectionSpecialty() const {
    return ui->comboBoxSpecialties->currentText().toStdString();
}

void MainWindowClientConsultationBooker::clearComboBoxSpecialties() {
    ui->comboBoxSpecialties->clear();
    this->addComboBoxSpecialties("--- TOUTES ---");
}

void MainWindowClientConsultationBooker::addComboBoxDoctors(string doctor) {
    ui->comboBoxDoctors->addItem(QString::fromStdString(doctor));
}

string MainWindowClientConsultationBooker::getSelectionDoctor() const {
    return ui->comboBoxDoctors->currentText().toStdString();
}

void MainWindowClientConsultationBooker::clearComboBoxDoctors() {
    ui->comboBoxDoctors->clear();
    this->addComboBoxDoctors("--- TOUS ---");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonction utiles de la fenêtre (ne pas modifier) ////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
string MainWindowClientConsultationBooker::getLastName() const {
    return ui->lineEditLastName->text().toStdString();
}

string MainWindowClientConsultationBooker::getFirstName() const {
    return ui->lineEditFirstName->text().toStdString();
}

int MainWindowClientConsultationBooker::getPatientId() const {
    return ui->spinBoxId->value();
}

void MainWindowClientConsultationBooker::setLastName(string value) {
    ui->lineEditLastName->setText(QString::fromStdString(value));
}

string MainWindowClientConsultationBooker::getStartDate() const {
    return ui->dateEditStartDate->date().toString("yyyy-MM-dd").toStdString();
}

string MainWindowClientConsultationBooker::getEndDate() const {
    return ui->dateEditEndDate->date().toString("yyyy-MM-dd").toStdString();
}

void MainWindowClientConsultationBooker::setFirstName(string value) {
    ui->lineEditFirstName->setText(QString::fromStdString(value));
}

void MainWindowClientConsultationBooker::setPatientId(int value) {
    if (value > 0) ui->spinBoxId->setValue(value);
}

bool MainWindowClientConsultationBooker::isNewPatientSelected() const {
    return ui->checkBoxNewPatient->isChecked();
}

void MainWindowClientConsultationBooker::setNewPatientChecked(bool state) {
    ui->checkBoxNewPatient->setChecked(state);
}

void MainWindowClientConsultationBooker::setStartDate(string date) {
    QDate qdate = QDate::fromString(QString::fromStdString(date), "yyyy-MM-dd");
    if (qdate.isValid()) ui->dateEditStartDate->setDate(qdate);
}

void MainWindowClientConsultationBooker::setEndDate(string date) {
    QDate qdate = QDate::fromString(QString::fromStdString(date), "yyyy-MM-dd");
    if (qdate.isValid()) ui->dateEditEndDate->setDate(qdate);
}

void MainWindowClientConsultationBooker::loginOk() {
    ui->lineEditLastName->setReadOnly(true);
    ui->lineEditFirstName->setReadOnly(true);
    ui->spinBoxId->setReadOnly(true);
    ui->checkBoxNewPatient->setEnabled(false);
    ui->pushButtonLogout->setEnabled(true);
    ui->pushButtonLogin->setEnabled(false);
    ui->pushButtonRechercher->setEnabled(true);
    ui->pushButtonReserver->setEnabled(true);
}

void MainWindowClientConsultationBooker::logoutOk() {
    ui->lineEditLastName->setReadOnly(false);
    setLastName("");
    ui->lineEditFirstName->setReadOnly(false);
    setFirstName("");
    ui->spinBoxId->setReadOnly(false);
    setPatientId(1);
    ui->checkBoxNewPatient->setEnabled(true);
    setNewPatientChecked(false);
    ui->pushButtonLogout->setEnabled(false);
    ui->pushButtonLogin->setEnabled(true);
    ui->pushButtonRechercher->setEnabled(false);
    ui->pushButtonReserver->setEnabled(false);
    setStartDate("2025-09-15");
    setEndDate("2025-12-31");
    clearComboBoxDoctors();
    clearComboBoxSpecialties();
    clearTableConsultations();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue (ne pas modifier) ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowClientConsultationBooker::dialogMessage(const string& title,const string& message) {
   QMessageBox::information(this,QString::fromStdString(title),QString::fromStdString(message));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowClientConsultationBooker::dialogError(const string& title,const string& message) {
   QMessageBox::critical(this,QString::fromStdString(title),QString::fromStdString(message));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
string MainWindowClientConsultationBooker::dialogInputText(const string& title,const string& question) {
    return QInputDialog::getText(this,QString::fromStdString(title),QString::fromStdString(question)).toStdString();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MainWindowClientConsultationBooker::dialogInputInt(const string& title,const string& question) {
    return QInputDialog::getInt(this,QString::fromStdString(title),QString::fromStdString(question));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions gestion des boutons (TO DO) //////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowClientConsultationBooker::on_pushButtonLogin_clicked()
{
    string lastName = this->getLastName();
    string firstName = this->getFirstName();
    int patientId = this->getPatientId();
    bool newPatient = this->isNewPatientSelected();

    cout << "lastName = " << lastName << endl;
    cout << "FirstName = " << firstName << endl;
    cout << "patientId = " << patientId << endl;
    cout << "newPatient = " << newPatient << endl;

    if((lastName.empty()) || (firstName.empty()))
        dialogError("Erreur de Login", "Champ nom ou prenom vide");
    else
    {
        char requete[200],reponse[200];

        // ***** Construction de la requete *********************
        if(newPatient)
            sprintf(requete,"LOGIN#%s#%s#%d#1#",lastName.c_str(),firstName.c_str(), patientId); 
        else
            sprintf(requete,"LOGIN#%s#%s#%d#-1#",lastName.c_str(),firstName.c_str(), patientId); 

        // ***** Envoi requete + réception réponse ************** 
        Echange(requete,reponse); 

        // ***** Parsing de la réponse ************************** 
        char *ptr = strtok(reponse,"#"); // entête = LOGIN (normalement...) 
        ptr = strtok(NULL,"#"); // statut = ok ou ko 

        if (strcmp(ptr,"ok") == 0)
        {
            printf("Login OK.\n"); 
            dialogMessage("Success", "Login reussi !");
            getSpecialites();
            getDoctor();
            loginOk();
        } 
        else 
        { 
            ptr = strtok(NULL,"#"); // raison du ko 
            printf("Erreur de login: %s\n",ptr); 
            dialogError("Erreur", ptr);
        } 
    }
}

void MainWindowClientConsultationBooker::on_pushButtonLogout_clicked()
{
    char requete[200],reponse[200];

    // ***** Construction de la requete *********************
    sprintf(requete,"LOGOUT"); 

    // ***** Envoi requete + réception réponse ************** 
    Echange(requete,reponse); 

    // ***** Parsing de la réponse ************************** 
    char *ptr = strtok(reponse,"#");
    ptr = strtok(NULL,"#"); // statut = ok ou ko 

    if (strcmp(ptr,"ok") == 0)
    {
        dialogMessage("Success", "Logout reussi !");
        logoutOk();
    } 
    else 
    { 
        ptr = strtok(NULL,"#"); // raison du ko 
        printf("Erreur de logout: %s\n",ptr); 
        dialogError("Erreur", ptr);
    } 
}

void MainWindowClientConsultationBooker::on_pushButtonRechercher_clicked()
{
    char requete[200],reponse[2048];
    int id, position;
    string date, heure;

    string specialty = this->getSelectionSpecialty();
    string doctor = this->getSelectionDoctor();
    string startDate = this->getStartDate();
    string endDate = this->getEndDate();

    cout << "specialty = " << specialty << endl;
    cout << "doctor = " << doctor << endl;
    cout << "startDate = " << startDate << endl;
    cout << "endDate = " << endDate << endl;

    if(startDate > endDate)
        dialogError("Erreur", "La date de debut doit etre inferieur à la date de fin");
    else
    {
        clearTableConsultations();
        videTableauConsultation();
        position = doctor.find(" ");
        string nomdoc = doctor.substr(0, position);
        string prenomdoc = doctor.substr(position + 1); 

        // ***** Construction de la requete *********************
        if((specialty == "--- TOUTES ---") && (doctor == "--- TOUS ---"))
            sprintf(requete,"SEARCH_CONS#0#0#0#%s#%s#", startDate.c_str(), endDate.c_str());
        else if((specialty != "--- TOUTES ---") && (doctor == "--- TOUS ---"))
            sprintf(requete,"SEARCH_CONS#%s#0#0#%s#%s#", specialty.c_str(), startDate.c_str(), endDate.c_str());
        else if((specialty == "--- TOUTES ---") && (doctor != "--- TOUS ---"))
            sprintf(requete,"SEARCH_CONS#0#%s#%s#%s#%s#", nomdoc.c_str(), prenomdoc.c_str(), startDate.c_str(), endDate.c_str());
        else
            sprintf(requete,"SEARCH_CONS#%s#%s#%s#%s#%s#", specialty.c_str(), nomdoc.c_str(), prenomdoc.c_str(), startDate.c_str(), endDate.c_str()); 

        printf("\n*****%s****\n", requete);

        // ***** Envoi requete + réception réponse ************** 
        Echange(requete,reponse); 

        // ***** Parsing de la réponse ************************** 
        char *ptr = strtok(reponse,"#");
        ptr = strtok(NULL,"#"); // statut = ok ou ko 

        if (strcmp(ptr,"ok") == 0)
        {

            printf("\nLecture des consultations reussi\n");
            nbrConsultation = atoi(strtok(NULL,"|"));     //recuperation du nombre de consultation envoye

            for(int i = 0; i < nbrConsultation; i++)
            {
                id = atoi(strtok(NULL,"#"));
                specialty = string(strtok(NULL,"#"));
                doctor = string(strtok(NULL,"#"));
                date = string(strtok(NULL,"#"));
                heure = string(strtok(NULL,"#"));
                addTupleTableConsultations(id, specialty, doctor, date, heure);
                listeConsultation[i] = id;
            }
        } 
        else 
        { 
            ptr = strtok(NULL,"#"); // raison du ko 
            dialogError("Erreur", ptr);
        }
    }

    
}

void MainWindowClientConsultationBooker::on_pushButtonReserver_clicked()
{
    char requete[200],reponse[2048];
    int idConsul;
    int selectedTow = this->getSelectionIndexTableConsultations();

    if(selectedTow == -1)
        dialogError("Erreur", "Veuillez choisir une reservation");
    else if(nbrConsultation == 0)
        dialogMessage("Erreur", "Veuillez effectuer une recherche avant !");
    else
    {
        string raison;
        cout << "selectedRow = " << selectedTow << endl;

        // ***** Construction de la requete *********************
        raison = dialogInputText("Revervation", "Veuillez entrer les raison de votre reservation");
        idConsul = listeConsultation[selectedTow];

        if(raison.empty())
            dialogError("Erreur", "Aucune raison saisie");
        else
        {
            sprintf(requete,"BOOK_CONS#%d#%d#%s#", idConsul, getPatientId(), raison.c_str()); 

            // ***** Envoi requete + réception réponse ************** 
            Echange(requete,reponse); 

            // ***** Parsing de la réponse ************************** 
            char *ptr = strtok(reponse,"#");
            ptr = strtok(NULL,"#"); // statut = ok ou ko 

            if (strcmp(ptr,"ok") == 0)
            {
                dialogMessage("Success", "Reservation reussi !");
                on_pushButtonRechercher_clicked();
            } 
            else 
            { 
                ptr = strtok(NULL,"#"); // raison du ko 
                dialogError("Erreur", ptr);
            }
        }
        
    }
    
}

//***** Echange de données entre client et serveur ****************** 
void MainWindowClientConsultationBooker::Echange(char* requete, char* reponse) 
{ 
  int nbEcrits, nbLus; 
 
  // ***** Envoi de la requete **************************** 
  if ((nbEcrits = Send(sClient,requete,strlen(requete))) == -1) 
  { 
    perror("Erreur de Send");
    exit(1); 
  } 
 
  // ***** Attente de la reponse ************************** 
  if ((nbLus = Receive(sClient,reponse)) < 0) 
  { 
    perror("Erreur de Receive"); 
    exit(1); 
  } 
 
  if (nbLus == 0) 
  { 
    printf("Serveur arrete, pas de reponse reçue...\n"); 
    exit(1); 
  } 
  reponse[nbLus] = 0; 
}

void MainWindowClientConsultationBooker::getSpecialites()
{
    char requete[200],reponse[2048];
    int count;

    // ***** Construction de la requete *********************
    sprintf(requete,"GET_SPEC");

    // ***** Envoi requete + réception réponse ************** 
    Echange(requete,reponse); 

    // ***** Parsing de la réponse ************************** 
    char *ptr = strtok(reponse,"#");
    ptr = strtok(NULL,"|"); // statut = ok ou ko 

    if (strcmp(ptr,"ok") == 0)
    {

        printf("\nLecture des specialites reussi\n");
        count = atoi(strtok(NULL,"|"));     //recuperation du nombre de specialite envoye

        for(int i = 0; i < count; i++)
        {
            addComboBoxSpecialties(strtok(NULL, "#"));
        }
    } 
    else 
    { 
        ptr = strtok(NULL,"#"); // raison du ko 
        dialogError("Erreur", ptr);
    } 
}

void MainWindowClientConsultationBooker::getDoctor()
{
    char requete[200],reponse[2048];
    int count;

    // ***** Construction de la requete *********************
    sprintf(requete,"GET_DOC");

    // ***** Envoi requete + réception réponse ************** 
    Echange(requete,reponse); 

    // ***** Parsing de la réponse ************************** 
    char *ptr = strtok(reponse,"#");
    ptr = strtok(NULL,"#"); // statut = ok ou ko 

    if (strcmp(ptr,"ok") == 0)
    {

        printf("\nLecture des docteurs reussi\n");
        count = atoi(strtok(NULL,"|"));     //recuperation du nombre de docteur envoye

        for(int i = 0; i < count; i++)
        {
            addComboBoxDoctors(strtok(NULL, "#"));
        }
    } 
    else 
    { 
        ptr = strtok(NULL,"#"); // raison du ko 
        dialogError("Erreur", ptr);
    }
}

void MainWindowClientConsultationBooker::videTableauConsultation()
{
    for(int i = 0; i < 20; i++)
        listeConsultation[i] = -1;
}