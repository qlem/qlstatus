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

int             parse_mem_file(t_mem *mem) {
    char        *line = NULL;
    size_t      size = 0;
    FILE        *stream;
    ssize_t     nb;

    stream = open_stream(PROC_MEMINFO);
    while ((nb = getline(&line, &size, stream)) != -1) {
        line[nb - 1] == '\n' ? line[nb - 1] = 0 : 0;
        parse_mem_stat(mem, line);
        line = NULL;
        size = 0;
        if (mem->total > -1 && mem->free > -1 && mem->buffers > -1 &&
            mem->cached > -1 && mem->sreclaim > -1) {
            close_stream(stream, PROC_MEMINFO);
            return 0;
        }
    }
    if (nb == -1 && errno) {
        fprintf(stderr, "Error reading file %s: %s\n", PROC_MEMINFO,
                strerror(errno));
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
        fprintf(stderr, "Cannot compute memory usage: missing statistics\n");
        exit(EXIT_FAILURE);
    }
    used = mem->total - mem->free - mem->buffers - mem->cached - mem->sreclaim;
    used = PERCENT(used, mem->total);
    module->critical = used >= mem->cthreshold ? 1 : 0;
    set_token_buffer(mem->tokens[0].buffer, mem->label);
    snprintf(mem->tokens[1].buffer, TBUFFER_MAX_SIZE, "%ld%%", used);
    set_module_buffer(module, module->opts[0].value, mem->tokens,
                      MBUFFER_MAX_SIZE);
    return NULL;
}

void            init_memory(void *data) {
    t_module    *module = data;
    t_mem       *mem = module->data;

    mem->tokens[0].fmtid = 'L';
    mem->tokens[1].fmtid = 'V';
    init_module_tokens(module, module->opts[0].value, mem->tokens, MEM_TOKENS);

    mem->label = module->opts[1].value;
    mem->cthreshold = ((int *)module->opts[2].value)[0];
}
