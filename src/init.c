/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#include "qlstatus.h"

void        init_volume(t_module *module) {
    module->enabled = 1;
    module->fmtid = 'V';
    module->label = VOLUME_LABEL;
    module->value = 0;
    module->unit = "%";
    module->routine = get_volume;
    module->is_thread = 1;
}

void        init_wireless(t_module *module) {
    module->enabled = 1;
    module->fmtid = 'W';
    module->label = WIRELESS_UNK_ESSID_LABEL;
    module->value = 0;
    module->unit = "%";
    module->routine = get_wireless;
    module->is_thread = 0;
}

void        init_memory(t_module *module) {
    module->enabled = 1;
    module->fmtid = 'M';
    module->label = MEM_LABEL;
    module->value = 0;
    module->unit = "%";
    module->routine = get_memory;
    module->is_thread = 0;
}

void        init_cpu_temp(t_module *module) {
    module->enabled = 1;
    module->fmtid = 'T';
    module->label = CPU_TEMP_LABEL;
    module->value = 0;
    module->unit = "°";
    module->routine = get_cpu_temp;
    module->is_thread = 0;
}

void        init_cpu_usage(t_module *module) {
    t_cpu   *cpu = NULL;

    cpu = alloc_ptr(sizeof(t_cpu));
    cpu->prev_idle = 0;
    cpu->prev_total = 0;

    module->enabled = 1;
    module->fmtid = 'U';
    module->label = CPU_USAGE_LABEL;
    module->value = 0;
    module->unit = "%";
    module->args = cpu;
    module->routine = get_cpu_usage;
    module->is_thread = 0;
}

void        init_battery(t_module *module) {
    module->enabled = 1;
    module->fmtid = 'B';
    module->label = BAT_LABEL_UNKNOW;
    module->value = 0;
    module->unit = "%";
    module->routine = get_battery;
    module->is_thread = 0;
}

void        init_brightness(t_module *module) {
    module->enabled = 1;
    module->fmtid = 'L';
    module->label = BRIGHTNESS_LABEL;
    module->value = 0;
    module->unit = "%";
    module->routine = get_brightness;
    module->is_thread = 0;
}
