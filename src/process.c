#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

/* Ordre crucial : d'abord les définitions, ensuite les prototypes */
#include "defs.h"
#include "process.h"

void init_process_list(process_list_t *list) {
    list->count = 0;
    list->capacity = 128; 
    list->procs = malloc(sizeof(process_info_t) * list->capacity);
}

void free_process_list(process_list_t *list) {
    if (list->procs) {
        free(list->procs);
        list->procs = NULL;
    }
}

/* Fonction interne (static recommandé mais laissé global pour simplicité) */
int is_numeric(const char *str) {
    while (*str) {
        if (!isdigit(*str)) return 0;
        str++;
    }
    return 1;
}

void update_local_processes(process_list_t *list) {
    DIR *proc_dir;
    struct dirent *entry;
    
    proc_dir = opendir("/proc");
    if (!proc_dir) return;

    list->count = 0;

    while ((entry = readdir(proc_dir)) != NULL) {
        if (!is_numeric(entry->d_name)) continue;

        int pid = atoi(entry->d_name);
        
        /* Redimensionnement sécurisé */
        if (list->count >= list->capacity) {
            list->capacity *= 2;
            list->procs = realloc(list->procs, sizeof(process_info_t) * list->capacity);
        }

        process_info_t *p = &list->procs[list->count];
        p->pid = pid;

        /* Lecture de /proc/[pid]/stat */
        char path[256];
        snprintf(path, sizeof(path), "/proc/%d/stat", pid);
        FILE *f = fopen(path, "r");
        if (f) {
            char comm_wrapper[256];
            char state;
            /* Lecture simplifiée */
            fscanf(f, "%*d %s %c", comm_wrapper, &state);
            
            /* Nettoyage du nom (retirer les parenthèses) */
            if (comm_wrapper[0] == '(') {
                size_t len = strlen(comm_wrapper);
                if (len > 0) comm_wrapper[len-1] = '\0';
                strncpy(p->command, comm_wrapper + 1, MAX_CMD_LEN - 1);
            } else {
                strncpy(p->command, comm_wrapper, MAX_CMD_LEN - 1);
            }
            
            p->state = state;
            fclose(f);
        }

        /* Lecture de l'UID */
        snprintf(path, sizeof(path), "/proc/%d/status", pid);
        f = fopen(path, "r");
        if (f) {
            char line[128];
            while (fgets(line, sizeof(line), f)) {
                if (strncmp(line, "Uid:", 4) == 0) {
                    int uid;
                    sscanf(line, "Uid: %d", &uid);
                    snprintf(p->user, MAX_USER_LEN, "%d", uid); 
                    break;
                }
            }
            fclose(f);
        }

        /* Valeurs par défaut */
        p->cpu_usage = 0.0; 
        p->mem_usage = 0.0;

        list->count++;
    }
    closedir(proc_dir);
}

int send_signal_process(int pid, int sig) {
    return kill(pid, sig);
}