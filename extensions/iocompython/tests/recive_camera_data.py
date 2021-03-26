# Module: receive-camera-data.py
# Pull data from IO device's (gina1) camera trough a server (frank, etc)
# This example logs into "cafenet" device network run by server in local computer.
# User name "root" and password "pass" identify the client to server.
# The camtest must be accepted as valid at server (this can be done with i-spy)
# Client verifies validity of the server by acceptable certificate bundle 'myhome-bundle.crt'.

from iocompython import Root, Connection, MemoryBlock, BrickBuffer
import ioterminal
import time

# 9000 = select device number automatically
my_device_nr = 9000

def main():
    root = Root('camtest', device_nr=my_device_nr, security='certchainfile=myhome-bundle.crt')
    ioterminal.start(root)

    Connection(root, "127.0.0.1", "tls,down,dynamic", user='root.cafenet', password='pass')
    camera_buffer = BrickBuffer(root, "exp.gina1.cafenet", "imp.gina1.cafenet", "rec_", timeout=-1)
    camera_buffer.set_receive(True);

    while (ioterminal.run(root)):
        data = camera_buffer.get()
        if data != None:
            print(data)

        time.sleep(0.01)

    root.delete()

if (__name__ == '__main__'):
    main()
