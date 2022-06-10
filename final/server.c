#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "networking.h"
#include "helper.h"
#include "queue.h"


int N = 0;
pthread_t *ntimes;
int *sendparam;
int active = 0;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
queue *connection_queue = NULL;
void thread_pool();
void *server_thread(void* param);

int main(int argc, char*argv[]){
    char ch;
    int n=0,m;
    int check_input = 0;
    int port,t;
    int sockfd;
    char *err_mass = "You should enter the correct command (Run Way: ./server -p PORT -t numberOfThreads)\n";
    struct sockaddr_in newAddr;
    int newfd;
    char buff[1024];
    while ((ch = getopt (argc, argv, "p:t:")) != -1){
      switch (ch){
        case 'p':
            port = atoi(optarg);
            check_input++;
            break;
        case 't':
            t = atoi(optarg);
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
    if(check_input != 2){
        write(2,err_mass,strlen(err_mass));
        exit(0);
    }
    if(port < 2000 && port > 65535){
        write(2,"You should enter port number bigger than 2000 and less than 65535\n",strlen("You should enter port number bigger than 2000 and less than 65535\n"));
        exit(0);
    }

    sockfd = server_socketfd(port);
    int res;

    ntimes = malloc(t * sizeof(pthread_t));
    sendparam = malloc(t * sizeof(int));
    N = t;
    connection_queue = create_queue(); 
    thread_pool();

    while(1){
        socklen_t addr_size = sizeof(struct sockaddr_in);
        if ((newfd = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size)) == -1)
            exitInf("accept error");
        
        pthread_mutex_lock(&mutex2);
        while (active == N) { // if everyone is busy, wait
            
            pthread_cond_wait(&cond2,&mutex2);
        }
        pthread_mutex_unlock(&mutex2);
        pthread_mutex_lock(&mutex1);
        add_rear(connection_queue,newfd); 
        pthread_mutex_unlock(&mutex1);

        pthread_cond_signal(&cond1); // thread göndermeye başlıcak


        /*
            read(newfd,buff,1024);
            printf("buff : %s\n ",buff);

            
            
            if(write(newfd,"1",1) ==-1){
                perror("Heeyyy");
            }

        */
    }

    return 0;

}

void thread_pool(){
    for (int i = 0; i < N; ++i) {
        sendparam[i] = i;
        pthread_create(&ntimes[i], NULL, server_thread,&sendparam[i]);
    }
}


void *server_thread(void* param){
    int p = *((int *)(param));
    char buff[1024];
    while(1){
        pthread_mutex_lock(&mutex1);
        while(isQueue_empty(connection_queue)) { // if no query just wait
            pthread_cond_wait(&cond1,&mutex1);
        }
        //if (exitSignal){
        //    pthread_mutex_unlock(&taskMutex);
        //    break;
        //}
        pthread_mutex_lock(&mutex2);
        active++;
        pthread_mutex_unlock(&mutex2);

        int current_fd = remove_front(connection_queue);
        pthread_mutex_unlock(&mutex1);

        read(current_fd,buff,1024);
        printf("gelen pid port ve şehirler %s \n",buff);
        if(strncmp(buff,"transactionCount",strlen("transactionCount"))==0){
            printf("This is client \n");
        }
        else{
            printf("This is servant");
        }
        if(write(current_fd,"1",1) ==-1){
                perror("Heeyyy");
        }

        pthread_mutex_lock(&mutex2);
        active--;
        pthread_mutex_unlock(&mutex2);
        pthread_cond_signal(&cond2);

    }

    return NULL;
}    
