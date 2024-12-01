from queue import SimpleQueue
from threading import Thread
from audio_device import AudioDevice
from audio_device_finder import AudioDeviceFinder
from audio_processor import AudioProcessor
class Engine:
    def __init__(self, audio_device_finder: AudioDeviceFinder, audio_queue: SimpleQueue, command_queue: SimpleQueue, output_handler = lambda message: None):
        self.started = False
        self.selected_audio_device = None
        self.selected_device_channel = None
        self.selected_audio_sample_rate = None
        self.selected_audio_sample_width = None
        self.output_handler = output_handler
        self.audio_device_finder = audio_device_finder
        self.command_queue = command_queue
        self.audio_queue = audio_queue
        self.audio_processor = None
        self.audio_processing_thread = None

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

        device_info = self.audio_device_finder.get_device_info(self.selected_audio_device)
        total_input_channels = device_info['maxInputChannels']

        self.audio_processor = AudioProcessor(self.audio_queue, self.selected_audio_sample_rate, self.selected_audio_sample_width)
        self.audio_processing_thread = Thread(target=self.audio_processor.process, daemon=False)
        self.audio_processing_thread.start()

        command_data = {}
        command_data['device_index'] = self.selected_audio_device
        command_data['channel_index'] = self.selected_device_channel if self.selected_device_channel != None else 0
        command_data['total_channels'] = total_input_channels
        command_data['sample_rate'] = self.selected_audio_sample_rate
        command_data['sample_width'] = self.selected_audio_sample_width

        self.command_queue.put(('start', command_data))
        self.started = True
    
    def stop_recording(self):
        if not self.started:
            self.output_handler("Recording is already stopped")
            return

        self.command_queue.put(('stop', {}))
        self.audio_processor.stop()
        self.audio_processing_thread.join()
        self.audio_processing_thread = None
        self.audio_processor = None
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
        if self.audio_processor != None:
            self.audio_processor.stop()

            if self.audio_processing_thread != None:
                self.audio_processing_thread.join()
                
        self.audio_processing_thread = None
        self.audio_processor = None
        self.command_queue.put(('stop', {}))
        self.command_queue.put(('exit', {}))