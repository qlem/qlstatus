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
    long        current = 0;
    long        max = 0;

    buffer = read_file(brg->current_file);
    current = to_int(buffer);
    free(buffer);
    buffer = read_file(brg->max_file);
    max = to_int(buffer);
    free(buffer);
    module->value = PERCENT(current, max);
    return NULL;
}

void            init_brightness(void *data) {
    t_module    *module = data;
    t_brg       *brg = module->data;
    char        *dir = NULL;
    int         i = -1;

    while (++i < BRG_NOPTS) {
        if (strcmp(module->opts[i].key, OPT_BRG_DIR) == 0) {
            dir = module->opts[i].value;
        }
    }
    brg->current_file = resolve_brightness_file(dir, BRG_CURRENT);
    brg->max_file = resolve_brightness_file(dir, BRG_MAX);
}
