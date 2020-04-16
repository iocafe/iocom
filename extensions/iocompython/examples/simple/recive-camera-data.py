# Module: testapp.py
from iocompython import Root, BrickBuffer
# from iocompython import Signal
# import threading
# import time

my_device_nr = 9000 # 9000 = select device number automatically 

def main():
    ioterminal.start()
    root = Root('camtest', device_nr=my_device_nr, network_name='iocafenet', security='certchainfile=myhome-bundle.crt')

    myinputs = MemoryBlock(root, 'upward,auto', 'exp', nbytes=256)
 
    data = json2bin(signal_conf)
    info = MemoryBlock(root, 'upward,auto', 'info', nbytes=len(data))
    info.publish(data)
    
    connection = Connection(root, "127.0.0.1", "socket")
 
    hor = Signal(root, "hor", "pekkanet")
 
    i = 1
 
    while (ioterminal.run(root)):
        hor.set(i)
        i = i + 1
        time.sleep(0.01) 
    
    root.delete()
 
if (__name__ == '__main__'): 
    main()
