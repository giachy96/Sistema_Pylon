"""Scheda GARA: controllo dei round, tabellone, risultati e registro della seriale."""
from __future__ import annotations

import time
from typing import Optional

from PySide6.QtCore import Qt, Signal
from PySide6.QtGui import QColor, QTextCharFormat, QTextCursor
from PySide6.QtWidgets import (QCheckBox, QComboBox, QFrame, QGridLayout, QGroupBox, QHBoxLayout, QLabel, QMessageBox,
                               QPlainTextEdit, QPushButton, QSplitter, QTableView, QVBoxLayout, QWidget, QHeaderView)

from ..controller import RaceController
from ..links import SimulatedCentral
from ..models import COLOR_NAMES, N_LAPS, Slot
from . import theme
from .tables import ScheduleFilter, ScheduleModel


class PilotCard(QFrame):
    def __init__(self, color: int, parent=None):
        super().__init__(parent)
        self.color = color
        self.setObjectName("card")
        self.setStyleSheet(
            f"QFrame#card {{ background: {theme.COLOR_SOFT[color]}; border: 2px solid {theme.COLOR_ACCENT[color]}; border-radius: 10px; }}"
            "QLabel { background: transparent; }"
        )
        lay = QVBoxLayout(self)
        lay.setContentsMargins(12, 8, 12, 8)
        lay.setSpacing(2)
        self.tag = QLabel(COLOR_NAMES[color].upper())
        self.tag.setStyleSheet(f"color:{theme.COLOR_ACCENT[color]}; font-weight:700; letter-spacing:1px;")
        self.name = QLabel("—")
        self.name.setStyleSheet("font-size:17pt; font-weight:700; color:#111827;")
        self.name.setWordWrap(True)
        self.mech = QLabel("")
        self.mech.setObjectName("muted")
        self.result = QLabel("")
        self.result.setStyleSheet("font-weight:600; color:#111827;")
        for w in (self.tag, self.name, self.mech, self.result):
            lay.addWidget(w)

    def show_slot(self, s: Optional[Slot]) -> None:
        if s is None:
            self.name.setText("—")
            self.mech.setText("")
            self.result.setText("")
            return
        self.name.setText(s.full_name or "—")
        self.mech.setText(f"Meccanico: {s.mech}" if s.mech else "")
        if s.score is not None:
            cuts = s.cuts or 0
            txt = f"Totale {s.total or '/'}  ·  {cuts} tagli  ·  Punti {s.score:g}"
            col = theme.OK if cuts == 0 else theme.WARN
            self.result.setStyleSheet(f"font-weight:700; color:{col};")
            self.result.setText(txt)
        elif s.needs_check:
            self.result.setStyleSheet(f"font-weight:700; color:{theme.ERR};")
            self.result.setText("Totale mancante: inserire a mano")
        else:
            self.result.setText("")


class LogView(QPlainTextEdit):
    COLORS = {"tx": "#7DD3FC", "rx": "#9CA3AF", "info": "#E5E7EB", "warn": "#FBBF24", "error": "#F87171"}
    PREFIX = {"tx": "→ ", "rx": "← ", "info": "", "warn": "⚠ ", "error": "✖ "}

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("log")
        self.setReadOnly(True)
        self.setMaximumBlockCount(2000)

    def add(self, text: str, level: str = "info") -> None:
        fmt = QTextCharFormat()
        fmt.setForeground(QColor(self.COLORS.get(level, "#E5E7EB")))
        cur = self.textCursor()
        cur.movePosition(QTextCursor.End)
        stamp = time.strftime("%H:%M:%S")
        cur.insertText(f"{stamp}  {self.PREFIX.get(level, '')}{text}\n", fmt)
        self.setTextCursor(cur)
        self.ensureCursorVisible()


