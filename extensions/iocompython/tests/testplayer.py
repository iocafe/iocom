# Module: testplayer.py
# import threading
# import time

class TestPlayer(object):
    def __init__(self, player_name, network_name):
       self.player_name = player_name
       self.network_name = network_name

    def run(self):
        print ("Hello, World " + self.network_name)


