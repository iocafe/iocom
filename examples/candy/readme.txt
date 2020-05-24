notes 22.5.2020/pekka
Candy - camera IO board example.

ESP32 camera as an IO device which connects to controller (server) trough TLS socket. 
This code can be compiled for Windows USB camera as well.

Setting like WiFi, camera device number, IO network and security configuration is based on "gazerbeam"
- Module must be either equipped with gazerbeam electronics to set wifi network name/password.
- Gazerbeam is also used to "factory reset" security to allow reconfiguring connection for to another server with different ceriticate.
- By default the Candy connects to any server sending "lighthouse" multicasts. Gazerbeam can be used to select specific "server" and/or IO device network.
- Only DHCP configuration makes sense (WiFi), static IP interface configuration is disabled.

Testing without gazerbeam
- Settings can be set from terminal.
- The "factory reset" must be done by erasing the flash with development tool (Platform IO).

Loading program to flash
- Use USB to TTL 3.3V converter (google "ESP32-cam pins", TX, RX, GND (and optionally 3.3V, if ESP32 is not powered from elsewhere))
- Before loading software connect GPIO pin 0 to GND
- Press reset button. Now board should be ready to receive program. After loading you need to disconnect GPIO 0 from ground and press reboot.

Files and folders
- platformio.ini - Settings for Visual Studio Code + Platform IO.

