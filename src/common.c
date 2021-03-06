/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

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

char        *v_strsncpy(char *dest, const char *src, int start, size_t n) {
    size_t  i = start - 1;

    while (++i < start + n) {
        dest[i] = src[i - start];
    }
    return dest;
}

char        *v_strncpy(char *dest, const char *src, size_t n) {
    size_t  i = -1;

    while (++i < n) {
        dest[i] = src[i];
    }
    return dest;
}

int         putstr(const char *str) {
    size_t  size = v_strlen(str);

    errno = 0;
    if (write(1, str, size) == -1) {
        fprintf(stderr, "Call to write() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return size;
}

char        *alloc_buffer(size_t size) {
    char    *buffer;

    errno = 0;
    if ((buffer = malloc(sizeof(char) * size)) == NULL) {
        fprintf(stderr, "Call to malloc() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    v_memset(buffer, 0, size);
    return buffer;
}

void        *alloc_ptr(size_t size) {
    void    *ptr;

    errno = 0;
    if ((ptr = malloc(size)) == NULL) {
        fprintf(stderr, "Call to malloc() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return ptr;
}

long        to_int(const char *str) {
    long    nb = 0;

    errno = 0;
    nb = strtol(str, NULL, BASE);
    if ((nb == LONG_MIN || nb == LONG_MAX)) {
        fprintf(stderr, "Call to strtol() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return nb;
}
