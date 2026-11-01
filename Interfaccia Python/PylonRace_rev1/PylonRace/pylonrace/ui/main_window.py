"""Finestra principale."""
from __future__ import annotations

from pathlib import Path
from typing import Optional

from PySide6.QtCore import QObject, Qt, Signal
from PySide6.QtGui import QAction, QGuiApplication, QIcon, QKeySequence, QPixmap
from PySide6.QtWidgets import (QComboBox, QFileDialog, QHBoxLayout, QLabel, QMainWindow, QMessageBox, QPushButton,
                               QTabWidget, QWidget, QVBoxLayout)

from .. import __version__
from ..config import AppConfig, app_dir
from ..controller import RaceController
from ..importer import import_xlsm
from ..links import Link, SerialLink, SimulatedCentral, list_ports
from ..models import Event
from ..telegram_bot import TelegramNotifier
from ..voice import Voice
from . import theme
from .classifica_tab import StandingsTab
from .gara_tab import RaceTab
from .piloti_tab import PilotsTab
from .settings_tab import SettingsTab
from .sorteggio_tab import DrawTab

SIM = "__SIM__"
DATA = Path(__file__).resolve().parent.parent / "data"
LOGO = DATA / "logo.png"


class _Bridge(QObject):
    """Porta nel thread dell'interfaccia i messaggi dei thread di Telegram e voce."""

    message = Signal(str, str)


