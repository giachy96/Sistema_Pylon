"""Sorteggio e ordine di volo (porta di generamatrice / sorteggio / riempimento / rdm_non_riocorsivo)."""
from __future__ import annotations

import math
import random
import time
from dataclasses import dataclass, field
from typing import Dict, List, Optional

from .models import DUMMY_A, DUMMY_B, Event, Pilot, Slot, norm

N_SLOTS = 3  # piazzole per round (rosso, verde, blu)


def genera_matrice(manche: int, n_piloti: int) -> List[List[int]]:
    """Matrice [round][colore] con i numeri pilota (1-based) di una manche.

    Porta fedele della funzione VBA: rotazione dei gruppi manche dopo manche e
    rotazione dei colori in base a manche mod 3.
    """
    n_round = math.ceil(n_piloti / N_SLOTS)
    man = manche
    if man >= n_round:
        man = man % n_round
    m = [[0] * (N_SLOTS + 1) for _ in range(n_round + 1)]
    c = 1
    for i in range(1, N_SLOTS + 1):
        for j in range(1, n_round + 1):
            if i == 1 or man == 1:
                m[j][i] = c
                c += 1
            elif i == 2:
                val = (c + manche - 2) % n_round + n_round + 1
                m[j][i] = val
                c += 1
            else:  # i == 3
                val = (c + manche - 2) % n_round + n_round + 1
                val = (val + manche - 2) % n_round + 2 * n_round + 1
                m[j][i] = val
                c += 1
    cols = [[m[j][i] for j in range(1, n_round + 1)] for i in range(1, N_SLOTS + 1)]
    if manche % N_SLOTS == 0:
        cols = [cols[1], cols[2], cols[0]]
    elif manche % N_SLOTS == 2:
        cols = [cols[2], cols[0], cols[1]]
    return [[cols[i][j] for i in range(N_SLOTS)] for j in range(n_round)]


def with_fillers(pilots: List[Pilot]) -> List[Optional[Pilot]]:
    """Porta il numero di piloti a multiplo di 3 con piloti fantasma (None) come 'riempimento'."""
    out: List[Optional[Pilot]] = list(pilots)
    rem = len(out) % N_SLOTS
    if rem:
        out += [None] * (N_SLOTS - rem)
    return out


def filler_names(n_real: int) -> List[str]:
    """Nomi mostrati sul tabellone per i fantasmi, come nel VBA."""
    rem = n_real % N_SLOTS
    if rem == 1:
        return [DUMMY_A, DUMMY_B]
    if rem == 2:
        return [DUMMY_A]
    return []


# ---------------------------------------------------------------------------
# Conflitti pilota / meccanico
# ---------------------------------------------------------------------------
def _rounds_for(n_manche: int, n_total: int) -> List[List[List[int]]]:
    return [genera_matrice(k, n_total) for k in range(1, n_manche + 1)]


def count_conflicts(order: List[Pilot], n_manche: int, matrices: Optional[list] = None) -> int:
    """Numero di (manche, round) in cui un pilota vola insieme a chi lo ha come meccanico."""
    full = with_fillers(order)
    if matrices is None:
        matrices = _rounds_for(n_manche, len(full))
    names = [norm(p.full_name) if p else "" for p in full]
    mechs = [norm(p.mech_full) if p else "" for p in full]
    bad = 0
    for mat in matrices:
        for rnd in mat:
            hit = False
            for a in rnd:
                for b in rnd:
                    if a != b and names[a - 1] and names[a - 1] == mechs[b - 1]:
                        hit = True
            if hit:
                bad += 1
    return bad


