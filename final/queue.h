#ifndef _LINKED_LIST_QUEUE_H_
#define _LINKED_LIST_QUEUE_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

typedef struct queue_node_t
{
    int data;
    struct queue_node_t *restp;
}queue_node_t;

typedef struct queuenode
{
    queue_node_t *frontp;
    queue_node_t *rearp;
    int size;
}queue;

queue *create_queue();
void add_rear(queue *head, int fd);
int remove_front(queue *head);
int isQueue_empty(queue *head);
int get_front(queue *head);
void print_queue(queue *head);
void free_queue(queue *head);

#endif