# ql-status
Light and modular status bar for tiling window managers like [dwm](https://dwm.suckless.org/) or [spectrwm](https://github.com/conformal/spectrwm).  
Only for Linux based distributions, written in C.

---
![alt text](https://raw.githubusercontent.com/qlem/qlstatus/master/screenshot.png)

## Goals
Light, modular, easy to use, easy to maintain.

## Features
- date and time
- current ESSID with signal quality in percent
- battery status with remaining percent
- power notifications
- brightness level in percent
- CPU usage in percent
- CPU frequency
- average temperature of inputs in degree Celsius
- detailed / percent used memory
- current audio volume in percent

## Dependencies
- libc
- POSIX threads (libpthread)
- libnotify
- PulseAudio Library (libpulse)
- [Netlink Protocol Library Suite](https://www.infradead.org/~tgr/libnl/) (libnl3)

## Install from Arch User Repository (AUR)
👉🏼  [link](https://aur.archlinux.org/packages/qlstatus/)  
Once installed, copy default configuration file into your config directory.
```
mkdir -p ~/.config/qlstatus
cp /etc/qlstatus.conf ~/.config/qlstatus/qlstatus.conf
```

## Install from sources
```
cd /path/to/repo
mkdir build
cmake -B build/ .
cmake --build build/
cp build/qlstatus ~/bin
```

## Configuration
You should override default value of some options by editing `~/.config/qlstatus/qlstatus.conf`.  
You can find an example of this file at the root of this repository. Each option has `key = value` form.

### Output format
To enable modules and change output order, override the value of `format` option.
```
format = %U  %M  %B < %D
```

Use following escape sequences to enable desired modules:
- `%D` date and time
- `%U` CPU usage
- `%F` CPU freq
- `%T` temperature
- `%M` memory
- `%L` brightness
- `%V` volume
- `%B` battery
- `%W` wireless

For each module you can change its output format by editing related format option, e.g.
```
# battery
battery_format = %L %V      # gives the output: bat 42%

...
battery_format = %V         # to only print remaining percent

...
# wireless
wireless_format = %L: %V    # gives: ESSID: 84%
```

Any no-escaped character will be print.

### Critical thresholds
For modules which support critical threshold, you can enable support of spectrwm colors and set the index of
desired spectrwm foreground color to use when module value reach the critical threshold, e.g.
```
enable_spectrwm_colors = 1
critical_color_index = 2
```

*-- The support of spectrwm colors is stopped for now --*

### Power notifications
By default notifications related to the battery are enabled. Change the value of `battery_notifications` option
to enable or disable power notifications.

Three notifications are triggered:
- when battery is fully charged
- when AC adpater is plugged
- when battery reach critical threshold

You can choose desired icons by specifying their name through the options:
- `battery_notify_icon_full`
- `battery_notify_icon_plugged`
- `battery_notify_icon_low`

## License
MIT