def shuffle_without_conflicts(
    pilots: List[Pilot],
    n_manche: int,
    rng: random.Random,
    force_shuffle: bool = False,
    max_tries: int = 20000,
    max_seconds: float = 8.0,
) -> tuple:
    """Rimescola l'elenco finché nessun pilota vola con il proprio 'assistito'.

    Come nel VBA, se non ci sono conflitti l'ordine resta invariato (a meno di force_shuffle).
    Ritorna (nuovo_ordine, conflitti_residui, tentativi).
    """
    order = list(pilots)
    n_total = len(with_fillers(order))
    matrices = _rounds_for(n_manche, n_total)
    cur = count_conflicts(order, n_manche, matrices)
    if cur == 0 and not force_shuffle:
        return order, 0, 0
    best, best_c = order, cur
    t0 = time.monotonic()
    tries = 0
    n = len(order)
    while tries < max_tries and time.monotonic() - t0 < max_seconds and best_c > 0:
        cand = order[:]
        rng.shuffle(cand)
        c = count_conflicts(cand, n_manche, matrices)
        tries += 1
        # ricerca locale: scambi casuali, si accetta se i conflitti non aumentano
        for _ in range(60):
            if c == 0 or n < 2:
                break
            i, j = rng.sample(range(n), 2)
            cand[i], cand[j] = cand[j], cand[i]
            c2 = count_conflicts(cand, n_manche, matrices)
            tries += 1
            if c2 <= c:
                c = c2
            else:
                cand[i], cand[j] = cand[j], cand[i]
        if c < best_c or (force_shuffle and best is order):
            best, best_c = cand, c
    return best, best_c, tries


# ---------------------------------------------------------------------------
# Generazione dell'ordine di volo
# ---------------------------------------------------------------------------
@dataclass
class DrawReport:
    conflicts: Dict[str, int] = field(default_factory=dict)
    tries: Dict[str, int] = field(default_factory=dict)
    fillers: Dict[str, int] = field(default_factory=dict)
    skipped: List[str] = field(default_factory=list)


MODE_LIST_ORDER = "list"      # ordine dell'elenco, nessun rimescolamento
MODE_AVOID = "avoid"          # come Excel: rimescola solo se ci sono conflitti
MODE_RANDOM = "random"        # sorteggio casuale + niente conflitti


def generate_schedule(
    event: Event,
    n_manche: int,
    category_order: Optional[List[str]] = None,
    mode: str = MODE_AVOID,
    seed: Optional[int] = None,
) -> DrawReport:
    """Riempie event.slots con l'ordine di volo. L'elenco event.pilots viene riordinato come in Excel."""
    rng = random.Random(seed)
    order = list(category_order or event.settings.category_order)
    report = DrawReport()
    event.settings.category_order = order
    event.sort_pilots_by_category()

    lists: Dict[str, List[Optional[Pilot]]] = {}
    for cat in order:
        pil = event.pilots_of(cat)
        if not pil:
            report.skipped.append(cat)
            continue
        if mode in (MODE_AVOID, MODE_RANDOM):
            pil, left, tries = shuffle_without_conflicts(
                pil, n_manche, rng, force_shuffle=(mode == MODE_RANDOM)
            )
            report.conflicts[cat] = left
            report.tries[cat] = tries
            # riporta l'ordine nell'elenco piloti (come fa Excel sul foglio Piloti)
            idxs = [i for i, p in enumerate(event.pilots) if p.category == cat]
            for i, p in zip(idxs, pil):
                event.pilots[i] = p
        else:
            report.conflicts[cat] = count_conflicts(pil, n_manche)
        full = with_fillers(pil)
        report.fillers[cat] = len(full) - len(pil)
        lists[cat] = full

    slots: List[Slot] = []
    for k in range(1, n_manche + 1):
        for cat in order:
            if cat not in lists:
                continue
            full = lists[cat]
            n_real = len(full) - report.fillers[cat]
            names = filler_names(n_real)
            mat = genera_matrice(k, len(full))
            for r, rnd in enumerate(mat, start=1):
                for color, num in enumerate(rnd):
                    p = full[num - 1]
                    if p is not None:
                        slots.append(Slot(cat, k, r, color, p.id, p.name, p.surname, p.mech_full))
                    else:
                        fill_idx = num - 1 - n_real
                        nm = names[fill_idx] if 0 <= fill_idx < len(names) else DUMMY_A
                        slots.append(Slot(cat, k, r, color, "", nm, nm, "", dummy=True))
    event.slots = slots
    event.current = -1
    return report
