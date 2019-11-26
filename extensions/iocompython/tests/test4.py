from iocompython import Root, EndPoint, Signal
import ioterminal
import testapp
import time

def main():
    ioterminal.start()

    root = Root('pythoncontrol')
    root.set_callback(root_callback)
    epoint = EndPoint(root, flags='socket')

    network_name = 'pekkanet'
    seven_segment = Signal(root, "seven_segment", network_name)
    hor = Signal(root, "hor", network_name)

    while (ioterminal.run(root)):
        time.sleep(0.01) 
        seven_segment.set(1, 0, 1, 0, 1, 0, 1)
#        print(seven_segment.get())
#        print(hor.get())

#    seven_segment.delete()
#    hor.delete()

    root.delete()

def root_callback(event_text, arg_text):
    print('callback: ' + event_text + ' ' + arg_text)

    if event_text == 'new_network':
        testapp.start(arg_text)

if (__name__ == '__main__'): 
    main()

