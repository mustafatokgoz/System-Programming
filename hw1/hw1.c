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
int open_file(char *);
void change_occurance(int , char *, char *,int);
void helper_op(int *, int *, int *);


int main(int argc, char **argv){
    
    if(argc < 3 || argc > 3){
        errExit("Arguman error");
    }
    seperate_argumans(argv,argc - 1);
    return 0;

}

void change_occurance(int temp_fd, char *cur_occ, char *occ,int many){
    int current_status = (many  -1 )* -1;
    //int current_status = (len(cur_occ) -1) * -1;
    //if (star == 0){
    //    current_status = (len(cur_occ) - 2 ) * -1;
    //}

    if(lseek(temp_fd,current_status,SEEK_END) == -1)
    {
        exitInf("lseek error");
    }
    write(temp_fd, occ, len(occ));
}


void helper_op(int *number_l, int *check, int *check2){
    if(*number_l == 1){
        *check2 = 1;
    }
    *number_l = 0;
    *check = 0;
    
}

void helper_else(boolean case_insensitive, char cur,char c, int *number_l, int *check, int *check2,int *star,int insense){
    if(case_insensitive == True){
        if ((c == (cur - 32)) || (c == (cur + 32))){
            if(insense == 0){
                *number_l = *number_l + 1;
                *check = *check + 1;
            }
            else{
                *star = 1;
            }
        }
        else{
            helper_op(number_l,check,check2);
        }
    }
    else{
        helper_op(number_l,check,check2);
    }
}


int find_star_number(char *word){
    int i = 0;
    int number = 0;
    for(i = 0; word[i]!='\0'; i++){
        if(word[i]=='*'){
            number++;
        }
    }
    
    return number;
}

void file_operations(char **arr, int count, char *file_name){
    int fd,temp_fd;
    char c;
    int i;
    int rd;
    char last_c;
    char temp_file[]="/tmp/my-temp-fileXXXXXX";
    boolean case_insensitive = False;
    int number_l = 0, check = 0, check2 = 0, star = 0, star_number = 0;
    struct flock fl = { F_WRLCK, SEEK_SET, 0,       0,     0 };

    fd = open_file(file_name);

    temp_fd=mkstemp(temp_file);

    unlink(temp_file);

    if (temp_fd < 1){
        exitInf("temp file error");
    }

    fl.l_pid    = getpid();

    
    for(i = 0; i < count; i = i + 2){
        if ((i+2) < count && len(arr[i+2]) == 1 && arr[i+2][0] == 'i'){
            case_insensitive = True;
        } 
        else{
            case_insensitive = False;
        }
        check = 0;
        check2 = 0;
        number_l = 0;

        if (fcntl(fd, F_SETLKW, &fl) == -1){
    		perror("fcntl");
    		exit(1);
        }    

        //write(1, "it will be lock\n" , len("it will be lock\n"));
        //write(1, "Press any to release lock: ", len("Press any to release lock: "));
        //getchar();
 
        if(lseek(fd,0,SEEK_SET) == -1)
        {
            exitInf("lseek error");
        }
        
        if(lseek(temp_fd,0,SEEK_SET) == -1)
        {
            exitInf("lseek error");
        }
        rd = read(fd, &c, 1);
        while(rd > 0){
            if (c == arr[i][check]){
                number_l ++;
                check ++;
            }
            else if (arr[i][check] == '*'){
                if (check > 0){
                    if (c != arr[i][check-1]){
                        helper_else(case_insensitive,arr[i][check-1],c,&number_l,&check,&check2,&star,1);
                    }
                    else{
                        star = 1;
                    }
                }
            }
            else{

                helper_else(case_insensitive,arr[i][check],c,&number_l,&check,&check2,&star,0);
            }
            //printf("bu %d %d ",number_l, len(arr[i]));
            if (number_l == len(arr[i])){
                if(star == 1){
                    star_number = find_star_number(arr[i]) + 1 ;
                    change_occurance(temp_fd,arr[i],arr[i+1],(len(arr[i]) - star_number));
                    perror("giriyor");
                    star = 0;
                }
                else{
                    star_number = find_star_number(arr[i]);
                    change_occurance(temp_fd,arr[i],arr[i+1],(len(arr[i])- star_number));
                }       
            }
            else{
                if(check2 == 0 && star == 0) {
                        write(temp_fd, &c, 1);
                        
                }
            }
            last_c = c;
            if(check2 == 0) {
                rd = read(fd, &c, 1);
            }
            check2 = 0;
            if(last_c != c && arr[i][check] == '*'){
                    check++;
                    number_l++;
                    if(number_l != len(arr[i])){
                    }
            }
            
        }

        write(1," ! buraya ! ",len(" ! buraya ! "));
        fl.l_type = F_UNLCK;  /* set to unlock same region */

        if (fcntl(fd, F_SETLK, &fl) == -1) {
    	    perror("fcntl");
    	    exit(1);
        }

        ftruncate(fd, 0);
        //close(fd);

        //fd=open(file_name,O_RDWR | O_TRUNC);

        if (fcntl(fd, F_SETLKW, &fl) == -1){
    		perror("fcntl");
    		exit(1);
        }

        if(lseek(fd,0,SEEK_SET) == -1)
        {
            exitInf("lseek error");
        }
        if(lseek(temp_fd,0,SEEK_SET) == -1)
        {
            exitInf("lseek error");
        }
        rd = read(temp_fd, &c, 1);
        while(rd > 0){
            write(fd,&c,1);
            write(1,&c,1);
            rd = read(temp_fd, &c, 1);
        }
        if (case_insensitive == True){
            i++;
        }

        fl.l_type = F_UNLCK;  /* set to unlock same region */
    
        if (fcntl(fd, F_SETLK, &fl) == -1) {
    	    perror("fcntl");
    	    exit(1);
        }
        ftruncate(temp_fd, 0);
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


int open_file(char *file_name){
    int fd = open (file_name, O_RDWR);
    if (fd == -1) {
        exitInf("file error");
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
                printf("bu %d ",control);
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




