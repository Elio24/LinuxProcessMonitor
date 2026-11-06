#ifndef PROCESS_H
#define PROCESS_H

/* Inclusion nécessaire pour connaitre process_list_t */
#include "defs.h"

void init_process_list(process_list_t *list);
void free_process_list(process_list_t *list);
void update_local_processes(process_list_t *list);
int send_signal_process(int pid, int signal);

#endif