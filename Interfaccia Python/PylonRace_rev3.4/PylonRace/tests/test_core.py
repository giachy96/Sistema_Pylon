import random
import warnings

import pytest

from pylonrace import draw, protocol, scoring, standings
from pylonrace.importer import DUMMY_MARKS, import_xlsm
from pylonrace.models import Event, Pilot, Slot


# ---------------------------------------------------------------- protocollo
def test_parse_buttons():
    assert protocol.parse_line("800,0,0,0,0,0,0,0,0,0").command == "next"
    assert protocol.parse_line("700,0,0,0,0,0,0,0,0,0\r\n").command == "prev"


def test_parse_result_normal():
    r = protocol.parse_line("5514,9.693,6.294,5.874,5.818,6.321,5.708,5.479,5.598,5.629,8.138,64.552")
    assert (r.color, r.kind, r.total) == (0, "normal", "64.552")
    assert r.laps[0] == "9.693" and len(r.laps) == 10


def test_parse_result_colors_and_kinds():
    assert protocol.parse_line("5524,1,2,3,4,5,6,7,8,9,10,55").color == 1
    assert protocol.parse_line("5534,1,2,3,4,5,6,7,8,9,10,55").color == 2
    assert protocol.parse_line("6524,,,,,,,,,,,").kind == "timeout"
    assert protocol.parse_line("4035,5.1-P1-P2,,,,,,,,,,").kind == "double_cut"
    assert protocol.parse_line("garbage") is None
    assert protocol.parse_line("") is None
    assert protocol.parse_line("5504,1") is None


def test_encode_command_basic():
    line = protocol.encode_command(850, "Q500", 2, 3, [("Giacomo", "Rossi"), ("A", "B"), ("C", "D")])
    assert line == "850,Q500,2,3,Giacomo,Rossi,A,B,C,D"


def test_encode_sanitizes_and_fits():
    line = protocol.encode_command(
        850, "F3E", 12, 12,
        [("Christophe", "Crèvecoeur-Longhissimo-Cognome"), ("Joris, Jr", "Van Der Bosch Von Wolf"),
         ("Abdulla", "Al-Thani Al-Hemaidi")],
    )
    assert len(line) <= protocol.MAX_LINE
    assert line.count(",") == 9          # nessuna virgola dentro ai nomi
    assert all(ord(c) < 128 for c in line)  # niente accenti sul display
    assert "Crevecoeur" in line or "Crevecoe" in line


def test_reset_line_matches_excel():
    assert protocol.reset_line() == "1400,,,,,,,,,"


# ------------------------------------------------------------------ punteggi
def test_scoring_rules():
    laps = [f"{5 + i / 10:.3f}" for i in range(10)]
    assert scoring.compute_score(laps, "64.552", "normal") == (0, 64.552)
    cut = laps[:]
    cut[6] = "5.004-P1"
    assert scoring.compute_score(cut, "56.392", "normal") == (1, 62.031)   # riga reale del file
    two = cut[:]
    two[7] = "5.1-P2"
    assert scoring.compute_score(two, "50", "normal") == (2, 200.0)
    dbl = laps[:]
    dbl[2] = "5.0-P1-P2"
    assert scoring.compute_score(dbl, "50", "normal")[1] == 200.0
    assert scoring.compute_score(laps, "", "timeout") == (0, 200.0)
    assert scoring.compute_score(laps, "", "normal") == (0, None)  # inghippo: da inserire a mano


# ---------------------------------------------------------------- sorteggio
@pytest.mark.parametrize("n", [3, 6, 9, 12, 15, 30])
def test_matrix_each_pilot_once_per_manche(n):
    for manche in range(1, 13):
        m = draw.genera_matrice(manche, n)
        flat = sorted(x for r in m for x in r)
        assert flat == list(range(1, n + 1)), (n, manche)


def test_matrix_known_values():
    # manche 1: ordine sequenziale a colonne; con 9 piloti: 3 round
    assert draw.genera_matrice(1, 9) == [[1, 4, 7], [2, 5, 8], [3, 6, 9]]
    # manche 3 (mod 3 == 0): colonne ruotate (c2, c3, c1)
    m1 = draw.genera_matrice(1, 9)
    m3 = draw.genera_matrice(3, 9)
    assert m3 != m1
    assert sorted(x for r in m3 for x in r) == list(range(1, 10))


