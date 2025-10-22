#include "mainwindowclientconsultationbooker.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Valeurs par défaut
    char serverIp[50] = "0.0.0.0";
    int serverPort = 50000;

    // Si l'utilisateur passe IP et port sur la ligne de commande :
    // usage: ./Client <ip> <port>
    if (argc >= 2) {
        strcpy(serverIp, argv[1]);
    }
    if (argc >= 3) {
        serverPort = atoi(argv[2]);
    }

    MainWindowClientConsultationBooker w(serverIp, serverPort);
    w.show();
    return a.exec();
}
