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

            # New network, means a new game board
            if e[0] == 'new_network':
                asteroidapp.start(root, e[1])

            # Switch 'inp' and 'exp' memory blocks to manual synchronization
            # Register new player on receipt of the info block
            if e[0] == 'mblk_as_source' or e[0] == 'mblk_as_target':
                mblk_name = e[3]
                mblk_path = mblk_name + '.' + e[2] + '.' + e[1]
                #if mblk_name == 'imp' or mblk_name == 'exp':
                #    root.set_mblk_param(root, mblk_path, "auto", 0)
                #if mblk_name == 'info':
                #    new_player()

    root.delete()

if (__name__ == '__main__'): 
    main()
