#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network.h"
#include "utils.h"
/* AJOUT CRUCIAL ICI : Inclusion des fonctions de gestion de processus */
#include "process.h"

void init_machine_manager(machine_manager_t *mm) {
    mm->count = 0;
    mm->capacity = 4;
    mm->machines = malloc(sizeof(machine_t) * mm->capacity);
}

void free_machine_manager(machine_manager_t *mm) {
    size_t i;
    for (i = 0; i < mm->count; ++i) {
        /* Maintenant le compilateur connaît cette fonction grâce à l'include */
        free_process_list(&mm->machines[i].list);
    }
    free(mm->machines);
}

void add_machine(machine_manager_t *mm, machine_t m) {
    if (mm->count >= mm->capacity) {
        mm->capacity *= 2;
        mm->machines = realloc(mm->machines, sizeof(machine_t) * mm->capacity);
    }
    /* Et celle-ci aussi */
    init_process_list(&m.list);
    mm->machines[mm->count++] = m;
}

int load_config_file(const char *path, machine_manager_t *mm) {
    FILE *f;
    char line[512];

    /* Déclarations au début pour compatibilité stricte */
    machine_t m;
    char *token;
    char *type_str;

    if (!check_secure_permissions(path)) {
        fprintf(stderr, "ERREUR: Droits fichiers incorrects pour %s (Attendu: 600)\n", path);
        return 0;
    }

    f = fopen(path, "r");
    if (!f) return 0;

    while (fgets(line, sizeof(line), f)) {
        memset(&m, 0, sizeof(machine_t));

        token = strtok(line, ":");
        if (!token) continue;
        strncpy(m.name, token, 63);

        token = strtok(NULL, ":");
        if (!token) continue;
        strncpy(m.hostname, token, 127);

        token = strtok(NULL, ":");
        if (!token) continue;
        m.port = atoi(token);

        token = strtok(NULL, ":");
        if (!token) continue;
        strncpy(m.username, token, 31);

        token = strtok(NULL, ":");
        if (!token) continue;
        strncpy(m.password, token, 63);
        
        token = strtok(NULL, ":");
        if (!token) continue;
        
        type_str = trim_whitespace(token);
        if (type_str && strcmp(type_str, "ssh") == 0) m.type = CONN_SSH;
        else if (type_str && strcmp(type_str, "telnet") == 0) m.type = CONN_TELNET;
        else m.type = CONN_SSH;

        m.enabled = 1;
        add_machine(mm, m);
    }
    fclose(f);
    return 1;
}

void update_remote_machine(machine_t *m) {
    char cmd[1024];
    FILE *fp;
    char buffer[512];
    process_info_t *p;

    if (m->type != CONN_SSH) return;

    snprintf(cmd, sizeof(cmd), 
        "sshpass -p '%s' ssh -o StrictHostKeyChecking=no -p %d %s@%s 'ps -Ao pid,state,pcpu,pmem,user,comm --no-headers'",
        m->password, m->port, m->username, m->hostname);

    fp = popen(cmd, "r");
    if (!fp) return;

    m->list.count = 0;

    while (fgets(buffer, sizeof(buffer), fp)) {
        
        if (m->list.count >= m->list.capacity) {
            m->list.capacity *= 2;
            m->list.procs = realloc(m->list.procs, sizeof(process_info_t) * m->list.capacity);
        }

        p = &m->list.procs[m->list.count];
        
        sscanf(buffer, "%d %c %lf %lf %31s %255s", 
            &p->pid, &p->state, &p->cpu_usage, &p->mem_usage, p->user, p->command);
        
        m->list.count++;
    }
    pclose(fp);
}