from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.graphics import Color, Rectangle, Line
from kivy.uix.widget import Widget
from kivy.uix.boxlayout import BoxLayout
from kivy.core.window import Window
from kivy.uix.popup import Popup
from kivy.uix.button import Button
from kivy.metrics import dp

from item import Item, make_my_text_input
from iconbutton import IconButton

class UserAccountItem(Item):
    def __init__(self, **kwargs):
        super(UserAccountItem, self).__init__(**kwargs)
        self.my_state_bits = -1

    def delete(self):
        pass

    def setup(self, ioc_root, groupdict, groupname, group, item, flags):
        self.my_group = group
        self.my_groupdict = groupdict
        self.my_groupname = groupname
        self.my_item = item
        self.set_value()

        self.register_event_type('on_remake_page')

        lb = GridLayout()
        lb.size_hint = (0.65, 1) 
        lb.add_widget(Widget())

        # Make buttons
        ncols = 1
        flaglist = flags.split(',')
        for button_name in flaglist:
            b = IconButton()
            b.set_image(button_name, groupdict, groupname)
            b.my_button_action = button_name
            b.bind(on_release = self.my_user_button_pressed)
            lb.add_widget(b)
            ncols += 1

        lb.cols = ncols
        self.add_widget(lb)

    def set_value(self):
        item = self.my_item

        ip = item.get("ip", "")
        text = item.get("user", "")
        if text == "":
            text = ip
            ip = ""

        self.my_label.text = '[size=16]' + text + '[/size]'

        description = ip
        privileges = item.get("privileges", None)
        if privileges != None:
            if description != "":
                description += " "
            description += '[color=9090FF]' + privileges + '[/color]'
        password = item.get("password", None)            
        if password != None:
            if description != "":
                description += " "
            description += '([color=FF9090]' + password + '[/color])'

        self.my_description.text = '[size=14][color=909090]' + description + '[/color][/size]'

    def on_remake_page(self, *args):
        pass

    def my_user_button_pressed(self, instance):
        action = instance.my_button_action
        if action == 'edit':
            self.my_edit_user_popup()
            return

        p = self.parent
        if action == 'delete' or action == 'blacklist' or action == 'accept':
            self.my_group.remove(self.my_item)
            p.remove_widget(self)

        if action == 'blacklist':
            self.my_groupdict['blacklist'].append(self.my_item);
            self.dispatch('on_remake_page', action)

        if action == 'accept':
            self.my_groupdict['accounts'].append(self.my_item);
            self.dispatch('on_remake_page', action)

    def my_set_account_attr(self, attr, value):
        if value == "":
            if self.my_item.get(attr, None) != None:
                del self.my_item[attr]
        else:
            self.my_item[attr] = value

    def my_edit_user_popup(self):
        titlelist = {"accounts": "edit user account", "whitelist":"edit whitelisted item", "blacklist":"edit blacklisted item"}
        item = self.my_item
        groupname = self.my_groupname

        # create grid of text inputs
        grid = GridLayout()
        grid.cols = 2;
        grid.spacing = [6, 6]
        nrows = 1
        self.user_name_input = make_my_text_input(item.get('user', ''))
        grid.add_widget(Label(text='user name'));
        grid.add_widget(self.user_name_input)
        if groupname == "accounts":
            self.password_input = make_my_text_input(item.get('password', ''))
            grid.add_widget(Label(text='password'));
            grid.add_widget(self.password_input)

            self.privileges_input = make_my_text_input(item.get('privileges', ''))
            grid.add_widget(Label(text='privileges'));
            grid.add_widget(self.privileges_input)
            nrows += 2

        if groupname == "blacklist" or groupname == "whitelist":
            self.ip_input = make_my_text_input(item.get('ip', ''))
            grid.add_widget(Label(text='ip'));
            grid.add_widget(self.ip_input)
            nrows += 1

        content = GridLayout()
        content.padding = [6, 6]
        content.cols = 1
        popup_width = min(0.95 * Window.width, dp(500))
        self.popup = popup = Popup(
            title=titlelist[groupname], content=content, size_hint=(None, None),
            size=(popup_width, str(160 + nrows * 70) + 'dp'))

        # construct the content, empty widgets are used as a spacers
        content.add_widget(Widget())
        content.add_widget(grid)
        content.add_widget(Widget())
        content.add_widget(Widget())

        # 2 buttons are created for accept or cancel the current value
        btnlayout = BoxLayout(size_hint_y=None, height='50dp', spacing='5dp')
        btn = Button(text='ok')
        btn.bind(on_release=self.my_user_edit_ok_button_pressed)
        btnlayout.add_widget(btn)
        btn = Button(text='cancel')
        btn.bind(on_release=popup.dismiss)
        btnlayout.add_widget(btn)
        content.add_widget(btnlayout)

        # all done, open the popup !
        popup.open()
        self.user_name_input.focus = True

    def my_user_edit_ok_button_pressed(self, instance):
        groupname = self.my_groupname
        self.my_set_account_attr("user", self.user_name_input.text)

        if groupname == "accounts":
            self.my_set_account_attr("password", self.password_input.text)
            self.my_set_account_attr("privileges", self.privileges_input.text)

        else:
            self.my_set_account_attr("password", "")
            self.my_set_account_attr("privileges", "")

        if groupname == "blacklist" or groupname == "whitelist":
            self.my_set_account_attr("ip", self.ip_input.text)
        else:
            self.my_set_account_attr("ip", "")

        self.set_value()
        self.popup.dismiss()

