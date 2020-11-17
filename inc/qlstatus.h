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

#include <libnotify/notify.h>

/* GLOBAL */
#define BASE 10
#define RATE "1s"
#define NB_MODULES 8
#define NSEC 999999999
#define NSEC_TO_SEC(nsec) nsec / (long)1e9
#define REM_NSEC(nsec) nsec % (long)1e9
#define PERCENT(value, total) value * 100 / total
#define CONFIG_FILE ".config/qlstatus/qlstatus.conf"
#define HOME_PATTERN "^HOME=(.+)$"
#define SPWM_COLOR_START "+@fg="
#define SPWM_COLOR_STOP "+@fg=0;"

/* OUTPUT FORMAT
 * %D: time info
 * %U: cpu usage
 * %T: temp average
 * %M: memory usage
 * %L: brightness level
 * %V: current volume
 * %B: remaining battery
 * %W: wireless info
 */
#define DEFAULT_FORMAT "%U  %T  %M  %L  %V  %B  %W  %D"

/* OPTIONS */
typedef enum        opt_type {
    STRING,
    NUMBER
}                   opt_type;

typedef struct      s_opt {
    char            *key;
    void            *value;
    char            *pattern;
    opt_type        type;
    uint8_t         to_free;
}                   t_opt;

// number of options per module
#define GLOBAL_NOPTS 4
#define TIME_NOPTS 1
#define BAT_NOPTS 12
#define CPU_NOPTS 3
#define TEMP_NOPTS 5
#define MEM_NOPTS 3
#define BRG_NOPTS 3
#define VOL_NOPTS 4
#define WLAN_NOPTS 3

// option patterns
#define TEXT_PATTERN "^.{1,100}$"
#define THRESHOLD_PATTERN "^[0-9]{1,3}$"
#define BOOLEAN_PATTERN "^0$|^1$"
#define PATH_PATTERN "^\\/$|^\\/([^\n\r\t /]+\\/?)+$"
#define LABEL_PATTERN "^.{1,5}$"
#define WL_LABEL_PATTERN "^.{1,15}$"
#define RATE_PATTERN "^[0-9]+s$|^[0-9]+ms$"
#define BAT_NAME_PATTERN "^BAT[0-9]$"
#define IN_TEMP_PATTERN "^([1-9])$|^([1-9]-[1-9])$"
#define COLOR_IDX_PATTERN "^[0-9]$"

// global options
#define OPT_FORMAT "format"
#define OPT_RATE "rate"
#define OPT_SPWM_COLORS "enable_spectrwm_colors"
#define OPT_C_COLOR_IDX "critical_color_index"

// time options
#define OPT_TIME_FORMAT "time_format"

// battery options
#define OPT_BAT_FORMAT "battery_format"
#define OPT_BAT_NAME "battery_name"
#define OPT_BAT_LB_FULL "battery_label_full"
#define OPT_BAT_LB_CHR "battery_label_charging"
#define OPT_BAT_LB_DIS "battery_label_discharging"
#define OPT_BAT_LB_UNK "battery_label_unknown"
#define OPT_BAT_CRITICAL "battery_critical"
#define OPT_BAT_FULL_DESIGN "battery_full_design"
#define OPT_BAT_NOTIFY "battery_notifications"
#define OPT_BAT_NOTIFY_ICON_FULL "battery_notify_icon_full"
#define OPT_BAT_NOTIFY_ICON_PLUGGED "battery_notify_icon_plugged"
#define OPT_BAT_NOTIFY_ICON_LOW "battery_notify_icon_low"

// cpu options
#define OPT_CPU_FORMAT "cpu_format"
#define OPT_CPU_LABEL "cpu_label"
#define OPT_CPU_CRITICAL "cpu_critical"

// temperature options
#define OPT_TEMP_FORMAT "temperature_format"
#define OPT_TEMP_LABEL "temperature_label"
#define OPT_TEMP_DIR "temperature_dir"
#define OPT_TEMP_INPUT "temperature_input"
#define OPT_TEMP_CRITICAL "temperature_critical"

// memory options
#define OPT_MEM_FORMAT "memory_format"
#define OPT_MEM_LABEL "memory_label"
#define OPT_MEM_CRITICAL "memory_critical"

// brightness options
#define OPT_BRG_FORMAT "brightness_format"
#define OPT_BRG_LABEL "brightness_label"
#define OPT_BRG_DIR "brightness_dir"

// volume options
#define OPT_VOL_FORMAT "volume_format"
#define OPT_VOL_LABEL "volume_label"
#define OPT_VOL_LB_MUTED "volume_muted_label"
#define OPT_VOL_SINK "volume_sink_name"

