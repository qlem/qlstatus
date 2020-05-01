/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void            *get_brightness(void *data) {
    t_module    *module = data;
    char        *buffer = NULL;
    long        current = 0;
    long        max = 0;

    buffer = read_file(BRIGHTNESS_CURRENT);
    current = to_int(buffer);
    free(buffer);
    buffer = read_file(BRIGHTNESS_MAX);
    max = to_int(buffer);
    free(buffer);
    module->value = PERCENT(current, max);
    return NULL;
}
