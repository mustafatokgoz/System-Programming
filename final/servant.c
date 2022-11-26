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
#include <dirent.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "helper.h"
#include "utility.h"
#include "bst_for_files.h"
#include "networking.h"

#define MAX 1024
void *connection_thread(void* param);
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread[MAX];
int t_count = 0;
node* root_thread;

pid_t get_pid_from_proc_self () {
    char target[32];
    int pid;
    int len = 0;
    
    len = readlink ("/proc/self", target, sizeof (target));
    target[len]='\0';
    sscanf (target, "%d", &pid);
    return (pid_t) pid;
}


sig_atomic_t sig_check=0;

/*to handle ctrl c*/
void signal_handle(int sig) {
    sig_check=1;
}


int main(int argc, char*argv[]){
    char ch;
    char *director_path,*c_param,*ip;
    int port;
    int check_input = 0;
    char *err_mass = "You should enter the correct command (Run Way: ./servant -d directoryPath -c 10-19 -r IP -p PORT)\n";
    int i;
    char buff[256];
    void *ret=NULL;
    int client;
    int low_bound,up_bound;

    struct sigaction sa;

    sa.sa_handler = signal_handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;
    sigaction(SIGINT,&sa,NULL);
    
    while ((ch = getopt (argc, argv, "d:c:r:p:")) != -1){
      switch (ch){
        case 'd':
            director_path = optarg;
            check_input++;
            break;
        case 'c':
            c_param = optarg;
            check_input++;
            break;
        case 'r':
            ip = optarg;
            check_input++;
            break;
        case 'p':
            port = atoi(optarg);
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
    if(check_input != 4){
        write(2,err_mass,strlen(err_mass));
        exit(0);
    }
    if(port < 2000 && port > 65535){
        write(2,"You should enter port number bigger than 2000 and less than 65535\n",strlen("You should enter port number bigger than 2000 and less than 65535\n"));
        exit(0);
    }
    if (seperate_c_paramater(&low_bound,&up_bound,c_param) == -1){
        write(2,err_mass,strlen(err_mass));
        exit(0);
    }

    node* root = NULL;
    char city1[70],city2[70];
    int pid = get_pid_from_proc_self();
    
    root = read_from_disk(director_path,low_bound,up_bound,root,city1,city2);
    root_thread = root;

    printf("Servant %d: loaded dataset, cities %s-%s\n",pid,city1,city2);
    

    int len;

    int unique_port;
    int server;
    
    unique_port = (pid%31250) * 2 + 3002;
    if(unique_port == port){
        unique_port = unique_port - 1;
    }

    len = sprintf(buff,"%d %d %s %s",pid,unique_port,city1,city2);
    buff[len] = '\0';
    

    client = client_to_server_connect(ip,port);

    server = server_socketfd(unique_port);
    printf("Servant %d: listening at port %d\n",pid,unique_port);
    write(client,buff,strlen(buff)+1);


    
    int newfd;
    struct sockaddr_in newAddr;

    while(1){
        if(sig_check == 1){
            pthread_mutex_lock(&mutex1);
            for(i = 0; i< t_count ;i++){
                if (!pthread_equal(pthread_self(), thread[i])){
                    pthread_join(thread[i],ret);
                }                
            }
            pthread_mutex_unlock(&mutex1);
            printf("Servant %d: termination message received, handled %d requests in total.\n",pid,t_count);

            break;
        }
        socklen_t addr_size = sizeof(struct sockaddr_in);
        if ((newfd = accept(server, (struct sockaddr *)&newAddr, &addr_size)) == -1){
                if(sig_check == 1){
                    pthread_mutex_lock(&mutex1);
                    for(i = 0; i< t_count ;i++){
                        if (!pthread_equal(pthread_self(), thread[i])){
                            pthread_join(thread[i],ret);
                        }  
                    }
                    pthread_mutex_unlock(&mutex1);
                    printf("Servant %d: termination message received, handled %d requests in total.\n",pid,t_count);
                    break;
                }
                if(errno == EINTR)
                    continue; // try again
                
            exitInf("accept error");

        }    
        pthread_create(&thread[t_count],NULL,connection_thread,&newfd);
    }


    
    free_tree(root);

    return 0;

}    


void *connection_thread(void* param){

    int newfd = *((int *)(param));
    char buff2[MAX];
    pthread_mutex_lock(&mutex1);
    read(newfd,buff2,MAX);
    char *token = strtok(buff2," ");
    char *type = strtok(NULL," ");
    char *date1 = strtok(NULL," ");
    char *date2 = strtok(NULL," ");
    char *city = strtok(NULL," ");
    if(token == NULL){
         exitInf("Request error typing");
    }
    if(sig_check == 1){
        pthread_mutex_unlock(&mutex1);
        return NULL;
    }
    int count3 = 0;
    char result[10];
    if(city == NULL){
        search(root_thread,date1,date2,type,"",0,&count3);
        sprintf(result,"%d",count3);
        write(newfd,result,strlen(result)+1);
    }
    else{
        search(root_thread,date1,date2,type,city,1,&count3);
        sprintf(result,"%d",count3);
        write(newfd,result,strlen(result)+1);
    }
    t_count++;
    close(newfd);
    if(sig_check == 1){
        pthread_mutex_unlock(&mutex1);
        return NULL;
    }
    pthread_mutex_unlock(&mutex1);

    return NULL;
}    



