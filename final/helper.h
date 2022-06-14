#ifndef HELPER_H
#define HELPER_H

typedef struct servants_ports{
    int port;
    char city1[70];
    char city2[70];
}servants_ports;


void exitInf(char*);
int open_file(char *filename);
int close_file(int fd);
#endif