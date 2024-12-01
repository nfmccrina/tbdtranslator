class AudioDevice:
    def __init__(self, device_index, channel_index, device_name, supported_sample_formats):
        if len(supported_sample_formats) < 1:
            raise RuntimeError()
        self.device_index = device_index
        self.channel_index = channel_index
        self.device_name = device_name
        self.supported_sample_formats = supported_sample_formats
    
    def __str__(self):
        return 'device index: {}, channel index: {}, name: {}, supported formats: {}'.format(self.device_index, self.channel_index, self.device_name, self.supported_sample_formats)
    
    def __repr__(self):
        return self.__str__()