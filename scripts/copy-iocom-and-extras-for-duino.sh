# copy-iocom-and-extras-for-duino.py 8.1.2020/pekka
# Copies iocom and other libraries files needed for PlatformIO Arduino builds
# into /coderoot/lib/arduino-platformio/* directories. 

cd /coderoot/eosal/osbuild/arduino-platformio
python3 ./copy-eosal-for-platformio.py

cd /coderoot/iocom/osbuild/arduino-platformio
python3 ./copy-iocom-for-platformio.py

cd /coderoot/iocom/extensions/devicedir/osbuild/arduino-platformio
python3 ./copy-devicedir-for-platformio.py

cd /coderoot/iocom/extensions/nodeconf/osbuild/arduino-platformio
python3 ./copy-nodeconf-for-platformio.py

cd /coderoot/iocom/extensions/ioserver/osbuild/arduino-platformio
python3 ./copy-ioserver-for-platformio.py

cd /coderoot/iocom/extensions/lighthouse/osbuild/arduino-platformio
python3 ./copy-lighthouse-for-platformio.py

cd /coderoot/iocom/extensions/selectwifi/osbuild/arduino-platformio
python3 ./copy-selectwifi-for-platformio.py

cd /coderoot/iocom/extensions/gazerbeam/osbuild/arduino-platformio
python3 ./copy-gazerbeam-for-platformio.py

cd /coderoot/pins/scripts
python3 ./copy-pins-for-duino.py esp32 -o /coderoot/lib/arduino-platformio/pins

