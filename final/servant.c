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


int main(int argc, char*argv[]){
    char ch;
    int n,m;
    char *director_path,*c_param,*ip;
    int port;
    int check_input = 0;
    char *err_mass = "You should enter the correct command (Run Way: ./servant -d directoryPath -c 10-19 -r IP -p PORT)\n";
    int i,j;
    char buff[256];
    void *ret;
    int low_bound,up_bound;
    
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
    if(port < 2000){
        write(2,"You should enter port number bigger than 2000\n",strlen("You should enter port number bigger than 2000\n"));
        exit(0);
    }
    if (seperate_c_paramater(&low_bound,&up_bound,c_param) == -1){
        write(2,err_mass,strlen(err_mass));
        exit(0);
    }

    printf("%s %s %d %d %d \n",director_path,ip,port,low_bound,up_bound);
    return 0;

}    


