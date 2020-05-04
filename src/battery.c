/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

char            *get_path(const char *dir, const char *pw_name,
                          const char *file) {
    char        *path = NULL;

    path = alloc_buffer(v_strlen(dir) + v_strlen(pw_name) + v_strlen(file) + 3);
    sprintf(path, "%s/%s/%s", dir, pw_name, file);
    return path;
}

char    *get_battery_status() {
    char    *buffer = NULL;
    char    *path = NULL;
    char    *status = NULL;

    path = get_path(POWER_DIR, BATTERY_NAME, BATTERY_STATUS_FILE);
    buffer = read_file(path);
    free(path);
    if (strcmp(buffer, BATTERY_STATUS_CHARGING) == 0) {
        status = BATTERY_LABEL_CHR;
    } else if (strcmp(buffer, BATTERY_STATUS_DISCHARGING) == 0) {
        status = BATTERY_LABEL_DIS;
    } else if (strcmp(buffer, BATTERY_STATUS_FULL) == 0) {
        status = BATTERY_LABEL_FULL;
    } else {
        status = BATTERY_LABEL_UNK;
    }
    free(buffer);
    return status;
}

void            *get_battery(void *data) {
    t_module    *module = data;
    char        *path = NULL;
    char        *buffer = NULL;
    long        current = 0;
    long        max = 0;

    module->label = get_battery_status();
    path = get_path(POWER_DIR, BATTERY_NAME, BATTERY_CURRENT_FILE);
    buffer = read_file(path);
    current = to_int(buffer);
    free(buffer);
    free(path);
    path = get_path(POWER_DIR, BATTERY_NAME, BATTERY_MAX_FILE);
    buffer = read_file(path);
    max = to_int(buffer);
    free(buffer);
    free(path);
    module->value = PERCENT(current, max);
    return NULL;
}
