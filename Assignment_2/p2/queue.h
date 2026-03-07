#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include "passenger.h"


typedef struct{
    int capacity;
    int size; 
    int front;
    int back;
    void **elements;
    
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full; 
    
} QUEUE;

void initializeQueue(QUEUE *q, int capacity);
bool isEmpty(QUEUE *q);
bool isFull(QUEUE *q);
void enqueue(QUEUE * q, void* elements);
void* dequeue(QUEUE* q);
void* peek(QUEUE* q);


#endif 