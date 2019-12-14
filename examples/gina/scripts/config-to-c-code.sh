export MYAPP=gina
export MYCODEROOT=/coderoot
export MYBIN=${MYCODEROOT}/bin/linux
export MYSCRIPTS=${MYCODEROOT}/iocom/scripts
export MYCONFIG=${MYCODEROOT}/iocom/examples/${MYAPP}/config

export MYHW=carol

python3 ${MYCODEROOT}/pins/scripts/pins-to-c.py ${MYCONFIG}/pins/${MYHW}/${MYAPP}-io.json -o ${MYCONFIG}/include/${MYHW}/${MYAPP}-io.c -s ${MYCONFIG}/signals/${MYAPP}-signals.json

${MYCODEROOT}/bin/linux/json --t2b -title ${MYCONFIG}/signals/${MYAPP}-signals.json ${MYCONFIG}/signals/${MYAPP}-signals.binjson
${MYCODEROOT}/bin/linux/json --b2t  ${MYCONFIG}/signals/${MYAPP}-signals.binjson ${MYCONFIG}/signals/${MYAPP}-signals-check.json
python3 ${MYCODEROOT}/eosal/scripts/bin-to-c.py -v ${MYAPP}_config ${MYCONFIG}/signals/${MYAPP}-signals.binjson -o ${MYCONFIG}/include/${MYHW}/${MYAPP}-info-mblk.c

python3 ${MYSCRIPTS}/signals-to-c.py ${MYCONFIG}/signals/${MYAPP}-signals.json -p ${MYCONFIG}/pins/${MYHW}/${MYAPP}-io.json -o ${MYCONFIG}/include/${MYHW}/${MYAPP}-signals.c

${MYCODEROOT}/bin/linux/json --t2b -title ${MYCONFIG}/network/${MYAPP}-network-defaults.json ${MYCONFIG}/network/${MYAPP}-network-defaults.binjson
${MYCODEROOT}/bin/linux/json --b2t  ${MYCONFIG}/network/${MYAPP}-network-defaults.binjson ${MYCONFIG}/network/${MYAPP}-network-defaults-check.json
python3 ${MYCODEROOT}/eosal/scripts/bin-to-c.py -v ${MYAPP}_network_defaults ${MYCONFIG}/network/${MYAPP}-network-defaults.binjson -o ${MYCONFIG}/include/${MYHW}/${MYAPP}-network-defaults.c

echo "*** Check that the output files have been generated (error checks are still missing)."
echo "*** You may need to recompile all C code since generated files in config/include/<hw> folder are not in compiler dependencies."
