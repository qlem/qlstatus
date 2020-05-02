/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

char        *append_module(t_module *module, char *buffer) {
    char    *new = NULL;
    char    *value = NULL;
    size_t  len = 0;
    size_t  mlen = 0;

    value = to_str(module->value);
    len = v_strlen(buffer);
    mlen = v_strlen(module->label) + v_strlen(value) +
                    v_strlen(module->unit) + 1;
    new = alloc_buffer(sizeof(char) * (len + mlen + 1));
    if (buffer) {
        sprintf(new, "%s%s %s%s", buffer, module->label, value, module->unit);
        free(buffer);
    } else {
        sprintf(new, "%s %s%s", module->label, value, module->unit);
    }
    // free label only for wireless module
    if (module->fmtid == 'W') {
        free(module->label);
    }
    free(value);
    return new;
}

char        *format(t_main *main) {
    char    *fmt = main->format;
    char    *buffer = NULL;
    size_t  size = 0;
    int     i = -1;
    int     j;

    if (!fmt || !fmt[0]) {
        printf("Format cannot be null\n");
        exit(EXIT_FAILURE);
    }
    while (fmt[++i]) {
        if (fmt[i] == '%') {
            ++i;
            j = -1;
            while (++j < NB_MODULES) {
                if (main->modules[j].enabled &&
                    main->modules[j].fmtid == fmt[i]) {
                    buffer = append_module(&main->modules[j], buffer);
                    break;
                }
            }
            if (j == NB_MODULES) {
                printf("Bad format\n");
                exit(EXIT_FAILURE);
            }
        } else {
            size = v_strlen(buffer) + 2;
            if ((buffer = realloc(buffer, sizeof(char) * size)) == NULL) {
                printf("Call to 'realloc()' failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            buffer[size - 2] = fmt[i];
            buffer[size - 1] = 0;
        }
    }
    return buffer;
}
