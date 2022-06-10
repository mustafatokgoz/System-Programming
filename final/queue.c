#include "queue.h"

queue *create_queue()
{
    queue *head = (queue *)malloc(sizeof(queue));
    head->size = 0;
    head->rearp = NULL;
    head->frontp = NULL;
    return head;
}

void add_rear(queue *head, int fd)
{
    if (head->size == 0)
    {
        head->rearp = (queue_node_t *)malloc(sizeof(queue_node_t));
        head->frontp = head->rearp;
    }
    else
    {
        head->rearp->restp = (queue_node_t *)malloc(sizeof(queue_node_t));
        head->rearp = head->rearp->restp;
    }
    head->rearp->data = fd;
    head->rearp->restp = NULL;
    ++(head->size);
}

int remove_front(queue *head)
{
    queue_node_t *to_freep;
    int ans;
    to_freep = head->frontp;
    ans = to_freep->data;
    head->frontp = to_freep->restp;
    free(to_freep);
    --(head->size);
    if (head->size == 0)
        head->rearp = NULL;
    return (ans);
}
int isQueue_empty(queue *head){
    return (head->size == 0);
}
int getf_front(queue *head){
    return head->frontp->data;
}
void print_queue(queue *head)
{
    queue_node_t *iter = head->frontp;
    for (int i = 0; i < head->size; i++)
    {
        printf("%d ", iter->data);
        iter = iter->restp;
    }
    printf("\n");
}
void free_queue(queue *head){
    while (head->frontp != NULL){
        queue_node_t *temp = head->frontp;
        head->frontp = head->frontp->restp;
        free(temp);
    }
    free(head);
}
