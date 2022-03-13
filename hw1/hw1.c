#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "helper.h"


typedef enum { False, True } boolean;
void seperate_argumans(char **,int);
void file_operations(char **,int, char*);
int create_file(char *);
void change_occurance(int , char *, char *);


int main(int argc, char **argv){
    
    if(argc < 3 || argc > 3){
        errExit("Arguman error");
    }
    seperate_argumans(argv,argc - 1);
    return 0;

}

void change_occurance(int temp_fd, char *cur_occ, char *occ){
    int current_status = (len(cur_occ) -1) * -1;
    if(lseek(temp_fd,current_status,SEEK_END) == -1)
    {
        exitInf(errno);
    }
    write(temp_fd, occ, len(occ));
}

void file_operations(char **arr, int count, char *file_name){
    int fd,temp_fd;
    char c;
    int i;
    int rd;
    char temp_file[]="/tmp/my-temp-fileXXXXXX";
    boolean case_insesitive = False;
    int number_l = 0, check = 0, check2 = 0;

    fd = create_file(file_name);

    temp_fd=mkstemp(temp_file);

    unlink(temp_file);

    if (temp_fd < 1){
        exitInf(errno);
    }
    
    for(i = 0; i < 1; i = i + 2){
        if ((i+2) < count && len(arr[i+2]) == 1 && arr[i+2][0] == 'i'){
            case_insesitive = True;
        } 
        else{
            case_insesitive = False;
        }
        check = 0;
        check2 = 0;
        number_l = 0;
        rd = read(fd, &c, 1);
        while(rd > 0){
            if (c == arr[i][check]){
                number_l ++;
                check ++;
            }
            else{
                if(number_l == 1){
                    check2 = 1;
                }
                number_l = 0;
                check = 0;
            }
            if (number_l == len(arr[i])){
                change_occurance(temp_fd,arr[i],arr[i+1]);
            }
            else{
                if(check2 == 0) {
                    write(temp_fd, &c, 1);
                }
            }
            if(check2 == 0) {
                rd = read(fd, &c, 1);
            }
            check2 = 0;
        }
        close(fd);
        fd=open(file_name,O_RDWR | O_TRUNC);
        if(lseek(fd,0,SEEK_SET) == -1)
        {
            exitInf(errno);
        }
        if(lseek(temp_fd,0,SEEK_SET) == -1)
        {
            exitInf(errno);
        }
        rd = read(temp_fd, &c, 1);
        while(rd > 0){
            write(fd,&c,1);
            rd = read(temp_fd, &c, 1);
        }

        if (case_insesitive == True){
            i++;
        }

    }
    
    /* 
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
    */
    close(fd);

}


int create_file(char *file_name){
    int fd = open (file_name, O_RDWR);
    if (fd == -1) {
        exitInf(errno);
    }
    return fd;
}

void seperate_argumans(char **args, int n){
    char *file_name = args[n];
    char *task = args[n-1];
    int i = 0;
    int length = len(task);
    int count = 0;
    int control = 0;
    int w_count = 0;
    int check = 0;
    char **arr;
    int j = 0;

    for(i = 0; i < length; i++){
        if (task[i] =='/'){
            if (w_count != 0){
                count++;
                control ++;
                w_count = 0;
            }
            if (control > 2){
                errExit("Wrong input task");
            }
        }
        else if (task[i]==';'){
            if (control > 3 || control < 2) {
                errExit("Wrong input task");
            }
            control = 0;
            if (w_count != 0){
                count++;
                w_count = 0;
            }
            
        } 
        else{
            w_count++;
        }
        
    }
    if (w_count != 0){
        count ++;
        w_count = 0;
    }

    arr = malloc(count * sizeof(char *));

    count = 0;
    w_count = 0;

    for(i = 0; i < length; i++){
        if (task[i] =='/'){
            if (w_count != 0){
                arr[count] = malloc(w_count * sizeof(char *));
                count++;
                control ++;
                w_count = 0;
            }
        }
        else if (task[i]==';'){
            if (w_count != 0){
                arr[count] = malloc(w_count * sizeof(char *));
                count++;
                w_count = 0;
            }
        } 
        else{
            w_count++;
        }
    }
    if (w_count != 0){
        arr[count] = malloc(w_count * sizeof(char *));
        count ++;
        w_count = 0;
    }

    count = 0;
    w_count = 0;

    for(i = 0; i < length; i++){
        if (task[i] =='/'){
            if (w_count != 0){
                count++;
                w_count = 0;
            }
        }
        else if (task[i]==';'){
            if (w_count != 0){
                count++;
                w_count = 0;
            }
        } 
        else{
            arr[count][w_count] = task[i];
            w_count++;
        }
    }
    if (w_count != 0){
        for (j = 0; j < w_count; j++){
            arr[count][w_count] = task[i];
        }
        count++;
        w_count = 0;
    }

    for(i = 0; i < count ; i++){
        printf("%s \n", arr[i]);
    }

    file_operations(arr,count,file_name);


    for ( i = 0; i < count; i++ ){
        free(arr[i]);
    }
    free(arr);
}




