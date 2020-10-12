/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

int     notify(NotifyNotification *notify, const char *summary,
               const char *body, const char *icon, NotifyUrgency urgency) {
    if (!notify_notification_update(notify, summary, body, icon)) {
        fprintf(stderr, "Call to notify_notification_update() failed\n");
        notify_uninit();
        return -1;
    }
    notify_notification_set_urgency(notify, urgency);
    if (!notify_notification_show(notify, NULL)) {
        fprintf(stderr, "Call to notify_notification_show() failed\n");
        notify_uninit();
        return -1;
    }
    return 0;
}

NotifyNotification      *notify_new(const char *summary) {
    NotifyNotification  *notify = NULL;

    if (!(notify = notify_notification_new(summary, NULL, NULL))) {
        fprintf(stderr, "Call to notify_notification_new() failed\n");
        notify_uninit();
        return NULL;
    }
    return notify;
}
