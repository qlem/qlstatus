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

char        *resolve_config_file(char **env) {
    int     i = -1;
    char    *home = NULL;
    char    *config = NULL;

    while (env[++i] && !(home = substring(HOME_PATTERN, env[i]))) {}
    if (!home) {
        return NULL;
    }
    config = alloc_buffer(v_strlen(home) + v_strlen(CONFIG_FILE) + 2);
    sprintf(config, "%s/%s", home, CONFIG_FILE);
    free(home);
    return config;
}

int             main(int argc, char **argv, char **env) {
    t_main      main;
    char        *buffer;
    char        *config;
    int         i = -1;

    (void)argc;
    (void)argv;

    t_opt       opts_battery[BATTERY_OPTS] = {
        {"battery_enabled", "1", OPT_BOOLEAN_PATTERN},
        {"battery_name", BATTERY_NAME, "^BAT[0-9]$"},
        {"battery_label_full", BATTERY_LABEL_FULL, OPT_LABEL_PATTERN},
        {"battery_label_charging", BATTERY_LABEL_CHARGING, OPT_LABEL_PATTERN},
        {"battery_label_discharging", BATTERY_LABEL_DISCHARGING, OPT_LABEL_PATTERN},
        {"battery_label_unknow", BATTERY_LABEL_UNKNOW, OPT_LABEL_PATTERN},
        {"battery_critical", "20", OPT_NUMBER_PATTERN}
    };

    t_opt       opts_cpu_usage[CPU_USAGE_OPTS] = {
        {"cpu_usage_enabled", "1", OPT_BOOLEAN_PATTERN},
        {"cpu_usage_label", "cpu", OPT_LABEL_PATTERN},
    };

    t_opt       opts_cpu_temp[CPU_TEMP_OPTS] = {
        {"cpu_temp_enabled", "1", OPT_BOOLEAN_PATTERN},
        {"cpu_temp_label", "temp", OPT_LABEL_PATTERN},
    };

    t_opt       opts_memory[MEM_OPTS] = {
        {"memory_enabled", "1", OPT_BOOLEAN_PATTERN},
        {"memory_label", "mem", OPT_LABEL_PATTERN},
    };

    t_opt       opts_brightness[BRIGHTNESS_OPTS] = {
        {"brightness_enabled", "1", OPT_BOOLEAN_PATTERN},
        {"brightness_label", "brg", OPT_LABEL_PATTERN},
    };

    t_opt       opts_volume[VOLUME_OPTS] = {
        {"volume_enabled", "1", OPT_BOOLEAN_PATTERN},
        {"volume_label", "vol", OPT_LABEL_PATTERN},
    };

    t_opt       opts_wireless[WIRELESS_OPTS] = {
        {"wireless_enabled", "1", OPT_BOOLEAN_PATTERN}
    };

    t_module            modules[NB_MODULES] = {
        {0, 'U', CPU_USAGE_LABEL, 0, "%", &opts_cpu_usage, CPU_USAGE_OPTS, get_cpu_usage, 0, 0},
        {0, 'T', CPU_TEMP_LABEL, 0, "%", &opts_cpu_temp, CPU_TEMP_OPTS, get_cpu_temp, 0, 0},
        {0, 'M', MEM_LABEL, 0, "%", &opts_memory, MEM_OPTS, get_memory, 0, 0},
        {0, 'L', BRIGHTNESS_LABEL, 0, "%", &opts_brightness, BRIGHTNESS_OPTS, get_brightness, 0, 0},
        {0, 'V', VOLUME_LABEL, 0, "%", &opts_volume, VOLUME_OPTS, get_volume, 1, 0},
        {1, 'B', BATTERY_LABEL_UNKNOW, 0, "%", &opts_battery, BATTERY_OPTS, get_battery, 0, 0},
        {0, 'W', WIRELESS_UNK_ESSID_LABEL, 0, "%", &opts_wireless, WIRELESS_OPTS, get_wireless, 0, 0}
    };

    main.modules = modules;
    main.format = "%B";

    if ((config = resolve_config_file(env))) {
        parse_config_file(&main, config);
        free(config);
    }

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
