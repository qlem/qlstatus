/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void    free_files(char **files) {
    int     i = -1;

    while (files[++i][0]) {
        free(files[i]);
    }
    free(files[i]);
    free(files);
}

char    **add_file(char **files, size_t *size, const char *file, const char *regex) {
    if (regex && !match_pattern(regex, file)) {
        return files;
    }
    free(files[*size - 1]);
    if ((files = realloc(files, sizeof(char *) * ++(*size))) == NULL) {
        printf("Call to 'realloc()' failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    files[*size - 2] = alloc_buffer(v_strlen(file) + 1);
    v_strncpy(files[*size - 2], file, v_strlen(file));
    files[*size - 1] = alloc_buffer(1);
    return files;
}

char                **read_dir(const char *path, const char *regex) {
    char            **files = NULL;
    size_t          size = 1;
    struct dirent   *s_dir;
    DIR             *dir;

    if ((dir = opendir(path)) == NULL) {
        printf("Cannot open dir '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    files = alloc_ptr(sizeof(char *));
    files[0] = alloc_buffer(1);
    while ((s_dir = readdir(dir)) != NULL) {
        files = add_file(files, &size, s_dir->d_name, regex);
    }
    if (errno) {
        printf("Cannot read dir '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (closedir(dir) == -1) {
        printf("Cannot close dir '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return files;
}
