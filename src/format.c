/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

char        *append_single_char(char *buffer, char c) {
    size_t  size;

    if (buffer == NULL) {
        buffer = alloc_buffer(sizeof(char) * 2);
        buffer[0] = c;
        return buffer;
    }
    size = v_strlen(buffer) + 2;
    if ((buffer = realloc(buffer, sizeof(char) * size)) == NULL) {
        fprintf(stderr, "Call to realloc() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    buffer[size - 2] = c;
    buffer[size - 1] = 0;
    return buffer;
}

void        set_generic_module_buffer(t_module *module, long value,
                                      char *label, char *unit) {
    char    *bval = NULL;

    bval = to_str(value);
    if (v_strlen(bval) > 6) {
        sprintf(module->buffer, "%s %s%s", label, "--", unit);
    } else {
        sprintf(module->buffer, "%s %s%s", label, bval, unit);
    }
    free(bval);
}

char        *append_module(t_main *main, t_module *module, char *buffer) {
    char    *new = NULL;
    size_t  blen;
    size_t  mlen;

    mlen = v_strlen(module->buffer);
    if (main->spwmcolors && module->critical && buffer == NULL) {
        new = alloc_buffer(mlen + 15);
        sprintf(new, "%s%d;%s%s", SPWM_COLOR_START, main->spwmcoloridx,
                module->buffer, SPWM_COLOR_STOP);
        return new;
    }
    if (main->spwmcolors && module->critical) {
        blen = v_strlen(buffer);
        new = alloc_buffer(sizeof(char) * (blen + mlen + 15));
        sprintf(new, "%s%s%d;%s%s", buffer, SPWM_COLOR_START,
                main->spwmcoloridx, module->buffer, SPWM_COLOR_STOP);
        free(buffer);
        return new;
    }
    if (buffer == NULL) {
        new = alloc_buffer(mlen + 1);
        v_strncpy(new, module->buffer, mlen);
        return new;
    }
    blen = v_strlen(buffer);
    new = alloc_buffer(sizeof(char) * (blen + mlen + 1));
    v_strsncpy(new, buffer, 0, blen);
    v_strsncpy(new, module->buffer, blen, mlen);
    free(buffer);
    return new;
}

char        *format(t_main *main) {
    char    *fmt = main->format;
    char    *buffer = NULL;
    int     i = -1;
    int     j;

    while (fmt[++i]) {
        if (fmt[i] == '%') {
            ++i;
            j = -1;
            while (++j < NB_MODULES) {
                if (main->modules[j].enabled &&
                    main->modules[j].fmtid == fmt[i]) {
                    buffer = append_module(main, &main->modules[j], buffer);
                    break;
                }
            }
            if (j == NB_MODULES) {
                fprintf(stderr, "Invalid escape sequence: %%%c\n", fmt[i]);
                exit(EXIT_FAILURE);
            }
        } else {
            buffer = append_single_char(buffer, fmt[i]);
        }
    }
    buffer = append_single_char(buffer, '\n');
    return buffer;
}
