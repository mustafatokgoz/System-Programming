#include "helper.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int len(char *str){
    int i = 0;
    for(i = 0; str[i] !='\0'; i++);
    return i;
}

void errExit(char *inf){
    char *errInf = "Error was occured for given information : ";
    char *errInf2 ="Example usage : ./hw1 \'/^Window[sz]*/Linux/i;/close[dD]$/open/\' inputFilePath";
    write(2, errInf, len(errInf));
    write(2, inf, len(inf));    
    write(2, "\n", 1);
    write(2,errInf2, len(errInf2));
    write(2, "\n", 1);
    exit(0);
}


void exitInf(char *err){
    perror(err);
    exit(0);
}

int open_file(char *filename){
    int fd = open (filename, O_RDWR);
    if (fd == -1) {
        exitInf("file error");
    }
    return fd;
}

void set_to_begin(int fd){
    if(lseek(fd,0,SEEK_SET) == -1){
            exitInf("lseek error");
    }
}





