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
#include <errno.h>

#include "networking.h"
#include "helper.h"
#include "queue.h"

#define MAX 1024

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
servants_ports servant_port[1024];
int number_of_ports = 0;
queue *connection_queue = NULL;
void thread_pool();
void *server_thread(void* param);
char *timestamp();

sig_atomic_t sig_check=0;

/*to handle ctrl c*/
void signal_handle(int sig) {
    sig_check=1;
}


int main(int argc, char*argv[]){
    char ch;
    //int n=0,m;
    int check_input = 0;
    int port,t;
    int sockfd;
    char *err_mass = "You should enter the correct command (Run Way: ./server -p PORT -t numberOfThreads)\n";
    struct sockaddr_in newAddr;
    int newfd,i;
    //char buff[1024];

    struct sigaction sa;

    sa.sa_handler = signal_handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;
    sigaction(SIGINT,&sa,NULL);


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
    //int res;

    ntimes = malloc(t * sizeof(pthread_t));
    sendparam = malloc(t * sizeof(int));
    N = t;
    connection_queue = create_queue(); 
    thread_pool();

    while(1){

        if(sig_check == 1){
            for(i = 0;i < number_of_ports; i++){
                kill(servant_port[i].pid,SIGINT);
            }
            break;
        }
        socklen_t addr_size = sizeof(struct sockaddr_in);
        if ((newfd = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size)) == -1){
                if(errno == EINTR)
                    continue; // try again
                if(sig_check == 1){
                    for(i = 0;i < number_of_ports; i++){
                        kill(servant_port[i].pid,SIGINT);
                    }
                break;
                }
                exitInf("accept error");
        }    
        
        pthread_mutex_lock(&mutex2);
        while (active == N) { // if everyone is busy, wait
            
            pthread_cond_wait(&cond2,&mutex2);
        }
        pthread_mutex_unlock(&mutex2);
        pthread_mutex_lock(&mutex1);
        add_rear(connection_queue,newfd); 
        pthread_mutex_unlock(&mutex1);

        pthread_cond_signal(&cond1); // thread göndermeye başlıcak
        if(sig_check == 1){
            for(i = 0;i < number_of_ports; i++){
                kill(servant_port[i].pid,SIGINT);
            }
            break;
        }
        /*
            read(newfd,buff,1024);
            printf("buff : %s\n ",buff);

            
            
            if(write(newfd,"1",1) ==-1){
                perror("Heeyyy");
            }

        */
    }
    close(sockfd);
    free(ntimes);
    free(sendparam);
    return 0;

}

void thread_pool(){
    for (int i = 0; i < N; ++i) {
        sendparam[i] = i;
        pthread_create(&ntimes[i], NULL, server_thread,&sendparam[i]);
    }
}


void *server_thread(void* param){
    //int p = *((int *)(param));
    char buff[MAX];
    int i;
    while(1){
        pthread_mutex_lock(&mutex1);
        while(isQueue_empty(connection_queue)) { // if no query just wait
            pthread_cond_wait(&cond1,&mutex1);
        }
        if (sig_check == 1){
            pthread_mutex_unlock(&mutex1);
            break;
        }
        pthread_mutex_lock(&mutex2);
        active++;
        pthread_mutex_unlock(&mutex2);

        int current_fd = remove_front(connection_queue);
        pthread_mutex_unlock(&mutex1);

        pthread_mutex_lock(&mutex3);

        read(current_fd,buff,MAX);
        if (sig_check == 1){
            pthread_mutex_unlock(&mutex3);
            break;
        }
        if(strncmp(buff,"transactionCount",strlen("transactionCount"))==0){
            printf("%s , Request arrived “%s”\n",timestamp(),buff);
            char temp_buff[MAX];
            strcpy(temp_buff,buff);
            char *token = strtok(buff," ");
            char *type = strtok(NULL," ");
            char *date1 = strtok(NULL," ");
            char *date2 = strtok(NULL," ");
            char *city = strtok(NULL," ");
            if(token == NULL || type == NULL || date1 == NULL || date2 == NULL){
                exitInf("request Error");
            }
            int fd_new;

            if(city == NULL){
                int total = 0;
                int cur_res;
                char res[MAX];
                printf("%s , Contacting ALL servants\n",timestamp());
                for(i = 0; i < number_of_ports; i++){
                    fd_new = client_to_server_connect("127.0.0.1",servant_port[i].port);
                    write(fd_new,temp_buff,strlen(temp_buff)+1);
                    read(fd_new,res,MAX);
                    cur_res = atoi(res);
                    total = total + cur_res;

                }
                sprintf(res,"%d",total);
                if(write(current_fd,res,strlen(res)+1) ==-1){
                    exitInf("Write Error");
                }
                printf("%s , Response received: %d, forwarded to client\n",timestamp(),total);
            }
            else{
                int index = 0;
                int check = 0;
                for(i = 0; i < number_of_ports; i++){
                    if(strcmp(city,servant_port[i].city1) >= 0 && strcmp(city,servant_port[i].city2) <= 0){
                        index = i;
                        check = 1;
                    }
                }
                if (check == 1){
                    printf("%s , Contacting servant %d\n",timestamp(),servant_port[index].pid);
                    fd_new = client_to_server_connect("127.0.0.1",servant_port[index].port);
                    write(fd_new,temp_buff,strlen(temp_buff)+1);
                    char res[MAX];
                    read(fd_new,res,MAX);
                    if(write(current_fd,res,strlen(res)+1) ==-1){
                        exitInf("Write Error");
                    }
                    printf("%s , Response received: %d, forwarded to client\n",timestamp(),atoi(res));
                }
                else{
                    char res[MAX];
                    strcpy(res,"ERROR");
                    if(write(current_fd,res,strlen(res)+1) ==-1){
                            exitInf("Write Error");
                    }
                    printf("%s , Response received: %s, forwarded to client\n",timestamp(),res);
                }
            }
            close(fd_new);
            if (sig_check == 1){
                pthread_mutex_unlock(&mutex3);
                break;
            }

        }
        else{
            char *token = strtok(buff," ");
            servant_port[number_of_ports].pid = atoi(token);
            token = strtok(NULL," ");

            servant_port[number_of_ports].port = atoi(token);
            if(servant_port[number_of_ports].port == 0){
                exitInf("wrong port number");
            }
            token = strtok(NULL," ");
            strcpy(servant_port[number_of_ports].city1,token);
            token = strtok(NULL," ");
            strcpy(servant_port[number_of_ports].city2,token);
            number_of_ports++;
            if(number_of_ports == 1024){
                exitInf("Maximum servant number exceeded");
            }
            printf("%s , Servant %d present at port %d handling cities %s-%s\n",timestamp(),servant_port[number_of_ports-1].pid,servant_port[number_of_ports-1].port,
                        servant_port[number_of_ports-1].city1,servant_port[number_of_ports-1].city2);

            
        }
        close(current_fd);
        
        
        pthread_mutex_unlock(&mutex3);

        pthread_mutex_lock(&mutex2);
        active--;
        pthread_mutex_unlock(&mutex2);
        pthread_cond_signal(&cond2);
       

    }

    return NULL;
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
