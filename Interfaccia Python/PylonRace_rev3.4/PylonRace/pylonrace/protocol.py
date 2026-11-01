"""Protocollo seriale della centrale (Centrale_Rev9.ino), 9600 baud, righe di testo CSV.

PC -> centrale  (terminatore LF, max ~98 caratteri: il buffer della centrale è di 100 byte)
    codice,gara,manche,round,nome1,cognome1,nome2,cognome2,nome3,cognome3
    1400 reset (soft reset della centrale)   850/750/500 invio nomi   2000 show   3000 start   6000 stop

centrale -> PC
    800,0,... / 700,0,...   pulsanti AVANTI / INDIETRO premuti sulla centrale
    5514|5524|5534,t1..t10,totale   fine gara normale   (rosso|verde|blu)
    6514|6524|6534,...              timeout
    4015|4025|4035,...              doppio taglio
    i tempi con taglio hanno il suffisso -P1 / -P2 / -P3  (es. 5.301-P1)
"""
from __future__ import annotations

import unicodedata
from dataclasses import dataclass
from typing import Optional, Union

from .models import N_LAPS

CODE_RESET = 1400
CODE_NAMES_FWD = 850
CODE_NAMES_BACK = 750
CODE_NAMES_OTHER = 500
CODE_SHOW = 2000
CODE_START = 3000
CODE_STOP = 6000

MAX_LINE = 98  # il buffer my_str[100] della centrale non ha controlli di overflow

KIND_BY_PREFIX = {"55": "normal", "65": "timeout", "40": "double_cut"}


@dataclass
class ButtonEvent:
    command: str  # "next" | "prev"


@dataclass
class ResultLine:
    color: int  # 0 rosso, 1 verde, 2 blu
    kind: str  # normal | timeout | double_cut
    code: str
    laps: list
    total: str
    raw: str


def parse_line(line: str) -> Optional[Union[ButtonEvent, ResultLine]]:
    """Interpreta una riga ricevuta dalla centrale. None se non riconosciuta."""
    raw = (line or "").strip().strip("\x00")
    if not raw:
        return None
    parts = [p.strip() for p in raw.split(",")]
    code = parts[0]
    if code == "800":
        return ButtonEvent("next")
    if code == "700":
        return ButtonEvent("prev")
    if len(code) == 4 and code.isdigit() and code[3] in "45" and code[2] in "123":
        kind = KIND_BY_PREFIX.get(code[:2])
        if kind is None:
            return None
        laps = parts[1:1 + N_LAPS]
        laps += [""] * (N_LAPS - len(laps))
        total = parts[1 + N_LAPS] if len(parts) > 1 + N_LAPS else ""
        return ResultLine(int(code[2]) - 1, kind, code, laps, total, raw)
    return None


# ---------------------------------------------------------------------------
# Invio
# ---------------------------------------------------------------------------
def clean_field(text: str, ascii_fold: bool = True) -> str:
    s = (text or "").replace(",", " ").replace("\r", " ").replace("\n", " ").replace("\x00", "")
    if ascii_fold:
        s = unicodedata.normalize("NFKD", s)
        s = "".join(c for c in s if not unicodedata.combining(c))
        s = s.encode("ascii", "replace").decode("ascii")
    return " ".join(s.split())


def encode_command(
    code: int,
    gara: str = "",
    manche: Union[int, str] = "",
    rnd: Union[int, str] = "",
    pilots: Optional[list] = None,
    ascii_fold: bool = True,
) -> str:
    """Costruisce la riga (senza terminatore). pilots = [(nome, cognome)] x3."""
    pilots = list(pilots or [])
    while len(pilots) < 3:
        pilots.append(("", ""))
    fields = [clean_field(gara, ascii_fold), str(manche), str(rnd)]
    for n, c in pilots[:3]:
        fields += [clean_field(n, ascii_fold), clean_field(c, ascii_fold)]
    # accorcia i nomi più lunghi finché la riga entra nel buffer della centrale
    head = str(code)
    while len(head + "," + ",".join(fields)) > MAX_LINE:
        idx = max(range(4, len(fields)), key=lambda i: len(fields[i]))
        if len(fields[idx]) <= 1:
            break
        fields[idx] = fields[idx][:-1]
    return head + "," + ",".join(fields)


def reset_line() -> str:
    return "1400,,,,,,,,,"
