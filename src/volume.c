/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

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
            module->label = get_option_value(module->opts, OPT_VOL_LB_MUTED,
                                             VOLUME_OPTS);
        } else {
            module->label = get_option_value(module->opts, OPT_VOL_LABEL,
                                             VOLUME_OPTS);
        }
    }
    pa_threaded_mainloop_signal(pulse->mainloop, 0);
}

void    context_state_cb(pa_context *context, void *data) {
    t_module            *module = data;
    t_pulse             *pulse = module->data;
    pa_context_state_t  state;

    state = pa_context_get_state(context);
    switch (state) {
        case PA_CONTEXT_READY:
            pulse->connected = 1;
            pa_threaded_mainloop_signal(pulse->mainloop, 0);
            break;
        default:
            break;
    }
}

void    pulse_connect(t_module *module) {
    t_pulse             *pulse = module->data;
    pa_mainloop_api     *mloop_api;

    pulse->mainloop = pa_threaded_mainloop_new();
    mloop_api = pa_threaded_mainloop_get_api(pulse->mainloop);
    pulse->context = pa_context_new(mloop_api, PULSE_APP_NAME);
    pa_context_set_state_callback(pulse->context, context_state_cb, module);
    pa_context_connect(pulse->context, NULL, PA_CONTEXT_NOFAIL |
            PA_CONTEXT_NOAUTOSPAWN, NULL);
    pa_threaded_mainloop_start(pulse->mainloop);
    pa_threaded_mainloop_wait(pulse->mainloop);
}

void        *get_volume(void *data) {
    t_module            *module = data;
    t_pulse             *pulse = module->data;
    char                *sink;
    pa_operation        *op;

    // TODO error handling
    if (!pulse->connected) {
        pulse_connect(module);
        pthread_exit(NULL);
    }
    sink = get_option_value(module->opts, OPT_VOL_SINK, VOLUME_OPTS);
    pa_threaded_mainloop_lock(pulse->mainloop);
    op = pa_context_get_sink_info_by_name(pulse->context, sink, &sink_info_cb,
                                     module);
    while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(pulse->mainloop);
    }
    pa_threaded_mainloop_unlock(pulse->mainloop);

    /* pa_threaded_mainloop_stop(pulse->mainloop);
    pa_threaded_mainloop_free(pulse->mainloop); */
    pthread_exit(NULL);
    return NULL;
}
