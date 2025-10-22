# Makefile pour Client Qt + MySQL + Serveur + CreationBD

QT_CFLAGS := $(shell pkg-config --cflags Qt5Widgets)
QT_LIBS   := $(shell pkg-config --libs Qt5Widgets)

all: Client CreationBD Serveur

# =========================
# Client Qt
# =========================
Client: ClientConsultationBookerQt/main.cpp \
      ClientConsultationBookerQt/mainwindowclientconsultationbooker.cpp \
      ClientConsultationBookerQt/moc_mainwindowclientconsultationbooker.cpp \
      protocol/CBP.cpp protocol/TCP.cpp
	g++ -Wall -std=c++17 -fPIC \
	    ClientConsultationBookerQt/main.cpp \
	    ClientConsultationBookerQt/mainwindowclientconsultationbooker.cpp \
	    ClientConsultationBookerQt/moc_mainwindowclientconsultationbooker.cpp \
	    protocol/CBP.cpp protocol/TCP.cpp \
	    -I/usr/include/mysql -L/usr/lib64/mysql $(QT_CFLAGS) $(QT_LIBS) \
	    -lmysqlclient -lpthread -o Client

# =========================
# Serveur
# =========================
Serveur: serveur/serveur.cpp protocol/CBP.cpp protocol/TCP.cpp protocol/TCP.h
	g++ -Wall -std=c++17 \
	    serveur/serveur.cpp protocol/CBP.cpp protocol/TCP.cpp \
	    -I/usr/include/mysql  -L/usr/lib64/mysql -lmysqlclient -lpthread -o Serveur

# =========================
# Creation de la base de données
# =========================
CreationBD: BD_Hospital/CreationBD.cpp
	g++ -Wall -std=c++17 \
	    BD_Hospital/CreationBD.cpp \
	    -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl \
	    -o CreationBD

# =========================
# Nettoyage
# =========================
clean:
	rm -f Client Serveur CreationBD