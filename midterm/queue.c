#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.h" 

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
 
    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->item = (struct element*)malloc(
        queue->capacity * sizeof(struct element));
    return queue;
}
 
// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}
 
// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}
 
// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, struct element item)
{
    if (isFull(queue))
        return;
    
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->item[queue->rear].pid  = item.pid;
    queue->item[queue->rear].array  = &item.array[0];
    queue->item[queue->rear].size  = item.size;
    queue->size = queue->size + 1;
}
 
// Function to remove an item from queue.
// It changes front and size
struct element dequeue(struct Queue* queue)
{
    struct element temp ;
    if (isEmpty(queue))
        return temp;
    struct element item = queue->item[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}
 
void freeQueue(struct Queue * queue){
    int i = 0;

    free(queue->item->array);
    free(queue->item);
    free(queue);
}

