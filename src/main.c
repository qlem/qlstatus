/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

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

int             resolve_rate(t_main *main, struct timespec *tp) {
    char        **unit;
    char        **buf;
    long        val;
    uint64_t    nsec;

    unit = multiple_subs("^[0-9]+(s)$|^[0-9]+(ms)$", main->rate, 2);
    buf = multiple_subs("^([0-9]+)s$|^([0-9]+)ms$", main->rate, 2);
    val = buf[0] ? to_int(buf[0]) : to_int(buf[1]);
    if (unit[0]) {
        tp->tv_sec = val;
        tp->tv_nsec = 0;
        free(unit[0]);
        free(buf[0]);
    } else if (unit[1]) {
        nsec = val * (long)1e6;
        tp->tv_sec = SEC(nsec);
        tp->tv_nsec = NSEC(nsec);
        free(unit[1]);
        free(buf[1]);
    } else {
        tp->tv_sec = 1;
        tp->tv_nsec = 0;
    }
    free(unit);
    free(buf);
    return 0;
}

int                     create_thread(t_module *module) {
    pthread_attr_t      attr;
    int                 err = 0;

    if ((err = pthread_attr_init(&attr)) != 0) {
        printf("Thread error: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = pthread_attr_setdetachstate(&attr,
                                PTHREAD_CREATE_JOINABLE)) != 0) {
        printf("Thread error: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = pthread_create(&module->thread, &attr, module->routine,
                                module)) != 0) {
        printf("Thread error: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = pthread_attr_destroy(&attr) != 0)) {
        printf("Thread error: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    return 0;
}

int     main(int argc, char **argv, char **env) {

    (void)argc;
    (void)argv;

    // global options
    t_opt       opts_main[GLOBAL_OPTS] = {
        {OPT_FORMAT, DEFAULT_FORMAT, OPT_FORMAT_PATTERN, OPT_OTHER},
        {OPT_RATE,   RATE,           OPT_RATE_PATTERN,   OPT_OTHER}
    };

    // battery options
    t_opt       opts_battery[BATTERY_OPTS] = {
        {OPT_BAT_ENABLED, "1",                OPT_BOOLEAN_PATTERN,  OPT_STATE},
        {OPT_BAT_LB_UNK,  BATTERY_LABEL_UNK,  OPT_LABEL_PATTERN,    OPT_LABEL},
        {OPT_BAT_LB_FULL, BATTERY_LABEL_FULL, OPT_LABEL_PATTERN,    OPT_OTHER},
        {OPT_BAT_LB_CHR,  BATTERY_LABEL_CHR,  OPT_LABEL_PATTERN,    OPT_OTHER},
        {OPT_BAT_LB_DIS,  BATTERY_LABEL_DIS,  OPT_LABEL_PATTERN,    OPT_OTHER},
        {OPT_BAT_NAME,    BATTERY_NAME,       OPT_BAT_NAME_PATTERN, OPT_OTHER},
        {OPT_BAT_CRITIC,  "20",               OPT_NUMBER_PATTERN,   OPT_OTHER}
    };

    // cpu usage options
    t_opt       opts_cpu_usage[CPU_USAGE_OPTS] = {
        {OPT_UCPU_ENABLED, "1",             OPT_BOOLEAN_PATTERN, OPT_STATE},
        {OPT_UCPU_LABEL,   CPU_USAGE_LABEL, OPT_LABEL_PATTERN,   OPT_LABEL},
        {OPT_UCPU_CRITIC,  "80",            OPT_NUMBER_PATTERN,  OPT_OTHER}
    };

    // cpu temp options
    t_opt       opts_cpu_temp[CPU_TEMP_OPTS] = {
        {OPT_TCPU_ENABLED, "1",            OPT_BOOLEAN_PATTERN, OPT_STATE},
        {OPT_TCPU_LABEL,   CPU_TEMP_LABEL, OPT_LABEL_PATTERN,   OPT_LABEL},
        {OPT_TCPU_DIR,     CPU_TEMP_DIR,   OPT_PATH_PATTERN,    OPT_OTHER},
        {OPT_TCPU_INPUT,   "1",            OPT_IN_TEMP_PATTERN, OPT_OTHER},
        {OPT_TCPU_CRITIC,  "70",           OPT_NUMBER_PATTERN,  OPT_OTHER}
    };

    // memory options
    t_opt       opts_memory[MEM_OPTS] = {
        {OPT_MEM_ENABLED, "1",       OPT_BOOLEAN_PATTERN, OPT_STATE},
        {OPT_MEM_LABEL,   MEM_LABEL, OPT_LABEL_PATTERN,   OPT_LABEL},
        {OPT_MEM_CRITIC,  "80",      OPT_NUMBER_PATTERN,  OPT_OTHER}
    };

    // brightness options
    t_opt       opts_brightness[BRIGHTNESS_OPTS] = {
        {OPT_BRG_ENABLED, "1",              OPT_BOOLEAN_PATTERN, OPT_STATE},
        {OPT_BRG_LABEL,   BRIGHTNESS_LABEL, OPT_LABEL_PATTERN,   OPT_LABEL},
        {OPT_BRG_DIR,     BRIGHTNESS_DIR,   OPT_PATH_PATTERN,    OPT_OTHER}
    };

    // volume options
    t_opt       opts_volume[VOLUME_OPTS] = {
        {OPT_VOL_ENABLED,  "1",                OPT_BOOLEAN_PATTERN, OPT_STATE},
        {OPT_VOL_LABEL,    VOLUME_LABEL,       OPT_LABEL_PATTERN,   OPT_LABEL},
        {OPT_VOL_LB_MUTED, VOLUME_MUTED_LABEL, OPT_LABEL_PATTERN,   OPT_OTHER},
        {OPT_VOL_SINK,     PULSE_SINK_NAME,    OPT_TEXT_PATTERN,    OPT_OTHER}
    };

    // wireless options
    t_opt       opts_wireless[WIRELESS_OPTS] = {
        {OPT_WLAN_ENABLED, "1",                OPT_BOOLEAN_PATTERN, OPT_STATE},
        {OPT_WLAN_LB_UNK,  WIRELESS_UNK_LABEL, OPT_LABEL_PATTERN,   OPT_LABEL},
        {OPT_WLAN_IFACE,   WIRELESS_INTERFACE, OPT_TEXT_PATTERN,    OPT_OTHER}
    };

    // extra data for cpu usage module
    t_cpu   cpu;
    cpu.prev_idle = 0;
    cpu.prev_total = 0;

    // extra data for volume module
    t_pulse     pulse;
    pulse.mainloop = NULL;
    pulse.context = NULL;
    pulse.connected = 0;

    // modules
    t_module            modules[NB_MODULES] = {
        {1, 'U', CPU_USAGE_LABEL, 0, "%", &cpu, opts_cpu_usage, CPU_USAGE_OPTS, get_cpu_usage, 0, 0},
        {1, 'T', CPU_TEMP_LABEL, 0, "°", NULL, opts_cpu_temp, CPU_TEMP_OPTS, get_cpu_temp, 0, 0},
        {1, 'M', MEM_LABEL, 0, "%", NULL, opts_memory, MEM_OPTS, get_memory, 0, 0},
        {1, 'L', BRIGHTNESS_LABEL, 0, "%", NULL, opts_brightness, BRIGHTNESS_OPTS, get_brightness, 0, 0},
        {1, 'V', VOLUME_LABEL, 0, "%", &pulse, opts_volume, VOLUME_OPTS, get_volume, 1, 0},
        {1, 'B', BATTERY_LABEL_UNK, 0, "%", NULL, opts_battery, BATTERY_OPTS, get_battery, 0, 0},
        {1, 'W', WIRELESS_UNK_LABEL, 0, "%", NULL, opts_wireless, WIRELESS_OPTS, get_wireless, 0, 0}
    };

    // vars declaration
    struct timespec tp;
    t_main          main;
    char            *config;
    char            *buffer;
    int             err = 0;
    int             i;

    // init global struct + load config file
    main.modules = modules;
    main.opts = opts_main;
    main.format = DEFAULT_FORMAT;
    main.rate = RATE;
    if ((config = resolve_config_file(env))) {
        parse_config_file(&main, config);
        free(config);
    }
    resolve_rate(&main, &tp);

    // main loop
    while (true) {
        // launch threaded modules
        i = -1;
        while (++i < NB_MODULES) {
            if (main.modules[i].enabled && main.modules[i].is_thread) {
                create_thread(&main.modules[i]);
            }
        }
        // execute other modules
        i = -1;
        while (++i < NB_MODULES) {
            if (main.modules[i].enabled && !main.modules[i].is_thread) {
                main.modules[i].routine(&main.modules[i]);
            }
        }
        // waiting for threaded modules
        i = -1;
        while (++i < NB_MODULES) {
            if (main.modules[i].enabled && main.modules[i].is_thread) {
                if ((err = pthread_join(main.modules[i].thread, NULL))) {
                    printf("Thread error: %s\n", strerror(err));
                    exit(EXIT_FAILURE);
                }
            }
        }
        // output
        buffer = format(&main);
        putstr(buffer);
        write(1, "\n", 1);
        free(buffer);
        v_sleep(tp.tv_sec, tp.tv_nsec);
    }
    return 0;
}
