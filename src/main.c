/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

static uint8_t running = 1;

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
        tp->tv_sec = NSEC_TO_SEC(nsec);
        tp->tv_nsec = REM_NSEC(nsec);
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
        printf("Call to pthread_attr_init() failed: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = pthread_attr_setdetachstate(&attr,
        PTHREAD_CREATE_JOINABLE)) != 0) {
        printf("Call to pthread_attr_setdetachstate() failed: %s\n",
               strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = pthread_create(&module->thread, &attr, module->routine,
        module)) != 0) {
        printf("Call to pthread_create() failed: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = pthread_attr_destroy(&attr) != 0)) {
        printf("Call to pthread_attr_destroy() failed: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    return 0;
}

void    signal_handler(int signum) {
    (void)signum;
    running = 0;
}

void        free_resources(t_main *main) {
    int     i = -1;
    int     j;

    // free global option values
    while (++i < GLOBAL_OPTS) {
        if (main->opts[i].to_free) {
            free(main->opts[i].value);
        }
    }

    i = -1;
    while (++i < NB_MODULES) {
        j = -1;
        // free module extra data
        if (main->modules[i].enabled && main->modules[i].mfree) {
            main->modules[i].mfree(&main->modules[i]);
        }
        // free module option values
        while (++j < main->modules[i].s_opts) {
            if (main->modules[i].opts[j].to_free) {
                free(main->modules[i].opts[j].value);
            }
        }
    }
}

void        subtract_time(struct timespec *end, struct timespec *start,
                          struct timespec *diff) {
    long    sdiff = end->tv_sec - start->tv_sec;
    long    nsdiff = end->tv_nsec - start->tv_nsec;

    if (sdiff < 0 || (sdiff == 0 && nsdiff < 0)) {
        diff->tv_sec = 0;
        diff->tv_nsec = 0;
    } else if (nsdiff < 0) {
        diff->tv_sec = sdiff - 1;
        diff->tv_nsec = NSEC + nsdiff;
    } else {
        diff->tv_sec = sdiff;
        diff->tv_nsec = nsdiff;
    }
}

int     main(int argc, char **argv, char **env) {

    (void)argc;
    (void)argv;

    // global options
    int     spwm_color = 0;
    int     color_idx = 1;
    t_opt   opts_global[GLOBAL_OPTS] = {
        {OPT_FORMAT, DEFAULT_FORMAT, FORMAT_PATTERN, STRING, OTHER, 0},
        {OPT_RATE, RATE, RATE_PATTERN, STRING, OTHER, 0},
        {OPT_SPWM_COLOR, &spwm_color, BOOLEAN_PATTERN, NUMBER, OTHER, 0},
        {OPT_COLOR_IDX, &color_idx, COLOR_IDX_PATTERN, NUMBER, OTHER, 0}};

    // battery options
    int     bat_enabled = 1;
    int     bat_threshold = 20;
    int     bat_full_design = 1;
    t_opt       opts_battery[BATTERY_OPTS] = {
        {OPT_BAT_ENABLED, &bat_enabled,   BOOLEAN_PATTERN,   NUMBER, STATE,  0},
        {OPT_BAT_LB_UNK,  BAT_LABEL_UNK,  LABEL_PATTERN,     STRING, LABEL,  0},
        {OPT_BAT_LB_FULL, BAT_LABEL_FULL, LABEL_PATTERN,     STRING, OTHER,  0},
        {OPT_BAT_LB_CHR,  BAT_LABEL_CHR,  LABEL_PATTERN,     STRING, OTHER,  0},
        {OPT_BAT_LB_DIS,  BAT_LABEL_DIS,  LABEL_PATTERN,     STRING, OTHER,  0},
        {OPT_BAT_NAME,    BATTERY_NAME,   BAT_NAME_PATTERN,  STRING, OTHER,  0},
        {OPT_BAT_CRITIC,  &bat_threshold, THRESHOLD_PATTERN, NUMBER, CRITIC, 0},
        {OPT_BAT_FULL_DESIGN, &bat_full_design, BOOLEAN_PATTERN, NUMBER, OTHER, 0}
    };

    // cpu usage options
    int     cpu_enabled = 1;
    int     cpu_threshold = 80;
    t_opt opts_cpu_usage[CPU_OPTS] = {
        {OPT_CPU_ENABLED, &cpu_enabled,   BOOLEAN_PATTERN,   NUMBER, STATE,  0},
        {OPT_CPU_LABEL,   CPU_LABEL,      LABEL_PATTERN,     STRING, LABEL,  0},
        {OPT_CPU_CRITIC,  &cpu_threshold, THRESHOLD_PATTERN, NUMBER, CRITIC, 0}
    };

    // temperature options
    int     temp_enabled = 1;
    int     temp_threshold = 80;
    t_opt       opts_temperature[TEMP_OPTS] = {
        {OPT_TEMP_ENABLED, &temp_enabled,   BOOLEAN_PATTERN,   NUMBER, STATE,  0},
        {OPT_TEMP_LABEL,   TEMP_LABEL,      LABEL_PATTERN,     STRING, LABEL,  0},
        {OPT_TEMP_DIR,     TEMP_DIR,        PATH_PATTERN,      STRING, OTHER,  0},
        {OPT_TEMP_INPUT,   "1",             IN_TEMP_PATTERN,   STRING, OTHER,  0},
        {OPT_TEMP_CRITIC,  &temp_threshold, THRESHOLD_PATTERN, NUMBER, CRITIC, 0}
    };

    // memory options
    int     mem_enabled = 1;
    int     mem_threshold = 80;
    t_opt       opts_memory[MEM_OPTS] = {
        {OPT_MEM_ENABLED, &mem_enabled,   BOOLEAN_PATTERN,   NUMBER, STATE,  0},
        {OPT_MEM_LABEL,   MEM_LABEL,      LABEL_PATTERN,     STRING, LABEL,  0},
        {OPT_MEM_CRITIC,  &mem_threshold, THRESHOLD_PATTERN, NUMBER, CRITIC, 0}
    };

    // brightness options
    int     brg_enabled = 1;
    t_opt       opts_brightness[BRIGHTNESS_OPTS] = {
        {OPT_BRG_ENABLED, &brg_enabled,     BOOLEAN_PATTERN, NUMBER, STATE, 0},
        {OPT_BRG_LABEL,   BRIGHTNESS_LABEL, LABEL_PATTERN,   STRING, LABEL, 0},
        {OPT_BRG_DIR,     BRIGHTNESS_DIR,   PATH_PATTERN,    STRING, OTHER, 0}
    };

    // volume options
    int     volume_enabled = 1;
    t_opt       opts_volume[VOLUME_OPTS] = {
        {OPT_VOL_ENABLED,  &volume_enabled,    BOOLEAN_PATTERN, NUMBER, STATE, 0},
        {OPT_VOL_LABEL,    VOLUME_LABEL,       LABEL_PATTERN,   STRING, LABEL, 0},
        {OPT_VOL_LB_MUTED, VOLUME_MUTED_LABEL, LABEL_PATTERN,   STRING, OTHER, 0},
        {OPT_VOL_SINK,     PULSE_SINK_NAME,    TEXT_PATTERN,    STRING, OTHER, 0}
    };

    // wireless options
    int     wireless_enabled = 1;
    t_opt       opts_wireless[WIRELESS_OPTS] = {
        {OPT_WLAN_ENABLED, &wireless_enabled,  BOOLEAN_PATTERN,  NUMBER, STATE, 0},
        {OPT_WLAN_LB_UNK,  WIRELESS_UNK_LABEL, WL_LABEL_PATTERN, STRING, LABEL, 0},
        {OPT_WLAN_IFACE,   WIRELESS_INTERFACE, TEXT_PATTERN,     STRING, OTHER, 0}
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
    t_module    modules[NB_MODULES] = {
        {1, 'W', WIRELESS_UNK_LABEL, 0, "%", 0, 0, NULL,   opts_wireless,    WIRELESS_OPTS,   get_wireless, wireless_free, 0},
        {1, 'B', BAT_LABEL_UNK,      0, "%", 0, 0, NULL,   opts_battery,     BATTERY_OPTS,    get_battery,     NULL,       0},
        {1, 'L', BRIGHTNESS_LABEL,   0, "%", 0, 0, NULL,   opts_brightness,  BRIGHTNESS_OPTS, get_brightness,  NULL,       0},
        {1, 'M', MEM_LABEL,          0, "%", 0, 0, NULL,   opts_memory,      MEM_OPTS,        get_memory,      NULL,       0},
        {1, 'T', TEMP_LABEL,         0, "°", 0, 0, NULL,   opts_temperature, TEMP_OPTS,       get_temperature, NULL,       0},
        {1, 'U', CPU_LABEL,          0, "%", 0, 0, &cpu,   opts_cpu_usage,   CPU_OPTS,        get_cpu_usage,   NULL,       0},
        {1, 'V', VOLUME_LABEL,       0, "%", 0, 0, &pulse, opts_volume,      VOLUME_OPTS,     get_volume, volume_free,     0}
    };

    // vars declaration
    struct timespec     rate;
    struct timespec     start;
    struct timespec     end;
    struct timespec     itime;
    struct timespec     prate;
    struct sigaction    act;
    t_main              main;
    char                *config;
    char                *buffer;
    int                 err = 0;
    int                 i;

    // init signal handler
    v_memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = signal_handler;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    // init main strutcure
    main.modules = modules;
    main.opts = opts_global;
    main.format = DEFAULT_FORMAT;
    main.rate = RATE;
    main.spwm_color = 0;
    main.color_idx = 1;

    // resolve/load config file
    if ((config = resolve_config_file(env))) {
        parse_config_file(&main, config);
        free(config);
    }
    resolve_rate(&main, &rate);

    // main loop
    while (1) {

        // free resources on exit
        if (!running) {
            printf("Exiting ql-status\n");
            free_resources(&main);
            return 0;
        }

        // store start time
        if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
            printf("Call to clock_gettime() failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        // create and start a new thread for each module
        i = -1;
        while (++i < NB_MODULES) {
            if (main.modules[i].enabled) {
                create_thread(&main.modules[i]);
            }
        }

        // waiting for all threads to finish
        i = -1;
        while (++i < NB_MODULES) {
            if (main.modules[i].enabled) {
                if ((err = pthread_join(main.modules[i].thread, NULL))) {
                    printf("Call to pthread_join() failed: %s\n",
                           strerror(err));
                    exit(EXIT_FAILURE);
                }
            }
        }

        // output
        buffer = format(&main);
        putstr(buffer);
        free(buffer);

        // adjust rate based on the execution time of the loop iteration
        if (clock_gettime(CLOCK_REALTIME, &end) == -1) {
            printf("Call to clock_gettime() failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        subtract_time(&end, &start, &itime);
        subtract_time(&rate, &itime, &prate);

        // waiting
        if ((err = clock_nanosleep(CLOCK_REALTIME, 0, &prate, NULL))) {
            if (err != EINTR) {
                printf("Call to clock_nanosleep() failed: %s\n", strerror(err));
                exit(EXIT_FAILURE);
            }
        }
    }
    return 0;
}
