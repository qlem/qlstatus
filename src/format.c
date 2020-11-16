/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

int         append_single_char_to_module_buffer(t_module *module, char c) {
    int     i = -1;

    while (++i < BUFFER_MAX_SIZE && module->buffer[i]) {}
    if (i == BUFFER_MAX_SIZE) {
        module->buffer[BUFFER_MAX_SIZE - 1] = '.';
        return 0;
    }
    module->buffer[i] = c;
    return 1;
}

int         append_token(t_module *module, const char *token) {
    int     i = -1;
    int     j = -1;

    while (++i < BUFFER_MAX_SIZE && module->buffer[i]) {}
    if (i == BUFFER_MAX_SIZE) {
        module->buffer[BUFFER_MAX_SIZE - 1] = '.';
        return 0;
    }
    while (token[++j] && i < BUFFER_MAX_SIZE) {
        module->buffer[i] = token[j];
        i++;
    }
    return j;
}

int         set_module_buffer(t_module *module, const char *format, t_token *tokens, int size) {
    int     i = -1;
    int     j;

    v_memset(module->buffer, 0, BUFFER_MAX_SIZE);
    while (format[++i]) {
        if (format[i] == '%') {
            ++i;
            switch (format[i]) {
                case 0:
                    fprintf(stderr, "Module [%c]: invalid escape sequence [%%]\n", module->fmtid);
                    exit(EXIT_FAILURE);
                case '%':
                    if (!append_single_char_to_module_buffer(module, '%')) {
                        return 0;
                    }
                    break;
                default:
                    j = -1;
                    while (++j < size) {
                        if (format[i] == tokens[j].fmtid && tokens[j].enabled) {
                            if (!append_token(module, tokens[j].buffer)) {
                                return 0;
                            }
                            break;
                        }
                    }
                    if (j == size) {
                        fprintf(stderr, "Module [%c]: invalid escape sequence [%%%c]\n", module->fmtid, format[i]);
                        exit(EXIT_FAILURE);
                    }
            }
        } else {
            if (!append_single_char_to_module_buffer(module, format[i])) {
                return 0;
            }
        }
    }
    return 0;
}

int         init_module_tokens(t_module *module, const char *format, t_token *tokens, int size) {
    int     i = -1;
    int     j;

    while (format[++i]) {
        if (format[i] == '%') {
            ++i;
            switch (format[i]) {
                case 0:
                    fprintf(stderr, "Module [%c]: invalid escape sequence [%%]\n", module->fmtid);
                    exit(EXIT_FAILURE);
                case '%':
                    break;
                default:
                    j = -1;
                    while (++j < size) {
                        if (format[i] == tokens[j].fmtid) {
                            tokens[j].enabled = 1;
                            break;
                        }
                    }
                    if (j == size) {
                        fprintf(stderr, "Module [%c]: invalid escape sequence [%%%c]\n", module->fmtid, format[i]);
                        exit(EXIT_FAILURE);
                    }
            }
        }
    }
    return 0;
}

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

char        *get_output_buffer(t_main *main) {
    char    *format = main->format;
    char    *buffer = NULL;
    int     i = -1;
    int     j;

    while (format[++i]) {
        if (format[i] == '%') {
            ++i;
            switch (format[i]) {
                case 0:
                    fprintf(stderr, "Output format: invalid escape sequence [%%]\n");
                    exit(EXIT_FAILURE);
                case '%':
                    buffer = append_single_char(buffer, '%');
                    break;
                default:
                    j = -1;
                    while (++j < NB_MODULES) {
                        if (main->modules[j].fmtid == format[i] && main->modules[j].enabled) {
                            buffer = append_module(main, &main->modules[j], buffer);
                            break;
                        }
                    }
                    if (j == NB_MODULES) {
                        fprintf(stderr, "Output format: invalid escape sequence [%%%c]\n", format[i]);
                        exit(EXIT_FAILURE);
                    }
            }
        } else {
            buffer = append_single_char(buffer, format[i]);
        }
        buffer = append_single_char(buffer, '\n');
    }
    return 0;
}

int         enable_modules(t_main *main) {
    char    *format = main->format;
    int     i = -1;
    int     j;

    while (format[++i]) {
        if (format[i] == '%') {
            ++i;
            switch (format[i]) {
                case 0:
                    fprintf(stderr, "Output format: invalid escape sequence [%%]\n");
                    exit(EXIT_FAILURE);
                case '%':
                    break;
                default:
                    j = -1;
                    while (++j < NB_MODULES) {
                        if (main->modules[j].fmtid == format[i]) {
                            main->modules[j].enabled = 1;
                            break;
                        }
                    }
                    if (j == NB_MODULES) {
                        fprintf(stderr, "Output format: invalid escape sequence [%%%c]\n", format[i]);
                        exit(EXIT_FAILURE);
                    }
            }
        }
    }
    return 0;
}
