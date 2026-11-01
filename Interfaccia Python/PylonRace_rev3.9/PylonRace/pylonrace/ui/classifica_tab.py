"""Scheda CLASSIFICA."""
from __future__ import annotations

from pathlib import Path

from PySide6.QtCore import Qt, QUrl
from PySide6.QtGui import QBrush, QColor, QDesktopServices, QFont
from PySide6.QtWidgets import (QCheckBox, QComboBox, QHBoxLayout, QHeaderView, QLabel, QMessageBox, QPushButton, QSpinBox,
                               QTableWidget, QTableWidgetItem, QTabWidget, QVBoxLayout, QWidget)

from .. import exporters, standings, telegram_bot
from ..controller import RaceController
from . import theme


class StandingsTab(QWidget):
    def __init__(self, ctl: RaceController, out_dir, parent=None):
        super().__init__(parent)
        self.ctl = ctl
        self._out_dir = out_dir
        lay = QVBoxLayout(self)
        lay.setContentsMargins(10, 10, 10, 10)
        bar = QHBoxLayout()
        self.btn = QPushButton("Genera classifica")
        self.btn.setObjectName("primary")
        self.chk_fai = QCheckBox("Separa FAI / non FAI")
        self.lbl_auto = QLabel("Aggiornamento automatico:")
        self.cmb_auto = QComboBox()
        self.cmb_auto.addItem("Solo a mano", "off")
        self.cmb_auto.addItem("A fine di ogni manche", "manche")
        self.cmb_auto.addItem("A fine di ogni categoria della manche", "category")
        self.cmb_auto.addItem("Ogni N minuti", "timer")
        self.cmb_auto.setToolTip("Quando generare il PDF e inviarlo su Telegram. Dopo la conferma di un reflight la "
                                 "classifica si aggiorna nelle modalità 'manche' e 'categoria'.")
        self.cmb_auto.setSizeAdjustPolicy(QComboBox.AdjustToContents)
        self.spin = QSpinBox()
        self.spin.setRange(1, 60)
        self.spin.setSuffix(" min")
        self.b_pdf = QPushButton("PDF")
        self.b_xlsx = QPushButton("Excel")
        self.b_tg = QPushButton("Telegram")
        for w in (self.btn, self.chk_fai, self.lbl_auto, self.cmb_auto, self.spin):
            bar.addWidget(w)
        bar.addStretch(1)
        for w in (self.b_tg, self.b_pdf, self.b_xlsx):
            bar.addWidget(w)
        lay.addLayout(bar)
        self.tabs = QTabWidget()
        lay.addWidget(self.tabs, 1)
        self.legend = QLabel(
            f'<span style="background:{theme.CUT1_BG}">&nbsp;1 taglio&nbsp;</span>&nbsp; '
            f'<span style="background:{theme.CUT2_BG}">&nbsp;2 tagli&nbsp;</span>&nbsp; '
            f'<span style="background:{theme.BEST_BG}">&nbsp;miglior tempo&nbsp;</span>&nbsp; '
            "<s>manche scartata</s>")
        lay.addWidget(self.legend)

        self.btn.clicked.connect(self.generate)
        self.chk_fai.toggled.connect(self._fai)
        self.cmb_auto.currentIndexChanged.connect(self._auto)
        self.spin.valueChanged.connect(self._auto)
        self.b_pdf.clicked.connect(lambda: self._export("pdf"))
        self.b_xlsx.clicked.connect(lambda: self._export("xlsx"))
        self.b_tg.clicked.connect(self._telegram)
        ctl.changed.connect(self.rebuild)
        self.reload()

    def reload(self) -> None:
        st = self.ctl.race.settings
        self.chk_fai.blockSignals(True)
        self.chk_fai.setChecked(st.fai_split)
        self.chk_fai.blockSignals(False)
        self.cmb_auto.blockSignals(True)
        self.cmb_auto.setCurrentIndex(max(0, self.cmb_auto.findData(st.classifica_mode)))
        self.cmb_auto.blockSignals(False)
        self.spin.setVisible(st.classifica_mode == "timer")
        self.spin.blockSignals(True)
        self.spin.setValue(st.classifica_minutes)
        self.spin.blockSignals(False)
        self.rebuild()

    def _fai(self, on: bool) -> None:
        self.ctl.race.settings.fai_split = on
        self.ctl.request_autosave()
        self.rebuild()

    def _auto(self, *_):
        st = self.ctl.race.settings
        st.classifica_mode = self.cmb_auto.currentData() or "off"
        st.auto_classifica = st.classifica_mode == "timer"
        st.classifica_minutes = self.spin.value()
        self.spin.setVisible(st.classifica_mode == "timer")
        self.ctl.apply_auto_classifica()
        self.ctl.request_autosave()

    def generate(self) -> None:
        self.rebuild()
        p = self.ctl.generate_classifica(notify=True)
        if p is None:
            QMessageBox.information(self, "Classifica", "Nessun dato per generare la classifica.")

    def rebuild(self) -> None:
        results = standings.compute_all(self.ctl.race)
        cur_cat = self.tabs.tabText(self.tabs.currentIndex()) if self.tabs.count() else ""
        self.tabs.clear()
        for cs in results:
            t = QTableWidget()
            self._fill(t, cs)
            label = cs.category + (f"  ({cs.discards} scart{'o' if cs.discards == 1 else 'i'})" if cs.discards else "")
            self.tabs.addTab(t, label)
        for i in range(self.tabs.count()):
            if self.tabs.tabText(i).startswith(cur_cat) and cur_cat:
                self.tabs.setCurrentIndex(i)

    @staticmethod
    def _fill(t: QTableWidget, cs: standings.CategoryStandings) -> None:
        n = cs.n_manches
        head = ["Pos", "Naz", "FAI", "Nome", "Cognome"] + [f"M{i}" for i in range(1, n + 1)] + ["Best", "Totale", "ABS"]
        t.setColumnCount(len(head))
        t.setHorizontalHeaderLabels(head)
        t.setRowCount(len(cs.rows))
        t.verticalHeader().setVisible(False)
        t.setEditTriggers(QTableWidget.NoEditTriggers)
        t.setAlternatingRowColors(True)

        def put(r, c, text, center=True, bg=None, strike=False, bold=False):
            it = QTableWidgetItem(text)
            it.setTextAlignment(Qt.AlignCenter if center else Qt.AlignLeft | Qt.AlignVCenter)
            if bg:
                it.setBackground(QBrush(QColor(bg)))
            if strike or bold:
                f = QFont()
                f.setStrikeOut(strike)
                f.setBold(bold)
                it.setFont(f)
            t.setItem(r, c, it)

        for r, row in enumerate(cs.rows):
            p = row.pilot
            put(r, 0, "" if row.place is None else str(row.place), bold=True)
            put(r, 1, p.nation)
            put(r, 2, p.fai)
            put(r, 3, p.name, center=False)
            put(r, 4, p.surname, center=False)
            for i in range(n):
                v = row.scores[i]
                bg = None
                if v is not None and row.cuts[i]:
                    bg = theme.CUT1_BG if row.cuts[i] == 1 else theme.CUT2_BG
                put(r, 5 + i, "" if v is None else f"{v:g}", bg=bg, strike=bool(v is not None and row.struck[i]))
            put(r, 5 + n, "" if row.best is None else f"{row.best:g}", bg=theme.BEST_BG if row.best_overall else None)
            put(r, 6 + n, f"{row.total:g}" if row.total else "", bold=True)
            put(r, 7 + n, str(row.abs_score) if row.total else "", bold=True)
        t.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeToContents)
        t.horizontalHeader().setStretchLastSection(False)

    def _export(self, kind: str) -> None:
        if not self.ctl.has_any_result():
            QMessageBox.information(self, "Classifica", "Nessun dato per generare la classifica.")
            return
        out = Path(self._out_dir())
        out.mkdir(parents=True, exist_ok=True)
        res = standings.compute_all(self.ctl.race)
        try:
            p = (exporters.classifica_pdf if kind == "pdf" else exporters.classifica_xlsx)(
                self.ctl.race, res, out / f"Classifica.{kind}")
        except Exception as exc:
            QMessageBox.critical(self, "Esportazione", f"Non riuscita: {exc}")
            return
        self.ctl.log.emit(f"Classifica esportata: {p}", "info")
        QDesktopServices.openUrl(QUrl.fromLocalFile(str(p)))

    def _telegram(self) -> None:
        """Genera il PDF aggiornato e lo manda su Telegram (o solo il testo, se così è impostato)."""
        n = self.ctl.notifier
        if n is None or not getattr(n, "ready", True):
            self.ctl.log.emit("Telegram non configurato", "warn")
            return
        if self.ctl.has_any_result():
            self.ctl.generate_classifica(notify=True, force_telegram=True)
        else:
            n.send(telegram_bot.format_ranking(self.ctl.race.settings.classifica_link, self.ctl.tg_templates))
