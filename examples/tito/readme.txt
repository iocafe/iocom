notes 22.5.2020/pekka
Tito - IO controller using static IO board configuration. 

Tito demonstrates how to create more complex devices by compining multiple simple ones. To upper level Tito is seen (mostly) as one IO device 
which has multiple "minions". Example could be a robot where Tito is "the robot", and minions are for example motor control board, camera module,
etc. Minions can join to Tito either trough network or serial communication.

Tito used static address scheme (as oppose to dynamic). This means that IO memory map of minions is linked into Tito and known in advange.
Advantage of this approach is that it is simple and easy. Downside is that changing some minion's IO signal addressess requires recompilation 
of both tito and the minion.

