/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void        close_stream(FILE *stream, const char *file) {
    if (fclose(stream) != 0) {
        fprintf(stderr, "Cannot close file %s: %s\n", file, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

FILE        *open_stream(const char *file) {
    FILE    *stream;

    if ((stream = fopen(file, "r")) == NULL) {
        fprintf(stderr, "Cannot open file %s: %s\n", file, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return stream;
}

off_t               file_len(const char *path) {
    struct stat     info;

    if (stat(path, &info) == -1) {
        fprintf(stderr, "Cannot get file info %s: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return info.st_size;
}

char        *read_file(const char *file) {
    char    *buffer;
    off_t   size;
    int     fd;

    if ((fd = open(file, O_RDONLY)) == -1) {
        fprintf(stderr, "Cannot open file %s: %s\n", file, strerror(errno));
        exit(EXIT_FAILURE);
    }
    size = file_len(file);
    buffer = alloc_buffer(size + 1);
    if (read(fd, buffer, size) == -1) {
        fprintf(stderr, "Cannot read file %s: %s\n", file, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (close(fd) == -1) {
        fprintf(stderr, "Cannot close file %s: %s\n", file, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return buffer;
}
