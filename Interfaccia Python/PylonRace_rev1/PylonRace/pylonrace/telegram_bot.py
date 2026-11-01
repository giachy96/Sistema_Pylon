"""Messaggi Telegram (porta di InvioMessaggi / InvioTempi / AvvisoClassifica)."""
from __future__ import annotations

import threading
from queue import Queue
from typing import Callable, List, Optional

from .models import COLOR_EMOJI, Slot

API = "https://api.telegram.org/bot{token}/sendMessage"


def _nm(s: Optional[Slot]) -> str:
    return s.full_name if s else ""


def format_start_line(slots: List[Slot], refuel: List[Slot], backward: bool = False) -> str:
    s0 = slots[0]
    lines = []
    if backward:
        lines.append("WARNING! THE MESSAGE ABOVE IS WRONG")
    lines += [
        "\U0001F6EB Start Line : ",
        f"Race : {s0.category}",
        f"Manche : {s0.manche}",
        f"Round :{s0.round}",
    ]
    for i, s in enumerate(slots):
        lines.append(f"{COLOR_EMOJI[i]} {_nm(s)}")
    refs = " , ".join(_nm(r) for r in refuel) if refuel else ""
    lines.append(f"⛽ Refueling :  {refs}")
    return "\n".join(lines)


def format_results(slots: List[Slot]) -> str:
    s0 = slots[0]
    out = [
        "\U0001F3C1RESULTS",
        f"Race :{s0.category}",
        f"Manche :{s0.manche}",
        f"Round :{s0.round}",
        "",
    ]
    for i, s in enumerate(slots):
        out.append(f"{COLOR_EMOJI[i]} {_nm(s)}")
        for n, lap in enumerate(s.laps, start=1):
            out.append(f"LAP: {n} {lap or '/'}")
        out.append(f"Time TOT: {s.total or '/'}")
        out.append(f"N.Cuts: {'' if s.cuts is None else s.cuts}")
        out.append(f"Score: {'' if s.score is None else s.score}")
        out.append("")
    return "\n".join(out).rstrip() + "\n"


def format_ranking(link: str = "") -> str:
    txt = "THE RACE RANKING HAS BEEN UPDATED\nhere the new results❗"
    if link:
        txt += "\n" + link
    return txt


class TelegramNotifier:
    """Invio in background (una coda, nessun blocco dell'interfaccia)."""

    def __init__(self, token: str = "", chat_id: str = "", on_result: Optional[Callable[[bool, str], None]] = None,
                 poster=None):
        self.token = token
        self.chat_id = chat_id
        self.on_result = on_result
        self._post = poster or self._requests_post
        self._q: "Queue[str]" = Queue()
        self._t = threading.Thread(target=self._run, daemon=True)
        self._t.start()

    def configure(self, token: str, chat_id: str) -> None:
        self.token, self.chat_id = token.strip(), str(chat_id).strip()

    @property
    def ready(self) -> bool:
        return bool(self.token and self.chat_id)

    def send(self, text: str) -> bool:
        if not self.ready:
            if self.on_result:
                self.on_result(False, "Telegram non configurato (token / chat ID mancanti nelle Impostazioni)")
            return False
        self._q.put(text)
        return True

    @staticmethod
    def _requests_post(url: str, data: dict):
        import requests

        r = requests.post(url, data=data, timeout=10)
        return r.status_code, r.text

    def _run(self) -> None:
        while True:
            text = self._q.get()
            try:
                code, body = self._post(API.format(token=self.token), {"chat_id": self.chat_id, "text": text})
                ok = code == 200
                msg = "Telegram: messaggio inviato" if ok else f"Telegram: errore {code} {body[:120]}"
            except Exception as exc:  # rete assente, timeout, ...
                ok, msg = False, f"Telegram: invio fallito ({exc.__class__.__name__})"
            if self.on_result:
                self.on_result(ok, msg)