def _mk_event(counts, pairs=0, seed=3):
    """Evento sintetico; `pairs` coppie pilota<->meccanico scelte a caso nella categoria."""
    ev = Event()
    rnd = random.Random(seed)
    for cat, n in counts.items():
        for i in range(n):
            ev.pilots.append(Pilot(f"{cat}{i + 1}", cat, f"N{cat}{i}", f"S{cat}{i}"))
        ps = ev.pilots_of(cat)
        idx = list(range(len(ps)))
        rnd.shuffle(idx)
        for k in range(pairs):
            a, b = ps[idx[2 * k]], ps[idx[2 * k + 1]]
            a.mech_name, a.mech_surname = b.name, b.surname
            b.mech_name, b.mech_surname = a.name, a.surname
    return ev


def test_schedule_structure_and_fillers():
    ev = _mk_event({"Q500": 3, "Q40": 7, "F3D": 8, "F3E": 0})
    rep = draw.generate_schedule(ev, 4, ["Q500", "Q40", "F3D", "F3E"], mode=draw.MODE_LIST_ORDER, seed=1)
    assert "F3E" in rep.skipped
    assert len(ev.slots) % 3 == 0
    for cat, n, rounds in (("Q500", 3, 1), ("Q40", 7, 3), ("F3D", 8, 3)):
        for m in range(1, 5):
            sl = [s for s in ev.slots if s.category == cat and s.manche == m]
            assert len(sl) == rounds * 3
            real = sorted(s.pilot_id for s in sl if not s.dummy)
            assert real == sorted(p.id for p in ev.pilots_of(cat))
            assert sum(s.dummy for s in sl) == rounds * 3 - n
    # riempimento: 7 piloti -> 2 fantasmi ("-------", "!!!!!!"); 8 -> 1 ("-------")
    q40 = [s.name for s in ev.slots if s.category == "Q40" and s.manche == 1 and s.dummy]
    assert sorted(q40) == sorted(["-------", "!!!!!!"])
    f3d = [s.name for s in ev.slots if s.category == "F3D" and s.manche == 1 and s.dummy]
    assert f3d == ["-------"]


def test_category_order_respected():
    ev = _mk_event({"Q500": 3, "Q40": 3, "F3D": 3, "F3E": 3})
    draw.generate_schedule(ev, 2, ["F3D", "Q40", "F3E", "Q500"], mode=draw.MODE_LIST_ORDER)
    seq = []
    for s in ev.slots:
        if not seq or seq[-1] != (s.manche, s.category):
            seq.append((s.manche, s.category))
    assert seq == [(1, "F3D"), (1, "Q40"), (1, "F3E"), (1, "Q500"),
                   (2, "F3D"), (2, "Q40"), (2, "F3E"), (2, "Q500")]


def test_conflict_avoidance_resolves():
    ev = _mk_event({"Q40": 18}, pairs=9)
    before = draw.count_conflicts(ev.pilots_of("Q40"), 6)
    assert before > 0  # la configurazione di partenza ha davvero dei conflitti
    rep = draw.generate_schedule(ev, 6, ["Q40"], mode=draw.MODE_AVOID, seed=7)
    assert rep.conflicts["Q40"] == 0
    assert draw.count_conflicts(ev.pilots_of("Q40"), 6) == 0
    # e senza conflitti iniziali l'ordine dell'elenco non viene toccato (come in Excel)
    ev2 = _mk_event({"Q40": 9})
    ids = [p.id for p in ev2.pilots]
    draw.generate_schedule(ev2, 3, ["Q40"], mode=draw.MODE_AVOID, seed=1)
    assert [p.id for p in ev2.pilots] == ids


# -------------------------------------------------- regressione sul file reale
def _approx(a, b, tol=0.002):
    return abs(float(a) - float(b)) <= tol


def test_import_counts(sample_path):
    ev = import_xlsm(sample_path)
    assert len(ev.pilots) == 53
    assert ev.n_rounds == 114
    assert all(p.name not in DUMMY_MARKS for p in ev.pilots)
    assert {s.color for s in ev.slots} == {0, 1, 2}


