/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

char        *append_single_char(char *buffer, char c) {
    size_t  size;

    size = v_strlen(buffer) + 2;
    if ((buffer = realloc(buffer, sizeof(char) * size)) == NULL) {
        printf("Call to realloc() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    buffer[size - 2] = c;
    buffer[size - 1] = 0;
    return buffer;
}

/*
    if (main->spwm_colors && module->critical) {
        size = v_strlen(module->label) + v_strlen(value) +
                                                v_strlen(module->unit) + 17;
        buffer = alloc_buffer(size);
        sprintf(buffer, " %s%d;%s %s%s%s", SPWM_COLOR_START,
                main->critical_color_idx, module->label, value, module->unit,
                SPWM_COLOR_STOP);
    }
*/

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

    (void)main;
    mlen = v_strlen(module->buffer);
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
                printf("Format error: bad format\n");
                exit(EXIT_FAILURE);
            }
        } else {
            buffer = append_single_char(buffer, fmt[i]);
        }
    }
    buffer = append_single_char(buffer, '\n');
    return buffer;
}
