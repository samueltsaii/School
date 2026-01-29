.phony all:
all: pman

pman: main.c linked_list.c bg.c
	gcc -Wall main.c linked_list.c bg.c -o pman

.PHONY clean:
clean:
	-rm -rf *.o *.exe
