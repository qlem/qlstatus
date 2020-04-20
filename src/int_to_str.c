// By Clément Dommerc

# include "qlstatus.h"

#define TEN 10

char    *nbr_eq_zero() {
    char    *str = NULL;

    str = alloc_buffer(2);
    str[0] = '0';
    return str;
}

int     get_length(long nb) {
    int     length = 0;

    while (nb > 0) {
        nb = nb / TEN;
        length++;
    }
    return length;
}

long    get_tens(int length) {
    long    tens = 1;

    while (length > 1) {
        tens = tens * TEN;
        length--;
    }
    return tens;
}

char    *to_str(long nb, int length, long tens) {
    char    *str = NULL;
    long    tmp;
    int     i;

    i = 0;
    str = alloc_buffer(length + 1);
    while (tens > 0) {
        tmp = nb / tens;
        tmp = tmp % TEN;
        tens = tens / TEN;
        str[i] = (char)(tmp + '0');
        i++;
    }
    return str;
}

char    *int_to_str(long nb) {
    int     length;
    long    tens;

    if (nb == 0) {
       return nbr_eq_zero(); 
    }
    length = get_length(nb);
    tens = get_tens(length);
    return to_str(nb, length, tens);
}
