# make-iocom-etc-arduino-zip-libraries.py 25.1.2021/pekka
# Zips iocom and related  libraries files needed for Arduino builds
# from /coderoot/lib/arduino/* directories into .

cd /coderoot/eosal/scripts
python3 ./make_duino_zip_library.py -i /coderoot/lib/arduino/eosal

python3 ./make_duino_zip_library.py -i /coderoot/lib/arduino/iocom

python3 ./make_duino_zip_library.py -i /coderoot/lib/arduino/devicedir

python3 ./make_duino_zip_library.py -i /coderoot/lib/arduino/deviceinfo

python3 ./make_duino_zip_library.py -i /coderoot/lib/arduino/nodeconf

python3 ./make_duino_zip_library.py -i /coderoot/lib/arduino/ioserver

python3 ./make_duino_zip_library.py -i /coderoot/lib/arduino/lighthouse

python3 ./make_duino_zip_library.py -i /coderoot/lib/arduino/selectwifi

python3 ./make_duino_zip_library.py -i /coderoot/lib/arduino/gazerbeam

python3 ./make_duino_zip_library.py -i /coderoot/lib/arduino/pins

