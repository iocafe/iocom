# Module: asteroidapp.py
from iocompython import Root, Signal
import threading, queue
import time
from asteroidplayer import AsteroidPlayer 
from asteroidrock import AsteroidRock
from asteroidbullet import AsteroidBullet

class AsteroidApp(object):
    def __init__(self, root, network_name):
        self.root = root
        self.network_name = network_name

        self.rocks = []
        self.rocks.append(AsteroidRock())
        self.rocks.append(AsteroidRock())
        self.rocks.append(AsteroidRock())
        self.bullets = []

        self.mytimer = time.time()
        self.queue = queue.Queue()
        self.players = {}

    def run(self):
        global root

        timer_now = time.time()
        dt = timer_now - self.mytimer
        self.mytimer = timer_now

        if self.queue.qsize() > 0:
            myevent = self.queue.get().split()
            print (myevent)
            if myevent[0] == 'new_player':
                print ("new player " + myevent[1])
                self.players[myevent[1]] = AsteroidPlayer(self.root, myevent[1], self.network_name)

            if myevent[0] == 'drop_player':
                player = self.players.get(myevent[1], None)
                if player != None:
                    del self.players[myevent[1]]

            if myevent[0] == 'exit':
                return False

        asteroid_players = self.players.values()
        n_rocks = len(self.rocks)
        n_bullets = len(self.bullets)
        n_objects = len(asteroid_players) + n_rocks + n_bullets
        object_data = []
        for player in asteroid_players:
            c = player.run(dt)
            if c != None:
                object_data.append(c)
                shot = player.fires()
                if shot != None:
                    self.bullets.append(AsteroidBullet(shot))

        for i in range(n_rocks):
            c = self.rocks[i].run(dt)
            if c != None:
                object_data.append(c)

        remove_bullets = []
        for b in self.bullets:
            c = b.run(dt)
            if c != None:
                object_data.append(c)

            else:
                remove_bullets.append(b)

        for b in remove_bullets:
            self.bullets.remove(b)

        for player in asteroid_players:
            player.set_object_data(object_data)

        return True

def run_testapp(myapp):
    while (myapp.run()):
        time.sleep(0.03)

def start(proot, network_name):
    global root
    root = proot
    print("Asteroids started for " + network_name)
    myapp = AsteroidApp(root, network_name)
    q = myapp.queue
    asteroidAppThread = threading.Thread(target=run_testapp, args=(myapp,), daemon=True)
    asteroidAppThread.start()

    return (asteroidAppThread, q)
