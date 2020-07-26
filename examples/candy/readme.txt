notes 22.7.2020/pekka
Candy - camera IO board example.

Camera as an IO device which connects to server trough TLS socket. Currently works with ESP32 camera, 
Raspberry PI camera and Windows USB camera.

By default the application connects to any server which sends "lighthouse" multicasts. 
Only DHCP configuration makes sense for WiFi camera, static IP interface configuration is disabled.

ESP32 specific
***************
On ESP32 WiFi network and other connection settings can be configured either using "gazerbeam" library (Android phone's
flash light) or throuhgh serial port. To use gazerbeam, the IO device must be either equipped with gazerbeam 
electronics to set wifi network name/password.

Loading program to flash for the first time: 
* Use USB to TTL 3.3V converter (google "ESP32-cam pins", TX, RX, GND (and 3.3V, if ESP32 is not powered from elsewhere). 
* Before loading software connect GPIO pin 0 to GND
- Press reset button. Now board should be ready to receive program. After loading you need to disconnect GPIO 0 from ground and press reboot.

