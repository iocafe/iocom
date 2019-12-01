import pyglet, math
from . import physicalobject, resources


class Player:
    """Physical object that responds to user input"""

    def __init__(self, *args, **kwargs):
        # super(Player, self).__init__(img=resources.player_image, *args, **kwargs)

        self.my_sprite = pyglet.sprite.Sprite(img=resources.asteroid_image, *args, **kwargs)

        # Create a child sprite to show when the ship is thrusting
        self.engine_sprite = pyglet.sprite.Sprite(img=resources.engine_image, *args, **kwargs)
        self.engine_sprite.visible = False

    # def set_sprite(self, visible, x, y, rotation, engine_visible):
    #    self.engine_sprite = pyglet.sprite.Sprite(img=resources.engine_image, *args, **kwargs)
    #    self.engine_sprite.visible = False


    def mysetplayer(self, visible, x, y, rotation, engine_visible):
#        super(Player, self).myset(visible, x, y, rotation)

        self.engine_sprite.rotation = rotation
        self.engine_sprite.x = x
        self.engine_sprite.y = y
        self.engine_sprite.visible = engine_visible

        self.my_sprite.x = x;
        self.my_sprite.y = y;
        self.my_sprite.rotation = rotation


#    def update(self, dt):
        # Do all the normal physics stuff
#        super(Player, self).update(dt)

    def delete(self):
        # We have a child sprite which must be deleted when this object
        # is deleted from batches, etc.
        self.engine_sprite.delete()
 #       super(Player, self).delete()
