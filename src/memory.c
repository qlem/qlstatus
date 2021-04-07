/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void        free_memory(void *data) {
    (void)data;
}

void        kb_format_logic(t_mem *mem, long value) {
    snprintf(mem->tokens[2].buffer, TBUFFER_MAX_SIZE, "%8ld", value);
    snprintf(mem->tokens[3].buffer, TBUFFER_MAX_SIZE, "%8ld", mem->total);
    set_token_buffer(mem->tokens[4].buffer, "kB");
}

void        mb_format_logic(t_mem *mem, long value) {
    snprintf(mem->tokens[2].buffer, TBUFFER_MAX_SIZE, "%5ld", value / MEGABYTE);
    snprintf(mem->tokens[3].buffer, TBUFFER_MAX_SIZE, "%5ld",
             mem->total / MEGABYTE);
    set_token_buffer(mem->tokens[4].buffer, "mB");
}

void        gb_format_logic(t_mem *mem, long value) {
    snprintf(mem->tokens[2].buffer, TBUFFER_MAX_SIZE, "%4.1f",
             (float)value / (MEGABYTE * MEGABYTE));
    snprintf(mem->tokens[3].buffer, TBUFFER_MAX_SIZE, "%4.1f",
             (float)mem->total / (MEGABYTE * MEGABYTE));
    clean_leading_zero(mem->tokens[2].buffer);
    remove_leading_zero(mem->tokens[3].buffer);
    set_token_buffer(mem->tokens[4].buffer, "gB");
}

void        value_smart_format_logic(t_mem *mem, long value) {
    if (value > (MEGABYTE * MEGABYTE) - 1) {
        snprintf(mem->tokens[2].buffer, TBUFFER_MAX_SIZE, "%4.1f",
                 (float)value / (MEGABYTE * MEGABYTE));
        clean_leading_zero(mem->tokens[2].buffer);
        mem->tokens[2].buffer[4] = 'g';
    } else if (value > MEGABYTE - 1) {
        snprintf(mem->tokens[2].buffer, TBUFFER_MAX_SIZE, "%4ldm",
                 value / MEGABYTE);
    } else {
        snprintf(mem->tokens[2].buffer, TBUFFER_MAX_SIZE, "%4ldk", value);
    }
}

void        total_smart_format_logic(t_mem *mem) {
    if (mem->total > (MEGABYTE * MEGABYTE) - 1) {
        snprintf(mem->tokens[3].buffer, TBUFFER_MAX_SIZE, "%.1f",
                 (float)mem->total / (MEGABYTE * MEGABYTE));
        remove_leading_zero(mem->tokens[3].buffer);
        set_token_buffer(mem->tokens[4].buffer, "gB");
    } else if (mem->total > MEGABYTE - 1) {
        snprintf(mem->tokens[3].buffer, TBUFFER_MAX_SIZE, "%ld",
                 mem->total / MEGABYTE);
        set_token_buffer(mem->tokens[4].buffer, "mB");
    } else {
        snprintf(mem->tokens[3].buffer, TBUFFER_MAX_SIZE, "%ld", mem->total);
        set_token_buffer(mem->tokens[4].buffer, "kB");
    }
}

void        parse_mem_value(t_mem *mem, char *rvalue) {
    char    *value;

    if ((value = substring(MEM_TOTAL_PATTERN, rvalue))) {
        mem->total = to_int(value);
    } else if ((value = substring(MEM_FREE_PATTERN, rvalue))) {
        mem->free = to_int(value);
    } else if ((value = substring(MEM_BUFFERS_PATTERN, rvalue))) {
        mem->buffers = to_int(value);
    } else if ((value = substring(MEM_CACHED_PATTERN, rvalue))) {
        mem->cached = to_int(value);
    } else if ((value = substring(MEM_SRECLAIM_PATTERN, rvalue))) {
        mem->sreclaim = to_int(value);
    }
    free(rvalue);
    free(value);
}

int             parse_mem_file(t_mem *mem) {
    char        *line = NULL;
    size_t      size = 0;
    FILE        *stream;
    ssize_t     nb;

    errno = 0;
    stream = open_stream(PROC_MEMINFO);
    while ((nb = getline(&line, &size, stream)) != -1) {
        line[nb - 1] == '\n' ? line[nb - 1] = 0 : 0;
        parse_mem_value(mem, line);
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
        exit(EXIT_FAILURE);
    }
    free(line);
    close_stream(stream, PROC_MEMINFO);
    return -1;
}

void            *run_memory(void *data) {
    t_module    *module = data;
    t_mem       *mem = module->data;
    long        value;

    mem->total = -1;
    mem->free = -1;
    mem->buffers = -1;
    mem->cached = -1;
    mem->sreclaim = -1;
    if (parse_mem_file(mem) == -1) {
        fprintf(stderr, "Cannot compute memory usage: missing values\n");
        exit(EXIT_FAILURE);
    }
    value = mem->total - mem->free - mem->buffers - mem->cached - mem->sreclaim;

    // set detailed memory usage
    if (mem->tokens[2].enabled || mem->tokens[3].enabled) {
        switch(mem->unit) {
            case KB:
                kb_format_logic(mem, value);
                break;
            case MB:
                mb_format_logic(mem, value);
                break;
            case GB:
                gb_format_logic(mem, value);
                break;
            case MSMT:
                value_smart_format_logic(mem, value);
                total_smart_format_logic(mem);
        }
    }

    // set percent memory usage
    value = PERCENT(value, mem->total);
    module->critical = value >= mem->cthreshold ? 1 : 0;
    snprintf(mem->tokens[1].buffer, TBUFFER_MAX_SIZE, "%2ld%%", value);
    set_module_buffer(module, mem->tokens, MEM_TOKENS);
    return NULL;
}

void            init_memory(void *data) {
    t_module    *module = data;
    t_mem       *mem = module->data;

    mem->tokens[0].fmtid = 'L';
    mem->tokens[1].fmtid = 'P';
    mem->tokens[2].fmtid = 'C';
    mem->tokens[3].fmtid = 'T';
    mem->tokens[4].fmtid = 'U';
    init_module_tokens(module, mem->tokens, MEM_TOKENS);

    if (strcmp("kB", module->opts[2].value) == 0) {
        mem->unit = KB;
    } else if (strcmp("mB", module->opts[2].value) == 0) {
        mem->unit = MB;
    } else if (strcmp("gB", module->opts[2].value) == 0) {
        mem->unit = GB;
    } else {
        mem->unit = MSMT;
    }

    set_token_buffer(mem->tokens[0].buffer, module->opts[1].value);
    set_token_buffer(mem->tokens[4].buffer, module->opts[2].value);
    mem->cthreshold = ((int *)module->opts[3].value)[0];
}
