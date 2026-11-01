"""Avvio di Pylon Race."""
import sys
from pathlib import Path

from PySide6.QtWidgets import QApplication

from pylonrace.config import AppConfig
from pylonrace.ui import theme
from pylonrace.ui.main_window import MainWindow


def main() -> int:
    app = QApplication(sys.argv)
    app.setApplicationName("Pylon Race")
    if sys.platform == "win32":  # icona corretta nella barra delle applicazioni
        try:
            import ctypes
            ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID("FTModels.PylonRace")
        except Exception:
            pass
    cfg = AppConfig.load()
    theme.apply(app, cfg.ui_pt)
    project = Path(sys.argv[1]) if len(sys.argv) > 1 else None
    win = MainWindow(cfg, project)
    win.show()
    return app.exec()


if __name__ == "__main__":
    sys.exit(main())
