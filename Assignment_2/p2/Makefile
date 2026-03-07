.phony all:
all: ACS

ACS: main.c heap.c heap.h queue.c queue.h get_time.c get_time.h passenger.h
	gcc -Wall main.c heap.c heap.h queue.c queue.h get_time.c get_time.h passenger.h -o ACS

.PHONY clean:
clean:
	-rm -rf *.o *.exe
