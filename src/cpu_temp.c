// By Clément Dommerc

#include "qlstatus.h"

bool    has_asterisk(char *path) {
    size_t  length;

    length = strlen(path);
    if (length >= 2 && path[length - 1] == '*' && path[length - 2] == '/') {
        return true;
    }
    return false;
}

char    *resolve_asterisk(char *path) {
    DIR             *dir;
    char            *parent;
    char            *resolved;
    char            **files;
    size_t          length;
    int             i = -1;

    length = strlen(path);
    parent = alloc_buffer(length - 1);
    if (strncpy(parent, path, length - 2) == NULL) {
        exit(1);
    }
    dir = open_dir(parent);
    files = read_dir(dir, NULL);
    while (files[++i][0]) {
        if (strcmp(files[i], ".") != 0 && strcmp(files[i], "..") != 0) {
            resolved = alloc_buffer(sizeof(char) * (length +
                        strlen(files[i]) + 2));
            sprintf(resolved, "%s/%s", parent, files[i]);
            free(parent);
            free_files(files);
            close_dir(dir);
            return resolved;
        }
    }
    close_dir(dir);
    return NULL;
}

long    compute_temp(char **files, char *parent) {
    char        *path;
    char        *buffer;
    long        temp = 0;
    long        sum = 0;
    long        rem = 0;
    int         i = -1;

    while (files[++i][0]) {
        path = alloc_buffer(strlen(parent) + strlen(files[i]) + 2);
        sprintf(path, "%s/%s", parent, files[i]);
        buffer = read_file(path);
        sum += strtol(buffer, NULL, BASE);
        free(path);
        free(buffer);
    }
    if (!sum) {
        free_files(files);
        return 0;
    }
    temp = sum / i;
    rem = temp % THOUSAND;
    temp = temp / THOUSAND;
    if (rem >= CPU_TEMP_ROUND) {
       temp += 1;
    }
    free_files(files);
    return temp;
}

char    *get_cpu_temp() {
    DIR         *dir;
    char        **files;
    char        *token;
    char        *buffer;
    long        temp = 0;

    if (has_asterisk(CPU_TEMP_DIR)) {
        buffer = resolve_asterisk(CPU_TEMP_DIR);
    } else {
        buffer = CPU_TEMP_DIR;
    }
    dir = open_dir(buffer);
    files = read_dir(dir, CPU_TEMP_INPUT_PATTERN);
    temp = compute_temp(files, buffer);
    free(buffer);
    buffer = int_to_str(temp);
    token = alloc_buffer(sizeof(char ) * (strlen(buffer) +
                strlen(CPU_TEMP_LABEL) + 4));
    sprintf(token, "%s %s°", CPU_TEMP_LABEL, buffer);
    free(buffer);
    close_dir(dir);
    return token;
}
