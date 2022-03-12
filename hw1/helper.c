#include "helper.h"
#include <stdlib.h>
#include <string.h>

void errExit(char *inf){
    char *errInf = "Error was occured for given information : ";
    write(2, errInf, strlen(errInf));
    write(2, inf, strlen(inf));    
    write(2, "\n", 1);
    exit(0);
}

void exitInf(char *inf){
    write(1, inf, strlen(inf));    
    write(1, "\n", 1);
    exit(0);
}


int len(char *str){
    int i = 0;
    for(i = 0; str[i] !='\0'; i++);
    return i;
}
