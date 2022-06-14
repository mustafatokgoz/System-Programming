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


#include "helper.h"
#include "utility.h"
#include "bst_for_files.h"
#include "networking.h"


void *request_thread(void* param);
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int arrived = 0;
int N = 0;
pthread_t *ntimes;
int *send;
char **requests;
char *request_path,*ip;
int port;

int main(int argc, char*argv[]){
    char ch;
    int n=0;
    int check_input = 0;
    char *err_mass = "You should enter the correct command (Run Way: ./client -r requestFile -q PORT -s IP)\n";
    int i,j;
    //char buff[256];
    void *ret;
    //int low_bound,up_bound;
    
    while ((ch = getopt (argc, argv, "r:q:s:")) != -1){
      switch (ch){
        case 'r':
            request_path = optarg;
            check_input++;
            break;
        case 'q':
            port = atoi(optarg);
            check_input++;
            break;     
        case 's':
            ip = optarg;
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
    if(port < 2000 && port > 65535){
        write(2,"You should enter port number bigger than 2000 and less than 65535\n",strlen("You should enter port number bigger than 2000 and less than 65535\n"));
        exit(0);
    }
    requests = get_requests(request_path,&n);

    if(requests == NULL){
        exitInf("File error");
    }


    //for(i = 0; i < n; i++){
    //    printf("%s \n",requests[i]);
    //}


    ntimes = malloc(n * sizeof(pthread_t));
    send = malloc(n * sizeof(int));
    N = n;
    printf("Client: I have loaded %d requests and I’m creating %d threads.\n",n,n);
    for(i = 0; i < n;i++){
        send[i] = i;
        pthread_create(&ntimes[i],NULL,request_thread,&send[i]);
    }

    for(j = 0; j < n; j++){
        pthread_join(ntimes[j], &ret);
    }
    printf("Client: All threads have terminated, goodbye\n");

    free_array2(requests,n);
    free(ntimes);
    free(send);


}

void *request_thread(void* param){
    int p = *((int *)(param));
    int client;
    char buff[10];

    pthread_mutex_lock(&mutex);

    printf("Client-Thread-%d: Thread-%d has been created\n",p,p);

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
    printf("Client-Thread-%d: I am requesting “/%s”\n",p,requests[p]);
    char req[strlen(requests[p])+1];
    int len = strlen(requests[p]);
    char respon[1024];

    sprintf(buff,"%d",len);
    strcpy(req,requests[p]);
    //req[len] ='\0';
    client = client_to_server_connect(ip,port);

    write(client,req,strlen(req)+1);
    
    if(read(client,respon,1024) == -1){
        perror("read error");
    }
    printf("Client-Thread-%d: The server’s response to “/%s” is %s\n",p,requests[p],respon);
    printf("Client-Thread-%d: Terminating\n",p);

    pthread_mutex_unlock(&mutex);
    return NULL;
}    

