# Module: asteroidplayer.py
from iocompython import Root, Signal
import re, math

class AsteroidPlayer(object):
    def __init__(self, root, player_name, network_name):
        self.root = root
        self.player_name = player_name
        tmp = re.findall(r'\d+', player_name)
        self.player_nr = int(tmp[0])
        self.network_name = network_name
        self.userctrl = Signal(root, 'userctrl.*.' + player_name, network_name);
        self.coords = Signal(root, 'coords.*.' + player_name, network_name)

        self.force_x = 0.0
        self.force_y = 0.0
        self.velocity_x = 0.3
        self.velocity_y = 0.05
        self.x = 100.0
        self.y = 100.0
        self.rotation = 0
        self.engine_visible = 0
        self.opacity = 0;

        self.timeline = 0;

    def run(self, dt):
        state_bits, data = self.userctrl.get()
        if state_bits & 2:
            self.rotation = data[0]
            self.force_x = data[1]
            self.force_y = data[2]
            self.engine_visible = data[3]
            self.resurrect_me = data[4]

        self.velocity_x += self.force_x * dt
        self.velocity_y += self.force_y * dt
        self.x += self.velocity_x * dt
        self.y += self.velocity_y * dt

        if self.x < 0:
            self.x = 0
            self.velocity_x = 0.1

        if self.x > 800:
            self.x = 800
            self.velocity_x = -0.1
        
        if self.y < 0:
            self.y = 0
            self.velocity_y = 0.1

        if self.y > 600:
            self.y = 600
            self.velocity_y = -0.1
        
        self.opacity += dt * 930.0;
        if self.opacity > 255:
                self.opacity = 255

        self.timeline += dt
        scale = 100
        # + 5 * math.sin(6.3 * self.timeline)

        # print(state_bits, data, self.player_name)

        return (self.player_nr, scale, self.x, self.y, self.rotation, self.engine_visible, 1, self.opacity)

    def set_coords(self, x):
        self.coords.set(x)
