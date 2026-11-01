"""Scheda PILOTI: elenco, inserimento e modifica."""
from __future__ import annotations

import json
from pathlib import Path
from typing import List, Optional

from PySide6.QtCore import Signal
from PySide6.QtWidgets import (QComboBox, QDialog, QDialogButtonBox, QFormLayout, QHBoxLayout, QHeaderView, QLabel,
                               QLineEdit, QMessageBox, QPushButton, QTableView, QVBoxLayout, QWidget)

from ..controller import RaceController
from ..models import CATEGORIES, Pilot, norm
from .tables import PilotModel

_COUNTRIES: Optional[list] = None


def countries() -> list:
    """[(sigla3, nome)] dal foglio 'Nazioni' del vecchio file Excel."""
    global _COUNTRIES
    if _COUNTRIES is None:
        try:
            raw = json.loads((Path(__file__).parent.parent / "data" / "countries.json").read_text(encoding="utf-8"))
            _COUNTRIES = [(a, b) for a, b, _ in raw]
        except (OSError, ValueError):
            _COUNTRIES = []
    return _COUNTRIES


class PilotDialog(QDialog):
    def __init__(self, parent=None, pilot: Optional[Pilot] = None, categories: Optional[List[str]] = None):
        super().__init__(parent)
        self.setWindowTitle("Pilota" if pilot is None else "Modifica pilota")
        self.setMinimumWidth(420)
        form = QFormLayout()
        self.cat = QComboBox()
        self.cat.addItems(categories or CATEGORIES)
        self.name = QLineEdit()
        self.surname = QLineEdit()
        self.mname = QLineEdit()
        self.msurname = QLineEdit()
        self.nation = QComboBox()
        self.nation.addItem("—", "")
        for code, label in countries():
            self.nation.addItem(f"{code} — {label}", code)
        self.nation.setEditable(False)
        self.fai = QLineEdit()
        self.phone = QLineEdit()
        self.pid = QLineEdit()
        self.pid.setPlaceholderText("automatico se vuoto")
        form.addRow("Gara *", self.cat)
        form.addRow("Nome pilota *", self.name)
        form.addRow("Cognome pilota *", self.surname)
        form.addRow("Nome meccanico", self.mname)
        form.addRow("Cognome meccanico", self.msurname)
        form.addRow("Nazionalità", self.nation)
        form.addRow("N° tessera FAI", self.fai)
        form.addRow("Telefono", self.phone)
        form.addRow("ID pilota", self.pid)
        box = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        box.accepted.connect(self._accept)
        box.rejected.connect(self.reject)
        lay = QVBoxLayout(self)
        lay.addLayout(form)
        lay.addWidget(box)
        if pilot:
            self.cat.setCurrentText(pilot.category)
            self.name.setText(pilot.name)
            self.surname.setText(pilot.surname)
            self.mname.setText(pilot.mech_name)
            self.msurname.setText(pilot.mech_surname)
            self.nation.setCurrentIndex(max(0, self.nation.findData(pilot.nation)))
            self.fai.setText(pilot.fai)
            self.phone.setText(pilot.phone)
            self.pid.setText(pilot.id)

    def _accept(self) -> None:
        if not (self.name.text().strip() and self.surname.text().strip()):
            QMessageBox.warning(self, "Campi obbligatori", "Riempire i campi Gara, Nome Pilota e Cognome Pilota!")
            return
        self.accept()

    def values(self) -> dict:
        return dict(
            category=self.cat.currentText(), name=self.name.text().strip(), surname=self.surname.text().strip(),
            mech_name=self.mname.text().strip(), mech_surname=self.msurname.text().strip(),
            nation=self.nation.currentData() or "", fai=self.fai.text().strip(), phone=self.phone.text().strip(),
            id=self.pid.text().strip(),
        )


