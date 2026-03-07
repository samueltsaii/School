#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <pthread.h>
#include <time.h>
#include "passenger.h"
#include "queue.h"
#include "heap.h"
#include "get_time.h"
#define MAX_LINE_LENGTH 64
/*Global variables that keeps track of the number of passengers to be served, the total time to serve buissness class 
passengers, and the total time to serve economy class passengers*/
int NUM_PASSENGERS;
double buissness_class_wait_for_serve_time = 0;
double economy_class_wait_for_serve_time = 0; 
/*Global initialization of buissness_class priority queue, economy_class priority queue, mutex lock type signatures, and
conditional variable type sginature.*/
HEAP buissness_class;
HEAP economy_class;
pthread_mutex_t print_lock;
pthread_mutex_t count_lock;
pthread_cond_t all_checked_out;


void* passenger(void* args){
    /*A passenger runner function that is passed to a passenger thread.
    A reference is made to the passenger struct at the head of the passenger queue. The passenger's 
    arrival time is converted from microseconds to seconds and put asleep.
    A check is then preformed to determine if the passenger struct is to be 
    inserted in the buissness class or economy class priority queue.*/
    PASSENGER* p = (PASSENGER*) args;
    
    usleep(p -> arrival_time * 1000000);

    pthread_mutex_lock(&print_lock); 
    printf("A customer arrives: customer ID %2d. \n", p -> customer_id); 
    pthread_mutex_unlock(&print_lock); 

    if(p -> passenger_class == 1){
        insert(&buissness_class, p);
        pthread_mutex_lock(&print_lock); 
        printf("A customer enters a queue: the queue ID 1, and length of the queue %d. \n", buissness_class.size); 
        pthread_mutex_unlock(&print_lock);
    }
    else{
        insert(&economy_class, p);
        pthread_mutex_lock(&print_lock); 
        printf("A customer enters a queue: the queue ID 0, and length of the queue %d. \n", economy_class.size);
        pthread_mutex_unlock(&print_lock);
    }
    return NULL; 
}

void* clerk(void* args){
    /*A clerk runner function that references and dequeues a passenger from a buissness class or economy class priority queue.
    The buissness class is always checked first ensuring that it is priortized. The target_time, current time in the simulation,
    and wait times are calcuated. If the wait time is greater than 0, the clerk thread will fall asleep until the current simulation
    time matches that of the target time. 
    
    Once the target time and simulation time matches, the clerk thread will be woken up to 
    decrement the NUM_PASSENGERS variable, update the total service time depending on whether the passenger came from buissness or 
    economy class, and print the following information to the user: customer_id, the time in which the clerk is
    finished serving the passenger, and the id of the clerk which served the customer.*/
    int clerk_id = *(int*)args;
    
    while(true){
        PASSENGER* p = NULL;

        pthread_mutex_lock(&buissness_class.lock);
        if(buissness_class.size > 0){
            pthread_mutex_unlock(&buissness_class.lock);
            p = getMIN(&buissness_class);
        } 
        else {
            pthread_mutex_unlock(&buissness_class.lock); 
            pthread_mutex_lock(&economy_class.lock); 
            if(economy_class.size > 0){
                pthread_mutex_unlock(&economy_class.lock); 
                p = getMIN(&economy_class);
            }
            else{
                pthread_mutex_unlock(&economy_class.lock);
            }
        }

        
        if(p != NULL){
            double arrival_time_seconds = (p->arrival_time)/10.0;
            double start_time = getCurrentSimulationTime();

            pthread_mutex_lock(&print_lock);
            printf("A clerk starts serving customer %d: start %.1f, clerk %d.\n",
                    p->customer_id, start_time, clerk_id); 
            pthread_mutex_unlock(&print_lock);
            
            double customer_wait_clerk_time = (start_time - arrival_time_seconds)/10.0; 
            double target_time = start_time + (p->service_time / 10.0); 
            
            double now = getCurrentSimulationTime();
            double wait_remaining = target_time - now;

            if(wait_remaining > 0) {
                usleep((useconds_t)(wait_remaining * 1000000));
            }

            double end_time = getCurrentSimulationTime();
            pthread_mutex_lock(&print_lock); 
            printf("-->>> A clerk finishes serving customer %d: end %.1f, clerk %d.\n",
                    p->customer_id, end_time, clerk_id);
            pthread_mutex_unlock(&print_lock);
            
            pthread_mutex_lock(&count_lock); 
            if(p->passenger_class == 1) buissness_class_wait_for_serve_time += customer_wait_clerk_time;
            else economy_class_wait_for_serve_time += customer_wait_clerk_time;

            NUM_PASSENGERS--; 
            if(NUM_PASSENGERS == 0) pthread_cond_signal(&all_checked_out);
            pthread_mutex_unlock(&count_lock);

            free(p);
        } 
        else {
            usleep(1000); 
        }
    }
    return NULL;
}


