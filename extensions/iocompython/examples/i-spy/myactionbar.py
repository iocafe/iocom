from kivy.app import App

from kivy.uix.actionbar import ActionBar, ActionView, ActionPrevious, ActionOverflow, ActionGroup, ActionButton
# , ActionSeparator, ActionCheck, ActionItem, ActionLabel

class MyActionBar(ActionBar):
    def __init__(self, **kwargs):
        super(MyActionBar, self).__init__(**kwargs)

        self.register_event_type('on_button_press')
        self.devices = {}
        self.create_my_action_bar()

    def create_my_action_bar(self):
        self.clear_widgets()
        av = ActionView()
        self.my_action_view = av
        ap = ActionPrevious(title='my action bar', with_previous=False)
        av.add_widget(ap)

        av.add_widget(ActionOverflow())
        # av.add_widget(ActionButton(text='btn0', 
        #       icon='atlas://data/images/defaulttheme/audio-volume-high'))

        ap.title = "nuudeli"

        n_devices = len(self.devices);
        if n_devices > 0:
            ag = ActionGroup(text='window')
            self.add_my_button('signals', ag)
            self.add_my_button('memory', ag)
            self.add_my_button('configure', ag)
            self.add_my_button('program', ag)
            av.add_widget(ag)

            ag = ActionGroup(text='devices')
            for d in self.devices:
                self.add_my_button(d, ag)
            av.add_widget(ag)

        ag = ActionGroup(text='i-spy')
        self.add_my_button('disconnect', ag)
        self.add_my_button('close', ag)
        av.add_widget(ag)

        self.add_widget(av)
        av.use_separator = True

    def add_my_button(self, text, ag):
        b = ActionButton(text=text)
        b.bind (on_release=self.my_button_pressed)
        ag.add_widget(b)

    def on_button_press(self, *args):
        print("button press dispatched")

    def my_button_pressed(self, instance):
        self.dispatch('on_button_press', instance.text)

    def add_my_device(self, dev_path):
        # self.clear_widgets()
        self.devices[dev_path] = True;
        self.create_my_action_bar()

    def remove_my_device(self, dev_path):
        if dev_path in self.devices:
            del self.devices[dev_path]
        self.create_my_action_bar()

class MainApp(App):
    def build(self):
        self.root = MyActionBar(pos_hint={'top': 1})
        return self.root

if __name__ == '__main__':
    MainApp().run()