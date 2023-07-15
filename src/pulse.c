/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#define _GNU_SOURCE
#include "qlstatus.h"

// Based from sources of pa_volume_snprint() function
#define VOLUME(avg) (((uint64_t)avg * 100 + (uint64_t)PA_VOLUME_NORM / 2) / (uint64_t)PA_VOLUME_NORM)

void            free_volume(void *data) {
    t_module    *module = data;
    t_pulse     *pulse = module->data;

    pa_context_disconnect(pulse->context);
    pa_context_unref(pulse->context);
    pa_threaded_mainloop_stop(pulse->mainloop);
    pa_threaded_mainloop_free(pulse->mainloop);
}

void        sink_info_cb(pa_context *context, const pa_sink_info *info, int eol, void *data) {
    t_module        *module = data;
    t_pulse         *pulse = module->data;
    pa_volume_t     avg;

    (void)context;
    if (eol < 0) {
        set_token_buffer(pulse->tokens[0].buffer, pulse->label);
        set_token_buffer(pulse->tokens[1].buffer, "--%");
    }
    if (eol == 0) {
        avg = pa_cvolume_avg(&info->volume);
        set_token_buffer(pulse->tokens[0].buffer, info->mute ? pulse->lb_mute : pulse->label);
        snprintf(pulse->tokens[1].buffer, TBUFFER_MAX_SIZE, "%2ld%%", VOLUME(avg));
    }
    set_module_buffer(module, pulse->tokens, VOLUME_TOKENS);
    pa_threaded_mainloop_signal(pulse->mainloop, 0);
}

void            context_state_cb(pa_context *context, void *data) {
    t_pulse     *pulse = data;

    if (pa_context_get_state(context) == PA_CONTEXT_READY) {
        pulse->connected = 1;
        pa_threaded_mainloop_signal(pulse->mainloop, 0);
    }
}

void                    *pulse_connect(void *data) {
    t_pulse             *pulse = data;
    pa_mainloop_api     *mloop_api;

    if ((pulse->mainloop = pa_threaded_mainloop_new()) == NULL) {
        fprintf(stderr, "Call to pa_threaded_mainloop_new() failed\n");
        exit(EXIT_FAILURE);
    }
    if ((mloop_api = pa_threaded_mainloop_get_api(pulse->mainloop)) == NULL) {
        fprintf(stderr, "Call to pa_threaded_mainloop_get_api() failed\n");
        exit(EXIT_FAILURE);
    }
    if ((pulse->context = pa_context_new(mloop_api, PULSE_APP_NAME)) == NULL) {
        fprintf(stderr, "Call to pa_context_new() failed\n");
        exit(EXIT_FAILURE);
    }
    pa_context_set_state_callback(pulse->context, context_state_cb, pulse);
    if (pa_context_connect(pulse->context, NULL, PA_CONTEXT_NOFAIL | PA_CONTEXT_NOAUTOSPAWN,
        NULL) < 0) {
        fprintf(stderr, "Call to pa_context_connect() failed\n");
        exit(EXIT_FAILURE);
    }
    if (pa_threaded_mainloop_start(pulse->mainloop) < 0) {
        fprintf(stderr, "Call to pa_threaded_mainloop_start() failed\n");
        exit(EXIT_FAILURE);
    }
    while (pulse->connected == 0) {
        pa_threaded_mainloop_lock(pulse->mainloop);
        pa_threaded_mainloop_wait(pulse->mainloop);
        pa_threaded_mainloop_unlock(pulse->mainloop);
    }
    return NULL;
}

void                    *run_volume(void *data) {
    t_module            *module = data;
    t_pulse             *pulse = module->data;
    pa_operation        *op = NULL;

    pa_threaded_mainloop_lock(pulse->mainloop);
    op = pa_context_get_sink_info_by_name(pulse->context, pulse->sink, &sink_info_cb, module);
    if (op == NULL) {
        fprintf(stderr, "Call to pa_context_get_sink_info_by_name() failed\n");
        exit(EXIT_FAILURE);
    }
    while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(pulse->mainloop);
    }
    pa_operation_unref(op);
    pa_threaded_mainloop_unlock(pulse->mainloop);
    return NULL;
}

void                    init_volume(void *data) {
    t_module            *module = data;
    t_pulse             *pulse = module->data;
    struct timespec     abstime;
    pthread_t           thread = 0;
    int                 err;

    pulse->tokens[0].fmtid = 'L';
    pulse->tokens[1].fmtid = 'V';
    init_module_tokens(module, pulse->tokens, VOLUME_TOKENS);

    pulse->label = module->opts[1].value;
    pulse->lb_mute = module->opts[2].value;
    pulse->sink = module->opts[3].value;

    if ((err = pthread_create(&thread, NULL, pulse_connect, pulse)) != 0) {
        fprintf(stderr, "Call to pthread_create() failed: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }

    errno = 0;
    if (clock_gettime(CLOCK_REALTIME, &abstime) == -1) {
        fprintf(stderr, "Call to clock_gettime() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (abstime.tv_nsec + PULSE_CONNECTION_TIMEOUT >= NSEC) {
        abstime.tv_sec += 1;
        abstime.tv_nsec = (abstime.tv_nsec + PULSE_CONNECTION_TIMEOUT) - NSEC;
    } else {
        abstime.tv_nsec += PULSE_CONNECTION_TIMEOUT;
    }

    if ((err = pthread_timedjoin_np(thread, NULL, &abstime)) != 0) {
        if (err == ETIMEDOUT) {
            fprintf(stderr, "Failed to initialize volume module: %s\n", strerror(err));
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "Call to pthread_join() failed: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
}
