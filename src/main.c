/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void        free_resources(t_main *main) {
    int     i = -1;
    int     j;

    // free global option values
    while (++i < GLOBAL_NOPTS) {
        if (main->opts[i].to_free) {
            free(main->opts[i].value);
        }
    }

    i = -1;
    while (++i < NB_MODULES) {
        j = -1;
        // free module extra data
        if (main->modules[i].enabled) {
            main->modules[i].mfree(&main->modules[i]);
        }
        // free module option values
        while (++j < main->modules[i].nopts) {
            if (main->modules[i].opts[j].to_free) {
                free(main->modules[i].opts[j].value);
            }
        }
    }

    // free libnotify
    notify_uninit();
}

void        print_usage() {
    putstr("Usage: qlstatus [OPTIONS]\n\n");
    putstr("Options:\n");
    putstr("  -h, --help            Print usage statement\n");
    putstr("  -v, --version         Print version and exit\n");
    putstr("  -c, --config <file>   Load settings from specified configuration file\n");
}

void        signal_handler(int signum) {
    (void)signum;
    putstr("\n(interrupt) Exiting ql-status.\n");
    exit(EXIT_SUCCESS);
}

char        *resolve_user_config(char **env) {
    char    *config = NULL;
    char    *home = NULL;
    int     i = -1;

    while (env[++i] && !(home = substring(HOME_PATTERN, env[i]))) {}
    if (!home) {
        fprintf(stderr, "Cannot resolve config file: HOME dir not found\n");
        return NULL;
    }
    config = alloc_buffer(v_strlen(home) + v_strlen(USERCONF) + 2);
    sprintf(config, "%s/%s", home, USERCONF);
    free(home);
    return config;
}

