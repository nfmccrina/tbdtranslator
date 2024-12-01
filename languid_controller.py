from engine import Engine
from main_window import MainWindow
from settings_window import SettingsWindow
from pubsub import pub

class LanguidController:
    def __init__(self, window: MainWindow, engine: Engine):
        self.window = window
        self.engine = engine
        self.engine.output_handler = window.show_message
        self.settings_window = SettingsWindow(self.window.root, engine.get_audio_device_info())

        pub.subscribe(self.engine.set_selected_audio_device, 'gui.settings.audio_device_changed')
        pub.subscribe(self.engine.set_selected_audio_format, 'gui.settings.audio_format_changed')
        pub.subscribe(self.engine.start_recording, 'gui.recording.recording_started')
        pub.subscribe(self.engine.stop_recording, 'gui.recording.recording_stopped')
        pub.subscribe(self.show_settings_window, 'gui.settings.window_opened')
        pub.subscribe(self.engine.exit, 'gui.quit')

    def start_recording(self):
        self.engine.start_recording()

    def stop_recording(self):
        self.engine.stop_recording()

    def show_settings_window(self):
        self.settings_window.show(self.engine.selected_audio_device if self.engine.selected_audio_device != None else None, \
                                  '{}hz Int{}'.format(self.engine.selected_audio_sample_rate, self.engine.selected_audio_sample_width) if self.engine.selected_audio_sample_rate != None and self.engine.selected_audio_sample_width != None else None)