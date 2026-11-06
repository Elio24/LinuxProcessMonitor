#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include "ui.h"
#include "network.h"
#include "process.h"

void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("Options:\n");
    printf("  -h, --help            Affiche cette aide\n");
    printf("  -d, --dry-run         Test l'accès sans interface graphique\n");
    printf("  -c, --remote-config   Fichier config (format nom:ip:port:user:pass:type)\n");
    printf("  -s, --remote-server   IP/DNS machine distante\n");
    printf("  -l, --login           user@host\n");
    printf("  -u, --username        Utilisateur\n");
    printf("  -p, --password        Mot de passe\n");
    printf("  -t, --connexion-type  ssh ou telnet\n");
    printf("  -P, --port            Port personnalisé\n");
    printf("  -a, --all             Ajouter la machine locale si distant spécifié\n");
}

void get_password_interactive(char *buffer, size_t size) {
    printf("Mot de passe requis : ");
    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
    }
}

int main(int argc, char *argv[]) {
    int opt;
    char *config_path = NULL;
    int dry_run = 0;
    int show_all = 0; /* Option -a */
    int remote_specified = 0; /* Si -c ou -s est utilisé */

    /* Machine temporaire pour les arguments CLI (-s, -l) */
    machine_t cli_machine;
    memset(&cli_machine, 0, sizeof(machine_t));
    cli_machine.port = 22; /* Port défaut SSH */
    cli_machine.type = CONN_SSH;
    snprintf(cli_machine.name, 64, "CLI_HOST");

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"dry-run", no_argument, 0, 'd'},
        {"remote-config", required_argument, 0, 'c'},
        {"remote-server", required_argument, 0, 's'},
        {"login", required_argument, 0, 'l'},
        {"username", required_argument, 0, 'u'},
        {"password", required_argument, 0, 'p'},
        {"connexion-type", required_argument, 0, 't'},
        {"port", required_argument, 0, 'P'},
        {"all", no_argument, 0, 'a'},
        {0, 0, 0, 0}
    };

    machine_manager_t mm;
    init_machine_manager(&mm);

    while ((opt = getopt_long(argc, argv, "hdc:s:l:u:p:t:P:a", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h': print_usage(argv[0]); free_machine_manager(&mm); exit(0);
            case 'd': dry_run = 1; break;
            case 'c': config_path = optarg; remote_specified = 1; break;
            case 'a': show_all = 1; break;
            
            /* Gestion manuelle d'une machine distante */
            case 's': 
                strncpy(cli_machine.hostname, optarg, 127); 
                remote_specified = 1; 
                break;
            case 'l': /* user@host */
                {
                    char *at = strchr(optarg, '@');
                    if (at) {
                        *at = '\0';
                        strncpy(cli_machine.username, optarg, 31);
                        strncpy(cli_machine.hostname, at + 1, 127);
                    } else {
                        strncpy(cli_machine.username, optarg, 31);
                    }
                    remote_specified = 1;
                }
                break;
            case 'u': strncpy(cli_machine.username, optarg, 31); break;
            case 'p': strncpy(cli_machine.password, optarg, 63); break;
            case 't': 
                if (strcmp(optarg, "telnet") == 0) cli_machine.type = CONN_TELNET;
                break;
            case 'P': cli_machine.port = atoi(optarg); break;
            default: print_usage(argv[0]); exit(1);
        }
    }

    /* LOGIQUE PRINCIPALE D'AJOUT DES MACHINES */

    /* 1. Ajouter la machine LOCALE si :
          - Aucune machine distante n'est spécifiée
          - OU SI l'option -a est présente */
    if (!remote_specified || show_all) {
        machine_t local;
        snprintf(local.name, 64, "LOCAL");
        local.type = CONN_LOCAL;
        local.port = 0;
        local.enabled = 1;
        init_process_list(&local.list);
        add_machine(&mm, local);
    }

    /* 2. Ajouter la machine CLI si définie (-s ou -l) */
    if (strlen(cli_machine.hostname) > 0) {
        /* Demande interactive password si manquant */
        if (strlen(cli_machine.password) == 0 && cli_machine.type == CONN_SSH) {
            get_password_interactive(cli_machine.password, 64);
        }
        cli_machine.enabled = 1;
        init_process_list(&cli_machine.list); // Important: init la liste
        add_machine(&mm, cli_machine);
    }

    /* 3. Charger le fichier config (-c) */
    if (config_path) {
        load_config_file(config_path, &mm);
    } else if (!remote_specified && access(".config", F_OK) != -1) {
        /* Si rien spécifié, on regarde si .config existe par défaut */
        /* NB: Le sujet dit "Lorsqu'aucun chemin de fichier config n'est fourni... cherché sous .config" */
        /* Mais aussi "Lorsqu'aucune option n'est spécifiée... affiche uniquement local". 
           On priorise le local par défaut, mais on charge .config si explicitement demandé ou si on veut étendre.
           Ici, pour respecter "NB", on ne charge .config par défaut que si on veut le comportement "htop complet".
           Pour simplifier et coller au sujet : on charge .config par défaut uniquement si demandé explicitement 
           OU pour suivre la logique "config par défaut".
           Correction selon sujet : "Lorsqu'aucun chemin... fourni, alors cherché... sous .config".
           Je le charge donc, mais la présence de remote_specified détermine l'affichage. */
         load_config_file(".config", &mm);
    }

    /* MODE DRY-RUN */
    if (dry_run) {
        printf("--- MODE DRY-RUN ---\n");
        for (size_t i = 0; i < mm.count; ++i) {
            printf("Connexion à %s (%s)... ", mm.machines[i].name, mm.machines[i].hostname);
            if (mm.machines[i].type == CONN_LOCAL) update_local_processes(&mm.machines[i].list);
            else update_remote_machine(&mm.machines[i]);
            printf("OK (%zu processus trouvés)\n", mm.machines[i].list.count);
        }
        printf("--- FIN DRY-RUN ---\n");
    } else {
        /* MODE UI */
        if (mm.count == 0) {
            fprintf(stderr, "Aucune machine à monitorer. Utilisez -h pour l'aide.\n");
        } else {
            ui_init();
            ui_run(&mm);
            ui_cleanup();
        }
    }

    free_machine_manager(&mm);
    return 0;
}