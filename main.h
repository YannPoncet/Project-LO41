#ifndef MAIN_H
#define MAIN_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>

#include "tools.h"

//Nombre d'ascenseurs
#define NB_ELEVATOR 3
//Nombre de personnes max dans l'ascenseurs
#define MAX_CAPACITY 10
//Nombre d'étages
#define NB_FLOOR 25
//Nombre max de résident
#define NB_MAX_RESIDENTS 100
//Nombre max de visiteurs (=visiteurs + livreurs)
#define NB_MAX_VISITORS 100


typedef struct{
	int id; //numero de l'ascenseur
  bool goesUp;
  bool goesDown;
	int currentFloor; //etage actuel
  int destinationFloor; //etage de destination
	int destinationList[MAX_CAPACITY]; //a list of destinations
  int state; //0: attente, 1: arrêté à un etage, 2: en mouvement
  int nbPersons; //nombre de personnes
} elevator;

elevator elevatorList[NB_ELEVATOR];

//Structure personne
typedef struct{
	int id; //id de la personne
  int type; //0 pour résident, 1 pour visiteur
  bool isAuthorized; //passe à vrai si un utilisateur est autorisé à accéder aux ascenseurs
  int currentFloor; //etage actuel du visiteur
  int elevatorLink; //sert à lier un ascenseur avec une personne
  bool isInAnElevator;
} visitor;



//Threads
void* threadElevator(void *arg);
void* threadVisitor(void *arg);
void* threadTerminal(void *arg);

#endif
