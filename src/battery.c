/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void            free_battery(void *data) {
    t_module    *module = data;
    t_power     *power = module->data;

    free(power->file);
}

char        *resolve_power_file(const char *dir, const char *pw_name,
                                const char *file) {
    char    *path = NULL;

    path = alloc_buffer(v_strlen(dir) + v_strlen(pw_name) + v_strlen(file) + 3);
    sprintf(path, "%s/%s/%s", dir, pw_name, file);
    return path;
}

void        set_battery_label(t_module *module, t_power *power) {
    if (strcmp(power->status, BAT_STATUS_DIS) == 0) {
        module->label = power->lb_dis;
        module->critical = module->value <= module->threshold ? 1 : 0;
    } else if (strcmp(power->status, BAT_STATUS_CHR) == 0) {
        module->label = power->lb_chr;
        module->critical = 0;
    } else if (strcmp(power->status, BAT_STATUS_FULL) == 0) {
        module->label = power->lb_full;
        module->critical = 0;
    } else {
        module->label = power->lb_unk;
        module->critical = module->value <= module->threshold ? 1 : 0;
    }
    free(power->status);
}

void        parse_power_line(t_power *power, const char *line) {
    char    *status = NULL;
    char    *current = NULL;
    char    *max = NULL;
    uint8_t full_design = power->full_design;

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

int         parse_power_file(t_power *power) {
    FILE    *stream;
    size_t  size = 0;
    char    *line = NULL;
    size_t  sline;
    ssize_t nb;

    stream = open_stream(power->file);
    while ((nb = getline(&line, &size, stream)) != -1) {
        sline = v_strlen(line);
        if (line[sline - 1] == '\n') {
            line[sline - 1] = 0;
        }
        parse_power_line(power, line);
        free(line);
        line = NULL;
        size = 0;
        if (power->status && power->current > -1 && power->max > -1) {
            close_stream(stream, power->file);
            return 0;
        }
    }
    if (nb == -1 && errno) {
        printf("Error reading file %s: %s\n", power->file, strerror(errno));
        close_stream(stream, power->file);
        exit(EXIT_FAILURE);
    }
    close_stream(stream, power->file);
    return -1;
}

void            *run_battery(void *data) {
    t_module    *module = data;
    t_power     *power = module->data;

    power->max = -1;
    power->current = -1;
    power->status = NULL;
    if (parse_power_file(power) == -1) {
        printf("Cannot compute battery percent: missing statistics\n");
        exit(EXIT_FAILURE);
    }
    module->value = PERCENT(power->current, power->max);
    set_battery_label(module, power);
    return NULL;
}

void            init_battery(void *data) {
    t_module    *module = data;
    t_power     *power = module->data;
    int         i = -1;

    while (++i < BAT_NOPTS) {
        if (strcmp(module->opts[i].key, OPT_BAT_NAME) == 0) {
            power->file = resolve_power_file(POWER_DIR, module->opts[i].value,
                                             POWER_FILE);
        } else if (strcmp(module->opts[i].key, OPT_BAT_LB_CHR) == 0) {
            power->lb_chr = module->opts[i].value;
        } else if (strcmp(module->opts[i].key, OPT_BAT_LB_DIS) == 0) {
            power->lb_dis = module->opts[i].value;
        } else if (strcmp(module->opts[i].key, OPT_BAT_LB_FULL) == 0) {
            power->lb_full = module->opts[i].value;
        } else if (strcmp(module->opts[i].key, OPT_BAT_LB_UNK) == 0) {
            power->lb_unk = module->opts[i].value;
        } else if (strcmp(module->opts[i].key, OPT_BAT_FULL_DESIGN) == 0) {
            power->full_design = ((int *)module->opts[i].value)[0];
        }
    }
}
