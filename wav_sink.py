from wave import Wave_write
import wave
from audio_buffer import AudioBuffer
from audio_sink import AudioSink


class WavSink(AudioSink):
    session_count: int = 0

    def __init__(self, sample_width: int, frame_rate: int):
        super().__init__()
        self.wav_file: Wave_write = None
        self.sample_width = sample_width
        self.frame_rate = frame_rate
        self.frames_written = 0
    
    def __enter__(self):
        self.wav_file = wave.open('output{}.wav'.format('' if WavSink.session_count == 0 else ' {}'.format(WavSink.session_count)), 'wb')
        self.wav_file.setnchannels(1)
        self.wav_file.setsampwidth(self.sample_width)
        self.wav_file.setframerate(self.frame_rate)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        print('wav sink: wrote {} frames'.format(self.frames_written))
        WavSink.session_count = WavSink.session_count + 1
        self.wav_file.close()
        self.wav_file = None

    def handle_audio(self, audio_buffer: AudioBuffer):
        if audio_buffer == None:
            return
        
        self.frames_written = self.frames_written + (len(audio_buffer.data) // audio_buffer.sample_width)
        self.wav_file.writeframes(audio_buffer.data)