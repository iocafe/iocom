# Module: testapp.py
import threading
import time
from testplayer import TestPlayer 

class TestApp(object):
    def __init__(self, network_name):
        self.network_name = network_name
        self.players = {}

    def getnetwork_name(self):
        return self.network_name

    def setnetwork_name(self, network_name):
        self.network_name = network_name

    def run(self):
        # Make sure that our player list is up to date
        # player_list = network.list("mblk", "info", "device_name");
        player_list = ['mydevice3']

        if len(player_list) == 0:
            return False
        
        # Add new players
        for player_name in player_list:
            p = self.players.get(player_name)
            if p == None:
                print ("new player " + player_name)
                self.players[player_name] = TestPlayer(player_name, self.network_name)

        # Remove players who dropped off
        for player_name in self.players:
            if player_name not in player_list:
                print ("player " + player_name + ' dropped off')

        return True

def run_testapp(myapp):
    while (myapp.run()):
        time.sleep(0.3)

def start(network_name):
    print("new: starting application for " + network_name)
    myapp = TestApp(network_name)
    testAppThread = threading.Thread(target=run_testapp, args=(myapp,), daemon=True)
    testAppThread.start()

