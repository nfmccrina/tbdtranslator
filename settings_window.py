from tkinter import *
from tkinter import ttk
from pubsub import pub

from audio_device import AudioDevice

class SettingsWindow:
    def __init__(self, master: Tk, audio_devices: list[AudioDevice]):
        self.master = master
        self.audio_devices = audio_devices
        self.settings_window = None
        self.frame = None
        self.device_selection_combobox = None
        self.format_selection_combobox = None

    def show(self, initial_device_index = None, initial_format_string = None):
        self.settings_window = Toplevel(master=self.master)
        self.settings_window.title("Languid Settings")
        self.frame = ttk.Frame(master=self.settings_window, padding=10)
        self.frame.grid()
        self.device_selection_combobox = ttk.Combobox(self.frame, values=list(map(lambda device: device.device_name, self.audio_devices)), state="readonly")
        self.format_selection_combobox = ttk.Combobox(self.frame, values=[], state="disabled")

        if initial_device_index != None:
            self.device_selection_combobox.set(self.get_device_name_by_index(initial_device_index))

        if initial_format_string != None:
            self.format_selection_combobox.set(initial_format_string)
            self.format_selection_combobox.configure(state="readonly")
    
        ttk.Label(self.frame, text="Audio device: ").grid(row=0, column=0)
        self.device_selection_combobox.grid(row=0, column=1)
        self.device_selection_combobox.bind("<<ComboboxSelected>>", lambda e, widget=self.device_selection_combobox: self.audio_device_changed_handler(e, widget))
        ttk.Label(self.frame, text="Audio format: ").grid(row=1, column=0)
        self.format_selection_combobox.grid(row=1, column=1)
        self.format_selection_combobox.bind("<<ComboboxSelected>>", lambda e, widget=self.format_selection_combobox: self.audio_format_changed_handler(e, widget))
        self.settings_window.mainloop()

    def audio_device_changed_handler(self, event, widget):
        device = None

        try:
            device = list(filter(lambda d: d.device_name == widget.get(), self.audio_devices)).pop()
        except ValueError:
            return
        
        if device == None:
            return
        
        self.format_selection_combobox.configure(state="readonly", values=list(map(lambda f: f.name(), device.supported_sample_formats)))
        pub.sendMessage('gui.settings.audio_device_changed', device_index=device.device_index, channel_index=device.channel_index)

    def audio_format_changed_handler(self, event, widget):
        pub.sendMessage('gui.settings.audio_format_changed', format_string=widget.get())

    def get_device_name_by_index(self, device_index):
        return list(filter(lambda d: d.device_index == device_index, self.audio_devices)).pop().device_name