import pyaudio
from audio_device import AudioDevice
from sample_format import SampleFormat, SAMPLE_RATES, SAMPLE_FORMATS

class AudioDeviceFinder:
    def get_device_info(self, device_index: int):
        pa = pyaudio.PyAudio()
        return pa.get_device_info_by_index(device_index=device_index)

    def find_system_audio_devices(self):
        pa = pyaudio.PyAudio()

        device_count = pa.get_device_count()
        devices = []

        for i in range(device_count):
            current_device = pa.get_device_info_by_index(i)
            max_channels = current_device['maxInputChannels']

            if max_channels < 1:
                continue

            supported_formats = []

            for sample_rate in SAMPLE_RATES:
                for sample_format in SAMPLE_FORMATS:

                    if pa.is_format_supported(sample_rate, i, max_channels, sample_format):
                        supported_formats.append(SampleFormat(sample_rate, sample_format))

            for channel in range(max_channels):
                channel_string = ': channel {}'.format(channel) if max_channels > 1 else ''

                devices.append(AudioDevice(i, channel, current_device['name'] + channel_string, supported_formats))
        
        return devices

