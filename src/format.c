/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

// deprecated
char        *append_module(t_main *main, t_module *module, char *buffer) {
    char    *new = NULL;
    size_t  blen;
    size_t  mlen;

    mlen = v_strlen(module->buffer);
    if (main->spwmcolors && module->critical && buffer == NULL) {
        new = alloc_buffer(mlen + 15);
        sprintf(new, "%s%d;%s%s", SPWM_COLOR_START, main->spwmcoloridx, module->buffer,
                SPWM_COLOR_STOP);
        return new;
    }
    if (main->spwmcolors && module->critical) {
        blen = v_strlen(buffer);
        new = alloc_buffer(sizeof(char) * (blen + mlen + 15));
        sprintf(new, "%s%s%d;%s%s", buffer, SPWM_COLOR_START, main->spwmcoloridx, module->buffer,
                SPWM_COLOR_STOP);
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

void        remove_leading_zero(char *buf) {
    int     i = -1;

    while (++i < TBUFFER_MAX_SIZE && buf[i]) {}
    if (i > 1 && buf[i - 1] == '0') {
        buf[i - 1] = 0;
        buf[i - 2] = 0;
    }
}

void        clean_leading_zero(char *buf) {
    int     i = -1;

    while (++i < TBUFFER_MAX_SIZE && buf[i]) {}
    if (i > 1 && buf[i - 1] == '0') {
        while (--i > 1) {
            buf[i] = buf[i - 2];
        }
        i = -1;
        while (buf[++i] == ' ') {}
        buf[i] = ' ';
        buf[i + 1] = ' ';
    }
}

int         print_output_buffer(const char *buffer) {
    int     i = -1;

    errno = 0;
    while (++i < BUFFER_MAX_SIZE && buffer[i]) {}
    if (write(1, buffer, i) == -1) {
        fprintf(stderr, "Call to write() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return i;
}

int         set_token_buffer(char *buffer, const char *src) {
    int     i = -1;

    v_memset(buffer, 0, TBUFFER_MAX_SIZE);
    while (++i < TBUFFER_MAX_SIZE && src[i]) {
        buffer[i] = src[i];
    }
    if (i == TBUFFER_MAX_SIZE && src[i]) {
        buffer[TBUFFER_MAX_SIZE - 1] = TRUNCATE_CHAR;
    }
    return 0;
}

void        append_final_new_line_char(char *buffer) {
    int     i = -1;

    while (++i < BUFFER_MAX_SIZE && buffer[i]) {}
    if (i == BUFFER_MAX_SIZE) {
        if (buffer[BUFFER_MAX_SIZE - 1] == TRUNCATE_CHAR) {
            buffer[BUFFER_MAX_SIZE - 2] = TRUNCATE_CHAR;
        }
        buffer[BUFFER_MAX_SIZE - 1] = '\n';
        return;
    }
    buffer[i] = '\n';
}

int         append_single_char(char *buffer, char c, int size) {
    int     i = -1;

    while (++i < size && buffer[i]) {}
    if (i == size) {
        buffer[size - 1] = TRUNCATE_CHAR;
        return 0;
    }
    buffer[i] = c;
    return 1;
}

int         append_to_module_buffer(char *buffer, const char *append) {
    int     i = -1;
    int     j = -1;

    while (++i < MBUFFER_MAX_SIZE && buffer[i]) {}
    if (i == MBUFFER_MAX_SIZE) {
        buffer[MBUFFER_MAX_SIZE - 1] = TRUNCATE_CHAR;
        return 0;
    }
    while (i < MBUFFER_MAX_SIZE && ++j < TBUFFER_MAX_SIZE && append[j]) {
        buffer[i] = append[j];
        i++;
    }
    return j;
}

int         set_module_buffer(t_module *module, t_token *tokens, int size) {
    char    *format = module->opts[0].value;
    int     i = -1;
    int     j;

    v_memset(module->buffer, 0, MBUFFER_MAX_SIZE);
    while (format[++i]) {
        if (format[i] == '%') {
            ++i;
            switch (format[i]) {
                case '%':
                    if (!append_single_char(module->buffer, '%', MBUFFER_MAX_SIZE)) {
                        return 0;
                    }
                    break;
                default:
                    j = -1;
                    while (++j < size) {
                        if (format[i] == tokens[j].fmtid && tokens[j].enabled) {
                            if (!append_to_module_buffer(module->buffer, tokens[j].buffer)) {
                                return 0;
                            }
                            break;
                        }
                    }
            }
        } else {
            if (!append_single_char(module->buffer, format[i], MBUFFER_MAX_SIZE)) {
                return 0;
            }
        }
    }
    return 0;
}

int         init_module_tokens(t_module *module, t_token *tokens, int size) {
    char    *format = module->opts[0].value;
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
                        fprintf(stderr, "Module [%c]: invalid escape sequence [%%%c]\n",
                                module->fmtid, format[i]);
                        exit(EXIT_FAILURE);
                    }
            }
        }
    }
    return 0;
}

int         append_to_buffer(char *buffer, const char *append) {
    int     i = -1;
    int     j = -1;

    while (++i < BUFFER_MAX_SIZE && buffer[i]) {}
    if (i == BUFFER_MAX_SIZE) {
        buffer[BUFFER_MAX_SIZE - 1] = TRUNCATE_CHAR;
        return 0;
    }
    while (i < BUFFER_MAX_SIZE && ++j < MBUFFER_MAX_SIZE && append[j]) {
        buffer[i] = append[j];
        i++;
    }
    return j;
}

int         set_output_buffer(t_main *main) {
    char    *format = main->format;
    int     i = -1;
    int     j;

    v_memset(main->buffer, 0, BUFFER_MAX_SIZE);
    while (format[++i]) {
        if (format[i] == '%') {
            ++i;
            switch (format[i]) {
                case '%':
                    if (!append_single_char(main->buffer, '%', BUFFER_MAX_SIZE)) {
                        append_final_new_line_char(main->buffer);
                        return 0;
                    }
                    break;
                default:
                    j = -1;
                    while (++j < NB_MODULES) {
                        if (main->modules[j].fmtid == format[i] && main->modules[j].enabled) {
                            if (!append_to_buffer(main->buffer, main->modules[j].buffer)) {
                                append_final_new_line_char(main->buffer);
                                return 0;
                            }
                            break;
                        }
                    }
            }
        } else {
            if (!append_single_char(main->buffer, format[i], BUFFER_MAX_SIZE)) {
                append_final_new_line_char(main->buffer);
                return 0;
            }
        }
    }
    append_final_new_line_char(main->buffer);
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
                            printf("Module [%c] enabled\n", main->modules[j].fmtid);
                            break;
                        }
                    }
                    if (j == NB_MODULES) {
                        fprintf(stderr, "Output format: invalid escape sequence [%%%c]\n",
                                format[i]);
                        exit(EXIT_FAILURE);
                    }
            }
        }
    }
    return 0;
}
