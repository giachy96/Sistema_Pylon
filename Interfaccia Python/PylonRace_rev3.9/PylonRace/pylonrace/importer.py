"""Import dal vecchio file Excel (Sistema Interfaccia PC Centrale.xlsm)."""
from __future__ import annotations

import warnings
from pathlib import Path

from .models import CATEGORIES, N_LAPS, Event, Pilot, Slot

DUMMY_MARKS = {"-------", "!!!!!!", "??????"}


def _s(v) -> str:
    if v is None:
        return ""
    if isinstance(v, float) and v.is_integer():
        v = int(v)
    return str(v).strip()


def _is_dummy_name(name: str) -> bool:
    return _s(name) in DUMMY_MARKS


def import_xlsm(path: Path, with_results: bool = True) -> Event:
    import openpyxl

    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        wb = openpyxl.load_workbook(path, data_only=True, keep_vba=False)

    ev = Event()
    ws = wb["Piloti"]
    title = _s(ws["E1"].value)
    if title:
        ev.settings.title = " ".join(title.split())

    for row in ws.iter_rows(min_row=8, max_col=10, values_only=True):
        n, cat, name, surname, mname, msurname, nat, fai, phone, pid = (list(row) + [None] * 10)[:10]
        cat = _s(cat)
        if cat not in CATEGORIES or not _s(name) or _is_dummy_name(name):
            continue
        ev.pilots.append(
            Pilot(
                id=_s(pid) or ev.next_pilot_id(cat),
                category=cat,
                name=_s(name),
                surname=_s(surname),
                mech_name=_s(mname),
                mech_surname=_s(msurname),
                nation=_s(nat),
                fai=_s(fai),
                phone=_s(phone),
            )
        )

    if with_results and "GARA" in wb.sheetnames:
        ws = wb["GARA"]
        cat, manche, rnd = "", 0, 0
        color = 0
        for row in ws.iter_rows(min_row=8, max_col=20, values_only=True):
            (a, b, c, d, e, f, g, *rest) = list(row) + [None] * 20
            if not _s(d) and not _s(e):
                continue
            if _s(a):
                cat, manche, rnd, color = _s(a), int(b), int(c), 0
            elif color >= 3:
                color = 0
            laps = [("" if _s(x) in ("", "/") else _s(x)) for x in rest[:N_LAPS]]
            total = _s(rest[N_LAPS]) if len(rest) > N_LAPS else ""
            total = "" if total == "/" else total
            cuts = rest[N_LAPS + 1] if len(rest) > N_LAPS + 1 else None
            score = rest[N_LAPS + 2] if len(rest) > N_LAPS + 2 else None
            dummy = _is_dummy_name(d)
            slot = Slot(
                category=cat,
                manche=manche,
                round=rnd,
                color=color,
                pilot_id="" if dummy else _s(f),
                name=_s(d),
                surname=_s(e),
                mech=_s(g),
                dummy=dummy,
                laps=laps,
                total=total,
                cuts=int(cuts) if isinstance(cuts, (int, float)) else None,
                score=float(score) if isinstance(score, (int, float)) else None,
                kind="manual" if isinstance(score, (int, float)) else "",
            )
            ev.slots.append(slot)
            color += 1
    return ev


def import_countries(path: Path) -> list:
    """[(sigla3, descrizione, sigla2)] dal foglio 'Nazioni'."""
    import openpyxl

    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        wb = openpyxl.load_workbook(path, data_only=True, keep_vba=False)
    ws = wb["Nazioni"]
    out = []
    for a, b, c in ws.iter_rows(min_row=2, max_col=3, values_only=True):
        if a and b:
            out.append([_s(a), _s(b), _s(c)])
    return out
