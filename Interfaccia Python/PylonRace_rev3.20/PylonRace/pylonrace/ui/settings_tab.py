"""Scheda IMPOSTAZIONI."""
from __future__ import annotations
from ..i18n import LANGUAGES, tr

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
        cols = QHBoxLayout(inner)
        cols.setContentsMargins(10, 10, 10, 10)
        cols.setSpacing(14)
        left, right = QVBoxLayout(), QVBoxLayout()
        cols.addLayout(left, 1)
        cols.addLayout(right, 1)
        outer = left

        # ---- evento
        g = QGroupBox(tr("Gara"))
        f = QFormLayout(g)
        self.title = QLineEdit()
        f.addRow(tr("Titolo (intestazioni e PDF)"), self.title)
        self.link = QLineEdit()
        self.link.setPlaceholderText(tr("link mostrato su Telegram con la classifica (es. Google Drive)"))
        f.addRow(tr("Link classifica"), self.link)
        self.pdf_tg = QCheckBox(tr("Invia il PDF della classifica su Telegram"))
        self.pdf_tg.setToolTip(tr("Se spento, su Telegram arriva solo il testo con il link classifica"))
        f.addRow("", self.pdf_tg)
        self.out = QLineEdit()
        b = QPushButton(tr("Sfoglia…"))
        b.clicked.connect(self._pick_dir)
        row = QHBoxLayout()
        row.addWidget(self.out, 1)
        row.addWidget(b)
        f.addRow(tr("Cartella file generati"), row)
        outer.addWidget(g)

        # ---- centrale
        g = QGroupBox(tr("Centrale (porta seriale)"))
        f = QFormLayout(g)
        self.port = QComboBox()
        self.port.setEditable(True)
        b = QPushButton(tr("Aggiorna elenco"))
        b.clicked.connect(self.refresh_ports)
        row = QHBoxLayout()
        row.addWidget(self.port, 1)
        row.addWidget(b)
        f.addRow(tr("Porta"), row)
        self.baud = QComboBox()
        for v in (9600, 19200, 38400, 57600, 115200):
            self.baud.addItem(str(v), v)
        f.addRow(tr("Baud rate"), self.baud)
        self.dtr = QCheckBox(tr("Attiva DTR all'apertura (come Data Streamer)"))
        f.addRow("", self.dtr)
        self.fold = QCheckBox(tr("Invia i nomi senza accenti (il display LCD non li mostra)"))
        f.addRow("", self.fold)
        self.back750 = QCheckBox(tr("Usa il codice 750 per INDIETRO (Excel usava sempre 850)"))
        f.addRow("", self.back750)
        self.guard = QDoubleSpinBox()
        self.guard.setRange(0, 60)
        self.guard.setSuffix(" s")
        f.addRow(tr("Pausa dopo i risultati"), self.guard)
        self.rwait = QDoubleSpinBox()
        self.rwait.setRange(0.5, 30)
        self.rwait.setSuffix(" s")
        f.addRow(tr("Attesa dopo il reset centrale"), self.rwait)
        outer.addWidget(g)

        outer.addStretch(1)
        outer = right

        # ---- telegram
        g = QGroupBox("Telegram")
        f = QFormLayout(g)
        self.token = QLineEdit()
        self.token.setEchoMode(QLineEdit.Password)
        self.token.setPlaceholderText(tr("token del bot (da @BotFather)"))
        self.chat = QLineEdit()
        self.chat.setPlaceholderText(tr("es. -1001234567890"))
        f.addRow(tr("Token bot"), self.token)
        f.addRow(tr("Chat ID"), self.chat)
        self.b_tg = QPushButton(tr("Invia messaggio di prova"))
        self.b_tpl = QPushButton(tr("Configura i messaggi…"))
        self.b_tpl.setToolTip(tr("Cambia il testo dei messaggi inviati su Telegram"))
        trow = QHBoxLayout()
        trow.addWidget(self.b_tg)
        trow.addWidget(self.b_tpl)
        f.addRow("", trow)
        outer.addWidget(g)

        # ---- voce
        g = QGroupBox(tr("Voce"))
        f = QFormLayout(g)
        self.voice_box = QComboBox()
        self.rate = QSpinBox()
        self.rate.setRange(80, 300)
        self.second = QDoubleSpinBox()
        self.second.setRange(0, 300)
        self.second.setSuffix(" s")
        self.b_voice = QPushButton(tr("Prova voce"))
        f.addRow(tr("Voce di sistema"), self.voice_box)
        f.addRow(tr("Velocità"), self.rate)
        f.addRow(tr("Seconda lettura dopo"), self.second)
        f.addRow("", self.b_voice)
        outer.addWidget(g)

        # ---- lingua (la scritta resta bilingue: chi non capisce la lingua attuale deve trovarla)
        g = QGroupBox(tr("Lingua") + (" / Language" if tr("Lingua") == "Lingua" else " / Lingua"))
        hl = QHBoxLayout(g)
        self.lang = QComboBox()
        for code, name in LANGUAGES.items():
            self.lang.addItem(name, code)
        self.lang.setCurrentIndex(max(0, self.lang.findData(cfg.language)))
        self.lang.setToolTip(tr("La nuova lingua vale dal prossimo avvio del programma"))
        hl.addWidget(self.lang, 1)
        outer.addWidget(g)
        self.lang.currentIndexChanged.connect(self._lang_changed)

        # ---- aiuto
        g = QGroupBox(tr("Aiuto"))
        hl = QHBoxLayout(g)
        self.btn_manual = QPushButton(tr("Apri il manuale utente"))
        self.btn_manual.setToolTip(tr("Apre il manuale in PDF (anche con F1)"))
        hl.addWidget(self.btn_manual)
        outer.addWidget(g)
        outer.addStretch(1)
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setFrameShape(QFrame.NoFrame)
        scroll.setWidget(inner)
        root = QVBoxLayout(self)
        root.setContentsMargins(0, 0, 0, 0)
        root.addWidget(scroll)

        # salvataggio proprio: apply() non deve mai riscrivere questo valore (all'avvio la casella è ancora vuota)
        self.pdf_tg.toggled.connect(self._pdf_changed)
        self.b_tg.clicked.connect(self._test_tg)
        self.b_tpl.clicked.connect(self._edit_templates)
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
        self.voice_box.addItem(tr("Predefinita di sistema"), "")
        for vid, name in self.voice.list_voices():
            self.voice_box.addItem(name, vid)
        self.voice_box.blockSignals(False)

    def _edit_templates(self) -> None:
        from .telegram_dialog import TelegramTemplatesDialog

        dlg = TelegramTemplatesDialog(self.ctl, self.cfg.telegram_templates, self)
        if dlg.exec():
            self.cfg.save()
            self.ctl.log.emit(tr("Testi dei messaggi Telegram aggiornati"), "info")

    def reload(self) -> None:
        st, c = self.ctl.race.settings, self.cfg
        for w in self.findChildren(QWidget):
            w.blockSignals(True)
        self.title.setText(st.title)
        self.link.setText(st.classifica_link)
        self.pdf_tg.setChecked(st.classifica_pdf_telegram)
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

    def _lang_changed(self, _i: int) -> None:
        code = self.lang.currentData()
        if code == self.cfg.language:
            return
        self.cfg.language = code
        self.cfg.save()
        QMessageBox.information(
            self, "Lingua / Language",
            "La nuova lingua verrà usata al prossimo avvio del programma.\n\n"
            "The new language will be used the next time you start the program.")

    def _pdf_changed(self, on: bool) -> None:
        self.ctl.race.settings.classifica_pdf_telegram = on
        self.ctl.request_autosave()

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
        d = QFileDialog.getExistingDirectory(self, tr("Cartella file generati"), self.out.text())
        if d:
            self.out.setText(d)
            self.apply()

    def _test_tg(self) -> None:
        self.apply()
        if not self.notifier.send(tr("Prova da Pylon Race: collegamento Telegram OK ✅")):
            QMessageBox.warning(self, "Telegram", tr("Inserisci token e chat ID."))

    def _test_voice(self) -> None:
        self.apply()
        self.voice.speak(announce_text(["Mario Rossi", "Luigi Verdi", "Anna Blu"], ["Paolo Neri", "Gina Bianchi", "Carlo Gialli"]))
