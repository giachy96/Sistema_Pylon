"""Finestra per modificare i testi dei messaggi Telegram."""
from __future__ import annotations

from PySide6.QtWidgets import (QDialog, QDialogButtonBox, QHBoxLayout, QLabel, QMessageBox, QPlainTextEdit, QPushButton,
                               QScrollArea, QVBoxLayout, QWidget, QFrame)

from .. import telegram_bot as tb
from ..models import Slot


def _sample_slots(ctl):
    """Tre partenti per l'anteprima: il round corrente (o il primo) oppure dati d'esempio."""
    ev = ctl.race
    if ev.n_rounds:
        rnd = ev.current if ev.current >= 0 else 0
        return ev.round_slots(rnd), ev.refuel_slots(rnd)
    names = [("Mario", "Rossi"), ("John", "Smith"), ("Hans", "Müller"), ("Jean", "Dupont")]
    sl = [Slot(category="Q40", manche=1, round=1, color=i, pilot_id=f"T{i}", name=names[i][0], surname=names[i][1],
               laps=[f"{5 + i}.{n}00" for n in range(10)], total="60.123", cuts=0, score=60.123) for i in range(3)]
    return sl, [Slot(category="Q40", manche=1, round=2, color=0, name="Jean", surname="Dupont")]


class TelegramTemplatesDialog(QDialog):
    def __init__(self, ctl, templates: dict, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Messaggi Telegram")
        self.resize(900, 700)
        self.ctl = ctl
        self.templates = templates
        self.edits = {}
        root = QVBoxLayout(self)
        info = QLabel("Modifica i testi dei messaggi. I segnaposto tra parentesi graffe, come {gara}, vengono sostituiti "
                      "con i dati del round. Puoi usare emoji e andare a capo liberamente. "
                      "Un testo lasciato vuoto torna a quello predefinito.")
        info.setWordWrap(True)
        root.addWidget(info)

        inner = QWidget()
        lay = QVBoxLayout(inner)
        for key in tb.DEFAULT_TEMPLATES:
            lay.addWidget(QLabel(f"<b>{tb.TEMPLATE_TITLES[key]}</b>"))
            e = QPlainTextEdit(tb.tpl(templates, key))
            e.setFixedHeight(self.fontMetrics().height() * {"start": 9, "results_pilot": 7, "results_header": 5}.get(key, 3) + 16)
            e.textChanged.connect(self._preview)
            self.edits[key] = e
            lay.addWidget(e)
            h = QLabel(f"Segnaposto: {tb.PLACEHOLDERS[key]}")
            h.setStyleSheet("color:#4B5563;")
            h.setWordWrap(True)
            lay.addWidget(h)
        lay.addWidget(QLabel("<b>Anteprima</b> (con i dati del round attuale)"))
        self.prev = QPlainTextEdit()
        self.prev.setReadOnly(True)
        self.prev.setMinimumHeight(self.fontMetrics().height() * 14)
        lay.addWidget(self.prev)
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setFrameShape(QFrame.NoFrame)
        scroll.setWidget(inner)
        root.addWidget(scroll, 1)

        row = QHBoxLayout()
        b_def = QPushButton("Ripristina i testi predefiniti")
        b_def.clicked.connect(self._reset)
        row.addWidget(b_def)
        row.addStretch(1)
        box = QDialogButtonBox(QDialogButtonBox.Save | QDialogButtonBox.Cancel)
        box.button(QDialogButtonBox.Save).setText("Salva")
        box.button(QDialogButtonBox.Cancel).setText("Annulla")
        box.accepted.connect(self.accept)
        box.rejected.connect(self.reject)
        row.addWidget(box)
        root.addLayout(row)
        self._preview()

    def current(self) -> dict:
        return {k: e.toPlainText() for k, e in self.edits.items()}

    def _preview(self) -> None:
        t = self.current()
        try:
            sl, ref = _sample_slots(self.ctl)
            start = tb.format_start_line(sl, ref, False, t)
            back = tb.format_start_line(sl, ref, True, t).split(start)[0]
            res = tb.format_results(sl, t)
            rank = tb.format_ranking("https://esempio.it/classifica", t)
        except Exception as exc:  # non dovrebbe succedere: i segnaposto errati restano scritti così come sono
            self.prev.setPlainText(f"Anteprima non disponibile: {exc}")
            return
        sep = "\n" + "─" * 40 + "\n"
        self.prev.setPlainText(start + sep + (back + "…" if back else "") + sep + res + sep + rank)

    def _reset(self) -> None:
        if QMessageBox.question(self, "Messaggi Telegram", "Ripristinare tutti i testi predefiniti?") != QMessageBox.Yes:
            return
        for k, e in self.edits.items():
            e.setPlainText(tb.DEFAULT_TEMPLATES[k])

    def accept(self) -> None:
        cur = self.current()
        # si salvano solo i testi diversi dal predefinito (così gli aggiornamenti futuri dei predefiniti si applicano)
        self.templates.clear()
        self.templates.update({k: v for k, v in cur.items() if v.strip() and v != tb.DEFAULT_TEMPLATES[k]})
        super().accept()
