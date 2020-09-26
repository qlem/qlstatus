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

void        set_battery_status(t_module *module, t_power *power) {
    if (strcmp(power->raw_status, BAT_STATUS_DIS) == 0) {
        module->label = power->lb_dis;
        module->critical = module->value <= module->threshold ? 1 : 0;
        power->status = module->critical ? PW_CRITICAL : PW_DISCHARGING;
    } else if (strcmp(power->raw_status, BAT_STATUS_CHR) == 0) {
        module->critical = 0;
        module->label = power->lb_chr;
        power->status = PW_CHARGING;
    } else if (strcmp(power->raw_status, BAT_STATUS_FULL) == 0) {
        module->critical = 0;
        module->label = power->lb_full;
        power->status = PW_FULL;
    } else {
        module->label = power->lb_unk;
        module->critical = module->value <= module->threshold ? 1 : 0;
        power->status = module->critical ? PW_CRITICAL : PW_UNKNOWN;
    }
    free(power->raw_status);
}

void        parse_power_line(t_power *power, const char *line) {
    char    *status = NULL;
    char    *current = NULL;
    char    *max = NULL;
    uint8_t full_design = power->full_design;

    if ((status = substring(PW_STATUS_PATTERN, line))) {
        power->raw_status = status;
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
        if (power->raw_status && power->current > -1 && power->max > -1) {
            close_stream(stream, power->file);
            return 0;
        }
    }
    if (nb == -1 && errno) {
        printf("Error reading file %s: %s\n", power->file, strerror(errno));
        close_stream(stream, power->file);
        exit(EXIT_FAILURE);
    }
    free(line);
    close_stream(stream, power->file);
    return -1;
}

void            power_notify(t_power *power) {
    if (power->status != power->last_status) {
        switch (power->status) {
            case PW_FULL:
                notify("Power", BAT_NOTIFY_FULL, power->ic_full,
                                                        NOTIFY_URGENCY_LOW);
                break;
            case PW_CHARGING:
                notify("Power", BAT_NOTIFY_PLUGGED, power->ic_plugged,
                                                        NOTIFY_URGENCY_LOW);
                break;
            case PW_CRITICAL:
                notify("Power", BAT_NOTIFY_LOW, power->ic_low,
                                                    NOTIFY_URGENCY_CRITICAL);
                break;
            default:
                break;
        }
    }
    power->last_status = power->status;
}

void            *run_battery(void *data) {
    t_module    *module = data;
    t_power     *power = module->data;

    power->max = -1;
    power->current = -1;
    power->raw_status = NULL;
    if (parse_power_file(power) == -1) {
        printf("Cannot compute battery percent: missing statistics\n");
        exit(EXIT_FAILURE);
    }
    module->value = PERCENT(power->current, power->max);
    set_battery_status(module, power);
    if (power->notify) {
        power_notify(power);
    }
    return NULL;
}

void            init_battery(void *data) {
    t_module    *module = data;
    t_power     *power = module->data;
    int         i = -1;

    power->status = PW_UNKNOWN;
    power->last_status = PW_UNKNOWN;
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
        } else if (strcmp(module->opts[i].key, OPT_BAT_NOTIFY) == 0) {
            power->notify = ((int *)module->opts[i].value)[0];
        } else if (strcmp(module->opts[i].key, OPT_BAT_NOTIFY_ICON_LOW) == 0) {
            power->ic_low = module->opts[i].value;
        } else if (strcmp(module->opts[i].key, OPT_BAT_NOTIFY_ICON_FULL) == 0) {
            power->ic_full = module->opts[i].value;
        } else if (strcmp(module->opts[i].key,
                          OPT_BAT_NOTIFY_ICON_PLUGGED) == 0) {
            power->ic_plugged = module->opts[i].value;
        }
    }
}
