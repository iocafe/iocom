notes 26.4.2021/pekka
4_ioboard_test example demonstrates IO board, connected trough TCP socket. This example assumes one memory block 
for inputs and other for the outputs. Implementation doesn't need dynamic memory allocation or multithreading, 
thus it should run on any platform including microcontrollers.

build_opt.h - Extra defines for Arduino IDE. Needed to set serial port Rx and Tx buffer sizes to 256.
platformio.ini - Settings for Visual Studio Code + Platform IO + Arduino build.


