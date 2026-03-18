# Module: asteroidbullet.py
import re, random

class AsteroidBullet(object):
    def __init__(self, shot):
        self.x = shot[0];
        self.y = shot[1];
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
        
        if self.x < -10 or self.x > 810 or self.y < -10 or self.y > 610:
            return None
        
        return (0, self.scale, self.x, self.y, self.rotation, self.engine_visible, 3, self.opacity)
