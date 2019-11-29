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
    ioterminal.start()
    root = Root('mydevice', device_nr=3, network_name='pekkanet')
    myinputs = MemoryBlock(root, 'upward,auto', 'exp', nbytes=256)

    data = json2bin(signal_conf)
    info = MemoryBlock(root, 'upward,auto', 'info', nbytes=len(data))
    info.publish(data)
    
    connection = Connection(root, "127.0.0.1", "socket")

    hor = Signal(root, "hor", "pekkanet")

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

