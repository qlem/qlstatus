/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void            free_time(void *data) {
    (void)data;
}

void            *run_time(void *data) {
    t_module    *module = data;
    t_mtime     *mtime = module->data;
    struct tm   *timeinfo;
    time_t      rawtime;

    v_memset(module->buffer, 0, BUFFER_MAX_SIZE);
    if (time(&rawtime) == ((time_t) -1)) {
        printf("Call to time() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if ((timeinfo = localtime(&rawtime)) == NULL) {
        printf("Call to localtime() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (strftime(module->buffer, BUFFER_MAX_SIZE, mtime->format,
                 timeinfo) == 0) {
        printf("Call to strftime() failed\n");
        exit(EXIT_FAILURE);
    }
    return NULL;
}

void            init_time(void *data) {
    t_module    *module = data;
    t_mtime     *time = module->data;
    int         i = -1;

    while (++i < TIME_NOPTS) {
        if (strcmp(module->opts[i].key, OPT_TIME_FORMAT) == 0) {
            time->format = module->opts[i].value;
        }
    }
}
