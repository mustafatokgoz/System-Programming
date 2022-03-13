#include "helper.h"
#include <stdlib.h>

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


void exitInf(int err){
    perror(err);
    exit(0);
}



