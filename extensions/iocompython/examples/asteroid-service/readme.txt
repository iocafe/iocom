asteroid-client-pyglet
notes 8.1.2020/pekka

This is simple multiplayer asteroid game client to demonstrate dynamic IOCOM use in client - server environment.

Server runs one instance of asteroid-service, which serves multiple IO device networks. In asteroid, IO device network equals to one game world. Multiple players (= IO devices) can connect to each IO device network.

Data structures in server end are created and deleted as needed. Dynamic data structure is used also in client end for Python support, altough in real IO devices programmed in C it is more efficient to use static configuration.

Note 1: Besides of python, this example client depends on iocompython and on pyglet. iocompython needs to be compliled to create iocompython.so (linux) or iecompython.pyd (windows DLL). Refer to internet on Pyglet game GUI library installation. 

Note 2: To run this asteroidserver.py needs to be running on some computer, and IP address of clients needs to point that.

