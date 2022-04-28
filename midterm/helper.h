#ifndef HELPER_H
#define HELPER_H

#include <sys/types.h>
#define CLIENT_FIFO_TEMPLATE "/tmp/seqnum_cl.%ld"
/* Template for building client FIFO name */
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)

struct request {
    pid_t pid;          /* PID of client */
    int seqLen;         /* Length of desired sequence */
};

struct response { 
    int seqNum; /* Start of sequence */
    int is_invertible;
};


void exitInf(char*);

#endif