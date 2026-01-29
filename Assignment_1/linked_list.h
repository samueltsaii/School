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
#include "bg.h"

#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

typedef struct Node Node;

struct Node{
    pid_t pid;
    char * path;
    Node * next;
};


Node * add_newNode(Node* head, pid_t new_pid, char * new_path);
Node * deleteNode(Node* head, pid_t pid);
void printList(struct Node *node);
int PifExist(Node *node, pid_t pid);



#endif