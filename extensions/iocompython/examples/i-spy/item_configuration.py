from item import Item

class ConfigurationItem(Item):
    def __init__(self, **kwargs):
        super(ConfigurationItem, self).__init__(**kwargs)

    def delete(self):
        pass

    def setup_setting(self, ioc_root, setting_name, dict, value_d, value, description):
        self.setup_variable(ioc_root, setting_name, description, False)
        self.my_dict = dict

        if value == None:
            self.set_value(value_d, 0)

        else:            
            self.set_value(value, 2) # 2 = state bit "connected" 

    def set_value(self, value, state_bits):
        if state_bits != self.my_state_bits:
            self.my_state_bits = state_bits
            self.my_redraw_state_bits(None)

        if self.my_checkbox != None:
            checked = False
            try:
                checked = int(value) != 0
            except:
                print("Unable to get check box state")        

            if self.my_checkbox.active != checked:
                self.my_checkbox.active = checked

        if self.my_text != None:
            self.my_text.text = str(value)

        self.my_dict[self.my_label_text] = value

    def on_checkbox_modified(self, i):
        self.my_dict[self.my_label_text] = self.my_checkbox.active

    def update_signal(self):
        pass

    def my_user_input(self, instance):
        try:
            v = self.textinput.text
            self.popup.dismiss()
            self.my_text.text = v
            self.my_dict[self.my_label_text] = v
        except:
            print("Conversion failed")

