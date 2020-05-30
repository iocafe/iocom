export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/coderoot/python/lib

cd /coderoot/python/bin
. ./debugpython/bin/activate

export PYTHONPATH=/coderoot/bin/linux/debug:/coderoot/python/lib/python3.9
/home/john/Qt/Tools/QtCreator/bin/qtcreator

