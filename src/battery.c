/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

char    *get_battery_status() {
    char    *buffer;
    char    *status = NULL;

    buffer = read_file(BAT_STATUS);
    if (strcmp(buffer, BAT_STATUS_CHARGING) == 0) {
        status = BAT_LABEL_CHARGING;
    } else if (strcmp(buffer, BAT_STATUS_DISCHARGING) == 0) {
        status = BAT_LABEL_DISCHARGING;
    } else if (strcmp(buffer, BAT_STATUS_FULL) == 0) {
        status = BAT_LABEL_FULL;
    } else {
        status = BAT_LABEL_UNKNOW;
    }
    free(buffer);
    return status;
}

char    *get_battery() {
    char        *token = NULL;
    char        *status = NULL;
    char        *buffer = NULL;
    long        current = 0;
    long        max = 0;
    long        value = 0;

    status = get_battery_status();
    buffer = read_file(BAT_CURRENT);
    current = to_int(buffer);
    free(buffer);
    buffer = read_file(BAT_MAX);
    max = to_int(buffer);
    free(buffer);
    value = PERCENT(current, max);
    token = alloc_buffer(TOKEN_SIZE);
    snprintf(token, TOKEN_SIZE, "%s %ld%%", status, value);
    return token;
}
