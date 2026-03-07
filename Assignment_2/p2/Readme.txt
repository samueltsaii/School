V_number: V01070411
Section
Name Samuel Tsai

How to compile and run your code:

Notice for compilation: 

- main.c contains the core simulation program functionality
- queue.c: implementation for a FIFO queue to hold passengers after struct initialization
- queue.h: header signature and function definitions for FIFO queue
- heap.c: implementation for buissness and economy class min priority queues where passengers at the root are checked out the soonest.
- passenger.h: header signature for passenger structs.
- get_time.c: get current simulation time. Adopted from Brightspace program.
- get_time.h: header signature and function for get_time.c

When running the "make" command on the command, the following should be linked: 

main.c
queue.c
queue.h
heap.c
heap.h
passenger.h
get_time.c
get_time.h

To run:
./ACS <input_customers.txt>