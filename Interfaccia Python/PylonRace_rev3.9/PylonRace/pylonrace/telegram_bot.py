"""Messaggi Telegram (porta di InvioMessaggi / InvioTempi / AvvisoClassifica)."""
from __future__ import annotations

import threading
from pathlib import Path
from queue import Queue
from typing import Callable, List, Optional

from .models import COLOR_EMOJI, REFLIGHT_BASE, Slot

API = "https://api.telegram.org/bot{token}/sendMessage"
API_DOC = "https://api.telegram.org/bot{token}/sendDocument"


def _nm(s: Optional[Slot]) -> str:
    return s.full_name if s else ""


# Testi predefiniti dei messaggi. L'utente può cambiarli (Impostazioni → Telegram → Configura messaggi).
# Nei testi si possono usare i segnaposto indicati in PLACEHOLDERS, scritti tra parentesi graffe.
DEFAULT_TEMPLATES = {
    "start": ("\U0001F6EB Start Line : \n"
              "Race : {gara}\n"
              "Manche : {manche} {reflight}\n"
              "Round :{round}\n"
              "\U0001F534 {rosso}\n"
              "\U0001F7E2 {verde}\n"
              "\U0001F535 {blu}\n"
              "⛽ Refueling :  {rifornimento}"),
    "start_warning": "WARNING! THE MESSAGE ABOVE IS WRONG\n",
    "results_header": "\U0001F3C1RESULTS\nRace :{gara}\nManche :{manche} {reflight}\nRound :{round}\n",
    "results_pilot": ("{emoji} {nome}\n"
                      "{giri}\n"
                      "Time TOT: {totale}\n"
                      "N.Cuts: {tagli}\n"
                      "Score: {punteggio}\n"),
    "ranking": "THE RACE RANKING HAS BEEN UPDATED\nhere the new results❗\n{link}",
}

TEMPLATE_TITLES = {
    "start": "Avviso di partenza (Start Line)",
    "start_warning": "Avviso aggiunto quando si torna INDIETRO",
    "results_header": "Risultati — intestazione",
    "results_pilot": "Risultati — blocco di ogni pilota (ripetuto per i 3 piloti)",
    "ranking": "Classifica aggiornata",
}

PLACEHOLDERS = {
    "start": "{gara} {manche} {reflight} {round} {rosso} {verde} {blu} {rifornimento}",
    "start_warning": "(nessun segnaposto)",
    "results_header": "{gara} {manche} {reflight} {round}",
    "results_pilot": "{emoji} {nome} {giri} {totale} {tagli} {punteggio}  (anche {lap1} … {lap10})",
    "ranking": "{link}",
}


def _fill(template: str, values: dict) -> str:
    """Sostituisce solo i segnaposto noti; un segnaposto sbagliato resta com'è, senza errori."""
    import re

    return re.sub(r"\{(\w+)\}", lambda m: str(values[m.group(1)]) if m.group(1) in values else m.group(0), template)


def _rf(s: Slot) -> str:
    """'(Reflight 1)' per i round di reflight, vuoto negli altri."""
    return f"(Reflight {s.manche - REFLIGHT_BASE + 1})" if s.is_reflight else ""


def tpl(templates: Optional[dict], key: str) -> str:
    t = (templates or {}).get(key)
    return t if isinstance(t, str) and t.strip() != "" else DEFAULT_TEMPLATES[key]


def format_start_line(slots: List[Slot], refuel: List[Slot], backward: bool = False,
                      templates: Optional[dict] = None) -> str:
    s0 = slots[0]
    vals = {
        "gara": s0.category, "manche": s0.manche, "round": s0.round, "reflight": _rf(s0),
        "rosso": _nm(slots[0]), "verde": _nm(slots[1]) if len(slots) > 1 else "", "blu": _nm(slots[2]) if len(slots) > 2 else "",
        "rifornimento": " , ".join(_nm(r) for r in refuel) if refuel else "",
    }
    txt = _fill(tpl(templates, "start"), vals)
    if backward:
        txt = _fill(tpl(templates, "start_warning"), {}) + txt
    return txt


def format_results(slots: List[Slot], templates: Optional[dict] = None) -> str:
    s0 = slots[0]
    head = _fill(tpl(templates, "results_header"), {"gara": s0.category, "manche": s0.manche, "round": s0.round, "reflight": _rf(s0)})
    blocks = []
    for i, s in enumerate(slots):
        vals = {
            "emoji": COLOR_EMOJI[i], "nome": _nm(s),
            "giri": "\n".join(f"LAP: {n} {lap or '/'}" for n, lap in enumerate(s.laps, start=1)),
            "totale": s.total or "/", "tagli": "" if s.cuts is None else s.cuts,
            "punteggio": "" if s.score is None else s.score,
        }
        for n in range(1, 11):
            vals[f"lap{n}"] = (s.laps[n - 1] if n <= len(s.laps) else "") or "/"
        blocks.append(_fill(tpl(templates, "results_pilot"), vals).rstrip("\n"))
    return (head.rstrip("\n") + "\n\n" + "\n\n".join(blocks)).rstrip() + "\n"


def format_ranking(link: str = "", templates: Optional[dict] = None) -> str:
    return _fill(tpl(templates, "ranking"), {"link": link}).rstrip()


class TelegramNotifier:
    """Invio in background (una coda, nessun blocco dell'interfaccia)."""

    def __init__(self, token: str = "", chat_id: str = "", on_result: Optional[Callable[[bool, str], None]] = None,
                 poster=None, doc_poster=None):
        self.token = token
        self.chat_id = chat_id
        self.on_result = on_result
        self._post = poster or self._requests_post
        self._post_doc = doc_poster or self._requests_post_doc
        self._q: "Queue" = Queue()
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

    def send_document(self, path, caption: str = "") -> bool:
        """Invia un file (es. il PDF della classifica) direttamente nella chat."""
        if not self.ready:
            if self.on_result:
                self.on_result(False, "Telegram non configurato (token / chat ID mancanti nelle Impostazioni)")
            return False
        self._q.put(("doc", str(path), caption))
        return True

    @staticmethod
    def _requests_post_doc(url: str, data: dict, path: str):
        import requests

        with open(path, "rb") as fh:
            r = requests.post(url, data=data, files={"document": (Path(path).name, fh, "application/pdf")}, timeout=60)
        return r.status_code, r.text

    @staticmethod
    def _requests_post(url: str, data: dict):
        import requests

        r = requests.post(url, data=data, timeout=10)
        return r.status_code, r.text

    def _run(self) -> None:
        while True:
            item = self._q.get()
            try:
                if isinstance(item, tuple):
                    _kind, path, caption = item
                    data = {"chat_id": self.chat_id}
                    if caption:
                        data["caption"] = caption[:1024]
                    code, body = self._post_doc(API_DOC.format(token=self.token), data, path)
                    ok = code == 200
                    msg = "Telegram: PDF inviato" if ok else f"Telegram: errore {code} {body[:120]}"
                else:
                    code, body = self._post(API.format(token=self.token), {"chat_id": self.chat_id, "text": item})
                    ok = code == 200
                    msg = "Telegram: messaggio inviato" if ok else f"Telegram: errore {code} {body[:120]}"
            except Exception as exc:  # rete assente, timeout, ...
                ok, msg = False, f"Telegram: invio fallito ({exc.__class__.__name__})"
            if self.on_result:
                self.on_result(ok, msg)
