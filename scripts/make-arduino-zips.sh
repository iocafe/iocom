rm -R -f /coderoot/lib/arduino/eosal
rm -R -f /coderoot/lib/arduino/iocom
rm -R -f /coderoot/lib/arduino/pins
rm -R -f /coderoot/lib/arduino/deviceinfo
rm -R -f /coderoot/lib/arduino/devicedir
rm -R -f /coderoot/lib/arduino/lighthouse
rm -R -f /coderoot/lib/arduino/nodeconf
# rm -R -f /coderoot/lib/arduino/ioserver
# rm -R -f /coderoot/lib/arduino/selectwifi
# rm -R -f /coderoot/lib/arduino/gazerbeam

rm -f /coderoot/lib/arduino-zips/*.zip

cd ~/Arduino/libraries 
rm -R -f ~/Arduino/libraries/*-eosal
rm -R -f ~/Arduino/libraries/*-iocom
rm -R -f ~/Arduino/libraries/*-pins
rm -R -f ~/Arduino/libraries/*-deviceinfo
rm -R -f ~/Arduino/libraries/*-devicedir
rm -R -f ~/Arduino/libraries/*-lighthouse
rm -R -f ~/Arduino/libraries/*-nodeconf

/coderoot/iocom/scripts/copy-iocom-and-extras-for-arduino.sh
/coderoot/iocom/scripts/make-iocom-etc-arduino-zip-libraries.sh