def _read_excel_classifica(sample_path):
    """Legge il foglio 'Classifica' di Excel: punteggi per manche, tagli (da colore cella),
    scarti (da barrato), best, totale e ABS calcolati da Excel."""
    import openpyxl

    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        wb = openpyxl.load_workbook(sample_path, data_only=True)
    ws = wb["Classifica"]
    data, cat = {}, None
    for r in range(8, ws.max_row + 1):
        place, naz, fai, name, surname = (ws.cell(r, c).value for c in range(1, 6))
        if place is None and name in ("Q500", "Q40", "F3D", "F3E"):
            cat = name
            data[cat] = []
            continue
        if not cat or place is None or not name or name in ("-------", "!!!!!!"):
            continue
        man, cuts, struck = [], [], []
        for c in range(6, 18):
            cell = ws.cell(r, c)
            man.append(cell.value)
            rgb = cell.fill.fgColor.rgb if cell.fill and cell.fill.fill_type else None
            cuts.append(1 if rgb == "FFFFFFCC" else 2 if rgb == "FF99FFFF" else 0)
            struck.append(bool(cell.font and cell.font.strike))
        data[cat].append(dict(place=place, naz=naz, fai=fai, name=name, surname=surname, man=man,
                              cuts=cuts, struck=struck, best=ws.cell(r, 18).value,
                              total=ws.cell(r, 19).value, abs=ws.cell(r, 20).value))
    return data


def test_standings_match_excel(sample_path):
    """Con gli stessi punteggi di manche, la classifica calcolata coincide con quella di Excel
    (posizioni, totale con scarti, best manche, punteggio ABS, manche scartate)."""
    data = _read_excel_classifica(sample_path)
    assert {k: len(v) for k, v in data.items()} == {"Q500": 3, "Q40": 29, "F3D": 13, "F3E": 8}
    ev = Event()
    for cat, rows in data.items():
        for i, d in enumerate(rows, start=1):
            pid = f"{cat}{i}"
            ev.pilots.append(Pilot(pid, cat, d["name"], d["surname"], nation=d["naz"] or "", fai=str(d["fai"] or "")))
            for m, v in enumerate(d["man"], start=1):
                if v is not None:
                    ev.slots.append(Slot(cat, m, 1, 0, pid, d["name"], d["surname"], score=float(v), cuts=d["cuts"][m - 1]))
    result = {c.category: c for c in standings.compute_all(ev, fai_split=False)}
    assert set(result) == set(data)
    checked = 0
    for cat, rows in data.items():
        got = result[cat].rows
        assert [f"{r.pilot.name} {r.pilot.surname}" for r in got] == [f"{d['name']} {d['surname']}" for d in rows], cat
        for r, d in zip(got, rows):
            tag = (cat, d["name"], d["surname"])
            assert r.place == d["place"], tag
            assert _approx(r.total, d["total"]), (tag, r.total, d["total"])
            assert abs(r.abs_score - d["abs"]) <= 1, (tag, r.abs_score, d["abs"])
            assert _approx(r.best, d["best"]), (tag, r.best, d["best"])
            assert r.struck[:len(d["struck"])] == d["struck"][:len(r.struck)], (tag, r.struck, d["struck"])
            checked += 1
    assert checked == 53


def test_discards_thresholds():
    assert [standings.discards_for(n) for n in (0, 3, 4, 7, 8, 11, 12)] == [0, 0, 1, 1, 2, 2, 3]


def test_fai_split_groups():
    ev = Event()
    for i, fai in enumerate(["100", "", "200", ""]):
        ev.pilots.append(Pilot(f"T{i}", "Q40", f"N{i}", f"S{i}", fai=fai))
    for i, sc in enumerate([60.0, 55.0, 58.0, 70.0]):
        ev.slots.append(Slot("Q40", 1, 1, i % 3, f"T{i}", f"N{i}", f"S{i}", score=sc, cuts=0))
    r = standings.compute_category(ev, "Q40", fai_split=True)
    assert [x.group for x in r.rows] == ["FAI", "FAI", "NON FAI", "NON FAI"]
    assert [x.place for x in r.rows] == [1, 2, 1, 2]
    assert r.rows[0].pilot.id == "T2"  # 58 batte 60 tra i FAI


def test_pure_random_mode_shuffles_without_check():
    from pylonrace import draw
    from pylonrace.models import Event, Pilot

    ev = Event()
    for i in range(1, 10):
        ev.pilots.append(Pilot(f"T{i}", "Q40", f"N{i}", f"C{i}", f"M{i}", f"MC{i}"))
    before = [p.id for p in ev.pilots]
    rep = draw.generate_schedule(ev, 3, ["Q40"], mode=draw.MODE_PURE, seed=3)
    assert sorted(p.id for p in ev.pilots) == sorted(before)
    assert [p.id for p in ev.pilots] != before
    assert rep.conflicts["Q40"] >= 0 and len(ev.slots) == 3 * 9
