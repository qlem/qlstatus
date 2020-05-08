# qlstatus
A small program that generates a system statistics stream for the status bar of window managers like [spectrwm](https://github.com/conformal/spectrwm). Only for Linux based distributions. Written in C.

---
![alt text](https://raw.githubusercontent.com/qlem/qlstatus/master/screenshot.png)

## Goals
Light, modular, simple to use, simple to maintain.. No Memory Leaks !

## Features
- current ESSID with signal quality in percent
- battery status with remaining percent
- brightness level in percent
- CPU usage in percent
- average temperature of inputs in degree Celsius
- used memory in percent
- current audio volume in percent
- visual warning when critical threshold is reached for battery, cpu usage, temperature and used memory (coming soon)

## Configuration
You should override the default value of some options by editing `~/.config/qlstatus/qlstatus.conf`.  
Each option has `key = value` form. Commented lines is not yet supported in the config file.

### Output format
A module is displayed with the form `label value[unit]`. 
You can change the order and the whitespaces between them by overriding the value of the `format` option.  

Each module is represented by a character id escaped by the symbol `%`:
- `%U` CPU
- `%T` temperature
- `%M` memory
- `%L` brightness
- `%V` volume
- `%B` battery
- `%W` wireless

Make sur that the modules present in the format string are enabled. See the options below.

### Global options
|Option |Description |Default value|
|:---   |:---        |:---         |
|`format`|Format the output stream.|`%U  %T  %M  %L  %V  %B  %W`|
|`rate`|Refresh rate of the display. `1s` or `1000ms` form|`1s`|

### Battery options
|Option |Description |Default value|
|:---   |:---        |:---         |
|`battery_enabled`|Enable or not this module. `1` or `0`|`1`|
|`battery_name`|Battery name. Can be found in `/sys/class/power_supply`|`BAT0`|
|`battery_label_full`|Label displayed when the battery state is full|`full`|
|`battery_label_charging`|Label displayed when the battery state is charging|`chr`|
|`battery_label_discharging`|Label displayed when the battery state is discharging|`bat`|
|`battery_label_unknown`|Label displayed when the battery state is unknown|`unk`|
|`battery_critical`|Critical threshold under which the label will be modified|`20`|

### CPU usage options
|Option |Description |Default value|
|:---   |:---        |:---         |
|`cpu_usage_enabled`|Enable or not this module. `1` or `0`|`1`|
|`cpu_usage_label`|Label displayed for this module|`cpu`|
|`cpu_usage_critical`|Critical threshold under which the label will be modified|`80`|

### Temperature options
|Option |Description |Default value|
|:---   |:---        |:---         |
|`temperature_enabled`|Enable or not this module. `1` or `0`|`1`|
|`temperature_label`|Label displayed for this module|`temp`|
|`temperature_dir`|Desired inputs directory. You can use an `*` in the last part of the path so the program resolve it by the 1st valid sub directory|`/sys/devices/platform/coretemp.0/hwmon/*`|
|`temperature_input`|Input files numbers that will be treated by the program. Can be a range `2-5` or a single input `3`|`2-5`|
|`temperature_critical`|Critical threshold under which the label will be modified|`70`|

### Memory options
|Option |Description |Default value|
|:---   |:---        |:---         |
|`memory_enabled`|Enable or not this module. `1` or `0`|`1`|
|`memory_label`|Displayed label for this module|`mem`|
|`memory_critical`|Critical threshold under which the label will be modified|`80`|

### Brightness options
|Option |Description |Default value|
|:---   |:---        |:---         |
|`brightness_enabled`|Enable or not this module. `1` or `0`|`1`|
|`brightness_label`|Displayed label for this module|`brg`|
|`brightness_dir`|Directory of the brightness class|`/sys/class/backlight/intel_backlight`|

### Volume options
|Option |Description |Default value|
|:---   |:---        |:---         |
|`volume_enabled`|Enable or not this module. `1` or `0`|`1`|
|`volume_label`|Displayed label when sound is not muted|`vol`|
|`volume_muted_label`|Displayed label when sound is muted|`mut`|
|`volume_sink_name`|Sink name. Can be found with the command `pacmd stat`|`alsa_output.pci-0000_00_1f.3.analog-stereo`|

### Wireless options
|Option |Description |Default value|
|:---   |:---        |:---         |
|`wireless_enabled`|Enable or not this module. `1` or `0`|`1`|
|`wireless_unknown_label`|Label displayed when no SSID found|`SSID unk:`|
|`wireless_interface`|Name of the wireless interface. Can be found with the command `ip link`|`wlp2s0`|

## Dependencies
- libc
- POSIX threads (libpthread)
- PulseAudio Library (libpulse)
- [Netlink Protocol Library Suite](https://www.infradead.org/~tgr/libnl/) (libnl3 libnl-genl-3)

## Compilation
TODO

## License
MIT
