set MYAPP=gina
set MYCODEROOT=c:/coderoot
set JSONTOOL=%MYCODEROOT%/win32/linux/json
set PINSTOC="python %MYCODEROOT%/pins/scripts/pins-to-c.py"
set BINTOC="python %MYCODEROOT%/eosal/scripts/bin-to-c.py"
set SIGNALSTOC="python %MYCODEROOT%/iocom/scripts/signals-to-c.py"
set MYCONFIG=%MYCODEROOT%/iocom/examples/%MYAPP%/config
set MYHW=carol
set MYINCLUDE=%MYCONFIG%/include/%MYHW%
set MYSIGNALS=%MYCONFIG%/signals/signals
set MYPINS=%MYCONFIG%/pins/%MYHW%/pins-io
set MYNETDEFAULTS=%MYCONFIG%/network/network-defaults

%PINSTOC% %MYPINS%.json -o %MYINCLUDE%/pins-io.c -s %MYSIGNALS%.json

%JSONTOOL% --t2b -title %MYSIGNALS%.json %MYSIGNALS%.binjson
%JSONTOOL% --b2t %MYSIGNALS%.binjson %MYSIGNALS%-check.json
%BINTOC% -v ioapp_signal_config %MYSIGNALS%.binjson -o %MYINCLUDE%/info-mblk.c

%SIGNALSTOC% %MYSIGNALS%.json -p %MYPINS%.json -o %MYINCLUDE%/signals.c

%JSONTOOL% --t2b -title %MYNETDEFAULTS%.json %MYNETDEFAULTS%.binjson
%JSONTOOL% --b2t  %MYNETDEFAULTS%.binjson %MYNETDEFAULTS%-check.json
%BINTOC% -v ioapp_network_defaults %MYNETDEFAULTS%.binjson -o %MYINCLUDE%/network-defaults.c

echo "*** Check that the output files have been generated (error checks are still missing)."
echo "*** You may need to recompile all C code since generated files in config/include/<hw> folder are not in compiler dependencies."
