# iocom
The IOCOM C library implements communication between a IoT/IO devices and control computer over TCP network (TLS optional) or full duplex serial communication.
The iocom depends on eosal library for platform/operating system abstraction.

7.8.2020 status: \
Still working on first demo/prototype projects and to implement features which come across as useful and fix bugs as we find these. First supported microcontroller platform is ESP-32. We use Linux as development environment (includes Raspberry Pi), so Windows specific stuff lags behind. We have been testing also on various STM32 nucleo boars, Teensy 3.6, etc, and plan is to support these after ESP32 is complete. There is still lot to do on documentation, usage instructions, micro-controller support and testing. Documentation has now been moved to Sphinx. 

Note:\
The iocom library still is very difficult to use for a new person, in practice it would require quite a bit of correspondence to make it work. It is anyhow available for download in case you want to participate in development while it is still easy to make changes.
https://github.com/iocafe/iocom/blob/master/doc/190626-iocom-library/190422-iocom-library.pdf
