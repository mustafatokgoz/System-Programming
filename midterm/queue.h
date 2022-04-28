#ifndef HELPER_H
#define HELPER_H


// A structure to represent a queue
struct element{
    int size;
    int pid;
    int *array;
};


struct Queue {
    int front, rear, size;
    unsigned capacity;
    struct element *item;
};
 
struct Queue* createQueue(unsigned capacity);
void freeQueue(struct Queue* queue);
int isEmpty(struct Queue* queue);
int isFull(struct Queue* queue);
void enqueue(struct Queue* queue, struct element item);
struct element dequeue(struct Queue* queue);



#endif