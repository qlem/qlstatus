// By Clément Dommerc

#ifndef QLSTATUS_H_
#define QLSTATUS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>
#include <regex.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>

#define BASE 10
#define TEN 10
#define CENT 100
#define THOUSAND 1000

/* BATTERY */
#define BAT_CURRENT "/sys/class/power_supply/BAT0/energy_now"
#define BAT_MAX "/sys/class/power_supply/BAT0/energy_full_design"
#define BAT_STATUS "/sys/class/power_supply/BAT0/status"
#define BAT_STATUS_FULL "Full\n"
#define BAT_STATUS_CHARGING "Charging\n"
#define BAT_STATUS_DISCHARGING "Discharging\n"
#define BAT_LABEL_FULL "full"
#define BAT_LABEL_CHARGING "chr"
#define BAT_LABEL_DISCHARGING "bat"
#define BAT_LABEL_UNKNOW "unk"

/* VOLUME */
// TODO

/* BRIGHTNESS */
#define BRIGHTNESS_CURRENT "/sys/class/backlight/intel_backlight/actual_brightness"
#define BRIGHTNESS_MAX "/sys/class/backlight/intel_backlight/max_brightness"
#define BRIGHTNESS_LABEL "brg"

/* CPU USAGE */
// TODO

/* CPU TEMP */
#define CPU_TEMP_DIR "/sys/devices/platform/coretemp.0/hwmon/*"
#define CPU_TEMP_INPUT_PATTERN "^temp[2-5]_input$"
#define CPU_TEMP_LABEL "cpu"
#define CPU_TEMP_ROUND 500

/* FUNCTIONS */
size_t  v_strlen(const char *str);
char    *v_strncpy(char *dest, const char *src, size_t n);
long    to_int(const char *str);
char	*to_str(long nb);
int     putstr(const char *str);
void    print(char *fmt, ...);

// alloc a buffer
void    v_memset(char *buffer, size_t size, char c);
char    *alloc_buffer(size_t size);

// read files / dirs
void    free_files(char **files);
void    close_dir(DIR *dir, const char *path);
DIR     *open_dir(const char *path);
char    **read_dir(DIR *dir, const char *regex);
char    *read_file(const char *path);

// modules
char    *get_battery();
char    *get_volume();
char    *get_brightness();
char    *get_cpu_usage();
char    *get_cpu_temp();

#endif /* !QLSTATUS_H_ */
