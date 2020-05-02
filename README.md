# qlstatus
A small program for generating an output stream for the status bar of window managers like [spectrwm](https://github.com/conformal/spectrwm). Written in C. Only for Linux based distributions.

---
![alt text](https://raw.githubusercontent.com/qlem/qlstatus/master/screenshot.png)

## Features
- current ESSID with signal quality in percent
- battery status with remaining percent
- brightness level in percent
- cpu usage in percent
- cpu temperature in degree Celsius
- used memory in percent
- current audio volume in percent

## Configuration
You can edit `qlstatus.h` and change default value of any variable. Then compile the binary.

### Configuration file
*- Coming soon -*

## Dependencies
- libc
- POSIX threads (libpthread)
- PulseAudio Library (libpulse)
- [Netlink Protocol Library Suite](https://www.infradead.org/~tgr/libnl/) (libnl3 libnl-genl-3)

## Compilation
TODO

## License
MIT
