#ifndef NETWORK_H
#define NETWORK_H

#include "defs.h"

/* Gère la liste des machines */
typedef struct {
    machine_t *machines;
    size_t count;
    size_t capacity;
} machine_manager_t;

void init_machine_manager(machine_manager_t *mm);
void free_machine_manager(machine_manager_t *mm);
void add_machine(machine_manager_t *mm, machine_t m);

/* Parsing et sync */
int load_config_file(const char *path, machine_manager_t *mm);
void update_remote_machine(machine_t *m);

#endif