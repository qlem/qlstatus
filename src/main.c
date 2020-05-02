/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void        init_volume(t_module *module) {
    module->enabled = 0;
    module->fmtid = 'V';
    module->label = VOLUME_LABEL;
    module->value = 0;
    module->unit = "%";
    module->routine = get_volume;
    module->is_thread = 1;
}

void        init_wireless(t_module *module) {
    module->enabled = 1;
    module->fmtid = 'W';
    module->label = WIRELESS_UNK_ESSID_LABEL;
    module->value = 0;
    module->unit = "%";
    module->routine = get_wireless;
    module->is_thread = 0;
}

void        init_memory(t_module *module) {
    module->enabled = 1;
    module->fmtid = 'M';
    module->label = MEM_LABEL;
    module->value = 0;
    module->unit = "%";
    module->routine = get_memory;
    module->is_thread = 0;
}

void        init_cpu_temp(t_module *module) {
    module->enabled = 1;
    module->fmtid = 'T';
    module->label = CPU_TEMP_LABEL;
    module->value = 0;
    module->unit = "°";
    module->routine = get_cpu_temp;
    module->is_thread = 0;
}

void        init_cpu_usage(t_module *module) {
    module->enabled = 1;
    module->fmtid = 'U';
    module->label = CPU_USAGE_LABEL;
    module->value = 0;
    module->unit = "%";
    module->routine = get_cpu_usage;
    module->is_thread = 0;
}

void        init_battery(t_module *module) {
    module->enabled = 1;
    module->fmtid = 'B';
    module->label = BAT_LABEL_UNKNOW;
    module->value = 0;
    module->unit = "%";
    module->routine = get_battery;
    module->is_thread = 0;
}

void        init_brightness(t_module *module) {
    module->enabled = 1;
    module->fmtid = 'L';
    module->label = BRIGHTNESS_LABEL;
    module->value = 0;
    module->unit = "%";
    module->routine = get_brightness;
    module->is_thread = 0;
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
    init_cpu_temp(&main.modules[2]);
    init_cpu_usage(&main.modules[3]);
    main.modules[3].args = &cpu;
    init_memory(&main.modules[4]);
    init_volume(&main.modules[5]);
    init_wireless(&main.modules[6]);

    main.format = "%U  %T  %M  %L  %B  %W";

    while (++i < NB_MODULES) {
        if (main.modules[i].enabled && main.modules[i].is_thread) {
            // TODO set attr
            if (pthread_create(&main.modules[i].thread, NULL, main.modules[i].routine, &main.modules[i]) != 0) {
                printf("Cannot create a new thread: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
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

    // MEMORY LEAKS
    // essid label
    // lot of libpulse stuff

    return 0;
}
