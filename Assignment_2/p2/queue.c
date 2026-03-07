#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <pthread.h>
#include <time.h>
#include "passenger.h"
# include "queue.h"

void initializeQueue(QUEUE *q, int capacity){
    q -> capacity = capacity;
    q -> size = 0; 
    q -> front = 0;
    q -> back  = 0;

    q -> elements = malloc(sizeof(void*) * capacity);
    pthread_mutex_init(&q -> lock, NULL);
    pthread_cond_init(&q -> not_empty, NULL);
    pthread_cond_init(&q -> not_full, NULL); 
}

bool isEmpty(QUEUE *q){
    return (q -> front == q -> back); 
}

bool isFull(QUEUE *q){
    return(q -> back == q -> capacity); 
}

void enqueue(QUEUE* q, void* elements){

    if(q -> size == q -> capacity){
        printf("Full. Queue Overflow");
    }

    q -> elements[q -> back] = elements;
    q -> back = (q -> back + 1) % q -> capacity;
    q -> size ++;
}

void* dequeue(QUEUE* q){

    pthread_mutex_lock(&q -> lock);

    while(q -> size == 0){
        pthread_cond_wait(&q -> not_empty, &q -> lock); 
    }
    
    void* p = q -> elements[q -> front];
    q -> front = (q -> front + 1) % q -> capacity; 
    q -> size --;

    pthread_cond_signal(&q -> not_full);
    pthread_mutex_unlock(&q -> lock);

    return p; 
}

void* peek(QUEUE *q){
    if(isEmpty(q)){
        printf("Queue is currently empty\n");
        return NULL; 
    }
    return q->elements[q->front];
}

void printQueue(QUEUE *q) {
    pthread_mutex_lock(&q->lock);
    int current = q->front;
    for (int i = 0; i < q->size; i++) {
        printf("%p ", q->elements[current]);
        current = (current + 1) % q->capacity;
    }
    printf("\n");
    pthread_mutex_unlock(&q->lock);
}
