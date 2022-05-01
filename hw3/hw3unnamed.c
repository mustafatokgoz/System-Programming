#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include "helper.h"


typedef enum boolI {False = 0,True = 1} boolI;
//typedef enum {Milk = 0,Flour,Walnut,Sugar,Empty} ingredient;

typedef struct track{
  boolI isMilk;
  boolI isFlour;
  boolI isWalnut;
  boolI isSugar;
  char ing[2];
}track;

int readfile_2_char(int,char*);
void create_chefs_and_pushers();
void lock_file(int fd, struct flock fl);
void unlock_file(int fd, struct flock fl);
void initilizeshared();
void destroy_shared();
void create_semaphores();
void destroy_semaphores();
int pick_ingredient(char f,char s, char *first,char *second);
int posting_required_semaphores(char first, char second);
void print_ingredients(char f,char s,char one, char two);
void pusher1();
void pusher2();
void pusher3();
void pusher4();
void chef0_func();
void chef1_func();
void chef2_func();
void chef3_func();
void chef4_func();
void chef5_func();

typedef struct unnamed{
  sem_t semMilk,semFlour,semWalnut,semSugar,chef0,chef1,chef2,chef3,chef4,chef5;
  sem_t mutex1,mutex2,semAgent;
}unnamed;



static unnamed *sem_unnamed;
static track *keep_track;
pid_t child_ids[10];

char *agent_name;

