/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

bool    has_asterisk(char *path) {
    size_t  length;

    length = v_strlen(path);
    if (length >= 2 && path[length - 1] == '*' && path[length - 2] == '/') {
        return true;
    }
    return false;
}

char    *resolve_asterisk(char *path) {
    char            *parent;
    char            *resolved = NULL;
    char            **files;
    size_t          length;
    int             i = -1;

    length = v_strlen(path);
    parent = alloc_buffer(length - 1);
    v_strncpy(parent, path, length - 2);
    files = read_dir(parent, NULL);
    while (files[++i][0]) {
        if (strcmp(files[i], ".") != 0 && strcmp(files[i], "..") != 0) {
            resolved = alloc_buffer(length + v_strlen(files[i]) + 2);
            sprintf(resolved, "%s/%s", parent, files[i]);
            break;
        }
    }
    free_files(files);
    free(parent);
    return resolved;
}

long    compute_temp(char **files, char *parent) {
    char        *path;
    char        *buffer;
    long        temp = 0;
    long        sum = 0;
    long        rem = 0;
    int         i = -1;

    while (files[++i][0]) {
        path = alloc_buffer(v_strlen(parent) + v_strlen(files[i]) + 2);
        sprintf(path, "%s/%s", parent, files[i]);
        buffer = read_file(path);
        sum += to_int(buffer);
        free(path);
        free(buffer);
    }
    temp = sum / i;
    rem = temp % 1000;
    temp = temp / 1000;
    if (rem >= CPU_TEMP_ROUND_THRESHOLD) {
       temp += 1;
    }
    return temp;
}

char    *get_cpu_temp() {
    char        *path;
    char        **files;
    char        *token;
    char        *buffer;
    long        temp = 0;

    if (has_asterisk(CPU_TEMP_DIR)) {
        path = resolve_asterisk(CPU_TEMP_DIR);
    } else {
        path = alloc_buffer(v_strlen(CPU_TEMP_DIR) + 1);
        v_strncpy(path, CPU_TEMP_DIR, v_strlen(CPU_TEMP_DIR));
    }
    files = read_dir(path, CPU_TEMP_INPUT_PATTERN);
    if (!files[0][0]) {
        printf("No input files found in '%s'\n", path);
        free_files(files);
        free(path);
        exit(EXIT_FAILURE);
    }
    temp = compute_temp(files, path);
    buffer = to_str(temp);
    token = alloc_buffer(v_strlen(buffer) + v_strlen(CPU_TEMP_LABEL) + 4);
    sprintf(token, "%s %s°", CPU_TEMP_LABEL, buffer);
    free_files(files);
    free(buffer);
    free(path);
    return token;
}
