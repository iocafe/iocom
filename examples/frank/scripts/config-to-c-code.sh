export MYAPP=frank
export MYCODEROOT=/coderoot
export JSONTOOL=${MYCODEROOT}/bin/linux/json
export BINTOC="python3 ${MYCODEROOT}/eosal/scripts/bin-to-c.py"
export SIGNALSTOC="python3 ${MYCODEROOT}/iocom/scripts/signals-to-c.py"

export MYCONFIG=${MYCODEROOT}/iocom/examples/${MYAPP}/config
export MYINCLUDE=${MYCONFIG}/include
export MYSIGNALS=${MYCONFIG}/signals/signals
export MYNETDEFAULTS=${MYCONFIG}/network/network-defaults

${JSONTOOL} --t2b -title ${MYSIGNALS}.json ${MYSIGNALS}.binjson
${JSONTOOL} --b2t  ${MYSIGNALS}.binjson ${MYSIGNALS}-check.json
${SIGNALSTOC} -a controller-static ${MYSIGNALS}.json -o ${MYCONFIG}/include/signals.c
${BINTOC} -v ioapp_signal_config ${MYSIGNALS}.binjson -o ${MYINCLUDE}/info-mblk-binary.c

${JSONTOOL} --t2b -title ${MYNETDEFAULTS}.json ${MYNETDEFAULTS}.binjson
${JSONTOOL} --b2t  ${MYNETDEFAULTS}.binjson ${MYNETDEFAULTS}-check.json
${BINTOC} -v ioapp_network_defaults ${MYNETDEFAULTS}.binjson -o ${MYINCLUDE}/network-defaults.c

echo "*** Check that the output files have been generated (error checks are still missing)."
echo "*** You may need to recompile all C code since generated files in config/include folder are not in compiler dependencies."
