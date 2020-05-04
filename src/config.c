/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void    free_opt(char **opt) {
    if (opt[0]) {
        free(opt[0]);
    }
    if (opt[1]) {
        free(opt[1]);
    }
    free(opt);
}

int      count_whitespace_from_end(const char *str, int len) {
    int     i = len;

    while (--i >= 0 && (str[i] == ' ' || str[i] == '\t')) {}
    return len - (i + 1);
}

int      count_whitespace_from_start(const char *str) {
    int     i = -1;

    while (str[++i] && (str[i] == ' ' || str[i] == '\t')) {}
    return i;
}

char        *trim(char *str) {
    char    *trim = NULL;
    int     size;
    int     start;
    int     end;
    int     i = -1;

    size = v_strlen(str);
    if (size == 0) {
        free(str);
        return  NULL;
    }
    start = count_whitespace_from_start(str);
    if (start == size) {
        free(str);
        return NULL;
    }
    end = count_whitespace_from_end(str, size);
    trim = alloc_buffer(size - (start + end) + 1);
    while (++i < size - (start + end)) {
        trim[i] = str[i + start];
    }
    free(str);
    return trim;
}

char        **parse_opt(char *line) {
    char    **opt = NULL;
    int     i = -1;
    int     j = 0;

    opt = alloc_ptr(sizeof(char *) * 2);
    opt[0] = NULL;
    opt[1] = NULL;
    while (line[++i] && line[i] != '=') {}
    opt[0] = alloc_buffer(i + 1);
    v_strncpy(opt[0], line, i);
    opt[0] = trim(opt[0]);
    while (line[++i]) {
        j++;
    }
    if (j > 0) {
        opt[1] = alloc_buffer(j + 1);
        v_strncpy(opt[1], line + (i - j), j);
        opt[1] = trim(opt[1]);
    }
    return opt;
}

int             check_global_opts(t_main *main, char **opt) {
    int         i = -1;

    while (++i < GLOBAL_OPTS) {
        if (strcmp(opt[0], main->opts[i].key) == 0) {
            if (!match_pattern(main->opts[i].p_value, opt[1])) {
                printf("Option value do not match pattern: %s\n", main->opts[i].key);
                return -1;
            }
            main->opts[i].value = opt[1];
            // set global format
            if (i == 0) {
                main->format = opt[1];
            // set global rate
            } else if (i == 1) {
                main->rate = opt[1];
            }
            return 0;
        }
    }
    return 1;
}

int             check_module_opts(t_module *modules, char **opt) {
    t_opt       *opts = NULL;
    int         i = -1;
    int         j;

    while (++i < NB_MODULES) {
        j = -1;
        opts = modules[i].opts;
        while (++j < modules[i].s_opts) {
            if (strcmp(opt[0], opts[j].key) == 0) {
                if (!match_pattern(opts[j].p_value, opt[1])) {
                    printf("Option value do not match pattern: %s\n", opts[j].key);
                    return -1;
                }
                opts[j].value = opt[1];
                // turn on / off module
                if (j == 0 && to_int(opt[1]) == 0) {
                    modules[i].enabled = 0;
                // set module label
                } else if (j == 1) {
                    modules[i].label= opt[1];
                }
                return 0;
            }
        }
    }
    return 1;
}

int         parse_config_line(t_main *main, char *line) {
    char    **opt;
    int     gcode;
    int     mcode;

    opt = parse_opt(line);
    if (!opt[0] && !opt[1]) {
        free_opt(opt);
        return 0;
    }
    if (!opt[0] && opt[1]) {
        printf("Key expected for value: %s\n", opt[1]);
        free_opt(opt);
        return -1;
    }
    if (opt[0] && !opt[1]) {
        printf("Value expected for key: %s\n", opt[0]);
        free_opt(opt);
        return -1;
    }
    gcode = check_global_opts(main, opt);
    if (gcode == -1) {
        free_opt(opt);
        return -1;
    }
    mcode = check_module_opts(main->modules, opt);
    if (mcode == -1) {
        free_opt(opt);
        return -1;
    }
    if (mcode == 1 && gcode == 1) {
        printf("Unknown option: %s\n", opt[0]);
        free_opt(opt);
        return -1;
    }

    // debug
    printf("Set option [%s] to [%s]\n", opt[0], opt[1]);

    free(opt[0]);
    free(opt);
    return 0;
}

int     parse_config_file(t_main *main, const char *file) {
    FILE    *stream;
    size_t  size = 0;
    char    *line = NULL;
    ssize_t nb;

    if ((stream = fopen(file, "r")) == NULL) {
        return -1;
    }
    while ((nb = getline(&line, &size, stream)) != -1) {
        if (line[v_strlen(line) - 1] == '\n') {
            line[v_strlen(line) - 1] = 0;
        }
        if (parse_config_line(main, line) < 0) {
            free(line);
            close_stream(stream, file);
            exit(EXIT_FAILURE);
        }
        free(line);
        line = NULL;
        size = 0;
    }
    if (nb == -1 && errno) {
        printf("Error reading file '%s': %s\n", file, strerror(errno));
        close_stream(stream, file);
        exit(EXIT_FAILURE);
    }
    free(line);
    close_stream(stream, file);
    return 0;
}
