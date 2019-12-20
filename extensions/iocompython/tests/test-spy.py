from iocompython import Root, MemoryBlock, Connection, Signal, json2bin
import ioterminal 
import time


def main():
    root = Root('spy', device_nr=10000, network_name='iocafenet', security='certchainfile=bob-bundle.crt')
    root.queue_events()
    ioterminal.start(root)
    
    connection = Connection(root, "127.0.0.1", "tls,downward,dynamic")

    while (ioterminal.run(root)):
        e = root.wait_com_event(1000)
    
    root.delete()

if (__name__ == '__main__'): 
    main()

