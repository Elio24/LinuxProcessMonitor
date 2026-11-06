#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "ui.h"
#include "process.h"

/* Variables globales pour la recherche (F4) */
static char filter_str[64] = {0};
static int filter_active = 0;

void ui_init(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    
    init_pair(1, COLOR_BLACK, COLOR_CYAN); /* Header */
    init_pair(2, COLOR_WHITE, COLOR_BLUE); /* Selection */
    init_pair(3, COLOR_RED, COLOR_BLACK);  /* Alerte/Info */
    
    timeout(1000);
}

void ui_cleanup(void) {
    endwin();
}

void draw_tabs(machine_manager_t *mm, int current_tab) {
    size_t i;
    move(0, 0);
    for (i = 0; i < mm->count; ++i) {
        if ((int)i == current_tab) attron(A_BOLD | COLOR_PAIR(2));
        printw(" [%s] ", mm->machines[i].name);
        if ((int)i == current_tab) attroff(A_BOLD | COLOR_PAIR(2));
    }
    /* Affichage de l'état du filtre */
    if (filter_active) {
        attron(COLOR_PAIR(3));
        printw(" [FILTRE: %s]", filter_str);
        attroff(COLOR_PAIR(3));
    }
    clrtoeol();
}

void draw_process_list(process_list_t *list, int *selected_idx, int start_idx, int max_rows) {
    int i;
    int display_row = 0;
    
    /* Header */
    attron(COLOR_PAIR(1));
    mvprintw(2, 0, "%-8s %-10s %-6s %-6s %-6s %s", "PID", "USER", "S", "CPU%", "MEM%", "COMMAND");
    clrtoeol();
    attroff(COLOR_PAIR(1));

    /* Protection index */
    if (*selected_idx >= (int)list->count) *selected_idx = list->count - 1;
    if (*selected_idx < 0) *selected_idx = 0;

    for (i = 0; i < (int)list->count; ++i) {
        process_info_t *p = &list->procs[i];

        /* Logique de filtre (F4) */
        if (filter_active && filter_str[0] != '\0') {
            if (!strstr(p->command, filter_str) && !strstr(p->user, filter_str)) {
                continue; /* On passe si ça ne correspond pas */
            }
        }

        /* Gestion du scroll */
        if (display_row < start_idx) {
            display_row++;
            continue;
        }
        if (display_row >= start_idx + max_rows) break;

        int screen_y = 3 + (display_row - start_idx);
        
        if (i == *selected_idx) attron(COLOR_PAIR(2));
        mvprintw(screen_y, 0, "%-8d %-10s %-6c %-6.1f %-6.1f %s", 
            p->pid, p->user, p->state, p->cpu_usage, p->mem_usage, p->command);
        clrtoeol();
        if (i == *selected_idx) attroff(COLOR_PAIR(2));
        
        display_row++;
    }
}

void ui_run(machine_manager_t *mm) {
    int running = 1;
    int current_tab = 0;
    int selected_idx = 0;
    int start_display_idx = 0;

    while (running) {
        machine_t *cm = &mm->machines[current_tab];
        int max_y, max_x;
        int ch;

        /* Update données */
        if (cm->type == CONN_LOCAL) update_local_processes(&cm->list);
        else update_remote_machine(cm);

        erase();
        draw_tabs(mm, current_tab);
        getmaxyx(stdscr, max_y, max_x);
        (void)max_x;

        /* Footer complet conforme au sujet */
        mvprintw(max_y - 2, 0, "F1:Aide F2/F3:Onglets F4:Recherche F5:Pause");
        mvprintw(max_y - 1, 0, "F6:Arret F7:Tuer F8:Redemarrer F10:Quitter");

        draw_process_list(&cm->list, &selected_idx, start_display_idx, max_y - 5);
        refresh();

        ch = getch();
        switch (ch) {
            case KEY_F(1): /* F1: Aide */
                mvprintw(max_y/2, 10, "Aide: F4=Filtre, F5=SIGSTOP, F6=SIGTERM, F7=SIGKILL");
                break;
            case KEY_F(2): /* F2: Suivant */
                current_tab = (current_tab + 1) % mm->count;
                selected_idx = 0;
                break;
            case KEY_F(3): /* F3: Précédent */
                current_tab = (current_tab - 1 + mm->count) % mm->count;
                selected_idx = 0;
                break;
            case KEY_F(4): /* F4: Rechercher */
                echo();
                curs_set(1);
                mvprintw(max_y - 3, 0, "Filtre (vide pour effacer): ");
                getnstr(filter_str, 63);
                filter_active = (strlen(filter_str) > 0);
                noecho();
                curs_set(0);
                break;
            case KEY_DOWN:
                if (selected_idx < (int)cm->list.count - 1) selected_idx++;
                if (selected_idx > start_display_idx + (max_y - 6)) start_display_idx++;
                break;
            case KEY_UP:
                if (selected_idx > 0) selected_idx--;
                if (selected_idx < start_display_idx) start_display_idx--;
                break;
            
            /* ACTIONS SUR PROCESSUS */
            case KEY_F(5): /* F5: Pause (SIGSTOP) */
                if (cm->list.count > 0) send_signal_process(cm->list.procs[selected_idx].pid, SIGSTOP);
                break;
            case KEY_F(6): /* F6: Arrêt (SIGTERM) */
                if (cm->list.count > 0) send_signal_process(cm->list.procs[selected_idx].pid, SIGTERM);
                break;
            case KEY_F(7): /* F7: Tuer (SIGKILL) - Ajout manquant */
                if (cm->list.count > 0) send_signal_process(cm->list.procs[selected_idx].pid, SIGKILL);
                break;
            case KEY_F(8): /* F8: Redémarrer (SIGHUP) - Ajout manquant */
                if (cm->list.count > 0) send_signal_process(cm->list.procs[selected_idx].pid, SIGHUP);
                break;
            
            case KEY_F(10): /* F10: Quitter */
                running = 0;
                break;
            default:
                break;
        }
    }
}