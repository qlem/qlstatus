/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

char        *resolve_brightness_file(const char *dir, const char *file) {
    char    *path = NULL;

    path = alloc_buffer(v_strlen(dir) + v_strlen(file) + 2);
    sprintf(path, "%s/%s", dir, file);
    return path;
}

void            *get_brightness(void *data) {
    t_module    *module = data;
    char        *buffer = NULL;
    char        *path = NULL;
    char        *dir = NULL;
    long        current = 0;
    long        max = 0;

    dir = get_opt_string_value(module->opts, OPT_BRG_DIR, BRIGHTNESS_OPTS);
    path = resolve_brightness_file(dir, BRIGHTNESS_CURRENT);
    buffer = read_file(path);
    current = to_int(buffer);
    free(path);
    free(buffer);
    path = resolve_brightness_file(dir, BRIGHTNESS_MAX);
    buffer = read_file(path);
    max = to_int(buffer);
    free(path);
    free(buffer);
    module->value = PERCENT(current, max);
    return NULL;
}
