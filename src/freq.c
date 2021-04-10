/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void            free_cpu_freq(void *data) {
    t_module    *module = data;
    t_freq      *freq = module->data;

    free_files(freq->inputs);
}

void            khz_format_logic(t_freq *freq, long value) {
    snprintf(freq->tokens[1].buffer, TBUFFER_MAX_SIZE, "%7ld", value);
}

void            mhz_format_logic(t_freq *freq, long value) {
    snprintf(freq->tokens[1].buffer, TBUFFER_MAX_SIZE, "%4ld", value / MEGAHERTZ);
}

void            ghz_format_logic(t_freq *freq, long value) {
    snprintf(freq->tokens[1].buffer, TBUFFER_MAX_SIZE, "%3.1f",
             (float)value / (MEGAHERTZ * MEGAHERTZ));
    clean_leading_zero(freq->tokens[1].buffer);
}

void            smart_format_logic(t_freq *freq, long value) {
    if (value > (MEGAHERTZ * MEGAHERTZ) - 1) {
        snprintf(freq->tokens[1].buffer, TBUFFER_MAX_SIZE, "%3.1f",
                 (float)value / (MEGAHERTZ * MEGAHERTZ));
        clean_leading_zero(freq->tokens[1].buffer);
        set_token_buffer(freq->tokens[2].buffer, "GHz");
    } else if (value > MEGAHERTZ - 1) {
        snprintf(freq->tokens[1].buffer, TBUFFER_MAX_SIZE, "%3ld", value / MEGAHERTZ);
        set_token_buffer(freq->tokens[2].buffer, "MHz");
    } else {
        snprintf(freq->tokens[1].buffer, TBUFFER_MAX_SIZE, "%3ld", value);
        set_token_buffer(freq->tokens[2].buffer, "KHz");
    }
}

void            *run_cpu_freq(void *data) {
    t_module    *module = data;
    t_freq      *freq = module->data;
    char        *buf;
    long        sum = 0;
    int         i = -1;
    long        value;

    while (freq->inputs[++i][0]) {
        buf = read_file(freq->inputs[i]);
        sum += to_int(buf);
        free(buf);
    }

    value = sum / i;
    switch(freq->unit) {
        case KHZ:
            khz_format_logic(freq, value);
            break;
        case MHZ:
            mhz_format_logic(freq, value);
            break;
        case GHZ:
            ghz_format_logic(freq, value);
            break;
        case FSMT:
            smart_format_logic(freq, value);
    }
    set_module_buffer(module, freq->tokens, FREQ_TOKENS);
    return NULL;
}

void            init_cpu_freq(void *data) {
    t_module    *module = data;
    t_freq      *freq = module->data;
    int         i = -1;
    size_t      sdir;

    freq->tokens[0].fmtid = 'L';
    freq->tokens[1].fmtid = 'V';
    freq->tokens[2].fmtid = 'U';
    init_module_tokens(module, freq->tokens, FREQ_TOKENS);

    set_token_buffer(freq->tokens[0].buffer, module->opts[1].value);
    freq->scaling = ((int *)module->opts[3].value)[0];

    if (strcmp(module->opts[2].value, "KHz") == 0) {
        freq->unit = KHZ;
        set_token_buffer(freq->tokens[2].buffer, "KHz");
    } else if (strcmp(module->opts[2].value, "MHz") == 0) {
        freq->unit = MHZ;
        set_token_buffer(freq->tokens[2].buffer, "MHz");
    } else if (strcmp(module->opts[2].value, "GHz") == 0) {
        freq->unit = GHZ;
        set_token_buffer(freq->tokens[2].buffer, "GHz");
    } else {
        freq->unit = FSMT;
    }

    freq->inputs = read_dir(FREQ_IN_DIR, FREQ_DIR_PATTERN);
    if (!freq->inputs[0][0]) {
        fprintf(stderr, "Cannot initialize CPU frequency module, no input directory found\n");
        exit(EXIT_FAILURE);
    }

    errno = 0;
    while (freq->inputs[++i][0]) {
        sdir = v_strlen(freq->inputs[i]);
        if ((freq->inputs[i] = realloc(freq->inputs[i], sizeof(char) * (sdir + 18))) == NULL) {
            fprintf(stderr, "Call to realloc() failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        v_strsncpy(freq->inputs[i], "/", sdir, 1);
        v_strsncpy(freq->inputs[i], freq->scaling ? FREQ_SCALING : FREQ_CURRENT, sdir + 1, 16);
        freq->inputs[i][sdir + 18 - 1] = 0;
    }
}
