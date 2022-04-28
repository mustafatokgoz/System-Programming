#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>
#include "helper.h"

static char clientFifo[CLIENT_FIFO_NAME_LEN];

int client_operations(char *server_file, char *data_file);
char *timestamp();

sig_atomic_t sig_check=0;

/*to handle ctrl c*/
void signal_handle(int sig) {
    sig_check=1;
}

static void removeFifo(void){
    unlink(clientFifo);
}

void quit_c(){
  exitInf("\nYour program interrupt with ctrl c from keyboard exitinggg..\n");
}


int main(int argc, char *argv[]){
  char c;
  char *server_file;
  char *data_file;
  int check_input = 0;
  char *err_mass="Your should enter the correct command (i.e ./client -s pathToServerFifo -o pathToDataFile).\n";
  struct sigaction sa;


  sa.sa_handler = signal_handle;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags=0;
  sigaction(SIGINT,&sa,NULL);
  
  //printf("%s\n",time_stamp()); 
  while ((c = getopt (argc, argv, "s:o:")) != -1){
      switch (c)
        {
        case 's':
          server_file = optarg;
          check_input++;
          break;
        case 'o':
          data_file = optarg;
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
    int res;
    char buff[256];
    time_t start,end;
    double time_diff = 0.0;

    start = time(NULL);
    if(sig_check == 1){
      quit_c();
    }
    res = client_operations(server_file,data_file);
  
    if(sig_check == 1){
      quit_c();
    }

    end = time(NULL);
    time_diff = (double)end - (double)start;
   if(res == 0)
      sprintf(buff,"%s , Client #%d: the matrix IS NOT invertible, total time %.2f second, goodbye \n",timestamp(),getpid(),time_diff);
    else{
      sprintf(buff,"%s , Client #%d: the matrix is invertible, total time %.2f second, goodbye \n",timestamp(),getpid(),time_diff);
    }  
    write(1,buff,strlen(buff));
    return 0;
  
  
}

int open_file(char *filename){
    int fd = open (filename, O_RDWR);
    if (fd == -1) {
        exitInf("file error");
    }
    return fd;
}

void read_from_file(int *arr, int *n, char *data_file,int flag){
  int fd,rd,i=0,j=0,k=0;
  char c;
  char *content;
  fd = open_file(data_file);

  content = malloc(1 * sizeof(char));
  rd = read(fd,&c,1);
  if (rd <= 0){
    exitInf("Output Read Error");
  }
  content[0] = c;
  i++;
  while(rd > 0){
      content = realloc(content ,(i+1) * sizeof(char));
      rd = read(fd,&c,1);
      if(c == ','){
           if(flag == 0){
             arr = realloc(arr, (j+1) * sizeof(int));
           }
           content[i] ='\0';
           if(i > 0){
               arr[j] = atoi(content);
           }
           i=0;
           j++;
           k++;
           *n = k+1;
      }
      else if (c != '\n'){
        content[i] = c;
        i++;
      }
      else{
        if(c == '\n'){
          if(flag == 0){
             arr = realloc(arr, (j+1) * sizeof(int));
          }
          content[i] ='\0';
          if (i > 0){
              arr[j] = atoi(content);
          }    
          i=0;
          j++;
          k=0;
        }
        i=0;
      }
  }
  if(flag == 0){
    free(arr);
  }
  free(content);
  close(fd);

}

char *timestamp()
{
    time_t now;
    now = time(NULL);
    char *tstr = ctime(&now);
    char *line = strchr(tstr, '\n');
    line[0] = '\0';
    return tstr;
}


int client_operations(char *server_file, char* data_file){
  int serverFd, clientFd;
  struct request req;
  struct response resp;
  int n = 0;
  int *tarr ;
  char buff[256];

  tarr = malloc(1 * sizeof(int));
  read_from_file(tarr,&n,data_file,0);
  int arr[n*n];

  read_from_file(arr,&n,data_file,1);

  sprintf(buff, "%s , Client #%d (%s) is submitting a %dx%d matrix\n",timestamp(),getpid(),data_file,n,n);
  write(1,buff,strlen(buff));
  if(sig_check == 1){
      free(tarr);
      quit_c();
  }

  umask(0);


  snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE,(long) getpid());
  if (mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST) 
      exitInf("Fifo error for client");
  if (atexit(removeFifo) != 0) 
      exitInf("atexit");

  req.pid = getpid();
  req.seqLen = n;


  serverFd = open(server_file, O_WRONLY); 
  if (serverFd == -1){
    exitInf("Server fifo open error");
  }

  if(sig_check == 1){
      close(serverFd);
      quit_c();
  }
  

  if (write(serverFd, &req, sizeof(struct request)) != sizeof(struct request)){
      exitInf("Can't write to server");
  }
    if (write(serverFd, &arr, n*n * sizeof(int)) != (n *n * sizeof(int))){
        exitInf("Can't write to server");
    }

  clientFd = open(clientFifo, O_RDONLY); 
  if (clientFd == -1){
    exitInf("Fifo error open for client");
  }
  if (read(clientFd, &resp, sizeof(struct response)) != sizeof(struct response)){
    exitInf("Can't read response from server");
  }
  if(sig_check == 1){
      close(serverFd);
      close(clientFd);
      quit_c();
  }

  close(serverFd);
  close(clientFd);
  return resp.is_invertible;

}





