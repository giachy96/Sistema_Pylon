"""Modelli per le tabelle (ordine di volo con risultati, piloti)."""
from __future__ import annotations
from ..i18n import color_letter, tr, trc

from typing import Optional

from PySide6.QtCore import QAbstractTableModel, QModelIndex, QSortFilterProxyModel, Qt
from PySide6.QtGui import QBrush, QColor, QFont, QPen
from PySide6.QtWidgets import QStyle, QStyledItemDelegate

from ..controller import RaceController
from ..models import N_LAPS, Pilot
from ..scoring import to_float
from . import theme

SCHED_HEAD = [trc("Gara", "col"), tr("Manche"), tr("Round"), "", tr("Pilota"), tr("ID"), tr("Meccanico")] + [f"L{i}" for i in range(1, N_LAPS + 1)] + [
    tr("Totale"), tr("Tagli"), tr("Punti")]
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
        self.marked: set = set()   # righe scelte durante 'Sposta pilota/round'

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
                if role == Qt.EditRole:
                    return s.manche
                return f"{s.manche} ({s.manche_label})" if s.is_reflight else s.manche
            if c == 2:
                return s.round
            if c == 3:
                return color_letter(s.color)
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
            if index.row() in self.marked:
                return QBrush(QColor("#FFB74D"))
            if state == "current":
                return QBrush(QColor(theme.COLOR_FLIGHT[s.color]))
            if state == "refuel":
                return QBrush(QColor(theme.REFUEL_BG))
            if s.is_reflight and c <= 2:
                return QBrush(QColor("#E6DBF7"))
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
            if state == "current" or c == C_SCORE or index.row() in self.marked:
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
                return tr("Totale mancante: inserisci il totale o il punteggio a mano")
            if c >= C_LAP0 and c != C_CUTS:
                return tr("Doppio clic per modificare. Tagli: aggiungi il suffisso -P1/-P2/-P3 (es. 5.301-P1)")
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


PILOT_HEAD = ["N°", trc("Gara", "col"), tr("Nome"), tr("Cognome"), tr("Nome mecc."), tr("Cognome mecc."), tr("Naz."), tr("N° FAI"), tr("Telefono"), tr("ID")]


# Un colore per ogni coppia pilota/meccanico (si riparte da capo per ogni gara: il problema conta solo dentro la gara)
PAIR_COLORS = ["#FFEE58", "#81D4FA", "#A5D6A7", "#FFAB91", "#CE93D8", "#FFCC80",
               "#80CBC4", "#F48FB1", "#C5E1A5", "#B39DDB", "#BCAAA4", "#90CAF9"]


class PilotModel(QAbstractTableModel):
    def __init__(self, ctl: RaceController, parent=None):
        super().__init__(parent)
        self.ctl = ctl
        self._flags: Optional[dict] = None

    def refresh(self) -> None:
        self.beginResetModel()
        self._flags = None
        self.endResetModel()

    def flags_of(self, p: Pilot) -> dict:
        """Legami pilota/meccanico, calcolati sui nomi (spazi e maiuscole ignorati).

        'mech_of': piloti che hanno p come meccanico; 'mech_pilot': il pilota che fa da meccanico a p (se esiste);
        'color': colore di p quando fa il meccanico di qualcuno; 'mech_color': colore del suo meccanico-pilota."""
        if self._flags is None:
            from ..models import norm

            pilots = self.ctl.race.pilots
            by_name: dict = {}
            for q in pilots:
                by_name.setdefault(norm(q.full_name), []).append(q)
            self._flags = {id(q): {"mech_of": [], "mech_pilot": None, "color": None, "mech_color": None}
                           for q in pilots}
            for q in pilots:                     # q ha come meccanico un altro pilota iscritto?
                cands = [x for x in by_name.get(norm(q.mech_full), []) if x is not q] if norm(q.mech_full) else []
                if cands:
                    m = next((x for x in cands if x.category == q.category), cands[0])
                    self._flags[id(q)]["mech_pilot"] = m
                    self._flags[id(m)]["mech_of"].append(q)
            counters: dict = {}
            for q in pilots:                     # un colore per ogni pilota che fa da meccanico
                if self._flags[id(q)]["mech_of"]:
                    k = counters.get(q.category, 0)
                    counters[q.category] = k + 1
                    self._flags[id(q)]["color"] = PAIR_COLORS[k % len(PAIR_COLORS)]
            for q in pilots:
                m = self._flags[id(q)]["mech_pilot"]
                if m is not None:
                    self._flags[id(q)]["mech_color"] = self._flags[id(m)]["color"]
        return self._flags.get(id(p), {"mech_of": [], "mech_pilot": None, "color": None, "mech_color": None})

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
        if c in (2, 3, 4, 5) and role in (Qt.BackgroundRole, Qt.ToolTipRole):
            fl = self.flags_of(p)
            if c in (2, 3) and fl["mech_of"]:          # il pilota è anche meccanico di qualcuno
                if role == Qt.BackgroundRole:
                    return QBrush(QColor(fl["color"]))
                return tr("È anche meccanico di: ") + ", ".join(f"{x.full_name} ({x.category})" for x in fl["mech_of"])
            if c in (4, 5) and fl["mech_pilot"] is not None:   # il suo meccanico è anche pilota
                if role == Qt.BackgroundRole:
                    return QBrush(QColor(fl["mech_color"]))
                m = fl["mech_pilot"]
                return tr(f"Il meccanico è anche pilota: {m.full_name} ({m.category})")
        return None


class KeepColorDelegate(QStyledItemDelegate):
    """La selezione non copre i colori delle righe: disegna solo un contorno scuro."""

    def paint(self, painter, option, index):
        from PySide6.QtWidgets import QStyleOptionViewItem

        opt = QStyleOptionViewItem(option)
        selected = bool(opt.state & QStyle.State_Selected)
        if selected:
            opt.state &= ~QStyle.State_Selected
        super().paint(painter, opt, index)
        if selected:
            painter.save()
            painter.setPen(QPen(QColor("#1F3A8A"), 2))
            painter.drawRect(opt.rect.adjusted(1, 1, -1, -1))
            painter.restore()
