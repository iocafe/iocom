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
    root = Root('mydevice', network_name='pekkanet')
    myinputs = MemoryBlock(root, 'source,auto', 'exp', nbytes=256)
    connection = Connection(root, "127.0.0.1", "socket")

    while (ioterminal.run(root)):
        time.sleep(0.01) 

    root.delete()

    print(signal_conf)

    print(json2bin(signal_conf))

if (__name__ == '__main__'): 
    main()

