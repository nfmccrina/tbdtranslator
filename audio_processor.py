import wave
import queue

class AudioProcessor(object):
    def __init__(self, sample_rate, sample_width):
        self.sample_rate = sample_rate
        self.sample_width = sample_width
        self.wav_file = None

    def __enter__(self):
        self.wav_file = wave.open('output.wav', 'wb')
        self.wav_file.setnchannels(1)
        self.wav_file.setframerate(self.sample_rate)
        self.wav_file.setsampwidth(int(self.sample_width))

    def __exit__(self):
        self.wav_file.close()

    def process_audio_chunk(self, data, frame_count):
        self.wav_file.writeframes(data)