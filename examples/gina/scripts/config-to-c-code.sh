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
