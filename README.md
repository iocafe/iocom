# iocom
The IOCOM C library implements communication between IoT/IO devices and control computer over TCP network (TLS optional) or full duplex serial communication.
The iocom depends on eosal library for platform/operating system abstraction.

7.2.2021 status: \
Working on eobjects data abstraction integration and demo/prototype projects. First supported microcontroller platform is ESP-32, minimum HW is Arduino UNO. We use Linux as development environment (includes Raspberry Pi), so Windows specific stuff lags behind. We have been testing also on various STM32 nucleo boars, Teensy 3.6,/4.1 etc, and plan is to support these after first prototype is complete. There is still quite a bit to do on documentation, usage instructions, higer level abstraction and user interfaces and micro-controller support. Documentation is in Sphinx https://iocafe-doc.readthedocs.io/en/latest/. 

Note:\
The iocom library still is very difficult to use for a new person, in practice it would require quite a bit of correspondence to make it work. It is anyhow available for download in case you want to participate in development while it is still easy to make changes.

ToDo:\
Now dynamic IO structure management is not clean. There are queued events, callback and direct calls to dynamic IO implementation. This all needs to be cleaned and base iocom needs to be used only through callbacks.
