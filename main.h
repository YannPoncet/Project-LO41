/* Projet réalisé par Boris Broglé et Yann Poncet dans le cadre de l'UV LO41 à l'UTBM */

#ifndef MAIN_H
#define MAIN_H

/* Includes */
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

/* Paramètres globaux */
#define NB_ELEVATOR 2 //Nombre d'ascenseurs
#define MAX_CAPACITY 4 //Nombre de personnes max dans l'ascenseurs
#define NB_FLOOR 10 //Nombre d'étages
#define NB_MAX_PERSONS 5 //Nombre max de personnes (résidents + visiteurs)
#define TIME_BETWEEN_FLOORS 0.5 //le temps pour passer d'un étage à un autre


/* Définition des couleurs utilisées */
#define ANSI_COLOR_RED     "\x1b[91m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

//Structure ascenseur
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

//Listes globales

int waitingList[NB_MAX_PERSONS]; //liste d'attente d'accès à la borne

Person personList[NB_MAX_PERSONS]; //liste des personnes dans le batiment

Elevator elevatorList[NB_ELEVATOR]; // liste des ascenseurs du batiment


//Threads
void* threadGod(void *arg); //crée des personnes
void* threadPerson(void *arg); //se balade dans l'immeuble
void* threadTerminal(void *arg); //gère le lien entre les ascenseurs et les personnes
void* threadElevator(void *arg); //déplace les gens d'un étage à un autre

//Conditions
pthread_cond_t cond_elevator_request_terminal[NB_ELEVATOR]; //l'ascenseur attend jusqu'à ce qu'il soit reveillé par le terminal
pthread_cond_t cond_elevator_request_person[NB_ELEVATOR][NB_MAX_PERSONS];
pthread_cond_t cond_person_request_terminal[NB_MAX_PERSONS];
pthread_cond_t cond_person_request_elevator_in[NB_MAX_PERSONS];
pthread_cond_t cond_person_request_elevator_out[NB_MAX_PERSONS];

//Mutex
pthread_mutex_t m_elevator[NB_ELEVATOR]; //utilisé dans les cond_wait
pthread_mutex_t m_person[NB_MAX_PERSONS]; //utilisé dans les cond_wait
pthread_mutex_t m_waitingList; //empêcher les personnes d'accéder à la liste d'attente en même temps



/* Fonctions utiles */


/* Fonctions dans les personnes */

//ajoute un personne à la liste d'attente en fonction de son ID
void addPersonToWaitingList(int id);


/* Fonctions dans le terminal */

//enlève la case vide en 1ere position dans la liste d'attente
void deleteEmptyBoxWaitingList(int waitingList[]);

//permet de trouver une personne dans la liste de personnes grâce à son ID
Person* findPerson(int id);

//permet de trouver un ascenseur dans la liste d'ascenseurs grâce à son ID
Elevator* findElevator(int elevId);

//permet de savoir si un etage se trouve dans la liste des destinations de l'ascenseur
bool isInDList(int destinationList[], int toTestFloor);

//permet de choisir un ascenseur grâce à plusieurs conditions testées par ordre d'importance (c'est l'IA)
int chooseElevator(Person* pers);


/* Fonctions dans les ascenseurs */

/*cette fonction permet de descendre ou monter l'ascenseur d'un étage (vers le haut ou le bas)
*elle simule également le temps de mouvement*/
void changeFloor(Elevator* elevator, int i);

//permet d'enlever l'étage actuel de la liste de destinations de l'ascenseur
void deleteDestination(int destinationList[], int currentFloor);

//enleve la case vide du tableau de destinations d'un ascenseur, utilisée dans deleteDestination
void deleteEmptyBoxElevator(int destinationList[]);

//actualise l'étage de destination en fonction du sens de l'ascenseur (aussi utilisée dans le terminal)
int updateDestinationFloor(int destinationList[], int currentFloor, bool goesUp, bool goesDown);

//vérifie si on doit s'arrêter à l'étage actuel
bool checkIfStop(int destinationList[], int currentFloor);

/*cette fonction permet d'inserer un element la liste de destination
d'un ascenseur en remettant tous les éléments dans l'ordre, (aussi utilisée dans le terminal)*/
void insertFloor(int destinationList[], int toInsert);


/* Fonctions d'affichage */

void printWaitingList(); //affiche la liste d'attente
void printDList(int destinationList[]); //affiche la liste des destinations d'un ascenseur

#endif
