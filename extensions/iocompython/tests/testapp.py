# Module: testapp.py
from iocompython import Root, Signal
import threading
import time
from testplayer import TestPlayer 

class TestApp(object):
    def __init__(self, root, network_name):
        self.root = root
        self.network_name = network_name
        self.players = {}

    def getnetwork_name(self):
        return self.network_name

    def setnetwork_name(self, network_name):
        self.network_name = network_name

    def run(self):
        global root
        # Make sure that our player list is up to date
        player_list = root.list_devices(self.network_name)
        if player_list == None:
            print('network closed')
            return False
        
        # Add new players
        for player_name in player_list:
            p = self.players.get(player_name)
            if p == None:
                print ("new player " + player_name)
                self.players[player_name] = TestPlayer(self.root, player_name, self.network_name)

        # Remove players who dropped off
        remove_these = []
        for player_name in self.players:
            if player_name not in player_list:
                print ("player " + player_name + ' dropped off')
                remove_these.append(player_name)

        for player_name in remove_these:
            self.players.pop(player_name)

        test_players = self.players.values()
        for tplayer in test_players:
            tplayer.run()

        return True

def run_testapp(myapp):
    while (myapp.run()):
        time.sleep(0.3)

def start(proot, network_name):
    global root
    root = proot
    print("new: starting application for " + network_name)
    myapp = TestApp(root, network_name)
    testAppThread = threading.Thread(target=run_testapp, args=(myapp,), daemon=True)
    testAppThread.start()