class RaceTab(QWidget):
    status_message = Signal(str)

    def __init__(self, ctl: RaceController, parent=None):
        super().__init__(parent)
        self.ctl = ctl
        self.model = ScheduleModel(ctl, self)
        self.proxy = ScheduleFilter(self)
        self.proxy.setSourceModel(self.model)
        self._build()
        ctl.changed.connect(self.refresh)
        ctl.round_changed.connect(self._on_round)
        ctl.log.connect(self.log.add)
        self.refresh()

    # ------------------------------------------------------------------ costruzione
    def _build(self) -> None:
        root = QHBoxLayout(self)
        root.setContentsMargins(10, 10, 10, 10)

        # ---- colonna sinistra: pannello di controllo
        left = QVBoxLayout()
        self.round_label = QLabel("Nessun round selezionato")
        self.round_label.setObjectName("h1")
        self.round_sub = QLabel("")
        self.round_sub.setObjectName("muted")
        left.addWidget(self.round_label)
        left.addWidget(self.round_sub)
        self.cards = [PilotCard(i) for i in range(3)]
        for c in self.cards:
            left.addWidget(c)
        self.refuel = QLabel("")
        self.refuel.setWordWrap(True)
        self.refuel.setStyleSheet(
            f"background:{theme.REFUEL_BG}; border:2px solid #E0B000; border-radius:10px; padding:8px 12px; font-weight:600;")
        left.addWidget(self.refuel)

        nav = QHBoxLayout()
        self.btn_prev = QPushButton("◀  INDIETRO")
        self.btn_prev.setObjectName("nav")
        self.btn_next = QPushButton("AVANTI  ▶")
        self.btn_next.setObjectName("nav")
        self.btn_next.setProperty("accent", True)
        self.btn_next.setStyleSheet("QPushButton#nav{background:#1F6FEB;color:white;border-color:#1A5FCC;}"
                                    "QPushButton#nav:hover{background:#1A5FCC;}"
                                    "QPushButton#nav:disabled{background:#A9C4F2;}")
        nav.addWidget(self.btn_prev)
        nav.addWidget(self.btn_next, 1)
        left.addLayout(nav)

        cmd = QHBoxLayout()
        self.btn_show, self.btn_start, self.btn_stop = (QPushButton(t) for t in ("SHOW", "START", "STOP"))
        for b, n in ((self.btn_show, "cmdShow"), (self.btn_start, "cmdStart"), (self.btn_stop, "cmdStop")):
            b.setObjectName(n)
            b.setMinimumHeight(44)
            cmd.addWidget(b)
        left.addLayout(cmd)

        row = QHBoxLayout()
        self.btn_here = QPushButton("Parti dal round selezionato")
        self.btn_reset = QPushButton("Reset selezione")
        row.addWidget(self.btn_here)
        row.addWidget(self.btn_reset)
        left.addLayout(row)

        opts = QHBoxLayout()
        self.chk_tg = QCheckBox("Telegram")
        self.chk_voice = QCheckBox("Voce")
        opts.addWidget(self.chk_tg)
        opts.addWidget(self.chk_voice)
        opts.addStretch(1)
        left.addLayout(opts)

        # pannello del simulatore (visibile solo con la centrale simulata)
        self.sim_box = QGroupBox("Centrale simulata — prova a tavolino")
        sg = QGridLayout(self.sim_box)
        self.sim_buttons = {}
        for i, (key, text) in enumerate((
            ("next", "Centrale: AVANTI"), ("prev", "Centrale: INDIETRO"),
            ("ok", "Fine: tutti regolari"), ("cut", "Fine: 1 taglio (verde)"),
            ("dnf", "Fine: timeout (blu)"), ("dbl", "Fine: doppio taglio (rosso)"))):
            b = QPushButton(text)
            self.sim_buttons[key] = b
            sg.addWidget(b, i // 2, i % 2)
        left.addWidget(self.sim_box)
        self.sim_box.hide()
        left.addStretch(1)

        lw = QWidget()
        lw.setLayout(left)
        lw.setFixedWidth(430)

        # ---- colonna destra: tabella + registro
        right = QVBoxLayout()
        flt = QHBoxLayout()
        flt.addWidget(QLabel("Mostra:"))
        self.cmb_cat = QComboBox()
        self.cmb_man = QComboBox()
        flt.addWidget(self.cmb_cat)
        flt.addWidget(self.cmb_man)
        self.chk_follow = QCheckBox("Segui il round corrente")
        self.chk_follow.setChecked(True)
        flt.addWidget(self.chk_follow)
        flt.addStretch(1)
        self.count_label = QLabel("")
        self.count_label.setObjectName("muted")
        flt.addWidget(self.count_label)
        right.addLayout(flt)

        self.table = QTableView()
        self.table.setModel(self.proxy)
        self.table.setSelectionBehavior(QTableView.SelectRows)
        self.table.verticalHeader().setVisible(False)
        self.table.verticalHeader().setDefaultSectionSize(26)
        hh = self.table.horizontalHeader()
        hh.setSectionResizeMode(QHeaderView.Interactive)
        hh.setStretchLastSection(False)
        widths = [52, 62, 56, 28, 190, 46, 170] + [64] * N_LAPS + [76, 52, 72]
        for i, w in enumerate(widths):
            self.table.setColumnWidth(i, w)
        self.table.setEditTriggers(QTableView.DoubleClicked | QTableView.EditKeyPressed)
        self.table.doubleClicked.connect(self._double_clicked)

        self.log = LogView()
        self.log.setMaximumHeight(170)
        split = QSplitter(Qt.Vertical)
        split.addWidget(self.table)
        split.addWidget(self.log)
        split.setStretchFactor(0, 5)
        split.setStretchFactor(1, 1)
        right.addWidget(split, 1)

        rw = QWidget()
        rw.setLayout(right)
        root.addWidget(lw)
        root.addWidget(rw, 1)

        # ---- segnali
        self.btn_next.clicked.connect(self._next)
        self.btn_prev.clicked.connect(lambda: self.ctl.go_prev("pc"))
        self.btn_show.clicked.connect(self.ctl.send_show)
        self.btn_start.clicked.connect(self.ctl.send_start)
        self.btn_stop.clicked.connect(self.ctl.send_stop)
        self.btn_here.clicked.connect(self._jump_here)
        self.btn_reset.clicked.connect(self._reset)
        self.chk_tg.toggled.connect(self._set_tg)
        self.chk_voice.toggled.connect(self._set_voice)
        self.cmb_cat.currentIndexChanged.connect(self._apply_filter)
        self.cmb_man.currentIndexChanged.connect(self._apply_filter)

    # ------------------------------------------------------------------ azioni
    def _next(self) -> None:
        if self.ctl.race.current < 0 and not self.ctl.connected:
            r = QMessageBox.question(
                self, "Centrale non collegata",
                "La centrale non è collegata al PC.\n\nVuoi avanzare comunque (solo a schermo, senza inviare i nomi)?",
                QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            if r != QMessageBox.Yes:
                return
        self.ctl.go_next("pc")

    def _jump_here(self) -> None:
        idx = self.table.currentIndex()
        if not idx.isValid():
            self.status_message.emit("Seleziona prima una riga della tabella")
            return
        row = self.proxy.mapToSource(idx).row()
        rnd = row // 3
        s = self.ctl.race.round_slots(rnd)[0]
        r = QMessageBox.question(self, "Selezione",
                                 f"Ripartire da {s.category} · manche {s.manche} · round {s.round}?",
                                 QMessageBox.Ok | QMessageBox.Cancel)
        if r == QMessageBox.Ok:
            self.ctl.jump_to(rnd)

    def _reset(self) -> None:
        r = QMessageBox.question(self, "Reset", "Azzerare la selezione del round corrente?\n(I risultati non vengono cancellati)")
        if r == QMessageBox.Yes:
            self.ctl.reset_selection()

    def _set_tg(self, on: bool) -> None:
        self.ctl.race.settings.telegram_on = on
        self.ctl.request_autosave()

    def _set_voice(self, on: bool) -> None:
        self.ctl.race.settings.voice_on = on
        self.ctl.request_autosave()

    def _double_clicked(self, idx) -> None:
        pass  # la modifica è gestita dall'editor della tabella

    # ------------------------------------------------------------------ simulatore
    def attach_simulator(self, link: Optional[SimulatedCentral]) -> None:
        for b in self.sim_buttons.values():
            try:
                b.clicked.disconnect()
            except (RuntimeError, TypeError):
                pass
        self.sim_box.setVisible(link is not None)
        if link is None:
            return
        sb = self.sim_buttons
        sb["next"].clicked.connect(link.press_next)
        sb["prev"].clicked.connect(link.press_prev)
        sb["ok"].clicked.connect(lambda: link.finish_round([("normal", 0)] * 3))
        sb["cut"].clicked.connect(lambda: link.finish_round([("normal", 0), ("normal", 1), ("normal", 0)]))
        sb["dnf"].clicked.connect(lambda: link.finish_round([("normal", 0), ("normal", 0), ("timeout", 0)]))
        sb["dbl"].clicked.connect(lambda: link.finish_round([("double_cut", 2), ("normal", 0), ("normal", 0)]))

    # ------------------------------------------------------------------ aggiornamento viste
    def reload(self) -> None:
        self.chk_tg.blockSignals(True)
        self.chk_voice.blockSignals(True)
        self.chk_tg.setChecked(self.ctl.race.settings.telegram_on)
        self.chk_voice.setChecked(self.ctl.race.settings.voice_on)
        self.chk_tg.blockSignals(False)
        self.chk_voice.blockSignals(False)
        self._fill_filters()
        self.refresh()

    def _fill_filters(self) -> None:
        ev = self.ctl.race
        cats = []
        for s in ev.slots:
            if s.category not in cats:
                cats.append(s.category)
        prev_c, prev_m = self.cmb_cat.currentData(), self.cmb_man.currentData()
        for cb in (self.cmb_cat, self.cmb_man):
            cb.blockSignals(True)
            cb.clear()
        self.cmb_cat.addItem("Tutte le categorie", None)
        for c in cats:
            self.cmb_cat.addItem(c, c)
        self.cmb_man.addItem("Tutte le manche", None)
        for m in sorted({s.manche for s in ev.slots}):
            self.cmb_man.addItem(f"Manche {m}", m)
        for cb, prev in ((self.cmb_cat, prev_c), (self.cmb_man, prev_m)):
            i = cb.findData(prev)
            cb.setCurrentIndex(max(0, i))
            cb.blockSignals(False)
        self._apply_filter()

    def _apply_filter(self) -> None:
        self.proxy.set_filter(self.cmb_cat.currentData(), self.cmb_man.currentData())

    def refresh(self) -> None:
        ev = self.ctl.race
        self.model.refresh()
        if self.cmb_cat.count() <= 1 and ev.slots:
            self._fill_filters()
        cur = ev.current
        slots = ev.current_slots()
        if slots:
            s0 = slots[0]
            self.round_label.setText(f"{s0.category}  ·  Manche {s0.manche}  ·  Round {s0.round}")
            self.round_sub.setText(f"Round {cur + 1} di {ev.n_rounds}")
            for card, s in zip(self.cards, slots):
                card.show_slot(s)
            ref = ev.refuel_slots(cur)
            self.refuel.setText("Al rifornimento: " + (" · ".join(s.full_name for s in ref) if ref else "—"))
        else:
            self.round_label.setText("Nessun round selezionato")
            self.round_sub.setText(f"{ev.n_rounds} round in programma" if ev.n_rounds else "Genera l'ordine di volo dalla scheda Sorteggio")
            for card in self.cards:
                card.show_slot(None)
            self.refuel.setText("Al rifornimento: —")
        has = ev.n_rounds > 0
        self.btn_next.setEnabled(has and cur + 1 < ev.n_rounds)
        self.btn_prev.setEnabled(cur > 0)
        self.btn_here.setEnabled(has)
        done = sum(1 for s in ev.slots if s.score is not None and not s.dummy)
        total = sum(1 for s in ev.slots if not s.dummy)
        self.count_label.setText(f"{done}/{total} risultati inseriti" if total else "")

    def _on_round(self, idx: int) -> None:
        if idx < 0 or not self.chk_follow.isChecked():
            return
        s = self.ctl.race.slots[3 * idx]
        # se il filtro nasconde il round corrente, allinealo
        if self.proxy.category and self.proxy.category != s.category:
            self.cmb_cat.setCurrentIndex(max(0, self.cmb_cat.findData(s.category)))
        if self.proxy.manche and self.proxy.manche != s.manche:
            self.cmb_man.setCurrentIndex(max(0, self.cmb_man.findData(s.manche)))
        src = self.model.index(3 * idx, 4)
        pidx = self.proxy.mapFromSource(src)
        if pidx.isValid():
            self.table.scrollTo(pidx, QTableView.PositionAtCenter)
            self.table.selectRow(pidx.row())
