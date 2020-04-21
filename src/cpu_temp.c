// By Clément Dommerc

#include "qlstatus.h"

long    compute_temp(char **files) {
    char        *path;
    char        *buffer;
    long        temp = 0;
    long        sum = 0;
    long        rem = 0;
    int         i = -1;

    while (files[++i][0]) {
        path = alloc_buffer(strlen(CPU_TEMP_DIR) + strlen(files[i]) + 2);
        sprintf(path, "%s/%s", CPU_TEMP_DIR, files[i]);
        buffer = read_file(path);
        sum += strtol(buffer, NULL, BASE);
        free(path);
        free(buffer);
    }
    temp = sum / (++i);
    rem = temp % THOUSAND;
    temp = temp / THOUSAND;
    if (rem >= MAGIC) {
       temp += 1;
    }
    free_files(files, i);
    return temp;
}

char    *get_cpu_temp() {
    DIR         *dir;
    char        **files;
    char        *token;
    char        *buffer;
    long        temp = 0;

    dir = open_dir(CPU_TEMP_DIR);
    files = read_dir(dir, CPU_TEMP_INPUT_PATTERN);
    temp = compute_temp(files);
    buffer = int_to_str(temp);
    token = alloc_buffer(sizeof(char ) * (strlen(buffer) +
                strlen(CPU_TEMP_LABEL) + 4));
    sprintf(token, "%s %s°", CPU_TEMP_LABEL, buffer);
    free(buffer);
    close_dir(dir);
    return token;
}
