// By Clément Dommerc

#include "qlstatus.h"

char    *get_brightness() {
    char    *token = NULL;
    char    *buffer = NULL;
    long    current = 0;
    long    max = 0;
    long    value = 0;

    buffer = read_file(BRIGHTNESS_CURRENT);
    current = to_int(buffer);
    free(buffer);
    buffer = read_file(BRIGHTNESS_MAX);
    max = to_int(buffer);
    free(buffer);
    value = PERCENT(current, max);
    buffer = to_str(value);
    token = alloc_buffer(v_strlen(buffer) + v_strlen(BRIGHTNESS_LABEL) + 3);
    sprintf(token, "%s %s%%", BRIGHTNESS_LABEL, buffer);
    free(buffer);
    return token;
}
