MODBUS comparison
====================
Since the MODBUS is well known, it is easy to get the general idea by listing the main differences 
between the protocols:

* The IOCOM is event based, while MODBUS uses polling. This means that data is transferred when it 
  changes, not when it is queried.
* Data transfers are synchronized in way that it guarantees that other end of communication receives 
  changes in same order the changes are made. 
* IOCOM uses compression, flow control, etc and aims for performance. This makes it more complex than
  MODBUS. While most MODBUS users write their own protocol implementation, I do not expect this to work
  well with IOCOM. I recommend using reference implementation, at least as starting point, unless there
  is need to port the protocol to Python, Java, etc. 
* IOCOM requires full duplex serial communication (at least logically), half duplex will not do. 
  It cannot do “one master – multiple slaves” serial communication, every device needs it’s own serial
  wire. If those features are important, then use MODBUS. It was designed specifically for that scenario
  and does it well.
* IOCOM handles secure TLS connections natively.


190901, updated 22.5.2020/pekka
