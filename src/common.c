/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

char        *get_option_value(t_opt *opts, const char *key, int size) {
    int     i = -1;

    while (++i < size) {
        if (strcmp(opts[i].key, key) == 0) {
            return opts[i].value;
        }
    }
    return NULL;
}

size_t      v_strlen(const char *str) {
    size_t  i = -1;

    while (str[++i]) {}
    return i;
}

void        v_memset(void *ptr, uint8_t c, size_t size) {
    size_t  i = -1;

    while (++i < size) {
        ((uint8_t *)ptr)[i] = c;
    }
}

char        *v_strncpy(char *dest, const char *src, size_t n) {
    size_t  i = -1;

    if (!dest || !src) {
        return NULL;
    }
    while (++i < n && src[i]) {
        dest[i] = src[i];
    }
    while (i < n) {
        dest[i++] = 0;
    }
    return dest;
}

int         putstr(const char *str) {
    size_t  size = v_strlen(str);

    if (write(1, str, size) == -1) {
        printf("Call to write() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return size;
}

char        *alloc_buffer(size_t size) {
    char    *buffer;

    if ((buffer = malloc(sizeof(char) * size)) == NULL) {
        printf("Call to malloc() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    v_memset(buffer, 0, size);
    return buffer;
}

void        *alloc_ptr(size_t size) {
    void    *ptr;

    if ((ptr = malloc(size)) == NULL) {
        printf("Call to malloc() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return ptr;
}

long        to_int(const char *str) {
    long    nb = 0;

    nb = strtol(str, NULL, BASE);
    if ((nb == LONG_MIN || nb == LONG_MAX)) {
        printf("Call to strtol() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return nb;
}
