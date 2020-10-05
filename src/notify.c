/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

int         notify(const char *summary, const char *body, const char *icon,
                   NotifyUrgency urgency) {
    NotifyNotification  *notify = NULL;
    if (!notify_init("qlstatus")) {
        printf("Call to notify_init() failed\n");
        return 1;
    }
    if (!(notify = notify_notification_new(summary, body, icon))) {
        printf("Call to notify_notification_new() failed\n");
        notify_uninit();
        return 1;
    }
    notify_notification_set_urgency(notify, urgency);
    if (!notify_notification_show(notify, NULL)) {
        printf("Call to notify_notification_show() failed\n");
        g_object_unref(G_OBJECT(notify));
        notify_uninit();
        return 1;
    }
    g_object_unref(G_OBJECT(notify));
    notify_uninit();
    return 0;
}
