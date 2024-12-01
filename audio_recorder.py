import time
import pyaudio
import wave
import queue

from audio_processor import AudioProcessor

class AudioRecorder:
    def __init__(self, audio_queue: queue.SimpleQueue, command_queue: queue.SimpleQueue):
        self.audio_queue = audio_queue
        self.command_queue = command_queue
        self.should_quit = False
        self.channel_index = None
        self.data_counter = 0
    
    def recording_callback(self, in_data, frame_count, time_info, status):
        self.data_counter = self.data_counter + frame_count
        self.audio_queue.put((in_data, frame_count, time_info, status))

        try:
            (command, _) = self.command_queue.get(block=False)

            if command == 'stop':
                print('audio_recorder has stopped recording! Recorded {} frames'.format(self.data_counter))
                self.data_counter = 0
                return (b'', pyaudio.paComplete)
            elif command == 'exit':
                self.should_quit = True
        except queue.Empty:
            pass

        return (b'', pyaudio.paContinue)

    def record(self):
        while not self.should_quit:
            (command, data) = self.command_queue.get()

            if command == 'start':
                device_index = data['device_index']
                total_channels = 1 # data['total_channels']
                sample_rate = 16000 # data['sample_rate']
                sample_width = 2 # data['sample_width']
                self.channel_index = data['channel_index']

                print('audio_recorder is recording!')
                pa = pyaudio.PyAudio()

                # stream = pa.open(rate=sample_rate, input_device_index=device_index, channels=total_channels, format=pa.get_format_from_width(sample_width), input=True, stream_callback=self.recording_callback)
                stream = pa.open(rate=sample_rate, input_device_index=device_index, channels=total_channels, format=pa.get_format_from_width(sample_width), input=True)

                # while stream.is_active():
                #     time.sleep(0.1)

                with AudioProcessor(sample_rate=sample_rate, sample_width=sample_width) as ap:
                    while True:
                        ap.pr ((stream.read(1024), 1024, sample_rate, sample_width))
                        try:
                            (command, data) = self.command_queue.get(block=False)

                            if command == 'stop' or command == 'exit':
                                print('audio_recorder has stopped recording! Recorded {} frames'.format(self.data_counter))
                                self.data_counter = 0
                        except queue.Empty:
                            pass

                stream.close()
                pa.terminate()
            elif command == 'stop':
                print('audio_recorder has stopped recording! Recorded {} frames'.format(self.data_counter))
                self.data_counter = 0
                pass
            elif command == 'exit':
                break
            else:
                continue
        
        print('audio_recorder is shutting down!')