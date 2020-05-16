/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void        free_cpu_usage(void *data) {
    (void)data;
}

long        compute_cpu_usage(t_cpu *cpu, const long *stats) {
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
    free(stats);
    return NULL;
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
    long        *stats;

    if ((rstats = parse_cpu_file()) == NULL) {
        printf("Cannot compute cpu usage: missing statistics\n");
        exit(EXIT_FAILURE);
    }
    if ((stats = parse_cpu_stats(rstats)) == NULL) {
        printf("Cannot compute cpu usage: missing statistics\n");
        free(rstats);
        exit(EXIT_FAILURE);
    }
    module->value = compute_cpu_usage(cpu, stats);
    module->critical = module->value >= module->threshold ? 1 : 0;
    free(rstats);
    free(stats);
    return NULL;
}

void        init_cpu_usage(void *data) {
    (void)data;
}
