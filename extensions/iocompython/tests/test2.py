from iocompython import Root, MemoryBlock, Connection
import ioterminal
import time


def root_callback(x):
    print('callback ' + str(x))


def main():
    ioterminal.start()

#    root = Root('pythondevice', network_name='pekkanet')
    root = Root('pythondevice')
    root.set_callback(root_callback)
    connection = Connection(root, "127.0.0.1", "socket")
    # connection = Connection(root, "192.168.1.119", "socket")

    # myinputs = MemoryBlock(root, 'source,auto', 'MYINPUTS', 15, nbytes=256)

    while (ioterminal.run(root)):
        # The rest of your program goes here.

        time.sleep(0.01) 
    print("End.")


#    end_point = EndPoint(root, "socket")

#    x = myinputs.set(10.1, 20, 30, [1.0, 2.1], 5, 'nasse', (1,2))

#    myinputs.delete()
    
    root.delete()

if (__name__ == '__main__'): 
    main()

