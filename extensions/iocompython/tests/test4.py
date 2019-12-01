from iocompython import Root, EndPoint
import ioterminal
import testapp
# import time
import queue

def main():
    global root, callback_queue
    ioterminal.start()

    root = Root('pythoncontrol')
    root.queue_events()
    epoint = EndPoint(root, flags='socket,dynamic')

    while (ioterminal.run(root)):
        e = root.wait_com_event(500)
        if e != None:
            if e[0] == 'new_network':
                testapp.start(root, e[1])

    root.delete()

if (__name__ == '__main__'): 
    main()
