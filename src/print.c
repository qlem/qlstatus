// By Clément Dommerc

#include "qlstatus.h"

void        print(char *fmt, ...) {
    char        *str = NULL;
    char        c = 0;
    va_list     ap;

    va_start(ap, fmt);
    while (*fmt) {
        switch (*fmt) {
            case 't':
                str = va_arg(ap, char *);
                write(1, str, strlen(str));
                break;
            default:
                c = (char)(*fmt);
                write(1, &c, 1);
                break;
        }
        fmt++;
    }
    va_end(ap);
}
