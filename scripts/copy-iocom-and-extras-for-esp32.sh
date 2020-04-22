# copy-iocom-and-extras-for-esp32.py 21.4.2020/pekka
# Copies iocom and other libraries files needed for ESP32 builds
# into /coderoot/lib/esp32/* directories. 

cd /coderoot/eosal/scripts
python3 ./copy-eosal-for-duino.py esp32 lwip mbedtls freertos -o /coderoot/lib/esp32/eosal

cd /coderoot/iocom/scripts
python3 ./copy-iocom-for-duino.py -o /coderoot/lib/esp32/iocom

cd /coderoot/iocom/extensions/devicedir/scripts
python3 ./copy-devicedir-for-duino.py -o /coderoot/lib/esp32/devicedir

cd /coderoot/iocom/extensions/nodeconf/scripts
python3 ./copy-nodeconf-for-duino.py -o /coderoot/lib/esp32/nodeconf

cd /coderoot/iocom/extensions/ioserver/scripts
python3 ./copy-ioserver-for-duino.py -o /coderoot/lib/esp32/ioserver

cd /coderoot/iocom/extensions/lighthouse/scripts
python3 ./copy-lighthouse-for-duino.py -o /coderoot/lib/esp32/lighthouse

cd /coderoot/iocom/extensions/selectwifi/scripts
python3 ./copy-selectwifi-for-duino.py -o /coderoot/lib/esp32/selectwifi

cd /coderoot/iocom/extensions/gazerbeam/scripts
python3 ./copy-gazerbeam-for-duino.py -o /coderoot/lib/esp32/gazerbeam

cd /coderoot/pins/scripts
python3 ./copy-pins-for-duino.py esp32 -o /coderoot/lib/esp32/pins

