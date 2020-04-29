/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

int             compile_regex(const char *regex, regex_t *preg) {
    int         errcode = 0;
    char        *buffer = NULL;
    size_t      size = 0;

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

char    *substring(const char *regex, const char *str) {
    regex_t         preg;
    regmatch_t      pmatch[2];
    char            *buffer = NULL;
    size_t          size = 0;
    int             i = -1;

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

bool        match_pattern(const char *regex, const char *str) {
    regex_t         preg;

    compile_regex(regex, &preg);
    if (regexec(&preg, str, 0, NULL, 0) == 0) {
        regfree(&preg);
        return true;
    }
    regfree(&preg);
    return false;
}
