import pyglet
from . import util
# from iocompython import Root, Signal


class PhysicalObject(pyglet.sprite.Sprite):
    """A sprite with physical properties such as velocity"""

    def __init__(self, *args, **kwargs):
        self.myworld = kwargs.pop('myworld')
        super(PhysicalObject, self).__init__(*args, **kwargs)

        # In addition to position, we have velocity
        self.x = 0
        self.y = 0

        # And a flag to remove this object from the game_object list
        self.dead = False

    def myset(self, visible, x, y, rotation):
        self.dead = not visible
        self.x = x
        self.y = y
        self.rotation = rotation

    def update(self, dt):
 #       global mycoords
        """This method should be called every frame."""

        # Update position according to velocity and time
        # self.x += self.velocity_x * dt
        # self.y += self.velocity_y * dt

#        cc = self.myworld.get()
#        if cc != None:
#            if cc[0] & 2:
#                dcc = cc[1]
#                self.x = dcc[0]

        # Wrap around the screen if necessary
        self.check_bounds()

    def check_bounds(self):
        """Use the classic Asteroids screen wrapping behavior"""
        min_x = -self.image.width / 2
        min_y = -self.image.height / 2
        max_x = 800 + self.image.width / 2
        max_y = 600 + self.image.height / 2
        if self.x < min_x:
            self.x = max_x
        if self.y < min_y:
            self.y = max_y
        if self.x > max_x:
            self.x = min_x
        if self.y > max_y:
            self.y = min_y

    def collides_with(self, other_object):
        """Determine if this object collides with another"""

        # Calculate distance between object centers that would be a collision,
        # assuming square resources
        collision_distance = self.image.width / 2 + other_object.image.width / 2

        # Get distance using position tuples
        actual_distance = util.distance(self.position, other_object.position)

        return (actual_distance <= collision_distance)

    def handle_collision_with(self, other_object):
        self.dead = True
