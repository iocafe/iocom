import pyglet
from . import resources


class PhysicalObject:
    def __init__(self, *args, **kwargs):
        self.image_nr = 0

    def set(self, scale, x, y, rotation, engine_visible, image_nr, opacity, batch):
        if x > 300:
            image_nr = 1

        else:
            image_nr = 2

        if image_nr != self.image_nr:
            if self.image_nr != 0:
                self.my_sprite.delete()
                self.engine_sprite.delete()

            self.image_nr = image_nr

            if image_nr == 1:
                img=resources.player_image
            if image_nr == 2:
                img=resources.asteroid_image
            if image_nr == 3:
                img=resources.bullet_image

            self.my_sprite = pyglet.sprite.Sprite(img=img, batch=batch)
            self.engine_sprite = pyglet.sprite.Sprite(img=resources.engine_image, batch=batch)

        self.my_sprite.x = x;
        self.my_sprite.y = y;
        self.my_sprite.rotation = rotation
        self.my_sprite.scale = 0.01 * scale

        self.engine_sprite.rotation = rotation
        self.engine_sprite.x = x
        self.engine_sprite.y = y
        self.engine_sprite.visible = engine_visible

    def delete(self):
        if self.image_nr != 0:
            self.my_sprite.delete()
            self.engine_sprite.delete()
            
        self.image_nr = 0            
