from iocompython import Root, EndPoint, Signal, Stream, json2bin
import ioterminal 
import time

def network_conf(mblk_name, device_name, network_name):
    global root, callback_queue

    exp_mblk_path = 'conf_exp.' + device_name + '.' + network_name
    imp_mblk_path = 'conf_imp.' + device_name + '.' + network_name

    frd = Stream(root, read = "frd_buf", exp = exp_mblk_path, imp = imp_mblk_path, select = 'netconf')

    while True;
        rval, bytedata = frd.read()
        print('Stream: ', rval, bytedata)

        if rval == 'completed':
            break

        if rval != 'ok':
            break
        
        time.sleep(0.01) 

    frd.delete()


def main():
    global root, callback_queue

    root = Root('netconftest')
    root.queue_events()
    ioterminal.start(root)
    epoint = EndPoint(root, flags='socket,dynamic')

    while (ioterminal.run(root)):
        e = root.wait_com_event(1000)
        if e != None:
            print(e)

            event = e[0]
            mblk_name = e[3]
            device_name = e[2]
            network_name = e[1]

            if event == 'new_mblk':
                if mblk_name == 'conf_exp':
                    network_conf(device_name, network_name)

            # New device
            # if event == 'new_device':
            #    network_conf(device_name, network_name)

    root.delete()


if (__name__ == '__main__'): 
    main()

