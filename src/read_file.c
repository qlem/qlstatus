// By Clément Dommerc

#include "qlstatus.h"

void    close_file(int fd, const char *path) {
    if (close(fd) == -1) {
        printf("Cannot close file '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int     open_file(const char *path, int flags) {
    int     fd = 0;

    if ((fd = open(path, flags)) == -1) {
        printf("Cannot open file '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}

off_t   get_file_size(const char *path) {
    struct stat     info;

    if (stat(path, &info) == -1) {
        printf("Cannot get file info '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return info.st_size;
}

char    *read_file(const char *path) {
    char    *buffer;
    int     fd;
    size_t  size;

    fd = open_file(path, O_RDONLY);
    size = (size_t)get_file_size(path);
    buffer = alloc_buffer(size + 1);
    if (read(fd, buffer, size) == -1) {
        printf("Cannot read file '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    close_file(fd, path);
    return buffer;
}
