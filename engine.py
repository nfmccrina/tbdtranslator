from queue import SimpleQueue
import threading
from audio_device import AudioDevice
from audio_device_finder import AudioDeviceFinder
from audio_processor import AudioProcessor
from pubsub import pub

from audio_recorder import AudioRecorder

class Engine:
    def __init__(self, output_handler = lambda message: None):
        self.started = False
        self.selected_audio_device = None
        self.selected_device_channel = None
        self.selected_audio_sample_rate = None
        self.selected_audio_sample_width = None
        self.output_handler = output_handler
        self.audio_device_finder = AudioDeviceFinder()
        self.audio_queue = SimpleQueue()
        self.audio_recorder = AudioRecorder(audio_queue=self.audio_queue)
        self.audio_processor = AudioProcessor(audio_queue=self.audio_queue)

    def start_recording(self):
        if self.started:
            self.output_handler("Recording is already in progress.")
            return
        
        if self.selected_audio_device == None:
            self.output_handler("No audio device has been selected.")
            return
        
        if self.selected_audio_sample_rate == None or self.selected_audio_sample_width == None:
            self.output_handler("The audio format has not been selected.")
            return
        
        # self.audio_processing_thread = threading.Thread(target=self.audio_processor.process_audio, daemon=False).start()

        device_info = self.audio_device_finder.get_device_info(self.selected_audio_device)
        total_input_channels = device_info['maxInputChannels']

        channel_index = self.selected_device_channel if self.selected_device_channel != None else 0

        pub.sendMessage('engine.start', device_index=self.selected_audio_device, channel_index=channel_index, total_channels=total_input_channels, sample_rate=self.selected_audio_sample_rate, sample_width=self.selected_audio_sample_width // 8)
        self.started = True
    
    def stop_recording(self):
        if not self.started:
            self.output_handler("Recording is already stopped")
            return

        pub.sendMessage('engine.stop')

        self.started = False
    
    def get_audio_device_info(self):
        return self.audio_device_finder.find_system_audio_devices()
    
    def set_selected_audio_device(self, device_index: int, channel_index: int):
        self.selected_audio_device = device_index
        self.selected_device_channel = channel_index

    def set_selected_audio_format(self, format_string: str):
        if self.started:
            self.output_handler("Cannot change audio format while recording.")
            return
        
        format_string_parts = format_string.split()
        self.selected_audio_sample_rate = int(format_string_parts[0].replace('hz', ''))
        self.selected_audio_sample_width = self.get_sample_width_from_format(format_string_parts[1])

    def get_sample_width_from_format(self, sample_format: str):
        if sample_format.endswith('8'):
            return 8
        elif sample_format.endswith('16'):
            return 16
        elif sample_format.endswith('24'):
            return 24
        elif sample_format.endswith('32'):
            return 32
        else:
            raise ValueError('Could not determine sample width for {} format.'.format(sample_format))
    
    def exit(self):
        pub.sendMessage('engine.stop')
        pub.sendMessage('engine.exit')