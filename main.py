from engine import Engine
from languid_controller import LanguidController
from main_window import MainWindow

if __name__ == '__main__':
    engine = Engine()
    main_window = MainWindow()
    controller = LanguidController(main_window, engine)

    main_window.start_event_loop()