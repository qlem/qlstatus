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
#include <dirent.h>
#include <regex.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>

#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/nl80211.h>
#include <linux/if_ether.h>

#include <pulse/thread-mainloop.h>
#include <pulse/context.h>
#include <pulse/introspect.h>
#include <pulse/volume.h>

/* GLOBAL */
#define BASE 10
#define RATE "1s"
#define NB_MODULES 7
#define NSEC 999999999
#define NSEC_TO_SEC(nsec) nsec / (long)1e9
#define REM_NSEC(nsec) nsec % (long)1e9
#define PERCENT(value, total) value * 100 / total
#define CONFIG_FILE ".config/qlstatus/qlstatus.conf"
#define HOME_PATTERN "^HOME=(/home/[a-zA-Z]+)$"

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

/* OPTIONS */
typedef enum        opt_type {
    OPT_STATE,
    OPT_LABEL,
    OPT_OTHER
}                   opt_type;

typedef struct      s_opt {
    char            *key;
    char            *value;
    char            *p_value;
    opt_type        type;
    uint8_t         to_free;
}                   t_opt;

// number of options per module
#define GLOBAL_OPTS 2
#define BATTERY_OPTS 7
#define CPU_OPTS 3
#define TEMP_OPTS 5
#define MEM_OPTS 3
#define BRIGHTNESS_OPTS 3
#define VOLUME_OPTS 4
#define WIRELESS_OPTS 3

// option patterns
#define OPT_TEXT_PATTERN "^.{1,100}$"
#define OPT_NUMBER_PATTERN "^[0-9]{1,4}$"
#define OPT_BOOLEAN_PATTERN "^0$|^1$"
#define OPT_PATH_PATTERN "^\\/.+([^/*]|\\/\\*)$"
#define OPT_LABEL_PATTERN "^.{1,5}$"
#define OPT_WL_LABEL_PATTERN "^.{1,15}:?$"
#define OPT_FORMAT_PATTERN "^.+$"
#define OPT_RATE_PATTERN "^[0-9]+s$|^[0-9]+ms$"
#define OPT_BAT_NAME_PATTERN "^BAT[0-9]$"
#define OPT_IN_TEMP_PATTERN "^([1-9])$|^([1-9]-[1-9])$"

// global options
#define OPT_FORMAT "format"
#define OPT_RATE "rate"

// battery options
#define OPT_BAT_ENABLED "battery_enabled"
#define OPT_BAT_NAME "battery_name"
#define OPT_BAT_LB_FULL "battery_label_full"
#define OPT_BAT_LB_CHR "battery_label_charging"
#define OPT_BAT_LB_DIS "battery_label_discharging"
#define OPT_BAT_LB_UNK "battery_label_unknown"
#define OPT_BAT_CRITIC "battery_critical"

// usage cpu options
#define OPT_CPU_ENABLED "cpu_usage_enabled"
#define OPT_CPU_LABEL "cpu_usage_label"
#define OPT_CPU_CRITIC "cpu_usage_critical"

// temperature
#define OPT_TEMP_ENABLED "temperature_enabled"
#define OPT_TEMP_LABEL "temperature_label"
#define OPT_TEMP_DIR "temperature_dir"
#define OPT_TEMP_INPUT "temperature_input"
#define OPT_TEMP_CRITIC "temperature_critical"

// memory options
#define OPT_MEM_ENABLED "memory_enabled"
#define OPT_MEM_LABEL "memory_label"
#define OPT_MEM_CRITIC "memory_critical"

// brightness options
#define OPT_BRG_ENABLED "brightness_enabled"
#define OPT_BRG_LABEL "brightness_label"
#define OPT_BRG_DIR "brightness_dir"

// volume options
#define OPT_VOL_ENABLED "volume_enabled"
#define OPT_VOL_LABEL "volume_label"
#define OPT_VOL_LB_MUTED "volume_muted_label"
#define OPT_VOL_SINK "volume_sink_name"

// wireless options
#define OPT_WLAN_ENABLED "wireless_enabled"
#define OPT_WLAN_LB_UNK "wireless_unknown_label"
#define OPT_WLAN_IFACE "wireless_interface"

/* MODULES */
typedef struct      s_module {
    uint8_t         enabled;
    char            fmtid;
    char            *label;
    long            value;
    char            *unit;
    void            *data;
    t_opt           *opts;
    int             s_opts;
    void            *(*routine)(void *);
    void            (*mfree)(void *);
    pthread_t       thread;
}                   t_module;

