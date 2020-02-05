# copy-iocom-and-extras-for-platformio.py 8.1.2020/pekka
# Copies iocom and other libraries files needed for PlatformIO Arduino builds
# into /coderoot/lib/arduino-platformio/* directories. 

cd /coderoot/eosal/build/arduino-platformio
python3 ./copy-eosal-for-platformio.py

cd /coderoot/iocom/build/arduino-platformio
python3 ./copy-iocom-for-platformio.py

cd /coderoot/iocom/extensions/devicedir/build/arduino-platformio
python3 ./copy-devicedir-for-platformio.py

cd /coderoot/iocom/extensions/nodeconf/build/arduino-platformio
python3 ./copy-nodeconf-for-platformio.py

cd /coderoot/iocom/extensions/ioserver/build/arduino-platformio
python3 ./copy-ioserver-for-platformio.py

cd /coderoot/iocom/extensions/selectwifi/build/arduino-platformio
python3 ./copy-selectwifi-for-platformio.py

cd /coderoot/pins/build/arduino-platformio
python3 ./copy-pins-for-platformio.py

