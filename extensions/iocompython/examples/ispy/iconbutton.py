from kivy.uix.button import Button
from kivy.uix.image import Image

class IconButton(Button):
    def __init__(self, **kwargs):
        super(IconButton, self).__init__(**kwargs)
        self.background_color = [0,0,0,0]
        self.size_hint_x = None
        self.width = 60

    def set_image(self, name, groupdict, groupname):
        self.icon = Image(source ='resources/' + name + '.png')
        self.icon.size_hint_x = None
        self.icon.size_hint_y = None
        self.add_widget(self.icon)
        self.my_groupdict = groupdict
        self.my_groupname = groupname
        self.bind(size=self.reposition_image, pos=self.reposition_image)

    def reposition_image(self, root, *args):
        self.icon.pos = self.pos;
        self.icon.size = self.size;
