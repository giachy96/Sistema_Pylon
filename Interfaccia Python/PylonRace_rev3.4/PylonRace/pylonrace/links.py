"""Collegamenti con la centrale: porta seriale reale e centrale simulata."""
from __future__ import annotations

import random
import threading
from typing import Callable, List, Optional

from PySide6.QtCore import QObject, QTimer, Signal


def list_ports() -> List[tuple]:
    """[(device, descrizione)] delle porte seriali presenti."""
    try:
        from serial.tools import list_ports as lp

        return [(p.device, p.description) for p in sorted(lp.comports(), key=lambda x: x.device)]
    except Exception:
        return []


class Link(QObject):
    """Interfaccia comune: riceve righe di testo dalla centrale, ne invia."""

    line_received = Signal(str)
    line_sent = Signal(str)
    state_changed = Signal(bool, str)  # connesso?, descrizione
    error = Signal(str)

    is_open = False

    def open(self) -> bool:  # pragma: no cover - interfaccia
        raise NotImplementedError

    def close(self) -> None:  # pragma: no cover
        raise NotImplementedError

    def send_line(self, line: str) -> bool:  # pragma: no cover
        raise NotImplementedError


class SerialLink(Link):
    """Porta seriale reale (CH340 della centrale, 9600 baud 8N1)."""

    def __init__(self, port: str = "", baud: int = 9600, dtr: bool = True, parent=None):
        super().__init__(parent)
        self.port, self.baud, self.dtr = port, baud, dtr
        self._ser = None
        self._thread: Optional[threading.Thread] = None
        self._stop = threading.Event()
        self._lock = threading.Lock()

    def open(self) -> bool:
        if self.is_open:
            return True
        try:
            import serial

            ser = serial.Serial()
            ser.port, ser.baudrate, ser.timeout = self.port, self.baud, 0.1
            ser.write_timeout = 2
            ser.dtr = self.dtr  # come Data Streamer: il pin checkPC della centrale può dipendere da questo
            ser.open()
        except Exception as exc:
            self.error.emit(f"Impossibile aprire {self.port}: {exc}")
            self.state_changed.emit(False, f"Errore su {self.port}")
            return False
        self._ser = ser
        self._stop.clear()
        self._thread = threading.Thread(target=self._read_loop, daemon=True)
        self._thread.start()
        self.is_open = True
        self.state_changed.emit(True, f"{self.port} @ {self.baud}")
        return True

    def close(self) -> None:
        self._stop.set()
        if self._thread:
            self._thread.join(timeout=1.0)
        with self._lock:
            if self._ser:
                try:
                    self._ser.close()
                except Exception:
                    pass
            self._ser = None
        was = self.is_open
        self.is_open = False
        if was:
            self.state_changed.emit(False, "Disconnesso")

    def send_line(self, line: str) -> bool:
        with self._lock:
            if not self._ser:
                self.error.emit("Porta seriale non aperta")
                return False
            try:
                self._ser.write((line + "\n").encode("ascii", "replace"))
                self._ser.flush()
            except Exception as exc:
                self.error.emit(f"Errore di scrittura: {exc}")
                return False
        self.line_sent.emit(line)
        return True

    def _read_loop(self) -> None:
        buf = bytearray()
        while not self._stop.is_set():
            try:
                chunk = self._ser.read(self._ser.in_waiting or 1)
            except Exception as exc:  # porta rimossa (USB scollegato)
                self.error.emit(f"Porta persa: {exc}")
                self.is_open = False
                self.state_changed.emit(False, "Porta persa")
                return
            if not chunk:
                continue
            buf += chunk
            while b"\n" in buf:
                raw, _, rest = bytes(buf).partition(b"\n")
                buf = bytearray(rest)
                text = raw.decode("utf-8", "replace").strip("\r\x00 ")
                if text:
                    self.line_received.emit(text)


class SimulatedCentral(Link):
    """Centrale simulata: risponde ai comandi del PC e può generare pulsanti e risultati.

    Utile per provare il programma a tavolino, senza hardware.
    """

    def __init__(self, parent=None, sleep: Optional[Callable[[int, Callable], None]] = None, seed=None):
        super().__init__(parent)
        self._schedule = sleep or (lambda ms, fn: QTimer.singleShot(ms, fn))
        self.rng = random.Random(seed)
        self.last_names_line = ""
        self.received: List[str] = []

    def open(self) -> bool:
        self.is_open = True
        self.state_changed.emit(True, "Centrale simulata")
        return True

    def close(self) -> None:
        if self.is_open:
            self.is_open = False
            self.state_changed.emit(False, "Disconnesso")

    def send_line(self, line: str) -> bool:
        if not self.is_open:
            return False
        self.received.append(line)
        if line.split(",")[0] in ("850", "750", "500"):
            self.last_names_line = line
        self.line_sent.emit(line)
        return True

    # ---- azioni dell'operatore sulla centrale simulata -------------------------
    def press_next(self) -> None:
        self.line_received.emit("800,0,0,0,0,0,0,0,0,0")

    def press_prev(self) -> None:
        self.line_received.emit("700,0,0,0,0,0,0,0,0,0")

    def make_result(self, color: int, kind: str = "normal", cuts: int = 0, base: float = 5.6) -> str:
        code = {"normal": "55", "timeout": "65", "double_cut": "40"}[kind] + str(color + 1) + ("5" if kind == "double_cut" else "4")
        if kind == "timeout":
            return f"{code}," + ",".join([""] * 10) + ","
        laps = [round(self.rng.uniform(base - 0.6, base + 0.9), 3) for _ in range(10)]
        laps[0] = round(laps[0] + 3.0, 3)
        cells = [f"{x:.3f}" for x in laps]
        total = sum(laps)
        picks = self.rng.sample(range(1, 10), k=min(cuts, 9)) if kind != "double_cut" else [4]
        for n, i in enumerate(picks):
            cells[i] += "-P1-P2" if kind == "double_cut" else f"-P{self.rng.randint(1, 3)}"
        return f"{code}," + ",".join(cells) + f",{total:.3f}"

    def finish_round(self, specs: Optional[List[tuple]] = None, delay_ms: int = 200) -> List[str]:
        """Invia le tre righe di fine round (rosso, verde, blu). specs = [(kind, cuts)] x3."""
        specs = specs or [("normal", 0)] * 3
        lines = [self.make_result(c, k, n) for c, (k, n) in enumerate(specs)]
        for i, ln in enumerate(lines):
            self._schedule(delay_ms * i, lambda ln=ln: self.line_received.emit(ln))
        return lines
