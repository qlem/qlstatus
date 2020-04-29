/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void    v_sleep() {
    struct timespec     tp;

    tp.tv_sec = 1;
    tp.tv_nsec = 0;
    if (clock_nanosleep(CLOCK_REALTIME, 0, &tp, NULL)) {
        printf("Call to 'clock_nanosleep()' failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int         main() {
    t_cpu   *cpu;
    char    *battery;
    char    *brightness;
    char    *cpu_temp;
    char    *cpu_usage;
    char    *wireless;
    char    *memory;

    cpu = alloc_ptr(sizeof(t_cpu));
    cpu->prev_idle = 0;
    cpu->prev_total = 0;
    while (1) {
        battery = get_battery();
        brightness = get_brightness();
        cpu_temp = get_cpu_temp();
        cpu_usage = get_cpu_usage(cpu);
        wireless = get_wireless();
        memory = get_memory();
        print("t  t  t  t  t  t\n", cpu_usage, cpu_temp, memory, brightness, battery, wireless);
        free(battery);
        free(brightness);
        free(cpu_temp);
        free(cpu_usage);
        free(wireless);
        free(memory);
        v_sleep();
    }
    free(cpu);
    return 0;
}
