/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void            free_volume(void *data) {
    t_module    *module = data;
    t_pulse     *pulse = module->data;

    pa_context_disconnect(pulse->context);
    pa_threaded_mainloop_stop(pulse->mainloop);
    pa_threaded_mainloop_free(pulse->mainloop);
}

void        sink_info_cb(pa_context *context, const pa_sink_info *info,
                         int eol, void *data) {
    t_module        *module = data;
    t_pulse         *pulse = module->data;
    pa_volume_t     volume_avg;

    (void)context;
    if (eol == 0) {
        volume_avg = pa_cvolume_avg(&info->volume);
        module->value = PERCENT(volume_avg, PA_VOLUME_NORM);
        if (info->mute) {
            module->label = pulse->lb_mute;
        } else {
            module->label = pulse->label;
        }
    }
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

    pulse->mainloop = pa_threaded_mainloop_new();
    mloop_api = pa_threaded_mainloop_get_api(pulse->mainloop);
    pulse->context = pa_context_new(mloop_api, PULSE_APP_NAME);
    pa_context_set_state_callback(pulse->context, context_state_cb, pulse);
    pa_context_connect(pulse->context, NULL, PA_CONTEXT_NOFAIL |
                       PA_CONTEXT_NOAUTOSPAWN, NULL);
    pa_threaded_mainloop_start(pulse->mainloop);
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

    // TODO error handling
    pa_threaded_mainloop_lock(pulse->mainloop);
    op = pa_context_get_sink_info_by_name(pulse->context, pulse->sink,
                                          &sink_info_cb, module);
    while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(pulse->mainloop);
    }
    pa_threaded_mainloop_unlock(pulse->mainloop);
    free(op);
    return NULL;
}

void            init_volume(void *data) {
    t_module    *module = data;
    t_pulse     *pulse = module->data;
    pthread_t   thread = 0;
    int         err = 0;
    int         i = -1;

    while (++i < VOL_NOPTS) {
        if (strcmp(module->opts[i].key, OPT_VOL_LABEL) == 0) {
            pulse->label = module->opts[i].value;
        } else if (strcmp(module->opts[i].key, OPT_VOL_LB_MUTED) == 0) {
            pulse->lb_mute = module->opts[i].value;
        } else if (strcmp(module->opts[i].key, OPT_VOL_SINK) == 0) {
            pulse->sink = module->opts[i].value;
        }
    }
    if ((err = pthread_create(&thread, NULL, pulse_connect, pulse)) != 0) {
        printf("Call to pthread_create() failed: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = pthread_join(thread, NULL))) {
        printf("Call to pthread_join() failed: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
}
