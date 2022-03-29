#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include "helper.h"

int create_child_process(char **arr,char **num);
int read_coordinates(char **num,int n, int fd);
void lock_file(int fd, struct flock fl);
void unlock_file(int fd, struct flock fl);
void free_array(char **arr,int n);
void read_file_from_output(char *outputfilename, int n);
void quit_signal_c();

sig_atomic_t sig_check=0;

/*to handle ctrl c*/
void signal_handle(int sig) {
    sig_check=1;
}


int main(int argc, char *argv[]){
    char **arr;
    char **num;
    char c;
    char *filename;
    char *outputfile;
    int check_input = 0;
    char *err_mass="Your should enter the correct command (i.e ./processP -i inputFilePath -o outputFilePath ).\n";
    struct sigaction sa;
    int fd,ofd;
    int i = 0,count = 0, j = 0;
    int length;
    char buffer[256];


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

    ofd = open(outputfile, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (ofd == -1) {
        perror("file error");
        exit(0);
    }

    if (ftruncate(ofd, 0) < 0){
            errExit("ftruncate error");
    }

    close(ofd);


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

    write(1,"Process P reading ",strlen("Process P reading "));
    write(1,filename,strlen(filename));
    write(1,"\n",1);
    while (read_coordinates(num,10,fd) == 0){
      write(1,"Created R_", strlen("Created R_"));
      length = sprintf(buffer, "%d with ",count);
      write(1,buffer,length);

      for(i = 0; i < 10; i++){
        length = sprintf(buffer, "(%c,%c,%c)",num[i][0],num[i][1],num[i][2]);
        write(1,buffer,length);
        if(i != 9){
          write(1,",",1);
        }
      }

      create_child_process(arr,num);
      count++;
      if(sig_check == 1){
        free_array(arr,2);
        free_array(num,11);
        quit_signal_c();
      }
      write(1,"\n",1);
    }

    
    while(wait(NULL)!=-1);

    if(sig_check==1){
      free_array(arr,2);
      free_array(num,11);
      quit_signal_c();

    }  
    write(1,"Reached EOF, collecting outputs from ", strlen("Reached EOF, collecting outputs from "));
    write(1,outputfile,strlen(outputfile));
    write(1,"\n",1);
    //write(1,"\nThis is parent side \n",strlen("\nThis is parent side \n"));
    

    free_array(arr,2);

    free_array(num,11);

    if(sig_check==1){
       quit_signal_c();
    }  
       
    read_file_from_output(outputfile,count);

    if(sig_check==1){
       quit_signal_c();
    }  
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

double calculate_norm(double *arr,int n){
  int i = 0;
  double sum = 0.0;
  double result = 0.0;
  for(i = 0; i < n; i++){
    sum += arr[i] * arr[i];
  }
  result = sqrt(sum); 
  return result;
}


void swap(double *first, double *second){
    double temp = *first;
    *first = *second;
    *second = temp;
}
 
void bubble_sort(double *arr, int n){
   int i = 0, j = 0;
   for (i = 0; i < n - 1; i++)      
       for (j = 0; j < n-i-1; j++)
           if (arr[j] > arr[j+1])
              swap(&arr[j], &arr[j+1]);
}
 


void find_closest_and_print(double *norm,int n){
    int i = 0,j = 0;
    int diff = 2147483647;
    int index1,index2;
    if ( n == 1){
      return;
    }
    bubble_sort(norm,n);
    for(i=0; i < n - 1 ;i++){
        if (fabs(norm[i] - norm[i+1]) < diff){
          diff = fabs(norm[i] - norm[i+1]);
          index1 = i;
          index2 = i+1;
        }
    }

    for(i = 0; i < n; i++){
      printf("\n norm i %.3f \n",norm[i]);
    }
    //write ile yaz
    //sprintfle double ları al ve sayırları buffera
    printf("The closest two matrices are R_%d and R_%d and their distance is %.3f",index1,index2,fabs(norm[index1]-norm[index2]));

}


void read_file_from_output(char *outputfilename, int n){
    int rd;
    int fd;
    char *content;
    char c;
    int i = 0, j = 0, k = 0;
    char buffer[256];
    double arr[9];
    double norm[n], res = 0.0;
    int n_num = 0;

    fd = open_file(outputfilename);
    content = malloc(1 * sizeof(char));
    rd = read(fd,&c,1);
    if (rd <= 0){
      exitInf("Output Read Error");
    }
    content[0] = c;
    i++;
    while(rd > 0 ){
      content = realloc(content ,(i+1) * sizeof(char));
      rd = read(fd,&c,1);
      if(c == ' '){
           arr[j] = atof(content);
           i=0;
           j++;
           if(j == 9){
             norm[k] = calculate_norm(arr,9);
             k++;
             j = 0;
           }
      }
      else if (c != '\n'){
        content[i] = c;
        i++;
      }
      else{
        i=0;
      }
    }

    find_closest_and_print(norm,n);
    free(content);
    close(fd);

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
          execve("./processR_i",arr,num);
          _exit(127); /* Failed exec; not supposed to reach this line */
      default: /* Parent */
          break;
               
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



