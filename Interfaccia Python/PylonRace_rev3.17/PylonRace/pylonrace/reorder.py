"""Modifiche manuali all'ordine di volo: scambio di due piloti e spostamento di un intero round.

Regole (come le usava l'utente a mano su Excel, ma con i controlli):
- non si tocca nulla che abbia già dei risultati;
- lo scambio di piloti è ammesso solo nella stessa categoria e nella stessa manche (anche tra round diversi);
- un round si può spostare prima/dopo qualunque altro, anche di un'altra categoria;
- le etichette (categoria, manche, numero di round, colore) restano quelle: si muovono i piloti o l'intera tripletta;
- non si può inserire un round prima del round in corso o di round già volati.
"""
from __future__ import annotations

from typing import List, Optional

from .models import Event, Slot, norm

PILOT_FIELDS = ("pilot_id", "name", "surname", "mech", "dummy")


def slot_has_result(s: Slot) -> bool:
    return bool(s.score is not None or s.total or any(s.laps) or s.cuts is not None or s.kind or s.needs_check)


def round_has_result(ev: Event, r: int) -> bool:
    return any(slot_has_result(s) for s in ev.round_slots(r))


def describe_slot(s: Slot) -> str:
    return f"{s.full_name or '—'} ({s.category} manche {s.manche} round {s.round} {'RVB'[s.color]})"


def describe_round(ev: Event, r: int) -> str:
    s = ev.round_slots(r)[0]
    return f"{s.category} manche {s.manche} round {s.round}"


def _last_locked_round(ev: Event) -> int:
    """Indice dell'ultimo round che non si può superare: il corrente o l'ultimo con risultati (-1 = nessuno)."""
    last = ev.current
    for r in range(ev.n_rounds):
        if round_has_result(ev, r):
            last = max(last, r)
    return last


# ---------------------------------------------------------------- scambio piloti
def check_swap(ev: Event, i: int, j: int) -> Optional[str]:
    """None se lo scambio è ammesso, altrimenti il motivo."""
    if not (0 <= i < len(ev.slots) and 0 <= j < len(ev.slots)):
        return "Posizione non valida"
    a, b = ev.slots[i], ev.slots[j]
    if i == j:
        return "Hai scelto due volte lo stesso pilota"
    if a.category != b.category:
        return f"I due piloti sono di categorie diverse ({a.category} / {b.category})"
    if a.manche != b.manche:
        return f"I due piloti sono in manche diverse ({a.manche} / {b.manche})"
    for s in (a, b):
        if slot_has_result(s):
            return f"{s.full_name or 'Il pilota'} ha già dei risultati: non si può spostare"
    return None


def swap_pilots(ev: Event, i: int, j: int) -> None:
    a, b = ev.slots[i], ev.slots[j]
    for f in PILOT_FIELDS:
        va, vb = getattr(a, f), getattr(b, f)
        setattr(a, f, vb)
        setattr(b, f, va)


def conflicts_in_round(ev: Event, r: int) -> List[str]:
    """Piloti del round che volano insieme al proprio meccanico (messaggi leggibili)."""
    sl = [s for s in ev.round_slots(r) if not s.dummy]
    out = []
    for a in sl:
        for b in sl:
            if a is not b and norm(a.mech) and norm(a.mech) == norm(b.full_name):
                out.append(f"{b.full_name} vola insieme a {a.full_name}, di cui è il meccanico")
    return out


# ---------------------------------------------------------------- spostamento round
def _rounds(ev: Event) -> List[List[Slot]]:
    return [ev.slots[3 * i: 3 * i + 3] for i in range(ev.n_rounds)]


def _reordered(rounds: List[List[Slot]], src: int, final: int) -> List[List[Slot]]:
    out = list(rounds)
    item = out.pop(src)
    out.insert(final, item)
    return out


def final_index(src: int, dst: int, after: bool) -> int:
    """Posizione finale del round src se viene messo prima/dopo il round dst (indici prima dello spostamento)."""
    d = dst - (1 if src < dst else 0)
    return d + (1 if after else 0)


def check_move_round(ev: Event, src: int, final: int) -> Optional[str]:
    n = ev.n_rounds
    if not (0 <= src < n and 0 <= final < n):
        return "Posizione non valida"
    if src == final:
        return "Il round resta dov'è"
    if src == ev.current:
        return "Il round in corso non si può spostare"
    if round_has_result(ev, src):
        return "Il round ha già dei risultati: non si può spostare"
    rounds = _rounds(ev)
    new = _reordered(rounds, src, final)
    moved = rounds[src][0]
    pos = next(i for i, r in enumerate(new) if r[0] is moved)
    locks = [r for r in range(n) if round_has_result(ev, r)]
    if ev.current >= 0:
        locks.append(ev.current)
    for r in locks:
        first = rounds[r][0]
        if pos < next(i for i, x in enumerate(new) if x[0] is first):
            return "Non si può inserire un round prima del round in corso o di round già volati"
    return None


def move_round(ev: Event, src: int, final: int) -> int:
    """Sposta il round src nella posizione finale; il round corrente resta lo stesso round. Restituisce 'final'."""
    rounds = _rounds(ev)
    cur_first = rounds[ev.current][0] if 0 <= ev.current < len(rounds) else None
    new = _reordered(rounds, src, final)
    ev.slots[:] = [s for r in new for s in r]
    if cur_first is not None:
        ev.current = next(i for i, r in enumerate(new) if r[0] is cur_first)
    return final


def round_index_of(ev: Event, first_slot: Slot) -> int:
    for i in range(ev.n_rounds):
        if ev.slots[3 * i] is first_slot:
            return i
    return -1
