import pyaudio

class SampleFormat:
    def __init__(self, rate, format):
        self.rate = rate
        self.format = format

    def pa_format_to_string(self, pa_format):
        if pa_format == pyaudio.paInt8:
            return 'Int8'
        elif pa_format == pyaudio.paUInt8:
            return 'UInt8'
        elif pa_format == pyaudio.paInt16:
            return 'Int16'
        elif pa_format == pyaudio.paInt24:
            return 'Int24'
        elif pa_format == pyaudio.paInt32:
            return 'Int32'
        elif pa_format == pyaudio.paFloat32:
            return 'Float32'
        else:
            raise ValueError("{} is not a valid format.".format(pa_format))

    def name(self):
        format_string = self.pa_format_to_string(self.format)

        return '{}hz {}'.format(self.rate, format_string)
    
    def __repr__(self):
        return self.name()

SAMPLE_RATES = [16000, 44100]
SAMPLE_FORMATS = [pyaudio.paInt16, pyaudio.paInt24]
    