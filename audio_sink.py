from abc import ABC, abstractmethod

from audio_buffer import AudioBuffer


class AudioSink(ABC):
    @abstractmethod
    def handle_audio(self, audio_buffer: AudioBuffer):
        pass