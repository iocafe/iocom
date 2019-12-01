# Module: asteroidservice.py
from iocompython import Root, EndPoint
import ioterminal
import asteroidapp
import queue, random, time

def main():
    global root, callback_queue
    ioterminal.start()

    root = Root('asteroid')
    root.queue_events()
    epoint = EndPoint(root, flags='socket,dynamic')
    random.seed(time.time())

    while (ioterminal.run(root)):
        e = root.wait_com_event(500)
        if e != None:
            print(e)
            if e[0] == 'new_network':
                asteroidapp.start(root, e[1])

    root.delete()

if (__name__ == '__main__'): 
    main()
