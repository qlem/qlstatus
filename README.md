# qlstatus
Small program for generating output stream for status bar of window managers like [spectrwm](https://github.com/conformal/spectrwm). Written in C. Only for Linux based distributions.

---
![alt text](https://raw.githubusercontent.com/qlem/qlstatus/master/screenshot.png)

## Modules
- current ESSID with signal quality in percent
- battery status with percent remaining
- brightness level in percent
- cpu usage in percent
- cpu temperature in degree Celsius
- current audio volume in percent (not yet available)

## Configuration
You can configure **qlstatus** by changing the values of the variables of the desired module in the header file `qlstatus.h`.

## Dependencies
- libc
- [Netlink Protocol Library Suite](https://www.infradead.org/~tgr/libnl/) (libnl3)

## Compilation
TODO

## License
MIT
