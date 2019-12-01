# Module: asteroidplayer.py
import re, random

class AsteroidRock(object):
    def __init__(self):
        self.x = random.random() * 800;
        self.y = random.random() * 600;
        self.rotation = random.random() * 360;
        self.opacity = 0;
        self.scale = 60

        self.velocity_x = 300.1 * (random.random() - 0.5)
        self.velocity_y = 300.1 * (random.random() - 0.5)
        self.rotation_speed = 1000.5 * (random.random() - 0.5)
        self.engine_visible = 0

    def run(self, dt):
#        self.velocity_x += self.force_x * dt
#        self.velocity_y += self.force_y * dt
        self.x += self.velocity_x * dt
        self.y += self.velocity_y * dt
        self.rotation += self.rotation_speed * dt
        if self.rotation < -180:
            self.rotation += 360;
        if self.rotation > 180:
            self.rotation -= 360;
        self.opacity += dt * 230.1;
        if self.opacity > 255:
            self.opacity = 255
        
        if self.x < -50:
            self.x += 900

        if self.x > 850:
            self.x -= 900
        
        if self.y < -50:
            self.y += 700

        if self.y > 650:
            self.y -= 700

        self.scale += dt * 10.0;
        if self.scale > 100:
            self.scale = 100
        
        return (0, self.scale, self.x, self.y, self.rotation, self.engine_visible, 2, self.opacity)
