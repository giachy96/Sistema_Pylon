"""Finestre del reflight: creazione dei round e conferma della sostituzione."""
from __future__ import annotations
from ..i18n import color_letter, tr, trc

from typing import List, Optional

from PySide6.QtCore import Qt
from PySide6.QtWidgets import (QAbstractItemView, QComboBox, QDialog, QDialogButtonBox, QHBoxLayout, QLabel,
                               QListWidget, QListWidgetItem, QMessageBox, QPushButton, QTableWidget,
                               QTableWidgetItem, QVBoxLayout)

from .. import reflight
from ..controller import RaceController
from ..models import COLOR_NAMES, Slot


def _res(s: Slot) -> List[str]:
    """Totale, tagli, punti di uno slot, come testo."""
    if s.score is None and not s.total:
        return ["—", "", ""]
    return [s.total or "/", "" if s.cuts is None else str(s.cuts), "" if s.score is None else f"{s.score:g}"]


def _laps(s: Slot) -> str:
    return " ".join(x for x in s.laps if x) or "—"


class NewReflightDialog(QDialog):
    """Sceglie i piloti da rivolare e la posizione del reflight nell'ordine di volo."""

    def __init__(self, ctl: RaceController, parent=None):
        super().__init__(parent)
        self.ctl = ctl
        self.setWindowTitle(tr("Nuovo reflight"))
        self.resize(900, 560)
        self.picks: List[int] = []          # indici degli slot scelti, in ordine di selezione
        ev = ctl.race
        self._cand = reflight.candidates(ev)

        lay = QVBoxLayout(self)
        lay.addWidget(QLabel(tr("Scegli i piloti che devono rivolare (anche di manche diverse). "
                             "Il colore di partenza resta quello originale se non c'è conflitto; "
                             "altrimenti R, V, B seguono l'ordine in cui li scegli.")))
        row = QHBoxLayout()
        left, right = QVBoxLayout(), QVBoxLayout()
        top = QHBoxLayout()
        top.addWidget(QLabel(tr("Voli già fatti:")))
        self.cmb_cat = QComboBox()
        self.cmb_cat.addItem(tr("Tutte le categorie"), None)
        for c in ev.settings.category_order:
            if any(ev.slots[i].category == c for i in self._cand):
                self.cmb_cat.addItem(c, c)
        top.addWidget(self.cmb_cat, 1)
        left.addLayout(top)
        self.table = QTableWidget(0, 5)
        self.table.setHorizontalHeaderLabels([trc("Gara", "col"), tr("Manche"), tr("Round"), tr("Pilota"), tr("Punti")])
        self.table.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.table.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.table.verticalHeader().setVisible(False)
        self.table.horizontalHeader().setStretchLastSection(True)
        left.addWidget(self.table, 1)
        self.btn_add = QPushButton(tr("Aggiungi  ›"))
        left.addWidget(self.btn_add)
        right.addWidget(QLabel(tr("Piloti che rivolano (in ordine di scelta):")))
        self.chosen = QListWidget()
        right.addWidget(self.chosen, 1)
        self.btn_remove = QPushButton(tr("‹  Togli"))
        right.addWidget(self.btn_remove)
        row.addLayout(left, 3)
        row.addLayout(right, 2)
        lay.addLayout(row, 1)

        pos = QHBoxLayout()
        pos.addWidget(QLabel(tr("Posizione:")))
        self.cmb_pos = QComboBox()
        lock = reflight._last_locked_round(ev)
        for k in range(lock + 1, ev.n_rounds + 1):
            if k < ev.n_rounds:
                s0 = ev.round_slots(k)[0]
                self.cmb_pos.addItem(tr(f"Prima del round {k + 1}  ({reflight.describe(s0)})"), k)
            else:
                self.cmb_pos.addItem(tr("In fondo all'ordine di volo"), k)
        pos.addWidget(self.cmb_pos, 1)
        lay.addLayout(pos)
        self.preview = QLabel("")
        self.preview.setWordWrap(True)
        lay.addWidget(self.preview)
        self.buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        self.buttons.button(QDialogButtonBox.Ok).setText(tr("Crea reflight"))
        self.buttons.button(QDialogButtonBox.Cancel).setText(tr("Annulla"))
        lay.addWidget(self.buttons)

        self.cmb_cat.currentIndexChanged.connect(self._fill)
        self.btn_add.clicked.connect(self._add)
        self.table.doubleClicked.connect(lambda _i: self._add())
        self.btn_remove.clicked.connect(self._remove)
        self.buttons.accepted.connect(self._accept)
        self.buttons.rejected.connect(self.reject)
        self._fill()
        self._update()

    def _fill(self) -> None:
        ev = self.ctl.race
        cat = self.cmb_cat.currentData()
        rows = [i for i in self._cand if (cat is None or ev.slots[i].category == cat) and i not in self.picks]
        self.table.setRowCount(len(rows))
        for r, i in enumerate(rows):
            s = ev.slots[i]
            vals = [s.category, str(s.manche), str(s.round), s.full_name, _res(s)[2]]
            for c, v in enumerate(vals):
                it = QTableWidgetItem(v)
                it.setData(Qt.UserRole, i)
                self.table.setItem(r, c, it)
        self.table.resizeColumnsToContents()

    def _add(self) -> None:
        for idx in self.table.selectionModel().selectedRows():
            i = self.table.item(idx.row(), 0).data(Qt.UserRole)
            if i not in self.picks:
                self.picks.append(i)
        self._refresh_lists()

    def _remove(self) -> None:
        for it in self.chosen.selectedItems():
            self.picks.remove(it.data(Qt.UserRole))
        self._refresh_lists()

    def _refresh_lists(self) -> None:
        ev = self.ctl.race
        self.chosen.clear()
        for i in self.picks:
            s = ev.slots[i]
            it = QListWidgetItem(tr(f"{s.category} · {s.full_name}  (manche {s.manche}, round {s.round}, {tr(COLOR_NAMES[s.color])})"))
            it.setData(Qt.UserRole, i)
            self.chosen.addItem(it)
        self._fill()
        self._update()

    def _update(self) -> None:
        ev = self.ctl.race
        if not self.picks:
            self.preview.setText(tr("Nessun pilota scelto."))
            return
        rounds = reflight.build_rounds(ev, self.picks, reflight.next_manche(ev))
        lines = []
        for sl in rounds:
            names = ", ".join(f"{color_letter(s.color)}: {s.full_name if not s.dummy else '—'}" for s in sl)
            lines.append(tr(f"{sl[0].category} · manche {sl[0].manche} · round {sl[0].round}  →  {names}"))
        self.preview.setText(tr("Verranno creati:\n") + "\n".join(lines))

    def _accept(self) -> None:
        err = self.ctl.create_reflight(self.picks, self.cmb_pos.currentData())
        if err:
            QMessageBox.warning(self, tr("Reflight"), err)
            return
        self.accept()


