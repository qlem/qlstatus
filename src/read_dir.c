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

void    close_dir(DIR *dir, const char *path) {
    if (closedir(dir) == -1) {
        printf("Cannot close dir '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

DIR     *open_dir(const char *path) {
    DIR     *dir;

    if ((dir = opendir(path)) == NULL) {
        printf("Cannot open dir '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return dir;
}

char    **add_file(char **files, const char *file, size_t *size) {
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

void    compile_regex(regex_t *preg, const char *regex, int flags) {
    char            *buffer = NULL;
    size_t          size = 0;
    int             errcode;

    if (regex && (errcode = regcomp(preg, regex, flags)) != 0) {
        size = regerror(errcode, preg, buffer, size);
        buffer = alloc_buffer(size + 1);
        regerror(errcode, preg, buffer, size);
        printf("Could not compile regex: %s\n", buffer);
        free(buffer);
        exit(EXIT_FAILURE);
    }
}

char    **read_dir(const char *path, const char *regex) {
    struct dirent   *s_dir;
    DIR             *dir;
    char            **files;
    regex_t         preg;
    size_t          size;

    if (regex) {
        compile_regex(&preg, regex, REG_EXTENDED);
    }
    files = alloc_ptr(sizeof(char *));
    files[0] = alloc_buffer(1);
    size = 1;
    dir = open_dir(path);
    while ((s_dir = readdir(dir)) != NULL) {
        if (regex) {
            if (regexec(&preg, s_dir->d_name, 0, NULL, 0) == 0) {
                files = add_file(files, s_dir->d_name, &size);
            }
        } else {
            files = add_file(files, s_dir->d_name, &size);
        }
    }
    if (errno) {
        printf("Cannot read dir '%s': %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    close_dir(dir, path);
    if (regex) {
        regfree(&preg);
    }
    return files;
}
