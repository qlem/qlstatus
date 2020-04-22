// By Clément Dommerc

#include "qlstatus.h"

void    v_sleep() {
    struct timespec     tp;

    tp.tv_sec = 1;
    tp.tv_nsec = 0;
    if (clock_nanosleep(CLOCK_REALTIME, 0, &tp, NULL)) {
        perror("Call to 'clock_nanosleep()' failed");
        exit(EXIT_FAILURE);
    }
}

int     main() {
    char    *battery;
    char    *brightness;
    char    *cpu_temp;

    while (1) {
        battery = get_battery();
        brightness = get_brightness();
        cpu_temp = get_cpu_temp();
        print("t  t  t\n", cpu_temp, brightness, battery);
        free(battery);
        free(brightness);
        free(cpu_temp);
        v_sleep();
    }
    return 0;
}
