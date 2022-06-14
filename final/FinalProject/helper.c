#include "helper.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

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


int close_file(int fd){
    if (close(fd) == -1){
        exitInf("file close error");
    }
    return 0;
}




