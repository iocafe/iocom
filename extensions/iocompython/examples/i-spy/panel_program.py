import json
import os
from kivy.app import App
from kivy.config import ConfigParser
from kivy.uix.filechooser import FileChooserListView
from kivy.uix.filechooser import FileChooserIconView
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button
from kivy.uix.textinput import TextInput
from kivy.uix.widget import Widget
from kivy.core.window import Window
from kivy.uix.popup import Popup
from kivy.uix.label import Label
from kivy.metrics import dp

from item_heading import HeadingItem
from item_button import ButtonItem
from item import make_my_text_input
from iocompython import Root,bin2json, json2bin

class MyFileChooser(FileChooserIconView):
    def __init__(self, **kwargs):
        super(MyFileChooser, self).__init__(**kwargs)

#    def on_submit(self, selected, touch=None):
#        print("HERE")

    pass

class ProgramPanel(GridLayout):
# class ProgramPanel(Panel):
    def __init__(self, **kwargs):
        super(ProgramPanel, self).__init__(**kwargs)
        self.cols = 1

        g = GridLayout(cols = 2)
        g.height = 60
        g.size_hint_y = None
        title = HeadingItem()
        g.add_widget(title)
        self.my_title = title

        bg = GridLayout(cols = 3)
        bg.padding = [8, 6]

        b = Button(text='1. program')
        b.bind(on_release = self.my_select_block_dialog)
        bg.add_widget(b)
        self.my_block_button = b

        b = Button(text='read')
        b.bind(on_release = self.my_read_block_dialog)
        bg.add_widget(b)

        b = Button(text='write')
        bg.add_widget(b)

        g.add_widget(bg)
        self.add_widget(g)

        g = GridLayout(cols = 2)
        g.height = 40
        g.size_hint_y = None
        g.padding = [8, 6]

        self.current_path = '/coderoot'

        self.my_path = TextInput(text=self.current_path,  multiline=False, write_tab=False)
        g.add_widget(self.my_path)
        self.my_fname = TextInput(text='',  multiline=False, write_tab=False)
        g.add_widget(self.my_fname)

        self.add_widget(g)

        self.my_file_chooser = MyFileChooser(path=self.current_path, size_hint=(1, 1), dirselect=True)
        self.add_widget(self.my_file_chooser)

        self.my_file_chooser.bind(selection=lambda *x: self.my_select(x[1:]))
        self.my_file_chooser.bind(path=lambda *x: self.my_set_path(x[1:]))

    def my_select(self, path):
        # Only one selection
        if len(path) < 1:
            return;
        path = str(path[0][0])
        if len(path) > len(self.current_path) + 1:
            self.my_fname.text = path[len(self.current_path) + 1:] 
        else:
            self.my_fname.text = ''

    def my_set_path(self, path):
        if len(path) < 1:
            return;
        path = str(path[0])
        self.current_path = path
        self.my_path.text = self.current_path
        self.my_fname.text = ''

    def set_device(self, ioc_root, device_path):
        self.ioc_root = ioc_root
        self.device_path = device_path
        self.my_title.set_group_label("program", self.device_path, 1)

    # Write the flash program to micro-controller
    def my_button_pressed(self, i):
        print("here")

        path = self.my_file_chooser.selection[0]
        with open(path, mode='rb') as file: # b is important -> binary
            file_content = file.read()
            rval = self.ioc_root.setconf(self.device_path, file_content, select=1)
            print(rval)

    def delete(self):
        pass

    def run(self):
        pass

    def my_read_block_dialog(self, instance):
        grid = GridLayout()
        grid.cols = 1
        grid.spacing = [6, 6]
        grid.padding = [6, 6]

        my_path = os.path.join(self.my_path.text, self.my_fname.text)

        if os.path.isdir(my_path):
            my_path = os.path.join(my_path, "unnamed.dat")

        if os.path.isfile(my_path):
            warn = Label(text = "FILE ALREADY EXISTS, IF YOU SELECT DOWNLOAD\nTHE FILE WILL BE OVERWRITTEN", markup = True, halign="center")
            grid.add_widget(warn)

        pathinput = make_my_text_input(my_path)
        grid.add_widget(pathinput)
        self.pathinput = pathinput

        self.popup = popup = Popup(
            title='confirm persistent block download', content=grid)

        bg = GridLayout(cols = 2)
        bg.spacing = [6, 6]

        b = Button(text='close')
        b.height = 60
        b.size_hint_y = None
        b.bind(on_release = popup.dismiss)
        bg.add_widget(b)

        b = Button(text='download')
        b.height = 60
        b.size_hint_y = None
        b.bind(on_release = self.read_selected)
        bg.add_widget(b)

        grid.add_widget(bg)

        # all done, open the popup !
        popup.open()

    def read_selected(self, instance):
        # self.my_block_button.text = instance.text
        file_content = self.ioc_root.getconf(self.device_path, select=7)
        if file_content == None:
            return

        with open(self.pathinput.text, mode='wb') as file: # b is important -> binary
            file.write(file_content)
         
        self.popup.dismiss()


    def my_select_block_dialog(self, instance):
        button_list = ["1. program", "2. config", "3. defaults", "4. server key", "6. server cert", "7. cert chain", "8. root cert", "12. cust", "21. accounts"]

        grid = GridLayout()
        grid.cols = 2;
        grid.spacing = [6, 6]
        grid.padding = [6, 6]

        for button_text in button_list:
            b = Button(text=button_text)
            b.height = 60
            b.size_hint_y = None
            b.bind(on_release = self.block_selected)
            grid.add_widget(b)

        self.popup = popup = Popup(
            title='select persistent block', content=grid)

        # all done, open the popup !
        popup.open()

    def block_selected(self, instance):
        self.my_block_button.text = instance.text
        self.popup.dismiss()

class MyProgramButton(Button):
    def __init__(self, **kwargs):
        super(MyProgramButton, self).__init__(**kwargs)
        b = Button()
        # b.size_hint = (0.35, 1)
        self.add_widget(b)
        self.my_button = b

    def setup_button(self, text, signal_me):
        self.my_button.text = text
        if signal_me != None:
            self.my_button.bind(on_release = signal_me.my_button_pressed)

class MainApp(App):
    def build(self):
        self.root = ProgramPanel()
        self.root.set_device(Root('testwidget'), "gina2.iocafenet")
        return self.root

if __name__ == '__main__':
    MainApp().run()
