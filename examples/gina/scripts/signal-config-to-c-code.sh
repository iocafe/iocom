export MYCODEROOT=/coderoot
export MYBIN=${MYCODEROOT}/bin/linux
export MYSCRIPTS=${MYCODEROOT}/iocom/scripts
export MYCONFIG=${MYCODEROOT}/iocom/examples/gina/config

export MYHW=carol

${MYCODEROOT}/bin/linux/json --t2b -title ${MYCONFIG}/signals/gina-signals.json ${MYCONFIG}/signals/gina-signals.json-bin
python3 ${MYSCRIPTS}/signals-to-c.py ${MYCONFIG}/signals/gina-signals.json -p ${MYCONFIG}/pins/${MYHW}/gina-io.json -o ${MYCONFIG}/include/${MYHW}/gina-signals.c
