"""Annuncio vocale dei piloti (equivalente di Application.Speech.Speak)."""
from __future__ import annotations

import os
import threading
from queue import Queue
from typing import Callable, List, Optional, Tuple


def announce_text(pilots: List[str], refuel: List[str]) -> str:
    # stesso testo del VBA (inglese: la lingua dipende dalla voce scelta)
    return (
        "At the start line ; " + ";".join(pilots) + ";. At the refueling ;" + ";".join(refuel)
    )


class Voice:
    """Sintesi vocale su thread dedicato. Usa SAPI5 su Windows tramite pyttsx3.

    Se il motore non è disponibile (altri sistemi, driver mancanti) l'app continua a funzionare.
    """

    def __init__(self, on_status: Optional[Callable[[str], None]] = None):
        self.voice_id = ""
        self.rate = 160
        self.on_status = on_status
        self.available: Optional[bool] = None
        self._q: "Queue[Optional[Tuple[str, str, int]]]" = Queue()
        self._t = threading.Thread(target=self._run, daemon=True)
        self._t.start()

    def speak(self, text: str) -> None:
        self._q.put((text, self.voice_id, self.rate))

    def stop(self) -> None:
        self._q.put(None)

    _voices_cache: Optional[List[Tuple[str, str]]] = None

    def list_voices(self, timeout: float = 4.0) -> List[Tuple[str, str]]:
        """Voci installate. Interroga il motore in un thread con timeout per non bloccare l'interfaccia."""
        if Voice._voices_cache is not None:
            return Voice._voices_cache
        if os.environ.get("PYLON_NO_TTS"):
            return []
        out: List[Tuple[str, str]] = []

        def work():
            try:
                import pyttsx3

                eng = pyttsx3.init()
                out.extend((v.id, v.name) for v in eng.getProperty("voices"))
            except Exception:
                pass

        t = threading.Thread(target=work, daemon=True)
        t.start()
        t.join(timeout)
        if not t.is_alive():
            Voice._voices_cache = out
        return out

    def _status(self, msg: str) -> None:
        if self.on_status:
            self.on_status(msg)

    def _run(self) -> None:
        engine = None
        while True:
            item = self._q.get()
            if item is None:
                continue
            text, vid, rate = item
            if os.environ.get("PYLON_NO_TTS"):
                continue
            try:
                if engine is None:
                    import pyttsx3

                    engine = pyttsx3.init()
                    self.available = True
                if vid:
                    engine.setProperty("voice", vid)
                engine.setProperty("rate", rate)
                engine.say(text)
                engine.runAndWait()
            except Exception as exc:
                if self.available is not False:
                    self._status(f"Voce non disponibile ({exc.__class__.__name__})")
                self.available = False
                engine = None
