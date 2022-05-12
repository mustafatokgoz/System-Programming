#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "helper.h"

#define COUNT1 0 
#define COUNT2 1

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

void lock_file(int fd, struct flock fl);
void unlock_file(int fd, struct flock fl);
int getCount(int num);
void post(int num);
void wait_two(int num1,int num2);
char *timestamp();

int Nloop;
int semid;

void *supplier(void* param){
    int rd;
    char c;
    struct flock fl = {F_WRLCK, SEEK_SET,0,0,0};
    char buff[256];
    int number1 = 0, number2 = 0;
    int fd = *((int *)(param));
    int c1,c2,nc1,nc2;
    char *currenttime;
    pthread_detach(pthread_self());

    lock_file(fd,fl);
    rd = read(fd,&c,1);
    while(rd > 0){
        if(c == '1'){
            
            //sprintf(buff,"%s - Supplier: read from input a ‘1’. Current amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),getCount(COUNT1),getCount(COUNT2));
            //write(1,buff,strlen(buff));
            
            c1 = getCount(COUNT1);
            c2 = getCount(COUNT2);
            currenttime = timestamp();
            fflush(stdout);
            fprintf(stdout,"%s - Supplier: read from input a ‘1’. Current amounts: %d x ‘1’, %d x ‘2’.\n",currenttime,c1,c2);
            fflush(stdout);

            //number1++;
            post(COUNT1);
            nc1 = getCount(COUNT1);
            nc2 = getCount(COUNT2);
            currenttime = timestamp();
            fflush(stdout);
            //sprintf(buff,"%s - Supplier: delivered a ‘1’. Post-delivery amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),getCount(COUNT1),getCount(COUNT2));
            //write(1,buff,strlen(buff));
            fprintf(stdout,"%s - Supplier: delivered a ‘1’. Post-delivery amounts: %d x ‘1’, %d x ‘2’.\n",currenttime,nc1,nc2);
            fflush(stdout);
   
        }
        else if(c == '2'){
            //sprintf(buff,"%s - Supplier: read from input a ‘2’. Current amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),getCount(COUNT1),getCount(COUNT2));
            //write(1,buff,strlen(buff));
            c1 = getCount(COUNT1);
            c2 = getCount(COUNT2);
            currenttime = timestamp();
            fflush(stdout);
            fprintf(stdout,"%s - Supplier: read from input a ‘2’. Current amounts: %d x ‘1’, %d x ‘2’.\n",currenttime,c1,c2);
            fflush(stdout);
            //number2++;
            post(COUNT2);

            nc1 = getCount(COUNT1);
            nc2 = getCount(COUNT2);
            //sprintf(buff,"%s - Supplier: delivered a ‘2’. Post-delivery amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),getCount(COUNT1),getCount(COUNT2));
            //write(1,buff,strlen(buff));
            currenttime = timestamp();
            fflush(stdout);
            fprintf(stdout,"%s - Supplier: delivered a ‘2’. Post-delivery amounts: %d x ‘1’, %d x ‘2’.\n",currenttime,nc1,nc2);
            fflush(stdout);
        } 
        
        rd = read(fd,&c,1);
    }
    unlock_file(fd,fl);
    fflush(stdout);
    sprintf(buff,"%s - The Supplier has left.\n",timestamp());
    write(1,buff,strlen(buff));
    fflush(stdout);

    pthread_exit(NULL);

}
void *consumer(void* param){
    int i = *((int *)(param));
    int j = 0;
    char buff[256];
    int c1,c2,nc1,nc2;
    char *currenttime;

    for(j = 0; j < Nloop;j++){
        //sprintf(buff ,"%s - Consumer-%d at iteration %d (waiting). Current amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),i,j,getCount(COUNT1),getCount(COUNT2));
        //write(1,buff,strlen(buff));
        
        c1 = getCount(COUNT1);
        c2 = getCount(COUNT2);
        currenttime = timestamp();
        fflush(stdout);
        fprintf(stdout ,"%s - Consumer-%d at iteration %d (waiting). Current amounts: %d x ‘1’, %d x ‘2’.\n",currenttime,i,j,c1,c2);
        fflush(stdout);
        // decrese take 1 and 2 
        wait_two(COUNT1,COUNT2);

        nc1 = getCount(COUNT1);
        nc2 = getCount(COUNT2);
        currenttime = timestamp();
        //sprintf(buff ,"%s - Consumer-%d at iteration %d (consumed). Post-consumption amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),i,j,getCount(COUNT1),getCount(COUNT2));
        //write(1,buff,strlen(buff));
        fflush(stdout);
        fprintf(stdout ,"%s - Consumer-%d at iteration %d (consumed). Current amounts: %d x ‘1’, %d x ‘2’.\n",currenttime,i,j,nc1,nc2);
        fflush(stdout);
        
    }
    fflush(stdout);
    sprintf(buff,"%s - Consumer-%d has left.\n",timestamp(),i);
    write(1,buff,strlen(buff));
    fflush(stdout);
    return NULL;
}


