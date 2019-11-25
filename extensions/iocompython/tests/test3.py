from iocompython import Root, MemoryBlock, Connection
import ioterminal
import time


def main():
    ioterminal.start()
    root = Root('TESTDEVICE', network_name='pekkanet')
    exp = MemoryBlock(root, 'source,auto', 'exp', nbytes=256)

    while (ioterminal.run(root)):
        time.sleep(0.01) 
    print("End.")

#    myinputs.delete()
   
    root.delete()

    print(x)

def new_mblk():
    print("new memory block")
    inputQueue = queue.Queue()

def del_mblk():
    print("del memory block")
    inputQueue = queue.Queue()


if (__name__ == '__main__'): 
    main()

