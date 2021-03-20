/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void        free_cpu_usage(void *data) {
    (void)data;
}

long        compute_cpu_usage(t_cpu *cpu) {
    long    diff_total;
    long    diff_idle;
    long    total = 0;
    long    idle = 0;
    int     i = -1;
    long    usage;

    while (++i < CPU_STATS_SIZE) {
        total += cpu->stats[i];
        if (i == 3 || i == 4) {
            idle += cpu->stats[i];
        }
    }
    diff_idle = idle - cpu->prev_idle;
    diff_total = total - cpu->prev_total;
    usage = PERCENT((diff_total - diff_idle), diff_total);
    cpu->prev_idle = idle;
    cpu->prev_total = total;
    return usage;
}

int         parse_cpu_values(t_cpu *cpu, char *line) {
    char    *token;
    int     i = 0;

    while ((token = strtok(line, " "))) {
        line = NULL;
        if (i > CPU_STATS_SIZE - 1) {
            return 0;
        }
        cpu->stats[i++] = to_int(token);
    }
    return -1;
}

char            *parse_cpu_file() {
    char        *stats = NULL;
    char        *line = NULL;
    size_t      size = 0;
    FILE        *stream;
    ssize_t     nb;

    stream = open_stream(PROC_STAT);
    nb = getline(&line, &size, stream);
    if (nb == -1) {
        if (errno) {
            fprintf(stderr, "Error reading file %s: %s\n", PROC_STAT,
                    strerror(errno));
            exit(EXIT_FAILURE);
        }
        free(line);
        close_stream(stream, PROC_STAT);
        return NULL;
    }
    line[nb - 1] == '\n' ? line[nb - 1] = 0 : 0;
    stats = substring(CPU_STATS_PATTERN, line);
    free(line);
    close_stream(stream, PROC_STAT);
    return stats;
}

void            *run_cpu_usage(void *data) {
    t_module    *module = data;
    t_cpu       *cpu = module->data;
    char        *rstats;
    long        value;

    if ((rstats = parse_cpu_file()) == NULL) {
        fprintf(stderr, "Cannot compute cpu usage: missing values\n");
        exit(EXIT_FAILURE);
    }
    if (parse_cpu_values(cpu, rstats) == -1) {
        fprintf(stderr, "Cannot compute cpu usage: missing values\n");
        exit(EXIT_FAILURE);
    }
    value = compute_cpu_usage(cpu);
    module->critical = value >= cpu->cthreshold ? 1 : 0;
    snprintf(cpu->tokens[1].buffer, TBUFFER_MAX_SIZE, "%ld%%", value);
    set_module_buffer(module, cpu->tokens, CPU_TOKENS);
    free(rstats);
    return NULL;
}

void            init_cpu_usage(void *data) {
    t_module    *module = data;
    t_cpu       *cpu = module->data;

    cpu->tokens[0].fmtid = 'L';
    cpu->tokens[1].fmtid = 'V';
    init_module_tokens(module, cpu->tokens, CPU_TOKENS);

    set_token_buffer(cpu->tokens[0].buffer, module->opts[1].value);
    cpu->cthreshold = ((int *)module->opts[2].value)[0];
}