int main(int argc, char *argv[]){
    char *inputfile;
    int i = 0;
    int check_input=0;
    char c;
    char *err_mass="Your should enter the correct command (i.e ./hw3named -i inputFilePath -n name).\n";
    char array[2];
    int fd;
    char first,second;
    char buff[256];
    pid_t childPid;
    int status;
    int total_dessert=0;

    while ((c = getopt (argc, argv, "i:n:")) != -1){
      switch (c){
        case 'i':
          inputfile = optarg;
          check_input++;
          break;
        case 'n':
          agent_name = optarg;
          check_input++;
          break;
        case '?':
          write(2,"Something went wrong.\n",strlen("Something went wrong.\n")); 
          return 1;
        default:
          write(2,"Something went wrong.\n",strlen("Something went wrong.\n"));  
          return 1;
        }
    }

    if(check_input != 2){
        write(2,err_mass,strlen(err_mass));
        exit(0);
    }

    fd = open_file(inputfile);

    create_semaphores();

    initilizeshared();

    create_chefs_and_pushers();

    while(readfile_2_char(fd,array) != -1){

        sem_wait(&sem_unnamed->semAgent);
        if (pick_ingredient(array[0],array[1],&first,&second) == -1){
          perror("Wrong input file");
          first = ' ';
          second = ' ';
        }
        sem_wait(&sem_unnamed->mutex2);

        keep_track->ing[0] = first;
        keep_track->ing[1] = second;

        print_ingredients(first,second,keep_track->ing[0],keep_track->ing[1]);
        
        sem_post(&sem_unnamed->mutex2);

        sprintf(buff,"the wholesaler (pid %d) is waiting for the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
        write(1,buff,strlen(buff));

        if (posting_required_semaphores(first,second) == -1){
          perror("posting error");
          break;
        }

        sem_wait(&sem_unnamed->semAgent);
        sprintf(buff,"the wholesaler (pid %d) has obtained the dessert and left - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
        write(1,buff,strlen(buff));
        sem_post(&sem_unnamed->semAgent);
    }
    
    
    for(i = 0;i <10; i++){
      sem_post(&sem_unnamed->mutex2);
      if(i == 0){
        sem_post(&sem_unnamed->semMilk);
        wait(NULL);
      }
      else if(i == 1){
        sem_post(&sem_unnamed->semFlour);
        wait(NULL);
      }
      else if (i == 2){
        sem_post(&sem_unnamed->semWalnut);
        wait(NULL);
      }
      else if(i == 3){
        sem_post(&sem_unnamed->semSugar);
        wait(NULL);
      }
      else if (i == 4){
        sem_post(&sem_unnamed->chef0);
        if ((childPid = waitpid(-1, &status, 0)) == -1 ){
          exitInf("waitpid error");
        }
        total_dessert += WEXITSTATUS(status);

      }
      else if (i == 5){
        sem_post(&sem_unnamed->chef1);
        if ((childPid = waitpid(-1, &status, 0)) == -1 ){
          exitInf("waitpid error");
        }
        total_dessert += WEXITSTATUS(status);
      }
      else if (i == 6){
        sem_post(&sem_unnamed->chef2);
        if ((childPid = waitpid(-1, &status, 0)) == -1 ){
          exitInf("waitpid error");
        }
        total_dessert += WEXITSTATUS(status);
      }
      else if (i == 7){
        sem_post(&sem_unnamed->chef3);
        if ((childPid = waitpid(-1, &status, 0)) == -1 ){
          exitInf("waitpid error");
        }
        total_dessert += WEXITSTATUS(status);
      }
      else if (i == 8){
        sem_post(&sem_unnamed->chef4);
        if ((childPid = waitpid(-1, &status, 0)) == -1 ){
          exitInf("waitpid error");
        }
        total_dessert += WEXITSTATUS(status);
      }
      else if (i == 9){
        sem_post(&sem_unnamed->chef5);
        if ((childPid = waitpid(-1, &status, 0)) == -1 ){
          exitInf("waitpid error");
        }
        total_dessert += WEXITSTATUS(status);
      }
    }
    sprintf(buff,"the wholesaler (pid %d) is done (total desserts: %d)\n",getpid(),total_dessert);
    write(1,buff,strlen(buff));

    destroy_shared();
    destroy_semaphores();
    close_file(fd);

    return 0;
}


void pusher1(){
  int value;
  while(True){
    sem_wait(&sem_unnamed->semMilk);
    sem_getvalue(&sem_unnamed->mutex2,&value);
    if(value > 1){
      break;
    }
    sem_wait(&sem_unnamed->mutex1);
    sem_wait(&sem_unnamed->mutex2);
    if (keep_track->isFlour == True){
        keep_track->isFlour=False;
        sem_post(&sem_unnamed->chef3);
    }    
    else if (keep_track->isSugar == True){
        keep_track->isSugar=False;
        sem_post(&sem_unnamed->chef5);
    }
    else if (keep_track->isWalnut == True){
        keep_track->isWalnut=False;
        sem_post(&sem_unnamed->chef4);
    }
    else{
        keep_track->isMilk = True;
    }
    sem_post(&sem_unnamed->mutex2);
    sem_post(&sem_unnamed->mutex1);
  } 

}

void pusher2(){
  int value;
  while(True){
    sem_wait(&sem_unnamed->semFlour);
    sem_getvalue(&sem_unnamed->mutex2,&value);
    if(value > 1){
      break;
    }
    sem_wait(&sem_unnamed->mutex1);
    sem_wait(&sem_unnamed->mutex2);
    if (keep_track->isMilk == True){
        keep_track->isMilk=False;
        sem_post(&sem_unnamed->chef3);
    }    
    else if (keep_track->isSugar == True){
        keep_track->isSugar=False;
        sem_post(&sem_unnamed->chef2);
    }
    else if (keep_track->isWalnut == True){
        keep_track->isWalnut=False;
        sem_post(&sem_unnamed->chef1);
    }
    else{
        keep_track->isFlour = True;
    }
    sem_post(&sem_unnamed->mutex2);
    sem_post(&sem_unnamed->mutex1);
  } 
}

void pusher3(){
  int value;
  while(True){
    sem_wait(&sem_unnamed->semWalnut);
    sem_getvalue(&sem_unnamed->mutex2,&value);
    if(value > 1){
      break;
    }
    sem_wait(&sem_unnamed->mutex1);
    sem_wait(&sem_unnamed->mutex2);
    if (keep_track->isMilk == True){
        keep_track->isMilk=False;
        sem_post(&sem_unnamed->chef4);
    }    
    else if (keep_track->isSugar == True){
        keep_track->isSugar=False;
        sem_post(&sem_unnamed->chef0);
    }
    else if (keep_track->isFlour == True){
        keep_track->isFlour=False;
        sem_post(&sem_unnamed->chef1);
    }
    else{
        keep_track->isWalnut = True;
    }
    sem_post(&sem_unnamed->mutex2);
    sem_post(&sem_unnamed->mutex1);
  } 
}
void pusher4(){
  int value;
  while(True){
    sem_wait(&sem_unnamed->semSugar);
    sem_getvalue(&sem_unnamed->mutex2,&value);
    if(value > 1){
      break;
    }
    sem_wait(&sem_unnamed->mutex1);
    sem_wait(&sem_unnamed->mutex2);
    if (keep_track->isMilk == True){
        keep_track->isMilk=False;
        sem_post(&sem_unnamed->chef5);
    }    
    else if (keep_track->isWalnut == True){
        keep_track->isWalnut=False;
        sem_post(&sem_unnamed->chef0);
    }
    else if (keep_track->isFlour == True){
        keep_track->isFlour=False;
        sem_post(&sem_unnamed->chef2);
    }
    else{
        keep_track->isSugar = True;
    }
    sem_post(&sem_unnamed->mutex2);
    sem_post(&sem_unnamed->mutex1);
  } 
}

void chef0_func(){
  char buff[256];
  int count_dessert = 0;
  int value;
  while(True){
    sem_wait(&sem_unnamed->mutex2);
    sprintf(buff,"chef0 (pid %d) is waiting for Walnuts and Sugar - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(&sem_unnamed->mutex2);

    sem_wait(&sem_unnamed->chef0);
    sem_getvalue(&sem_unnamed->mutex2,&value);
    if(value > 1){
      break;
    }
    sem_wait(&sem_unnamed->mutex2);
    
    if(keep_track->ing[0] == 'W'){
      keep_track->ing[0] = ' ';
    }  
    else{
      keep_track->ing[1] = ' ';  
    }
    sprintf(buff,"chef0 (pid %d) has taken the Walnuts - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    if(keep_track->ing[0] == 'S'){
      keep_track->ing[0] = ' ';
    }  
    else{
      keep_track->ing[1] = ' ';
    }

    sprintf(buff,"chef0 (pid %d) has taken the Sugar - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    sprintf(buff,"chef0 (pid %d) is preparing the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    count_dessert++;
    sprintf(buff,"chef0 (pid %d) has delivered the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    sem_post(&sem_unnamed->mutex2);
    sem_post(&sem_unnamed->semAgent);
  }

  sprintf(buff,"chef0 (pid %d) is exiting\n",getpid());
  write(1,buff,strlen(buff));
  if(count_dessert > 254){
    count_dessert = 254;
  }
  exit(count_dessert);

}
void chef1_func(){
  int count_dessert=0;
  char buff[256];
  int value;
  while(True){
    sem_wait(&sem_unnamed->mutex2);
    sprintf(buff,"chef1 (pid %d) is waiting for Flour and Walnuts - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(&sem_unnamed->mutex2);
    sem_wait(&sem_unnamed->chef1);
    sem_getvalue(&sem_unnamed->mutex2,&value);
    if(value > 1){
      break;
    }
    sem_wait(&sem_unnamed->mutex2);
    
    if(keep_track->ing[0] == 'F'){
      keep_track->ing[0] = ' ';
    }  
    else{
      keep_track->ing[1] = ' '; 
    }

    sprintf(buff,"chef1 (pid %d) has taken the Flour - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    
    if(keep_track->ing[0] == 'W'){  
      keep_track->ing[0] = ' ';
    }
    else{
      keep_track->ing[1] = ' '; 
    }

    sprintf(buff,"chef1 (pid %d) has taken the Walnuts - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    sprintf(buff,"chef1 (pid %d) is preparing the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    count_dessert++;
    sprintf(buff,"chef1 (pid %d) has delivered the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(&sem_unnamed->mutex2);
    sem_post(&sem_unnamed->semAgent);
  }

  sprintf(buff,"chef1 (pid %d) is exiting\n",getpid());
  write(1,buff,strlen(buff));
  if(count_dessert > 254){
    count_dessert = 254;
  }
  exit(count_dessert);
}

void chef2_func(){
  int count_dessert=0;
  char buff[256];
  int value;
  while(True){
    sem_wait(&sem_unnamed->mutex2);
    sprintf(buff,"chef2 (pid %d) is waiting for Sugar and Flour - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(&sem_unnamed->mutex2);
    sem_wait(&sem_unnamed->chef2);
    sem_getvalue(&sem_unnamed->mutex2,&value);
    if(value > 1){
      break;
    }
    sem_wait(&sem_unnamed->mutex2);
    
    if(keep_track->ing[0] == 'S'){
      keep_track->ing[0] = ' ';
    }  
    else{
      keep_track->ing[1] = ' '; 
    }

    sprintf(buff,"chef2 (pid %d) has taken the Sugar - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));


    
    if(keep_track->ing[0] == 'F'){
      keep_track->ing[0] = ' ';
    }  
    else{
      keep_track->ing[1] = ' '; 
    }

    sprintf(buff,"chef2 (pid %d) has taken the Flour - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    sprintf(buff,"chef2 (pid %d) is preparing the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    count_dessert++;
    sprintf(buff,"chef2 (pid %d) has delivered the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(&sem_unnamed->mutex2);
    sem_post(&sem_unnamed->semAgent);
  }
  sprintf(buff,"chef2 (pid %d) is exiting\n",getpid());
  write(1,buff,strlen(buff));
  if(count_dessert > 254){
    count_dessert = 254;
  }
  exit(count_dessert);
}
void chef3_func(){
  char buff[256];
  int count_dessert=0;
  int value;
  while(True){
    sem_wait(&sem_unnamed->mutex2);
    sprintf(buff,"chef3 (pid %d) is waiting for Milk and Flour - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(&sem_unnamed->mutex2);

    sem_wait(&sem_unnamed->chef3);
    sem_getvalue(&sem_unnamed->mutex2,&value);
    if(value > 1){
      break;
    }
    sem_wait(&sem_unnamed->mutex2);
    
    if(keep_track->ing[0] == 'M'){
      keep_track->ing[0] = ' ';
    }  
    else{
      keep_track->ing[1] = ' '; 
    }  

    sprintf(buff,"chef3 (pid %d) has taken the Milk - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    
    if(keep_track->ing[0] == 'F'){
      keep_track->ing[0] = ' ';
    }  
    else{
      keep_track->ing[1] = ' '; 
    }  

    sprintf(buff,"chef3 (pid %d) has taken the Flour - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    sprintf(buff,"chef3 (pid %d) is preparing the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    count_dessert++;

    sprintf(buff,"chef3 (pid %d) has delivered the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(&sem_unnamed->mutex2);
    sem_post(&sem_unnamed->semAgent);
  }
  sprintf(buff,"chef3 (pid %d) is exiting\n",getpid());
  write(1,buff,strlen(buff));
  if(count_dessert > 254){
    count_dessert = 254;
  }
  exit(count_dessert);
}

void chef4_func(){
  char buff[256];
  int count_dessert=0;
  int value;
  while(True){
    sem_wait(&sem_unnamed->mutex2);
    sprintf(buff,"chef4 (pid %d) is waiting for Milk and Walnuts - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(&sem_unnamed->mutex2);

    sem_wait(&sem_unnamed->chef4);
    sem_getvalue(&sem_unnamed->mutex2,&value);
    if(value > 1){
      break;
    }
    sem_wait(&sem_unnamed->mutex2);
    
    if(keep_track->ing[0] == 'M'){
      keep_track->ing[0] = ' ';
    }  
    else{
      keep_track->ing[1] = ' '; 
    }  

    sprintf(buff,"chef4 (pid %d) has taken the Milk - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    if(keep_track->ing[0] == 'W'){
      keep_track->ing[0] = ' ';
    }  
    else{
      keep_track->ing[1] = ' '; 
    }  

    sprintf(buff,"chef4 (pid %d) has taken the Walnuts - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
  
    sprintf(buff,"chef4 (pid %d) is preparing the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    count_dessert++;

    sprintf(buff,"chef4 (pid %d) has delivered the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(&sem_unnamed->mutex2);
    sem_post(&sem_unnamed->semAgent);
  }
  sprintf(buff,"chef4 (pid %d) is exiting\n",getpid());
  write(1,buff,strlen(buff));
  if(count_dessert > 254){
    count_dessert = 254;
  }
  exit(count_dessert);
}
void chef5_func(){
  char buff[256];
  int count_dessert=0;
  int value;
  while(True){
    sem_wait(&sem_unnamed->mutex2);
    sprintf(buff,"chef5 (pid %d) is waiting for Sugar and Milk - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(&sem_unnamed->mutex2);
    
    sem_wait(&sem_unnamed->chef5);
    sem_getvalue(&sem_unnamed->mutex2,&value);
    if(value > 1){
      break;
    }
    sem_wait(&sem_unnamed->mutex2);
    if(keep_track->ing[0] == 'S'){
      keep_track->ing[0] = ' ';
    } 
    else{
      keep_track->ing[1] = ' '; 
    }  

    sprintf(buff,"chef5 (pid %d) has taken the Sugar - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    if(keep_track->ing[0] == 'M'){
      keep_track->ing[0] = ' ';
    }  
    else{
      keep_track->ing[1] = ' '; 
    }

    sprintf(buff,"chef5 (pid %d) has taken the Milk - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    sprintf(buff,"chef5 (pid %d) is preparing the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    count_dessert++;

    sprintf(buff,"chef5 (pid %d) has delivered the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(&sem_unnamed->mutex2);
    sem_post(&sem_unnamed->semAgent);
  }

  sprintf(buff,"chef5 (pid %d) is exiting\n",getpid());
  write(1,buff,strlen(buff));
  if(count_dessert > 254){
    count_dessert = 254;
  }
  exit(count_dessert);
}

void create_chefs_and_pushers(){
  int i = 0;
  for(i = 0; i < 10; i++){
    switch (child_ids[i] = fork()) {
        case -1: 
            exitInf("fork");  
        case 0: /* Child */
            if(i == 0){
              pusher1();
              exit(0);
            }
            else if(i == 1){
              pusher2();
              exit(0);
            }
            else if(i == 2){
              pusher3();
              exit(0);
            }
            else if(i == 3){
              pusher4();
              exit(0);
            }
            else if(i == 4){
              chef0_func();
            }
            else if(i == 5){
              chef1_func();
            }
            else if(i == 6){
              chef2_func();
              exit(0);
            }
            else if(i == 7){
              chef3_func();
            }
            else if(i == 8){
              chef4_func();
            }
            else if(i == 9){
              chef5_func();
            }
            exit(0);
        default: /* Parent */
            break;
      }
  }
}



int readfile_2_char(int fd,char array[]){
    int rd;
    char c;
    struct flock fl = {F_WRLCK, SEEK_SET,0,0,0};
    lock_file(fd,fl);
    rd = read(fd,&c,1);
    if (rd <= 0 || c == '\n'){
        unlock_file(fd,fl);
        return -1;
    }
    array[0] = c;
    rd = read(fd,&c,1);
    if (rd <= 0 || c == '\n'){
        unlock_file(fd,fl);
        return -1;
    }
    array[1] = c;

    rd = read(fd,&c,1);
    if(rd < 0){
        unlock_file(fd,fl);
        return -1;
    }
    if(rd > 0 && c != '\n'){
        unlock_file(fd,fl);
        return -1;
    }
    unlock_file(fd,fl);
    return 0;
}

void create_semaphores(){
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IRWXU;
    char memoryName[25];
    strcpy(memoryName,"unnamed_semaphore");
    shm_unlink(memoryName);
    int memFd = shm_open(memoryName, O_RDWR | O_CREAT , mode);
    if (memFd == -1)
        exitInf("shm_open error!");
    if (ftruncate(memFd, sizeof(*sem_unnamed)) == -1)  
        exitInf("ftruncate error");

    sem_unnamed = (unnamed *)mmap(NULL, sizeof(*sem_unnamed), PROT_READ | PROT_WRITE, MAP_SHARED, memFd, 0);
    if (sem_unnamed == MAP_FAILED)
        exitInf("mmap");
    
    if (sem_init(&sem_unnamed->semMilk,1,0) < 0){
        perror("semaphore initilization error");
    }
    if (sem_init(&sem_unnamed->semFlour,1,0) < 0){
        perror("semaphore initilization error");
    }
    if (sem_init(&sem_unnamed->semWalnut,1,0) < 0){
        perror("semaphore initilization error");
    }
    if (sem_init(&sem_unnamed->semSugar,1,0) < 0){
        perror("semaphore initilization error");
    }
    if (sem_init(&sem_unnamed->chef0,1,0) < 0){
        perror("semaphore initilization error");
    }
    if (sem_init(&sem_unnamed->chef1,1,0) < 0){
        perror("semaphore initilization error");
    }
    if (sem_init(&sem_unnamed->chef2,1,0) < 0){
        perror("semaphore initilization error");
    }
    if (sem_init(&sem_unnamed->chef3,1,0) < 0){
        perror("semaphore initilization error");
    }
    if (sem_init(&sem_unnamed->chef4,1,0) < 0){
        perror("semaphore initilization error");
    }
    if (sem_init(&sem_unnamed->chef5,1,0) < 0){
        perror("semaphore initilization error");
    }
    if (sem_init(&sem_unnamed->mutex1,1,1) < 0){
        perror("semaphore initilization error");
    }
    if (sem_init(&sem_unnamed->mutex2,1,1) < 0){
        perror("semaphore initilization error");
    }
    if (sem_init(&sem_unnamed->semAgent,1,1) < 0){
        perror("semaphore initilization error");
    }
    close(memFd);
}

void destroy_semaphores(){
  if (sem_destroy(&sem_unnamed->semMilk) == -1)
      exitInf("sem close");
  if (sem_destroy(&sem_unnamed->semFlour) == -1)
      exitInf("sem close");
  if (sem_destroy(&sem_unnamed->semWalnut) == -1)
      exitInf("sem close");
  if (sem_destroy(&sem_unnamed->semSugar) == -1)
      exitInf("sem close");
  if (sem_destroy(&sem_unnamed->chef0) == -1)
      exitInf("sem close");
  if (sem_destroy(&sem_unnamed->chef1) == -1)
      exitInf("sem close");
  if (sem_destroy(&sem_unnamed->chef2) == -1)
      exitInf("sem close");
  if (sem_destroy(&sem_unnamed->chef3) == -1)
      exitInf("sem close");
  if (sem_destroy(&sem_unnamed->chef4) == -1)
      exitInf("sem close");
  if (sem_destroy(&sem_unnamed->chef5) == -1)
      exitInf("sem close");
  if (sem_destroy(&sem_unnamed->mutex1) == -1)
      exitInf("sem close");
  if (sem_destroy(&sem_unnamed->mutex2) == -1)
      exitInf("sem close");
  if (sem_destroy(&sem_unnamed->semAgent) == -1)
      exitInf("sem close");  


  char memoryName[25];
  strcpy(memoryName,"unnnamed_semaphore");
  shm_unlink(memoryName);     
}


void lock_file(int fd, struct flock fl){
    fl.l_type = F_WRLCK;
    if (fcntl(fd, F_SETLKW, &fl) == -1){
    		perror("fcntl");
    		exit(1);
    }   
}

void unlock_file(int fd, struct flock fl){
    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLKW, &fl) == -1){
    		perror("fcntl");
    		exit(1);
    }    
}

void initilizeshared(){  
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IRWXU;
    char memoryName[15];
    strcpy(memoryName,"chef_track");
    shm_unlink(memoryName);
    
    int memFd = shm_open(memoryName, O_RDWR | O_CREAT , mode);
    if (memFd == -1)
        exitInf("shm_open error!");
    if (ftruncate(memFd, sizeof(*keep_track)) == -1)  
        exitInf("ftruncate error");

    keep_track = (track *)mmap(NULL, sizeof(*keep_track), PROT_READ | PROT_WRITE, MAP_SHARED, memFd, 0);
    if (keep_track == MAP_FAILED)
        exitInf("mmap");
    
    keep_track->isMilk= 0;
    keep_track->isFlour = 0;
    keep_track->isSugar = 0;
    keep_track->isWalnut=0;
    keep_track->ing[0] = ' ';
    keep_track->ing[1] = ' ';
    close(memFd);
}


void destroy_shared(){
  char memoryName[15];
  strcpy(memoryName,"chef_track");
  shm_unlink(memoryName);
}



int pick_ingredient(char f,char s, char *first,char *second){
  switch(f){
    case 'M':
        *first = 'M';
         break;
    case 'F':
        *first = 'F';
         break;
    case 'W':
        *first = 'W';
        break; 
    case 'S':
        *first = 'S';
        break; 
    default:
        return -1;
  }
  switch(s){
    case 'M':
        *second = 'M';
         break;
    case 'F':
        *second = 'F';
         break;
    case 'W':
        *second = 'W';
        break; 
    case 'S':
        *second = 'S';
        break; 
    default:
        return -1;
  }
  return 0;
}

void print_ingredients(char f,char s,char one,char two){
  char buff[256];
  char buff2[50];
  switch(f){
    case 'M':
         sprintf(buff2,"Milk");
         break;
    case 'F':
        sprintf(buff2,"Flour");
         break;
    case 'W':
        sprintf(buff2,"Walnuts");
        break; 
    case 'S':
        sprintf(buff2,"Sugar");
        break; 
    default:
        break;
  }
  switch(s){
    case 'M':
         sprintf(buff,"the wholesaler (pid %d) delivers %s and Milk - (%c %c)\n",getpid(),buff2,one,two);
         write(1,buff,strlen(buff));
         break;
    case 'F':
        sprintf(buff,"the wholesaler (pid %d) delivers %s and Flour - (%c %c)\n",getpid(),buff2,one,two);
         write(1,buff,strlen(buff));
         break;
    case 'W':
        sprintf(buff,"the wholesaler (pid %d) delivers %s and Walnuts - (%c %c)\n",getpid(),buff2,one,two);
         write(1,buff,strlen(buff));
        break; 
    case 'S':
        sprintf(buff,"the wholesaler (pid %d) delivers %s and Sugar - (%c %c)\n",getpid(),buff2,one,two);
         write(1,buff,strlen(buff));
        break; 
    default:
        break;
  }
}

int posting_required_semaphores(char first,char second){
  switch(first){
    case 'M':
        sem_post(&sem_unnamed->semMilk);
         break;
    case 'F':
        sem_post(&sem_unnamed->semFlour);
         break;
    case 'W':
        sem_post(&sem_unnamed->semWalnut);
        break; 
    case 'S':
        sem_post(&sem_unnamed->semSugar);
        break; 
    default:
        return -1;
  }
  switch(second){
    case 'M':
        if(first!='M')
          sem_post(&sem_unnamed->semMilk);
        else{
          return -1;
        }  
        break;
    case 'F':
        if(first!='F')
          sem_post(&sem_unnamed->semFlour);
        else{
          return -1;
        }  
        break;
    case 'W':
        if(first!='W')
          sem_post(&sem_unnamed->semWalnut);
        else{
          return -1;
        }  
        break; 
    case 'S':
        if(first!='S')
          sem_post(&sem_unnamed->semSugar);
        else{
          return -1;
        }
        break; 
    default:
        return -1;
  }
  return 0;
}





