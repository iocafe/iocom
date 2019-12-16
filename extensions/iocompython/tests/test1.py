from iocompython import Root, MemoryBlock, Connection, Signal, json2bin
import ioterminal 
import time

signal_conf = ('{'
  '"mblk": ['
  '{'
    '"name": "exp",'
    '"groups": ['
       '{'
         '"name": "control",'
         '"signals": ['
           '{"name": "ver", "type": "short"},'
           '{"name": "hor"}'
         ']'
       '}'
    ']'
  '},'
  '{'
    '"name": "imp",'
    '"groups": ['
      '{'
        '"name": "world",'
        '"signals": ['
           '{"name": "ss", "type": "int", "array": 8}'
         ']'
      '},'
      '{'
        '"name": "me",'
        '"signals": ['
          '{"name": "x", "type": "short"},'
          '{"name": "y"}'
        ']'
      '}'
    ']'
  '}'
  ']'
'}')



def main():
    root = Root('mydevice', device_nr=10000, network_name='iocafenet')
    root.queue_events()
    ioterminal.start(root)
    myinputs = MemoryBlock(root, 'upward,auto', 'exp', nbytes=256)

    data = json2bin(signal_conf)
    info = MemoryBlock(root, 'upward,auto', 'info', nbytes=len(data))
    info.publish(data)
    
    connection = Connection(root, "127.0.0.1", "socket,upward")

    hor = Signal(root, "hor")

    i = 1

    while (ioterminal.run(root)):

        hor.set(i)
        i = i + 1
        time.sleep(0.01) 
    
# connection.delete is needed, should not be
#    hor.delete()
#    myinputs.delete()
#    info.delete()
#    connection.delete()
    root.delete()

if (__name__ == '__main__'): 
    main()

