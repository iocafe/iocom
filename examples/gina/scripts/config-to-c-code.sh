export MYAPP=gina
export MYCODEROOT=/coderoot
export JSONTOOL=${MYCODEROOT}/bin/linux/json
export PINSTOC="python3 ${MYCODEROOT}/pins/scripts/pins-to-c.py"
export BINTOC="python3 ${MYCODEROOT}/eosal/scripts/bin-to-c.py"
export SIGNALSTOC="python3 ${MYCODEROOT}/iocom/scripts/signals-to-c.py"
export MYCONFIG=${MYCODEROOT}/iocom/examples/${MYAPP}/config
export MYHW=carol
export MYINCLUDE=${MYCONFIG}/include/${MYHW}
export MYSIGNALS=${MYCONFIG}/signals/signals
export MYPINS=${MYCONFIG}/pins/${MYHW}/pins-io
export MYNETDEFAULTS=${MYCONFIG}/network/network-defaults

${PINSTOC} ${MYPINS}.json -o ${MYINCLUDE}/pins-io.c -s ${MYSIGNALS}.json

${JSONTOOL} --t2b -title ${MYSIGNALS}.json ${MYSIGNALS}.binjson
${JSONTOOL} --b2t ${MYSIGNALS}.binjson ${MYSIGNALS}-check.json
${BINTOC} -v ${MYAPP}_config ${MYSIGNALS}.binjson -o ${MYINCLUDE}/info-mblk.c

${SIGNALSTOC} ${MYSIGNALS}.json -p ${MYPINS}.json -o ${MYINCLUDE}/signals.c

${JSONTOOL} --t2b -title ${MYNETDEFAULTS}.json ${MYNETDEFAULTS}.binjson
${JSONTOOL} --b2t  ${MYNETDEFAULTS}.binjson ${MYNETDEFAULTS}-check.json
${BINTOC} -v ${MYAPP}_network_defaults ${MYNETDEFAULTS}.binjson -o ${MYINCLUDE}/network-defaults.c

echo "*** Check that the output files have been generated (error checks are still missing)."
echo "*** You may need to recompile all C code since generated files in config/include/<hw> folder are not in compiler dependencies."
