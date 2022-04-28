#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include "helper.h"

int readfile_2_char(int,char*);
int create_chefs_and_pushers();



int main(int argc, char *argv[]){
    char *inputfile;
    char *name;
    int check_input=0;
    char c;
    char *err_mass="Your should enter the correct command (i.e ./hw3named -i inputFilePath -n name).\n";
    char array[2];
    int fd;

    while ((c = getopt (argc, argv, "i:n:")) != -1){
      switch (c){
        case 'i':
          inputfile = optarg;
          check_input++;
          break;
        case 'n':
          name = optarg;
          check_input++;
          break;
        case '?':
          write(2,"Something went wrong.\n",strlen("Something went wrong.\n")); 
          return 1;
        default:
          write(2,"Something went wrong.\n",strlen("Something went wrong.\n"));  
          return 1;
        }
    }

    if(check_input != 2){
        write(2,err_mass,strlen(err_mass));
        exit(0);
    }

    fd = open_file(inputfile);




    while(readfile_2_char(fd,array) != -1){
        printf(" 1. %c  2. %c \n",array[0],array[1]);
    }
    



    close_file(fd);

    return 0;
}







int readfile_2_char(int fd,char array[]){
    int rd;
    char c;
    rd = read(fd,&c,1);
    if (rd <= 0 || c == '\n'){
        return -1;
    }
    array[0] = c;
    rd = read(fd,&c,1);
    if (rd <= 0 || c == '\n'){
        return -1;
    }
    array[1] = c;

    rd = read(fd,&c,1);
    if(rd < 0){
        return -1;
    }
    if(rd > 0 && c != '\n'){
        return -1;
    }
    return 0;
}
