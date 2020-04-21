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

int     main() {
    struct timespec     tp;
    char                *battery;
    char                *brightness;
    char                *cpu_temp;

    tp.tv_sec = 1;
    tp.tv_nsec = 0;
    while (1) {
        battery = get_battery();
        brightness = get_brightness();
        cpu_temp = get_cpu_temp();
        print("t  t  t\n", cpu_temp, brightness, battery);
        free(battery);
        free(brightness);
        free(cpu_temp);
        clock_nanosleep(CLOCK_REALTIME, 0, &tp, NULL);
    }
    return 0;
}
