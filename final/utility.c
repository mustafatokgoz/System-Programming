#include "utility.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



int seperate_c_paramater(int *lower_bound,int *upper_bound,char *c_parameter){
    int i = 0;
    char *token = strtok(c_parameter,"-");
    *lower_bound = atoi(token);
    if(*lower_bound == 0){
        return -1;
    }
    token = strtok(NULL,"-");
    *upper_bound = atoi(token);
    if(*upper_bound == 0){
        return -1;
    }
    return 0;
}