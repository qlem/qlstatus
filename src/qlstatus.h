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
#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/nl80211.h>
#include <linux/if_ether.h>

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

/* BRIGHTNESS */
#define BRIGHTNESS_CURRENT "/sys/class/backlight/intel_backlight/actual_brightness"
#define BRIGHTNESS_MAX "/sys/class/backlight/intel_backlight/max_brightness"
#define BRIGHTNESS_LABEL "brg"

/* CPU USAGE */
#define PROC_STAT "/proc/stat"
#define CPU_USAGE_LABEL "cpu"

typedef struct      s_cpu {
    long            prev_idle;
    long            prev_total;
}                   t_cpu;

/* CPU TEMP */
#define CPU_TEMP_DIR "/sys/devices/platform/coretemp.0/hwmon/*"
#define CPU_TEMP_INPUT_PATTERN "^temp[2-5]_input$"
#define CPU_TEMP_LABEL "cpu"
#define CPU_TEMP_ROUND 500

/* WIRELESS */
#define WIRELESS_INTERFACE "wlp2s0"
#define NL80211 "nl80211"
#define WLAN_EID_SSID 0
#define WIRELESS_INFO_FLAG_HAS_ESSID (1 << 0)
#define WIRELESS_INFO_FLAG_HAS_QUALITY (1 << 1)
#define WIRELESS_ESSID_MAX_SIZE 16
#define WIRELESS_UNK_ESSID_LABEL "ESSID unk"
#define WIRELESS_UNK_QUALITY_LABEL "-"
#define NOISE_FLOOR_DBM (-90)
#define SIGNAL_MAX_DBM (-20)
#define PERCENT_VALUE(value, total) ((int)((value) * 100 / (float)(total) + 0.5f))
#define WIRELESS_PREFIX_ERROR "Wireless module error"

typedef struct      s_wireless {
    unsigned int    flags;
    int             nl80211_id;
    unsigned int    if_index;
    uint8_t         bssid[ETH_ALEN];
    char            *essid;
    int             quality;
    int             quality_max;
}                   t_wireless;

/* VOLUME */
// TODO

/* FUNCTIONS */
size_t  v_strlen(const char *str);
char    *v_strncpy(char *dest, const char *src, size_t n);
void    v_memset(void *ptr, int c, size_t size);
long    to_int(const char *str);
char	*to_str(long nb);
int     putstr(const char *str);
void    print(char *fmt, ...);

// memory allocation
char    *alloc_buffer(size_t size);
void    *alloc_ptr(size_t size);

// read files / dirs
void    free_files(char **files);
char    **read_dir(const char *path, const char *regex);
char    *read_file(const char *path);

// modules
char    *get_battery();
char    *get_volume();
char    *get_brightness();
char    *get_cpu_usage(t_cpu *cpu);
char    *get_cpu_temp();
char    *get_wireless();

#endif /* !QLSTATUS_H_ */
