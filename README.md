# ql-status
Light and modular status bar for tiling window managers like [dwm](https://dwm.suckless.org/) or [spectrwm](https://github.com/conformal/spectrwm). Only for Linux based distributions, written in C.

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
- average temperature of inputs in degree Celsius
- used memory in percent
- current audio volume in percent
- critical thresholds warning

## Dependencies
- libc
- POSIX threads (libpthread)
- libnotify
- PulseAudio Library (libpulse)
- [Netlink Protocol Library Suite](https://www.infradead.org/~tgr/libnl/) (libnl3)

## Install from Arch User Repository (AUR)
Link: [qlstatus package](https://aur.archlinux.org/packages/qlstatus/)  
Once installed, copy the default configuration file into your config directory:
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
You should override the default value of some options by editing `~/.config/qlstatus/qlstatus.conf`. You can find an example of this file at the root of this repository.
Each option has `key = value` form.

~~For the detailed list of available options, see the [wiki](https://github.com/qlem/qlstatus/wiki/Options).~~

### Output format
To enable and change the output order of some modules, override the value of the `format` option.
```
format = %U  %M  %B < %D
```

Use the following escape sequences to enable the desired modules:
- `%D` date and time
- `%U` CPU
- `%T` temperature
- `%M` memory
- `%L` brightness
- `%V` volume
- `%B` battery
- `%W` wireless

For each module you can change its output format, e.g.:
```
# battery
battery_format = %L %V      # gives the output: bat 42%
...

# temperature
temperature = %V            # to only print the module value
...

# wireless
wireless_format = %L: %V    # gives: ESSID: 84%
...
```

Any no-escaped character will be print.

### Critical thresholds
For modules that have a critical threshold (battery, cpu usage, temperature and memory), you can enable the support of the spectrwm colors `enable_spectrwm_colors = 1`
and set the index of the desired spectrwm foreground color to use when the module value reach the critical threshold `critical_color_index = n`.

*-- The support of spectrwm colors is stopped for now --*

### Power notifications
By default notifications related to the battery are enabled. Change the value of the option `battery_notifications` to enable or disable power notifications.

Three notifications are triggered:
- when the battery is fully charged
- when the AC adpater is plugged
- when the battery reach the critical threshold

You can choose the desired icons by specifying their name through the options:
- `battery_notify_icon_full`
- `battery_notify_icon_plugged`
- `battery_notify_icon_low`

## License
MIT
