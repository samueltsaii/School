#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/times.h>

struct tms time_buffer;
    clock_t start_time, end_time;
    long clock_ticks;
    double system_time_seconds;

void get_time();