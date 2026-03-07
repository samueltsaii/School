#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

struct timeval start_time; // simulation start time

void initStartTime(){
	gettimeofday(&start_time, NULL);
}

double getCurrentSimulationTime(){
	
	struct timeval cur_time;
	double cur_secs, init_secs;
	
	//pthread_mutex_lock(&start_time_mtex); you may need a lock here
	init_secs = (start_time.tv_sec + (double) start_time.tv_usec / 1000000);
	//pthread_mutex_unlock(&start_time_mtex);
	
	gettimeofday(&cur_time, NULL);
	cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);
	
	return cur_secs - init_secs;
}