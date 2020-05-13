# ql-status
A small program that generates a stream of system statistics for the status bar of window managers like [spectrwm](https://github.com/conformal/spectrwm). Only for Linux based distributions, written in C.

---
![alt text](https://raw.githubusercontent.com/qlem/qlstatus/master/screenshot.png)

## Goals
Light, modular, eazy to use, eazy to maintain.. No Memory Leaks !

## Features
- current ESSID with signal quality in percent
- battery status with remaining percent
- brightness level in percent
- CPU usage in percent
- average temperature of inputs in degree Celsius
- used memory in percent
- current audio volume in percent
- critical thresholds warning

## Configuration
You should override the default value of some options by editing `~/.config/qlstatus/qlstatus.conf`.  
Each option has `key = value` form. For the detailed list of available options, see the [wiki](https://github.com/qlem/qlstatus/wiki/Options).

### Output format
Each module is displayed with the form `label value[unit]`. You can change the order and the whitespaces between them by overriding 
the value of the `format` option, e.g. `format = %U  %M  %B`.

Use the following escape sequences to add the desired modules:
- `%U` CPU
- `%T` temperature
- `%M` memory
- `%L` brightness
- `%V` volume
- `%B` battery
- `%W` wireless

Make sur that the modules present in the format string are **enabled**.

### Critical thresholds
For modules that have a critical threshold (battery, cpu usage, temperature and memory), you can enable the support of the spectrwm colors `enable_spectrwm_colors = 1` 
and set the index of the desired spectrwm foreground color to use when the module value reach the critical threshold `critical_color_index = n`.

## Dependencies
- libc
- POSIX threads (libpthread)
- PulseAudio Library (libpulse)
- [Netlink Protocol Library Suite](https://www.infradead.org/~tgr/libnl/) (libnl3)

## Compilation
Compilation and basic install:
```
cd /path/to/repo
mkdir build
cmake -B build/ .
cmake --build build/
cp build/qlstatus ~/bin
```

## License
MIT
