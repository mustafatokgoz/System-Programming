#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include "helper.h"

void lock_file(int fd, struct flock fl);
void unlock_file(int fd, struct flock fl);


void *threadtest(void* param){
       sleep(2);
       printf("Deneme Test 123 \n");
   int *ret = malloc(sizeof(int));
    *ret = 7;
    pthread_exit(ret);
}



int main(int argc, char *argv[]){
    char c;
    int C,N;
    char *inputfile;
    int check_input = 0;
    char *err_mass = "You should enter the correct command (Example: ./hw4 -C 10 -N 5 -F inputfilePath)\n";


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
    printf("%d %d %s \n",C,N,inputfile);
    pthread_t t1;
    void *ret;
    pthread_create(&t1,NULL,threadtest,NULL);
    pthread_join(t1, &ret);
    int *a = (int*) ret;
    printf("\n ho %d\n", *a);



    return 0;
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




