/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

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

bool    has_asterisk(char *path) {
    size_t  length;

    length = v_strlen(path);
    if (length >= 2 && path[length - 1] == '*' && path[length - 2] == '/') {
        return true;
    }
    return false;
}

char        *resolve_temp_input_regex(const char *input) {
    char    *regex = NULL;
    char    *start = "^temp[";
    char    *end = "]_input$";
    size_t  in_len = 0;
    size_t  start_len = 0;
    size_t  end_len = 0;
    int     i = -1;

    in_len = v_strlen(input);
    start_len = v_strlen(start);
    end_len = v_strlen(end);
    regex = alloc_buffer(start_len + end_len + in_len + 1);
    while (++i < (int)start_len) {
        regex[i] = start[i];
    }
    i--;
    while (++i < (int)(start_len + in_len)) {
        regex[i] = input[i - start_len];
    }
    i--;
    while (++i < (int)(start_len + in_len + end_len)) {
        regex[i] = end[i - start_len - in_len];
    }
    return regex;
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

void            *get_cpu_temp(void *data) {
    t_module    *module = data;
    char        *path;
    char        *rpath;
    char        *in;
    char        *in_regex;
    char        **files;

    path = get_option_value(module->opts, OPT_TCPU_DIR, CPU_TEMP_OPTS);
    if (has_asterisk(path)) {
        rpath = resolve_asterisk(path);
    } else {
        rpath = alloc_buffer(v_strlen(path) + 1);
        v_strncpy(rpath, path, v_strlen(path));
    }
    in = get_option_value(module->opts, OPT_TCPU_INPUT, CPU_TEMP_OPTS);
    in_regex = resolve_temp_input_regex(in);
    files = read_dir(rpath, in_regex);
    if (!files[0][0]) {
        printf("Cannot compute cpu temp, no input files found in %s\n", rpath);
        free(in_regex);
        free_files(files);
        free(rpath);
        exit(EXIT_FAILURE);
    }
    module->value = compute_temp(files, rpath);
    free(in_regex);
    free_files(files);
    free(rpath);
    return NULL;
}
