asteroid-client-pyglet
notes 26.4.2021/pekka

This is simple multiplayer asteroid game client to demonstrate dynamic IOCOM use in client - server environment.

Server runs one instance of asteroid-service, which serves multiple IO device networks. In asteroid, IO device network equals to one game world. Multiple players (= IO devices) can connect to each IO device network.

Data structures in server end are created and deleted as needed. Dynamic data structure is used also in client end for Python support, altough in real IO devices programmed in C it is more efficient to use static configuration.

Note 1: Besides of python, this example client depends on iocompython and on pyglet. iocompython needs to be compliled to create iocompython.so (linux) or iecompython.pyd (windows DLL). Refer to internet on Pyglet game GUI library installation. 

Note 2: To run this asteroidservice.py needs to be running on some computer, and IP address of the asteroid client needs to point that computer.

Note 3: This app uses synchronous receive and send. Receive makes sure that all inputs (number of objects and data for objects) are pair and from same transfer frame. Synchronous send ensures that all changes are sent as one TCP packet (as far as feasible).
