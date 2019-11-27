from iocompython import Root, EndPoint, Signal
import ioterminal
import testapp
import time

import queue

def main():
    global root, callback_queue
    ioterminal.start()

    root = Root('pythoncontrol')

    callback_queue = queue.Queue()
    root.set_callback(root_callback)
    epoint = EndPoint(root, flags='socket')

    network_name = 'pekkanet'
#    seven_segment = Signal(root, "seven_segment", network_name)
    coords = Signal(root, "coords", network_name)
#    x = 10;

    while (ioterminal.run(root)):
        if callback_queue.qsize() > 0:
            network_name = callback_queue.get()
            testapp.start(root, network_name)

#        time.sleep(0.1) 
#        seven_segment.set(1, 0, 1, 0, 1, 0, 1)

#        coords.set(1, x, x+30, 45, 1)
#        x = x + 1
#        if  x > 100:
#            x = 0
#        print(root.list_devices("pekkanet"))
#        print(seven_segment.get())

    root.delete()

def root_callback(event_text, arg_text):
    global callback_queue
    print('callback: ' + event_text + ' ' + arg_text)

    if event_text == 'new_network':
        callback_queue.put(arg_text)

if (__name__ == '__main__'): 
    main()