class PilotsTab(QWidget):
    schedule_obsolete = Signal()
    import_requested = Signal()

    def __init__(self, ctl: RaceController, parent=None):
        super().__init__(parent)
        self.ctl = ctl
        self.model = PilotModel(ctl, self)
        lay = QVBoxLayout(self)
        lay.setContentsMargins(10, 10, 10, 10)
        bar = QHBoxLayout()
        self.btn_add = QPushButton("＋  Aggiungi pilota")
        self.btn_add.setObjectName("primary")
        self.btn_edit = QPushButton("Modifica")
        self.btn_del = QPushButton("Elimina")
        self.btn_del.setObjectName("danger")
        self.btn_sort = QPushButton("Ordina per gara")
        self.btn_imp = QPushButton("Importa da Excel…")
        self.cmb = QComboBox()
        self.cmb.addItem("Tutte le gare", "")
        for c in CATEGORIES:
            self.cmb.addItem(c, c)
        self.search = QLineEdit()
        self.search.setPlaceholderText("Cerca…")
        self.search.setClearButtonEnabled(True)
        for w in (self.btn_add, self.btn_edit, self.btn_del, self.btn_sort, self.btn_imp):
            bar.addWidget(w)
        bar.addStretch(1)
        bar.addWidget(self.cmb)
        bar.addWidget(self.search)
        lay.addLayout(bar)
        self.table = QTableView()
        self.table.setModel(self.model)
        self.table.setSelectionBehavior(QTableView.SelectRows)
        self.table.setSelectionMode(QTableView.SingleSelection)
        self.table.setEditTriggers(QTableView.NoEditTriggers)
        self.table.verticalHeader().setVisible(False)
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeToContents)
        self.table.horizontalHeader().setStretchLastSection(True)
        lay.addWidget(self.table, 1)
        self.info = QLabel("")
        self.info.setObjectName("muted")
        lay.addWidget(self.info)

        self.btn_add.clicked.connect(self.add)
        self.btn_edit.clicked.connect(self.edit)
        self.btn_del.clicked.connect(self.delete)
        self.btn_sort.clicked.connect(self.sort)
        self.btn_imp.clicked.connect(self.import_requested)
        self.table.doubleClicked.connect(lambda _: self.edit())
        self.cmb.currentIndexChanged.connect(self._filter)
        self.search.textChanged.connect(self._filter)
        self.reload()

    def reload(self) -> None:
        self.model.refresh()
        self._filter()

    def _filter(self) -> None:
        cat, q = self.cmb.currentData(), norm(self.search.text())
        shown = 0
        for r, p in enumerate(self.ctl.race.pilots):
            hide = bool((cat and p.category != cat) or (q and q not in norm(
                f"{p.name} {p.surname} {p.mech_full} {p.nation} {p.fai} {p.id}")))
            self.table.setRowHidden(r, hide)
            shown += not hide
        counts = ", ".join(f"{c}: {len(self.ctl.race.pilots_of(c))}" for c in CATEGORIES
                           if self.ctl.race.pilots_of(c))
        self.info.setText(f"{len(self.ctl.race.pilots)} piloti  ({counts})   ·   mostrati {shown}")

    def _selected(self) -> Optional[Pilot]:
        idx = self.table.currentIndex()
        return self.model.pilot(idx.row()) if idx.isValid() else None

    def _after_change(self, warn_schedule: bool = False) -> None:
        self.model.refresh()
        self._filter()
        self.ctl.request_autosave()
        self.ctl.changed.emit()
        if warn_schedule and self.ctl.race.slots:
            self.schedule_obsolete.emit()

    def add(self) -> None:
        dlg = PilotDialog(self, categories=self.ctl.race.settings.category_order or CATEGORIES)
        if self.cmb.currentData():
            dlg.cat.setCurrentText(self.cmb.currentData())
        while dlg.exec() == QDialog.Accepted:
            v = dlg.values()
            dup = [p for p in self.ctl.race.pilots if p.category == v["category"]
                   and norm(p.full_name) == norm(f"{v['name']} {v['surname']}")]
            if dup:
                QMessageBox.warning(self, "Pilota già presente", "Esiste già un pilota con questo nome nella stessa gara.")
                continue
            pid = v.pop("id") or self.ctl.race.next_pilot_id(v["category"])
            if any(p.id == pid for p in self.ctl.race.pilots):
                QMessageBox.warning(self, "ID duplicato", f"L'ID {pid} è già usato da un altro pilota.")
                continue
            self.ctl.race.pilots.append(Pilot(id=pid, **v))
            self._after_change(warn_schedule=True)
            QMessageBox.information(self, "Pilota", "Pilota inserito correttamente")
            break

    def edit(self) -> None:
        p = self._selected()
        if not p:
            return
        dlg = PilotDialog(self, p, self.ctl.race.settings.category_order or CATEGORIES)
        if dlg.exec() != QDialog.Accepted:
            return
        v = dlg.values()
        new_id = v.pop("id") or p.id
        if new_id != p.id and any(q.id == new_id for q in self.ctl.race.pilots):
            QMessageBox.warning(self, "ID duplicato", f"L'ID {new_id} è già usato da un altro pilota.")
            return
        if new_id != p.id:  # aggiorna i riferimenti nell'ordine di volo
            for s in self.ctl.race.slots:
                if s.pilot_id == p.id and s.category == p.category:
                    s.pilot_id = new_id
        cat_changed = v["category"] != p.category
        for k, val in v.items():
            setattr(p, k, val)
        p.id = new_id
        self.ctl.race.update_pilot_snapshot(p)
        self._after_change(warn_schedule=cat_changed)

    def delete(self) -> None:
        p = self._selected()
        if not p:
            return
        r = QMessageBox.question(self, "Elimina pilota", f"Eliminare {p.full_name} ({p.category})?")
        if r == QMessageBox.Yes:
            self.ctl.race.pilots.remove(p)
            self._after_change(warn_schedule=True)

    def sort(self) -> None:
        self.ctl.race.sort_pilots_by_category()
        self._after_change()
