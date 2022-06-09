#ifndef NETWORKING_H
#define NETWORKING_H


int client_to_server_connect(char *ip,int port);
int server_socketfd(int port);

#endif