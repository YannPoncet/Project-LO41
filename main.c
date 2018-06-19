#include "main.h"


//Threads
pthread_t thread_elevator[NB_ELEVATOR]; //Tableau de Thread pour les ascenseurs
pthread_t thread_person[NB_MAX_PERSONS];
pthread_t thread_terminal;
pthread_t thread_dieu;


//Main function
int main(int argc, char const *argv[]) {
  srand(time(NULL));

  printf("\nMain => first!!!\n");

  //Creation du thread borne
  pthread_create(&thread_terminal,NULL,threadTerminal,(void *)0);

  //Creation du thread dieu
  pthread_create(&thread_dieu,NULL,threadDieu,(void *)0);

  //Creation des threads ascenseur
  for(int i=0; i<NB_ELEVATOR; i++) {
    pthread_create(&thread_elevator[i], NULL, threadElevator ,(void *) i );
  }





  //Attente de la fin des threads ascenseurs
  for(int i=0; i<NB_ELEVATOR; i++) {
		pthread_join(thread_elevator[i], NULL);
	}

  //Attente de la fin du thread borne
  pthread_join(thread_terminal,NULL);
  return 0;

  //Attente de la fin du thread dieu
  pthread_join(thread_dieu,NULL);
  return 0;
}

void * threadElevator(void *arg) {

  int id = *(int*) arg;
  elevator* elevator = &elevatorList[id];
  elevator->id = id;
  elevator->goesUp = false;
  elevator->goesDown = false;
  elevator->currentFloor = 0;
  elevator->destinationFloor = 0;
  elevator->state = 0;
  elevator->nbPersons = 0;
  for(int i=0; i<MAX_CAPACITY; i++){
    elevator->destinationList[i]=-1;
  }

  while(1){
    printf("threadElevator => je suis l'ascenseur\n");
    if(elevator->goesUp){
      changeFloor(elevator,1); //l'ascenseur monte
      if(checkIfStop(elevator->destinationList, elevator->currentFloor)){ //on vérifie si on doit s'arrêter au nouvel étage
        for(int i=0; i<NB_MAX_PERSONS; i++){
          if(personList[i].elevatorLink == elevator->id){
              //TODO reveille la personne
          }
        }
      }
    } else if(elevator->goesDown){
      changeFloor(elevator, -1); //l'ascenseur monte
      if(checkIfStop(elevator->destinationList, elevator->currentFloor)){ //on vérifie si on doit s'arrêter au nouvel étage
        for(int i=0; i<NB_MAX_PERSONS; i++){
          if(personList[i].elevatorLink == elevator->id){
              //TODO reveille la personne
          }
        }
      }
    }

    /*goesUp = false;
    goesDown = false;

    if(!goesUp && !goesDown){
      if(currentFloor - firstmecquimonte.destination > 0){
        goesUp = true;
      } else {
        goesDown = true;
      }
      destinationFloor = firstmecquimonte.destination;
    }*/

    /*L'ascenseur rejoint les étages dans l'ordre de destinationList
    * Cette liste sera triée dans l'ordre croissant si l'ascenseur monte
    * et dans l'ordre decroissant lorsqu'il descend
    */
  }
}


void* threadPerson(void *arg){
  while(1){
    printf("threadPerson: je suis une personne\n");
    sleep(1);
  }
}


void* threadTerminal(void *arg){
  while(1){
    printf("threadTerminal => je suis la borneuh \n");
    sleep(1);
  }
}

//Dieu crée aléatoirement des personnes qui rentrent ou sortent de l'immeuble, pour la simulation et pour éviter d'avoir à le faire manuellement
void* threadDieu(void *arg){
  while(1){
    printf("threadDieu => hey folks this is Rhykker \n");
    int r = rand()%100; //random between 0 and 99
    if(r<50) { //dieu a une chance sur 2 de créer une personne
      //Creation des threads person
      for(int i=0; i<NB_PERSONS_BASE; i++) {
        pthread_create(&thread_person[i], NULL, threadPerson ,(void *) i );
      }
    }
    sleep(1);
  }
}











/* Fonctions utilisées pour les ascenseurs */

void changeFloor(elevator* elevator, int i){
  elevator->currentFloor +=i;
  sleep(TIME_BETWEEN_FLOORS);
}

int updateDestinationFloor(int destinationList[], bool goesUp, bool goesDown, int nbPersons){
  if(goesUp){
    return destinationList[nbPersons-1];
  }

  if(goesDown){
    return destinationList[0];
  }

  return -1;
}

bool checkIfStop(int destinationList[], int currentFloor){
  for(int i=0; i<MAX_CAPACITY; i++){
    if(destinationList[i] == currentFloor){
      return true;
    }
  }
  return false;
}

void deleteEmptyBox(int destinationList[]){
  //enlever la case vide (tout mettre à gauche)
  bool isFinished = false;
  int i=0;
  while(i<MAX_CAPACITY && !isFinished){
    if(destinationList[i] == -1){
      int j = i+1;
      while(j<MAX_CAPACITY && !isFinished){
        if(destinationList[j]>-1){
          destinationList[j-1] = destinationList[j];
        } else {
          isFinished = true;
        }
        if(j==MAX_CAPACITY-1){
          isFinished = true;
        }
        j++;
      }
    }
    i++;
  }
}

void insertFloor(int destinationList[], int toInsert, int nbPersons) {
  bool isFinished = false;
  int i = 0;
  while(i<MAX_CAPACITY && !isFinished) {
    if(destinationList[i]==-1){
      destinationList[i] = toInsert;
      isFinished = true;
    } else if(destinationList[i]==toInsert) {
      isFinished = true;
    } else if(destinationList[i]>toInsert) {
      for(int j=nbPersons; j>i; j--){
        destinationList[j] = destinationList[j-1];
      }
      destinationList[i] = toInsert;
      isFinished = true;
    }
  }
}

//inutile mais j'ai passé du temps dessus donc je ne l'enlève pas
/*
void updateList(int destinationList[], int currentFloor, bool goesUp){

  //enlever les cases vides (tout mettre à gauche)
  bool hasReachedEnd = false;
  bool isFinished = false;
  //on regarde chaque case, on regarde celles qui sont vides
  for(int i=0; i<MAX_CAPACITY; i++){
    if(destinationList[i]==-1 && !hasReachedEnd){
      //on a trouvé une case vide et on a pas itéré sur toute la liste
      for(int j=i+1; j<MAX_CAPACITY; j++){
        if(!hasReachedEnd && !isFinished){
          //si on trouve une case non vide, on met la valeur à la suite
          if(destinationList[j]>-1){
            destinationList[i] = destinationList[j];
            i = j+1;
            isFinished = true;
          }
          if(j==MAX_CAPACITY-1){
            hasReachedEnd = true;
          }
        }
      }
    }
  }

  //trier la liste
  //si on va vers le haut
  if(goesUp){
    //on va vérifier si la liste est triée
    for(int i=0; i<MAX_CAPACITY; i++){
      for(int j=i+1; j<MAX_CAPACITY; j++0){
        if(destinationList[j]>currentFloor && destinationList[j]<destinationList[i]){
          int tmp = destinationList[j];
          for(int k=j; k>=i; k--){
            destinationList[k] = destinationList[k-1];
          }
          destinationList[i] = tmp;
        }
      }
    }
  }
}*/
