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

        ag = ActionGroup(text='devices')
        for d in self.devices:
            b = ActionButton(text=d)
            ag.add_widget(b)

        av.add_widget(ag)

        b = ActionButton(text='signals')
        av.add_widget(b)
        b = ActionButton(text='memory')
        av.add_widget(b)
        b = ActionButton(text='configure')
        av.add_widget(b)
        b = ActionButton(text='program')
        av.add_widget(b)

        b = ActionButton(text='close')
        b.bind (on_release=self.my_close_pressed)
        av.add_widget(b)

        ''' 
        for i in range(1, 5):
            b = ActionButton(text='btn{}'.format(i))
            av.add_widget(b)

        ag = ActionGroup(text='group')
        for i in range(5, 8):
            ag.add_widget(ActionButton(text='Btn{}'.format(i)))
        av.add_widget(ag)
        av.remove_widget(b)            
        '''

        self.add_widget(av)
        av.use_separator = True

    def on_button_press(self, *args):
        print("button press dispatched")

    def my_close_pressed(self, instance):
        self.dispatch('on_button_press', 'close')

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