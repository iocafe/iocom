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
#    myinputs = MemoryBlock(root, 'source,auto', 'MYINPUTS', 15, nbytes=256)
#    end_point = EndPoint(root, "socket", new_mblk=)

#    x = myinputs.set(10.1, 20, 30, [1.0, 2.1], 5, 'nasse', (1,2))

    while (True):
        if (inputQueue.qsize() > 0):
            input_str = inputQueue.get()
            print("input_str = {}".format(input_str))

            if (input_str == EXIT_COMMAND):
                print("Exiting serial terminal.")
                break

            # Insert your code here to do whatever you want with the input_str.

        # The rest of your program goes here.

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

