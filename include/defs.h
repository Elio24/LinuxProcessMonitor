#ifndef DEFS_H
#define DEFS_H

#include <stddef.h>
#include <time.h>

#define MAX_CMD_LEN 256
#define MAX_USER_LEN 32
#define MAX_HOST_LEN 128
#define MAX_PASS_LEN 64

/* États des processus */
typedef enum {
    STATE_RUNNING = 'R',
    STATE_SLEEPING = 'S',
    STATE_ZOMBIE = 'Z',
    STATE_STOPPED = 'T',
    STATE_UNKNOWN = '?'
} proc_state_t;

/* Structure d'un processus */
typedef struct {
    int pid;
    char user[MAX_USER_LEN];
    char state;
    double cpu_usage;
    double mem_usage;
    char command[MAX_CMD_LEN];
} process_info_t;

/* Conteneur de liste de processus */
typedef struct {
    process_info_t *procs;
    size_t count;
    size_t capacity;
} process_list_t;

/* Type de connexion */
typedef enum {
    CONN_LOCAL,
    CONN_SSH,
    CONN_TELNET
} connection_type_t;

/* Structure d'une machine */
typedef struct {
    char name[64];
    char hostname[MAX_HOST_LEN];
    int port;
    char username[MAX_USER_LEN];
    char password[MAX_PASS_LEN];
    connection_type_t type;
    int enabled; /* Pour savoir si on doit la monitorer */
    
    process_list_t list;
} machine_t;

#endif