// By Clément Dommerc

#include "qlstatus.h"

void    free_files(char **files) {
    int     i = -1;

    while (files[++i][0]) {}
    free(files[i]);
    free(files);
}

void    close_dir(DIR *dir, const char *path) {
    int     errsv = 0;

    if (closedir(dir) == -1) {
        if (errno) {
            errsv = errno;
            printf("Cannot close dir %s: %s\n", path, strerror(errsv));
        } else {
            printf("Cannot close dir %s\n", path);
        }
        exit(EXIT_FAILURE);
    }
}

DIR     *open_dir(const char *path) {
    DIR     *dir;
    int     errsv = 0;

    if ((dir = opendir(path)) == NULL) {
        if (errsv) {
            errsv = errno;
            printf("Cannot open dir %s: %s\n", path, strerror(errsv));
        } else {
            printf("Cannot open dir %s\n", path);
        }
        exit(EXIT_FAILURE);
    }
    return dir;
}

char    **add_file(char **files, char *file, size_t *size) {
    free(files[*size - 1]);
    if ((files = realloc(files, sizeof(char *) * ++(*size))) == NULL) {
        perror("Call to 'realloc()' failed");
        exit(EXIT_FAILURE);
    }
    files[*size - 2] = file;
    files[*size - 1] = alloc_buffer(1);
    return files;
}

char    **read_dir(DIR *dir, const char *regex) {
    char            *buffer = NULL;
    char            **files;
    struct dirent   *s_dir;
    regex_t         preg;
    size_t          size = 0;
    int             errcode;

    if (regex && (errcode = regcomp(&preg, regex, REG_EXTENDED)) != 0) {
        size = regerror(errcode, &preg, buffer, size);
        buffer = alloc_buffer(size + 1); 
        regerror(errcode, &preg, buffer, size);
        printf("Could not compile regex: %s\n", buffer);
        free(buffer);
        exit(EXIT_FAILURE);
    }

    if ((files = malloc(sizeof(char *))) == NULL) {
        perror("Call to 'malloc()' failed");
        exit(EXIT_FAILURE);
    }
 
    size = 1;
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
