"""Scheda GARA: controllo dei round, tabellone, risultati e registro della seriale."""
from __future__ import annotations

import time
from typing import Optional

from PySide6.QtCore import QTimer, Qt, Signal
from PySide6.QtGui import QColor, QFontMetrics, QKeySequence, QShortcut, QTextCharFormat, QTextCursor
from PySide6.QtWidgets import (QCheckBox, QComboBox, QFrame, QGridLayout, QGroupBox, QHBoxLayout, QLabel, QMessageBox,
                               QPlainTextEdit, QPushButton, QScrollArea, QSizePolicy, QSplitter, QTableView, QVBoxLayout, QWidget, QHeaderView)

from ..controller import RaceController
from ..links import SimulatedCentral
from ..models import COLOR_NAMES, N_LAPS, Slot
from . import theme
from .widgets import ElidedLabel, char_w
from .tables import KeepColorDelegate, ScheduleFilter, ScheduleModel


class PilotCard(QFrame):
    """Casella compatta di un pilota in volo: riga 1 colore + nome, riga 2 meccanico + risultato."""

    def __init__(self, color: int, parent=None):
        super().__init__(parent)
        self.color = color
        self.setObjectName("card")
        self.setStyleSheet(
            f"QFrame#card {{ background: {theme.COLOR_SOFT[color]}; border: 2px solid {theme.COLOR_ACCENT[color]}; border-radius: 8px; }}"
            "QLabel { background: transparent; }"
        )
        lay = QVBoxLayout(self)
        lay.setContentsMargins(10, 3, 10, 3)
        lay.setSpacing(0)
        r1 = QHBoxLayout()
        r1.setSpacing(8)
        self.tag = QLabel(COLOR_NAMES[color][0].upper())
        self.tag.setAlignment(Qt.AlignCenter)
        self.tag.setStyleSheet(f"color:white; background:{theme.COLOR_ACCENT[color]}; border-radius:4px; font-weight:700;")
        self.name = ElidedLabel("—")          # una riga: mai fuori dalla casella
        self.name.setStyleSheet("font-weight:700; color:#111827;")
        r1.addWidget(self.tag)
        r1.addWidget(self.name, 1)
        r2 = QHBoxLayout()
        r2.setSpacing(8)
        self.mech = ElidedLabel("")
        self.mech.setStyleSheet(f"color:{theme.MUTED};")
        self.result = QLabel("")
        self.result.setStyleSheet("font-weight:600; color:#111827;")
        self.result.setAlignment(Qt.AlignRight | Qt.AlignVCenter)
        r2.addWidget(self.mech, 1)
        r2.addWidget(self.result)
        lay.addLayout(r1)
        lay.addLayout(r2)
        self.relayout()

    def relayout(self) -> None:
        """Altezza e dimensione del nome calcolate dal font corrente (si adatta a schermo e zoom)."""
        f = self.font()
        f.setPointSizeF(f.pointSizeF() * 1.3)
        f.setBold(True)
        self.name.setFont(f)
        fm = self.fontMetrics()
        self.tag.setFixedWidth(fm.horizontalAdvance("0") * 2 + 6)
        small = self.font()
        small.setPointSizeF(small.pointSizeF() * 0.9)
        self.mech.setFont(small)
        self.result.setFont(small)
        self.setFixedHeight(self.name.fontMetrics().height() + fm.height() + 6 + 4 + 2)

    def show_slot(self, s: Optional[Slot]) -> None:
        if s is None:
            self.name.setText("—")
            self.mech.setText("")
            self.result.setText("")
            return
        self.name.setText(s.full_name or "—")
        self.mech.setText(f"Mecc.: {s.mech}" if s.mech else "")
        if s.score is not None:
            cuts = s.cuts or 0
            self.result.setStyleSheet(f"font-weight:700; color:{theme.OK if cuts == 0 else theme.WARN};")
            self.result.setText(f"{s.total or '/'} · {cuts} tagli · {s.score:g} pt")
        elif s.needs_check:
            self.result.setStyleSheet(f"font-weight:700; color:{theme.ERR};")
            self.result.setText("Totale mancante")
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

        # ---- colonna sinistra: in alto round, piloti e COMANDI (sempre visibili); sotto, le cose secondarie (scorre)
        top = QVBoxLayout()
        top.setContentsMargins(0, 0, 0, 0)
        top.setSpacing(6)
        hdr = QHBoxLayout()
        self.round_label = ElidedLabel("Nessun round selezionato")
        self.round_label.setStyleSheet("font-weight:700; color:#111827;")
        self.round_label.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        self.round_sub = ElidedLabel("")
        self.round_sub.setObjectName("muted")
        self.round_sub.setAlignment(Qt.AlignRight | Qt.AlignVCenter)
        hdr.addWidget(self.round_label, 1)
        top.addLayout(hdr)
        self.cards = [PilotCard(i) for i in range(3)]
        for c in self.cards:
            top.addWidget(c)

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
        top.addLayout(nav)

        cmd = QHBoxLayout()
        self.btn_show, self.btn_start, self.btn_stop = (QPushButton(t) for t in ("SHOW", "START", "STOP"))
        for b, n in ((self.btn_show, "cmdShow"), (self.btn_start, "cmdStart"), (self.btn_stop, "cmdStop")):
            b.setObjectName(n)
            cmd.addWidget(b)
        top.addLayout(cmd)

        self.refuel = QLabel("")
        self.refuel.setWordWrap(True)
        self.refuel.setStyleSheet(
            f"background:{theme.REFUEL_BG}; border:2px solid #E0B000; border-radius:8px; padding:3px 8px; font-weight:600;")
        top.addWidget(self.refuel)

        sec = QVBoxLayout()
        row = QHBoxLayout()
        self.btn_here = QPushButton("Parti da qui")
        self.btn_here.setToolTip("Parti dal round selezionato nella tabella")
        self.btn_reset = QPushButton("Reset")
        self.btn_reset.setToolTip("Azzera la selezione del round")
        row.addWidget(self.btn_here)
        row.addWidget(self.btn_reset)
        sec.addLayout(row)

        opts = QHBoxLayout()
        self.chk_tg = QCheckBox("Telegram")
        self.chk_voice = QCheckBox("Voce")
        opts.addWidget(self.chk_tg)
        opts.addWidget(self.chk_voice)
        opts.addStretch(1)
        sec.addLayout(opts)
        self.round_sub.setAlignment(Qt.AlignLeft | Qt.AlignVCenter)
        sec.addWidget(self.round_sub)

        # pannello del simulatore (visibile solo con la centrale simulata)
        self.sim_box = QGroupBox("Centrale simulata")
        sg = QGridLayout(self.sim_box)
        self.sim_buttons = {}
        for i, (key, text) in enumerate((
            ("next", "Centrale: AVANTI"), ("prev", "Centrale: INDIETRO"),
            ("ok", "Fine: regolari"), ("cut", "Fine: 1 taglio (verde)"),
            ("dnf", "Fine: timeout (blu)"), ("dbl", "Fine: doppio taglio (rosso)"))):
            b = QPushButton(text)
            self.sim_buttons[key] = b
            sg.addWidget(b, i, 0)
        sec.addWidget(self.sim_box)
        self.sim_box.hide()
        sec.addStretch(1)

        sw = QWidget()
        sw.setLayout(sec)
        lscroll = QScrollArea()
        self.left_scroll = lscroll
        lscroll.setWidget(sw)
        lscroll.setWidgetResizable(True)
        lscroll.setFrameShape(QFrame.NoFrame)
        lscroll.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        lscroll.setStyleSheet("QScrollArea{background:transparent;} QScrollArea > QWidget > QWidget{background:transparent;}")
        left = QVBoxLayout()
        left.setContentsMargins(0, 0, 0, 0)
        left.addLayout(top)
        left.addWidget(lscroll, 1)
        lw = QWidget()
        lw.setLayout(left)
        self.left_widget = lw

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
        for w in (self.cmb_cat, self.cmb_man, self.chk_follow):
            w.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.cmb_cat.setSizeAdjustPolicy(QComboBox.AdjustToContents)
        self.cmb_man.setSizeAdjustPolicy(QComboBox.AdjustToContents)
        right.addLayout(flt)
        self.btn_mv_pilot = QPushButton("Sposta pilota")
        self.btn_mv_pilot.setToolTip("Scambia due piloti della stessa categoria e manche")
        self.btn_mv_round = QPushButton("Sposta round")
        self.btn_mv_round.setToolTip("Sposta un'intera tripletta prima o dopo un altro round")
        self.btn_undo = QPushButton("Annulla modifica")
        self.btn_undo.setEnabled(False)
        for b in (self.btn_mv_pilot, self.btn_mv_round, self.btn_undo):
            b.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.btn_rf_new = QPushButton("Nuovo reflight")
        self.btn_rf_new.setToolTip("Fa rivolare uno o più piloti: crea i round di reflight nella posizione che scegli")
        self.btn_rf_ok = QPushButton("Conferma reflight")
        self.btn_rf_ok.setToolTip("Sostituisce i risultati originali con quelli dei reflight già volati")
        for b in (self.btn_rf_new, self.btn_rf_ok):
            b.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.btn_export = QPushButton("Esporta in Excel")
        self.btn_export.setToolTip("Esporta in un file Excel la griglia visualizzata (rispetta i filtri)")
        self.btn_export.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.count_label = ElidedLabel("")
        self.count_label.setStyleSheet(f"color:{theme.MUTED};")

        self.banner = QFrame()
        self.banner.setStyleSheet("QFrame{background:#FFE0B2; border:2px solid #FB8C00; border-radius:8px;} QLabel{border:none; background:transparent; font-weight:700; color:#4E2A00;}")
        bl = QHBoxLayout(self.banner)
        bl.setContentsMargins(10, 4, 10, 4)
        self.banner_label = QLabel("")
        self.banner_label.setWordWrap(True)
        self.btn_banner_cancel = QPushButton("Annulla selezione")
        bl.addWidget(self.banner_label, 1)
        bl.addWidget(self.btn_banner_cancel)
        self.banner.hide()
        right.addWidget(self.banner)
        self.table = QTableView()
        self.table.setModel(self.proxy)
        self.table.setSelectionBehavior(QTableView.SelectRows)
        self.table.setItemDelegate(KeepColorDelegate(self.table))
        self.table.verticalHeader().setVisible(False)
        self.table.verticalHeader().setDefaultSectionSize(self.table.fontMetrics().height() + 10)
        hh = self.table.horizontalHeader()
        hh.setSectionResizeMode(QHeaderView.Interactive)
        hh.setStretchLastSection(False)
        self.table.setEditTriggers(QTableView.DoubleClicked | QTableView.EditKeyPressed)
        self.table.doubleClicked.connect(self._double_clicked)

        self.log = LogView()
        split = QSplitter(Qt.Vertical)
        split.addWidget(self.table)
        split.addWidget(self.log)
        split.setStretchFactor(0, 5)
        split.setStretchFactor(1, 1)
        right.addWidget(split, 1)
        bottom = QHBoxLayout()
        bottom.addWidget(self.count_label, 1)
        for b in (self.btn_mv_pilot, self.btn_mv_round, self.btn_undo, self.btn_rf_new, self.btn_rf_ok):
            bottom.addWidget(b)
        bottom.addWidget(self.btn_export)
        right.addLayout(bottom)

        rw = QWidget()
        rw.setLayout(right)
        root.addWidget(lw)
        root.addWidget(rw, 1)

        # ---- segnali
        self.relayout()
        self.btn_mv_pilot.clicked.connect(lambda: self._start_move("pilot"))
        self.btn_mv_round.clicked.connect(lambda: self._start_move("round"))
        self.btn_undo.clicked.connect(self._undo)
        self.btn_rf_new.clicked.connect(self._reflight_new)
        self.btn_rf_ok.clicked.connect(self._reflight_confirm)
        self.ctl.reflight_finished.connect(lambda r: QTimer.singleShot(0, lambda: self._reflight_confirm(r)))
        self.btn_banner_cancel.clicked.connect(self._cancel_move)
        self.table.clicked.connect(self._table_clicked)
        self.ctl.edits_changed.connect(self._update_undo)
        QShortcut(QKeySequence("Esc"), self, activated=self._cancel_move)
        self.btn_export.clicked.connect(self._export_grid)
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

    def relayout(self) -> None:
        """Ricalcola larghezze e altezze dal font corrente: chiamata all'avvio e quando cambia lo zoom."""
        w = char_w(self, 1)
        self.left_widget.setFixedWidth(max(300, 42 * w))
        for c in self.cards:
            c.relayout()
        f = self.font()
        f.setPointSizeF(f.pointSizeF() * 1.3)
        f.setBold(True)
        self.round_label.setFont(f)
        fm = self.fontMetrics()
        self.refuel.setMinimumHeight(2 * fm.height() + 14)
        for b in (self.btn_show, self.btn_start, self.btn_stop):
            b.setMinimumHeight(fm.height() * 2 + 4)
        self.log.setMaximumHeight(fm.height() * 11)
        tf = self.font()                                   # tabella un po' più compatta del resto: i dati sono tanti
        tf.setPointSizeF(tf.pointSizeF() * 0.92)
        self.table.setFont(tf)
        self.table.horizontalHeader().setFont(tf)
        self.table.verticalHeader().setDefaultSectionSize(self.table.fontMetrics().height() + 10)
        self._chars = [5, 6, 5, 2.5, 19, 4, 17] + [8] * N_LAPS + [8, 5, 7]
        self._fit_columns()

    def _fit_columns(self) -> None:
        """Larghezze colonne dal font; se la tabella non ci sta nello spazio disponibile le stringe un po' (min 75%)."""
        chars = getattr(self, "_chars", None)
        if not chars:
            return
        from .tables import SCHED_HEAD

        hf = self.table.horizontalHeader().font()
        hf.setBold(True)
        hm = QFontMetrics(hf)
        want = [max(char_w(self.table, n) + 12, hm.horizontalAdvance(SCHED_HEAD[i]) + 20) for i, n in enumerate(chars)]
        laps = set(range(7, 7 + N_LAPS))
        names = {4, 6}
        fixed = sum(w for i, w in enumerate(want) if i not in laps | names)
        lw = sum(want[i] for i in laps)
        nw = sum(want[i] for i in names)
        avail = self.table.viewport().width()
        kl = kn = 1.0
        if avail > 100 and fixed + lw + nw > avail:
            kl = max(0.8, (avail - fixed - nw) / lw)               # prima si stringono i giri...
            if fixed + lw * kl + nw > avail:
                kn = max(0.6, (avail - fixed - lw * kl) / nw)      # ...poi, se serve, anche i nomi
        for i, wd in enumerate(want):
            self.table.setColumnWidth(i, int(wd * (kl if i in laps else kn if i in names else 1.0)))

    def resizeEvent(self, e) -> None:
        super().resizeEvent(e)
        QTimer.singleShot(0, self._fit_columns)

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

    def _export_grid(self) -> None:
        from pathlib import Path

        from PySide6.QtWidgets import QFileDialog

        from ..exporters import griglia_xlsx

        m = self.proxy
        head = [str(m.headerData(c, Qt.Horizontal) or "") for c in range(m.columnCount())]
        rows, fills = [], []
        for r in range(m.rowCount()):
            row, fl = [], []
            for c in range(m.columnCount()):
                ix = m.index(r, c)
                row.append(ix.data(Qt.EditRole) if c >= 7 else ix.data(Qt.DisplayRole))
                b = ix.data(Qt.BackgroundRole)
                fl.append(b.color().name() if b is not None else None)
            rows.append(row)
            fills.append(fl)
        if not rows:
            self.status_message.emit("Nessuna riga da esportare")
            return
        folder = Path(self.ctl._output_dir())
        folder.mkdir(parents=True, exist_ok=True)
        f, _ = QFileDialog.getSaveFileName(self, "Esporta griglia", str(folder / "Griglia_Gara.xlsx"), "Excel (*.xlsx)")
        if not f:
            return
        try:
            griglia_xlsx(Path(f), self.ctl.race.settings.title, head, rows, fills)
        except Exception as exc:
            QMessageBox.critical(self, "Esporta", f"Non riuscito: {exc}")
            return
        self.status_message.emit(f"Griglia esportata: {f}")

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

    # ------------------------------------------------------------------ sposta pilota / sposta round
    _mode: Optional[str] = None      # None | "pilot" | "round"
    _first: Optional[int] = None     # riga (pilota) o round scelto al primo passo

    def _start_move(self, mode: str) -> None:
        if self._mode == mode:
            self._cancel_move()
            return
        if not self.ctl.race.slots:
            self.status_message.emit("Nessun ordine di volo: generalo dalla scheda Sorteggio")
            return
        self._cancel_move()
        self._mode, self._first = mode, None
        self.table.setEditTriggers(QTableView.NoEditTriggers)
        self.btn_mv_pilot.setEnabled(mode == "pilot")
        self.btn_mv_round.setEnabled(mode == "round")
        self._banner_step()

    def _banner_step(self) -> None:
        if self._mode == "pilot":
            txt = ("SPOSTA PILOTA — 1/2: clicca nella tabella il pilota da spostare" if self._first is None else
                   "SPOSTA PILOTA — 2/2: clicca il pilota con cui scambiarlo (stessa categoria e manche)")
        else:
            txt = ("SPOSTA ROUND — 1/2: clicca una riga del round da spostare" if self._first is None else
                   "SPOSTA ROUND — 2/2: clicca una riga del round davanti o dietro al quale metterlo")
        self.banner_label.setText(txt + "   (Esc per annullare)")
        self.banner.show()

    def _cancel_move(self) -> None:
        if self._mode is None:
            return
        self._mode, self._first = None, None
        self.model.marked = set()
        self.model.refresh()
        self.banner.hide()
        self.table.setEditTriggers(QTableView.DoubleClicked | QTableView.EditKeyPressed)
        self.btn_mv_pilot.setEnabled(True)
        self.btn_mv_round.setEnabled(True)

    def _warn(self, text: str) -> None:
        QMessageBox.warning(self, "Sposta", text)

    def _table_clicked(self, idx) -> None:
        if self._mode is None or not idx.isValid():
            return
        row = self.proxy.mapToSource(idx).row()
        if self._mode == "pilot":
            self._click_pilot(row)
        else:
            self._click_round(row // 3)

    def _click_pilot(self, row: int) -> None:
        from .. import reorder

        ev = self.ctl.race
        if self._first is None:
            s = ev.slots[row]
            if reorder.slot_has_result(s):
                self._warn(f"{s.full_name or 'Il pilota'} ha già dei risultati: non si può spostare.")
                return
            self._first = row
            self.model.marked = {row}
            self.model.refresh()
            self._banner_step()
            return
        err = reorder.check_swap(ev, self._first, row)
        if err:
            self._warn(err)
            return
        a, b = ev.slots[self._first], ev.slots[row]
        msg = f"Scambiare\n\n  {reorder.describe_slot(a)}\n\ncon\n\n  {reorder.describe_slot(b)} ?"
        conflicts = self.ctl.swap_conflicts_preview(self._first, row)
        if conflicts:
            msg += "\n\nATTENZIONE, dopo lo scambio:\n  " + "\n  ".join(conflicts)
        in_flight = ev.current in {self._first // 3, row // 3}
        if in_flight:
            msg += "\n\nUno dei due è nel round in corso: i nomi verranno rinviati alla centrale."
        if QMessageBox.question(self, "Sposta pilota", msg, QMessageBox.Yes | QMessageBox.No, QMessageBox.No) != QMessageBox.Yes:
            return
        first = self._first
        self._cancel_move()
        err = self.ctl.swap_pilots(first, row)
        self.status_message.emit(err or "Piloti scambiati")

    def _click_round(self, r: int) -> None:
        from .. import reorder

        ev = self.ctl.race
        if self._first is None:
            if r == ev.current:
                self._warn("Il round in corso non si può spostare.")
                return
            if reorder.round_has_result(ev, r):
                self._warn("Il round ha già dei risultati: non si può spostare.")
                return
            self._first = r
            self.model.marked = {3 * r, 3 * r + 1, 3 * r + 2}
            self.model.refresh()
            self._banner_step()
            return
        if r == self._first:
            self._warn("Hai scelto lo stesso round: scegli quello davanti o dietro al quale metterlo.")
            return
        box = QMessageBox(self)
        box.setWindowTitle("Sposta round")
        box.setText(f"Mettere il round\n  {reorder.describe_round(ev, self._first)}\nrispetto al round\n  {reorder.describe_round(ev, r)} ?")
        b_before = box.addButton("Prima di questo round", QMessageBox.AcceptRole)
        b_after = box.addButton("Dopo questo round", QMessageBox.AcceptRole)
        box.addButton("Annulla", QMessageBox.RejectRole)
        box.exec()
        if box.clickedButton() not in (b_before, b_after):
            return
        final = reorder.final_index(self._first, r, after=box.clickedButton() is b_after)
        err = reorder.check_move_round(ev, self._first, final)
        if err:
            self._warn(err)
            return
        first = self._first
        self._cancel_move()
        err = self.ctl.move_round(first, final)
        self.status_message.emit(err or "Round spostato")

    # ------------------------------------------------------------------ reflight
    def _reflight_new(self) -> None:
        from .reflight_dialog import NewReflightDialog

        if self._mode:
            self._cancel_move()
        if not self.ctl.race.slots:
            self.status_message.emit("Nessun ordine di volo: generalo dalla scheda Sorteggio")
            return
        if not NewReflightDialog(self.ctl, self).exec():
            return
        self._fill_filters()
        self.status_message.emit("Reflight creato")

    def _reflight_confirm(self, r: Optional[int] = None) -> None:
        from .. import reflight
        from .reflight_dialog import ConfirmReflightDialog

        pending = reflight.pending_rounds(self.ctl.race)
        if not pending:
            self.status_message.emit("Nessun reflight da confermare")
            return
        if r is None or r not in pending:
            r = pending[0]
        if ConfirmReflightDialog(self.ctl, r, self).exec():
            self.status_message.emit("Reflight confermato: risultati sostituiti")

    def _undo(self) -> None:
        if self._mode:
            self._cancel_move()
        err = self.ctl.undo_edit()
        if err:
            self._warn(err)
        else:
            self.status_message.emit("Modifica annullata")

    def _update_undo(self) -> None:
        self.btn_undo.setEnabled(self.ctl.can_undo)
        self.btn_undo.setToolTip(("Annulla: " + self.ctl.undo_label) if self.ctl.can_undo else "Nessuna modifica da annullare")

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
        self._cancel_move()
        self.ctl.clear_edit_history()
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
            self.cmb_man.addItem(f"Manche {m}" if m < 100 else f"Reflight RF{m - 99} (manche {m})", m)
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
        mans = sorted({x.manche for x in ev.slots})
        if (self.cmb_cat.count() <= 1 and ev.slots) or mans != getattr(self, "_manches", mans):
            self._fill_filters()
        self._manches = mans
        cur = ev.current
        slots = ev.current_slots()
        if slots:
            s0 = slots[0]
            self.round_label.setText(f"{s0.category} · Reflight {s0.manche_label} · Round {s0.round}" if s0.is_reflight
                                     else f"{s0.category} · Manche {s0.manche} · Round {s0.round}")
            self.round_sub.setText(f"Avanzamento: {cur + 1} / {ev.n_rounds} round")
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
        from .. import reflight
        self.btn_rf_ok.setEnabled(bool(reflight.pending_rounds(ev)))

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
