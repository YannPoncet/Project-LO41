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
#include <time.h>

//Nombre d'ascenseurs
#define NB_ELEVATOR 3
//Nombre de personnes max dans l'ascenseurs
#define MAX_CAPACITY 2
//Nombre d'étages
#define NB_FLOOR 10
//Nombre max de résident
#define NB_MAX_PERSONS 7
//le temps pour passer d'un étage à un autre
#define TIME_BETWEEN_FLOORS 1

#define ANSI_COLOR_RED     "\x1b[91m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

typedef struct{
	int id; //numero de l'ascenseur
  bool goesUp;
  bool goesDown;
	int currentFloor; //etage actuel
  int destinationFloor; //etage de destination
	int destinationList[NB_FLOOR]; //a list of destinations
  int state; //0: attente, 1: arrêté à un etage, 2: en mouvement
  int currentNbPersons; //nombre de personnes
	int previsonalNbPersons; //nombre de personnes qui seront dedans si toutes les personnes qui attendent y rentrent
} Elevator;

Elevator elevatorList[NB_ELEVATOR];

//Structure personne
typedef struct{
	int id; //id de la personne
  int type; //0 pour résident, 1 pour visiteur
  bool isAuthorized; //passe à vrai si une personne est autorisée à accéder aux ascenseurs
  int currentFloor; //etage actuel de la personne
	int wantedFloor; //etage ou veut aller la personne
  int elevatorLink; //sert à lier un ascenseur avec une personne
  bool isInAnElevator;
} Person;


int waitingList[NB_MAX_PERSONS];


Person personList[NB_MAX_PERSONS];



//Threads
void* threadElevator(void *arg);
void* threadPerson(void *arg);
void* threadTerminal(void *arg);
void* threadGod(void *arg);

//Conditions
pthread_cond_t cond_elevator_request_terminal[NB_ELEVATOR]; //l'ascenseur attend jusqu'à ce qu'il soit reveillé par le terminal
pthread_cond_t cond_elevator_request_person[NB_ELEVATOR][NB_MAX_PERSONS];
pthread_cond_t cond_person_request_terminal[NB_MAX_PERSONS];
pthread_cond_t cond_person_request_elevator_in[NB_MAX_PERSONS];
pthread_cond_t cond_person_request_elevator_out[NB_MAX_PERSONS];

//Mutex
pthread_mutex_t m_elevator[NB_ELEVATOR];
pthread_mutex_t m_person[NB_MAX_PERSONS];
pthread_mutex_t m_waitingList;


/** Le fichier tools sert à rassembler toutes les  qui
  * encombreraient le main autrement.
  */

/* fonctions utiles */

void addPersonToWaitingList(int id);

Person* findPerson(int id);

/* Fonctions pour ascenseurs */

void printWaitingList();
void printDList(int destinationList[]);

/*cette fonction permet de descendre ou monter l'ascenseur d'un étage (vers le haut ou le bas)
*elle simule également le temps de mouvement
*/
void changeFloor(Elevator* elevator, int i);

void deleteDestination(int destinationList[], int currentFloor);

//actualise l'étage de destination en fonction du sens de l'ascenseur
int updateDestinationFloor(int destinationList[], int currentFloor, bool goesUp, bool goesDown);

//vérifie si on doit s'arrêter à l'étage actuel
bool checkIfStop(int destinationList[], int currentFloor);

//enlève la case vide en 1ere position
void deleteEmptyBoxWaitingList(int waitingList[]);

//enleve la case vide du tableau
void deleteEmptyBoxElevator(int destinationList[]);

//cette fonction permet d'inserer un element la liste en remettant tous les éléments dans l'ordre
void insertFloor(int destinationList[], int toInsert);

/*cette fonction permet de trier la liste en remettant tous les éléments dans l'ordre
* si on monte et qu'un étage de la liste de destination est inferieur à l'étage actuel
* on le met en dernier dans la liste, de même si on descend et qu'un étage est supérieur à l'étage actuel
*/
void updateList(int destinationList[], int currentFloor, bool goesUp);

#endif
