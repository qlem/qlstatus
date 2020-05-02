/*
 * Copyright (c) 2020 Clément Dommerc <clement.dommerc@gmail.com>
 * MIT License
 *
 */

#ifndef QLSTATUS_H_
#define QLSTATUS_H_

#include <stdio.h>
#include <pthread.h>
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

#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/nl80211.h>
#include <linux/if_ether.h>

#include <pulse/mainloop.h>
#include <pulse/context.h>
#include <pulse/proplist.h>
#include <pulse/subscribe.h>
#include <pulse/introspect.h>
#include <pulse/volume.h>

/* GLOBAL */
#define BASE 10
#define RATE 1E9
#define NB_MODULES 7
#define SEC(nsec) nsec / (long)1E9
#define NSEC(nsec) nsec % (long)1E9
#define PERCENT(value, total) value * 100 / total

/* MODULE STRUCTURE */
typedef struct      s_module {
    uint8_t         enabled;
    char            fmtid;
    char            *label;
    long            value;
    char            *unit;
    void            *args;
    void            *(*routine) (void *);
    uint8_t         is_thread;
    pthread_t       thread;
}                   t_module;

/* GLOBAL STRUCTURE */
typedef struct      s_main {
    t_module        modules[NB_MODULES];
    char            *format;
}                   t_main;

/* OUTPUT FORMAT
 * %U: cpu usage
 * %T: cpu temp
 * %M: memory usage
 * %L: brightness level
 * %V: current volume
 * %B: remaining battery
 * %W: wireless info
 */
#define DEFAULT_FORMAT "%U  %T  %M  %L  %V  %B  %W"

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

/* BRIGHTNESS */
#define BRIGHTNESS_CURRENT "/sys/class/backlight/intel_backlight/actual_brightness"
#define BRIGHTNESS_MAX "/sys/class/backlight/intel_backlight/max_brightness"
#define BRIGHTNESS_LABEL "brg"

/* CPU USAGE */
#define PROC_STAT "/proc/stat"
#define CPU_STATS_PATTERN "^cpu[ \t]+(([0-9]+ ){9}[0-9]+)$"
#define CPU_USAGE_LABEL "cpu"
#define CPU_STATS_SIZE 8

typedef struct      s_cpu {
    long            prev_idle;
    long            prev_total;
}                   t_cpu;

/* CPU TEMP */
#define CPU_TEMP_DIR "/sys/devices/platform/coretemp.0/hwmon/*"
#define CPU_TEMP_INPUT_PATTERN "^temp[2-5]_input$"
#define CPU_TEMP_LABEL "cpu"
#define CPU_TEMP_ROUND_THRESHOLD 500

/* WIRELESS */
#define NL80211 "nl80211"
#define WLAN_EID_SSID 0
#define WIRELESS_INTERFACE "wlp2s0"
#define WIRELESS_FLAG_HAS_ESSID (1 << 0)
#define WIRELESS_FLAG_HAS_SIGNAL (1 << 1)
#define WIRELESS_ESSID_MAX_SIZE 16
#define WIRELESS_UNK_ESSID_LABEL "SSID unk:"
#define WIRELESS_UNK_QUALITY_LABEL "-"
#define WIRELESS_PREFIX_ERROR "Wireless module error"
#define NOISE_FLOOR_DBM (-90)
#define SIGNAL_MAX_DBM (-20)

typedef struct      s_wireless {
    unsigned int    flags;
    unsigned int    if_index;
    int             nl80211_id;
    uint8_t         bssid[ETH_ALEN];
    char            *essid;
    int             signal;
}                   t_wireless;

/* MEMORY */
#define PROC_MEMINFO "/proc/meminfo"
#define MEM_TOTAL_PATTERN "^MemTotal:[ \t]+([0-9]+) kB$"
#define MEM_FREE_PATTERN "^MemFree:[ \t]+([0-9]+) kB$"
#define MEM_BUFFERS_PATTERN "^Buffers:[ \t]+([0-9]+) kB$"
#define MEM_CACHED_PATTERN "^Cached:[ \t]+([0-9]+) kB$"
#define MEM_SRECLAIM_PATTERN "^SReclaimable:[ \t]+([0-9]+) kB$"
#define MEM_LABEL "mem"

typedef struct  s_meminfo {
    long        total;
    long        free;
    long        buffers;
    long        cached;
    long        sreclaim;
}               t_meminfo;

/* VOLUME */
#define PA_APP_NAME "qlstatus"
#define PA_APP_NAME_LEN 8
#define PA_RATE 1E8
#define VOLUME_LABEL "vol"
#define VOLUME_MUTED_LABEL "muted"

/* FUNCTIONS */
size_t  v_strlen(const char *str);
char    *v_strncpy(char *dest, const char *src, size_t n);
void    v_memset(void *ptr, uint8_t c, size_t size);
void    v_sleep(time_t sec, long nsec);
long    to_int(const char *str);
char	*to_str(long nb);
int     putstr(const char *str);

// format
char    *format(t_main *main);

// regex
bool    match_pattern(const char *regex, const char *str);
char    *substring(const char *regex, const char *str);

// alloc memory
char    *alloc_buffer(size_t size);
void    *alloc_ptr(size_t size);

// read files / dirs
void    close_stream(FILE *stream, const char *file);
FILE    *open_stream(const char *file);
void    free_files(char **files);
char    **read_dir(const char *path, const char *regex);
char    *read_file(const char *file);

// modules
void    *get_battery(void *data);
void    *get_volume(void *data);
void    *get_brightness(void *data);
void    *get_cpu_usage(void *data);
void    *get_cpu_temp(void *data);
void    *get_wireless(void *data);
void    *get_memory(void *data);

#endif /* !QLSTATUS_H_ */
