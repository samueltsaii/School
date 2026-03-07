#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <pthread.h>
#include <time.h>
#include "passenger.h"
#include "heap.h"
/*Min Heap priority queue that priortizes passengers based on the expected check out time: passenger -> arrival_time + 
passenger -> service time. The min heap priority queue holds a capacity (there are no more than 5 passengers in a single queue
to match the number of available clerks), current size, and a dynamically allocated array to hold passengers. Passengers with
the mininum expected check out time will be checked out first.
*/
void initializeHeap(HEAP* h, int capacity){
    h -> capacity = capacity;
    h -> size = 0;
    h -> array = (PASSENGER**)malloc(capacity * sizeof(PASSENGER*));
    h -> num_served = 0; 

    pthread_mutex_init(&h -> lock, NULL);
    pthread_cond_init(&h -> not_full, NULL);
    pthread_cond_init(&h -> not_empty, NULL); 
}

void swap(PASSENGER** a, PASSENGER** b){
    /*Helper function that swaps passengers.*/
    PASSENGER* temp = *a;
    *a = *b;
    *b = temp; 
}

void heapify(HEAP* h, int i){
    /*Function that maintains heap property*/
    int min = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if(left < h -> size && h -> array[left]-> service_time < h -> array[min] -> service_time){
        min = left;
    }
    if(right < h -> size && h -> array[right]-> service_time < h -> array[min] -> service_time){
        min = right;
    }
    if(min != i){
        swap(&h -> array[i], &h -> array[min]);
        heapify(h, min); 
    }
}


void insert(HEAP* h, PASSENGER* new_passenger){
    /*Insertion operation that adds a passenger struct into the heap. Prioritization is based on the expected
    check out time, passenger -> arrival_time + passenger -> service time. If the heap is at maxinum capacity,
    a conditional wait is called preventing any more insertion operations from being preformed until a not_full signal is recieved.*/
    pthread_mutex_lock(&h -> lock);
    while(h -> size == h -> capacity){ 
        pthread_cond_wait(&h -> not_full, &h -> lock); 
    }

    int i = h -> size ++;
    h -> array[i] = new_passenger;

    while(i != 0 && (h -> array[(i - 1) / 2] -> arrival_time + h -> array[(i - 1) / 2] -> service_time ) 
        > (h -> array[i] -> arrival_time + h -> array[i] -> service_time )){
        swap(&h -> array[i], &h -> array[(i - 1) / 2]);
        i = (i - 1) / 2; 
    }
    h -> num_served ++; 
    pthread_cond_signal(&h -> not_empty);
    pthread_mutex_unlock(&h -> lock); 
}

PASSENGER* getMIN(HEAP* h){
    /*Removes and returns passenger struct based on which passenger holds the mininum expected check out time.
    Once a passenger is checked out, the size is decremented and heapify is called to maintain the heap property.
    A conditional wait is called when the heap is empty and the getMin operator can not be preformed until recieving a
    not_empty signal*/
    pthread_mutex_lock(&h -> lock); 

    while(h -> size <= 0){
        pthread_cond_wait(&h -> not_empty, &h -> lock); 
    }

    PASSENGER* root = h -> array[0];
    h -> array[0] = h -> array[h -> size - 1];
    h -> size --;
    heapify(h, 0);

    pthread_cond_signal(&h -> not_full); 
    pthread_mutex_unlock(&h -> lock); 
    return root; 
    
}

