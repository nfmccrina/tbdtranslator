import threading
import wave
import queue

from audio_buffer import AudioBuffer
from audio_sink import AudioSink
from wav_sink import WavSink
from pubsub import pub

class AudioProcessor(object):
    def __init__(self, audio_queue: queue.SimpleQueue):
        self.audio_queue = audio_queue
        self.recording_stopped = True
        self.processing_thread = None
        pub.subscribe(self.on_recording_start, 'engine.start')
        pub.subscribe(self.on_recording_stop, 'engine.stop')
        pub.subscribe(self.on_recording_stop, 'engine.exit')

    def on_recording_stop(self):
        self.recording_stopped = True
        
        if self.processing_thread != None:
            self.processing_thread.join()
        
        self.processing_thread = None

    def on_recording_start(self, device_index: int, channel_index: int, total_channels: int, sample_rate: int, sample_width: int):
        self.recording_stopped = False
        self.processing_thread = threading.Thread(None, self.process_audio)
        self.processing_thread.start()

    def process_audio(self):
        print('starting processing')
        audio_buffer: AudioBuffer = None
        while not self.recording_stopped:
            try:
                audio_buffer: AudioBuffer = self.audio_queue.get(block=False)
                break
            except queue.Empty:
                continue

        if self.recording_stopped and audio_buffer == None:
            return
                
        audio_sink: AudioSink
        with WavSink(sample_width=audio_buffer.sample_width, frame_rate=audio_buffer.sample_rate) as audio_sink:
            while not self.recording_stopped:
                if audio_buffer != None:
                    audio_sink.handle_audio(audio_buffer=audio_buffer)
                
                try:
                    audio_buffer: AudioBuffer = self.audio_queue.get(block=False)
                except queue.Empty:
                    audio_buffer = None
            
            while True:
                try:
                    audio_buffer: AudioBuffer = self.audio_queue.get(block=False)
                except queue.Empty:
                    break

                audio_sink.handle_audio(audio_buffer=audio_buffer)
        
        print('stopping processing')

    def speech_api_handler(self, wav_file):
        pass
