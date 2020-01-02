set MYAPP=tito
set MYCODEROOT=c:/coderoot
set MYBIN=%MYCODEROOT%/bin/linux
set MYSCRIPTS=%MYCODEROOT%/iocom/scripts
set MYCONFIG=%MYCODEROOT%/iocom/examples/%MYAPP%/config

set MYDEVOS=win32

rem -----

%MYCODEROOT%/bin/linux/json --t2b -title %MYCONFIG%/signals/accounts.json %MYCONFIG%/signals/%MYAPP%-accounts.binjson
%MYCODEROOT%/bin/linux/json --b2t  %MYCONFIG%/signals/%MYAPP%-accounts.binjson %MYCONFIG%/signals/accounts-check.json
python %MYSCRIPTS%/signals-to-c.py -a controller-static %MYCONFIG%/signals/accounts.json -o %MYCONFIG%/include/%MYAPP%-accounts.c
