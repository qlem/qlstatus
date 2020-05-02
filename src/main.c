/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

int                     create_thread(t_module *module) {
    pthread_attr_t      attr;

    if (pthread_attr_init(&attr) != 0) {
        printf("Cannot init thread attribute\n");
        exit(EXIT_FAILURE);
    }
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        printf("Cannot set detach state of thread attribute\n");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&module->thread, &attr, module->routine, module) != 0) {
        printf("Cannot create a new thread: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (pthread_attr_destroy(&attr) != 0) {
        printf("Cannot destroy thread attribute\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int                     main() {
    t_main              main;
    t_cpu               cpu;
    char                *buffer;
    int                 i = -1;

    cpu.prev_idle = 0;
    cpu.prev_total = 0;

    init_battery(&main.modules[0]);
    init_brightness(&main.modules[1]);
    init_cpu_usage(&main.modules[2]);
    main.modules[2].args = &cpu;
    init_cpu_temp(&main.modules[3]);
    init_memory(&main.modules[4]);
    init_volume(&main.modules[5]);
    init_wireless(&main.modules[6]);

    main.format = DEFAULT_FORMAT;

    while (++i < NB_MODULES) {
        if (main.modules[i].enabled && main.modules[i].is_thread) {
            create_thread(&main.modules[i]);
        }
    }

    while (true) {
        i = -1;
        while (++i < NB_MODULES) {
            if (main.modules[i].enabled && !main.modules[i].is_thread) {
                main.modules[i].routine(&main.modules[i]);
            }
        }
        buffer = format(&main);
        putstr(buffer);
        write(1, "\n", 1);
        free(buffer);
        v_sleep(SEC((long)RATE), NSEC((long)RATE));
    }
    return 0;
}
