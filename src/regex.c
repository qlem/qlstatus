/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

int             compile_regex(const char *regex, regex_t *preg) {
    char        *buffer = NULL;
    size_t      size = 0;
    int         errcode;

    if ((errcode = regcomp(preg, regex, REG_EXTENDED)) != 0) {
        size = regerror(errcode, preg, buffer, size);
        buffer = alloc_buffer(size + 1);
        regerror(errcode, preg, buffer, size);
        printf("Cannot compile regex: %s\n", buffer);
        free(buffer);
        exit(EXIT_FAILURE);
    }
    return 0;
}

char        **multiple_subs(const char *regex, const char *str, int nmatch) {
    regmatch_t      pmatch[nmatch + 1];
    char            **match = NULL;
    int             i = -1;
    regex_t         preg;
    size_t          size;
    int             j;

    compile_regex(regex, &preg);
    match = alloc_ptr(sizeof(char *) * nmatch);
    while (++i < nmatch) {
        match[i] = NULL;
    }
    if (regexec(&preg, str, nmatch + 1, pmatch, 0) == 0) {
        i = 0;
        while (++i < nmatch + 1) {
            if (pmatch[i].rm_so > -1) {
                size = pmatch[i].rm_eo - pmatch[i].rm_so;
                match[i - 1] = alloc_buffer(size + 1);
                j = -1;
                while (++j < (int)size) {
                    match[i - 1][j] = str[pmatch[i].rm_so + j];
                }
            }
        }
    }
    regfree(&preg);
    return match;
}

char                *substring(const char *regex, const char *str) {
    char            *buffer = NULL;
    regmatch_t      pmatch[2];
    int             i = -1;
    regex_t         preg;
    size_t          size;

    compile_regex(regex, &preg);
    if (regexec(&preg, str, 2, pmatch, 0) == 0) {
        if (pmatch[1].rm_so > -1) {
            size = pmatch[1].rm_eo - pmatch[1].rm_so;
            buffer = alloc_buffer(size + 1);
            while (++i < (int)size) {
                buffer[i] = str[pmatch[1].rm_so + i];
            }
        }
        regfree(&preg);
        return buffer;
    }
    regfree(&preg);
    return buffer;
}

bool            match_pattern(const char *regex, const char *str) {
    regex_t     preg;

    compile_regex(regex, &preg);
    if (regexec(&preg, str, 0, NULL, 0) == 0) {
        regfree(&preg);
        return true;
    }
    regfree(&preg);
    return false;
}
