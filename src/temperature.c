/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void            free_temperature(void *data) {
    t_module    *module = data;
    t_temp      *temp = module->data;

    free_files(temp->inputs);
}

bool        has_asterisk(const char *path) {
    size_t  length;

    length = v_strlen(path);
    if (length >= 2 && path[length - 1] == '*' && path[length - 2] == '/') {
        return true;
    }
    return false;
}

char        *resolve_temp_dir(const char *path) {
    char    *resolved = NULL;
    char    **files = NULL;
    char    *parent = NULL;
    size_t  size;

    size = v_strlen(path);
    if (!has_asterisk(path)) {
        resolved = alloc_buffer(size + 1);
        v_strncpy(resolved, path, size);
        return resolved;
    }
    parent = alloc_buffer(size);
    v_strncpy(parent, path, size - 1);
    files = read_dir(parent, NULL);
    if (!files[0][0]) {
        fprintf(stderr, "Cannot resolve temp directory %s\n", path);
        free_files(files);
        free(parent);
        exit(EXIT_FAILURE);
    }
    size = v_strlen(files[0]);
    resolved = alloc_buffer(size + 1);
    v_strncpy(resolved, files[0], size);
    free_files(files);
    free(parent);
    return resolved;
}

char        *resolve_temp_input_regex(const char *input) {
    char    *start = "^temp[";
    char    *end = "]_input$";
    char    *regex = NULL;
    size_t  start_len;
    size_t  end_len;
    size_t  in_len;
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

long        compute_temp(char **files) {
    char    *buffer;
    long    sum = 0;
    int     i = -1;
    long    temp;
    long    rem;

    while (files[++i][0]) {
        buffer = read_file(files[i]);
        sum += to_int(buffer);
        free(buffer);
    }
    temp = sum / i;
    rem = temp % 1000;
    temp = temp / 1000;
    if (rem >= TEMP_ROUND_THRESHOLD) {
        temp += 1;
    }
    return temp;
}

void            *run_temperature(void *data) {
    t_module    *module = data;
    t_temp      *temp = module->data;
    long        value;

    value = compute_temp(temp->inputs);
    module->critical = value >= temp->cthreshold ? 1 : 0;
    set_token_buffer(temp->tokens[0].buffer, temp->label);
    snprintf(temp->tokens[1].buffer, TBUFFER_MAX_SIZE, "%ld°", value);
    set_module_buffer(module, temp->tokens, TEMP_TOKENS);
    return NULL;
}

void            init_temperature(void *data) {
    t_module    *module = data;
    t_temp      *temp = module->data;
    char        *in_regex = NULL;
    char        *dir = NULL;

    temp->tokens[0].fmtid = 'L';
    temp->tokens[1].fmtid = 'V';
    init_module_tokens(module, temp->tokens, TEMP_TOKENS);

    temp->label = module->opts[1].value;
    dir = resolve_temp_dir(module->opts[2].value);
    in_regex = resolve_temp_input_regex(module->opts[3].value);
    temp->cthreshold = ((int *)module->opts[4].value)[0];

    temp->inputs = read_dir(dir, in_regex);
    free(in_regex);
    if (!temp->inputs[0][0]) {
        fprintf(stderr, "Cannot resolve temp dir, no input files found in %s\n",
                dir);
        free_files(temp->inputs);
        free(dir);
        exit(EXIT_FAILURE);
    }
    free(dir);
}
