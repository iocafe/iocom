set MYAPP=tito
set MYCODEROOT=c:/coderoot
set MYBIN=%MYCODEROOT%/bin/linux
set MYSCRIPTS=%MYCODEROOT%/iocom/scripts
set MYCONFIG=%MYCODEROOT%/iocom/examples/%MYAPP%/config

set MYDEVOS=win32

rem -----

python %MYSCRIPTS%/signals-to-c.py -a controller-static %MYCODEROOT%/iocom/examples/gina/config/signals/gina-signals.json -o %MYCONFIG%/include/gina-for-%MYAPP%.c
