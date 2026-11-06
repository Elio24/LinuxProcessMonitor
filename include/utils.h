#ifndef UTILS_H
#define UTILS_H

/* Supprime les espaces début/fin */
char *trim_whitespace(char *str);

/* Vérifie les droits 600 sur un fichier */
int check_secure_permissions(const char *path);

#endif