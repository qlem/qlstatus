/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

char        *append_single_char(char *buffer, char c) {
    size_t  size = 0;

    size = v_strlen(buffer) + 2;
    if ((buffer = realloc(buffer, sizeof(char) * size)) == NULL) {
        printf("Call to realloc() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    buffer[size - 2] = c;
    buffer[size - 1] = 0;
    return buffer;
}

char        *get_module_buffer(t_main *main, t_module *module) {
    char    *buffer = NULL;
    char    *value = NULL;
    size_t  size = 0;

    value = to_str(module->value);
    if (main->spwm_colors && module->critical) {
        size = v_strlen(module->label) + v_strlen(value) +
               v_strlen(module->unit) + 17;
        buffer = alloc_buffer(size);
        sprintf(buffer, " %s%d;%s %s%s%s", SPWM_COLOR_START,
                main->critical_color_idx, module->label, value,
                module->unit, SPWM_COLOR_STOP);
    } else {
        size = v_strlen(module->label) + v_strlen(value) +
               v_strlen(module->unit) + 2;
        buffer = alloc_buffer(size);
        sprintf(buffer, "%s %s%s", module->label, value, module->unit);
    }
    free(value);
    return buffer;
}

char        *append_module(t_main *main, t_module *module, char *buffer) {
    char    *new = NULL;
    char    *module_b = NULL;
    size_t  blen = 0;
    size_t  mlen = 0;

    module_b = get_module_buffer(main, module);
    if (buffer == NULL) {
        return module_b;
    }
    blen = v_strlen(buffer);
    mlen = v_strlen(module_b);
    new = alloc_buffer(sizeof(char) * (blen + mlen + 1));
    v_strsncpy(new, buffer, 0, blen);
    v_strsncpy(new, module_b, blen, mlen);
    free(module_b);
    free(buffer);
    return new;
}

char        *format(t_main *main) {
    char    *fmt = main->format;
    char    *buffer = NULL;
    int     i = -1;
    int     j;

    if (!fmt || !fmt[0]) {
        printf("Format error: format cannot be null\n");
        exit(EXIT_FAILURE);
    }
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
