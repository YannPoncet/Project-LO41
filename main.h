#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>

//Nombre d'ascenseurs
#define NB_ELEVATOR 3
//Nombre d'étages
#define NB_FLOOR 25
//Nombre max de résident
#define NB_MAX_RESIDENT 100
//Nombre max de visiteurs (=visiteurs + livreurs)
#define NB_MAX_VISITEUR 100

//Structure ascenseur
typedef struct{
	int id;
	int floor;
  int state; //0: idle, 1: stopped at a floor, 2: moving
  int capacity; //number of persons
  int max_capacity;
} elevator;

typedef struct{
  int nb_residents;
  char  residentName[NB_MAX_RESIDENT][50];
} application;