class MainWindow(QMainWindow):
    def __init__(self, cfg: AppConfig, project: Optional[Path] = None):
        super().__init__()
        self.cfg = cfg
        self.setWindowTitle(f"Pylon Race {__version__} — Centrale PC")
        self.setMinimumSize(1000, 620)  # le schede lunghe scorrono
        scr = QGuiApplication.primaryScreen()
        if scr:
            g = scr.availableGeometry()
            self.resize(min(1480, int(g.width() * 0.92)), min(880, int(g.height() * 0.9)))
        else:
            self.resize(1480, 880)

        self._bridge = _Bridge()
        self.notifier = TelegramNotifier(
            cfg.telegram_token, cfg.telegram_chat_id,
            on_result=lambda ok, m: self._bridge.message.emit(m, "info" if ok else "error"))
        self.voice = Voice(on_status=lambda m: self._bridge.message.emit(m, "warn"))
        self.voice.voice_id, self.voice.rate = cfg.voice_id, cfg.voice_rate

        path = project or (Path(cfg.last_project) if cfg.last_project else None)
        if not (path and path.exists()):
            path = app_dir() / "gara_corrente.json"
        try:
            event = Event.load(path) if path.exists() else Event()
        except (OSError, ValueError):
            event = Event()
        self.ctl = RaceController(event, self.notifier, self.voice, output_dir=self.output_dir)
        self.ctl.project_path = path
        self._bridge.message.connect(self.ctl.log)
        self.link: Optional[Link] = None

        self._build_menu()
        self._build_ui()
        self.ctl.log.connect(self._status_from_log)
        self.ctl.set_event(event, path)
        self.reload_all()
        self.refresh_ports()

    # ------------------------------------------------------------------ cartelle
    def output_dir(self) -> Path:
        if self.cfg.output_dir:
            return Path(self.cfg.output_dir)
        d = Path.home() / "PylonRace" / "output"
        return d

    # ------------------------------------------------------------------ UI
    def _build_menu(self) -> None:
        m = self.menuBar().addMenu("&File")
        for text, slot, key in (
            ("Nuova gara", self.new_event, QKeySequence.New),
            ("Apri gara…", self.open_event, QKeySequence.Open),
            ("Salva con nome…", self.save_as, QKeySequence.SaveAs),
            ("Importa dal vecchio file Excel (.xlsm)…", self.import_excel, None),
        ):
            a = QAction(text, self)
            if key:
                a.setShortcut(key)
            a.triggered.connect(slot)
            m.addAction(a)
        m.addSeparator()
        q = QAction("Esci", self)
        q.setShortcut(QKeySequence.Quit)
        q.triggered.connect(self.close)
        m.addAction(q)
        h = self.menuBar().addMenu("&Aiuto")
        a = QAction("Informazioni", self)
        a.triggered.connect(self.about)
        h.addAction(a)

    def _build_ui(self) -> None:
        root = QWidget()
        root.setObjectName("root")
        self.setCentralWidget(root)
        lay = QVBoxLayout(root)
        lay.setContentsMargins(10, 8, 10, 6)

        top = QHBoxLayout()
        self.logo_label = QLabel()
        if LOGO.exists():
            self.setWindowIcon(QIcon(str(LOGO)))
            self.logo_label.setPixmap(QPixmap(str(LOGO)).scaled(44, 44, Qt.KeepAspectRatio, Qt.SmoothTransformation))
            top.addWidget(self.logo_label)
        self.title_label = QLabel("")
        self.title_label.setObjectName("h1")
        top.addWidget(self.title_label, 1)
        top.addWidget(QLabel("Centrale:"))
        self.port_box = QComboBox()
        self.port_box.setMinimumWidth(300)
        top.addWidget(self.port_box)
        self.btn_ref = QPushButton("Aggiorna")
        self.btn_ref.setToolTip("Aggiorna l'elenco delle porte seriali")
        top.addWidget(self.btn_ref)
        self.btn_conn = QPushButton("Connetti")
        self.btn_conn.setObjectName("primary")
        self.btn_conn.setMinimumWidth(110)
        top.addWidget(self.btn_conn)
        self.led = QLabel("●  Non connesso")
        self.led.setMinimumWidth(190)
        top.addWidget(self.led)
        lay.addLayout(top)
        self._set_led(False, "Non connesso")

        self.tabs = QTabWidget()
        lay.addWidget(self.tabs, 1)
        self.race_tab = RaceTab(self.ctl)
        self.pilots_tab = PilotsTab(self.ctl)
        self.draw_tab = DrawTab(self.ctl, self.output_dir)
        self.stand_tab = StandingsTab(self.ctl, self.output_dir)
        self.set_tab = SettingsTab(self.ctl, self.cfg, self.voice, self.notifier)
        for w, name in ((self.race_tab, "Gara"), (self.pilots_tab, "Piloti"), (self.draw_tab, "Sorteggio"),
                        (self.stand_tab, "Classifica"), (self.set_tab, "Impostazioni")):
            self.tabs.addTab(w, name)

        self.btn_conn.clicked.connect(self.toggle_connection)
        self.btn_ref.clicked.connect(self.refresh_ports)
        self.race_tab.status_message.connect(lambda m: self.statusBar().showMessage(m, 5000))
        self.pilots_tab.import_requested.connect(self.import_excel)
        self.pilots_tab.schedule_obsolete.connect(self._schedule_obsolete)
        self.draw_tab.schedule_changed.connect(self.race_tab.reload)
        self.set_tab.config_changed.connect(self._config_changed)
        self.set_tab.port.editTextChanged.connect(lambda *_: None)

    def _schedule_obsolete(self) -> None:
        self.statusBar().showMessage(
            "Elenco piloti modificato: l'ordine di volo esistente non cambia. Rigeneralo dalla scheda Sorteggio se serve.", 9000)

    def _status_from_log(self, msg: str, level: str) -> None:
        if level in ("info", "warn", "error"):
            self.statusBar().showMessage(msg, 8000)

    def _config_changed(self) -> None:
        self.title_label.setText(self.ctl.race.settings.title)

    # ------------------------------------------------------------------ connessione
    def refresh_ports(self) -> None:
        cur = self.port_box.currentData() or self.cfg.port
        self.port_box.clear()
        self.port_box.addItem("Centrale simulata (prova a tavolino)", SIM)
        ports = list_ports()
        for dev, desc in ports:
            self.port_box.addItem(f"{dev} — {desc}", dev)
        i = self.port_box.findData(cur)
        if i < 0:  # scegli automaticamente il convertitore CH340 della centrale
            for k, (dev, desc) in enumerate(ports, start=1):
                if "CH340" in desc.upper() or "USB-SERIAL" in desc.upper():
                    i = k
                    break
        self.port_box.setCurrentIndex(i if i >= 0 else (1 if ports else 0))

    def _set_led(self, on: bool, text: str, error: bool = False) -> None:
        color = theme.OK if on else (theme.ERR if error else theme.MUTED)
        self.led.setText(f"●  {text}")
        self.led.setStyleSheet(f"color:{color}; font-weight:600;")
        self.btn_conn.setText("Disconnetti" if on else "Connetti")

    def toggle_connection(self) -> None:
        if self.link and self.link.is_open:
            self.disconnect_link()
            return
        sel = self.port_box.currentData()
        if sel == SIM:
            link: Link = SimulatedCentral()
        else:
            if not sel:
                QMessageBox.warning(self, "Centrale", "Nessuna porta seriale disponibile. Collega la centrale al PC.")
                return
            self.cfg.port = sel
            link = SerialLink(sel, self.cfg.baud, self.cfg.dtr)
        link.state_changed.connect(lambda on, t: self._set_led(on, t, error=not on and "Errore" in t))
        self.ctl.set_link(link)
        if link.open():
            self.link = link
            self.cfg.save()
            self.race_tab.attach_simulator(link if isinstance(link, SimulatedCentral) else None)
            self.ctl.log.emit(f"Collegato a {self.port_box.currentText()}", "info")

    def disconnect_link(self) -> None:
        if self.link:
            self.link.close()
        self.race_tab.attach_simulator(None)
        self.link = None

    # ------------------------------------------------------------------ file
    def reload_all(self) -> None:
        self.title_label.setText(self.ctl.race.settings.title)
        self.race_tab.reload()
        self.pilots_tab.reload()
        self.draw_tab.reload()
        self.stand_tab.reload()
        self.set_tab.reload()
        self.setWindowTitle(f"Pylon Race {__version__} — {self.ctl.race.settings.title}")

    def _load_event(self, event: Event, path: Optional[Path]) -> None:
        self.ctl.set_event(event, path)
        if path:
            self.cfg.last_project = str(path)
            self.cfg.save()
        self.reload_all()

    def new_event(self) -> None:
        if QMessageBox.question(self, "Nuova gara", "Iniziare una nuova gara vuota?\nLa gara corrente resta salvata su disco.",
                                QMessageBox.Yes | QMessageBox.No, QMessageBox.No) != QMessageBox.Yes:
            return
        self.ctl.save_now()
        folder = Path(self.cfg.last_project).parent if self.cfg.last_project else app_dir()
        n = 1
        while (folder / f"gara_{n}.json").exists():
            n += 1
        self._load_event(Event(), folder / f"gara_{n}.json")

    def open_event(self) -> None:
        f, _ = QFileDialog.getOpenFileName(self, "Apri gara", str(app_dir()), "Gara Pylon (*.json)")
        if f:
            try:
                ev = Event.load(Path(f))
            except Exception as exc:
                QMessageBox.critical(self, "Apri gara", f"File non valido: {exc}")
                return
            self.ctl.save_now()
            self._load_event(ev, Path(f))

    def save_as(self) -> None:
        f, _ = QFileDialog.getSaveFileName(self, "Salva gara con nome", str(app_dir() / "gara.json"), "Gara Pylon (*.json)")
        if f:
            self.ctl.project_path = Path(f)
            self.ctl.save_now()
            self.cfg.last_project = f
            self.cfg.save()
            self.statusBar().showMessage(f"Salvato: {f}", 5000)

    def import_excel(self) -> None:
        f, _ = QFileDialog.getOpenFileName(self, "Importa dal vecchio file Excel", str(Path.home()),
                                           "Excel con macro (*.xlsm *.xlsx)")
        if not f:
            return
        try:
            ev = import_xlsm(Path(f))
        except Exception as exc:
            QMessageBox.critical(self, "Importazione", f"Non riuscita: {exc}")
            return
        keep = QMessageBox.question(
            self, "Importazione",
            f"Trovati {len(ev.pilots)} piloti e {ev.n_rounds} round (con i risultati già presenti).\n\n"
            "Sostituire la gara corrente con questi dati?", QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
        if keep != QMessageBox.Yes:
            return
        old = self.ctl.race.settings
        ev.settings.telegram_on, ev.settings.voice_on = old.telegram_on, old.voice_on
        ev.settings.classifica_link = old.classifica_link
        self.ctl.save_now()
        self._load_event(ev, self.ctl.project_path)
        self.ctl.request_autosave()

    def about(self) -> None:
        box = QMessageBox(self)
        box.setWindowTitle("Informazioni")
        if LOGO.exists():
            box.setIconPixmap(QPixmap(str(LOGO)).scaled(96, 96, Qt.KeepAspectRatio, Qt.SmoothTransformation))
        box.setText(f"<b>FT Models — Pylon Race</b> {__version__}<br>Gestione gare di pylon racing RC: "
                    "ordine di volo, centrale via seriale, tempi, tagli, classifica.")
        box.exec()

    # ------------------------------------------------------------------ chiusura
    def closeEvent(self, e) -> None:
        self.ctl.save_now()
        self.disconnect_link()
        self.cfg.save()
        e.accept()
