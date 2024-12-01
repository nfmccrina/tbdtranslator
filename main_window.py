from tkinter import *
from tkinter import ttk
from tkinter import messagebox
from pubsub import pub

class MainWindow:
    def __init__(self):
        self.root = Tk()
        self.root.title("Languid")

        self.frame = ttk.Frame(self.root, padding=10)
        self.frame.grid()

        self.root.createcommand("tk::mac::ShowPreferences", lambda: pub.sendMessage('gui.settings.window_opened'))
        self.root.createcommand("tk::mac::Quit", self.quit)
        self.root.protocol('WM_DELETE_WINDOW', self.quit)

        self.setup_start_stop_buttons()

    def setup_start_stop_buttons(self):
        ttk.Button(self.frame, text="Start Recording", command=lambda: pub.sendMessage('gui.recording.recording_started')).grid(row=0, column=0)
        ttk.Button(self.frame, text="Stop Recording", command=lambda: pub.sendMessage('gui.recording.recording_stopped')).grid(row=0, column=1)

    def start_event_loop(self):
        self.root.mainloop()
    
    def show_message(self, message):
        msg = messagebox.showinfo(message=message, parent=self.frame)

    def quit(self):
        pub.sendMessage('gui.quit')
        self.root.destroy()