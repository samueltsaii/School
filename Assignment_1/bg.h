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

#ifndef _BG_H_
#define _BG_H_

#include "linked_list.h"

extern Node* head; 
void func_BG(char **cmd); 
void func_BGlist(char **cmd);
void func_BGkill(char * str_pid); 
void func_BGstop(char * str_pid);
void func_BGstart(char * str_pid);
void get_context_switches(int pid, long *vol_switches, long *invol_switches); 
void func_pstat(char * str_pid); 

#endif