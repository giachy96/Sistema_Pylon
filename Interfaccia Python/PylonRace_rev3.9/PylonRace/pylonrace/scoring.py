"""Calcolo di tagli e punteggio di una manche (porta di DecodeStringone del VBA)."""
from __future__ import annotations

from typing import Optional, Tuple

from .models import N_LAPS, Slot
from .protocol import ResultLine

DOUBLE_MARKS = ("-P1-P2", "-P1-P3", "-P2-P3")
CUT_PENALTY = 1.10
DNF_SCORE = 200.0


def count_cuts(cells: list) -> int:
    """Conta i tagli: ogni cella con 'P' vale 1; due tagli nello stesso giro portano a 2."""
    n = 0
    for c in cells:
        c = str(c or "")
        if "P" in c:
            n += 1
        if any(m in c for m in DOUBLE_MARKS):
            n = 2
    return n


def to_float(text: str) -> Optional[float]:
    try:
        return float(str(text).strip().replace(",", "."))
    except (ValueError, AttributeError):
        return None


def compute_score(cells: list, total: str, kind: str) -> Tuple[int, Optional[float]]:
    """(tagli, punteggio). Punteggio None se manca il totale (da inserire a mano)."""
    cuts = count_cuts(list(cells) + [total])
    if kind in ("timeout", "double_cut") or cuts >= 2:
        return cuts, DNF_SCORE
    tot = to_float(total)
    if tot is None:
        return cuts, None
    if cuts == 1:
        return cuts, round(tot * CUT_PENALTY, 3)
    return cuts, tot


def apply_result(slot: Slot, res: ResultLine) -> None:
    """Scrive nello slot i tempi ricevuti e il punteggio calcolato."""
    slot.laps = [("" if (c or "").strip() in ("", "/") else c.strip()) for c in res.laps][:N_LAPS]
    slot.total = res.total.strip() if res.total.strip() != "/" else ""
    cuts, score = compute_score(slot.laps, slot.total, res.kind)
    slot.cuts = cuts
    slot.score = score
    slot.kind = res.kind
    slot.needs_check = score is None


def recompute(slot: Slot) -> None:
    """Ricalcola tagli/punteggio dopo una modifica manuale di tempi o totale."""
    kind = slot.kind if slot.kind in ("timeout", "double_cut") else "normal"
    cuts, score = compute_score(slot.laps, slot.total, kind)
    slot.cuts = cuts
    slot.score = score
    slot.needs_check = score is None
