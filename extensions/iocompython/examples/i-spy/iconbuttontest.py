from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.lang import Builder
from kivy.uix.button import Button
from kivy.uix.image import Image

class MyIconButton(Button):
    def __init__(self, **kwargs):
        super(MyIconButton, self).__init__(**kwargs)
        self.size_hint_x = None
        self.width = 60
        self.background_color = [0,0,0,0]

    def set_image(self):
        self.icon = Image(source ='atlas://data/images/defaulttheme/audio-volume-high')
        self.icon.size_hint_x = None
        self.icon.size_hint_y = None
        self.add_widget(self.icon)
        self.bind(size=self.reposition_image, pos=self.reposition_image)

    def reposition_image(self, root, *args):
        self.icon.pos = self.pos;
        self.icon.size = self.size;

class ButtonsApp(App, BoxLayout):
    def build(self):
        self.root = MyIconButton()
        self.root.set_image()
        return self.root


if __name__ == "__main__":
    ButtonsApp().run()