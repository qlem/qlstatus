// By Clément Dommerc

#include "qlstatus.h"

void    free_files(char **files) {
    int     i = -1;

    while (files[++i][0]) {}
    free(files[i]);
    free(files);
}

DIR     *open_dir(const char *path) {
    DIR     *dir;
    if ((dir = opendir(path)) == NULL) {
        exit(1);
    }
    return dir;
}

void    close_dir(DIR *dir) {
    if (closedir(dir) == -1) {
        exit(1);
    }
}

char    **add_file(char **files, char *file, int *size) {
    free(files[*size - 1]);
    if ((files = realloc(files, sizeof(char *) * ++(*size))) == NULL) {
        exit(1);
    }
    files[*size - 2] = file;
    files[*size - 1] = alloc_buffer(1);
    return files;
}

char    **read_dir(DIR *dir, const char *regex) {
    char            **files;
    struct dirent   *s_dir;
    regex_t         preg;
    int             size = 1;

    if (regex && regcomp(&preg, regex, REG_EXTENDED) != 0) {
        exit(1);
    }
    if ((files = malloc(sizeof(char *))) == NULL) {
        exit(1);
    }
    files[0] = alloc_buffer(1);
    while ((s_dir = readdir(dir)) != NULL) {
        if (regex) {
            if (regexec(&preg, s_dir->d_name, 0, NULL, 0) == 0) {
                files = add_file(files, s_dir->d_name, &size);
            }
        } else {
            files = add_file(files, s_dir->d_name, &size);
        }
    }
    if (regex) {
        regfree(&preg);
    }
    return files;
}
