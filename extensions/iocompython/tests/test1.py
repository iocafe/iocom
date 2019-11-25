from iocompython import Root, MemoryBlock, Connection
import ioterminal
import time


def main():
    ioterminal.start()
    root = Root('mydevice', network_name='pekkanet')
    myinputs = MemoryBlock(root, 'source,auto', 'exp', nbytes=256)
    connection = Connection(root, "127.0.0.1", "socket")

    while (ioterminal.run(root)):
        time.sleep(0.01) 

    root.delete()

if (__name__ == '__main__'): 
    main()

