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
- visual warning when critical threshold is reached for battery, cpu usage, temperature and used memory (coming soon)

## Configuration
You should override the default value of some options by editing `~/.config/qlstatus/qlstatus.conf`.  
Each option has `key = value` form. For the detailed list of available options, see the [wiki](https://github.com/qlem/qlstatus/wiki/Options).

### Output format
The modules are displayed with the form `label value[unit]`. 
You can change the order and the whitespaces between them by overriding the value of the `format` option, e.g. `%U  %M  %B`.

Each module is represented by a character id escaped by the symbol `%`:
- `%U` CPU
- `%T` temperature
- `%M` memory
- `%L` brightness
- `%V` volume
- `%B` battery
- `%W` wireless

Make sur that the modules present in the format string are enabled.

## Dependencies
- libc
- POSIX threads (libpthread)
- PulseAudio Library (libpulse)
- [Netlink Protocol Library Suite](https://www.infradead.org/~tgr/libnl/) (libnl3)

## Compilation
TODO

## License
MIT
