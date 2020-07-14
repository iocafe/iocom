# copy_iocom_and_extras_for_esp32.py 21.4.2020/pekka
# Copies iocom and other libraries files needed for ESP32 builds
# into /coderoot/lib/esp32/* directories.

cd /coderoot/eosal/scripts
python3 ./copy_eosal_for_duino.py esp32 lwip mbedtls freertos -o /coderoot/lib/esp32/eosal

cd /coderoot/iocom/scripts
python3 ./copy_iocom_for_duino.py -o /coderoot/lib/esp32/iocom

cd /coderoot/iocom/extensions/devicedir/scripts
python3 ./copy_devicedir_for_duino.py -o /coderoot/lib/esp32/devicedir

cd /coderoot/iocom/extensions/deviceinfo/scripts
python3 ./copy_deviceinfo_for_duino.py -o /coderoot/lib/esp32/deviceinfo

cd /coderoot/iocom/extensions/nodeconf/scripts
python3 ./copy_nodeconf_for_duino.py -o /coderoot/lib/esp32/nodeconf

cd /coderoot/iocom/extensions/ioserver/scripts
python3 ./copy_ioserver_for_duino.py -o /coderoot/lib/esp32/ioserver

cd /coderoot/iocom/extensions/lighthouse/scripts
python3 ./copy_lighthouse_for_duino.py -o /coderoot/lib/esp32/lighthouse

cd /coderoot/iocom/extensions/selectwifi/scripts
python3 ./copy_selectwifi_for_duino.py -o /coderoot/lib/esp32/selectwifi

cd /coderoot/iocom/extensions/gazerbeam/scripts
python3 ./copy_gazerbeam_for_duino.py -o /coderoot/lib/esp32/gazerbeam

cd /coderoot/pins/scripts
python3 ./copy_pins_for_duino.py esp32 -o /coderoot/lib/esp32/pins

cd /coderoot/eosal/dependencies/eosal_jpeg/scripts
python3 ./copy_eosal_jpeg_for_duino.py esp32 -o /coderoot/lib/esp32/eosal_jpeg