// wireless options
#define OPT_WLAN_FORMAT "wireless_format"
#define OPT_WLAN_LB_UNK "wireless_unknown_label"
#define OPT_WLAN_IFACE "wireless_interface"

/* TOKENS */
#define TBUFFER_MAX_SIZE 16

typedef struct      s_token {
    uint8_t         enabled;
    char            fmtid;
    char            buffer[TBUFFER_MAX_SIZE];
}                   t_token;

/* MODULES */
#define MBUFFER_MAX_SIZE 32

typedef struct      s_module {
    uint8_t         enabled;
    char            fmtid;
    char            buffer[MBUFFER_MAX_SIZE];
    uint8_t         critical;
    void            *data;
    t_opt           *opts;
    int             nopts;
    pthread_t       thread;
    void            *(*routine)(void *);
    void            (*init)(void *);
    void            (*mfree)(void *);
}                   t_module;

// time
#define TIME_DEFAULT_FORMAT "%a %d %b %Y, %R %Z"

typedef struct      s_mtime {
    char            *format;
}                   t_mtime;

// battery
#define BAT_FORMAT "%L %V"
#define BAT_TOKENS 2
#define BATTERY_NAME "BAT0"
#define POWER_DIR "/sys/class/power_supply"
#define POWER_FILE "uevent"
#define PW_STATUS_PATTERN "^POWER_SUPPLY_STATUS=(Discharging|Charging|Full|Unknown)$"
#define PW_MAX_FD_PATTERN "^POWER_SUPPLY_ENERGY_FULL_DESIGN=([0-9]+)$"
#define PW_MAX_PATTERN "^POWER_SUPPLY_ENERGY_FULL=([0-9]+)$"
#define PW_CURRENT_PATTERN "^POWER_SUPPLY_ENERGY_NOW=([0-9]+)$"
#define BAT_STATUS_FULL "Full"
#define BAT_STATUS_CHR "Charging"
#define BAT_STATUS_DIS "Discharging"
#define BAT_LABEL_FULL "full"
#define BAT_LABEL_CHR "chr"
#define BAT_LABEL_DIS "bat"
#define BAT_LABEL_UNK "unk"
#define BAT_NOTIFY_FULL "Battery fully charged"
#define BAT_NOTIFY_PLUGGED "AC adapter plugged"
#define BAT_NOTIFY_LOW "Battery is low"

typedef enum        pw_status {
    PW_FULL,
    PW_CHARGING,
    PW_DISCHARGING,
    PW_CRITICAL,
    PW_UNKNOWN
}                   pw_status;

typedef struct          s_notify {
    uint8_t             enabled;
    NotifyNotification  *notify;
    char                *ic_full;
    char                *ic_plugged;
    char                *ic_low;
}                       t_notify;

typedef struct      s_power {
    char            *file;
    char            *lb_chr;
    char            *lb_dis;
    char            *lb_unk;
    char            *lb_full;
    uint8_t         full_design;
    char            *raw_status;
    long            current;
    long            max;
    int             cthreshold;
    pw_status       status;
    pw_status       last_status;
    t_notify        notify;
    t_token         tokens[BAT_TOKENS];
}                   t_power;

// brightness
#define BRG_FORMAT "%L %V"
#define BRG_TOKENS 2
#define BRG_DIR "/sys/class/backlight/intel_backlight"
#define BRG_CURRENT "actual_brightness"
#define BRG_MAX "max_brightness"
#define BRG_LABEL "brg"

typedef struct      s_brg {
    char            *label;
    char            *current_file;
    char            *max_file;
    t_token         tokens[BRG_TOKENS];
}                   t_brg;

// cpu
#define CPU_FORMAT "%L %V"
#define CPU_TOKENS 2
#define PROC_STAT "/proc/stat"
#define CPU_STATS_PATTERN "^cpu[ \t]+(([0-9]+ ){9}[0-9]+)$"
#define CPU_LABEL "cpu"
#define CPU_STATS_SIZE 8

typedef struct      s_cpu {
    char            *label;
    long            stats[CPU_STATS_SIZE];
    long            prev_idle;
    long            prev_total;
    int             cthreshold;
    t_token         tokens[CPU_TOKENS];
}                   t_cpu;

// temperature
#define TEMP_FORMAT "%L %V"
#define TEMP_TOKENS 2
#define TEMP_DIR "/sys/devices/platform/coretemp.0/hwmon/*"
#define TEMP_LABEL "temp"
#define TEMP_ROUND_THRESHOLD 500

typedef struct      s_temp {
    char            *label;
    char            **inputs;
    int             cthreshold;
    t_token         tokens[TEMP_TOKENS];
}                   t_temp;

