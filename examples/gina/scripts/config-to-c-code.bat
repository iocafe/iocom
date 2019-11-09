set MYAPP=gina
set MYCODEROOT=c:/coderoot
set MYBIN=%MYCODEROOT%/bin/linux
set MYSCRIPTS=%MYCODEROOT%/iocom/scripts
set MYCONFIG=%MYCODEROOT%/iocom/examples/%MYAPP%/config

set MYDEVOS=win32
set MYHW=carol

python %MYCODEROOT%/pins/scripts/pins-to-c.py %MYCONFIG%/pins/%MYHW%/%MYAPP%-io.json -o %MYCONFIG%/include/%MYHW%/%MYAPP%-io.c -s %MYCONFIG%/signals/%MYAPP%-signals.json

rem -----
%MYCODEROOT%/bin/%MYDEVOS%/json --t2b -title %MYCONFIG%/signals/%MYAPP%-signals.json %MYCONFIG%/signals/%MYAPP%-signals.binjson
%MYCODEROOT%/bin/%MYDEVOS%/json --b2t  %MYCONFIG%/signals/%MYAPP%-signals.binjson %MYCONFIG%/signals/%MYAPP%-signals-check.json

python %MYCODEROOT%/eosal/scripts/bin-to-c.py -v %MYAPP%_config %MYCONFIG%/signals/%MYAPP%-signals.binjson -o %MYCONFIG%/include/%MYHW%/%MYAPP%-info-mblk.c
python %MYSCRIPTS%/signals-to-c.py %MYCONFIG%/signals/%MYAPP%-signals.json -p %MYCONFIG%/pins/%MYHW%/%MYAPP%-io.json -o %MYCONFIG%/include/%MYHW%/%MYAPP%-signals.c
