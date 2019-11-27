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

        self.velocity_x = 0.1
        self.velocity_x = 0.1

    def run(self):
        state_bits, data = self.userctrl.get()
        if state_bits & 2:
            print(data)

        # my_rotation, my_force_x, my_force_y, my_engine_visible, resurrect_me

        # print ("Hello, World " + self.network_name)