class ConfirmReflightDialog(QDialog):
    """Mostra il risultato del reflight accanto a quello da sostituire; si può cambiare il bersaglio."""

    HEAD = [tr("Pilota"), tr("Reflight: tempi"), tr("Totale"), tr("Tagli"), tr("Punti"),
            tr("Sostituisce"), tr("Totale"), tr("Tagli"), tr("Punti")]

    def __init__(self, ctl: RaceController, round_index: int, parent=None):
        super().__init__(parent)
        self.ctl = ctl
        self.setWindowTitle(tr("Conferma reflight"))
        self.resize(980, 340)
        self._rows = [(rs, ctl.reflight_proposals(round_index)[k][1])
                      for k, (rs, _t) in enumerate(ctl.reflight_proposals(round_index))]
        self._targets: List[Optional[Slot]] = [t for _rs, t in self._rows]
        lay = QVBoxLayout(self)
        s0 = ctl.race.round_slots(round_index)[0]
        lay.addWidget(QLabel(tr(f"{reflight.describe(s0)}: il risultato qui sotto sostituirà quello indicato a destra. "
                             "Puoi scegliere un altro volo dello stesso pilota da sostituire.")))
        self.table = QTableWidget(len(self._rows), len(self.HEAD))
        self.table.setHorizontalHeaderLabels(self.HEAD)
        self.table.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.table.setSelectionMode(QAbstractItemView.NoSelection)
        self.table.verticalHeader().setVisible(False)
        lay.addWidget(self.table, 1)
        self.note = QLabel("")
        self.note.setStyleSheet("color:#B26A00;")
        bb = QDialogButtonBox()
        self.btn_ok = bb.addButton(tr("Conferma sostituzione"), QDialogButtonBox.AcceptRole)
        bb.addButton(tr("Più tardi"), QDialogButtonBox.RejectRole)
        bb.accepted.connect(self._accept)
        bb.rejected.connect(self.reject)
        self.combos: List[QComboBox] = []
        for r, (rs, t) in enumerate(self._rows):
            for c, v in enumerate([rs.full_name, _laps(rs), *_res(rs)]):
                self._set(r, c, v)
            self.table.item(r, 1).setToolTip(_laps(rs))
            cmb = QComboBox()
            for cand in reflight.target_choices(ctl.race, rs):
                cmb.addItem(tr(f"manche {cand.manche} · round {cand.round}"), cand)
            if t is not None:
                for k in range(cmb.count()):
                    if cmb.itemData(k) is t:
                        cmb.setCurrentIndex(k)
            self.combos.append(cmb)
            self.table.setCellWidget(r, 5, cmb)
            cmb.currentIndexChanged.connect(lambda _i, row=r: self._target_changed(row))
            self._target_changed(r)
        self.table.resizeColumnsToContents()
        self.table.setColumnWidth(1, 220)
        self.table.setColumnWidth(5, 190)
        lay.addWidget(self.note)
        lay.addWidget(bb)

    def _set(self, r: int, c: int, v: str) -> None:
        it = QTableWidgetItem(v)
        it.setTextAlignment(Qt.AlignLeft | Qt.AlignVCenter if c in (0, 1) else Qt.AlignCenter)
        self.table.setItem(r, c, it)

    def _target_changed(self, row: int) -> None:
        t = self.combos[row].currentData()
        self._targets[row] = t
        for c, v in zip((6, 7, 8), _res(t) if t is not None else ["", "", ""]):
            self._set(row, c, v)
        notes = []
        for k, tt in enumerate(self._targets):
            if tt is None:
                notes.append(tr(f"{self._rows[k][0].full_name}: nessun volo da sostituire"))
            elif tt.score is None and not tt.total:
                notes.append(tr(f"{self._rows[k][0].full_name}: il volo scelto non ha risultato"))
        self.note.setText("  ·  ".join(notes))
        self.btn_ok.setEnabled(all(t is not None for t in self._targets))

    def pairs(self) -> list:
        return [(rs, t) for (rs, _), t in zip(self._rows, self._targets) if t is not None]

    def _accept(self) -> None:
        err = self.ctl.apply_reflight(self.pairs())
        if err:
            QMessageBox.warning(self, tr("Reflight"), err)
            return
        self.accept()
