#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include "helper.h"

int **A, **B, **C;
pthread_t *mtimes;
int pow_num = 0;
int arrived = 0;
int N = 0;
int fd1 = -1,fd2 = -1;
int *send;
clock_t start_t, end_t;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int power_of_2(int num);
int read_files_into_memory(int fd1, int fd2);
void free_array(int **arr, int n);
void *calculation(void* param);
void calculate_product(int r1,int c2,int c1, int j1);
char *timestamp();
void quit_c();
void calculate_dft();

sig_atomic_t sig_check=0;

/*to handle ctrl c*/
void signal_handle(int sig) {
    int i;
    if(A!=NULL)
        free_array(A,pow_num);
    if(B!=NULL)    
        free_array(B,pow_num);
    if(C!=NULL)    
        free_array(C,pow_num);
    if (fd1 != -1)
        close(fd1);
    if (fd2 != -1)    
        close(fd2);
    if(send!=NULL){    
        for(i = 0;i < N; i++){
            pthread_cancel(mtimes[i]);
        }
        free(send);
    }    
    if(mtimes!=NULL)  
        free(mtimes);   
    quit_c();
}

void quit_c(){
  exitInf("\nYour program interrupt with ctrl c from keyboard exitinggg..\n");
}


int main(int argc, char*argv[]){
    char c;
    int n,m;
    char *inputfile1,*inputfile2,*output;
    int check_input = 0;
    char *err_mass = "You should enter the correct command (Example: ./hw5 -i filePath1 -j filePath2 -o output -n 4 -m 2)\n";
    int i,j;
    char buff[256];
    void *ret;
    struct sigaction sa;


    sa.sa_handler = signal_handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;
    sigaction(SIGINT,&sa,NULL);
    
    start_t = clock();
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
    pow_num = power_of_2(n);
    N = m;

    if(check_input != 5 || m < 2 || n < 3 || m%2 != 0 || pow_num % m != 0){
        write(2,err_mass,strlen(err_mass));
        exit(0);
    }

    if(sig_check == 1){
        quit_c();
    }
   

    A = calloc(pow_num, sizeof(int*));
    for(i = 0; i < pow_num; i++){
        A[i] = calloc(pow_num , sizeof(int));
    }

    B = calloc(pow_num, sizeof(int*));
    for(i = 0; i < pow_num; i++){
        B[i] = calloc(pow_num , sizeof(int));
    }
    

    C = calloc(pow_num, sizeof(int*));
    for(i = 0; i < pow_num; i++){
        C[i] = calloc(pow_num , sizeof(int));
    }


    
    fd1 = open_file(inputfile1);
    fd2 = open_file(inputfile2);

    if (read_files_into_memory(fd1,fd2) == 1){
        sprintf(buff,"Fatal Error \n");
        write(2,buff,strlen(buff));
        free_array(A,pow_num);
        free_array(B,pow_num);
        free_array(C,pow_num);
        close_file(fd1);
        close_file(fd2);
        exit(0);
    }

    sprintf(buff,"%s , Two matrices of size %dx%d have been read. The number of threads is %d\n",timestamp(),pow_num,pow_num,m);
    write(1,buff,strlen(buff));

    mtimes = malloc(m * sizeof(pthread_t));
    send = malloc(m * sizeof(int));

    

    for(i = 0; i < m;i++){
        send[i] = i+1;
        pthread_create(&mtimes[i],NULL,calculation,&send[i]);
    }



    for(j = 0; j < m; j++){
        pthread_join(mtimes[j], &ret);
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
    for(i = 0; i < pow_num; i++){
        for(j = 0; j < pow_num; j++){
            printf(" %d ",C[i][j]);
        }
        printf("\n");
    }
    */
  
    for(i = 0; i < pow_num; i++){
        for(j = 0; j < pow_num; j++){
            printf(" %d ",C[i][j]);
        }
        printf("\n");
    }

    
    free_array(A,pow_num);
    free_array(B,pow_num);
    free_array(C,pow_num);
    free(send);
    free(mtimes);
    close_file(fd1);
    close_file(fd2);

    return 0;
}

void *calculation(void* param){
    int p = *((int *)(param));
    int **D,i,j;
    char buff[256];
    double total_t;
    int k;

    
    pthread_mutex_lock(&mutex);
    end_t = clock();
    total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
    sprintf(buff,"%s , Thread %d has reached the rendezvous point in %f seconds.\n",timestamp(),p,total_t);
    write(1,buff,strlen(buff));
    calculate_product(pow_num,(pow_num/N),pow_num,(pow_num/N)*(p-1));

    //free_array(D,pow_num);


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


    sprintf(buff,"%s , Thread %d is advancing to the second part\n",timestamp(),p);
    write(1,buff,strlen(buff));

    D = calloc(pow_num, sizeof(int*));
    for(i = 0; i < pow_num; i++){
        D[i] = calloc(pow_num/N, sizeof(int));
    }

    for(i = 0; i < pow_num; i++){
        k = 0;
        for(j=(pow_num/N)*(p-1); j < (pow_num/N)*p; j++){
            D[i][k] = C[i][j];
            k++;
        }
    }

    calculate_dft(pow_num/N,pow_num,0,D);
    //free_array(D,pow_num);

    end_t = clock();
    total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
    sprintf(buff,"%s , Thread %d has finished the second part in %f seconds.\n",timestamp(),p,total_t);
    write(1,buff,strlen(buff));

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

void calculate_product(int r1,int c2, int c1,int j1){
    int i,j,k;
    for(i=0;i<r1;i++){    
        for(j=0;j<c2;j++){      
            for(k=0; k < c1 ;k++){    
                C[i][j+j1] += A[i][k]*B[k][j+j1]; 
            }    
        }    
    }
 
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

void calculate_dft(int width,int height,int j1,int **D){
    float RE[height][width];
    float IM [height][width];
    int k = 0;
    for(int i=0;i<height;i++){
        for(int j=0;j<width;j++){
            float ak=0; 
            float bk=0;
            for(int ii=0;ii<height;ii++){
                for(int jj=0;jj<width;jj++){
                float x=-2.0*M_PI*(float)i*(float)ii/(float)height;
                float y=-2.0*M_PI*j*(jj+j1)/(float)width;
                ak+=D[ii][jj+j1]*cos(x+y);
                bk+=D[ii][jj + j1]*1.0*sin(x+y);
            }
        }
        RE[i][j]=ak;
        IM[i][j]=bk;
        }
    
    }
    for(k = 0;k< height;k++){
        for(int j=0 ; j< width; j++){
            fprintf(stdout," %f, %fi ",RE[k][j],IM[k][j]);
        }
        fprintf(stdout,"\n");
    }

}


char *timestamp(){
    time_t localTime;
    localTime=time(NULL);
    char *str = asctime( localtime(&localTime));
    char *removed = strchr(str, '\n');
    if(removed != NULL)
        removed[0] = '\0';
    return str;
}
