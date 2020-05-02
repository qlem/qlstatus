/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void        sink_info_cb(pa_context *context, const pa_sink_info *info, int eol, void *data) {
    t_module        *module = data;
    pa_volume_t     volume_avg;
    long            volume;
    int             muted;

    (void)context;
    if (eol == 0) {
        muted = pa_cvolume_is_muted(&info->volume);
        volume_avg = pa_cvolume_avg(&info->volume);
        volume = PERCENT(volume_avg, PA_VOLUME_NORM);
        module->value = volume;
        if (muted) {
            module->label = VOLUME_MUTED_LABEL;
        } else {
            module->label = VOLUME_LABEL;
        }
    }
}

void        context_succes_cb(pa_context *context, int success, void *data) {
    (void)context;
    (void)data;
    printf("success: %d\n", success);
}

void        context_subscribe_cb(pa_context *context, pa_subscription_event_type_t type, uint32_t idx, void *data) {
    (void)data;
    if ((type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SINK) {
        if ((type & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_CHANGE) {
            printf("sink updated: %d\n", idx);
            pa_context_get_sink_info_by_index(context, idx, &sink_info_cb, NULL);
        }
    }
}

void        context_state_cb(pa_context *context, void *data) {
    pa_context_state_t  state;

    (void)data;
    state = pa_context_get_state(context);
    switch (state) {
        case PA_CONTEXT_UNCONNECTED:
            printf("unconnected\n");
            break;
        case PA_CONTEXT_CONNECTING:
            printf("connecting\n");
            break;
        case PA_CONTEXT_AUTHORIZING:
            printf("auth\n");
            break;
        case PA_CONTEXT_SETTING_NAME:
            printf("setting name\n");
            break;
        case PA_CONTEXT_READY:
            printf("ready\n");
            break;
        case PA_CONTEXT_FAILED:
            printf("failed\n");
            break;
        case PA_CONTEXT_TERMINATED:
            printf("terminated\n");
            break;
    }
}

void            *get_volume(void *data) {
    t_module    *module = data;

    pa_mainloop         *mainloop;
    pa_mainloop_api     *mloop_api;
    pa_proplist         *proplist;
    pa_context          *context;
    pa_context_state_t  state;
    pa_operation        *operation;

    mainloop = pa_mainloop_new();
    mloop_api = pa_mainloop_get_api(mainloop);
    proplist = pa_proplist_new();
    pa_proplist_set(proplist, PA_PROP_APPLICATION_NAME, PA_APP_NAME, PA_APP_NAME_LEN);
    context = pa_context_new_with_proplist(mloop_api, PA_APP_NAME, proplist);

    /* pa_context_set_state_callback(context, &context_state_cb, NULL); */

    pa_context_connect(context, NULL, PA_CONTEXT_NOFAIL, NULL);

    while (true) {
        pa_mainloop_iterate(mainloop, 0, 0);
        state = pa_context_get_state(context);
        if (state == PA_CONTEXT_READY) {
            break;
        }
    }

    /* pa_context_set_subscribe_callback(context, &context_subscribe_cb, NULL);
     * pa_context_subscribe(context, PA_SUBSCRIPTION_MASK_SINK, &context_succes_cb, NULL); */

    operation = pa_context_get_sink_info_by_index(context, 0, &sink_info_cb, module);

    while (true) {
        pa_mainloop_iterate(mainloop, 0, 0);
        if (pa_operation_get_state(operation) != PA_OPERATION_RUNNING) {
            operation = pa_context_get_sink_info_by_index(context, 0, &sink_info_cb, module);
        }
        v_sleep(0, (long)PA_RATE);
    }

    pa_context_disconnect(context);
    pa_proplist_free(proplist);
    pa_mainloop_free(mainloop);

    return NULL;
}
