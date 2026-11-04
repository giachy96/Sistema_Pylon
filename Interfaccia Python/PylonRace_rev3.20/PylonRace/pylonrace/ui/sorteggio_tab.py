"""Scheda SORTEGGIO: genera l'ordine di volo (equivalente di 'GENERA LISTA VOLI')."""
from __future__ import annotations
from ..i18n import ordinal, tr

from pathlib import Path

from PySide6.QtCore import QUrl, Signal
from PySide6.QtGui import QDesktopServices
from PySide6.QtWidgets import (QComboBox, QFormLayout, QGroupBox, QHBoxLayout, QLabel, QMessageBox, QPlainTextEdit,
                               QPushButton, QSpinBox, QVBoxLayout, QWidget)

from .. import draw, exporters
from ..controller import RaceController
from ..models import CATEGORIES


class OrderPicker(QWidget):
    """Ordine delle categorie: 4 menu a tendina in fila (1ª, 2ª, 3ª, 4ª).

    Una categoria scelta sparisce dalle opzioni degli altri menu; quando ne resta una sola, riempie da sola
    l'unica posizione vuota. Ogni posizione si può svuotare per cambiare idea.
    """

    changed = Signal()
    EMPTY = tr("— scegli —")

    def __init__(self, parent=None):
        super().__init__(parent)
        lay = QHBoxLayout(self)
        lay.setContentsMargins(0, 0, 0, 0)
        self.combos = []
        self.sel = [None] * len(CATEGORIES)
        for i in range(len(CATEGORIES)):
            col = QVBoxLayout()
            col.setSpacing(2)
            col.addWidget(QLabel(ordinal(i + 1)))
            cb = QComboBox()
            cb.setSizeAdjustPolicy(QComboBox.AdjustToContents)
            cb.setMinimumWidth(110)
            cb.currentIndexChanged.connect(lambda _x, k=i: self._picked(k))
            self.combos.append(cb)
            col.addWidget(cb)
            lay.addLayout(col)
        lay.addStretch(1)
        self._rebuild()

    def _rebuild(self) -> None:
        for i, cb in enumerate(self.combos):
            taken = {c for j, c in enumerate(self.sel) if c and j != i}
            cb.blockSignals(True)
            cb.clear()
            cb.addItem(self.EMPTY, None)
            for c in CATEGORIES:
                if c not in taken:
                    cb.addItem(c, c)
            cb.setCurrentIndex(max(0, cb.findData(self.sel[i])))
            cb.blockSignals(False)

    def _picked(self, k: int) -> None:
        self.sel[k] = self.combos[k].currentData()
        left = [c for c in CATEGORIES if c not in self.sel]
        empty = [i for i, c in enumerate(self.sel) if not c]
        if self.sel[k] and len(left) == 1 and len(empty) == 1:   # l'ultima posizione si riempie da sola
            self.sel[empty[0]] = left[0]
        self._rebuild()
        self.changed.emit()

    def set_order(self, order) -> None:
        order = list(order or [])
        self.sel = [None] * len(CATEGORIES)
        if sorted(order) == sorted(CATEGORIES):
            self.sel = order
        self._rebuild()

    def order(self) -> list:
        return [c for c in self.sel if c]

    def is_complete(self) -> bool:
        return len(self.order()) == len(CATEGORIES)

MODES = [
    (draw.MODE_RANDOM, tr("Controllo pilota-meccanico (sorteggio casuale, nessun pilota vola col proprio meccanico)")),
    (draw.MODE_PURE, tr("Casuale, senza controllo pilota-meccanico")),
    (draw.MODE_LIST_ORDER, tr("Mantieni l'ordine dell'elenco piloti"))
]


