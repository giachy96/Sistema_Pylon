"""Reflight: uno o più piloti rivolano un round (errore di guida, falsa partenza) e il nuovo risultato
sostituisce quello compromesso.

Regole:
- il nome della gara non cambia (resta la categoria); il reflight è identificato dal numero di manche:
  100 per il primo reflight, 101 per il secondo, ecc.; il numero di round riparte da 1 per ogni categoria;
- si possono rivolare piloti di manche diverse della stessa categoria (e di categorie diverse: round separati);
- la posizione nell'ordine di volo la sceglie sempre l'utente, mai prima del round in corso o già volato;
- finito il volo, il risultato va a sostituire quello dello slot originale dopo conferma (il valore vecchio è
  conservato per poterlo ripristinare); la classifica ignora i round di reflight e legge solo gli slot originali.
"""
from __future__ import annotations

from typing import Dict, List, Optional, Tuple

from .draw import filler_names
from .models import DUMMY_A, REFLIGHT_BASE, Event, Slot
from .reorder import _last_locked_round, slot_has_result

RESULT_FIELDS = ("laps", "total", "cuts", "score", "kind", "needs_check")


# ---------------------------------------------------------------- risultati
def snapshot(s: Slot) -> dict:
    return {"laps": list(s.laps), "total": s.total, "cuts": s.cuts, "score": s.score,
            "kind": s.kind, "needs_check": s.needs_check}


def restore(s: Slot, snap: dict) -> None:
    s.laps = list(snap["laps"])
    s.total, s.cuts, s.score = snap["total"], snap["cuts"], snap["score"]
    s.kind, s.needs_check = snap["kind"], snap["needs_check"]


# ---------------------------------------------------------------- ricerca
def next_manche(ev: Event) -> int:
    used = [s.manche for s in ev.slots if s.is_reflight]
    return max(used) + 1 if used else REFLIGHT_BASE


def find_target(ev: Event, rs: Slot, manche: Optional[int] = None) -> Optional[Slot]:
    """Lo slot originale (stesso pilota e categoria) che il reflight sostituisce."""
    m = rs.target_manche if manche is None else manche
    for s in ev.slots:
        if (not s.is_reflight and not s.dummy and s.category == rs.category
                and s.pilot_id == rs.pilot_id and s.manche == m):
            return s
    return None


def target_choices(ev: Event, rs: Slot) -> List[Slot]:
    """Tutti i voli dello stesso pilota che si possono sostituire (per 'cambia bersaglio')."""
    return [s for s in ev.slots if (not s.is_reflight and not s.dummy and s.category == rs.category
                                    and s.pilot_id == rs.pilot_id)]


def candidates(ev: Event) -> List[int]:
    """Indici degli slot che si possono rivolare: voli reali, non reflight, con un risultato."""
    return [i for i, s in enumerate(ev.slots) if not s.is_reflight and not s.dummy and slot_has_result(s)]


def reflight_rounds(ev: Event) -> List[int]:
    return [r for r in range(ev.n_rounds) if ev.round_slots(r)[0].is_reflight]


def pending_rounds(ev: Event) -> List[int]:
    """Round di reflight già volati con almeno un risultato non ancora sostituito."""
    out = []
    for r in reflight_rounds(ev):
        sl = [s for s in ev.round_slots(r) if not s.dummy]
        if sl and all(slot_has_result(s) for s in sl) and any(not s.applied for s in sl):
            out.append(r)
    return out


