#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

void lock_file(int fd, struct flock fl);
void unlock_file(int fd, struct flock fl);
int open_file(char *filename);

extern char **environ;

int main(int argc, char *argv[]){
    char **ep,*temp;
    int fd;
    int result = 0;
    char buffer[30];
    int length = 0;
    int i = 0,j = 0;
    struct flock fl = { F_WRLCK, SEEK_SET, 0,       0,     0 };
    if (argc == 1){
        fd = open_file(argv[0]);
        i = 0;
        for (ep = environ; *ep != NULL; ep++){
            write(1,*ep,strlen(*ep));
            temp = *ep;
            for(i = 0;i < 4; i++){
                write(1,&temp[i],1);
                result += (int) temp[i];
            }
            write(1,"\n",1);
        
        }
        write(1,"\n",1);
        lock_file(fd,fl);
        length = sprintf(buffer, "%d", result);
        write(fd,buffer,length);
        write(fd,"\n",1);
        unlock_file(fd,fl);

    }    
    else{
         write(1,"wrong input arr\n",strlen("wrong input arr\n"));
    }
    


    return 0;
}


int open_file(char *filename){
    int fd;
    fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        perror("file error");
        exit(0);
    }
    return fd;
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