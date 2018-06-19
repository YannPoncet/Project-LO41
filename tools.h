#ifndef TOOLS_H
#define TOOLS_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>

#include "main.h"

#define TIME_TO_CHANGE_FLOOR 1

/** Le fichier tools sert à rassembler toutes les fonctions utiles qui
  * encombreraient le main autrement.
  */


/* Fonctions pour ascenseurs */

//cette fonction permet de descendre ou monter l'ascenseur d'un étage (vers le haut ou le bas)
//elle simule également le temps de mouvement
/*void changeFloor(elevator* elevator, int i);*/


#endif
