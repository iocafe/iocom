# Module: ioterminal.py
import threading
import queue

def read_kbd_input(inputQueue):
    print('Ready for keyboard input:')
    while (True):
        input_str = input()
        inputQueue.put(input_str)

def run(root):
    global inputQueue
    if inputQueue.qsize() > 0:
        input_str = inputQueue.get()
        if len(input_str) < 1:
            return True

        if input_str == 'exit' or input_str[0] == 'x' or input_str[0] == 'q':
            return False

        parts = input_str.split()
        arg = "";
        if len(parts) > 1:
            arg = parts[1]

        arg2 = "";
        if len(parts) > 2:
            arg2 = parts[2]

        if input_str[0] == 'm':
            print(root.print('memory_blocks', arg, arg2))

        if input_str[0] == 'e':
            print(root.print('end_points'))

        if input_str[0] == 'c':
            print(root.print('connections'))

        if input_str[0] == 'd':
            print(root.print('signals', arg, arg2))

    return True

def start():
    global inputQueue
    inputQueue = queue.Queue()
    inputThread = threading.Thread(target=read_kbd_input, args=(inputQueue,), daemon=True)
    inputThread.start()

