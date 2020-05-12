# iocom
The IOCOM C library implements communication between a IoT/IO devices and control computer over TCP network (TLS optional) or full duplex serial communication.

15.4.2020 status: \
Now we are working on first real demo/prototype projects and implement features which come across as useful and fix bugs as we find these. First supported microcontroller platform is ESP-32 and we use Linux as development environment, so Windows specific stuff lags behind. There is still lot to do on documentation, usage instructions, micro-controller support and testing. Idea is to try to move and complete the documentation with Sprinx. The iocom depends on eosal library for platform/operating system abstraction. 

Note:\
The iocom library still is very difficult to use for a new person, in practice it would require quite a bit of correspondence to make it work. It is anyhow available for download in case you want to participate in development while it is still easy to make changes.
https://github.com/iocafe/iocom/blob/master/doc/190626-iocom-library/190422-iocom-library.pdf
