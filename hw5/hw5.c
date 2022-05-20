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
double **writefile,**writefile2;
int pow_num = 0;
int arrived = 0;
int N = 0;
int fd1 = -1,fd2 = -1,out=-1;
int *send;
clock_t start_t, end_t;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int power_of_2(int num);
int read_files_into_memory(int fd1, int fd2);
void free_array(int **arr, int n);
void free_array2(double **arr, int n);
void *calculation(void* param);
void calculate_product(int r1,int c2,int c1, int j1);
char *timestamp();
void quit_c();
void calculate_dft(int width,int height,int j1,double **D);
void lock_file(int fd, struct flock fl);
void unlock_file(int fd, struct flock fl);

sig_atomic_t sig_check=0;

/*to handle ctrl c*/
void signal_handle(int sig) {
    int i;
    sig_check = 1;
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
    if (out != -1)    
        close(fd2);    
    if(send!=NULL){    
        for(i = 0;i < N; i++){
            pthread_cancel(mtimes[i]);
        }
        free(send);
    }    
    if(mtimes!=NULL)  
        free(mtimes);   
    if(writefile!= NULL){
        free_array2(writefile2,pow_num);
    }   
    if(writefile2!= NULL){
        free_array2(writefile2,pow_num);
    }    
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
    int k,l;
    struct sigaction sa;
    time_t e_time;
    double total_t;

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

    writefile2 = calloc(pow_num, sizeof(double*));
    for(i = 0; i < pow_num; i++){
        writefile2[i] = calloc(pow_num * 2, sizeof(double));
    }


    
    fd1 = open_file(inputfile1);
    fd2 = open_file(inputfile2);
    out = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out == -1){
        exitInf("Log file doesn't open\n");
    }

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
        writefile = (double **) ret;
        for(k = 0;k< pow_num;k++){
            for(l=0 ; l < pow_num/N; l++){
                    writefile2[k][l + (pow_num/N ) * j] = writefile[k][l];
                    writefile2[k][l + (pow_num/N ) * j + 1] = writefile[k][l+1];
            }
        }
        free_array2(writefile,pow_num);
    }

    for(k = 0; k < pow_num; k++){
        for(l = 0; l < pow_num; l++){
            if(l == pow_num-1){
                sprintf(buff,"%f + %fj",writefile2[k][l],writefile2[k][l+1]);
                write(out,buff,strlen(buff));
            }
            else{
                sprintf(buff,"%f + %fj, ",writefile2[k][l],writefile2[k][l+1]);
                write(out,buff,strlen(buff));
            }
        }
        write(out,"\n",1);
    }

    free_array2(writefile2,pow_num);

    e_time = clock();

    total_t = (double)(e_time - start_t) / CLOCKS_PER_SEC;
    sprintf(buff,"%s , The process has written the output file. The total time spent is %.3f seconds\n",timestamp(),total_t);
    write(1,buff,strlen(buff));

    /*
  
    for(i = 0; i < pow_num; i++){
        for(j = 0; j < pow_num; j++){
            printf(" %d ",C[i][j]);
        }
        printf("\n");
    }
    */


    free_array(A,pow_num);
    free_array(B,pow_num);
    free_array(C,pow_num);
    free(send);
    free(mtimes);
    close_file(fd1);
    close_file(fd2);
    close_file(out);

    return 0;
}

void *calculation(void* param){
    int p = *((int *)(param));
    double **D;
    int i;
    char buff[256];
    double total_t;
    time_t e_time;

    
    pthread_mutex_lock(&mutex);
    e_time = clock();
    total_t = (double)(e_time - start_t) / CLOCKS_PER_SEC;
    sprintf(buff,"%s , Thread %d has reached the rendezvous point in %.4f seconds.\n",timestamp(),p,total_t);
    write(1,buff,strlen(buff));
    calculate_product(pow_num,(pow_num/N),pow_num,(pow_num/N)*(p-1));

    
    ++arrived;
    for(;;){
        if(arrived < N){
            pthread_cond_wait(&cond,&mutex);
        }    
        else{
            pthread_cond_broadcast(&cond);
            sprintf(buff,"%s , Thread %d is advancing to the second part\n",timestamp(),p);
            write(1,buff,strlen(buff));
            break;
        }    
    }



    D = calloc(pow_num, sizeof(double*));
    for(i = 0; i < pow_num; i++){
        D[i] = calloc((pow_num/N)*2, sizeof(double));
    }

    calculate_dft(pow_num/N,pow_num,(pow_num/N)*(p-1),D);

    e_time = clock();
    total_t = (double)(e_time - start_t) / CLOCKS_PER_SEC;
    sprintf(buff,"%s , Thread %d has finished the second part in %.4f seconds.\n",timestamp(),p,total_t);
    write(1,buff,strlen(buff));

    pthread_mutex_unlock(&mutex);

    pthread_exit(D);
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

void free_array2(double **arr, int n){
  int i = 0;
  for ( i = 0; i < n; i++ ){
        free(arr[i]);
  }
  free(arr);
}

void calculate_dft(int width,int height,int j1,double **D){
    float RE[height][width];
    float IM [height][width];
    int k = 0,i,j,i2,j2;
    float an,bn,x,y;
    for(i=0;i<height;i++){
        for(j=0;j<width;j++){
            an=0; 
            bn=0;
            for(i2=0;i2<height;i2++){
                for(j2=0;j2<width;j2++){
                    x=-2.0*M_PI*i*i2/(float)height;
                    y=-2.0*M_PI*(j)*(j2)/(float)width;
                    an+=C[i2][j2]*cos(x+y);
                    bn+=C[i2][j2]*sin(x+y);
                }
            }
            RE[i][j]=an;
            IM[i][j]=bn;
        }
    }

    for(k = 0;k< height;k++){
        for(int j=0 ; j< width; j++){
            D[k][j] =  RE[k][j];
            D[k][j+1] = IM[k][j];
        }
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




char *timestamp(){
    time_t localTime;
    localTime=time(NULL);
    char *str = asctime( localtime(&localTime));
    char *removed = strchr(str, '\n');
    if(removed != NULL)
        removed[0] = '\0';
    return str;
}
