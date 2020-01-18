export MYAPP=ioserver
export MYCODEROOT=/coderoot
export JSONTOOL=${MYCODEROOT}/bin/linux/json
export BINTOC="python3 ${MYCODEROOT}/eosal/scripts/bin-to-c.py"
export SIGNALSTOC="python3 ${MYCODEROOT}/iocom/scripts/signals-to-c.py"

export MYCONFIG=${MYCODEROOT}/iocom/extensions/${MYAPP}/config
export MYINCLUDE=${MYCONFIG}/include
export MYACCOUNTS=${MYCONFIG}/signals/accounts
export MYACCOUNTSDEFAULTS=${MYCONFIG}/network/default-accounts

${JSONTOOL} --t2b -title ${MYACCOUNTS}.json ${MYACCOUNTS}.binjson
${JSONTOOL} --b2t  ${MYACCOUNTS}.binjson ${MYACCOUNTS}-check.json
${SIGNALSTOC} -a controller-static ${MYACCOUNTS}.json -o ${MYCONFIG}/include/account-signals.c
${BINTOC} -v ioserver_account_config ${MYACCOUNTS}.binjson -o ${MYINCLUDE}/accounts-mblk-binary.c

${JSONTOOL} --t2b -title ${MYACCOUNTSDEFAULTS}.json ${MYACCOUNTSDEFAULTS}.binjson
${JSONTOOL} --b2t  ${MYACCOUNTSDEFAULTS}.binjson ${MYACCOUNTSDEFAULTS}-check.json
${BINTOC} -v ioserver_account_defaults ${MYACCOUNTSDEFAULTS}.binjson -o ${MYINCLUDE}/account-defaults.c

echo "*** Check that the output files have been generated (error checks are still missing)."
echo "*** You may need to recompile all C code since generated files in config/include folder are not in compiler dependencies."