"""Classifica per categoria (porta di GenClassifica / PrelevaDatiGara del VBA)."""
from __future__ import annotations

from dataclasses import dataclass, field
from decimal import ROUND_HALF_UP, Decimal
from typing import Dict, List, Optional

from .models import Event, Pilot


@dataclass
class StandingRow:
    pilot: Pilot
    scores: List[Optional[float]]  # per manche (indice 0 = manche 1)
    cuts: List[Optional[int]]
    struck: List[bool]
    best: Optional[float] = None
    total: float = 0.0
    abs_score: int = 0
    place: Optional[int] = None
    best_overall: bool = False
    group: str = ""  # "FAI" | "NON FAI" | ""

    @property
    def has_results(self) -> bool:
        return any(s is not None for s in self.scores)


@dataclass
class CategoryStandings:
    category: str
    n_manches: int
    discards: int
    rows: List[StandingRow] = field(default_factory=list)


def discards_for(completed_manches: int) -> int:
    """1 scarto dopo 4 manche complete, 2 dopo 8, 3 dopo 12."""
    if completed_manches >= 12:
        return 3
    if completed_manches >= 8:
        return 2
    if completed_manches >= 4:
        return 1
    return 0


def _round_half_up(x: float) -> int:
    return int(Decimal(repr(x)).quantize(Decimal("1"), rounding=ROUND_HALF_UP))


def compute_category(event: Event, category: str, fai_split: Optional[bool] = None) -> CategoryStandings:
    fai_split = event.settings.fai_split if fai_split is None else fai_split
    slots = [s for s in event.slots if s.category == category and not s.is_reflight]  # i reflight contano solo dopo la sostituzione
    n_man = max((s.manche for s in slots), default=0)
    pilots = event.pilots_of(category)

    by_pilot: Dict[str, dict] = {}
    for s in slots:
        if s.dummy:
            continue
        d = by_pilot.setdefault(s.pilot_id, {})
        if s.score is not None:
            d[s.manche] = (s.score, s.cuts or 0)

    # manche "complete": tutti i piloti reali hanno un punteggio
    completed = 0
    for m in range(1, n_man + 1):
        if pilots and all(m in by_pilot.get(p.id, {}) for p in pilots):
            completed = m
    n_disc = discards_for(completed)

    rows: List[StandingRow] = []
    for p in pilots:
        d = by_pilot.get(p.id, {})
        scores = [d[m][0] if m in d else None for m in range(1, n_man + 1)]
        cuts = [d[m][1] if m in d else None for m in range(1, n_man + 1)]
        struck = [False] * n_man
        for _ in range(n_disc):
            vmax, imax = 0.0, -1
            for i, v in enumerate(scores):
                if v is not None and not struck[i] and v > vmax:
                    vmax, imax = v, i
            if imax >= 0:
                struck[imax] = True
        total = sum(v for v, st in zip(scores, struck) if v is not None and not st)
        clean = [v for v, c in zip(scores, cuts) if v is not None and v > 0 and not c]
        best = min(clean) if clean else None
        rows.append(StandingRow(p, scores, cuts, struck, best, round(total, 3)))

    with_res = [r for r in rows if r.total > 0]
    if with_res:
        min_tot = min(r.total for r in with_res)
        for r in with_res:
            r.abs_score = _round_half_up(min_tot / r.total * 1000)
    bests = [r for r in rows if r.best is not None]
    if bests:
        min(bests, key=lambda r: r.best).best_overall = True

    def order(rs: List[StandingRow]) -> List[StandingRow]:
        ranked = sorted([r for r in rs if r.total > 0], key=lambda r: r.total)  # stabile
        rest = [r for r in rs if r.total <= 0]
        for i, r in enumerate(ranked, start=1):
            r.place = i
        return ranked + rest

    if fai_split:
        fai = [r for r in rows if r.pilot.has_fai]
        non = [r for r in rows if not r.pilot.has_fai]
        for r in fai:
            r.group = "FAI"
        for r in non:
            r.group = "NON FAI"
        out = order(fai) + order(non)
    else:
        out = order(rows)
    return CategoryStandings(category, n_man, n_disc, out)


def compute_all(event: Event, fai_split: Optional[bool] = None) -> List[CategoryStandings]:
    return [
        compute_category(event, c, fai_split)
        for c in event.settings.category_order
        if event.pilots_of(c)
    ]
