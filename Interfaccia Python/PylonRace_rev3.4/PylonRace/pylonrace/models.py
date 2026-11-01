"""Modello dati dell'evento (equivalente dei fogli Piloti / GARA / Impostazioni)."""
from __future__ import annotations

import json
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Iterable, Optional

CATEGORIES = ["Q500", "Q40", "F3D", "F3E"]
COLOR_NAMES = ("Rosso", "Verde", "Blu")
COLOR_EMOJI = ("\U0001F534", "\U0001F7E2", "\U0001F535")  # rosso, verde, blu
N_LAPS = 10
DUMMY_A = "-------"
DUMMY_B = "!!!!!!"

# Preset dell'ordine delle categorie (UserForm2 del file Excel)
CATEGORY_ORDERS = [
    ["Q500", "Q40", "F3D", "F3E"],
    ["Q500", "F3E", "Q40", "F3D"],
    ["Q40", "Q500", "F3E", "F3D"],
    ["F3D", "Q40", "F3E", "Q500"],
]


def norm(s: str) -> str:
    """Normalizza un nome per i confronti (spazi e maiuscole)."""
    return " ".join((s or "").split()).casefold()


@dataclass
class Pilot:
    id: str
    category: str
    name: str
    surname: str
    mech_name: str = ""
    mech_surname: str = ""
    nation: str = ""
    fai: str = ""
    phone: str = ""

    @property
    def full_name(self) -> str:
        return f"{self.name} {self.surname}".strip()

    @property
    def mech_full(self) -> str:
        return f"{self.mech_name} {self.mech_surname}".strip()

    @property
    def has_fai(self) -> bool:
        return bool(str(self.fai).strip())


@dataclass
class Slot:
    """Una riga del foglio GARA: un pilota in un round, con il suo risultato."""

    category: str
    manche: int
    round: int
    color: int  # 0 rosso, 1 verde, 2 blu
    pilot_id: str
    name: str
    surname: str
    mech: str = ""
    dummy: bool = False
    laps: list = field(default_factory=lambda: [""] * N_LAPS)
    total: str = ""
    cuts: Optional[int] = None
    score: Optional[float] = None
    kind: str = ""  # "", normal, timeout, double_cut, manual
    needs_check: bool = False

    @property
    def full_name(self) -> str:
        return f"{self.name} {self.surname}".strip()

    @property
    def has_result(self) -> bool:
        return self.score is not None

    def clear_result(self) -> None:
        self.laps = [""] * N_LAPS
        self.total = ""
        self.cuts = None
        self.score = None
        self.kind = ""
        self.needs_check = False


@dataclass
class EventSettings:
    title: str = "Gara di Pylon Racing"
    category_order: list = field(default_factory=lambda: list(CATEGORIES[:]))
    fai_split: bool = False
    guard_seconds: float = 5.0
    reset_wait_seconds: float = 3.0
    ascii_fold: bool = True
    back_uses_750: bool = False
    auto_classifica: bool = False
    classifica_minutes: int = 5
    classifica_link: str = ""
    second_read_seconds: float = 30.0
    telegram_on: bool = False
    voice_on: bool = False


@dataclass
class Event:
    settings: EventSettings = field(default_factory=EventSettings)
    pilots: list = field(default_factory=list)
    slots: list = field(default_factory=list)
    current: int = -1  # indice del round corrente (-1 = nessuno)

    # ---- accesso ai round -------------------------------------------------
    @property
    def n_rounds(self) -> int:
        return len(self.slots) // 3

    def round_slots(self, i: int) -> list:
        return self.slots[3 * i: 3 * i + 3]

    def current_slots(self) -> list:
        if 0 <= self.current < self.n_rounds:
            return self.round_slots(self.current)
        return []

    def refuel_slots(self, i: int) -> list:
        """I tre piloti del round successivo (quelli indicati al rifornimento)."""
        return self.round_slots(i + 1) if i + 1 < self.n_rounds else []

    # ---- piloti ----------------------------------------------------------
    def pilots_of(self, category: str) -> list:
        return [p for p in self.pilots if p.category == category]

    def pilot_by_id(self, pid: str) -> Optional[Pilot]:
        for p in self.pilots:
            if p.id == pid:
                return p
        return None

    def next_pilot_id(self, category: str) -> str:
        prefix = {"Q500": "R", "Q40": "T", "F3D": "D", "F3E": "E"}.get(category, category[:1].upper())
        used = {p.id for p in self.pilots}
        n = len(self.pilots_of(category)) + 1
        while f"{prefix}{n}" in used:
            n += 1
        return f"{prefix}{n}"

    def sort_pilots_by_category(self) -> None:
        order = {c: i for i, c in enumerate(self.settings.category_order)}
        self.pilots.sort(key=lambda p: order.get(p.category, 99))  # sort stabile

    def update_pilot_snapshot(self, pilot: Pilot) -> None:
        for s in self.slots:
            if s.pilot_id == pilot.id and s.category == pilot.category and not s.dummy:
                s.name, s.surname, s.mech = pilot.name, pilot.surname, pilot.mech_full

    # ---- (de)serializzazione ---------------------------------------------
    def to_dict(self) -> dict:
        return {
            "version": 1,
            "settings": asdict(self.settings),
            "pilots": [asdict(p) for p in self.pilots],
            "slots": [asdict(s) for s in self.slots],
            "current": self.current,
        }

    @staticmethod
    def from_dict(d: dict) -> "Event":
        st = EventSettings()
        for k, v in (d.get("settings") or {}).items():
            if hasattr(st, k):
                setattr(st, k, v)
        pilots = [Pilot(**p) for p in d.get("pilots", [])]
        slots = []
        for s in d.get("slots", []):
            s = dict(s)
            s["laps"] = list(s.get("laps") or [""] * N_LAPS)
            slots.append(Slot(**s))
        return Event(settings=st, pilots=pilots, slots=slots, current=int(d.get("current", -1)))

    def save(self, path: Path) -> None:
        path = Path(path)
        path.parent.mkdir(parents=True, exist_ok=True)
        tmp = path.with_suffix(path.suffix + ".tmp")
        tmp.write_text(json.dumps(self.to_dict(), ensure_ascii=False, indent=1), encoding="utf-8")
        if path.exists():
            bak = path.with_suffix(path.suffix + ".bak")
            try:
                bak.write_bytes(path.read_bytes())
            except OSError:
                pass
        tmp.replace(path)

    @staticmethod
    def load(path: Path) -> "Event":
        return Event.from_dict(json.loads(Path(path).read_text(encoding="utf-8")))


def iter_real(slots: Iterable[Slot]):
    return (s for s in slots if not s.dummy)
