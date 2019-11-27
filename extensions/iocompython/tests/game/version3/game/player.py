import pyglet, math
from . import physicalobject, resources


class Player(physicalobject.PhysicalObject):
    """Physical object that responds to user input"""

    def __init__(self, *args, **kwargs):
        super(Player, self).__init__(img=resources.player_image, *args, **kwargs)

        # Create a child sprite to show when the ship is thrusting
        dummy = kwargs.pop('myworld')
        self.engine_sprite = pyglet.sprite.Sprite(img=resources.engine_image, *args, **kwargs)
        self.engine_sprite.visible = False

        # Set some easy-to-tweak constants
        self.thrust = 300.0
        self.rotate_speed = 200.0

    def mysetplayer(self, visible, x, y, rotation, engine_visible):
        super(Player, self).myset(visible, x, y, rotation)

        self.engine_sprite.rotation = self.rotation
        self.engine_sprite.x = self.x
        self.engine_sprite.y = self.y
        self.engine_sprite.visible = engine_visible

    def update(self, dt):
        # Do all the normal physics stuff
        super(Player, self).update(dt)

    def delete(self):
        # We have a child sprite which must be deleted when this object
        # is deleted from batches, etc.
        self.engine_sprite.delete()
        super(Player, self).delete()
