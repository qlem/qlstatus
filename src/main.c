// By Clément Dommerc

#include "qlstatus.h"

char    *read_file(char *path) {
    char            *buffer;
    struct stat     info;
    int             fd;
    size_t          size;

    if ((fd = open(path, O_RDONLY)) == -1) {
        exit(1);
    }
    if (stat(path, &info) == -1) {
        exit(1);
    }
    size = (size_t)info.st_size;
    buffer = alloc_buffer(size + 1);
    if (read(fd, buffer, size) == -1) {
       exit(1);
    }
    if (close(fd) == -1) {
        exit(1);
    }
    return buffer;
}

void        print(char *fmt, ...) {
    char        *str = NULL;
    char        c = 0;
    va_list     ap;

    va_start(ap, fmt);
    while (*fmt) {
        switch (*fmt) {
            case 't':
                str = va_arg(ap, char *);
                write(0, str, strlen(str));
                break;
            default:
                c = (char)(*fmt);
                write(0, &c, 1);
                break;
        }
        fmt++;
    }
    va_end(ap);
}

int     main() {
    struct timespec     tp;
    char                *battery;

    tp.tv_sec = 1;
    tp.tv_nsec = 0;
    while (1) {
        battery = get_battery();
        print("t", battery);
        free(battery);
        clock_nanosleep(CLOCK_REALTIME, 0, &tp, NULL);
    }
    return 0;
}
