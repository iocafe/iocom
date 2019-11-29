from iocompython import Root, EndPoint, Signal
import ioterminal
import testapp
import time
import queue

def main():
    global root, callback_queue
    ioterminal.start()

    root = Root('pythoncontrol')

    # callback_queue = queue.Queue()
    # root.set_callback(root_callback)
    epoint = EndPoint(root, flags='socket,dynamic')

    networks = {}

    while (ioterminal.run(root)):
        # if callback_queue.qsize() > 0:
        #    network_name = callback_queue.get()
        #    testapp.start(root, network_name)

        network_list = root.list_networks()
        
        for network_name in network_list:
            p = networks.get(network_name)
            if p == None:
                print ("new network " + network_name)
                networks[network_name] = 'uke'
                testapp.start(root, network_name)

        time.sleep(0.3) 

    root.delete()

# def root_callback(event_text, arg_text):
#    global callback_queue
#    print('callback: ' + event_text + ' ' + arg_text)

#    if event_text == 'new_network':
#        callback_queue.put(arg_text)

if (__name__ == '__main__'): 
    main()
