# copy-iocom-and-extras-for-arduino.py 21.4.2020/pekka
# Copies iocom and other libraries files needed for Arduino builds
# into /coderoot/lib/arduino/* directories. 

cd /coderoot/eosal/scripts
python3 ./copy-eosal-for-duino.py duino sam -o /coderoot/lib/arduino/eosal

cd /coderoot/iocom/scripts
python3 ./copy-iocom-for-duino.py -o /coderoot/lib/arduino/iocom

cd /coderoot/iocom/extensions/devicedir/scripts
python3 ./copy-devicedir-for-duino.py -o /coderoot/lib/arduino/devicedir

cd /coderoot/iocom/extensions/nodeconf/scripts
python3 ./copy-nodeconf-for-duino.py -o /coderoot/lib/arduino/nodeconf

cd /coderoot/iocom/extensions/ioserver/scripts
python3 ./copy-ioserver-for-duino.py -o /coderoot/lib/arduino/ioserver

cd /coderoot/iocom/extensions/lighthouse/scripts
python3 ./copy-lighthouse-for-duino.py -o /coderoot/lib/arduino/lighthouse

cd /coderoot/iocom/extensions/selectwifi/scripts
python3 ./copy-selectwifi-for-duino.py -o /coderoot/lib/arduino/selectwifi

cd /coderoot/iocom/extensions/gazerbeam/scripts
python3 ./copy-gazerbeam-for-duino.py -o /coderoot/lib/arduino/gazerbeam

cd /coderoot/pins/scripts
python3 ./copy-pins-for-duino.py duino -o /coderoot/lib/arduino/pins

