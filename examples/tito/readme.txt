notes 6.11.2019/pekka
Tito - IO controller using static IO board configuration. 

Tito is controller which runs multiple device IO networks, for example "iocafenet" for Pekka's devices and and "markkunet" for Markku.
This idea can used by cloud server, which need to serve several people, connect devices of one person together but still keep devices private to owner.

This program uses static configuration, and creates "iocafenet" and "markkunet" and devices in those at start at start. More practical approach is to
do this dynamically, so that networks and devices appear at controller as needed, but this example simplifies a few aspects of it.
Also signals in IO board are mapped by statically by signal address. This is also not optimal for versioning. Expanding form here to using "info" memory
block, which named signals, can allow running different versions of IO devices.

