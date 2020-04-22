// By Clément Dommerc

#include "qlstatus.h"

void    close_file(int fd, const char *path) {
    int     errsv = 0;

    if (close(fd) == -1) {
        if (errno) {
            errsv = errno;
            printf("Cannot close file %s: %s\n", path, strerror(errsv));
        } else {
            printf("Cannot close file %s\n", path);
        }
        exit(EXIT_FAILURE);
    }
}

int     open_file(const char *path, int flags) {
    int     fd = 0;
    int     errsv = 0;

    if ((fd = open(path, flags)) == -1) {
        if (errno) { 
            errsv = errno;
            printf("Cannot open file %s: %s\n", path, strerror(errsv));
        } else {
            printf("Cannot open file %s\n", path);
        }
        exit(EXIT_FAILURE);
    }
    return fd;
}

off_t   get_file_size(const char *path) {
    struct stat     info;
    int     errsv = 0;

    if (stat(path, &info) == -1) {
        if (errno) { 
            errsv = errno;
            printf("Cannot get file info %s: %s\n", path, strerror(errsv));
        } else {
            printf("Cannot get file info %s\n", path);
        }
        exit(EXIT_FAILURE);
    }
    return info.st_size;
}

char    *read_file(const char *path) {
    char    *buffer;
    int     fd;
    size_t  size;
    int     errsv = 0;      

    fd = open_file(path, O_RDONLY);
    size = (size_t)get_file_size(path);
    buffer = alloc_buffer(size + 1);
    if (read(fd, buffer, size) == -1) {
        if (errno) {
            errsv = errno;
            printf("Cannot read file %s: %s\n", path, strerror(errsv));
        } else {
            printf("Cannot read file %s\n", path);
        }
        exit(EXIT_FAILURE);
    }
    close_file(fd, path);
    return buffer;
}
