#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "helper.h"


void file_operations(char **arr, int count, char* file_name){
    int fd;
    char c;
    int rd;
    fd = create_file(file_name);

    rd = read(fd, &c, 1);
    while(c!='\n' && rd > 0){
        write(1,&c,1);
        rd = read(fd, &c, 1);
    }
    if(lseek(fd,0,SEEK_SET) == -1)
    {
        exitInf(errno);
    }
    write(fd,"deneme123",len("deneme123"));
    close(fd);
    fd=open(file_name,O_RDWR | O_TRUNC);
    if(lseek(fd,0,SEEK_SET) == -1)
    {
        exitInf(errno);
    }
    write(fd,"deneme123",len("deneme123"));

    if(lseek(fd,0,SEEK_SET) == -1)
    {
        exitInf(errno);
    }
    rd = read(fd, &c, 1);
    while(c!='\n' && rd > 0){
        write(1,&c,1);
        rd = read(fd, &c, 1);
    }
    write(1,"\n",1);

    close(fd);

}