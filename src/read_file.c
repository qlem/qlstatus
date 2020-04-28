/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

off_t   file_len(const char *path) {
    struct stat     info;

    if (stat(path, &info) == -1) {
        printf("Cannot get file info '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return info.st_size;
}

char        *read_file(const char *path) {
    int     fd;
    char    *buffer;
    off_t   size;

    if ((fd = open(path, O_RDONLY)) == -1) {
        printf("Cannot open file '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    size = file_len(path);
    buffer = alloc_buffer(size + 1);
    if (read(fd, buffer, size) == -1) {
        printf("Cannot read file '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (close(fd) == -1) {
        printf("Cannot close file '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return buffer;
}
