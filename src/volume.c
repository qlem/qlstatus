/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void        sink_info_cb(pa_context *context, const pa_sink_info *info,
                         int eol, void *data) {
    t_module        *module = data;
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
}

void            *get_volume(void *data) {
    t_module    *module = data;
    char                *sink;
    pa_mainloop         *mainloop;
    pa_mainloop_api     *mloop_api;
    pa_context          *context;
    pa_operation        *operation;

    // TODO error handling
    sink = get_option_value(module->opts, OPT_VOL_SINK, VOLUME_OPTS);
    mainloop = pa_mainloop_new();
    mloop_api = pa_mainloop_get_api(mainloop);
    context = pa_context_new(mloop_api, PULSE_APP_NAME);
    pa_context_connect(context, NULL, PA_CONTEXT_NOFAIL, NULL);
    while (true) {
        pa_mainloop_iterate(mainloop, 0, 0);
        if (pa_context_get_state(context) == PA_CONTEXT_READY) {
            break;
        }
    }
    operation = pa_context_get_sink_info_by_name(context, sink, &sink_info_cb, 
                                                 module);
    while (true) {
        pa_mainloop_iterate(mainloop, 0, 0);
        if (pa_operation_get_state(operation) != PA_OPERATION_RUNNING) {
            operation = pa_context_get_sink_info_by_name(context, sink,
                            &sink_info_cb, module);
        }
        v_sleep(0, PULSE_RATE);
    }
    pa_context_disconnect(context);
    pa_mainloop_free(mainloop);
    return NULL;
}
