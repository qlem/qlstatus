/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

long    compute_cpu_usage(t_cpu *cpu, const long *stats) {
    long    idle = 0;
    long    total = 0;
    long    usage = 0;
    long    diff_idle = 0;
    long    diff_total = 0;
    int     i = -1;

    while (++i < CPU_STATS_SIZE) {
        total += stats[i];
        if (i == 3 || i == 4) {
            idle += stats[i];
        }
    }
    diff_idle = idle - cpu->prev_idle;
    diff_total = total - cpu->prev_total;
    usage = PERCENT((diff_total - diff_idle), diff_total);
    cpu->prev_idle = idle;
    cpu->prev_total = total;
    return usage;
}

long        *parse_cpu_stats(char *line) {
    long    *stats = NULL;
    char    *token;
    long    stat;
    int     i = 0;

    stats = alloc_ptr(sizeof(long) * CPU_STATS_SIZE);
    while ((token = strtok(line, " "))) {
        line = NULL;
        if (i > CPU_STATS_SIZE - 1) {
            return stats;
        }
        stat = to_int(token);
        stats[i++] = stat;
    }
    return stats;
}

char        *parse_cpu_file() {
    FILE    *stream;
    size_t  size = 0;
    char    *line = NULL;
    char    *stats = NULL;

    stream = open_stream(PROC_STAT);
    if (getline(&line, &size, stream) == -1) {
        if (errno) {
            printf("Cannot read file %s: %s\n", PROC_STAT, strerror(errno));
        } else {
            printf("Cannot compute cpu usage\n");
        }
        close_stream(stream, PROC_STAT);
        exit(EXIT_FAILURE);
    }
    if (line[v_strlen(line) - 1] == '\n') {
        line[v_strlen(line) - 1] = 0;
    }
    stats = substring(CPU_STATS_PATTERN, line);
    free(line);
    close_stream(stream, PROC_STAT);
    return stats;
}

void            *get_cpu_usage(void *data) {
    t_module    *module = data;
    t_cpu       *cpu = module->data;
    char        *rstats;
    long        *stats;

    if ((rstats = parse_cpu_file()) == NULL) {
        printf("Cannot compute cpu usage\n");
        exit(EXIT_FAILURE);
    }
    stats = parse_cpu_stats(rstats);
    module->value = compute_cpu_usage(cpu, stats);
    free(rstats);
    free(stats);
    return NULL;
}
