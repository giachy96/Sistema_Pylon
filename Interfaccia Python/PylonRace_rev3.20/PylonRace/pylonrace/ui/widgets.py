"""Widget di supporto che si adattano allo spazio disponibile."""
from __future__ import annotations

import re
from typing import Callable, Optional

from PySide6.QtCore import QTimer, Qt
from PySide6.QtWidgets import QHeaderView, QLabel, QSizePolicy


class ElidedLabel(QLabel):
    """Etichetta a una riga: se il testo non entra lo accorcia con «…» (testo intero nel suggerimento)."""

    def __init__(self, text: str = "", parent=None):
        super().__init__(parent)
        self._full = ""
        self.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Preferred)
        self.setMinimumWidth(40)
        self.setText(text)

    def full_text(self) -> str:
        return self._full

    def setText(self, text: str) -> None:
        self._full = text
        self.setToolTip(text)
        self._elide()

    def resizeEvent(self, e) -> None:
        super().resizeEvent(e)
        self._elide()

    def changeEvent(self, e) -> None:
        super().changeEvent(e)
        self._elide()

    def _elide(self) -> None:
        QLabel.setText(self, self.fontMetrics().elidedText(self._full, Qt.ElideRight, max(self.width() - 2, 10)))


def char_w(widget, n: float) -> int:
    """Larghezza in pixel di n cifre nel font corrente (le misure seguono la dimensione del testo)."""
    return int(n * widget.fontMetrics().horizontalAdvance("0"))


class ColumnMemory:
    """Colonne regolabili a mano con il mouse; le larghezze scelte si ricordano (per nome colonna) e si salvano.

    Le colonne M1, M2… della classifica condividono la stessa larghezza. Senza una misura salvata
    la colonna prende la larghezza del contenuto.
    """

    def __init__(self, store: Optional[dict] = None, save: Optional[Callable[[], None]] = None, prefix: str = ""):
        self.store = store if store is not None else {}
        self._save = save
        self.prefix = prefix
        self._busy = False
        self._pending = False

    @staticmethod
    def _key(label: str) -> str:
        return "M" if re.fullmatch(r"[MR]\d+", label or "") else (label or "")

    def apply(self, table, labels: list) -> None:
        hh = table.horizontalHeader()
        hh.setSectionResizeMode(QHeaderView.Interactive)
        hh.setStretchLastSection(False)
        hh.setMinimumSectionSize(24)
        self._busy = True
        try:
            table.resizeColumnsToContents()
            for i, lab in enumerate(labels):
                w = self.store.get(self.prefix + self._key(lab))
                if isinstance(w, int) and w >= 24:
                    table.setColumnWidth(i, w)
        finally:
            self._busy = False
        if not getattr(hh, "_colmem_connected", False):
            hh._colmem_connected = True
            hh.sectionResized.connect(lambda i, old, new, t=table: self._resized(t, i, new))

    def _resized(self, table, i: int, new: int) -> None:
        if self._busy:
            return
        m = table.model()
        lab = str(m.headerData(i, Qt.Horizontal, Qt.DisplayRole) or "") if m else ""
        if not lab:
            return
        self.store[self.prefix + self._key(lab)] = int(new)
        if self._key(lab) == "M":      # tutte le colonne M insieme
            self._busy = True
            try:
                for j in range(table.columnCount() if hasattr(table, "columnCount") else m.columnCount()):
                    if self._key(str(m.headerData(j, Qt.Horizontal, Qt.DisplayRole) or "")) == "M":
                        table.setColumnWidth(j, int(new))
            finally:
                self._busy = False
        if not self._pending:               # salva dopo una breve pausa, non a ogni pixel trascinato
            self._pending = True
            QTimer.singleShot(600, self._flush)

    def _flush(self) -> None:
        self._pending = False
        if self._save:
            self._save()
