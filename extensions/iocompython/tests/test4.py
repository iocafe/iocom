from iocompython import Root, EndPoint, Signal
import ioterminal
import time

def main():
    ioterminal.start()

    root = Root('pythoncontrol')
    root.set_callback(root_callback)
    epoint = EndPoint(root, flags='socket')

    seven_segment = Signal(root, "seven_segment", "pekkanet")
    hor = Signal(root, "hor", "pekkanet")

    while (ioterminal.run(root)):
        time.sleep(0.01) 
        seven_segment.set(1, 0, 1, 0, 1, 0, 1)
#        print(seven_segment.get())
        print(hor.get())

#    seven_segment.delete()
#    hor.delete()

    root.delete()

def root_callback(x):
    print('callback ' + str(x))

if (__name__ == '__main__'): 
    main()

