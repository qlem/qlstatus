/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void    v_sleep(time_t sec, long nsec) {
    struct timespec     tp;

    tp.tv_sec = sec;
    tp.tv_nsec = nsec;
    if (clock_nanosleep(CLOCK_REALTIME, 0, &tp, NULL)) {
        printf("Call to 'clock_nanosleep()' failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

size_t  v_strlen(const char *str) {
    size_t     i = -1;
    
    if (str == NULL) {
        return 0;
    }
    while (str[++i]) {}
    return i;
}

void    v_memset(void *ptr, uint8_t c, size_t size) {
    size_t  i = -1;

    while (++i < size) {
        ((uint8_t *)ptr)[i] = c;
    }
}

char    *v_strncpy(char *dest, const char *src, size_t n) {
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

int     putstr(const char *str) {
    size_t  size = v_strlen(str);

    write(1, str, size);
    return size;
}

char    *alloc_buffer(size_t size) {
    char    *buffer;

    if ((buffer = malloc(sizeof(char) * size)) == NULL) {
        printf("Call to 'malloc()' failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    v_memset(buffer, 0, size);
    return buffer;
}

void    *alloc_ptr(size_t size) {
    void    *ptr;

    if ((ptr = malloc(size)) == NULL) {
        printf("Call to 'malloc()' failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return ptr;
}

long    to_int(const char *str) {
    long    nb = 0;

    nb = strtol(str, NULL, BASE);
    if ((nb == LONG_MIN || nb == LONG_MAX)) {
        printf("Call to 'strtol()' failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return nb;
}
