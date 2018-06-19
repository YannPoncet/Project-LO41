#include "main.h"
#include "tools.c"


int main(int argc, char const *argv[]) {
  printf("first lol");
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

    while(1){
      printf("je suis l'ascenseur\n");
      if(elevator->nbPersons==0){
        //TODO reveille les personnes à l'étage actuel
        sleep(1);
        //TODO attendre que qqn rentre dedans pour etre reveillé
      } else {
        if(elevator->goesUp){
          //changeFloor(elevator,1); //l'ascenseur monte
          /*TODO
          if(checkIfStop()){ //on vérifie si on doit s'arrêter au nouvel étage
              for(listePersonnes){
                if(personne.ascenseur == elevator->id){
                    //TODO reveille la personne
                  }
                }
              }
          }
        } */
        } else {
          //changeFloor(elevator, -1); //l'ascenseur monte
          /*TODO
          if(checkIfStop()){ //on vérifie si on doit s'arrêter au nouvel étage
              for(listePersonnes){
                if(personne.ascenseur == elevator->id){
                    //TODO reveille la personne
                  }
                }
              }
            }
          } */
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
      *
      *
      *
      *
      */
  }
}


void * threadPerson(void *arg){
  while(1){
    printf("je suis une personne\n");
    sleep(1);
  }
}


void * threadTerminal(void *arg){
  while(1){
    printf("je suis la borneuh \n");
    sleep(3);
  }
}
