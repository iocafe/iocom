from item import Item
from iocompython import Signal

class IoSignalItem(Item):
    def __init__(self, **kwargs):
        super(IoSignalItem, self).__init__(**kwargs)

    def delete(self):
        self.signal.delete()
        self.signal = None

    def setup_signal(self, ioc_root, signal_name, signal_addr, signal_type, n,  mblk_name, mblk_flags, device_path):
        flaglist = mblk_flags.split(',')
        self.my_up = "up" in flaglist
        self.my_down = "down" in flaglist

        description = signal_type 
        if n > 1:
            description += "[" + str(n) + "]"
        description += " at '" + mblk_name + "' address " + str(signal_addr) 

        self.setup_variable(ioc_root, signal_name, description, signal_type == "boolean" and n <= 1)
        self.signal = Signal(ioc_root, signal_name + "." + mblk_name + "." + device_path)

    def on_checkbox_modified(self, i):
        if self.my_up and not self.my_down:
            self.update_signal()

        else:            
            self.signal.set(self.my_checkbox.active)

    def update_signal(self):
        try:
            v = self.signal.get_ext(check_tbuf=self.my_up)
        except:
            print("mysettings.py: update_signal failed")
            v = (0, 0)

        new_state_bits = int(v[0])
        if new_state_bits != self.my_state_bits:
            self.my_state_bits = new_state_bits
            self.my_redraw_state_bits(None)

        if self.my_checkbox != None:
            checked = False
            try:
                checked = int(v[1]) != 0
            except:
                print("mysettings.py: Unable to get check box state")        

            if self.my_checkbox.active != checked:
                self.my_checkbox.active = checked

        if self.my_text != None:
            self.my_text.text = str(v[1])

    def my_user_input(self, instance):
        try:
            v = self.textinput.text
            self.popup.dismiss()
            self.signal.set(v)
        except:
            print("Conversion failed")