int             resolve_rate(t_main *main, struct timespec *tp) {
    char        **unit;
    char        **buf;
    uint64_t    nsec;
    long        val;

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

void        compute_tick(struct timespec *ref, struct timespec *rate,
                         struct timespec *tick) {
    long    sec = ref->tv_sec + rate->tv_sec;
    long    nsec = ref->tv_nsec + rate->tv_nsec;

    if (nsec >= NSEC) {
        tick->tv_sec = sec + 1;
        tick->tv_nsec = nsec - NSEC;
    } else {
        tick->tv_sec = sec;
        tick->tv_nsec = nsec;
    }
}

int             main(int argc, char **argv, char **env) {
    char        *sconfig = NULL;

    // handle arguments
    if (argc == 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) {
        printf("%s\n", VERSION);
        return 0;
    }

    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage();
        return 0;
    }

    if (argc == 3 && (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--config") == 0)) {
        sconfig = argv[2];
    } else if (argc > 1) {
        print_usage();
        return EXIT_FAILURE;
    }

    // global options
    int         spwmcolors = 0;
    int         spwmcoloridx = 1;
    t_opt       opts_global[] = {
        {OPT_FORMAT,      DEFAULT_FORMAT, TEXT_PATTERN,      STRING, 0},
        {OPT_RATE,        RATE,           RATE_PATTERN,      STRING, 0},
        {OPT_SPWM_COLORS, &spwmcolors,    BOOLEAN_PATTERN,   NUMBER, 0},
        {OPT_C_COLOR_IDX, &spwmcoloridx,  COLOR_IDX_PATTERN, NUMBER, 0}
    };

    // time options
    t_opt       opts_time[] = {
        {"time_format", "%a %d %b %Y, %R %Z", TEXT_PATTERN, STRING, 0}
    };

    // battery options
    int         bat_threshold = 20;
    int         bat_full_design = 1;
    int         bat_notify = 1;
    t_opt       opts_bat[] = {
        {"battery_format",              "%L %V",          TEXT_PATTERN,      STRING, 0},
        {"battery_label_unknown",       "unk",            LABEL_PATTERN,     STRING, 0},
        {"battery_label_full",          "full",           LABEL_PATTERN,     STRING, 0},
        {"battery_label_charging",      "chr",            LABEL_PATTERN,     STRING, 0},
        {"battery_label_discharging",   "bat",            LABEL_PATTERN,     STRING, 0},
        {"battery_name",                "BAT0",           BAT_NAME_PATTERN,  STRING, 0},
        {"battery_critical",            &bat_threshold,   THRESHOLD_PATTERN, NUMBER, 0},
        {"battery_full_design",         &bat_full_design, BOOLEAN_PATTERN,   NUMBER, 0},
        {"power_notifications",         &bat_notify,      BOOLEAN_PATTERN,   NUMBER, 0},
        {"power_notify_icon_full",      NULL,             TEXT_PATTERN,      STRING, 0},
        {"power_notify_icon_plugged",   NULL,             TEXT_PATTERN,      STRING, 0},
        {"power_notify_icon_low",       NULL,             TEXT_PATTERN,      STRING, 0}
    };

    // cpu usage options
    int         cpu_threshold = 80;
    t_opt       opts_cpu[] = {
        {"cpu_format",   "%L %V",        TEXT_PATTERN,      STRING, 0},
        {"cpu_label",    "cpu",          LABEL_PATTERN,     STRING, 0},
        {"cpu_critical", &cpu_threshold, THRESHOLD_PATTERN, NUMBER, 0}
    };

    // cpu freq options
    int         scaling = 1;
    t_opt       opts_freq[] = {
        {"cpu_freq_format",  "%L %V%U", TEXT_PATTERN,      STRING, 0},
        {"cpu_freq_label",   "freq",    LABEL_PATTERN,     STRING, 0},
        {"cpu_freq_unit",    "MHz",     FREQ_UNIT_PATTERN, STRING, 0},
        {"cpu_freq_scaling", &scaling,  BOOLEAN_PATTERN,   NUMBER, 0}
    };

    // temperature options
    int         temp_threshold = 80;
    t_opt       opts_temp[] = {
        {"temperature_format",   "%L %V",         TEXT_PATTERN,      STRING, 0},
        {"temperature_label",    "temp",          LABEL_PATTERN,     STRING, 0},
        {"temperature_dir",      TEMP_DIR,        PATH_PATTERN,      STRING, 0},
        {"temperature_input",    "1",             IN_TEMP_PATTERN,   STRING, 0},
        {"temperature_critical", &temp_threshold, THRESHOLD_PATTERN, NUMBER, 0}
    };

    // memory options
    int         mem_threshold = 80;
    t_opt       opts_mem[] = {
        {"memory_format",   "%L %C/%T%U (%P)", TEXT_PATTERN,      STRING, 0},
        {"memory_label",    "mem",             LABEL_PATTERN,     STRING, 0},
        {"memory_unit",     "MiB",             MEM_UNIT_PATTERN,  STRING, 0},
        {"memory_critical", &mem_threshold,    THRESHOLD_PATTERN, NUMBER, 0}
    };

    // brightness options
    t_opt       opts_brg[] = {
        {"brightness_format", "%L %V", TEXT_PATTERN,  STRING, 0},
        {"brightness_label",  "brg",   LABEL_PATTERN, STRING, 0},
        {"brightness_dir",    BRG_DIR, PATH_PATTERN,  STRING, 0}
    };

    // volume options
    t_opt       opts_vol[] = {
        {"volume_format",      "%L %V",         TEXT_PATTERN,  STRING, 0},
        {"volume_label",       "vol",           LABEL_PATTERN, STRING, 0},
        {"volume_muted_label", "mut",           LABEL_PATTERN, STRING, 0},
        {"volume_sink_name",   PULSE_SINK_NAME, TEXT_PATTERN,  STRING, 0}
    };

    // wireless options
    t_opt       opts_wlan[] = {
        {"wireless_format",        "%L: %V",   TEXT_PATTERN,     STRING, 0},
        {"wireless_unknown_label", "SSID unk", WL_LABEL_PATTERN, STRING, 0},
        {"wireless_interface",     "wlan0",    TEXT_PATTERN,     STRING, 0}
    };

    // time
    t_mtime     time;
    v_memset(&time, 0, sizeof(t_mtime));

    // battery data
    t_power     power;
    v_memset(&power, 0, sizeof(t_power));

    // cpu data
    t_cpu   cpu;
    v_memset(&cpu, 0, sizeof(t_cpu));

    // cpu freq data
    t_freq  freq;
    v_memset(&freq, 0, sizeof(t_freq));

    // mem data
    t_mem   mem;
    v_memset(&mem, 0, sizeof(t_mem));

    // temp data
    t_temp      temp;
    v_memset(&temp, 0, sizeof(t_temp));

    // brightness data
    t_brg       brg;
    v_memset(&brg, 0, sizeof(t_brg));

    // volume data
    t_pulse     pulse;
    v_memset(&pulse, 0, sizeof(t_pulse));

    // wireless data
    t_wlan      wlan;
    v_memset(&wlan, 0, sizeof(t_wlan));

    // modules
    t_module modules[] = {
        {0, 'V', {}, 0, &pulse, opts_vol,  VOL_NOPTS,  0, run_volume,      init_volume,      free_volume},
        {0, 'W', {}, 0, &wlan,  opts_wlan, WLAN_NOPTS, 0, run_wireless,    init_wireless,    free_wireless},
        {0, 'T', {}, 0, &temp,  opts_temp, TEMP_NOPTS, 0, run_temperature, init_temperature, free_temperature},
        {0, 'B', {}, 0, &power, opts_bat,  BAT_NOPTS,  0, run_battery,     init_battery,     free_battery},
        {0, 'L', {}, 0, &brg,   opts_brg,  BRG_NOPTS,  0, run_brightness,  init_brightness,  free_brightness},
        {0, 'M', {}, 0, &mem,   opts_mem,  MEM_NOPTS,  0, run_memory,      init_memory,      free_memory},
        {0, 'U', {}, 0, &cpu,   opts_cpu,  CPU_NOPTS,  0, run_cpu_usage,   init_cpu_usage,   free_cpu_usage},
        {0, 'F', {}, 0, &freq,  opts_freq, FREQ_NOPTS, 0, run_cpu_freq,    init_cpu_freq,    free_cpu_freq},
        {0, 'D', {}, 0, &time,  opts_time, TIME_NOPTS, 0, run_time,        init_time,        free_time}
    };

    // vars declaration
    struct timespec     rate;
    struct timespec     ref;
    struct timespec     tick;
    struct sigaction    act;
    char                *config;
    int                 i = -1;
    t_main              main;
    int                 err;

    errno = 0;
    // init signal handler
    v_memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = signal_handler;
    if (sigemptyset(&act.sa_mask) == -1) {
        fprintf(stderr, "Call to sigemptyset() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGINT, &act, NULL) == -1) {
        fprintf(stderr, "Call to sigaction() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &act, NULL) == -1) {
        fprintf(stderr, "Call to sigaction() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // init main structure
    v_memset(&main, 0, sizeof(t_main));
    main.modules = modules;
    main.opts = opts_global;
    main.format = DEFAULT_FORMAT;
    main.rate = RATE;
    main.spwmcolors = 0;
    main.spwmcoloridx = 1;

    // init libnotify
    if (!notify_init("qlstatus")) {
        fprintf(stderr, "Call to notify_init() failed\n");
        exit(EXIT_FAILURE);
    }

    // resolve/load config file
    if (sconfig && load_config_file(&main, sconfig) < 0) {
        exit(EXIT_FAILURE);
    } else if (!(config = resolve_user_config(env)) || load_config_file(&main, config) < 0) {
        load_config_file(&main, SYSCONF);
    }

    // enable modules from output format option
    enable_modules(&main);

    // resolve refresh rate
    resolve_rate(&main, &rate);

    // init modules
    while (++i < NB_MODULES) {
        if (main.modules[i].enabled) {
            main.modules[i].init(&main.modules[i]);
        }
    }

    // main loop
    while (true) {

        errno = 0;
        // store reference time
        if (clock_gettime(CLOCK_REALTIME, &ref) == -1) {
            fprintf(stderr, "Call to clock_gettime() failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        // create and start a new thread for each module
        i = -1;
        while (++i < NB_MODULES) {
            if (main.modules[i].enabled) {
                if ((err = pthread_create(&main.modules[i].thread, NULL,
                    main.modules[i].routine, &main.modules[i])) != 0) {
                    fprintf(stderr, "Call to pthread_create() failed: %s\n", strerror(err));
                    exit(EXIT_FAILURE);
                }
            }
        }

        // waiting for all threads to finish
        i = -1;
        while (++i < NB_MODULES) {
            if (main.modules[i].enabled) {
                if ((err = pthread_join(main.modules[i].thread, NULL)) != 0) {
                    fprintf(stderr, "Call to pthread_join() failed: %s\n", strerror(err));
                    exit(EXIT_FAILURE);
                }
            }
        }

        // output
        set_output_buffer(&main);
        print_output_buffer(main.buffer);

        // compute tick duration
        compute_tick(&ref, &rate, &tick);

        // waiting
        if ((err = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &tick, NULL))) {
            if (err != EINTR) {
                fprintf(stderr, "Call to clock_nanosleep() failed: %s\n", strerror(err));
                exit(EXIT_FAILURE);
            }
        }
    }
    return 0;
}
