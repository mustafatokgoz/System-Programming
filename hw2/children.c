#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>


int main(int argc, char *argv[]){
    if (argc == 2){
        write(1,argv[1],strlen(argv[1]));
    }    
    else{
         write(1,"wrong input arr\n",strlen("wrong input arr\n"));
    }
    return 0;
}