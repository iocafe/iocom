# Module: asteroidservice.py
from iocompython import Root, EndPoint
import ioterminal
import asteroidapp
import queue, random, time

running_apps = {}

def main():
    global root, callback_queue

    root = Root('asteroid', security='certfile=bob.crt,keyfile=bob.key')
    root.queue_events()
    ioterminal.start(root)
    epoint = EndPoint(root, flags='tls,dynamic')
    random.seed(time.time())

    while (ioterminal.run(root)):
        e = root.wait_com_event(1000)
        if e != None:
            print(e)

            event = e[0]
            mblk_name = e[3]
            device_name = e[2]
            network_name = e[1]

            # New network, means a new game board
            if event == 'new_network':
                running_apps[network_name] = asteroidapp.start(root, network_name)

            # Close the game board
            if event == 'network_disconnected':
                a = running_apps.get(network_name, None)
                if a != None:
                    a[1].put('exit ' + network_name)
                    del running_apps[network_name]

            # Switch 'imp' and 'exp' memory blocks to manual synchronization
            if event == 'new_mblk':
                mblk_path = mblk_name + '.' + device_name + '.' + network_name
                if mblk_name == 'imp' or mblk_name == 'exp':
                    root.set_mblk_param(mblk_path, "auto", 0)

            # New player
            if event == 'new_device':
                a = running_apps.get(network_name, None)
                if a != None:
                    a[1].put('new_player ' + device_name)

            # Player dropped off
            if event == 'device_disconnected':
                a = running_apps.get(network_name, None)
                if a != None:
                    a[1].put('drop_player ' + device_name)

    root.delete()

if (__name__ == '__main__'): 
    main()
