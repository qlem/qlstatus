/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

char    *get_brightness() {
    char    *token = NULL;
    char    *buffer = NULL;
    long    current = 0;
    long    max = 0;
    long    value = 0;

    buffer = read_file(BRIGHTNESS_CURRENT);
    current = to_int(buffer);
    free(buffer);
    buffer = read_file(BRIGHTNESS_MAX);
    max = to_int(buffer);
    free(buffer);
    value = PERCENT(current, max);
    token = alloc_buffer(TOKEN_SIZE);
    snprintf(token, TOKEN_SIZE, "%s %ld%%", BRIGHTNESS_LABEL, value);
    return token;
}
