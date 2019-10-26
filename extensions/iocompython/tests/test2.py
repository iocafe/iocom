from iocompython import Root, MemoryBlock, Connection
import threading
import queue
import time

def read_kbd_input(inputQueue):
    print('Ready for keyboard input:')
    while (True):
        input_str = input()
        inputQueue.put(input_str)

def main():
    EXIT_COMMAND = "exit"
    inputQueue = queue.Queue()

    inputThread = threading.Thread(target=read_kbd_input, args=(inputQueue,), daemon=True)
    inputThread.start()

    root = Root('TESTDEVICE', network_name='TESTNET')
    myinputs = MemoryBlock(root, 'source,auto', 'MYINPUTS', 15, nbytes=256)
#    connection = Connection(root, "127.0.0.1", "socket")

    x = myinputs.set(10.1, 20, 30, [1.0, 2.1], 5, 'nasse', (1,2))

    myinputs.delete()
    
    root.delete()

    print(x)

if (__name__ == '__main__'): 
    main()

