from iocompython import Root, MemoryBlock, Connection, json2bin
import ioterminal
import time

signal_conf = ('{'
  '"name": "gina",'
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
           '{"name": "", "type": "int", "array": 8}'
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
    myinputs = MemoryBlock(root, 'source,auto', 'exp', nbytes=256)

    data = json2bin(signal_conf)
    info = MemoryBlock(root, 'source,auto', 'info', nbytes=len(data))

    connection = Connection(root, "127.0.0.1", "socket")

    while (ioterminal.run(root)):
        time.sleep(0.01) 
        info.write(data)

    root.delete()



if (__name__ == '__main__'): 
    main()

