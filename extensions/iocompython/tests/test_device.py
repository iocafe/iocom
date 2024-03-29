# Python device with configuration transfer
from iocompython import Root, MemoryBlock, Connection, Signal, json2bin
import ioterminal 
import time

def main():
    device_name = 'mydevice'
    device_nr = 3
    full_device_name = device_name + str(device_nr)

    root = Root(device_name, device_nr=device_nr, network_name='cafenet', security='certchainfile=myhome-bundle.crt')
    root.queue_events()
    ioterminal.start(root)
    exp = MemoryBlock(root, 'up', 'exp')
    imp = MemoryBlock(root, 'down', 'imp')
    conf_exp = MemoryBlock(root, 'up', 'conf_exp')
    conf_imp = MemoryBlock(root, 'down', 'conf_imp')

    with open('test_device.json', 'r') as file:
        signal_conf = file.read()
    data = json2bin(signal_conf)

    info = MemoryBlock(root, 'up,auto', 'info', nbytes=len(data))
    info.publish(data)
    
    connection = Connection(root, "127.0.0.1", "tls,up")

    frd_cmd = Signal(root, "frd_cmd")
    tod_cmd = Signal(root, "tod_cmd")

    stream = root.initconf(full_device_name, flags = 'device')

    while (ioterminal.run(root)):
        imp.receive()
        conf_imp.receive()

        # If this device receives "what is congifuration" command, return it. Here massive "Hulabaloo" 
        # for any persistent block number.
        state_bits, cmd = frd_cmd.get_ext()
        if (state_bits & 2) and cmd == 1:
            print(root.setconf(full_device_name, str.encode("Hulabaloo"), flags = 'device'))

        # If this device receives "configure youself" command, just print the received configuration.
        state_bits, cmd = tod_cmd.get_ext()
        if (state_bits & 2) and cmd == 1:
            print(root.getconf(full_device_name, flags = 'device'))

        exp.send()
        conf_exp.send()
        time.sleep(0.03) 
    
    root.delete()

if (__name__ == '__main__'): 
    main()

