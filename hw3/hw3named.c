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



sem_t *semMilk,*semFlour,*semWalnut,*semSugar,*chef0,*chef1,*chef2,*chef3,*chef4,*chef5;
sem_t *mutex1,*mutex2,*semAgent;


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
    int value1=0;

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
    destroy_semaphores();
    create_semaphores();
    
    initilizeshared();

    create_chefs_and_pushers();

    while(readfile_2_char(fd,array) != -1){

        sem_wait(semAgent);
        if (pick_ingredient(array[0],array[1],&first,&second) == -1){
          perror("Wrong input file");
          first = 'E';
          second = 'E';
        }
        sem_wait(mutex2);
        sprintf(buff,"the wholesaler (pid %d) delivers %c and %c\n",getpid(),first,second);
        write(1,buff,strlen(buff));
        keep_track->ing[0] = first;
        keep_track->ing[1] = second;
        sem_post(mutex2);
        if (posting_required_semaphores(first,second) == -1){
          perror("posting error");
          break;
        }
        sprintf(buff,"the wholesaler (pid %d) is waiting for the dessert\n",getpid());
        write(1,buff,strlen(buff));

        sem_wait(semAgent);
        sprintf(buff,"the wholesaler (pid %d) has obtained the dessert and left\n",getpid());
        write(1,buff,strlen(buff));
        sem_post(semAgent);
    }

    for(i = 0;i <10; i++){
      wait(NULL);
    }

    destroy_shared();
    destroy_semaphores();
    close_file(fd);

    return 0;
}


void pusher1(){
  while(True){
    sem_wait(semMilk);
    sem_wait(mutex1);
    sem_wait(mutex2);
    if (keep_track->isFlour == True){
        keep_track->isFlour=False;
        sem_post(chef3);
    }    
    else if (keep_track->isSugar == True){
        keep_track->isSugar=False;
        sem_post(chef5);
    }
    else if (keep_track->isWalnut == True){
        keep_track->isWalnut=False;
        sem_post(chef4);
    }
    else{
        keep_track->isMilk = True;
    }
    sem_post(mutex2);
    sem_post(mutex1);
  } 

}

void pusher2(){
  while(True){
    sem_wait(semFlour);
    sem_wait(mutex1);
    sem_wait(mutex2);
    if (keep_track->isMilk == True){
        keep_track->isMilk=False;
        sem_post(chef3);
    }    
    else if (keep_track->isSugar == True){
        keep_track->isSugar=False;
        sem_post(chef2);
    }
    else if (keep_track->isWalnut == True){
        keep_track->isWalnut=False;
        sem_post(chef1);
    }
    else{
        keep_track->isFlour = True;
    }
    sem_post(mutex2);
    sem_post(mutex1);
  } 
}

void pusher3(){
  while(True){
    sem_wait(semWalnut);
    sem_wait(mutex1);
    sem_wait(mutex2);
    if (keep_track->isMilk == True){
        keep_track->isMilk=False;
        sem_post(chef4);
    }    
    else if (keep_track->isSugar == True){
        keep_track->isSugar=False;
        sem_post(chef0);
    }
    else if (keep_track->isFlour == True){
        keep_track->isFlour=False;
        sem_post(chef1);
    }
    else{
        keep_track->isWalnut = True;
    }
    sem_post(mutex2);
    sem_post(mutex1);
  } 
}
void pusher4(){
  while(True){
    sem_wait(semSugar);
    sem_wait(mutex1);
    sem_wait(mutex2);
    if (keep_track->isMilk == True){
        keep_track->isMilk=False;
        sem_post(chef5);
    }    
    else if (keep_track->isWalnut == True){
        keep_track->isWalnut=False;
        sem_post(chef0);
    }
    else if (keep_track->isFlour == True){
        keep_track->isFlour=False;
        sem_post(chef2);
    }
    else{
        keep_track->isSugar = True;
    }
    sem_post(mutex2);
    sem_post(mutex1);
  } 
}

