// By Clément Dommerc

#include <stdlib.h>

void    v_memset(char *buffer, size_t size, char c) {
    size_t  i = -1;

    while (++i < size) {
        buffer[i] = c;
    }
}

char    *alloc_buffer(size_t size) {
    char    *buffer;

    if ((buffer = malloc(sizeof(char) * size)) == NULL) {
        exit(1);
    }
    v_memset(buffer, size, 0);
    return buffer;
}
