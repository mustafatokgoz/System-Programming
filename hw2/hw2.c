#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "helper.h"

int create_child_process(char *arr[],int n,int m);




int main(int argc, char *argv[]){
    char *arr[] = {"jim", "jams", NULL };
    create_child_process(arr,5,2);
    return 0;
}


int create_child_process(char *arr[],int n,int m){
    int status;
    pid_t childPid[n];
    int i = 0;
    int j = 0;
    write(1,"\nThis is parent side \n",strlen("\nThis is parent side \n"));
    for(i = 0; i < n ; i++){
        switch (childPid[i] = fork()) { 
            case -1: 
                write(1,"error",strlen("error"));
                return -1;
                    /* process creatiotempn error */  
            case 0: /* Child */
                write(1,"tttt\n",strlen("tttt\n"));
                execve("./deneme123",arr,NULL);
                _exit(127); /* Failed exec; not supposed to reach this line */
            default: /* Parent */
                break;
               
        }
    }


     //while(wait(NULL)!=-1);
     for(j = 0; j < n; j++){
        if (waitpid(childPid[j], &status, 0) == -1){  
            return -1; 
        }      
    }
    write(1,"\nThis is parent side \n",strlen("\nThis is parent side \n"));
     write(1,"\nThis is parent side \n",strlen("\nThis is parent side \n"));
}






