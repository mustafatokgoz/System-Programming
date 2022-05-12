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
#include <signal.h>
#include "helper.h"

#define COUNT1 0 
#define COUNT2 1

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

sig_atomic_t sig_check=0;





void lock_file(int fd, struct flock fl);
void unlock_file(int fd, struct flock fl);
int getCount(int num);
void post(int num);
void wait_two(int num1,int num2);
char *timestamp();

int Nloop,Cnum;
int semid;
pthread_t *Ctimes;

void quit_c(){
  exitInf("\nYour program interrupt with ctrl c from keyboard exitinggg..\n");
}

/*to handle ctrl c*/
void signal_handle(int sig) {
    int j = 0;
    sig_check=1;

    for(j = 0;j < Cnum; j++){
        pthread_cancel(Ctimes[j]);
    }
    free(Ctimes);
    quit_c();
}


void *supplier(void* param){
    int rd;
    char c;
    struct flock fl = {F_WRLCK, SEEK_SET,0,0,0};
    int fd = *((int *)(param));
    int j;
    
    pthread_detach(pthread_self());

    lock_file(fd,fl);
    rd = read(fd,&c,1);
    while(rd > 0){
        if(sig_check == 1){
            unlock_file(fd,fl);
            close_file(fd);
            for(j = 0;j < Cnum; j++){
                pthread_cancel(Ctimes[j]);
            }
            free(Ctimes);
            quit_c();
        }
        if(c == '1'){
            
            //sprintf(buff,"%s - Supplier: read from input a ‘1’. Current amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),getCount(COUNT1),getCount(COUNT2));
            //write(1,buff,strlen(buff));

            fflush(stdout);
            fprintf(stdout,"%s - Supplier: read from input a ‘1’. Current amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),getCount(COUNT1),getCount(COUNT2));
            fflush(stdout);

           
            post(COUNT1);

            fflush(stdout);
            //sprintf(buff,"%s - Supplier: delivered a ‘1’. Post-delivery amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),getCount(COUNT1),getCount(COUNT2));
            //write(1,buff,strlen(buff));
            fprintf(stdout,"%s - Supplier: delivered a ‘1’. Post-delivery amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),getCount(COUNT1),getCount(COUNT2));
            fflush(stdout);
   
        }
        else if(c == '2'){
            //sprintf(buff,"%s - Supplier: read from input a ‘2’. Current amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),getCount(COUNT1),getCount(COUNT2));
            //write(1,buff,strlen(buff));
            fflush(stdout);
            fprintf(stdout,"%s - Supplier: read from input a ‘2’. Current amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),getCount(COUNT1),getCount(COUNT2));
            fflush(stdout);
            
            post(COUNT2);
            fflush(stdout);
            //sprintf(buff,"%s - Supplier: delivered a ‘2’. Post-delivery amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),getCount(COUNT1),getCount(COUNT2));
            //write(1,buff,strlen(buff));

            fprintf(stdout,"%s - Supplier: delivered a ‘2’. Post-delivery amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),getCount(COUNT1),getCount(COUNT2));
            fflush(stdout);
        } 
        
        rd = read(fd,&c,1);
    }
    unlock_file(fd,fl);
    fflush(stdout);
    fprintf(stdout,"%s - The Supplier has left.\n",timestamp());
    fflush(stdout);

    pthread_exit(NULL);

}
void *consumer(void* param){
    int i = *((int *)(param));
    int j = 0;

    for(j = 0; j < Nloop;j++){
        //sprintf(buff ,"%s - Consumer-%d at iteration %d (waiting). Current amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),i,j,getCount(COUNT1),getCount(COUNT2));
        //write(1,buff,strlen(buff));
        if(sig_check == 1){
            for(j = 0;j < Cnum; j++){
                pthread_cancel(Ctimes[j]);
            }
            free(Ctimes);
            quit_c();
        }
        fflush(stdout);



        fprintf(stdout ,"%s - Consumer-%d at iteration %d (waiting). Current amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),i,j,getCount(COUNT1),getCount(COUNT2));
        fflush(stdout);
        // decrese take 1 and 2 
        wait_two(COUNT1,COUNT2);

        fflush(stdout);
        //sprintf(buff ,"%s - Consumer-%d at iteration %d (consumed). Post-consumption amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),i,j,getCount(COUNT1),getCount(COUNT2));
        //write(1,buff,strlen(buff));
        
        fprintf(stdout ,"%s - Consumer-%d at iteration %d (consumed). Current amounts: %d x ‘1’, %d x ‘2’.\n",timestamp(),i,j,getCount(COUNT1),getCount(COUNT2));
        fflush(stdout);
        
    }
    fflush(stdout);
    fprintf(stdout,"%s - Consumer-%d has left.\n",timestamp(),i);
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
    key_t keys;
    union semun un[2];
    struct sigaction sa;


    sa.sa_handler = signal_handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;
    sigaction(SIGINT,&sa,NULL);

    setvbuf(stdout, NULL, _IONBF,0);

    un[COUNT1].val = 0;
    un[COUNT2].val = 0;
    //srand(time(NULL));

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
    if(check_input != 3 || C < 4 || N < 2){
        write(2,err_mass,strlen(err_mass));
        exit(0);
    }



    fd = open_file(inputfile);
    Nloop = N;
    Cnum = C;

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

    if(sig_check == 1){
        close_file(fd);
        free(Ctimes);
        quit_c();
    }


    int i = 0, j= 0,send[C];

    for(i = 0; i < C;i++){
        send[i] = i;
        pthread_create(&Ctimes[i],NULL,consumer,&send[i]);
    }

    if(sig_check == 1){
        close_file(fd);
        for(j = 0;j < C; j++){
            pthread_cancel(Ctimes[j]);
        }
        free(Ctimes);
        quit_c();
    }
    

    for(j = 0;j < C; j++){
        pthread_join(Ctimes[j], &ret);
    }
    
    if(sig_check == 1){
        close_file(fd);
        free(Ctimes);
        quit_c();
    }

    free(Ctimes);


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
    time_t localTime;
    localTime=time(NULL);
    char *str = asctime( localtime(&localTime));
    char *removed = strchr(str, '\n');
    if(removed != NULL)
        removed[0] = '\0';
    return str;
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




