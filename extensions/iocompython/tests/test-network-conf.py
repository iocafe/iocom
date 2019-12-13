from iocompython import Root, EndPoint, Signal, Stream, json2bin
import ioterminal 
import time

def get_network_conf(device_name, network_name):
    global root, callback_queue

    exp_mblk_path = 'conf_exp.' + device_name + '.' + network_name
    imp_mblk_path = 'conf_imp.' + device_name + '.' + network_name

    stream = Stream(root, frd = "frd_buf", tod = "tod_buf", exp = exp_mblk_path, imp = imp_mblk_path, select = 2)
    stream.start_read()

    while True:
        s = stream.run()
        if s != None:
            break
        time.sleep(0.01) 

    if s == 'completed':
        data = stream.get_data();
        print(data)

    else:
        print(s)

    stream.delete()


def set_network_conf(device_name, network_name):
    global root, callback_queue

    exp_mblk_path = 'conf_exp.' + device_name + '.' + network_name
    imp_mblk_path = 'conf_imp.' + device_name + '.' + network_name

    stream = Stream(root, frd = "frd_buf", tod = "tod_buf", exp = exp_mblk_path, imp = imp_mblk_path, select = 2)

    my_conf_bytes = str.encode("My dummy network configuration string")
    stream.start_write(my_conf_bytes)

    while True:
        s = stream.run()
        if s != None:
            break
        time.sleep(0.01) 

    if s == 'completed':
        print("success")

    else:
        print(s)

    stream.delete()


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

            # New device. This has a potential problem. 
            if event == 'new_device':
                #set_network_conf(device_name, network_name)
#                get_network_conf(device_name, network_name)
#                print(root.setconf(device_name + "." + network_name, str.encode("Dummy config data")))
                print(root.getconf(device_name + "." + network_name))

    root.delete()


if (__name__ == '__main__'): 
    main()

