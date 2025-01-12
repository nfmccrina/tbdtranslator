import time
import pyaudio
import wave
import queue

from audio_buffer import AudioBuffer
from audio_processor import AudioProcessor
import threading
from pubsub import pub

class AudioRecorder:
    def __init__(self, audio_queue: queue.SimpleQueue):
        self.audio_queue = audio_queue
        self.should_quit = False
        self.should_stop = False
        self.buffer_size = 1024
        self.total_channels = 1
        self.sample_rate = 16000
        self.sample_width = 2
        self.data_counter = 0
        self.data_info = []
        self.recording_thread: threading.Thread = None

        pub.subscribe(self.start_recording, 'engine.start')
        pub.subscribe(self.stop_recording, 'engine.stop')
        pub.subscribe(self.stop_recording, 'engine.exit')
    
    def pyaudio_callback(self, in_data, frame_count, time_info, status):
        self.data_info.append((len(in_data), frame_count))
        self.data_counter = self.data_counter + frame_count
        audio_buffer: AudioBuffer = AudioBuffer(in_data, sample_rate=self.sample_rate, sample_width=self.sample_width)
        self.audio_queue.put(audio_buffer)

        if self.should_stop:
            print('stopping')
            return (b'', pyaudio.paComplete)
        else:
            return (b'', pyaudio.paContinue)
    
    def start_recording(self, device_index: int, channel_index: int, total_channels: int, sample_rate: int, sample_width: int):
        self.should_stop = False
        self.channel_index = channel_index
        self.sample_rate = sample_rate
        self.sample_width = sample_width
        self.total_channels = total_channels
        self.recording_thread = threading.Thread(target=self.record, daemon=False, args=[device_index, total_channels, sample_rate, sample_width]).start()

    def stop_recording(self):
        self.should_stop = True
        
        if self.recording_thread != None:
            self.recording_thread.join()

        self.recording_thread = None
        for i in self.data_info:
            data_length, frame_count = i
            print('{}, {}'.format(frame_count, data_length))
        self.data_info = []
        print('audio_recorder has stopped recording! Recorded {} frames'.format(self.data_counter))
        self.data_counter = 0

    def record(self, device_index, total_channels, sample_rate, sample_width):
        print('starting record')

        print('audio_recorder is recording!')
        pa = pyaudio.PyAudio()

        stream = pa.open(rate=sample_rate, frames_per_buffer=1024, input_device_index=device_index, channels=total_channels, format=pa.get_format_from_width(sample_width), input=True, stream_callback=self.pyaudio_callback)

        while stream.is_active():
            time.sleep(0.1)

        print('stopping record')
        
        stream.close()
        pa.terminate()
