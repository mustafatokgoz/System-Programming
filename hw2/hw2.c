#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include "helper.h"

int create_child_process(char **arr,char **num);
int read_coordinates(char **num,int n, int fd);
void lock_file(int fd, struct flock fl);
void unlock_file(int fd, struct flock fl);
void free_array(char **arr,int n);
void quit_signal_c();

sig_atomic_t sig_check=0;

/*to handle ctrl c*/
void signal_handle(int sig) {
    sig_check=1;
}


int main(int argc, char *argv[]){
    char **arr;
    //char num[10][4];
    char **num;
    char c;
    char *filename;
    char *outputfile;
    int check_input = 0;
    char *err_mass="Your should enter the correct command (i.e ./processP -i inputFilePath -o outputFilePath ).\n";
    struct sigaction sa;
    int fd;
    int i = 0;


    sa.sa_handler = signal_handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;
    sigaction(SIGINT,&sa,NULL);

    while ((c = getopt (argc, argv, "i:o:")) != -1){
      switch (c)
        {
        case 'i':
          filename = optarg;
          check_input++;
          break;
        case 'o':
          outputfile = optarg;
          check_input++;
          break;
        case '?':
          write(2,"Something went wrong.\n",strlen("Something went wrong.\n")); 
          return 1;
        default:
          write(2,"Something went wrong.\n",strlen("Something went wrong.\n"));  
          return 1;
        }
        if(sig_check==1){
          write(2,"\nYour program was cut by keyboard\n",strlen("\nYour program was cut by keyboard\n"));
          exit(0);
        }  
    }
    
    if(check_input != 2){
        write(2,err_mass,strlen(err_mass));
        exit(0);
    }
    if(sig_check==1){
       quit_signal_c();
    }  

    fd = open_file(filename);



    num = malloc(11 * sizeof(char *));
    for(i = 0 ; i < 10; i++){
        num[i] = malloc(4 * sizeof(char *));
    }
    num[10] = NULL;



    arr = malloc(2 * sizeof(char *));
    arr[0] = malloc((strlen(outputfile)+1) * sizeof(char *));
    strcpy(arr[0],outputfile);
    arr[0][strlen(outputfile)]='\0';
    arr[1] = NULL;

    while (read_coordinates(num,10,fd) == 0){
      for(i = 0; i < 10; i++){
        write(1,num[i],strlen(num[i]));
        write(1,"\n",1);
      }
      create_child_process(arr,num);
      
      if(sig_check == 1){
        free_array(arr,2);
        free_array(num,11);
        quit_signal_c();
      }
      write(1,"\n",1);
    }
    
    while(wait(NULL)!=-1);

    write(1,"\nThis is parent side \n",strlen("\nThis is parent side \n"));

    free_array(arr,2);

    free_array(num,11);


    close(fd);
    return 0;
}

void quit_signal_c(){
  write(2,"\nYour program was cut by keyboard\n",strlen("\nYour program was cut by keyboard\n"));
  exit(0);
}

void free_array(char **arr, int n){
  int i = 0;
  for ( i = 0; i < n; i++ ){
        free(arr[i]);
  }
  free(arr);
}


int read_coordinates(char **num,int n,int fd){
  int rd;
  char coord[3];
  int many = 0;
  int i = 0;
  rd = read(fd, coord, 3);
  while(rd>0  && many < 10){
      for (i = 0; i < 3; i++){
        num[many][i]= coord[i];
      }
      num[many][3]='\0';
      if(many < 9)
        rd = read(fd, coord, 3);
      many++;
  }
  if (many != 10){
    return 1;
  }
  return 0;
}





int create_child_process(char **arr,char **num){
    int status;
    int i = 0;
    int j = 0;
    switch (fork()) { 
      case -1: 
          write(1,"error",strlen("error"));
          return -1;  
      case 0: /* Child */
          write(1,"tttt\n",strlen("tttt\n"));
          execve("./deneme123",arr,num);
          _exit(127); /* Failed exec; not supposed to reach this line */
      default: /* Parent */
          break;
               
    }
    


     //while(wait(NULL)!=-1);
     //for(j = 0; j < n; j++){
     //   if (waitpid(childPid[j], &status, 0) == -1){  
     //       return -1; 
     //   }      
      //}
}





void lock_file(int fd, struct flock fl){
    fl.l_type = F_WRLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1){
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



