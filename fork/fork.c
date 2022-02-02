#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include "helper.h"

int main(){
    int i = 0;
    char *child = "Child process \n";
    char *parent = "Parent process \n";
    int cur_pid;
    char pid[1024];
    for(i = 0; i < 5; i++){
        switch(fork()){
            case -1:
                errExit("Fork error");
            case 0:
                write(1, child, strlen(child));
                cur_pid = getpid();
                sprintf(pid, "%d", cur_pid);
                write(1, pid , strlen(pid));
                exitInf("Success");
            default:
                write(1, parent, strlen(parent));
                cur_pid = getpid();
                sprintf(pid, "%d", cur_pid);
                write(1, pid, strlen(pid));
                break;    
        }
    }

    return 0;

}