void chef0_func(){
  char buff[256];
  int count_dessert = 0;
  while(True){
    sprintf(buff,"chef0 (pid %d) is waiting for Walnuts and Sugar\n",getpid());
    write(1,buff,strlen(buff));
    sem_wait(chef0);
    sem_wait(mutex2);
    sprintf(buff,"chef0 (pid %d) has taken the Walnuts - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    if(keep_track->ing[0] == 'W'){
      keep_track->ing[0] = 'E';
    }  
    else{
      keep_track->ing[1] = 'E';  
    }    
    sprintf(buff,"chef0 (pid %d) has taken the Sugar - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    if(keep_track->ing[0] == 'S'){
      keep_track->ing[0] = 'E';
    }  
    else{
      keep_track->ing[1] = 'E';
    }   
    sprintf(buff,"chef0 (pid %d) is preparing the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    count_dessert++;
    sprintf(buff,"chef0 (pid %d) has delivered the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    sem_post(mutex2);
    sem_post(semAgent);
  }
}
void chef1_func(){
  int count_dessert=0;
  char buff[256];
  while(True){
    sprintf(buff,"chef1 (pid %d) is waiting for Flour and Walnuts\n",getpid());
    write(1,buff,strlen(buff));
    sem_wait(chef1);
    sem_wait(mutex2);
    sprintf(buff,"chef1 (pid %d) has taken the Flour - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    if(keep_track->ing[0] == 'F'){
      keep_track->ing[0] = 'E';
    }  
    else{
      keep_track->ing[1] = 'E'; 
    }
    sprintf(buff,"chef1 (pid %d) has taken the Walnuts - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    if(keep_track->ing[0] == 'W'){  
      keep_track->ing[0] = 'E';
    }
    else{
      keep_track->ing[1] = 'E'; 
    }
    sprintf(buff,"chef1 (pid %d) is preparing the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    count_dessert++;
    sprintf(buff,"chef1 (pid %d) has delivered the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(mutex2);
    sem_post(semAgent);
  }
}

void chef2_func(){
  int count_dessert=0;
  char buff[256];
  while(True){
    sprintf(buff,"chef2 (pid %d) is waiting for Sugar and Flour\n",getpid());
    write(1,buff,strlen(buff));
    sem_wait(chef2);
    sem_wait(mutex2);
    sprintf(buff,"chef2 (pid %d) has taken the Sugar - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    if(keep_track->ing[0] == 'S'){
      keep_track->ing[0] = 'E';
    }  
    else{
      keep_track->ing[1] = 'E'; 
    }
    sprintf(buff,"chef2 (pid %d) has taken the Flour - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    if(keep_track->ing[0] == 'F'){
      keep_track->ing[0] = 'E';
    }  
    else{
      keep_track->ing[1] = 'E'; 
    }  
    sprintf(buff,"chef2 (pid %d) is preparing the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    count_dessert++;
    sprintf(buff,"chef2 (pid %d) has delivered the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(mutex2);
    sem_post(semAgent);
  }
}
void chef3_func(){
  char buff[256];
  int count_dessert=0;
  while(True){
    sprintf(buff,"chef3 (pid %d) is waiting for Milk and Flour\n",getpid());
    write(1,buff,strlen(buff));
    sem_wait(chef3);
    sem_wait(mutex2);
    sprintf(buff,"chef3 (pid %d) has taken the Milk - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    if(keep_track->ing[0] == 'M'){
      keep_track->ing[0] = 'E';
    }  
    else{
      keep_track->ing[1] = 'E'; 
    }  

    sprintf(buff,"chef3 (pid %d) has taken the Flour - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    if(keep_track->ing[0] == 'F'){
      keep_track->ing[0] = 'E';
    }  
    else{
      keep_track->ing[1] = 'E'; 
    }  
    sprintf(buff,"chef3 (pid %d) is preparing the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    count_dessert++;

    sprintf(buff,"chef3 (pid %d) has delivered the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(mutex2);
    sem_post(semAgent);
  }
}

void chef4_func(){
  char buff[256];
  int count_dessert=0;
  while(True){
    sprintf(buff,"chef4 (pid %d) is waiting for Milk and Walnuts\n",getpid());
    write(1,buff,strlen(buff));
    sem_wait(chef4);
    sem_wait(mutex2);
    sprintf(buff,"chef4 (pid %d) has taken the Milk - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    if(keep_track->ing[0] == 'M'){
      keep_track->ing[0] = 'E';
    }  
    else{
      keep_track->ing[1] = 'E'; 
    }  

    sprintf(buff,"chef4 (pid %d) has taken the Walnuts - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    if(keep_track->ing[0] == 'W'){
      keep_track->ing[0] = 'E';
    }  
    else{
      keep_track->ing[1] = 'E'; 
    }  
    sprintf(buff,"chef4 (pid %d) is preparing the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    count_dessert++;

    sprintf(buff,"chef4 (pid %d) has delivered the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(mutex2);
    sem_post(semAgent);
  }
}
void chef5_func(){
  char buff[256];
  int count_dessert=0;
  while(True){
    sprintf(buff,"chef5 (pid %d) is waiting for Sugar and Milk\n",getpid());
    write(1,buff,strlen(buff));
    sem_wait(chef5);
    sem_wait(mutex2);
    sprintf(buff,"chef5 (pid %d) has taken the Sugar - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    if(keep_track->ing[0] == 'S'){
      keep_track->ing[0] = 'E';
    } 
    else{
      keep_track->ing[1] = 'E'; 
    }  

    sprintf(buff,"chef5 (pid %d) has taken the Milk - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    if(keep_track->ing[0] == 'M'){
      keep_track->ing[0] = 'E';
    }  
    else{
      keep_track->ing[1] = 'E'; 
    }
    sprintf(buff,"chef5 (pid %d) is preparing the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));

    count_dessert++;

    sprintf(buff,"chef5 (pid %d) has delivered the dessert - (%c %c)\n",getpid(),keep_track->ing[0],keep_track->ing[1]);
    write(1,buff,strlen(buff));
    sem_post(mutex2);
    sem_post(semAgent);
  }
}

void create_chefs_and_pushers(){
  int i = 0;
  for(i = 0; i < 10; i++){
    switch (child_ids[i] = fork()) {
        case -1: 
            exitInf("fork");  
        case 0: /* Child */
            create_semaphores();
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
              exit(0);
            }
            else if(i == 5){
              chef1_func();
              exit(0);
            }
            else if(i == 6){
              chef2_func();
              exit(0);
            }
            else if(i == 7){
              chef3_func();
              exit(0);
            }
            else if(i == 8){
              chef4_func();
              exit(0);
            }
            else if(i == 9){
              chef5_func();
              exit(0);
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
    semMilk = sem_open("milk_semaphore", O_CREAT , 0666, 0);
    if (semMilk == SEM_FAILED)
    {
      exitInf("semaphore");
    }
    semFlour = sem_open("flour_semaphore", O_CREAT , 0666, 0);
    if (semFlour == SEM_FAILED)
    {
      exitInf("semaphore");
    }
    semWalnut = sem_open("walnut_semaphore", O_CREAT, 0666, 0);
    if (semWalnut == SEM_FAILED)
    {
      exitInf("semaphore");
    }
    semSugar = sem_open("sugar_semaphore", O_CREAT, 0666, 0);
    if (semSugar == SEM_FAILED)
    {
      exitInf("semaphore");
    }
    chef0 = sem_open("chef0_semaphore", O_CREAT , 0666, 0);
    if (chef0 == SEM_FAILED)
    {
      exitInf("semaphore");
    }
    chef1 = sem_open("chef1_semaphore", O_CREAT , 0666, 0);
    if (chef1 == SEM_FAILED)
    {
      exitInf("semaphore");
    }
    chef2 = sem_open("chef2_semaphore", O_CREAT, 0666, 0);
    if (chef2 == SEM_FAILED)
    {
      exitInf("semaphore");
    }
    chef3 = sem_open("chef3_semaphore", O_CREAT, 0666, 0);
    if (chef3 == SEM_FAILED)
    {
      exitInf("semaphore");
    }
    chef4 = sem_open("chef4_semaphore", O_CREAT , 0666, 0);
    if (chef1 == SEM_FAILED)
    {
      exitInf("semaphore");
    }
    chef5 = sem_open("chef5_semaphore", O_CREAT, 0666, 0);
    if (chef2 == SEM_FAILED)
    {
      exitInf("semaphore");
    }
    mutex1 = sem_open("mutex1", O_CREAT, 0666, 1);
    if (mutex1 == SEM_FAILED)
    {
      exitInf("semaphore");
    }
    mutex2 = sem_open("mutex2", O_CREAT, 0666, 1);
    if (mutex2 == SEM_FAILED)
    {
      exitInf("semaphore");
    }
    semAgent = sem_open(agent_name, O_CREAT, 0666, 1);
    if (semAgent == SEM_FAILED)
    {
      exitInf("semaphore");
    }
}

void destroy_semaphores(){
  if (sem_close(semMilk) == -1)
      exitInf("sem close");
  if (sem_close(semFlour) == -1)
      exitInf("sem close");
  if (sem_close(semWalnut) == -1)
      exitInf("sem close");
  if (sem_close(semSugar) == -1)
      exitInf("sem close");
  if (sem_close(chef0) == -1)
      exitInf("sem close");
  if (sem_close(chef1) == -1)
      exitInf("sem close");
  if (sem_close(chef2) == -1)
      exitInf("sem close");
  if (sem_close(chef3) == -1)
      exitInf("sem close");
  if (sem_close(chef4) == -1)
      exitInf("sem close");
  if (sem_close(chef5) == -1)
      exitInf("sem close"); 
  if (sem_close(mutex1) == -1)
      exitInf("sem close");
  if (sem_close(mutex2) == -1)
      exitInf("sem close");
  if (sem_close(semAgent) == -1)
      exitInf("sem close");    
 
  if (sem_unlink("milk_semaphore") == -1){
      exitInf("unlink error");
  }
  if (sem_unlink("flour_semaphore") == -1){
      exitInf("unlink error");
  }
  if (sem_unlink("walnut_semaphore") == -1){
      exitInf("unlink error");
  }
  if (sem_unlink("sugar_semaphore") == -1){
      exitInf("unlink error");
  }
  if (sem_unlink("chef0_semaphore") == -1){
      exitInf("unlink error");
  }
  if (sem_unlink("chef1_semaphore") == -1){
      exitInf("unlink error");
  }
  if (sem_unlink("chef2_semaphore") == -1){
      exitInf("unlink error");
  }
  if (sem_unlink("chef3_semaphore") == -1){
      exitInf("unlink error");
  }
  if (sem_unlink("chef4_semaphore") == -1){
      exitInf("unlink error");
  }
  if (sem_unlink("chef5_semaphore") == -1){
      exitInf("unlink error");
  }
  if (sem_unlink("mutex1") == -1){
      exitInf("unlink error");
  }
  if (sem_unlink("mutex2") == -1){
      exitInf("unlink error");
  }
  if (sem_unlink(agent_name) == -1){
      exitInf("unlink error");
  }
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
    keep_track->ing[0] = 'E';
    keep_track->ing[1] = 'E';
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
int posting_required_semaphores(char first,char second){
  switch(first){
    case 'M':
        sem_post(semMilk);
         break;
    case 'F':
        sem_post(semFlour);
         break;
    case 'W':
        sem_post(semWalnut);
        break; 
    case 'S':
        sem_post(semSugar);
        break; 
    default:
        return -1;
  }
  switch(second){
    case 'M':
        if(first!='M')
          sem_post(semMilk);
        else{
          return -1;
        }  
        break;
    case 'F':
        if(first!='F')
          sem_post(semFlour);
        else{
          return -1;
        }  
        break;
    case 'W':
        if(first!='W')
          sem_post(semWalnut);
        else{
          return -1;
        }  
        break; 
    case 'S':
        if(first!='S')
          sem_post(semSugar);
        else{
          return -1;
        }
        break; 
    default:
        return -1;
  }
  return 0;
}





