#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include "queue.h"


static struct Queue *denemeq;

void initilizeshared(){  
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IRWXU;
    char memoryName[15];
    denemeq = createQueue(20);
    strcpy(memoryName,"queue_mem");
    shm_unlink(memoryName);
    int memFd = shm_open(memoryName, O_RDWR | O_CREAT , mode);
    if (memFd == -1)
       
    if (ftruncate(memFd, sizeof(*denemeq)) == -1)  
        

    denemeq = (struct queue *)mmap(NULL, sizeof(*denemeq), PROT_READ | PROT_WRITE, MAP_SHARED, memFd, 0);
    if (denemeq == MAP_FAILED)
        
    close(memFd);
}

void destroy_shared(){
  char memoryName[15];
  strcpy(memoryName,"queue_mem");
  shm_unlink(memoryName);
}


int main(){
    initilizeshared();
    struct element temp,temp2;
    temp.pid = 0;
    temp.array = malloc(10 * sizeof(int));
    int i = 0;
    for (i = 0; i < 10; i++){
        temp.array[i] =i; 
    }
    temp.size = 10;
    enqueue(denemeq,temp);

    temp2 = dequeue(denemeq);

    printf("bu pid %d , bu size %d \n", temp2.pid,temp2.size);
    for(i = 0; i < temp2.size;i++){
        printf("  bu %d  ", temp2.array[i]);
    }
    printf("\n");
    freeQueue(denemeq);
    destroy_shared();
    return 0;

}