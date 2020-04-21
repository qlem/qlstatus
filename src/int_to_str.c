// By Clément Dommerc

# include "qlstatus.h"

typedef struct      s_info {
    int     length;
    int     tens;
} t_info;

char    *nbr_eq_zero() {
    char    *str = NULL;

    str = alloc_buffer(2);
    str[0] = '0';
    return str;
}

void        compute_tens(long nb, t_info *info) {
    info->length = 0;
    info->tens = 1;
    while (nb > 0) {
        nb = nb / TEN;
        info->length++;
        if (info->length > 1) {
            info->tens = info->tens * TEN;
        }
    }
}

char    *int_to_str(long nb) {
    char    *str = NULL;
    t_info  info;
    long    tmp;
    int     i;

    i = 0;
    if (nb <= 0) {
       return nbr_eq_zero(); 
    }
    compute_tens(nb, &info);
    str = alloc_buffer(info.length + 1);
    while (info.tens > 0) {
        tmp = nb / info.tens;
        tmp = tmp % TEN;
        info.tens = info.tens / TEN;
        str[i] = (char)(tmp + '0');
        i++;
    }
    return str;
}
