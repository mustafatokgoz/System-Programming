#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include "helper.h"

int **A, **B;
pthread_t *mtimes;
int pow_num = 0;
int arrived = 0;
int N = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int power_of_2(int num);
int read_files_into_memory(int fd1, int fd2);
void free_array(int **arr, int n);
void *calculation(void* param);

int main(int argc, char*argv[]){
    char c;
    int n,m;
    char *inputfile1,*inputfile2,*output;
    int check_input = 0;
    char *err_mass = "You should enter the correct command (Example: ./hw5 -i filePath1 -j filePath2 -o output -n 4 -m 2)\n";
    int i,j,fd1,fd2;
    int *send;
    char buff[256];
    void *ret;

    while ((c = getopt (argc, argv, "i:j:o:n:m:")) != -1){
      switch (c){
        case 'i':
            inputfile1 = optarg;
            check_input++;
            break;
        case 'j':
            inputfile2 = optarg;
            check_input++;
            break;
        case 'o':
            output = optarg;
            check_input++;
            break;
        case 'n':
            n = atoi(optarg);
            check_input++;
            break;    
        case 'm':
            m = atoi(optarg);
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
    if(check_input != 5 || m < 2 || n < 2 || m%2 != 0){
        write(2,err_mass,strlen(err_mass));
        exit(0);
    }

    pow_num = power_of_2(n);
    N = m;

    A = calloc(pow_num, sizeof(int*));
    for(i = 0; i < pow_num; i++){
        A[i] = calloc(pow_num , sizeof(int));
    }

    B = calloc(pow_num, sizeof(int*));
    for(i = 0; i < pow_num; i++){
        B[i] = calloc(pow_num , sizeof(int));
    }

    fd1 = open_file(inputfile1);
    fd2 = open_file(inputfile2);

    if (read_files_into_memory(fd1,fd2) == 1){
        sprintf(buff,"Fatal Error \n");
        write(2,buff,strlen(buff));
        free_array(A,pow_num);
        free_array(B,pow_num);
        close_file(fd1);
        close_file(fd2);
        exit(0);
    }

    mtimes = malloc(m * sizeof(pthread_t));
    send = malloc(m * sizeof(int));

    for(i = 0; i < m;i++){
        send[i] = i;
        pthread_create(&mtimes[i],NULL,calculation,&send[i]);
    }

    for(j = 0; j < m; j++){
        pthread_join(mtimes[j], &ret);
        sprintf(buff," Thread number #%d  finished Second round\n",j);
        write(1,buff,strlen(buff));
    }


    /*
    printf(" %s %s %s %d %d %d\n",inputfile1,inputfile2,output,n,m,power_of_2(n));

    for(i = 0; i < pow_num; i++){
        for(j = 0; j < pow_num; j++){
            printf(" %d ",A[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    for(i = 0; i < pow_num; i++){
        for(j = 0; j < pow_num; j++){
            printf(" %d ",B[i][j]);
        }
        printf("\n");
    }
    */
    free_array(A,pow_num);
    free_array(B,pow_num);
    free(send);
    close_file(fd1);
    close_file(fd2);

    return 0;
}

void *calculation(void* param){
    int p = *((int *)(param));
    pthread_mutex_lock(&mutex);

    printf(" Thread number #%d \n",p);

    printf(" Thread number #%d\n",p);

    ++arrived;
    for(;;){
        if(arrived < N){
            pthread_cond_wait(&cond,&mutex);
        }    
        else{
            pthread_cond_broadcast(&cond);
            break;
        }    
    }
    
    printf("Thread %d is advancing to the second part\n",p);

    pthread_mutex_unlock(&mutex);
   
    pthread_exit(NULL);
}    


int read_files_into_memory(int fd1, int fd2){
    int i,j;
    int rd = 1;
    char c;
    
    for (i = 0; i < pow_num; i++){
        for(j = 0; j < pow_num; j++){
            rd = read(fd1, &c, 1);
            if(rd > 0){
                A[i][j] = (int) c;
            }
            else{
                return 1;
            }
        }
    }

    for (i = 0; i < pow_num; i++){
        for (j = 0; j < pow_num; j++){
            rd = read(fd2, &c, 1);
            if(rd > 0){
                B[i][j] = (int) c;
            }
            else{
                return 1;
            }
        }    
    }

    return 0;
}


int power_of_2(int num){
    int i = 0;
    int res = 1;
    for(i = 0; i < num; i++){
        res *= 2;
    }
    return res;
}


void free_array(int **arr, int n){
  int i = 0;
  for ( i = 0; i < n; i++ ){
        free(arr[i]);
  }
  free(arr);
}
