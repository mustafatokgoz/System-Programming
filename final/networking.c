#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helper.h"



int client_to_server_connect(char *ip,int port){
    int clientSocket;
    struct sockaddr_in serverAddr;

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        exitInf("socket error");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(port);
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0)
        exitInf("connect error");
    return clientSocket;    
}


int server_socketfd(int port){
    int socketfd;
    struct sockaddr_in serverAddr;
    struct sockaddr_in newAddr;

    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        exitInf("socket error");

    int value = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int)) == -1)
        exitInf("setsockopt error");
    memset(&serverAddr, '\0', sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;


    if (bind(socketfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
        exitInf("bind error");

    if (listen(socketfd, 30) == -1)
        exitInf("listen error");
    return socketfd;
}