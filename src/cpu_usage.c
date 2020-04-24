// By Clément Dommerc

#include "qlstatus.h"

long        *parse_line(char *line) {
    long    *stats;
    char    *token;
    long    tmp;
    int     i = 0;

    stats = alloc_ptr(sizeof(long) * TEN);
    strtok(line, " ");
    while ((token = strtok(NULL, " "))) {
        tmp = to_int(token);
        stats[i++] = tmp;
    }
    // TODO error handling here
    return stats;
}

char        *get_line() {
    FILE    *stream;
    char    **lines;
    size_t  *sizes;
    char    *line;

    if ((stream = fopen(PROC_STAT, "r")) == NULL) {
        printf("Cannot open file '%s': %s\n", PROC_STAT, strerror(errno));
        exit(EXIT_FAILURE);
    }
    lines = alloc_ptr(sizeof(char *));
    sizes = alloc_ptr(sizeof(size_t));
    lines[0] = NULL;
    sizes[0] = 0;
    if ((getline(lines, sizes, stream)) == -1 && errno) {
        printf("Cannot read file '%s': %s\n", PROC_STAT, strerror(errno));
        exit(EXIT_FAILURE);
    }
    lines[0][v_strlen(lines[0]) - 1] = 0;
    line = lines[0];
    free(lines);
    free(sizes);
    if (fclose(stream) != 0) {
        printf("Cannot close file '%s': %s\n", PROC_STAT, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return line;
}

long    compute_usage(t_cpu *cpu, const long *stats) {
    long    idle = 0;
    long    total = 0;
    long    usage = 0;
    long    diff_idle = 0;
    long    diff_total = 0;
    int     i = -1;

    while (++i < TEN) {
        total += stats[i];
        if (i == 3 || i == 4) {
            idle += stats[i];
        }
    }
    diff_idle = idle - cpu->prev_idle;
    diff_total = total - cpu->prev_total;
    usage = (THOUSAND * (diff_total - diff_idle) / diff_total) / TEN;
    cpu->prev_idle = idle;
    cpu->prev_total = total;
    return usage;
}

char    *get_cpu_usage(t_cpu *cpu) {
    char    *token;
    char    *buffer;
    char    *line;
    long    *stats;
    long    usage;

    line = get_line();
    stats = parse_line(line);
    usage = compute_usage(cpu, stats);
    buffer = to_str(usage);
    token = alloc_buffer(sizeof(char) * (v_strlen(buffer) +
                v_strlen(CPU_USAGE_LABEL) + 3));
    sprintf(token, "%s %s%%", CPU_USAGE_LABEL, buffer);
    free(stats);
    free(line);
    free(buffer);
    return token;
}
