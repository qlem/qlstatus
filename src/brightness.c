/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void            free_brightness(void *data) {
    t_module    *module = data;
    t_brg       *brg = module->data;

    free(brg->current_file);
    free(brg->max_file);
}

char        *resolve_brightness_file(const char *dir, const char *file) {
    char    *path = NULL;

    path = alloc_buffer(v_strlen(dir) + v_strlen(file) + 2);
    sprintf(path, "%s/%s", dir, file);
    return path;
}

void            *run_brightness(void *data) {
    t_module    *module = data;
    t_brg       *brg = module->data;
    char        *buffer = NULL;
    long        current;
    long        max;

    buffer = read_file(brg->current_file);
    current = to_int(buffer);
    free(buffer);
    buffer = read_file(brg->max_file);
    max = to_int(buffer);
    free(buffer);
    set_token_buffer(brg->tokens[0].buffer, brg->label);
    snprintf(brg->tokens[1].buffer, TBUFFER_MAX_SIZE, "%ld%%",
             PERCENT(current, max));
    set_module_buffer(module, brg->tokens, BRG_TOKENS);
    return NULL;
}

void            init_brightness(void *data) {
    t_module    *module = data;
    t_brg       *brg = module->data;
    char        *dir = NULL;

    brg->tokens[0].fmtid = 'L';
    brg->tokens[1].fmtid = 'V';
    init_module_tokens(module, brg->tokens, BRG_TOKENS);

    brg->label = module->opts[1].value;
    dir = module->opts[2].value;

    brg->current_file = resolve_brightness_file(dir, BRG_CURRENT);
    brg->max_file = resolve_brightness_file(dir, BRG_MAX);
}
