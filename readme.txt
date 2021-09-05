9.8.2021 / pekka
The IOCOM C library implements communication between IoT/IO devices and control computer over 
TCP network (TLS optional) or full duplex serial communication. The iocom depends on eosal 
library for platform/operating system abstraction.

Build files:
- platformio.ini: PlatformIO build main configuration file.
- CmakeLists.txt: Cmake build configuration, used also by PlatformIO ESP-IDF builds.
- code/CmakeLists.txt: Second CmakeLists.txt, only for PlatformIO: Lists source files and registers library component.
- library.json: Library manifest for PlatformIO builds.
