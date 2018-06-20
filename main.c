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

for(int i=0; i<NB_MAX_PERSONS; i++){ //initialisation de la liste d'attente de la borne
  	waitingList[i] = -1;
  }

 printf("oui");
  //Creation des threads ascenseur
  for(int i=0; i<NB_ELEVATOR; i++) {
    pthread_create(&thread_elevator[i], NULL, threadElevator ,(void *) i );
  }

  //Creation du thread borne
  pthread_create(&thread_terminal,NULL,threadTerminal,(void *)0);

  //Creation du thread dieu
  pthread_create(&thread_dieu,NULL,threadDieu,(void *)0);





  //Attente de la fin du thread dieu
  pthread_join(thread_dieu,NULL);

  //Attente de la fin du thread borne
  pthread_join(thread_terminal,NULL);

  //Attente de la fin des threads ascenseurs
  for(int i=0; i<NB_ELEVATOR; i++) {
		pthread_join(thread_elevator[i], NULL);
	}

  printf("This is the end\n");
  return 0;
}


void * threadElevator(void *arg) {
  int id = *((int*)(&arg));
  Elevator* elevator = &elevatorList[id];
  elevator->id = id;
  elevator->goesUp = false;
  elevator->goesDown = false;
  elevator->currentFloor = 0;
  elevator->destinationFloor = 0;
  elevator->state = 0;
  elevator->currentNbPersons = 0;
  elevator->previsonalNbPersons = 0;
  for(int i=0; i<NB_FLOOR; i++){
    elevator->destinationList[i]=-1;
  }

  printf("threadElevator => je suis l'ascenseur\n");
  while(1){
    //on attend que la borne nous dise de changer d'étage
    pthread_cond_wait(&cond_elevator_request_terminal[elevator->id], &m_elevator[elevator->id]);
    if(elevator->goesUp){
      changeFloor(elevator,1); //l'ascenseur monte
      if(checkIfStop(elevator->destinationList, elevator->currentFloor)){ //on vérifie si on doit s'arrêter au nouvel étage
        deleteDestination(elevator->destinationList, elevator->currentFloor); //on enlève cette destination de la liste
        elevator->destinationFloor=updateDestinationFloor(elevator->destinationList, elevator->currentFloor, elevator->goesUp, elevator->goesDown); //on met à jour l'étage de destination
        if(elevator->destinationFloor == -1){ //si il n'y a plus de destination, on arrête l'ascenseur
          elevator->goesUp = false;
          elevator->goesDown = false;
        } else if(elevator->destinationFloor < elevator->currentFloor){
          elevator->goesUp = false;
          elevator->goesDown = true;
        }
        for(int i=0; i<NB_MAX_PERSONS; i++){
          Person* p = &personList[i];
          if(p->elevatorLink == elevator->id){
            if(p->isInAnElevator) {
              p->elevatorLink = -1;
              p->currentFloor = elevator->currentFloor;
              p->wantedFloor = -1;
              p->isInAnElevator = false;
            } else {
              p->isInAnElevator = true;
              insertFloor(elevator->destinationList, p->wantedFloor); //on ajoute la destination du monsieur/madame/les2alafois à la liste des destinations
              elevator->destinationFloor=updateDestinationFloor(elevator->destinationList, elevator->currentFloor, elevator->goesUp, elevator->goesDown); //on met à jour l'étage de destination
            }
            //on reveille une personne est sortie ou rentrée
            pthread_cond_signal(&cond_person_request_elevator[p->id]);
          }
        }
      }
    } else if(elevator->goesDown){
      changeFloor(elevator, -1); //l'ascenseur monte
      if(checkIfStop(elevator->destinationList, elevator->currentFloor)){ //on vérifie si on doit s'arrêter au nouvel étage
        deleteDestination(elevator->destinationList, elevator->currentFloor); //on enlève cette destination de la liste
        elevator->destinationFloor=updateDestinationFloor(elevator->destinationList, elevator->currentFloor, elevator->goesUp, elevator->goesDown); //on met à jour l'étage de destination
        if(elevator->destinationFloor == -1){ //si il n'y a plus de destination, on arrête l'ascenseur
          elevator->goesUp = false;
          elevator->goesDown = false;
        } else if(elevator->destinationFloor > elevator->currentFloor){
          elevator->goesUp = true;
          elevator->goesDown = false;
        }
        for(int i=0; i<NB_MAX_PERSONS; i++){
          Person* p = &personList[i];
          if(p->elevatorLink == elevator->id){
            if(p->isInAnElevator) {
              p->elevatorLink = -1;
              p->currentFloor = elevator->currentFloor;
              p->wantedFloor = -1;
              p->isInAnElevator = false;
            } else {
              p->isInAnElevator = true;
              insertFloor(elevator->destinationList, p->wantedFloor); //on ajoute la destination du monsieur/madame/les2alafois à la liste des destinations
              elevator->destinationFloor=updateDestinationFloor(elevator->destinationList, elevator->currentFloor, elevator->goesUp, elevator->goesDown); //on met à jour l'étage de destination
            }

            //on reveille une personne est sortie ou rentree
            pthread_cond_signal(&cond_person_request_elevator[p->id]);
          }
        }
      }
    } else {
      for(int i=0; i<NB_MAX_PERSONS; i++){
        Person* p = &personList[i];
        if(p->elevatorLink == elevator->id){
          if(p->isInAnElevator) {
            p->elevatorLink = -1;
            p->currentFloor = elevator->currentFloor;
            p->wantedFloor = -1;
            p->isInAnElevator = false;
            printf("threadElevator => je suis a l'arret et la personne %d sort\n", p->id);
          } else {
            printf("threadElevator => je suis a l'arret et la personne %d entre\n", p->id);
            p->isInAnElevator = true;
            if(elevator->currentFloor - p->wantedFloor > 0){
              elevator->goesDown = true;
              printf("threadElevator => je vais descendre\n");
            } else if(elevator->currentFloor - p->wantedFloor < 0){
              elevator->goesUp = true;
              printf("threadElevator => je vais monter\n");
            }

            insertFloor(elevator->destinationList, p->wantedFloor); //on ajoute la destination du monsieur/madame/les2alafois à la liste des destinations
            elevator->destinationFloor=updateDestinationFloor(elevator->destinationList, elevator->currentFloor, elevator->goesUp, elevator->goesDown); //on met à jour l'étage de destination
            printf("threadElevator => ma destination est %d\n", elevator->destinationFloor);
          }
        }
        //on reveille une personne est sortie ou rentree
        pthread_cond_signal(&cond_person_request_elevator[p->id]);
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

void* threadTerminal(void *arg) {
  printf("threadTerminal => je suis la borneuh \n");
  while(1){
    if(waitingList[0]>-1) { //si il y a qqn dans la liste d'attente
      Person* pers = findPerson(waitingList[0]);
      int i=0;
      while(i<NB_ELEVATOR && pers->elevatorLink==-1) {
        Elevator* elev = &elevatorList[i];
        if(elev->previsonalNbPersons<MAX_CAPACITY) {

          if(elev->goesUp || elev->goesDown){ //si l'ascenseur se déplace vers le haut ou vers le bas
            insertFloor(elev->destinationList, pers->currentFloor);
            elev->destinationFloor=updateDestinationFloor(elev->destinationList, elev->currentFloor, elev->goesUp, elev->goesDown);
            pers->elevatorLink = elev->id;
          }
          else { //ascenseur à l'arret
            insertFloor(elev->destinationList, pers->currentFloor);
            elev->destinationFloor=updateDestinationFloor(elev->destinationList, elev->currentFloor, elev->goesUp, elev->goesDown);
            pers->elevatorLink = elev->id;

            if(elev->currentFloor < pers->currentFloor){
              elev->goesUp = true;
            } else if(elev->currentFloor > pers->currentFloor) {
              elev->goesDown = true;
            }
          }
          printf("threadTerminal => La personne %d allant à l'étage %d via ascenseur %d va être réveillée\n", pers->id, pers->wantedFloor, pers->elevatorLink);
          //on reveille la personne qui attend que le terminal lui associe un ascenseur
          waitingList[0] = -1;
          deleteEmptyBoxWaitingList(waitingList);
          pthread_cond_signal(&cond_person_request_terminal[pers->id]);


          /*if(elev->goesUp){
            if(elev->currentFloor<pers->currentFloor && elev->currentFloor<pers->wantedFloor) {
              //TODO reveille la personne pers, update att pers, ajoute etage à la liste de destinations de l'ascenseur, update la destinationFloor
            }
          } else if(elev->goesDown) {
            if(elev->currentFloor>pers->currentFloor && elev->currentFloor>pers->wantedFloor) {
              //TODO reveille la personne pers
            }
          } else {
            if(elev->currentFloor==pers->currentFloor) {
              //TODO reveille la personne pers
            } else if(elev->currentFloor<pers->currentFloor){
              elev->goesUp = true;
              //TODO tout le bordel (2 destinations ici)
            } else {
              elev->goesDown = true;
              //TODO tout le bordel (2 destinations ici)
            }
          }*/
        }
        i++;
      }
    }

    for(int i=0; i<NB_ELEVATOR; i++) {
      //reveille les ascenseurs (qui attendent à chaque etage)
      pthread_cond_signal(&cond_elevator_request_terminal[i]);
    }
    sleep(1);
  }
}

//Dieu crée aléatoirement des personnes qui rentrent ou sortent de l'immeuble, pour la simulation et pour éviter d'avoir à le faire manuellement
void* threadDieu(void *arg){
  int i = 0; //va suivre le nombre de personnnes créées par le dieu
  printf("threadDieu => hey folks this is Rhykker \n");
  while(i<NB_MAX_PERSONS){//pour que l'on ne mette pas plus de personnes que possible dans le tableau
    int r = rand()%100; //random between 0 and 99
    if(r<50) { //dieu a une chance sur 2 de créer une personne
      //Creation des threads person
      pthread_create(&thread_person[i], NULL, threadPerson ,(void *) i );
      i++;
    }
    sleep(1);
  }
  for(int i=0; i<NB_MAX_PERSONS; i++) {
    pthread_join(thread_person[i], NULL);
  }

  return NULL;
}


void* threadPerson(void *arg) {
  int id = *((int*)(&arg));
  Person* person = personList+id;
  person->id = id;
  person->type = 0;
  person->isAuthorized = false;
  person->currentFloor = 0;
  person->wantedFloor = rand()%(NB_FLOOR-1)+1; //Entre 1 et 24 inclus
  person->elevatorLink = -1;
  person->isInAnElevator = false;

  printf("threadPerson => dieu vient de me créer, je suis la personne %d, et je vais à l'étage %d\n",person->id, person->wantedFloor);
  while(1){
    if(person->wantedFloor > -1) { //si la personne veut aller quelque part
      printf("eLink: %d, isIn: %d\n", person->elevatorLink, person->isInAnElevator);
      if(person->elevatorLink == -1 && !person->isInAnElevator) { //si la personne n'est pas dans un ascenseur
        //puisque la personne est réveillée, on l'ajoute a la liste d'attente et on l'endort
        addPersonToWaitingList(person->id);
        //on attend que le terminal nous donne un ascenseur
        pthread_cond_wait(&cond_person_request_terminal[person->id], &m_person[person->id]);
      } else if(person->elevatorLink > -1 && !person->isInAnElevator){ //si la personne a été associée à un ascenseur, elle attend que l'ascenseur lui dise d'y rentrer
        //on attend que l'ascenseur nous dise de rentrer dedans de lui
        printf("J'attend l'ascenseur\n");
        pthread_cond_wait(&cond_person_request_elevator[person->id], &m_person[person->id]);
        printf("threadPerson => je rentre dans un ascenseur\n");
      } else if(person->elevatorLink > -1 && person->isInAnElevator) { //si la personne est dans l'ascenseur, elle attend que l'ascenseur lui dise de sortir
        //on attend que l'ascenseur nous dise de sortir de dedans de lui
        pthread_cond_wait(&cond_person_request_elevator[person->id], &m_person[person->id]);
      }
    } else {
      //TODO Il y aura ici une certaine chance que la personne veuille changer d'étage
    }
    sleep(1);
  }
}

void addPersonToWaitingList(int id) {
  int i = 0;
  bool inserted = false;
  while(i<NB_MAX_PERSONS && !inserted) {
    if(waitingList[i] == -1) {
      waitingList[i] = id;
      inserted = true;
    }
  }
}

Person* findPerson(int id){
  for(int i=0; i<NB_MAX_PERSONS; i++){
    if(personList[i].id == id)
      return &personList[i];
  }
}




/* Fonctions utilisées pour les ascenseurs */

void changeFloor(Elevator* elevator, int i){
  elevator->currentFloor +=i;
  printf("threadElevator => je %d d'étage, je vais à l'étage %d\n", i, elevator->currentFloor);
  sleep(TIME_BETWEEN_FLOORS);
}

void deleteDestination(int destinationList[], int currentFloor){
  for(int i=0; i<NB_FLOOR; i++){
    if(currentFloor == destinationList[i]){
      destinationList[i] = -1;
    }
  }
  deleteEmptyBoxElevator(destinationList);
}


int updateDestinationFloor(int destinationList[], int currentFloor, bool goesUp, bool goesDown){
  if(destinationList[0]>-1){ //si on a une destination
    bool lastOneFound = false;
    if(goesUp){ //si on monte la destination est l'étage le plus haut
      int i = 0;
      while(i<NB_FLOOR && !lastOneFound){
        i++;
        if(destinationList[i]==-1){
          lastOneFound = true;
        }
      }
      if(destinationList[i-1] > currentFloor){
        return destinationList[i-1];
      } else {
        return destinationList[0];
      }
    }

    if(goesDown){ //si on descend la destination est l'étage le plus bas
      if(destinationList[0] < currentFloor){
        return destinationList[0];
      } else {
        int i = 0;
        while(i<NB_FLOOR){
          i++;
          if(destinationList[i]==-1){
            return destinationList[i-1];
          }
        }
      }
    }
  }
  return -1;
}

bool checkIfStop(int destinationList[], int currentFloor){
  for(int i=0; i<NB_FLOOR; i++){
    if(destinationList[i] == currentFloor){
      return true;
    }
  }
  return false;
}

void deleteEmptyBoxWaitingList(int waitingList[]){
  int i=0;
  while(i<NB_MAX_PERSONS && waitingList[i]>-1){
    waitingList[i] = waitingList[i+1];
    i++;
  }
}

void deleteEmptyBoxElevator(int destinationList[]){
  //enlever la case vide (tout mettre à gauche)
  bool isFinished = false;
  int i=0;
  while(i<NB_FLOOR && !isFinished){
    if(destinationList[i] == -1){
      int j = i+1;
      while(j<NB_FLOOR && !isFinished){
        if(destinationList[j]>-1){
          destinationList[j-1] = destinationList[j];
        } else {
          isFinished = true;
        }
        if(j==NB_FLOOR-1){
          isFinished = true;
        }
        j++;
      }
    }
    i++;
  }
}

void insertFloor(int destinationList[], int toInsert) {
  bool isFinished = false;
  int i = 0;
  while(i<NB_FLOOR && !isFinished) {
    if(destinationList[i]==-1){
      destinationList[i] = toInsert;
      isFinished = true;
    } else if(destinationList[i]==toInsert) {
      isFinished = true;
    } else if(destinationList[i]>toInsert) {
      for(int j=NB_FLOOR-1; j>i; j--){
        destinationList[j] = destinationList[j-1];
      }
      destinationList[i] = toInsert;
      isFinished = true;
    }
    i++;
  }
}

//inutile mais j'ai passé du temps dessus donc je ne l'enlève pas
/*
void updateList(int destinationList[], int currentFloor, bool goesUp){

  //enlever les cases vides (tout mettre à gauche)
  bool hasReachedEnd = false;
  bool isFinished = false;
  //on regarde chaque case, on regarde celles qui sont vides
  for(int i=0; i<NB_FLOOR; i++){
    if(destinationList[i]==-1 && !hasReachedEnd){
      //on a trouvé une case vide et on a pas itéré sur toute la liste
      for(int j=i+1; j<NB_FLOOR; j++){
        if(!hasReachedEnd && !isFinished){
          //si on trouve une case non vide, on met la valeur à la suite
          if(destinationList[j]>-1){
            destinationList[i] = destinationList[j];
            i = j+1;
            isFinished = true;
          }
          if(j==NB_FLOOR-1){
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
    for(int i=0; i<NB_FLOOR; i++){
      for(int j=i+1; j<NB_FLOOR; j++0){
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
