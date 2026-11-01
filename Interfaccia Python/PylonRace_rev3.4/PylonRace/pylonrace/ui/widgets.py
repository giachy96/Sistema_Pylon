"""Widget di supporto che si adattano allo spazio disponibile."""
from __future__ import annotations

from PySide6.QtCore import Qt
from PySide6.QtWidgets import QLabel, QSizePolicy


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
