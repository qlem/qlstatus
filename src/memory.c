/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void        free_memory(void *data) {
    (void)data;
}

void        parse_mem_stat(t_mem *mem, char *rstat) {
    char    *stat;

    if ((stat = substring(MEM_TOTAL_PATTERN, rstat))) {
        mem->total = to_int(stat);
    } else if ((stat = substring(MEM_FREE_PATTERN, rstat))) {
        mem->free = to_int(stat);
    } else if ((stat = substring(MEM_BUFFERS_PATTERN, rstat))) {
        mem->buffers = to_int(stat);
    } else if ((stat = substring(MEM_CACHED_PATTERN, rstat))) {
        mem->cached = to_int(stat);
    } else if ((stat = substring(MEM_SRECLAIM_PATTERN, rstat))) {
        mem->sreclaim = to_int(stat);
    }
    free(rstat);
    free(stat);
}

int         parse_mem_file(t_mem *mem) {
    FILE    *stream;
    size_t  size = 0;
    char    *line = NULL;
    size_t  sline;
    ssize_t nb;

    stream = open_stream(PROC_MEMINFO);
    while ((nb = getline(&line, &size, stream)) != -1) {
        sline = v_strlen(line);
        if (line[sline - 1] == '\n') {
            line[sline - 1] = 0;
        }
        parse_mem_stat(mem, line);
        line = NULL;
        size = 0;
        if (mem->total > -1 && mem->free > -1 &&
            mem->buffers > -1 && mem->cached > -1 &&
            mem->sreclaim > -1) {
            close_stream(stream, PROC_MEMINFO);
            return 0;
        }
    }
    if (nb == -1 && errno) {
        printf("Error reading file %s: %s\n", PROC_MEMINFO, strerror(errno));
        close_stream(stream, PROC_MEMINFO);
        exit(EXIT_FAILURE);
    }
    free(line);
    close_stream(stream, PROC_MEMINFO);
    return -1;
}

void            *run_memory(void *data) {
    t_module    *module = data;
    t_mem       *mem = module->data;
    long        used;

    mem->total = -1;
    mem->free = -1;
    mem->buffers = -1;
    mem->cached = -1;
    mem->sreclaim = -1;
    if (parse_mem_file(mem) == -1) {
        printf("Cannot compute memory usage: missing statistics\n");
        exit(EXIT_FAILURE);
    }
    used = mem->total - mem->free - mem->buffers - mem->cached - mem->sreclaim;
    used = PERCENT(used, mem->total);
    module->critical = used >= mem->cthreshold ? 1 : 0;
    v_memset(module->buffer, 0, BUFFER_MAX_SIZE);
    set_generic_module_buffer(module, used, mem->label, "%");
    return NULL;
}

void            init_memory(void *data) {
    t_module    *module = data;
    t_mem       *mem = module->data;
    int         i = -1;

    while (++i < MEM_NOPTS) {
        if (strcmp(module->opts[i].key, OPT_MEM_LABEL) == 0) {
            mem->label = module->opts[i].value;
        } else if (strcmp(module->opts[i].key, OPT_MEM_CRITICAL) == 0) {
            mem->cthreshold = ((int *)module->opts[i].value)[0];
        }
    }
}