int main(int argc, char *argv[]){
    char c;
    int C,N;
    char *inputfile;
    int check_input = 0;
    char *err_mass = "You should enter the correct command (Example: ./hw4 -C 10 -N 5 -F inputfilePath)\n";
    int fd;
    pthread_t t1;
    pthread_t *Ctimes;
    key_t keys;
    union semun un[2];
    char buff[1024];
   memset( buff, '\0', sizeof( buff ));
   setvbuf(stdout, buff, _IONBF, 0);

    un[COUNT1].val = 0;
    un[COUNT2].val = 0;
    srand(time(NULL));

    while ((c = getopt (argc, argv, "C:N:F:")) != -1){
      switch (c){
        case 'C':
            C = atoi(optarg);
            check_input++;
            break;
        case 'N':
            N = atoi(optarg);
            check_input++;
            break;
        case 'F':
            inputfile = optarg;
            check_input++;
            break;  
        case '?':
          write(2,"Something went wrong.\n",strlen("Something went wrong.\n"));
          write(2,err_mass,strlen(err_mass)); 
          return 1;
        default:
          write(2,"Something went wrong.\n",strlen("Something went wrong.\n"));  
          write(2,err_mass,strlen(err_mass));
          return 1;
        }
    }
    if(check_input != 3){
        write(2,err_mass,strlen(err_mass));
        exit(0);
    }



    fd = open_file(inputfile);
    Nloop = N;

    //if ((keys = ftok("/tmp", 'a')) == -1){
	//    exitInf("error ftok key");
    //}
    keys = IPC_PRIVATE;
    if((semid = semget(keys, 2, 0666 | IPC_CREAT ) ) == -1){
	    exitInf("error semget");
    }    



    if(semctl(semid, COUNT1, SETVAL, un[COUNT1]) == -1 || semctl(semid, COUNT2, SETVAL, un[COUNT2]) == -1){
        exitInf("error intializing semaphores");
    }



    Ctimes = malloc(C * sizeof(pthread_t));

    void *ret;
    pthread_create(&t1,NULL,supplier,&fd);

    int i = 0, j= 0,send[C];

    for(i = 0; i < C;i++){
        send[i] = i;
        pthread_create(&Ctimes[i],NULL,consumer,&send[i]);
    }

    

    for(j = 0;j < C; j++){
        pthread_join(Ctimes[j], &ret);
    }
    
    //int *a = (int*) ret;
    //printf("\n ho %d\n", *a);

    free(Ctimes);
    if(semctl(semid, COUNT1, IPC_RMID, un) == -1){
        exitInf("error intializing semaphores1");
    }

    return 0;
}

int getCount(int num){
    int n;
    if((n = semctl(semid, num, GETVAL)) == -1){
	    perror("error semctl");
    }
    return n;
}




void post(int num){
    struct sembuf sb = {num , 1 , 0};

    if(semop(semid, &sb, 1) == -1){
	    perror("error semop post");
    }    

}


void wait_two(int num1,int num2){
    struct sembuf sb[2];
    sb[0].sem_num = num1;
    sb[0].sem_op = -1;
    sb[0].sem_flg = 0;
    sb[1].sem_num = num2;
    sb[1].sem_op = -1;
    sb[1].sem_flg = 0;

    if(semop(semid, sb, 2) == -1){
        perror("error wait_two");
    }

}



char *timestamp()
{
    time_t now;
    now = time(NULL);
    char *tstr = ctime(&now);
    char *line = strchr(tstr, '\n');
    line[0] = '\0';
    return tstr;
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




