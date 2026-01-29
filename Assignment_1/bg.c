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
#include <sys/times.h>

Node* head = NULL;

void func_BG(char **cmd){
    pid_t pid = fork();
    
    if(pid < 0){
      perror("Failed to fork");
    }
    else if(pid == 0){
      if(execvp(cmd[1], &cmd[1]) == -1){
        perror("Execution failed");
        exit(1);
      }
    }
    else{
      head = add_newNode(head, pid, cmd[1]);
    }
}


void func_BGlist(char **cmd){
	printList(head);
}


void func_BGkill(char * str_pid){
  int key_pid = atoi(str_pid);
  if(key_pid < 0){
    printf("Process %d not found.\n", key_pid);
  }

	if(PifExist(head, key_pid)){
    if(kill(key_pid, SIGKILL) == 0){
      waitpid(key_pid, NULL, 0); 
      head = deleteNode(head, key_pid);
      printf("Process removed %d", key_pid); 
    }
    else{
      perror("kill"); 
    }
  }
  else{
    printf("Process not found"); 
    }
  }


void func_BGstop(char * str_pid){
	int key_pid = atoi(str_pid);
	if(PifExist(head, key_pid)){
    kill(key_pid, SIGSTOP);
    }
  else{
  printf("Process %d not found.\n", key_pid); 
  }
  }


void func_BGstart(char * str_pid){
  int key_pid = atoi(str_pid);
    if(PifExist(head, key_pid)){
      kill(key_pid, SIGCONT);
      }
    else{
    printf("Process %d not found.\n", key_pid); 
    }
    }

void get_context_switches(int pid, long *vol_switches, long *invol_switches){
    char path[256];
    char line[256];
    sprintf(path, "/proc/%d/status", pid);
    
    FILE *fp = fopen(path, "r");
    if (fp == NULL) return;

    *vol_switches = 0;
    *invol_switches = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "voluntary_ctxt_switches:", 24) == 0) {
            sscanf(line + 24, "%ld", vol_switches);
        } else if (strncmp(line, "nonvoluntary_ctxt_switches:", 27) == 0) {
            sscanf(line + 27, "%ld", invol_switches);
        }
    }
    fclose(fp);
}

void func_pstat(char * str_pid){
  pid_t pid = atoi(str_pid);
  Node*curr = head;

  while(curr != NULL && curr ->pid != pid){
    curr = curr -> next; 
  }

  if(curr == NULL){
    printf("Error: process %d does not exist\n", pid);
    return; 
  }

  char path[256];
    sprintf(path, "/proc/%d/stat", pid);
    FILE *fp = fopen(path, "r");
    
    if (fp == NULL){
      return;
    }

    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        fclose(fp);
        return;
    }
    fclose(fp);
    
    char *first_paren = strrchr(buffer ,'(');
    char *last_paren = strrchr(buffer, ')');
    if (first_paren == NULL || last_paren == NULL){
      return;
    }
    int comm_len = last_paren - first_paren - 1;
    char comm[256];
    strncpy(comm, first_paren + 1, comm_len);
    comm[comm_len] = '\0'; 

    char state;
    long utime = 0;
    long stime = 0;
    long rss = 0; 

    sscanf(last_paren + 2, 
    "%c %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %ld %ld %*s %*s %*s %*s %*s %*s %*s %*s %ld",
    &state, &utime, &stime, &rss);
        

    long clock_tick = sysconf(_SC_CLK_TCK);
    double user_time_seconds = (double)utime / clock_tick;
    double system_time_seconds = (double)stime / clock_tick;

    long page_bytes = sysconf(_SC_PAGESIZE);
    long rss_bytes = rss * page_bytes;

    long vol_switches, invol_switches;
    get_context_switches(pid, &vol_switches, &invol_switches);

    printf("(%s)\n%c\n%f\n%f\n%ld\n%ld\n%ld\n", comm, state, user_time_seconds,
       system_time_seconds, rss_bytes, vol_switches, invol_switches);
    
}