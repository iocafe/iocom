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

        ap.title = "i-spy"

        n_devices = len(self.devices);
        if n_devices > 0:
            ag = ActionGroup(text='window')
            self.add_my_button('signals', None, ag)
            self.add_my_button('memory', None, ag)
            self.add_my_button('configure', None, ag)
            self.add_my_button('program', None, ag)
            av.add_widget(ag)

            ag = ActionGroup(text='devices')
            for d in self.devices:
                s = d.split('.', 1)
                txt = s[0] + '[size=14]\n[color=9090FF]' + s[1] + '[/color][/size]'
                self.add_my_button(txt, d, ag)
            av.add_widget(ag)

        ag = ActionGroup(text='i-spy')
        self.add_my_button('disconnect', None, ag)
        self.add_my_button('close', None, ag)
        av.add_widget(ag)

        self.add_widget(av)
        av.use_separator = True

    def add_my_button(self, text, myaction, ag):
        b = ActionButton(text=text, markup = True, halign="center")
        b.bind (on_release=self.my_button_pressed)
        if myaction != None:
            b.myaction = myaction
        else:
            b.myaction = text            
        ag.add_widget(b)

    def on_button_press(self, *args):
        pass
        # print("button press dispatched")

    def my_button_pressed(self, instance):
        self.dispatch('on_button_press', instance.myaction)

    def add_my_device(self, device_path):
        self.devices[device_path] = True;
        self.create_my_action_bar()

    def remove_my_device(self, device_path):
        if device_path in self.devices:
            del self.devices[device_path]
        self.create_my_action_bar()

class MainApp(App):
    def build(self):
        self.root = MyActionBar(pos_hint={'top': 1})
        return self.root

if __name__ == '__main__':
    MainApp().run()