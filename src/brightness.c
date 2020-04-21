// By Clément Dommerc

#include "qlstatus.h"

char    *get_brightness() {
    char    *token = NULL;
    char    *buffer = NULL;
    long    current = 0;
    long    max = 0;
    long    value = 0;

    buffer = read_file(BRIGHTNESS_CURRENT);
    current = strtol(buffer, NULL, BASE);
    free(buffer);
    buffer = read_file(BRIGHTNESS_MAX);
    max = strtol(buffer, NULL, BASE);
    free(buffer);
    value = current * CENT / max;
    buffer = int_to_str(value);
    token = alloc_buffer(sizeof(char) * strlen(buffer) 
            + strlen(BRIGHTNESS_LABEL) + 3);
    sprintf(token, "%s %s%%", BRIGHTNESS_LABEL, buffer);
    free(buffer);
    return token;
}
