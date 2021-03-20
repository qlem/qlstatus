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
    if (power->notify.enabled) {
        g_object_unref(G_OBJECT(power->notify.notify));
    }
}

char        *resolve_power_file(const char *dir, const char *pw_name,
                                const char *file) {
    char    *path = NULL;

    path = alloc_buffer(v_strlen(dir) + v_strlen(pw_name) + v_strlen(file) + 3);
    sprintf(path, "%s/%s/%s", dir, pw_name, file);
    return path;
}

static void         set_buffer(t_module *module, t_power *power) {
    int             value;

    module->critical = 0;
    if (!power->raw_status || power->current < -1 || power->max < -1) {
        set_token_buffer(power->tokens[0].buffer, power->lb_dis);
        set_token_buffer(power->tokens[1].buffer, "--%");
        power->status = PW_UNKNOWN;
        return;
    }
    value = PERCENT(power->current, power->max);
    snprintf(power->tokens[1].buffer, TBUFFER_MAX_SIZE, "%d%%", value);
    if (strcmp(power->raw_status, BAT_STATUS_DIS) == 0) {
        set_token_buffer(power->tokens[0].buffer, power->lb_dis);
        power->status = value <= power->cthreshold ? PW_CRITICAL
                                                   : PW_DISCHARGING;
        module->critical = power->status == PW_CRITICAL ? 1 : 0;
    } else if (strcmp(power->raw_status, BAT_STATUS_CHR) == 0) {
        set_token_buffer(power->tokens[0].buffer, power->lb_chr);
        power->status = PW_CHARGING;
    } else if (strcmp(power->raw_status, BAT_STATUS_FULL) == 0) {
        set_token_buffer(power->tokens[0].buffer, power->lb_full);
        power->status = PW_FULL;
    } else {
        set_token_buffer(power->tokens[0].buffer, power->lb_unk);
        power->status = value <= power->cthreshold ? PW_CRITICAL : PW_UNKNOWN;
        module->critical = power->status == PW_CRITICAL ? 1 : 0;
    }
    set_module_buffer(module, power->tokens, BAT_TOKENS);
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
    ssize_t nb;

    stream = open_stream(power->file);
    while ((nb = getline(&line, &size, stream)) != -1) {
        line[nb - 1] == '\n' ? line[nb - 1] = 0 : 0;
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
        fprintf(stderr, "Error reading file %s: %s\n", power->file,
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    free(line);
    close_stream(stream, power->file);
    return 0;
}

void            power_notify(t_power *power) {
    switch (power->status) {
        case PW_FULL:
            notify(power->notify.notify, "Power", BAT_NOTIFY_FULL,
                   power->notify.ic_full, NOTIFY_URGENCY_NORMAL);
            break;
        case PW_CHARGING:
            notify(power->notify.notify, "Power", BAT_NOTIFY_PLUGGED,
                   power->notify.ic_plugged, NOTIFY_URGENCY_NORMAL);
            break;
        case PW_CRITICAL:
            notify(power->notify.notify, "Power", BAT_NOTIFY_LOW,
                   power->notify.ic_low, NOTIFY_URGENCY_CRITICAL);
            break;
        default:
            break;
    }
}

void            *run_battery(void *data) {
    t_module    *module = data;
    t_power     *power = module->data;

    power->max = -1;
    power->current = -1;
    power->raw_status = NULL;
    parse_power_file(power);
    set_buffer(module, power);
    free(power->raw_status);
    if (power->notify.enabled && power->status != power->last_status) {
        power_notify(power);
    }
    power->last_status = power->status;
    return NULL;
}

void            init_battery(void *data) {
    t_module    *module = data;
    t_power     *power = module->data;

    power->status = PW_UNKNOWN;
    power->last_status = PW_UNKNOWN;

    power->tokens[0].fmtid = 'L';
    power->tokens[1].fmtid = 'V';
    init_module_tokens(module, power->tokens, BAT_TOKENS);

    power->lb_unk = module->opts[1].value;
    power->lb_full = module->opts[2].value;
    power->lb_chr = module->opts[3].value;
    power->lb_dis = module->opts[4].value;
    power->file = resolve_power_file(POWER_DIR, module->opts[5].value,
                                     POWER_FILE);
    power->cthreshold = ((int *)module->opts[6].value)[0];
    power->full_design = ((int *)module->opts[7].value)[0];
    power->notify.enabled = ((int *)module->opts[8].value)[0];
    power->notify.ic_full = module->opts[9].value;
    power->notify.ic_plugged = module->opts[10].value;
    power->notify.ic_low = module->opts[11].value;

    if (power->notify.enabled) {
        power->notify.notify = notify_new("Power");
    }
}
