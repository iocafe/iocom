# make-iocom-etc-arduino-zip-libraries.py 25.1.2021/pekka
# Zips iocom and related  libraries files needed for Arduino builds
# from /coderoot/lib/arduino/* directories into .

cd /coderoot/eosal/scripts
python3 ./make_duino_zip_library.py eosal -i /coderoot/lib/arduino/eosal -o /coderoot/lib/arduino-zips
python3 ./make_duino_zip_library.py iocom -i /coderoot/lib/arduino/iocom -o /coderoot/lib/arduino-zips
python3 ./make_duino_zip_library.py devicedir -i /coderoot/lib/arduino/devicedir -o /coderoot/lib/arduino-zips
python3 ./make_duino_zip_library.py deviceinfo -i /coderoot/lib/arduino/deviceinfo -o /coderoot/lib/arduino-zips
python3 ./make_duino_zip_library.py nodeconf -i /coderoot/lib/arduino/nodeconf -o /coderoot/lib/arduino-zips
# python3 ./make_duino_zip_library.py ioserver -i /coderoot/lib/arduino/ioserver -o /coderoot/lib/arduino-zips
python3 ./make_duino_zip_library.py lighthouse -i /coderoot/lib/arduino/lighthouse -o /coderoot/lib/arduino-zips
# python3 ./make_duino_zip_library.py selectwifi -i /coderoot/lib/arduino/selectwifi -o /coderoot/lib/arduino-zips
# python3 ./make_duino_zip_library.py gazerbeam -i /coderoot/lib/arduino/gazerbeam -o /coderoot/lib/arduino-zips
python3 ./make_duino_zip_library.py pins -i /coderoot/lib/arduino/pins -o /coderoot/lib/arduino-zips

