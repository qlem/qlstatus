/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

char        *resolve_power_file(const char *dir, const char *pw_name,
                                const char *file) {
    char    *path = NULL;

    path = alloc_buffer(v_strlen(dir) + v_strlen(pw_name) + v_strlen(file) + 3);
    sprintf(path, "%s/%s/%s", dir, pw_name, file);
    return path;
}

void        set_battery_label(t_module *module, t_power *power) {
    if (strcmp(power->status, BAT_STATUS_DIS) == 0) {
        module->label = get_opt_string_value(module->opts, OPT_BAT_LB_DIS,
                                             BATTERY_OPTS);
        module->critical = module->value <= module->threshold ? 1 : 0;
    } else if (strcmp(power->status, BAT_STATUS_CHR) == 0) {
        module->label = get_opt_string_value(module->opts, OPT_BAT_LB_CHR,
                                             BATTERY_OPTS);
        module->critical = 0;
    } else if (strcmp(power->status, BAT_STATUS_FULL) == 0) {
        module->label = get_opt_string_value(module->opts, OPT_BAT_LB_FULL,
                                             BATTERY_OPTS);
        module->critical = 0;
    } else {
        module->label = get_opt_string_value(module->opts, OPT_BAT_LB_UNK,
                                             BATTERY_OPTS);
        module->critical = module->value <= module->threshold ? 1 : 0;
    }
}

void        parse_power_line(t_power *power, const char *line,
                             uint8_t full_design) {
    char    *status = NULL;
    char    *current = NULL;
    char    *max = NULL;

    if ((status = substring(PW_STATUS_PATTERN, line))) {
        power->status = status;
    } else if ((full_design && (max = substring(PW_MAX_FD_PATTERN, line))) ||
               (!full_design && (max = substring(PW_MAX_PATTERN, line)))) {
        power->max = to_int(max);
        free(max);
    } else if ((current = substring(PW_CURRENT_PATTERN, line))) {
        power->current = to_int(current);
        free(current);
    }
}

int         parse_power_file(t_power *power, const char *file,
                             uint8_t full_design) {
    FILE    *stream;
    size_t  size = 0;
    char    *line = NULL;
    size_t  sline;
    ssize_t nb;

    stream = open_stream(file);
    while ((nb = getline(&line, &size, stream)) != -1) {
        sline = v_strlen(line);
        if (line[sline - 1] == '\n') {
            line[sline - 1] = 0;
        }
        parse_power_line(power, line, full_design);
        free(line);
        line = NULL;
        size = 0;
        if (power->status && power->current > -1 && power->max > -1) {
            close_stream(stream, file);
            return 0;
        }
    }
    if (nb == -1 && errno) {
        printf("Error reading file %s: %s\n", file, strerror(errno));
        close_stream(stream, file);
        exit(EXIT_FAILURE);
    }
    close_stream(stream, file);
    return -1;
}

void            *get_battery(void *data) {
    t_module    *module = data;
    t_power     power;
    char        *file = NULL;
    char        *bat = NULL;
    uint8_t     full_design;

    power.max = -1;
    power.current = -1;
    power.status = NULL;
    bat = get_opt_string_value(module->opts, OPT_BAT_NAME, BATTERY_OPTS);
    full_design = get_opt_number_value(module->opts, OPT_BAT_FULL_DESIGN,
                                       BATTERY_OPTS);
    file = resolve_power_file(POWER_DIR, bat, POWER_FILE);
    if (parse_power_file(&power, file, full_design) == -1) {
        printf("Cannot compute battery percent\n");
        free(file);
        exit(EXIT_FAILURE);
    }
    module->value = PERCENT(power.current, power.max);
    set_battery_label(module, &power);
    free(power.status);
    free(file);
    return NULL;
}
