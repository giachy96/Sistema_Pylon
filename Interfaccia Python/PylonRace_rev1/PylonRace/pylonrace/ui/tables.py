"""Modelli per le tabelle (ordine di volo con risultati, piloti)."""
from __future__ import annotations

from typing import Optional

from PySide6.QtCore import QAbstractTableModel, QModelIndex, QSortFilterProxyModel, Qt
from PySide6.QtGui import QBrush, QColor, QFont

from ..controller import RaceController
from ..models import COLOR_NAMES, N_LAPS, Pilot
from ..scoring import to_float
from . import theme

SCHED_HEAD = ["Gara", "Manche", "Round", "", "Pilota", "ID", "Meccanico"] + [f"L{i}" for i in range(1, N_LAPS + 1)] + [
    "Totale", "Tagli", "Punti"]
C_LAP0 = 7
C_TOTAL = C_LAP0 + N_LAPS
C_CUTS = C_TOTAL + 1
C_SCORE = C_TOTAL + 2


class ScheduleModel(QAbstractTableModel):
    """Ordine di volo (foglio GARA) con tempi e punteggi modificabili."""

    def __init__(self, ctl: RaceController, parent=None):
        super().__init__(parent)
        self.ctl = ctl
        self._n = len(ctl.race.slots)

    def refresh(self) -> None:
        n = len(self.ctl.race.slots)
        if n != self._n:
            self.beginResetModel()
            self._n = n
            self.endResetModel()
        elif n:
            self.dataChanged.emit(self.index(0, 0), self.index(n - 1, self.columnCount() - 1))

    # ---- struttura
    def rowCount(self, parent=QModelIndex()):
        return 0 if parent.isValid() else self._n

    def columnCount(self, parent=QModelIndex()):
        return len(SCHED_HEAD)

    def headerData(self, section, orientation, role=Qt.DisplayRole):
        if role == Qt.DisplayRole and orientation == Qt.Horizontal:
            return SCHED_HEAD[section]
        return None

    def flags(self, index):
        f = super().flags(index)
        if index.column() >= C_LAP0:
            f |= Qt.ItemIsEditable
        return f

    # ---- dati
    def _state(self, row: int) -> str:
        """current | refuel | '' """
        cur = self.ctl.race.current
        if cur < 0:
            return ""
        rnd = row // 3
        if rnd == cur:
            return "current"
        if rnd == cur + 1:
            return "refuel"
        return ""

    def data(self, index, role=Qt.DisplayRole):
        if not index.isValid() or index.row() >= len(self.ctl.race.slots):
            return None
        s = self.ctl.race.slots[index.row()]
        c = index.column()
        state = self._state(index.row())
        if role in (Qt.DisplayRole, Qt.EditRole):
            if c == 0:
                return s.category
            if c == 1:
                return s.manche
            if c == 2:
                return s.round
            if c == 3:
                return COLOR_NAMES[s.color][0]
            if c == 4:
                return s.full_name
            if c == 5:
                return s.pilot_id
            if c == 6:
                return s.mech
            if C_LAP0 <= c < C_TOTAL:
                v = s.laps[c - C_LAP0]
                return v if (v or role == Qt.EditRole) else ("/" if s.score is not None else "")
            if c == C_TOTAL:
                return s.total if (s.total or role == Qt.EditRole) else ("/" if s.score is not None else "")
            if c == C_CUTS:
                return "" if s.cuts is None else s.cuts
            if c == C_SCORE:
                return "" if s.score is None else (f"{s.score:g}" if role == Qt.DisplayRole else str(s.score))
        elif role == Qt.BackgroundRole:
            if state == "current":
                return QBrush(QColor(theme.COLOR_STRONG[s.color]))
            if state == "refuel":
                return QBrush(QColor(theme.REFUEL_BG))
            if c == C_SCORE and s.cuts:
                return QBrush(QColor(theme.CUT1_BG if s.cuts == 1 else theme.CUT2_BG))
            if c == 3:
                return QBrush(QColor(theme.COLOR_STRONG[s.color]))
            if (index.row() // 3) % 2:
                return QBrush(QColor("#F7F8FA"))
            if s.needs_check and c in (C_TOTAL, C_SCORE):
                return QBrush(QColor("#FFD6D6"))
        elif role == Qt.ForegroundRole:
            if s.dummy:
                return QBrush(QColor(theme.MUTED))
        elif role == Qt.FontRole:
            f = QFont()
            if state == "current" or c == C_SCORE:
                f.setBold(True)
            if s.dummy:
                f.setItalic(True)
            return f
        elif role == Qt.TextAlignmentRole:
            if c in (4, 6):
                return int(Qt.AlignLeft | Qt.AlignVCenter)
            return int(Qt.AlignCenter)
        elif role == Qt.ToolTipRole:
            if s.needs_check:
                return "Totale mancante: inserisci il totale o il punteggio a mano"
            if c >= C_LAP0 and c != C_CUTS:
                return "Doppio clic per modificare. Tagli: aggiungi il suffisso -P1/-P2/-P3 (es. 5.301-P1)"
        return None

    def setData(self, index, value, role=Qt.EditRole):
        if role != Qt.EditRole or not index.isValid():
            return False
        s = self.ctl.race.slots[index.row()]
        c = index.column()
        text = str(value).strip()
        if C_LAP0 <= c < C_TOTAL:
            self.ctl.edit_lap(s, c - C_LAP0, text)
        elif c == C_TOTAL:
            self.ctl.edit_total(s, text)
        elif c == C_SCORE:
            if text in ("", "/"):
                self.ctl.edit_score(s, None)
            else:
                v = to_float(text)
                if v is None:
                    return False
                self.ctl.edit_score(s, v)
        else:
            return False
        return True


class ScheduleFilter(QSortFilterProxyModel):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.category: Optional[str] = None
        self.manche: Optional[int] = None

    def set_filter(self, category: Optional[str], manche: Optional[int]) -> None:
        self.category, self.manche = category, manche
        self.invalidateFilter()

    def filterAcceptsRow(self, row, parent):
        m = self.sourceModel()
        s = m.ctl.race.slots[row]
        if self.category and s.category != self.category:
            return False
        if self.manche and s.manche != self.manche:
            return False
        return True


PILOT_HEAD = ["N°", "Gara", "Nome", "Cognome", "Nome mecc.", "Cognome mecc.", "Naz.", "N° FAI", "Telefono", "ID"]


class PilotModel(QAbstractTableModel):
    def __init__(self, ctl: RaceController, parent=None):
        super().__init__(parent)
        self.ctl = ctl

    def refresh(self) -> None:
        self.beginResetModel()
        self.endResetModel()

    def pilot(self, row: int) -> Pilot:
        return self.ctl.race.pilots[row]

    def rowCount(self, parent=QModelIndex()):
        return 0 if parent.isValid() else len(self.ctl.race.pilots)

    def columnCount(self, parent=QModelIndex()):
        return len(PILOT_HEAD)

    def headerData(self, section, orientation, role=Qt.DisplayRole):
        if role == Qt.DisplayRole and orientation == Qt.Horizontal:
            return PILOT_HEAD[section]
        return None

    def data(self, index, role=Qt.DisplayRole):
        if not index.isValid():
            return None
        pilots = self.ctl.race.pilots
        p = pilots[index.row()]
        c = index.column()
        if role == Qt.DisplayRole:
            if c == 0:
                same = [q for q in pilots[: index.row() + 1] if q.category == p.category]
                return len(same)
            return [None, p.category, p.name, p.surname, p.mech_name, p.mech_surname, p.nation, p.fai, p.phone, p.id][c]
        if role == Qt.TextAlignmentRole and c in (0, 1, 6, 7, 9):
            return int(Qt.AlignCenter)
        return None
