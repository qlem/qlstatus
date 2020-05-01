/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

char        *append_module(t_main *main, char *buffer, char fmtid) {
    char    *nbuffer = NULL;
    char    *value = NULL;
    int     i = -1;
    size_t  len = 0;
    size_t  nlen = 0;

    while (++i < NB_MODULES) {
        if (main->modules[i].enabled && fmtid == main->modules[i].fmtid) {
            value = to_str(main->modules[i].value);
            len = v_strlen(buffer);
            nlen = v_strlen(main->modules[i].label) + v_strlen(value) + 
                    v_strlen(main->modules[i].unit) + 1;
            nbuffer = alloc_buffer(sizeof(char) * (len + nlen + 1));
            if (buffer) {
                sprintf(nbuffer, "%s%s %s%s", buffer, main->modules[i].label, value, 
                        main->modules[i].unit);
                free(buffer);
            } else { 
                sprintf(nbuffer, "%s %s%s", main->modules[i].label, value, main->modules[i].unit);
            }
            free(value);
            return nbuffer;
        }
    }
    return buffer;
}

char        *format(t_main *main) {
    char    *fmt = main->format;
    char    *buffer = NULL;
    size_t  size = 0;
    int     i = -1;
    
    if (!fmt || !fmt[0]) {
        printf("Format cannot be null\n");
        exit(EXIT_FAILURE);
    }
    while (fmt[++i]) {
        if (fmt[i] == '%') {
            switch (fmt[i + 1]) {
                case 'U':
                    buffer = append_module(main, buffer, 'U');
                    break;
                case 'T':
                    buffer = append_module(main, buffer, 'T');
                    break;
                case 'M':
                    buffer = append_module(main, buffer, 'M');
                    break;
                case 'L':
                    buffer = append_module(main, buffer, 'L');
                    break;
                case 'V':
                    buffer = append_module(main, buffer, 'V');
                    break;
                case 'B':
                    buffer = append_module(main, buffer, 'B');
                    break;
                case 'W':
                    buffer = append_module(main, buffer, 'W');
                    break;
                default:
                    printf("Bad format\n");
                    exit(EXIT_FAILURE);
            }
            i++;
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
