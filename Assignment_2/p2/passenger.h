#ifndef PASSENGER_H
#define PASSENGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*passenger struct header signature defining necessary data types.*/
typedef struct {
    int customer_id;
    int passenger_class;
    double arrival_time;
    double service_time;
} PASSENGER;

#endif
