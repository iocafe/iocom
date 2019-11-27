# Module: testplayer.py
from iocompython import Root, Signal
# import threading
# import time

class TestPlayer(object):
    def __init__(self, root, player_name, network_name):
        self.root = root
        self.player_name = player_name
        self.network_name = network_name
        self.userctrl = Signal(root, 'userctrl.*.' + player_name, network_name);

        self.force_x = 0.0
        self.force_y = 0.0
        self.velocity_x = 0.1
        self.velocity_y = 0.01
        self.x = 0.1
        self.y = 0.01
        self.rotation = 0
        self.engine_visible = 0
        self.resurrect_me = 0

    def run(self):
        state_bits, data = self.userctrl.get()
        if state_bits & 2:
            self.rotation = data[0]
            self.force_x = data[1]
            self.force_x = data[2]
            self.engine_visible = data[3]
            self.resurrect_me = data[4]
            print (data)

        # my_rotation, my_force_x, my_force_y, my_engine_visible, resurrect_me

        # print ("Hello, World " + self.network_name)


