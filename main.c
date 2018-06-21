#include "main.h"


//Threads
pthread_t thread_elevator[NB_ELEVATOR]; //Tableau de Thread pour les ascenseurs
pthread_t thread_person[NB_MAX_PERSONS];
pthread_t thread_terminal;
pthread_t thread_god;

int nbExistingPerson=0;

bool personwait[NB_ELEVATOR][NB_MAX_PERSONS] = {{false}};
bool elevatorwait[NB_ELEVATOR][NB_MAX_PERSONS] = {{false}};

//Main function
int main(int argc, char const *argv[]) {
  srand(time(NULL));

  printf("\nMain => Début du programme\n");

  for(int i=0; i<NB_ELEVATOR; i++){
  	cond_elevator_request_terminal[i] = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
  }
  for(int i=0; i<NB_ELEVATOR; i++){
  	for(int j=0; j<NB_MAX_PERSONS; j++)
  	cond_elevator_request_person[i][j] = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
  }
  for(int i=0; i<NB_MAX_PERSONS; i++){
  	cond_person_request_terminal[i] = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
  }
  for(int i=0; i<NB_MAX_PERSONS; i++){
  	cond_person_request_elevator_in[i] = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
  }
  for(int i=0; i<NB_MAX_PERSONS; i++){
  	cond_person_request_elevator_out[i] = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
  }

  for(int i=0; i<NB_ELEVATOR; i++){
  	m_elevator[i]  = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  }
  for(int i=0; i<NB_MAX_PERSONS; i++){
  	m_person[i]  = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  }

  for(int i=0; i<NB_MAX_PERSONS; i++){ //initialisation de la liste d'attente de la borne
  	waitingList[i] = -1;
  }
  //Creation des threads ascenseur
  for(int i=0; i<NB_ELEVATOR; i++) {
    pthread_create(&thread_elevator[i], NULL, threadElevator ,(void *) i );
  }

  //Creation du thread borne
  pthread_create(&thread_terminal,NULL,threadTerminal,(void *)0);

  //Creation du thread dieu
  pthread_create(&thread_god,NULL,threadGod,(void *)0);





  //Attente de la fin du thread dieu
  pthread_join(thread_god,NULL);

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

  printf(ANSI_COLOR_GREEN "Je suis l'ascenseur\n" ANSI_COLOR_RESET);
  while(1){
    //on attend que la borne nous dise de changer d'étage
    pthread_cond_wait(&cond_elevator_request_terminal[elevator->id], &m_elevator[elevator->id]);
    if(elevator->goesUp){
      changeFloor(elevator,1); //l'ascenseur monte
      if(checkIfStop(elevator->destinationList, elevator->currentFloor)){ //on vérifie si on doit s'arrêter au nouvel étage
        for(int i=0; i<nbExistingPerson; i++){
          Person* p = &personList[i];
          if(p->elevatorLink == elevator->id){
            if(p->isInAnElevator) {
              if(elevator->currentFloor == p->wantedFloor){
                p->elevatorLink = -1;
                p->currentFloor = elevator->currentFloor;
                p->wantedFloor = -1;
                p->isInAnElevator = false;
                elevator->previsonalNbPersons --;
                printf("DestFloor = %d\n", elevator->destinationFloor);
                deleteDestination(elevator->destinationList, elevator->currentFloor); //on enlève cette destination de la liste
                elevator->destinationFloor=updateDestinationFloor(elevator->destinationList, elevator->currentFloor, elevator->goesUp, elevator->goesDown); //on met à jour l'étage de destination
                printf("DestFloor² = %d\n", elevator->destinationFloor);
                printf(ANSI_COLOR_GREEN "Ascenseur %d: la personne %d sort\n" ANSI_COLOR_RESET, elevator->id, p->id);
                //on reveille une personne est sortie
                printf("ascenseur reveille personne %d pour sortir\n", p->id);
                pthread_cond_signal(&cond_person_request_elevator_out[p->id]);
              }
            } else if(elevator->currentFloor == p->currentFloor){
              pthread_cond_wait(&cond_elevator_request_person[elevator->id][p->id], &m_elevator[elevator->id]);
              elevatorwait[elevator->id][p->id]=true;
              printf("DestFloor = %d\n", elevator->destinationFloor);
              deleteDestination(elevator->destinationList, elevator->currentFloor); //on enlève cette destination de la liste
              elevator->destinationFloor=updateDestinationFloor(elevator->destinationList, elevator->currentFloor, elevator->goesUp, elevator->goesDown); //on met à jour l'étage de destination
              printf("DestFloor² = %d\n", elevator->destinationFloor);
              printf(ANSI_COLOR_GREEN "Ascenseur %d: la personne %d entre\n" ANSI_COLOR_RESET, elevator->id, p->id);
              p->isInAnElevator = true;
              insertFloor(elevator->destinationList, p->wantedFloor); //on ajoute la destination du monsieur/madame/les2alafois à la liste des destinations
              elevator->destinationFloor=updateDestinationFloor(elevator->destinationList, elevator->currentFloor, elevator->goesUp, elevator->goesDown); //on met à jour l'étage de destination
              //on reveille une personne est rentrée
              printf("ascenseur reveille personne %d pour rentrer\n", p->id);
              while(!personwait[elevator->id][p->id]) {
                pthread_cond_signal(&cond_person_request_elevator_in[p->id]);
              }
              personwait[elevator->id][p->id]=false;
            }
          }
        }
        if(elevator->destinationFloor == -1){ //si il n'y a plus de destination, on arrête l'ascenseur
          elevator->goesUp = false;
          elevator->goesDown = false;
          printf(ANSI_COLOR_GREEN "Ascenseur %d: je m'arrête\n" ANSI_COLOR_RESET, elevator->id);
        } else if(elevator->destinationFloor < elevator->currentFloor){
          elevator->goesUp = false;
          elevator->goesDown = true;
          printf(ANSI_COLOR_GREEN "Ascenseur %d: je change de sens, je pars vers le bas\n" ANSI_COLOR_RESET, elevator->id);
        }
      }
    } else if(elevator->goesDown){
      changeFloor(elevator, -1); //l'ascenseur monte
      if(checkIfStop(elevator->destinationList, elevator->currentFloor)){ //on vérifie si on doit s'arrêter au nouvel étage
        for(int i=0; i<nbExistingPerson; i++){
          Person* p = &personList[i];
          if(p->elevatorLink == elevator->id){
            if(p->isInAnElevator) {
              if(elevator->currentFloor == p->wantedFloor){
                p->elevatorLink = -1;
                p->currentFloor = elevator->currentFloor;
                p->wantedFloor = -1;
                p->isInAnElevator = false;
                elevator->previsonalNbPersons --;
                deleteDestination(elevator->destinationList, elevator->currentFloor); //on enlève cette destination de la liste
                elevator->destinationFloor=updateDestinationFloor(elevator->destinationList, elevator->currentFloor, elevator->goesUp, elevator->goesDown); //on met à jour l'étage de destination
                printf(ANSI_COLOR_GREEN "Ascenseur %d: la personne %d sort\n" ANSI_COLOR_RESET, elevator->id, p->id);
                //on reveille une personne est sortie
                printf("ascenseur reveille personne %d pour sortir\n", p->id);
                pthread_cond_signal(&cond_person_request_elevator_out[p->id]);
              }
            } else if(elevator->currentFloor == p->currentFloor) {
              printf("Je me bloque en attendant %d\n", p->id);
              pthread_cond_wait(&cond_elevator_request_person[elevator->id][p->id], &m_elevator[elevator->id]);
              elevatorwait[elevator->id][p->id]=true;
              deleteDestination(elevator->destinationList, elevator->currentFloor); //on enlève cette destination de la liste
              elevator->destinationFloor=updateDestinationFloor(elevator->destinationList, elevator->currentFloor, elevator->goesUp, elevator->goesDown); //on met à jour l'étage de destination
              printf(ANSI_COLOR_GREEN "Ascenseur %d: la personne %d entre\n" ANSI_COLOR_RESET, elevator->id, p->id);
              p->isInAnElevator = true;
              insertFloor(elevator->destinationList, p->wantedFloor); //on ajoute la destination du monsieur/madame/les2alafois à la liste des destinations
              elevator->destinationFloor=updateDestinationFloor(elevator->destinationList, elevator->currentFloor, elevator->goesUp, elevator->goesDown); //on met à jour l'étage de destination
              //on reveille une personne est rentree
              printf("ascenseur reveille personne %d pour rentrer\n", p->id);
              while(!personwait[elevator->id][p->id]) {
                pthread_cond_signal(&cond_person_request_elevator_in[p->id]);
              }
              personwait[elevator->id][p->id]=false;
            }
          }
        }
        if(elevator->destinationFloor == -1){ //si il n'y a plus de destination, on arrête l'ascenseur
          elevator->goesUp = false;
          elevator->goesDown = false;
          printf(ANSI_COLOR_GREEN "Ascenseur %d: je m'arrête\n" ANSI_COLOR_RESET, elevator->id);
        } else if(elevator->destinationFloor > elevator->currentFloor){
          elevator->goesUp = true;
          elevator->goesDown = false;
          printf(ANSI_COLOR_GREEN "Ascenseur %d: je change de sens, je pars vers le haut\n" ANSI_COLOR_RESET, elevator->id);
        }
      }
    } else {
      for(int i=0; i<nbExistingPerson; i++){
        Person* p = &personList[i];
        if(p->elevatorLink == elevator->id){
          if(p->isInAnElevator) {
            if(elevator->currentFloor == p->wantedFloor){
              p->elevatorLink = -1;
              p->currentFloor = elevator->currentFloor;
              p->wantedFloor = -1;
              p->isInAnElevator = false;
              elevator->previsonalNbPersons --;
              deleteDestination(elevator->destinationList, elevator->currentFloor); //on enlève cette destination de la liste
              elevator->destinationFloor=updateDestinationFloor(elevator->destinationList, elevator->currentFloor, elevator->goesUp, elevator->goesDown); //on met à jour l'étage de destination
              printf(ANSI_COLOR_GREEN "Ascenseur %d: je suis a l'arret et la personne %d sort\n" ANSI_COLOR_RESET, elevator->id, p->id);
              //on reveille une personne est sortie
              printf("ascenseur reveille personne %d pour sortir\n", p->id);
              pthread_cond_signal(&cond_person_request_elevator_out[p->id]);
            }
          } else if(elevator->currentFloor == p->currentFloor) {
            pthread_cond_wait(&cond_elevator_request_person[elevator->id][p->id], &m_elevator[elevator->id]);
            elevatorwait[elevator->id][p->id]=true;
            deleteDestination(elevator->destinationList, elevator->currentFloor); //on enlève cette destination de la liste
            elevator->destinationFloor=updateDestinationFloor(elevator->destinationList, elevator->currentFloor, elevator->goesUp, elevator->goesDown); //on met à jour l'étage de destination
            printf(ANSI_COLOR_GREEN "Ascenseur %d: je suis a l'arret et la personne %d entre\n" ANSI_COLOR_RESET, elevator->id, p->id);
            p->isInAnElevator = true;
            if(elevator->currentFloor - p->wantedFloor > 0){
              elevator->goesDown = true;
              printf(ANSI_COLOR_GREEN "Ascenseur %d: je vais descendre\n" ANSI_COLOR_RESET,elevator->id);
            } else if(elevator->currentFloor - p->wantedFloor < 0){
              elevator->goesUp = true;
              printf(ANSI_COLOR_GREEN "Ascenseur %d: je vais monter\n" ANSI_COLOR_RESET,elevator->id);
            }

            insertFloor(elevator->destinationList, p->wantedFloor); //on ajoute la destination du monsieur/madame/les2alafois à la liste des destinations
            elevator->destinationFloor=updateDestinationFloor(elevator->destinationList, elevator->currentFloor, elevator->goesUp, elevator->goesDown); //on met à jour l'étage de destination
            printf(ANSI_COLOR_GREEN "Ascenseur %d: ma destination est %d\n" ANSI_COLOR_RESET,elevator->id , elevator->destinationFloor);
            //on reveille une personne est rentree
            printf("ascenseur reveille personne %d pour rentrer\n", p->id);
            while(!personwait[elevator->id][p->id]) {
              pthread_cond_signal(&cond_person_request_elevator_in[p->id]);
            }
            personwait[elevator->id][p->id]=false;
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

void* threadTerminal(void *arg) {
  int tmpPrevNbPers = 0;
  printf(ANSI_COLOR_YELLOW "Borne: Borne en marche \n" ANSI_COLOR_RESET);
  while(1){
    //printWaitingList();
    if(waitingList[0]>-1) { //si il y a qqn dans la liste d'attente
      Person* pers = findPerson(waitingList[0]);
      int i=0;
      while(i<NB_ELEVATOR && pers->elevatorLink==-1) {
        Elevator* elev = &elevatorList[i];
/*
        if(elev->previsonalNbPersons != tmpPrevNbPers) {
          printf("previsonalNbPersons: %d\n", elev->previsonalNbPersons);
          tmpPrevNbPers = elev->previsonalNbPersons;
        }
        */
        //printf("previsonalNbPersons: %d\n", elev->previsonalNbPersons);

        if(elev->previsonalNbPersons<MAX_CAPACITY) {
          //printf("dans condition previsonalNbPersons: %d\n", elev->previsonalNbPersons);
          if(elev->goesUp || elev->goesDown){ //si l'ascenseur se déplace vers le haut ou vers le bas
            insertFloor(elev->destinationList, pers->currentFloor);
            elev->destinationFloor=updateDestinationFloor(elev->destinationList, elev->currentFloor, elev->goesUp, elev->goesDown);
            pers->elevatorLink = elev->id;
            elev->previsonalNbPersons++;
          }
          else { //ascenseur à l'arret
            insertFloor(elev->destinationList, pers->currentFloor);
            elev->destinationFloor=updateDestinationFloor(elev->destinationList, elev->currentFloor, elev->goesUp, elev->goesDown);
            pers->elevatorLink = elev->id;
            elev->previsonalNbPersons++;

            if(elev->currentFloor < pers->currentFloor){
              elev->goesUp = true;
            } else if(elev->currentFloor > pers->currentFloor) {
              elev->goesDown = true;
            }
          }
          printf(ANSI_COLOR_YELLOW "Borne: La personne %d va attendre l'ascenseur %d pour aller à l'étage %d\n" ANSI_COLOR_RESET, pers->id, pers->elevatorLink, pers->wantedFloor);
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
void* threadGod(void *arg){
  int i = 0; //va suivre le nombre de personnnes créées par le dieu
  printf(ANSI_COLOR_RED "Dieu: Dieu est prêt à créer des personnes\n" ANSI_COLOR_RESET);
  while(i<NB_MAX_PERSONS){//pour que l'on ne mette pas plus de personnes que possible dans le tableau
    int r = rand()%100; //random between 0 and 99
    if(r<100) { //dieu a une chance sur 2 de créer une personne
      //Creation des threads person
      printf(ANSI_COLOR_RED "Dieu: Je crée la personne %d\n" ANSI_COLOR_RESET, i);
      pthread_create(&thread_person[i], NULL, threadPerson ,(void *) i );
      i++;
    }
    //sleep(1);
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

  nbExistingPerson++;
  printf(ANSI_COLOR_CYAN "Personne %d: Dieu vient de me créer, et je veux aller à l'étage %d\n" ANSI_COLOR_RESET,person->id, person->wantedFloor);

  while(1){
    if(person->wantedFloor > -1) { //si la personne veut aller quelque part
      if(person->elevatorLink == -1 && !person->isInAnElevator && person->wantedFloor>-1) { //si la personne n'est pas dans un ascenseur
        //puisque la personne est réveillée, on l'ajoute a la liste d'attente et on l'endort
        addPersonToWaitingList(person->id);
        //on attend que le terminal nous donne un ascenseur
        pthread_cond_wait(&cond_person_request_terminal[person->id], &m_person[person->id]);
      } else if(person->elevatorLink > -1 && !person->isInAnElevator && person->wantedFloor>-1){ //si la personne a été associée à un ascenseur, elle attend que l'ascenseur lui dise d'y rentrer
        //on dit à l'ascenseur qu'on va rentrer dans lui quand il sera là
        printf("personne %d: reveille ascenseur %d\n",person->id, person->elevatorLink);
        while(!elevatorwait[person->elevatorLink][person->id]){
          pthread_cond_signal(&cond_elevator_request_person[person->elevatorLink][person->id]);
        }
        elevatorwait[person->elevatorLink][person->id] = false;
        printf(ANSI_COLOR_CYAN "Personne %d: J'attend l'ascenseur a l'étage %d\n" ANSI_COLOR_RESET, person->id, person->currentFloor);
        //on attend que l'ascenseur nous dise de rentrer dedans de lui
        pthread_cond_wait(&cond_person_request_elevator_in[person->id], &m_person[person->id]);
        personwait[person->elevatorLink][person->id] = true;
        printf(ANSI_COLOR_CYAN "Personne %d: Je rentre dans l'ascenseur %d\n" ANSI_COLOR_RESET, person->id, person->elevatorLink);
      } else if(person->elevatorLink > -1 && person->isInAnElevator && person->wantedFloor>-1) { //si la personne est dans l'ascenseur, elle attend que l'ascenseur lui dise de sortir
        //on attend que l'ascenseur nous dise de sortir de dedans de lui
        pthread_cond_wait(&cond_person_request_elevator_out[person->id], &m_person[person->id]);
        printf(ANSI_COLOR_CYAN "Personne %d: Je sors de l'ascenseur \n" ANSI_COLOR_RESET, person->id);
      }
    } else {
      //TODO Il y aura ici une certaine chance que la personne veuille changer d'étage
      if(person->currentFloor != 1)
        person->wantedFloor = 1;
    }
    sleep(1);
  }
}

void printWaitingList() {
  printf("Waiting list: ");
  for(int i=0; i<NB_MAX_PERSONS; i++) {
    printf("%d ", waitingList[i]);
  }
  printf("\n");
}

void addPersonToWaitingList(int id) {
  int i = 0;
  bool inserted = false;
  printf("");
  while(i<nbExistingPerson && !inserted) {
    if(waitingList[i] == -1) {
      waitingList[i] = id;
      inserted = true;
    }
  }
}

Person* findPerson(int id){
  for(int i=0; i<nbExistingPerson; i++){
    if(personList[i].id == id)
      return &personList[i];
  }
}




/* Fonctions utilisées pour les ascenseurs */

void changeFloor(Elevator* elevator, int i){
  elevator->currentFloor +=i;
  printf(ANSI_COLOR_GREEN "Elevator %d: je %d d'étage, je vais à l'étage %d\n" ANSI_COLOR_RESET, elevator->id, i, elevator->currentFloor);
  sleep(TIME_BETWEEN_FLOORS);
}

void deleteDestination(int destinationList[], int currentFloor){
  //printDList(destinationList);
  for(int i=0; i<NB_FLOOR; i++){
    if(currentFloor == destinationList[i]){
      destinationList[i] = -1;
    }
  }
  deleteEmptyBoxElevator(destinationList);
  //printDList(destinationList);
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
  if(NB_MAX_PERSONS>1){
    waitingList[0] = waitingList[1];
    int i=1;
    while(i<NB_MAX_PERSONS-1 && waitingList[i]>-1){
      waitingList[i] = waitingList[i+1];
      i++;
    }
    if(i==NB_MAX_PERSONS-1) {
      waitingList[i] = -1;
    }
  } else {
    waitingList[0] = -1;
  }

}

void printDList(int destinationList[]) {
  printf("Destination List: ");
  for(int i=0; i<NB_FLOOR; i++) {
    printf("%d ", destinationList[i]);
  }
  printf("\n");
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
        if(isFinished){
          destinationList[j-1] = -1;
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