# ---------------------------------------------------------------- creazione
def check_create(ev: Event, picks: List[int], insert_at: int) -> Optional[str]:
    if not picks:
        return "Scegli almeno un pilota da rivolare"
    if len(set(picks)) != len(picks):
        return "Un pilota è stato scelto due volte"
    ok = set(candidates(ev))
    for i in picks:
        if i not in ok:
            return "Si possono rivolare solo voli che hanno già un risultato"
    seen = set()
    for i in picks:
        s = ev.slots[i]
        key = (s.category, s.pilot_id, s.manche)
        if key in seen:
            return f"{s.full_name} è stato scelto due volte"
        seen.add(key)
        for r in ev.slots:
            if (r.is_reflight and not r.applied and r.category == s.category and r.pilot_id == s.pilot_id
                    and r.target_manche == s.manche):
                return f"{s.full_name} (manche {s.manche}) ha già un reflight in programma"
    if not (0 <= insert_at <= ev.n_rounds):
        return "Posizione non valida"
    if insert_at <= _last_locked_round(ev):
        return "Non si può inserire un reflight prima del round in corso o di round già volati"
    return None


def _colors(group: List[Slot]) -> List[int]:
    """Colore originale se non c'è conflitto, altrimenti R, V, B nell'ordine di selezione."""
    orig = [s.color for s in group]
    if len(set(orig)) == len(orig):
        return orig
    return list(range(len(group)))


def build_rounds(ev: Event, picks: List[int], manche: int) -> List[List[Slot]]:
    by_cat: Dict[str, List[Slot]] = {}
    for i in picks:
        by_cat.setdefault(ev.slots[i].category, []).append(ev.slots[i])
    order = {c: k for k, c in enumerate(ev.settings.category_order)}
    out: List[List[Slot]] = []
    for cat in sorted(by_cat, key=lambda c: order.get(c, 99)):
        src = by_cat[cat]
        for n, k in enumerate(range(0, len(src), 3), start=1):
            group = src[k:k + 3]
            colors = _colors(group)
            slots: List[Slot] = []
            for o, c in zip(group, colors):
                p = ev.pilot_by_id(o.pilot_id)
                mech = p.mech_full if p else o.mech
                slots.append(Slot(cat, manche, n, c, o.pilot_id, o.name, o.surname, mech, False,
                                  target_manche=o.manche))
            free = [c for c in range(3) if c not in colors]
            for c, nm in zip(free, filler_names(len(group)) or [DUMMY_A] * len(free)):
                slots.append(Slot(cat, manche, n, c, "", nm, nm, "", True))
            slots.sort(key=lambda s: s.color)
            out.append(slots)
    return out


def create(ev: Event, picks: List[int], insert_at: int) -> Tuple[int, List[Slot]]:
    """Crea i round di reflight e li inserisce a insert_at. Restituisce (manche, slot creati)."""
    manche = next_manche(ev)
    rounds = build_rounds(ev, picks, manche)
    new = [s for r in rounds for s in r]
    ev.slots[3 * insert_at:3 * insert_at] = new
    if ev.current >= insert_at:
        ev.current += len(rounds)
    return manche, new


def remove(ev: Event, new_slots: List[Slot]) -> bool:
    ids = {id(s) for s in new_slots}
    idx = [i for i, s in enumerate(ev.slots) if id(s) in ids]
    if not idx:
        return False
    first = idx[0]
    ev.slots[:] = [s for s in ev.slots if id(s) not in ids]
    if ev.current > first // 3:
        ev.current -= len(idx) // 3
    return True


# ---------------------------------------------------------------- sostituzione
def revert(ev: Event, rs: Slot) -> bool:
    """Rimette nello slot originale il valore che c'era prima della sostituzione."""
    if not rs.applied:
        return False
    t = find_target(ev, rs)
    if t is not None and rs.backup is not None:
        restore(t, rs.backup)
    rs.applied, rs.backup = False, None
    return True


def apply(ev: Event, rs: Slot, target: Slot) -> None:
    """Scrive il risultato del reflight nello slot originale (conservando il valore vecchio)."""
    if rs.applied:
        revert(ev, rs)
    rs.backup = snapshot(target)
    restore(target, snapshot(rs))
    rs.target_manche = target.manche
    rs.applied = True


def describe(s: Slot) -> str:
    if s.is_reflight:
        return f"Reflight {s.manche_label} {s.category} round {s.round}"
    return f"{s.category} manche {s.manche} round {s.round}"
