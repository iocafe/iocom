# Module: asteroidapp.py
from iocompython import Root, Signal
import threading
import time
from asteroidplayer import AsteroidPlayer 
from asteroidrock import AsteroidRock

class AsteroidApp(object):
    def __init__(self, root, network_name):
        self.root = root
        self.network_name = network_name
        self.players = {}

        self.rocks = []
        self.rocks.append(AsteroidRock())
        self.rocks.append(AsteroidRock())
        self.rocks.append(AsteroidRock())

        self.mytimer = time.time()

#    def getnetwork_name(self):
#        return self.network_name

#    def setnetwork_name(self, network_name):
#        self.network_name = network_name

    def run(self):
        global root

        timer_now = time.time()
        dt = timer_now - self.mytimer
        self.mytimer = timer_now

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
                self.players[player_name] = AsteroidPlayer(self.root, player_name, self.network_name)

        # Remove players who dropped off
        remove_these = []
        for player_name in self.players:
            if player_name not in player_list:
                print ("player " + player_name + ' dropped off')
                remove_these.append(player_name)

        for player_name in remove_these:
            self.players.pop(player_name)

        asteroid_players = self.players.values()
        n_rocks = len(self.rocks)
        coords = [len(asteroid_players) + n_rocks]
        for player in asteroid_players:
            coords.append(player.run(dt))

        for i in range(n_rocks):
            coords.append(self.rocks[i].run(dt))

#        print(coords)

        for player in asteroid_players:
            player.set_coords(coords)

        return True

def run_testapp(myapp):
    while (myapp.run()):
        time.sleep(0.03)

def start(proot, network_name):
    global root
    root = proot
    print("Asteroids started for " + network_name)
    myapp = AsteroidApp(root, network_name)
    asteroidAppThread = threading.Thread(target=run_testapp, args=(myapp,), daemon=True)
    asteroidAppThread.start()

