from iocompython import Root, MemoryBlock, Connection, Signal, json2bin
import ioterminal 
import time

def main():
    root = Root('mydevice', device_nr=3, network_name='iocafenet')
    root.queue_events()
    ioterminal.start(root)
    exp = MemoryBlock(root, 'upward', 'exp')
    imp = MemoryBlock(root, 'downward', 'imp')
    conf_exp = MemoryBlock(root, 'upward', 'conf_exp')
    conf_imp = MemoryBlock(root, 'downward', 'conf_imp')

    with open('test-device.json', 'r') as file:
        signal_conf = file.read()
    data = json2bin(signal_conf)

    info = MemoryBlock(root, 'upward,auto', 'info', nbytes=len(data))
    info.publish(data)
    
    connection = Connection(root, "127.0.0.1", "socket,upward")

    frd_cmd = Signal(root, "frd_cmd")
    tod_cmd = Signal(root, "tod_cmd")

    while (ioterminal.run(root)):
        imp.receive();
        conf_imp.receive();

        print(tod_cmd.get())

        exp.send();
        conf_exp.send();


#        hor.set(i)
#        i = i + 1
        time.sleep(0.03) 
    
    root.delete()

if (__name__ == '__main__'): 
    main()