// wireless
#define WLAN_FORMAT "%L: %V"
#define WLAN_TOKENS 2
#define NL80211 "nl80211"
#define WLAN_EID_SSID 0
#define WLAN_INTERFACE "wlan0"
#define WLAN_FLAG_HAS_ESSID (1 << 0)
#define WLAN_FLAG_HAS_SIGNAL (1 << 1)
#define WLAN_ESSID_MAX_SIZE 16
#define WLAN_UNK_LABEL "SSID unk"
#define NOISE_FLOOR_DBM (-90)
#define SIGNAL_MAX_DBM (-20)

typedef struct      s_wlan {
    struct nl_sock  *socket;
    unsigned int    flags;
    unsigned int    ifindex;
    char            *ifname;
    int             nl80211_id;
    uint8_t         bssid[ETH_ALEN];
    char            essid[WLAN_ESSID_MAX_SIZE];
    char            *lb_unk;
    int             signal;
    t_token         tokens[WLAN_TOKENS];
}                   t_wlan;

// memory
#define MEM_FORMAT "%L %V"
#define MEM_TOKENS 2
#define PROC_MEMINFO "/proc/meminfo"
#define MEM_TOTAL_PATTERN "^MemTotal:[ \t]+([0-9]+) kB$"
#define MEM_FREE_PATTERN "^MemFree:[ \t]+([0-9]+) kB$"
#define MEM_BUFFERS_PATTERN "^Buffers:[ \t]+([0-9]+) kB$"
#define MEM_CACHED_PATTERN "^Cached:[ \t]+([0-9]+) kB$"
#define MEM_SRECLAIM_PATTERN "^SReclaimable:[ \t]+([0-9]+) kB$"
#define MEM_LABEL "mem"

typedef struct  s_mem {
    char        *label;
    long        total;
    long        free;
    long        buffers;
    long        cached;
    long        sreclaim;
    int         cthreshold;
    t_token     tokens[MEM_TOKENS];
}               t_mem;

// volume audio
#define VOLUME_FORMAT "%L %V"
#define VOLUME_TOKENS 2
#define PULSE_SINK_NAME "alsa_output.pci-0000_00_1f.3.analog-stereo"
#define PULSE_APP_NAME "qlstatus"
#define VOLUME_LABEL "vol"
#define VOLUME_MUTED_LABEL "mut"

typedef struct              s_pulse {
    char                    *sink;
    char                    *label;
    char                    *lb_mute;
    pa_threaded_mainloop    *mainloop;
    pa_context              *context;
    uint8_t                 connected;
    t_token                 tokens[VOLUME_TOKENS];
}                           t_pulse;

/* GLOBAL STRUCTURE */
#define BUFFER_MAX_SIZE 256

typedef struct          s_main {
    t_module            *modules;
    char                buffer[BUFFER_MAX_SIZE];
    char                *format;
    char                *rate;
    uint8_t             spwmcolors;
    uint8_t             spwmcoloridx;
    t_opt               *opts;
}                       t_main;

/* FUNCTIONS */
size_t  v_strlen(const char *str);
char    *v_strncpy(char *dest, const char *src, size_t n);
char    *v_strsncpy(char *dest, const char *src, int start, size_t n);
void    v_memset(void *ptr, uint8_t c, size_t size);
long    to_int(const char *str);
char	*to_str(long nb);
int     putstr(const char *str);

// output format
int     enable_modules(t_main *main);
int     set_output_buffer(t_main *main);
int     set_token_buffer(char *buffer, const char *src);
int     set_module_buffer(t_module *module, const char *format, t_token *tokens, int size);
int     init_module_tokens(t_module *module, const char *format, t_token *tokens, int size);

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

// config file
int     load_config_file(t_main *main, const char *file);

// notify
NotifyNotification      *notify_new(const char *summary);
int     notify(NotifyNotification *notify, const char *summary, const char *body, const char *icon, NotifyUrgency urgency);

// routines
void    *run_time(void *data);
void    *run_battery(void *data);
void    *run_cpu_usage(void *data);
void    *run_memory(void *data);
void    *run_temperature(void *data);
void    *run_brightness(void *data);
void    *run_wireless(void *data);
void    *run_volume(void *data);

// init modules
void    init_time(void *data);
void    init_battery(void *data);
void    init_cpu_usage(void *data);
void    init_memory(void *data);
void    init_temperature(void *data);
void    init_brightness(void *data);
void    init_wireless(void *data);
void    init_volume(void *data);

// free modules
void    free_time(void *data);
void    free_battery(void *data);
void    free_cpu_usage(void *data);
void    free_memory(void *data);
void    free_temperature(void *data);
void    free_brightness(void *data);
void    free_wireless(void *data);
void    free_volume(void *data);

#endif /* !QLSTATUS_H_ */
