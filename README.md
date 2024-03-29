# ql-status
Light and modular system monitoring software for status bar of tiling window managers like [xmonad](https://xmonad.org/) or [dwm](https://dwm.suckless.org/).  
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
- mounted filesystem usage

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
Compile from sources and install binary, man page and system-configuration file:
```
cd /path/to/repo
mkdir build
cmake -B build/
cmake --build build/
cmake --install build/
```

Prefix install directory is set by default to `/usr/local`. To changes this use `--prefix` option:
```
cmake --install build/ --prefix /my/install/dir
```

Once installed, the list of installed files can be found in `build/install_manifest.txt`.

## Configuration
You should override default value of some options by editing `~/.config/qlstatus/qlstatus.conf`.  
You can find an example of this file at the root of this repository. Each option has `key = value` form.

### Output format
To enable modules and change the output order of the modules, override the value of the `format` option.
```
format = %U  %M  %B < %D
```

Use following escape sequences to enable the desired modules:
- `%D` date and time
- `%U` CPU usage
- `%F` CPU freq
- `%T` temperature
- `%M` memory
- `%L` brightness
- `%V` volume
- `%B` battery
- `%W` wireless
- `%S` mounted filesystem usage

For each module you can change its output format by editing the related format option, e.g.
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
For modules which support a critical threshold, you can enable support of spectrwm colors and set the index of
desired spectrwm foreground color to use when a module value reach the critical threshold, e.g.
```
enable_spectrwm_colors = 1
critical_color_index = 2
```

*-- The support of spectrwm colors is stopped for now --*

### Power notifications
By default notifications related to the battery are enabled. Change the value of the `power_notifications` option
to enable or disable power notifications.

Three notifications are triggered:
- when battery is fully charged
- when AC adpater is plugged
- when battery reach the critical threshold

You can choose desired icons by specifying their name through the following options:
- `power_notify_icon_full`
- `power_notify_icon_plugged`
- `power_notify_icon_low`

## License
MIT
