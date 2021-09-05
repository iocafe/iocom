devicedir library
23.8.2021 / pekka

Debug console related functions:
- Get current IOCOM state as JSON. This include memory block, transfer buffer, connection and end point structures. 
- Set IO device's wifi user, password, IP address from serial console.

The library is mostly used for development testing.

Build files:
- platformio.ini: PlatformIO build main configuration file.
- CmakeLists.txt: Cmake build configuration, used also by PlatformIO ESP-IDF builds.
- code/CmakeLists.txt: Second CmakeLists.txt, only for PlatformIO: Lists source files and registers library component.
- library.json: Library manifest for PlatformIO builds.

