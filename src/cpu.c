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
    long    idle = 0;
    long    total = 0;
    long    usage = 0;
    long    diff_idle = 0;
    long    diff_total = 0;
    int     i = -1;

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

int         parse_cpu_stats(t_cpu *cpu, char *line) {
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

char        *parse_cpu_file() {
    FILE    *stream;
    size_t  size = 0;
    char    *line = NULL;
    size_t  sline;
    char    *stats = NULL;
    ssize_t nb = 0;

    stream = open_stream(PROC_STAT);
    nb = getline(&line, &size, stream);
    if (nb == -1) {
        if (errno) {
            printf("Error reading file %s: %s\n", PROC_STAT, strerror(errno));
            close_stream(stream, PROC_STAT);
            exit(EXIT_FAILURE);
        }
        free(line);
        close_stream(stream, PROC_STAT);
        return NULL;
    }
    sline = v_strlen(line);
    if (line[sline - 1] == '\n') {
        line[sline - 1] = 0;
    }
    stats = substring(CPU_STATS_PATTERN, line);
    free(line);
    close_stream(stream, PROC_STAT);
    return stats;
}

void            *run_cpu_usage(void *data) {
    t_module    *module = data;
    t_cpu       *cpu = module->data;
    char        *rstats;

    if ((rstats = parse_cpu_file()) == NULL) {
        printf("Cannot compute cpu usage: missing statistics\n");
        exit(EXIT_FAILURE);
    }
    if (parse_cpu_stats(cpu, rstats) == -1) {
        printf("Cannot compute cpu usage: missing statistics\n");
        free(rstats);
        exit(EXIT_FAILURE);
    }
    module->value = compute_cpu_usage(cpu);
    module->critical = module->value >= module->threshold ? 1 : 0;
    free(rstats);
    return NULL;
}

void        init_cpu_usage(void *data) {
    (void)data;
}
