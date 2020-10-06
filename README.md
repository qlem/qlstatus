# ql-status
Light and modular status bar for tiling window managers like [dwm](https://dwm.suckless.org/) or [spectrwm](https://github.com/conformal/spectrwm). Only for Linux based distributions, written in C.

---
![alt text](https://raw.githubusercontent.com/qlem/qlstatus/master/screenshot.png)

## Goals
Light, modular, eazy to use, eazy to maintain.. No Memory Leaks !

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
You should override the default value of some options by editing `~/.config/qlstatus/qlstatus.conf`.  
Each option has `key = value` form. For the detailed list of available options, see the [wiki](https://github.com/qlem/qlstatus/wiki/Options).

### Output format
Change the output order of the modules by overriding the value of the `format` option, e.g. `format = %U  %M  %B`.

Use the following escape sequences to add the desired modules:
- `%D` date and time
- `%U` CPU
- `%T` temperature
- `%M` memory
- `%L` brightness
- `%V` volume
- `%B` battery
- `%W` wireless

Make sure the modules present in the format string are **enabled**.

### Critical thresholds
For modules that have a critical threshold (battery, cpu usage, temperature and memory), you can enable the support of the spectrwm colors `enable_spectrwm_colors = 1`
and set the index of the desired spectrwm foreground color to use when the module value reach the critical threshold `critical_color_index = n`.

### Power notifications
By default notifications related to the battery are enabled. Change the value of the option `battery_notifications` to enable or disable power notifications.

Three notifications can be send:
- when the battery is fully charged
- when the AC adpater is plugged
- when the battery reach the critical threshold

You can choose the desired icons by specifying their name through the options:
- `battery_notify_icon_full`
- `battery_notify_icon_plugged`
- `battery_notify_icon_low`

## License
MIT
