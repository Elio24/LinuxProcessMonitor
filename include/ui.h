#ifndef UI_H
#define UI_H

/* On inclut network.h pour avoir accès à la structure machine_manager_t */
#include "network.h"

/* Initialisation de l'environnement ncurses */
void ui_init(void);

/* Nettoyage et fermeture de ncurses */
void ui_cleanup(void);

/* Boucle principale de l'interface */
void ui_run(machine_manager_t *mm);

#endif