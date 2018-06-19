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
#define MAX_CAPACITY 10
//Nombre d'étages
#define NB_FLOOR 25
//Nombre max de résident
#define NB_MAX_PERSONS 100
//le temps pour passer d'un étage à un autre
#define TIME_BETWEEN_FLOORS 1


//Nombre de personnes initiales utilisées pour simuler
#define NB_PERSONS_BASE 10


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
} person;

person personList[NB_MAX_PERSONS];

//Threads
void* threadElevator(void *arg);
void* threadPerson(void *arg);
void* threadTerminal(void *arg);
void* threadDieu(void *arg);

/** Le fichier tools sert à rassembler toutes les  qui
  * encombreraient le main autrement.
  */

/* fonctions utiles */
/* Fonctions pour ascenseurs */

/*cette fonction permet de descendre ou monter l'ascenseur d'un étage (vers le haut ou le bas)
*elle simule également le temps de mouvement
*/
void changeFloor(elevator* elevator, int i);

//actualise l'étage de destination en fonction du sens de l'ascenseur
int updateDestinationFloor(int destinationList[], bool goesUp, bool goesDown, int nbPersons);

//vérifie si on doit s'arrêter à l'étage actuel
bool checkIfStop(int destinationList[], int currentFloor);

//enleve la case vide du tableau
void deleteEmptyBox(int destinationList[]);

//cette fonction permet d'inserer un element la liste en remettant tous les éléments dans l'ordre
void insertFloor(int destinationList[], int toInsert, int nbPersons);

/*cette fonction permet de trier la liste en remettant tous les éléments dans l'ordre
* si on monte et qu'un étage de la liste de destination est inferieur à l'étage actuel
* on le met en dernier dans la liste, de même si on descend et qu'un étage est supérieur à l'étage actuel
*/
void updateList(int destinationList[], int currentFloor, bool goesUp);

#endif
