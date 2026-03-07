#ifndef HEAP_H
#define HEAP_H

#include <pthread.h>
#include "passenger.h"

typedef struct{
    PASSENGER** array;
    int size;
    int capacity;
    int num_served; 

    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
}HEAP;

void initializeHeap(HEAP* h, int capacity); 
void swap(PASSENGER** a, PASSENGER** b);
void heapify(HEAP* h, int i);
void insert(HEAP* h, PASSENGER* new_passenger);
PASSENGER* getMIN(HEAP* h); 


#endif