// battery
#define BATTERY_NAME "BAT0"
#define POWER_DIR "/sys/class/power_supply"
#define POWER_FILE "uevent"
#define PW_STATUS_PATTERN "^POWER_SUPPLY_STATUS=(Discharging|Charging|Full|Unknown)$"
#define PW_MAX_PATTERN "^POWER_SUPPLY_ENERGY_FULL_DESIGN=([0-9]+)$"
#define PW_CURRENT_PATTERN "^POWER_SUPPLY_ENERGY_NOW=([0-9]+)$"
#define BATTERY_STATUS_FULL "Full"
#define BATTERY_STATUS_CHR "Charging"
#define BATTERY_STATUS_DIS "Discharging"
#define BATTERY_STATUS_UNK "Unknown"
#define BATTERY_LABEL_FULL "full"
#define BATTERY_LABEL_CHR "chr"
#define BATTERY_LABEL_DIS "bat"
#define BATTERY_LABEL_UNK "unk"

typedef struct      s_power {
    char            *status;
    long            current;
    long            max;
}                   t_power;

// brightness
#define BRIGHTNESS_DIR "/sys/class/backlight/intel_backlight"
#define BRIGHTNESS_CURRENT "actual_brightness"
#define BRIGHTNESS_MAX "max_brightness"
#define BRIGHTNESS_LABEL "brg"

// usage cpu
#define PROC_STAT "/proc/stat"
#define CPU_STATS_PATTERN "^cpu[ \t]+(([0-9]+ ){9}[0-9]+)$"
#define CPU_LABEL "cpu"
#define CPU_STATS_SIZE 8

typedef struct      s_cpu {
    long            prev_idle;
    long            prev_total;
}                   t_cpu;

// temperature
#define TEMP_DIR "/sys/devices/platform/coretemp.0/hwmon/*"
#define TEMP_LABEL "temp"
#define TEMP_ROUND_THRESHOLD 500

// wireless
#define NL80211 "nl80211"
#define WLAN_EID_SSID 0
#define WIRELESS_INTERFACE "wlan0"
#define WIRELESS_FLAG_HAS_ESSID (1 << 0)
#define WIRELESS_FLAG_HAS_SIGNAL (1 << 1)
#define WIRELESS_ESSID_MAX_SIZE 16
#define WIRELESS_UNK_LABEL "SSID unk:"
#define NOISE_FLOOR_DBM (-90)
#define SIGNAL_MAX_DBM (-20)

typedef struct      s_wireless {
    unsigned int    flags;
    unsigned int    ifindex;
    char            *ifname;
    int             nl80211_id;
    uint8_t         bssid[ETH_ALEN];
    char            *essid;
    int             signal;
}                   t_wireless;

// memory
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

// volume
#define PULSE_SINK_NAME "alsa_output.pci-0000_00_1f.3.analog-stereo"
#define PULSE_APP_NAME "qlstatus"
#define VOLUME_LABEL "vol"
#define VOLUME_MUTED_LABEL "mut"

typedef struct              s_pulse {
    pa_threaded_mainloop    *mainloop;
    pa_context              *context;
    uint8_t                 connected;
}                           t_pulse;

/* GLOBAL STRUCTURE */
typedef struct          s_main {
    t_module            *modules;
    char                *format;
    char                *rate;
    t_opt               *opts;
}                       t_main;

/* FUNCTIONS */
size_t  v_strlen(const char *str);
char    *v_strncpy(char *dest, const char *src, size_t n);
void    v_memset(void *ptr, uint8_t c, size_t size);
long    to_int(const char *str);
char	*to_str(long nb);
int     putstr(const char *str);
char    *get_option_value(t_opt *opts, const char *key, int size);

// format
char    *format(t_main *main);

// regex
bool    match_pattern(const char *regex, const char *str);
char    *substring(const char *regex, const char *str);
char    **multiple_subs(const char *regex, const char *str, int nmatch);

// alloc memory
char    *alloc_buffer(size_t size);
void    *alloc_ptr(size_t size);

// read files / dirs
void    close_stream(FILE *stream, const char *file);
FILE    *open_stream(const char *file);
void    free_files(char **files);
char    **read_dir(const char *path, const char *regex);
char    *read_file(const char *file);

// init modules
int     parse_config_file(t_main *main, const char *file);

// module routines
void    *get_battery(void *data);
void    *get_volume(void *data);
void    *get_brightness(void *data);
void    *get_cpu_usage(void *data);
void    *get_temperature(void *data);
void    *get_wireless(void *data);
void    *get_memory(void *data);

// free modules
void    volume_free(void *data);
void    wireless_free(void *data);

#endif /* !QLSTATUS_H_ */
