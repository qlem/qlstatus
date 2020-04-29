/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void        set_mem_stat(t_meminfo *meminfo, char *rstat) {
    char    *stat;

    if ((stat = substring(MEM_TOTAL_PATTERN, rstat))) {
        meminfo->total = to_int(stat);
    } else if ((stat = substring(MEM_FREE_PATTERN, rstat))) {
        meminfo->free = to_int(stat);
    } else if ((stat = substring(MEM_BUFFERS_PATTERN, rstat))) {
        meminfo->buffers = to_int(stat);
    } else if ((stat = substring(MEM_CACHED_PATTERN, rstat))) {
        meminfo->cached = to_int(stat);
    }
    free(rstat);
    free(stat);
}

int         get_meminfo(t_meminfo *meminfo) {
    FILE    *stream;
    size_t  size = 0;
    char    *line = NULL;
    ssize_t nb;

    stream = open_stream(PROC_MEMINFO);
    while ((nb = getline(&line, &size, stream)) != -1) {
        line[v_strlen(line) - 1] = 0;
        set_mem_stat(meminfo, line);
        line = NULL;
        size = 0;
        if (meminfo->total > -1 && meminfo->free > -1 && meminfo->buffers > -1 && meminfo->cached > -1) {
            close_stream(stream, PROC_MEMINFO);
            return 0;
        }
    }
    if (nb == -1 && errno) {
        printf("Error reading file '%s': %s\n", PROC_MEMINFO, strerror(errno));
        close_stream(stream, PROC_MEMINFO);
        exit(EXIT_FAILURE);
    }
    close_stream(stream, PROC_MEMINFO);
    return -1;
}

char            *get_memory() {
    char        *token;
    t_meminfo   meminfo;
    long        used;

    meminfo.total = -1;
    meminfo.free = -1;
    meminfo.buffers = -1;
    meminfo.cached = -1;
    if (get_meminfo(&meminfo) == -1) {
        printf("Unable to get memory stats\n");
        exit(EXIT_FAILURE);
    }
    used = meminfo.total - meminfo.free - meminfo.buffers - meminfo.cached;
    token = alloc_buffer(TOKEN_SIZE);
    snprintf(token, TOKEN_SIZE, "%s %ld%%", MEM_LABEL, PERCENT(used, meminfo.total));
    return token;
}
