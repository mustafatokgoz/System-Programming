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

    socklen_t addr_size = sizeof(struct sockaddr_in);
    if ((newfd = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size)) == -1)
        exitInf("accept error");

    while(read(newfd,buff,1024)==1024);
    printf("buff : %s \n",buff);
    


    return 0;

}

