from kivy.uix.button import Button
from item import Item

class ButtonItem(Item):
    def __init__(self, **kwargs):
        super(ButtonItem, self).__init__(**kwargs)
        b = Button()
        # b.size_hint = (0.35, 1)
        self.add_widget(b)
        self.my_button = b

    def setup_button(self, text, signal_me):
        self.my_button.text = text
        if signal_me != None:
            self.my_button.bind(on_release = signal_me.my_button_pressed)

