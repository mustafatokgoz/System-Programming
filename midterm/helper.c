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

