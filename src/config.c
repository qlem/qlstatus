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

void        print_option(t_opt *opt) {
    if (opt->type == NUMBER) {
        printf("Set option [%s] to [%d]\n", opt->key, ((int *)opt->value)[0]);
    } else {
        printf("Set option [%s] to [%s]\n", opt->key, (char *)opt->value);
    }
}

int         check_global_opts(t_main *main, char **opt, int nline) {
    int     i = -1;

    while (++i < GLOBAL_OPTS) {
        if (strcmp(opt[0], main->opts[i].key) == 0) {
            // check if value match pattern
            if (!match_pattern(main->opts[i].pattern, opt[1])) {
                printf("Invalid value at line %d: %s\n", nline, opt[0]);
                return -1;
            }
            // set option value
            if (main->opts[i].type == NUMBER) {
                main->opts[i].value = alloc_ptr(sizeof(long));
                ((long *)main->opts[i].value)[0] = to_int(opt[1]);
                free(opt[1]);
            } else {
                main->opts[i].value = opt[1];
            }
            main->opts[i].to_free = 1;
            // set global format
            if (strcmp(main->opts[i].key, OPT_FORMAT) == 0) {
                main->format = opt[1];
            // set global rate
            } else if (strcmp(main->opts[i].key, OPT_RATE) == 0) {
                main->rate = opt[1];
            // enable/disable spectrwm colors support
            } else if (strcmp(main->opts[i].key, OPT_SPWM_COLORS) == 0) {
                main->spwm_colors = ((long *)main->opts[i].value)[0];
            // set critical color index
            } else if (strcmp(main->opts[i].key, OPT_C_COLOR_IDX) == 0) {
                main->critical_color_idx = ((long *)main->opts[i].value)[0];
            }
            // logging
            print_option(&main->opts[i]);
            return 0;
        }
    }
    return 1;
}

int             check_module_opts(t_module *modules, char **opt, int nline) {
    t_opt       *opts = NULL;
    int         i = -1;
    int         j;

    while (++i < NB_MODULES) {
        j = -1;
        opts = modules[i].opts;
        while (++j < modules[i].nopts) {
            if (strcmp(opt[0], opts[j].key) == 0) {
                // check if value match pattern
                if (!match_pattern(opts[j].pattern, opt[1])) {
                    printf("Invalid value at line %d: %s\n", nline, opt[0]);
                    return -1;
                }
                // set option value
                if (opts[j].type == NUMBER) {
                    opts[j].value = alloc_ptr(sizeof(long));
                    ((long *)opts[j].value)[0] = to_int(opt[1]);
                    free(opt[1]);
                } else {
                    opts[j].value = opt[1];
                }
                opts[j].to_free = 1;
                // enable/disable module
                if (opts[j].category == STATE) {
                    modules[i].enabled = ((long *)opts[j].value)[0];
                // set module label
                } else if (opts[j].category == LABEL) {
                    modules[i].label = opt[1];
                // set module critical threshold
                } else if (opts[j].category == CRITICAL) {
                    modules[i].threshold = ((long *)opts[j].value)[0];
                }
                // logging
                print_option(&opts[j]);
                return 0;
            }
        }
    }
    return 1;
}

int         count_whitespace_from_end(const char *str, int len) {
    int     i = len;

    while (--i >= 0 && (str[i] == ' ' || str[i] == '\t')) {}
    return len - (i + 1);
}

int         count_whitespace_from_start(const char *str) {
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

char        *remove_in_line_comment(char *line) {
    char    *sub = NULL;
    int     i = -1;

    while (line[++i]) {
        if (line[i] == '#' && (line[i - 1] == ' ' || line[i - 1] == '\t')) {
            sub = alloc_buffer(i + 1);
            v_strncpy(sub, line, i);
            free(line);
            return sub;
        }
    }
    return line;
}

char        **parse_opt(char *line) {
    char    **opt = NULL;
    int     i = -1;
    int     j = 0;
    int     size;

    opt = alloc_ptr(sizeof(char *) * 2);
    opt[0] = NULL;
    opt[1] = NULL;
    size = v_strlen(line);
    if (size == 1) {
        opt[0] = alloc_buffer(2);
        opt[0][0] = line[0];
        return opt;
    }
    while (line[++i] && line[i] != '=') {}
    opt[0] = alloc_buffer(i + 1);
    v_strncpy(opt[0], line, i);
    opt[0] = trim(opt[0]);
    if (line[0] == 0) {
        return opt;
    }
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

int         parse_config_line(t_main *main, char *line, int nline) {
    char    **opt;
    int     gcode;
    int     mcode;

    // skip blank lines and comments
    if (line[0] == 0) {
        free(line);
        return 0;
    }
    line = trim(line);
    if (!line) {
        return 0;
    }
    if (line[0] == '#') {
        free(line);
        return 0;
    }

    // remove in line comments
    line = remove_in_line_comment(line);
    line = trim(line);

    // retrieve key and value
    opt = parse_opt(line);

    // check if key and value exist
    if (!opt[0] || !opt[1]) {
        printf("Invalid option at line %d: %s\n", nline,
               opt[0] ? opt[0] : opt[1]);
        free(line);
        free_opt(opt);
        return -1;
    }

    // check if the option is a global option
    gcode = check_global_opts(main, opt, nline);
    if (gcode == -1) {
        free(line);
        free_opt(opt);
        return -1;
    }

    // check if the option is a module option
    mcode = check_module_opts(main->modules, opt, nline);
    if (mcode == -1) {
        free(line);
        free_opt(opt);
        return -1;
    }

    // check if the option exist
    if (mcode == 1 && gcode == 1) {
        printf("Invalid option at line %d: %s\n", nline, opt[0]);
        free(line);
        free_opt(opt);
        return -1;
    }

    // free
    free(line);
    free(opt[0]);
    free(opt);
    return 0;
}

int         parse_config_file(t_main *main, const char *file) {
    FILE    *stream;
    size_t  size = 0;
    char    *line = NULL;
    size_t  sline = 0;
    ssize_t nb;
    int     i = 0;

    if ((stream = fopen(file, "r")) == NULL) {
        return -1;
    }
    while ((nb = getline(&line, &size, stream)) != -1) {
        ++i;
        sline = v_strlen(line);
        if (line[sline - 1] == '\n') {
            line[sline - 1] = 0;
        }
        if (parse_config_line(main, line, i) < 0) {
            close_stream(stream, file);
            exit(EXIT_FAILURE);
        }
        line = NULL;
        size = 0;
    }
    if (nb == -1 && errno) {
        printf("Error reading file %s: %s\n", file, strerror(errno));
        close_stream(stream, file);
        exit(EXIT_FAILURE);
    }
    free(line);
    close_stream(stream, file);
    return 0;
}
