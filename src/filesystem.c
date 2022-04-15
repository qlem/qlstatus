/*
 * Copyright (c) 2022 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

static const char   units[5] = {'B', 'K', 'M', 'G', 'T'};

void        free_filesystem(void *data) {
    (void)data;
}

void        set_unit_suffix(char *buf, int power) {
    int     i = -1;

    while (++i < TBUFFER_MAX_SIZE && buf[i]) {}
    if (i < TBUFFER_MAX_SIZE) {
        buf[i] = units[power];
    }
    if (power > 0 && i < TBUFFER_MAX_SIZE - 2) {
        buf[i + 1] = 'i';
        buf[i + 2] = 'B';
    }
}

void        fs_format_logic(t_fsys *fsys, unsigned long rvalue, int tindex) {
    float   value = (float)rvalue;
    int     power = 0;

    while (value >= KILOBYTE) {
        value = value / KILOBYTE;
        power++;
    }

    if (value < 10) {
        if (tindex == 3) {
            snprintf(fsys->tokens[tindex].buffer, TBUFFER_MAX_SIZE, "%.1f", value);
            remove_leading_zero(fsys->tokens[tindex].buffer);
            set_unit_suffix(fsys->tokens[tindex].buffer, power);
        } else {
            snprintf(fsys->tokens[tindex].buffer, TBUFFER_MAX_SIZE, "%4.1f", value);
            clean_leading_zero(fsys->tokens[tindex].buffer);
            fsys->tokens[tindex].buffer[4] = units[power];
        }
    } else {
        if (tindex == 3) {
            snprintf(fsys->tokens[tindex].buffer, TBUFFER_MAX_SIZE, "%d", (int)value);
            set_unit_suffix(fsys->tokens[tindex].buffer, power);
        } else {
            snprintf(fsys->tokens[tindex].buffer, TBUFFER_MAX_SIZE, "%4d%c", (int)value,
                     units[power]);
        }
    }
}

void                *run_filesystem(void *data) {
    t_module        *module = data;
    t_fsys          *fsys = module->data;
    struct statvfs  stats;
    unsigned long   used;
    unsigned long   free;
    unsigned long   rfree;
    unsigned long   total;
    int             value;

    errno = 0;
    if (statvfs(fsys->path, &stats) == -1) {
        fprintf(stderr, "Call to statvfs() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    total = stats.f_blocks * stats.f_frsize;
    free = stats.f_bavail * stats.f_bsize;
    rfree = stats.f_bfree * stats.f_bsize;
    used = total - rfree;

    // set detailed filesystem usage
    fs_format_logic(fsys, used, 2);
    fs_format_logic(fsys, total, 3);
    fs_format_logic(fsys, fsys->real_free ? rfree : free, 4);

    // set percent memory usage
    value = PERCENT(used, total);
    module->critical = value >= fsys->cthreshold ? 1 : 0;
    snprintf(fsys->tokens[1].buffer, TBUFFER_MAX_SIZE, "%2d%%", value);
    set_module_buffer(module, fsys->tokens, MEM_TOKENS);
    return NULL;
}

void            init_filesystem(void *data) {
    t_module    *module = data;
    t_fsys      *fsys = module->data;

    fsys->tokens[0].fmtid = 'L';
    fsys->tokens[1].fmtid = 'P';
    fsys->tokens[2].fmtid = 'C';
    fsys->tokens[3].fmtid = 'T';
    fsys->tokens[4].fmtid = 'F';
    init_module_tokens(module, fsys->tokens, FSYS_TOKENS);

    fsys->path = module->opts[2].value;
    fsys->cthreshold = ((int *)module->opts[3].value)[0];
    fsys->real_free = ((int *)module->opts[4].value)[0];
    set_token_buffer(fsys->tokens[0].buffer, module->opts[1].value);
}