class DrawTab(QWidget):
    schedule_changed = Signal()

    def __init__(self, ctl: RaceController, out_dir, parent=None):
        super().__init__(parent)
        self.ctl = ctl
        self._out_dir = out_dir
        lay = QVBoxLayout(self)
        lay.setContentsMargins(10, 10, 10, 10)

        box = QGroupBox(tr("Nuovo ordine di volo"))
        form = QFormLayout(box)
        self.spin = QSpinBox()
        self.spin.setRange(2, 12)
        self.spin.setValue(6)
        self.order = OrderPicker()
        self.mode = QComboBox()
        for k, t in MODES:
            self.mode.addItem(t, k)
        form.addRow(tr("Numero di manche (2–12)"), self.spin)
        form.addRow(tr("Ordine delle categorie"), self.order)
        form.addRow(tr("Sorteggio"), self.mode)
        self.btn = QPushButton(tr("Genera ordine di volo"))
        self.btn.setObjectName("primary")
        self.btn.setMinimumHeight(40)
        form.addRow(self.btn)
        lay.addWidget(box)

        self.report = QPlainTextEdit()
        self.report.setReadOnly(True)
        self.report.setPlaceholderText(tr("Il riepilogo del sorteggio comparirà qui."))
        lay.addWidget(self.report, 1)

        ex = QHBoxLayout()
        self.b_xlsx = QPushButton(tr("Esporta Ordine_Volo.xlsx"))
        self.b_pdf = QPushButton(tr("Esporta Ordine_Volo.pdf"))
        self.b_open = QPushButton(tr("Apri cartella"))
        for b in (self.b_xlsx, self.b_pdf, self.b_open):
            ex.addWidget(b)
        ex.addStretch(1)
        lay.addLayout(ex)
        self.hint = QLabel(tr("Controllare la disposizione pilota/meccanico prima di iniziare la gara."))
        self.hint.setObjectName("muted")
        lay.addWidget(self.hint)

        self.btn.clicked.connect(self.generate)
        self.b_xlsx.clicked.connect(lambda: self._export("xlsx"))
        self.b_pdf.clicked.connect(lambda: self._export("pdf"))
        self.b_open.clicked.connect(lambda: QDesktopServices.openUrl(QUrl.fromLocalFile(str(self._out_dir()))))
        self.reload()

    def reload(self) -> None:
        st = self.ctl.race.settings
        self.order.set_order(st.category_order)
        ev = self.ctl.race
        if ev.slots:
            n = max(s.manche for s in ev.slots)
            self.spin.setValue(max(2, n))
            self.report.setPlainText(self._summary(None))
        has = bool(ev.slots)
        self.b_xlsx.setEnabled(has)
        self.b_pdf.setEnabled(has)

    def _summary(self, rep) -> str:
        ev = self.ctl.race
        lines = [tr(f"Ordine di volo: {ev.n_rounds} round, {len(ev.slots)} partenze.")]
        for cat in ev.settings.category_order:
            pil = ev.pilots_of(cat)
            if not pil:
                continue
            sl = [s for s in ev.slots if s.category == cat]
            fill = sum(1 for s in sl if s.dummy and s.manche == 1)
            extra = ""
            if rep is not None:
                c = rep.conflicts.get(cat, 0)
                extra = (tr("  ✔ nessun conflitto pilota/meccanico") if c == 0
                         else tr(f"  ⚠ {c} round con pilota e meccanico insieme (provare di nuovo)"))
            lines.append(tr(f"  {cat}: {len(pil)} piloti") + (tr(f" + {fill} fantasma") if fill else "") + extra)
        if rep is not None and rep.skipped:
            lines.append(tr("Categorie senza piloti (saltate): ") + ", ".join(rep.skipped))
        return "\n".join(lines)

    def generate(self) -> None:
        ev = self.ctl.race
        if not ev.pilots:
            QMessageBox.warning(self, tr("Sorteggio"), tr("Inserisci o importa prima i piloti."))
            return
        if not self.order.is_complete():
            QMessageBox.warning(self, tr("Sorteggio"), tr("Scegli l'ordine di tutte e 4 le categorie (1ª, 2ª, 3ª, 4ª)."))
            return
        if ev.slots:
            with_res = sum(1 for s in ev.slots if s.score is not None)
            msg = tr("Esiste già un ordine di volo.")
            if with_res:
                msg += tr(f"\nVerranno cancellati anche {with_res} risultati già inseriti!")
            r = QMessageBox.question(self, tr("Rigenerare?"), msg + tr("\n\nVuoi sostituirlo?"),
                                     QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            if r != QMessageBox.Yes:
                return
        rep = draw.generate_schedule(ev, self.spin.value(), self.order.order(), self.mode.currentData())
        self.ctl.clear_edit_history()
        self.report.setPlainText(self._summary(rep))
        self.ctl.request_autosave()
        self.ctl.changed.emit()
        self.ctl.round_changed.emit(-1)
        self.schedule_changed.emit()
        self.b_xlsx.setEnabled(True)
        self.b_pdf.setEnabled(True)
        if any(v for v in rep.conflicts.values()):
            QMessageBox.warning(self, tr("Conflitti residui"),
                                tr("Per alcune categorie non è stato possibile evitare tutti i conflitti pilota/meccanico. "
                                "Riprova il sorteggio o riduci il numero di manche."))

    def _export(self, kind: str) -> None:
        out = Path(self._out_dir())
        out.mkdir(parents=True, exist_ok=True)
        try:
            p = (exporters.ordine_volo_xlsx if kind == "xlsx" else exporters.ordine_volo_pdf)(
                self.ctl.race, out / f"Ordine_Volo.{kind}")
        except Exception as exc:
            QMessageBox.critical(self, tr("Esportazione"), tr(f"Non riuscita: {exc}"))
            return
        self.ctl.log.emit(tr(f"Ordine di volo esportato: {p}"), "info")
        QDesktopServices.openUrl(QUrl.fromLocalFile(str(p)))
