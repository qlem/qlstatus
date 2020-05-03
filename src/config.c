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
    int     i;

    i = len - 1;
    while (i >= 0 && (str[i] == ' ' || str[i] == '\t')) {
        i--;
    }
    return len - (i + 1);
}

int      count_whitespace_from_start(const char *str) {
    int     i = 0;

    while (str[i] && (str[i] == ' ' || str[i] == '\t')) {
        i++;
    }
    return i;
}

char        *trim(const char *str) {
    char    *trim = NULL;
    int     size;
    int     start;
    int     end;
    int     i = -1;

    size = v_strlen(str);
    if (size == 0) {
        return  NULL;
    }
    start = count_whitespace_from_start(str);
    if (start == size) {
        return NULL;
    }
    end = count_whitespace_from_end(str, size);
    trim = alloc_buffer(size - (start + end) + 1);
    while (++i < size - (start + end)) {
        trim[i] = str[i + start];
    }
    return trim;
}

char        **parse_opt(char *line) {
    char    **opt = NULL;
    char    *token = NULL;
    int     i = 0;

    opt = alloc_ptr(sizeof(char *) * 2);
    opt[0] = NULL;
    opt[1] = NULL;
    while ((token = strtok(line, "="))) {
        line = NULL;
        if (i > 1) {
            return NULL;
        }
        opt[i++] = trim(token);
    }
    return opt;
}

int         parse_config_line(t_main *main, char *line) {
    t_opt   *opts = NULL;
    char    **opt;
    int     i = -1;
    int     j;
    bool    match = false;

    if (!(opt = parse_opt(line)) || !opt[0] || !opt[1]) {
        free_opt(opt);
        printf("Config file error\n");
        return -1;
    }

    while (++i < NB_MODULES) {
        j = -1;
        opts = main->modules[i].opts;
        while (++j < main->modules[i].s_opts) {
            if (strcmp(opt[0], opts[j].key) == 0) {
                // TODO do stuff
                match = true;
                break;
            }
        }
        if (match) {
            break;
        }
    }

    if (!match) {
        printf("Unknown option: %s\n", opt[0]);
        free_opt(opt);
        return -1;
    }
    free_opt(opt);
    return 0;
}

int     parse_config_file(t_main *main, const char *file) {
    FILE    *stream;
    size_t  size = 0;
    char    *line = NULL;
    ssize_t nb;

    (void)main;
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
