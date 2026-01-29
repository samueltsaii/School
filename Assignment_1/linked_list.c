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
#include "linked_list.h"


Node * add_newNode(Node* head, pid_t new_pid, char * new_path){
	Node* new_node = (Node*) malloc(sizeof(Node));
	if(new_node == NULL){
		perror("unable to allocate memory"); 
		return head;
	}

	new_node -> pid = new_pid;
	new_node -> path = strdup(new_path);
	new_node -> next = head; 
	return new_node; 

}


Node * deleteNode(Node* head, pid_t pid){
	Node* curr = head;
	Node* prev = NULL;
	if(curr != NULL && curr-> pid == pid){
		head = curr -> next;
		free(curr);
	}
	while(curr != NULL && curr -> pid != pid){
		prev = curr;
		curr = curr ->next;
	}
	if (curr == NULL){
		return head;
	}
	prev ->next = curr -> next;
	free(curr);
	return head;
}

void printList(Node *node){
	Node* curr = node;
	if(curr == NULL){
		printf("empty list");
	}
	while(curr != NULL){
		printf("%d %s\n", curr -> pid, curr -> path);
		curr = curr -> next;
	}
}


int PifExist(Node *node, pid_t pid){
	Node* curr = node;
	while(curr != NULL){
		if(curr -> pid == pid){
			return 1; 
		}
		curr = curr -> next;
	}
	return 0;
}

