# Module: recive-camera-data.py
# Pull data from IO device's (gina1) camera trough a server (frank, etc) 

from iocompython import Root, Connection, MemoryBlock, BrickBuffer
import ioterminal 
import time


# 9000 = select device number automatically 
my_device_nr = 9000 

def main():
    root = Root('camtest', device_nr=my_device_nr, network_name='iocafenet', security='certchainfile=myhome-bundle.crt')
#    root.queue_events()
    ioterminal.start(root)

    # Memory blocks are created explisitely (not dynamically) to keep code simple.
    exp = MemoryBlock(root, 'up,auto', 'exp.gina1.iocafenet')
    imp = MemoryBlock(root, 'down,auto', 'imp.gina1.iocafenet')
 
    connection = Connection(root, "127.0.0.1", "tls,down", user='ispy.iocafenet', password='pass')
    camera_buffer = BrickBuffer(root, "rec_", "exp.gina1.iocafenet", "imp.gina1.iocafenet")

    while (ioterminal.run(root)):
        data = camera_buffer.get()
        if data != None:
            print(data)

        time.sleep(0.01) 
    
    root.delete()
 
if (__name__ == '__main__'): 
    main()
