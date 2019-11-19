from iocompython import Root, MemoryBlock, Connection
import threading
import queue
import time

def read_kbd_input(inputQueue):
    print('Ready for keyboard input:')
    while (True):
        input_str = input()
        inputQueue.put(input_str)


def handle_kbd_input(root, inputQueue):
    if inputQueue.qsize() > 0:
        input_str = inputQueue.get()
        if len(input_str) < 1:
            return True

        print("input_str = {}".format(input_str))

        if input_str == 'exit' or input_str[0] == 'x':
            return False

        parts = input_str.split()
        arg = "";
        if len(parts) > 1:
            arg = parts[1]

        if input_str[0] == 'm':
            print(root.print('memory_blocks', arg))

        if input_str[0] == 'e':
            print(root.print('end_points'))

        if input_str[0] == 'c':
            print(root.print('connections'))

    return True

def root_callback(x):
    print('callback ' + str(x))


def main():
    inputQueue = queue.Queue()

    inputThread = threading.Thread(target=read_kbd_input, args=(inputQueue,), daemon=True)
    inputThread.start()

#    root = Root('pythondevice', network_name='pekkanet')
    root = Root('pythondevice')
    root.set_callback(root_callback)
    # connection = Connection(root, "127.0.0.1", "socket")
    connection = Connection(root, "192.168.1.119", "socket")

    # myinputs = MemoryBlock(root, 'source,auto', 'MYINPUTS', 15, nbytes=256)

    while (handle_kbd_input(root, inputQueue)):

        # The rest of your program goes here.

        time.sleep(0.01) 
    print("End.")


#    end_point = EndPoint(root, "socket")

#    x = myinputs.set(10.1, 20, 30, [1.0, 2.1], 5, 'nasse', (1,2))

#    myinputs.delete()
    
    root.delete()

if (__name__ == '__main__'): 
    main()

