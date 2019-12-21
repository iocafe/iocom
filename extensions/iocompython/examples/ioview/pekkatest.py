import kivy
from kivy.app import App
from kivy.uix.settings import Settings, SettingItem, SettingsPanel, SettingTitle
from kivy.uix.button import Button
from kivy.uix.image import AsyncImage

class SettingsApp(App):
    def build(self):
        self.settings_panel = Settings() #create instance of Settings

    def add_one_panel(self):
        panel = SettingsPanel(title="I like trains", settings=self)
        panel.add_widget(AsyncImage(source="http://i3.kym-cdn.com/entries/icons/original/000/004/795/I-LIKE-TRAINS.jpg"))
        self.settings_panel.add_widget(panel)
        print("Hello World from ", self)

        panel = SettingsPanel(title="Customized", settings=self) #create instance of left side panel
        item1 = SettingItem(panel=panel, title="easy :)", desc="press that button to see it your self", settings = self) #create instance of one item in left side panel
        item2 = SettingTitle(title="Kivy is awesome") #another widget in left side panel
        button = Button(text="Add one more panel")

        item1.add_widget(button) #add widget to item1 in left side panel
        button.bind(on_release=add_one_panel) #bind that button to function

        panel.add_widget(item1) # add item1 to left side panel
        panel.add_widget(item2) # add item2 to left side panel
        self.settings_panel.add_widget(panel) #add left side panel itself to the settings menu

        return self.settings_panel # show the settings interface



a = SettingsApp()
a.add_one_panel()
a.run()