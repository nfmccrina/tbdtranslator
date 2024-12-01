from queue import SimpleQueue
from audio_device_finder import AudioDeviceFinder
from audio_recorder import AudioRecorder
from engine import Engine
from languid_controller import LanguidController
from main_window import MainWindow
import threading

if __name__ == '__main__':
    audio_queue = SimpleQueue()
    command_queue = SimpleQueue()
    audio_device_finder = AudioDeviceFinder()
    engine = Engine(audio_device_finder=audio_device_finder, audio_queue=audio_queue, command_queue=command_queue)
    main_window = MainWindow()
    controller = LanguidController(main_window, engine)
    audio_recorder = AudioRecorder(audio_queue=audio_queue, command_queue=command_queue)

    threading.Thread(target=audio_recorder.record, daemon=False).start()

    main_window.start_event_loop()