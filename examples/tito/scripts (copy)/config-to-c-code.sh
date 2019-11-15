export MYAPP=tito
export MYCODEROOT=/coderoot
export MYBIN=${MYCODEROOT}/bin/linux
export MYSCRIPTS=${MYCODEROOT}/iocom/scripts
export MYCONFIG=${MYCODEROOT}/iocom/examples/${MYAPP}/config

python3 ${MYSCRIPTS}/signals-to-c.py -a controller-static ${MYCODEROOT}/iocom/examples/gina/config/signals/gina-signals.json -o ${MYCONFIG}/include/gina-for-${MYAPP}.c
