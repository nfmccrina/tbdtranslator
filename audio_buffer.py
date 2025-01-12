class AudioBuffer:
    def __init__(self, data: bytes, sample_rate: int, sample_width: int):
        self.data = data
        self.sample_rate = sample_rate
        self.sample_width = sample_width