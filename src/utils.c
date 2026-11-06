#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include "utils.h"

char *trim_whitespace(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

int check_secure_permissions(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        perror("stat");
        return 0;
    }
    /* Vérifie si les droits sont exactement 600 (rw-------) */
    if ((st.st_mode & 0777) != 0600) {
        return 0;
    }
    return 1;
}