int main(int argc, char* argv[]){
    /*Read from input file and initialize passenger struct types based on the corresponding:
        customer_id -> 1st column
        passenger_class -> 2nd column following colon
        arrival_time -> 3rd column following comma
        service_time -> 4th column following comma
        */
    char *filename = argv[1];
    char buffer[MAX_LINE_LENGTH];
    const char *deliminiters = ":,\n";
    FILE *fptr;

    if(argc != 2){
        fprintf(stderr, "Argument must be {file_name}.txt"); 
        exit(1); 
    }

    fptr = fopen(filename, "r");
    if(fptr == NULL){
        fprintf(stderr, "Error Opening file");
        exit(1); 
    }

    fgets(buffer, sizeof(buffer), fptr);
    NUM_PASSENGERS = atoi(buffer);
    
    QUEUE passenger_queue;
    initializeQueue(&passenger_queue, NUM_PASSENGERS);

    int index = 0;
    while(fgets(buffer, sizeof(buffer), fptr) != NULL && index < NUM_PASSENGERS){
        
        char *token = strtok(buffer, deliminiters);

        PASSENGER* p = malloc((sizeof(PASSENGER))); 
        
        p -> customer_id = atoi(token);
        token = strtok(NULL, deliminiters);

        p -> passenger_class = atoi(token);
        token = strtok(NULL, deliminiters);

        if(atof(token) < 0){
            p -> arrival_time = (-1 * atof(token)); 
        }
        else{
            p -> arrival_time = atof(token);
        }
        token = strtok(NULL, deliminiters);

        if(atof(token) < 0){
            p -> service_time = (-1 * atof(token)); 
        }
        else{
            p -> service_time = atof(token);
        }
        
        enqueue(&passenger_queue, p); 
        index++;
    }
    fclose(fptr);
    /*File reading section ends. References to buissness class and economy class priority queues are passed into
    priority queue heap initialization functions.*/
    initStartTime();
    pthread_mutex_init(&print_lock, NULL);
    initializeHeap(&buissness_class, NUM_PASSENGERS);
    initializeHeap(&economy_class, NUM_PASSENGERS);

    /*Initialize an array of 5 clerk threds*/
    pthread_t clerks[5];
    int clerk_id[5]; 
    for(int i = 0; i < 5; i++){
        clerk_id[i] = i + 1;
        pthread_create(&clerks[i], NULL, clerk, (void*)&clerk_id[i]); 
    }
    /*Initialize NUM_PASSENGERS passenger threads*/
    pthread_t *passenger_threads = malloc(sizeof(pthread_t) * NUM_PASSENGERS);

    for(int i = 0; i < NUM_PASSENGERS; i++){
        PASSENGER *p = (PASSENGER*)dequeue(&passenger_queue);
        pthread_create(&passenger_threads[i], NULL, passenger, (void*)p); 
    }
    /*Join passenger threads once finished*/
    for(int i = 0; i < NUM_PASSENGERS; i++){
        pthread_join(passenger_threads[i], NULL); 
    }
    /*Conditional variables used to ensure that all passengers are checked out. Clerk threads are put asleep
    when not serving a passenger. If the passenger at the head of the passenger priority queue matches the 
    current simulation time, a available clerk thread will be awaken and start serving the passenger. The simulation
    ends when NUM_PASSENGERS reaches 0, falling through the conditional wait.*/
    pthread_mutex_lock(&count_lock);
    while(NUM_PASSENGERS != 0){
        pthread_cond_wait(&all_checked_out, &count_lock); 
    }
    pthread_mutex_unlock(&count_lock);
    
    double buissness_class_wait_srvc_avg = buissness_class_wait_for_serve_time/ buissness_class.num_served;
    double economy_class_wait_srvc_avg = economy_class_wait_for_serve_time/ economy_class.num_served;

    double total_time = buissness_class_wait_for_serve_time + economy_class_wait_for_serve_time;
    double total_srvc_avg = total_time /(buissness_class.num_served + economy_class.num_served); 

    printf("\n");
    printf("All passengers served and checked out!\n");
    printf("__________________________________________________________________________________\n"); 
    printf("Total waiting time for buissness class passengers: %.1f\n", buissness_class_wait_for_serve_time);
    printf("Average waiting time buissness class passengers: %.1f\n", buissness_class_wait_srvc_avg);
    printf("\n");
    printf("Total waiting time for economy class passengers: %.1f\n", economy_class_wait_for_serve_time);
    printf("Average waiting time for economy class passengers: %.1f\n", economy_class_wait_srvc_avg);
    printf("\n");
    printf("Total waiting time for all passengers: %.1f\n", total_time);
    printf("Average waiting time for all passengers: %.1f\n",total_srvc_avg);
    
    return 0; 
}

