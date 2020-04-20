// By Clément Dommerc

#include "qlstatus.h"

char    *get_battery_status() {
    char    *buffer;
    char    *f_status = NULL;

    buffer = read_file(BAT_STATUS);
    if (strcmp(buffer, BAT_STATUS_CHARGING) == 0) {
        f_status = BAT_F_STATUS_CHARGING;
    } else if (strcmp(buffer, BAT_STATUS_DISCHARGING) == 0) {
        f_status = BAT_F_STATUS_DISCHARGING;
    } else if (strcmp(buffer, BAT_STATUS_FULL) == 0) {
        f_status = BAT_F_STATUS_FULL;
    } else {
        f_status = BAT_F_STATUS_UNKNOW;
    }
    free(buffer);
    return f_status;
}

char    *get_battery() {
    const int   base = 10;
    char        *status = NULL;
    char        *buffer = NULL;
    char        *token = NULL;
    long        current = 0;
    long        max = 0;
    long        value = 0;

    status = get_battery_status();
    buffer = read_file(BAT_CURRENT);
    current = strtol(buffer, NULL, base);
    free(buffer);
    buffer = read_file(BAT_MAX);
    max = strtol(buffer, NULL, base);
    free(buffer);
    value = current * CENT / max;
    buffer = int_to_str(value);
    token = alloc_buffer(sizeof(char) * strlen(status) + strlen(buffer) + 3);
    sprintf(token, "%s %s%%", status, buffer);
    free(buffer);
    return token;
}
