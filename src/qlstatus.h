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

#define CENT 100
#define BAT_CURRENT "/sys/class/power_supply/BAT0/energy_now"
#define BAT_MAX "/sys/class/power_supply/BAT0/energy_full_design"
#define BAT_STATUS "/sys/class/power_supply/BAT0/status"
#define BAT_STATUS_FULL "Full\n"
#define BAT_STATUS_CHARGING "Charging\n"
#define BAT_STATUS_DISCHARGING "Discharging\n"
#define BAT_F_STATUS_FULL "full"
#define BAT_F_STATUS_CHARGING "chr"
#define BAT_F_STATUS_DISCHARGING "bat"
#define BAT_F_STATUS_UNKNOW "unk"

char    *read_file(char *path);
void    v_memset(char *buffer, size_t size, char c);
char    *alloc_buffer(int size);
char	*int_to_str(long nb);
char    *get_battery();

#endif /* !QLSTATUS_H_ */
