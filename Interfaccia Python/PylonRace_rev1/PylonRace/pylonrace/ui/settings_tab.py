"""Scheda IMPOSTAZIONI."""
from __future__ import annotations

from PySide6.QtCore import Signal
from PySide6.QtWidgets import (QCheckBox, QComboBox, QDoubleSpinBox, QFileDialog, QFormLayout, QGroupBox, QHBoxLayout,
                               QFrame, QLineEdit, QMessageBox, QPushButton, QScrollArea, QSpinBox, QVBoxLayout, QWidget)

from ..config import AppConfig
from ..controller import RaceController
from ..links import list_ports
from ..voice import Voice, announce_text


class SettingsTab(QWidget):
    config_changed = Signal()

    def __init__(self, ctl: RaceController, cfg: AppConfig, voice: Voice, notifier, parent=None):
        super().__init__(parent)
        self.ctl, self.cfg, self.voice, self.notifier = ctl, cfg, voice, notifier
        inner = QWidget()
        inner.setObjectName("scrollInner")
        outer = QVBoxLayout(inner)
        outer.setContentsMargins(10, 10, 10, 10)

        # ---- evento
        g = QGroupBox("Gara")
        f = QFormLayout(g)
        self.title = QLineEdit()
        f.addRow("Titolo (intestazioni e PDF)", self.title)
        self.link = QLineEdit()
        self.link.setPlaceholderText("link che Telegram mostra insieme all'avviso classifica (es. Google Drive)")
        f.addRow("Link classifica", self.link)
        self.out = QLineEdit()
        b = QPushButton("Sfoglia…")
        b.clicked.connect(self._pick_dir)
        row = QHBoxLayout()
        row.addWidget(self.out, 1)
        row.addWidget(b)
        f.addRow("Cartella file generati", row)
        outer.addWidget(g)

        # ---- centrale
        g = QGroupBox("Centrale (porta seriale)")
        f = QFormLayout(g)
        self.port = QComboBox()
        self.port.setEditable(True)
        b = QPushButton("Aggiorna elenco")
        b.clicked.connect(self.refresh_ports)
        row = QHBoxLayout()
        row.addWidget(self.port, 1)
        row.addWidget(b)
        f.addRow("Porta", row)
        self.baud = QComboBox()
        for v in (9600, 19200, 38400, 57600, 115200):
            self.baud.addItem(str(v), v)
        f.addRow("Baud rate", self.baud)
        self.dtr = QCheckBox("Attiva DTR all'apertura (come Data Streamer)")
        f.addRow("", self.dtr)
        self.fold = QCheckBox("Invia i nomi senza accenti (il display LCD non li mostra)")
        f.addRow("", self.fold)
        self.back750 = QCheckBox("Usa il codice 750 per INDIETRO (Excel usava sempre 850)")
        f.addRow("", self.back750)
        self.guard = QDoubleSpinBox()
        self.guard.setRange(0, 60)
        self.guard.setSuffix(" s")
        f.addRow("Pausa dopo i risultati", self.guard)
        self.rwait = QDoubleSpinBox()
        self.rwait.setRange(0.5, 30)
        self.rwait.setSuffix(" s")
        f.addRow("Attesa dopo il reset centrale", self.rwait)
        outer.addWidget(g)

        # ---- telegram
        g = QGroupBox("Telegram")
        f = QFormLayout(g)
        self.token = QLineEdit()
        self.token.setEchoMode(QLineEdit.Password)
        self.token.setPlaceholderText("token del bot (da @BotFather)")
        self.chat = QLineEdit()
        self.chat.setPlaceholderText("es. -1001234567890")
        f.addRow("Token bot", self.token)
        f.addRow("Chat ID", self.chat)
        self.b_tg = QPushButton("Invia messaggio di prova")
        f.addRow("", self.b_tg)
        outer.addWidget(g)

        # ---- voce
        g = QGroupBox("Voce")
        f = QFormLayout(g)
        self.voice_box = QComboBox()
        self.rate = QSpinBox()
        self.rate.setRange(80, 300)
        self.second = QDoubleSpinBox()
        self.second.setRange(0, 300)
        self.second.setSuffix(" s")
        self.b_voice = QPushButton("Prova voce")
        f.addRow("Voce di sistema", self.voice_box)
        f.addRow("Velocità", self.rate)
        f.addRow("Seconda lettura dopo", self.second)
        f.addRow("", self.b_voice)
        outer.addWidget(g)
        outer.addStretch(1)
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setFrameShape(QFrame.NoFrame)
        scroll.setWidget(inner)
        root = QVBoxLayout(self)
        root.setContentsMargins(0, 0, 0, 0)
        root.addWidget(scroll)

        self.b_tg.clicked.connect(self._test_tg)
        self.b_voice.clicked.connect(self._test_voice)
        for sig in (self.title.editingFinished, self.link.editingFinished, self.out.editingFinished,
                    self.token.editingFinished, self.chat.editingFinished):
            sig.connect(self.apply)
        for w in (self.baud, self.voice_box, self.port):
            (w.currentIndexChanged if w is not self.port else w.editTextChanged).connect(lambda *_: self.apply())
        for w in (self.dtr, self.fold, self.back750):
            w.toggled.connect(lambda *_: self.apply())
        for w in (self.guard, self.rwait, self.second, self.rate):
            w.editingFinished.connect(self.apply)
        self.refresh_ports()
        self._load_voices()
        self.reload()

    # ------------------------------------------------------------------
    def refresh_ports(self) -> None:
        cur = self.port.currentText() or self.cfg.port
        self.port.blockSignals(True)
        self.port.clear()
        for dev, desc in list_ports():
            self.port.addItem(f"{dev} — {desc}", dev)
        i = self.port.findData(cur)
        if i >= 0:
            self.port.setCurrentIndex(i)
        else:
            self.port.setEditText(cur)
        self.port.blockSignals(False)

    def port_device(self) -> str:
        i = self.port.currentIndex()
        txt = self.port.currentText()
        if i >= 0 and txt == self.port.itemText(i):
            return self.port.itemData(i) or txt
        return txt.split(" ")[0].strip()

    def _load_voices(self) -> None:
        self.voice_box.blockSignals(True)
        self.voice_box.clear()
        self.voice_box.addItem("Predefinita di sistema", "")
        for vid, name in self.voice.list_voices():
            self.voice_box.addItem(name, vid)
        self.voice_box.blockSignals(False)

    def reload(self) -> None:
        st, c = self.ctl.race.settings, self.cfg
        for w in self.findChildren(QWidget):
            w.blockSignals(True)
        self.title.setText(st.title)
        self.link.setText(st.classifica_link)
        self.out.setText(c.output_dir)
        self.baud.setCurrentIndex(max(0, self.baud.findData(c.baud)))
        self.dtr.setChecked(c.dtr)
        self.fold.setChecked(st.ascii_fold)
        self.back750.setChecked(st.back_uses_750)
        self.guard.setValue(st.guard_seconds)
        self.rwait.setValue(st.reset_wait_seconds)
        self.token.setText(c.telegram_token)
        self.chat.setText(c.telegram_chat_id)
        self.voice_box.setCurrentIndex(max(0, self.voice_box.findData(c.voice_id)))
        self.rate.setValue(c.voice_rate)
        self.second.setValue(st.second_read_seconds)
        if c.port:
            i = self.port.findData(c.port)
            if i >= 0:
                self.port.setCurrentIndex(i)
            else:
                self.port.setEditText(c.port)
        for w in self.findChildren(QWidget):
            w.blockSignals(False)

    def apply(self) -> None:
        st, c = self.ctl.race.settings, self.cfg
        st.title = self.title.text().strip() or st.title
        st.classifica_link = self.link.text().strip()
        c.output_dir = self.out.text().strip()
        c.baud = self.baud.currentData() or 9600
        c.dtr = self.dtr.isChecked()
        c.port = self.port_device()
        st.ascii_fold = self.fold.isChecked()
        st.back_uses_750 = self.back750.isChecked()
        st.guard_seconds = self.guard.value()
        st.reset_wait_seconds = self.rwait.value()
        st.second_read_seconds = self.second.value()
        c.telegram_token = self.token.text().strip()
        c.telegram_chat_id = self.chat.text().strip()
        c.voice_id = self.voice_box.currentData() or ""
        c.voice_rate = self.rate.value()
        self.notifier.configure(c.telegram_token, c.telegram_chat_id)
        self.voice.voice_id, self.voice.rate = c.voice_id, c.voice_rate
        c.save()
        self.ctl.request_autosave()
        self.config_changed.emit()

    def _pick_dir(self) -> None:
        d = QFileDialog.getExistingDirectory(self, "Cartella file generati", self.out.text())
        if d:
            self.out.setText(d)
            self.apply()

    def _test_tg(self) -> None:
        self.apply()
        if not self.notifier.send("Prova da Pylon Race: collegamento Telegram OK ✅"):
            QMessageBox.warning(self, "Telegram", "Inserisci token e chat ID.")

    def _test_voice(self) -> None:
        self.apply()
        self.voice.speak(announce_text(["Mario Rossi", "Luigi Verdi", "Anna Blu"], ["Paolo Neri", "Gina Bianchi", "Carlo Gialli"]))
