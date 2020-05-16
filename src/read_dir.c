/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void        free_files(char **files) {
    int     i = -1;

    while (files[++i][0]) {
        free(files[i]);
    }
    free(files[i]);
    free(files);
}

char        **add_file(char **files, size_t *size, const char *file,
                       const char *regex, const char *dir) {
    size_t  sfile;
    size_t  sdir;

    if (strcmp(file, ".") == 0 || strcmp(file, "..") == 0 ||
        (regex && !match_pattern(regex, file))) {
        return files;
    }
    free(files[*size - 1]);
    if ((files = realloc(files, sizeof(char *) * ++(*size))) == NULL) {
        printf("Call to realloc() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    sfile = v_strlen(file);
    sdir = v_strlen(dir);
    if (dir[sdir - 1] == '/') {
        files[*size - 2] = alloc_buffer(sdir + sfile + 1);
        v_strncpy(files[*size - 2], dir, sdir);
        v_strsncpy(files[*size - 2], file, sdir, sfile);
    } else {
        files[*size - 2] = alloc_buffer(sfile + sdir + 2);
        sprintf(files[*size - 2], "%s/%s", dir, file);
    }
    files[*size - 1] = alloc_buffer(1);
    return files;
}

char                **read_dir(const char *path, const char *regex) {
    char            **files = NULL;
    size_t          size = 1;
    struct dirent   *s_dir;
    DIR             *dir;

    if ((dir = opendir(path)) == NULL) {
        printf("Cannot open dir %s: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    files = alloc_ptr(sizeof(char *));
    files[0] = alloc_buffer(1);
    while ((s_dir = readdir(dir)) != NULL) {
        files = add_file(files, &size, s_dir->d_name, regex, path);
    }
    if (errno) {
        printf("Error readding dir %s: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (closedir(dir) == -1) {
        printf("Cannot close dir %s: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return files;
}
