import json
import os
import re
import time
from kivy.app import App
from kivy.config import ConfigParser
from kivy.uix.filechooser import FileChooserListView
from kivy.uix.filechooser import FileChooserIconView
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button
from kivy.uix.textinput import TextInput
from kivy.uix.widget import Widget
from kivy.uix.progressbar import ProgressBar
from kivy.core.window import Window
from kivy.uix.popup import Popup
from kivy.uix.label import Label
from kivy.metrics import dp

from item_heading import HeadingItem
from item_button import ButtonItem
from item import make_my_text_input
from error_popup import MyErrorPopup
from iocompython import Root, Stream, bin2json, json2bin

class MyFileChooser(FileChooserListView):
    def __init__(self, **kwargs):
        super(MyFileChooser, self).__init__(**kwargs)

#    def on_submit(self, selected, touch=None):
#        print("HERE")

    pass

class ProgramPanel(GridLayout):
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
        self.my_set_select()

        b = Button(text='read')
        b.bind(on_release = self.my_read_block_dialog)
        bg.add_widget(b)

        b = Button(text='write')
        b.bind(on_release = self.my_write_block_dialog)
        bg.add_widget(b)

        g.add_widget(bg)
        self.add_widget(g)

        g = GridLayout(cols = 2)
        g.height = 40
        g.size_hint_y = None
        g.padding = [8, 6]

        self.current_path = '/coderoot/packages'

        self.my_path = TextInput(text=self.current_path,  multiline=False, write_tab=False)
        g.add_widget(self.my_path)
        self.my_fname = TextInput(text='',  multiline=False, write_tab=False)
        g.add_widget(self.my_fname)

        self.add_widget(g)

        self.my_file_chooser = MyFileChooser(path=self.current_path, size_hint=(1, 1), dirselect=True)
        self.add_widget(self.my_file_chooser)

        self.my_file_chooser.bind(selection=lambda *x: self.my_select(x[1:]))
        self.my_file_chooser.bind(path=lambda *x: self.my_set_path(x[1:]))

        self.stream = None

    def my_set_select(self):
        extension_list = {"1.":".elf", "4.":".key", "6.":".crt", "7.":".crt", "8.":".crt"}

        self.my_select_nr = int(re.search(r'\d+', self.my_block_button.text).group())
        l = self.my_block_button.text.split()
        ext = extension_list.get(l[0], ".dat")
        self.my_ext = ext
        self.my_default_fname = l[1] + '-' + str(self.my_select_nr) + ext

    def my_select(self, path):
        # Only one selection
        if len(path) < 1:
            return
        path = str(path[0][0])
        if len(path) > len(self.current_path) + 1:
            self.my_fname.text = path[len(self.current_path) + 1:] 
        else:
            self.my_fname.text = ''

    def my_set_path(self, path):
        if len(path) < 1:
            return
        path = str(path[0])
        self.current_path = path
        self.my_path.text = self.current_path
        self.my_fname.text = ''

    def set_device(self, ioc_root, device_path):
        self.ioc_root = ioc_root
        self.device_path = device_path
        self.my_title.set_group_label("program", self.device_path, 1)

    def delete(self):
        if self.stream != None:
            self.stream.delete()
            self.stream = None

    def run(self):
        if self.stream != None:
            if self.writing:
                self.run_write()
            else:
                self.run_read()

    def my_read_block_dialog(self, instance):
        grid = GridLayout()
        grid.cols = 1
        grid.spacing = [6, 6]
        grid.padding = [6, 6]

        my_path = os.path.join(self.my_path.text, self.my_fname.text)

        if os.path.isdir(my_path):
            my_path = os.path.join(my_path, self.my_default_fname)

        if os.path.isfile(my_path):
            warn = Label(text = "FILE ALREADY EXISTS, IF YOU SELECT DOWNLOAD\nTHE FILE WILL BE OVERWRITTEN", markup = True, halign="center")
            grid.add_widget(warn)

        pathinput = make_my_text_input(my_path, True)
        grid.add_widget(pathinput)
        self.pathinput = pathinput

        self.popup = popup = Popup(
            title='confirm persistent block download', content=grid)

        popup.size_hint=(None, None)
        popup.size=(0.95 * Window.width, 250)

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

        pb = ProgressBar(value=0, max=1000)
        grid.add_widget(pb)
        popup.progress_bar = pb

        # all done, open the popup !
        popup.open()
        
    def read_selected(self, instance):
        # This would read file with one command, but we want progress bar, etc, so more 
        # complex way is used. 
        # file_content = self.ioc_root.getconf(self.device_path, select=self.my_select_nr)

        exp_mblk_path = 'conf_exp.' + self.device_path
        imp_mblk_path = 'conf_imp.' + self.device_path

        stream = Stream(self.ioc_root, exp = exp_mblk_path, imp = imp_mblk_path, select = self.my_select_nr)
        self.stream = stream
        self.writing = False

        stream.start_read()
        self.stream_prev_moved = -1

    def run_read(self):
        count = 16
        while True:
            s = self.stream.run()
            if s != None:
                break
            time.sleep(0.001)
            count = count - 1
            if count < 1:
                break

        moved = self.stream.bytes_moved() * 0.01
        if moved != self.stream_prev_moved:
            self.stream_prev_moved = moved
            self.popup.progress_bar.value = moved % 1000

        if s == None:
            return

        self.popup.dismiss()

        if s != 'completed':
            s = self.stream.status()
            p = MyErrorPopup()
            p.error_message('failed: ' + str(s))        
            self.stream.delete()
            self.stream = None
            return

        file_content = self.stream.get_data() 

        if s == 'completed' and file_content != None:
            # If this is key or certificate (text content), remove terminating '\0' character, if any.
            # We should not have it in PC text files
            if self.my_ext == '.key' or self.my_ext == '.crt':
                l = len(file_content)
                if file_content[l-1] == 0:
                    file_content = file_content[:l-1]

            with open(self.pathinput.text, mode='wb') as file: # b is important -> binary
                file.write(file_content)

            p = MyErrorPopup()
            p.success_message('file successfully read from the IO device') 

        else:
            s = self.stream.status()
            p = MyErrorPopup()
            p.error_message('failed: ' + str(s))        

        self.stream.delete()
        self.stream = None
        self.my_file_chooser._update_files()

    def my_write_block_dialog(self, instance):
        grid = GridLayout()
        grid.cols = 1
        grid.spacing = [6, 6]
        grid.padding = [6, 6]

        my_path = os.path.join(self.my_path.text, self.my_fname.text)

        if os.path.isdir(my_path):
            my_path = os.path.join(my_path, self.my_default_fname)

        pathinput = make_my_text_input(my_path, True)
        grid.add_widget(pathinput)
        self.pathinput = pathinput

        self.popup = popup = Popup(
            title='confirm file upload', content=grid)

        popup.size_hint=(None, None)
        popup.size=(0.95 * Window.width, 250)

        bg = GridLayout(cols = 2)
        bg.spacing = [6, 6]

        b = Button(text='close')
        b.height = 60
        b.size_hint_y = None
        b.bind(on_release = popup.dismiss)
        bg.add_widget(b)

        b = Button(text='upload')
        b.height = 60
        b.size_hint_y = None
        b.bind(on_release = self.write_selected)
        bg.add_widget(b)

        grid.add_widget(bg)

        pb = ProgressBar(value=0, max=1000)
        grid.add_widget(pb)
        popup.progress_bar = pb

        # all done, open the popup !
        popup.open()

    def write_selected(self, instance):
        try:
            with open(self.pathinput.text, mode='rb') as file: # b is important -> binary
                file_content = file.read()

                # If this is key or certificate (text content), append terminating '\0' character, if missing.
                # We should not have it in PC text files but need it on micro-controller
                if self.my_ext == '.key' or self.my_ext == '.crt':
                    l = len(file_content)
                    if file_content[l-1] != 0:
                        file_content = bytearray(file_content)
                        file_content.append(0)
                        file_content = bytes(file_content)

                # The self.ioc_root.setconf just writes the file content. The same is implemented with
                # start_rite(), run_write() in python to be able to display scroll bar.
                # rval = self.ioc_root.setconf(self.device_path, file_content, select=self.my_select_nr)
                self.start_write(file_content)

        except:
            p = MyErrorPopup()
            p.error_message('Unable to read file ' + self.pathinput.text)
        
    def start_write(self, file_content):
        exp_mblk_path = 'conf_exp.' + self.device_path
        imp_mblk_path = 'conf_imp.' + self.device_path

        stream = Stream(self.ioc_root, exp = exp_mblk_path, imp = imp_mblk_path, select = self.my_select_nr)
        self.stream = stream
        self.writing = True

        stream.start_write(file_content)
        self.stream_total_bytes = len(file_content)
        self.stream_prev_moved = -1
        self.file_transferred = False

    def run_write(self):
        if self.file_transferred:
            self.wait_for_transfer_status()
        else:
            self.transfer_file_to_device()

    def transfer_file_to_device(self):
        count = 16
        while True:
            s = self.stream.run()
            if s != None:
                break
            time.sleep(0.001)
            count = count - 1
            if count < 1:
                break

        moved = self.stream.bytes_moved()
        if moved != self.stream_prev_moved and self.stream_total_bytes > 0:
            self.stream_prev_moved = moved
            self.popup.progress_bar.value = 1000.0 * moved / self.stream_total_bytes

        if s == None:
            return

        if s == 'completed':
            self.file_transferred  = True

        else:
            self.popup.dismiss()
            p = MyErrorPopup()
            p.error_message('failed: ' + str(s))

    def wait_for_transfer_status(self):
        s = self.stream.status()
        if s == None:
            return

        self.popup.dismiss()

        if s == 'completed':
            p = MyErrorPopup()
            p.success_message('file written to the IO device')

        else:
            p = MyErrorPopup()
            p.error_message('failed: ' + str(s))

        self.stream.delete()
        self.stream = None

    def my_select_block_dialog(self, instance):
        button_list = ["1. program", "2. config", "3. defaults", "5. server key", "6. server cert", "7. root cert", "8. cert chain", "9. publish chain", "10. wifi select", "12. cust", "21. accounts"]

        grid = GridLayout()
        grid.cols = 2
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

        popup.size_hint=(None, None)
        popup.size=(0.90 * Window.width, 0.90 * Window.height)

        # all done, open the popup !
        popup.open()

    def block_selected(self, instance):
        self.my_block_button.text = instance.text
        self.my_set_select()
        self.popup.dismiss()

class MainApp(App):
    def build(self):
        self.root = ProgramPanel()
        self.root.set_device(Root('testwidget'), "gina2.iocafenet")
        return self.root

if __name__ == '__main__':
    MainApp().run()
