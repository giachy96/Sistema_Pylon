import json

import pytest

from pylonrace import reflight, standings
from pylonrace.controller import RaceController
from pylonrace.importer import import_xlsm
from pylonrace.models import Event


@pytest.fixture
def ev(sample_path):
    e = import_xlsm(sample_path)
    for s in e.slots:
        s.clear_result()
    return e


def _idx(ev, cat, manche, rnd, color=0):
    return next(i for i, s in enumerate(ev.slots)
                if (s.category, s.manche, s.round, s.color) == (cat, manche, rnd, color))


def _fly(s, total="60.000", score=60.0, cuts=0):
    s.laps = ["6.000"] * 10
    s.total, s.score, s.cuts, s.kind = total, score, cuts, "normal"


def _ctl(ev):
    return RaceController(ev, scheduler=lambda ms, fn: fn(), clock=lambda: 0.0)


def _pos(ev):
    """Prima posizione ammessa: subito dopo l'ultimo round volato o in corso."""
    return reflight._last_locked_round(ev) + 1


def _rr(ev, k=0):
    """Slot del k-esimo round di reflight."""
    return ev.round_slots(reflight.reflight_rounds(ev)[k])


def test_create_numbering_colors_and_dummies(ev):
    a, b = _idx(ev, "Q40", 1, 1, 0), _idx(ev, "Q40", 2, 1, 0)   # stesso colore -> conflitto
    for i in (a, b):
        _fly(ev.slots[i])
    n0 = ev.n_rounds
    ctl = _ctl(ev)
    assert ctl.create_reflight([a, b], _pos(ev)) is None
    assert ev.n_rounds == n0 + 1
    r = _rr(ev)
    assert all(s.manche == 100 and s.category == "Q40" and s.round == 1 for s in r)
    assert [s.color for s in r] == [0, 1, 2]                      # R, V, B nell'ordine di selezione
    assert [s.pilot_id for s in r[:2]] == [ev.slots[a].pilot_id, ev.slots[b].pilot_id]
    assert [s.dummy for s in r] == [False, False, True]
    assert r[0].target_manche == 1 and r[1].target_manche == 2
    # secondo reflight: manche 101, round di nuovo da 1
    c = _idx(ev, "Q500", 1, 1, 1)
    _fly(ev.slots[c])
    assert ctl.create_reflight([c], _pos(ev)) is None
    new = {(_rr(ev, k)[0].manche, _rr(ev, k)[0].category, _rr(ev, k)[0].round) for k in (0, 1)}
    assert new == {(100, 'Q40', 1), (101, 'Q500', 1)}


def test_keep_original_colors_without_clash(ev):
    a, b = _idx(ev, "Q40", 1, 1, 0), _idx(ev, "Q40", 1, 1, 2)
    for i in (a, b):
        _fly(ev.slots[i])
    ctl = _ctl(ev)
    assert ctl.create_reflight([b, a], _pos(ev)) is None
    r = _rr(ev)
    assert r[0].color == 0 and r[2].color == 2 and r[1].dummy


def test_more_than_three_makes_more_rounds_and_per_category(ev):
    picks = []
    for rnd in (1, 2):
        for col in range(3):
            i = _idx(ev, "Q40", 1, rnd, col)
            _fly(ev.slots[i])
            picks.append(i)
    d = _idx(ev, "F3D", 1, 1, 0)
    _fly(ev.slots[d])
    picks.append(d)
    ctl = _ctl(ev)
    assert ctl.create_reflight(picks, _pos(ev)) is None
    rf = [ev.round_slots(r)[0] for r in reflight.reflight_rounds(ev)]
    # ordine delle categorie come da impostazioni; round da 1 per ogni categoria
    exp = [(c, 100, n) for c, n in (("Q40", 1), ("Q40", 2), ("F3D", 1))]
    cats = ev.settings.category_order
    exp.sort(key=lambda t: (cats.index(t[0]), t[2]))
    assert [(s.category, s.manche, s.round) for s in rf] == exp
    assert all(sum(1 for x in r if x.dummy) == (2 if r[0].category == "F3D" else 0) for r in map(ev.round_slots, reflight.reflight_rounds(ev)))


def test_position_rules(ev):
    i = _idx(ev, "Q40", 1, 1)
    _fly(ev.slots[i])
    ev.current = max(5, i // 3)
    cur = ev.current
    ctl = _ctl(ev)
    assert "prima del round" in ctl.create_reflight([i], cur)
    assert "Posizione" in ctl.create_reflight([i], 10 ** 6)
    assert "almeno" in ctl.create_reflight([], cur + 1)
    nr = _idx(ev, "Q40", 3, 1)
    assert "risultato" in ctl.create_reflight([nr], cur + 1)     # volo senza risultato
    assert ctl.create_reflight([i], cur + 1) is None
    assert ev.current == cur
    assert "già un reflight" in ctl.create_reflight([i], cur + 1)  # doppione in sospeso


def test_apply_replaces_value_and_standings_ignore_reflight(ev):
    i = _idx(ev, "Q40", 1, 1)
    p = ev.slots[i]
    _fly(p, "90.000", 200.0, 2)
    ctl = _ctl(ev)
    assert ctl.create_reflight([i], _pos(ev)) is None
    rs = _rr(ev)[0]
    _fly(rs, "58.000", 58.0, 0)
    before = next(r for r in standings.compute_category(ev, "Q40").rows if r.pilot.id == p.pilot_id)
    assert before.scores[0] == 200.0                              # il reflight non entra da solo
    assert reflight.pending_rounds(ev) == reflight.reflight_rounds(ev)
    prop = ctl.reflight_proposals(reflight.reflight_rounds(ev)[0])
    assert prop[0][1] is p
    assert ctl.apply_reflight([(rs, p)]) is None
    assert (p.total, p.score, p.cuts) == ("58.000", 58.0, 0)
    assert rs.applied and rs.backup["score"] == 200.0
    assert reflight.pending_rounds(ev) == []
    after = next(r for r in standings.compute_category(ev, "Q40").rows if r.pilot.id == p.pilot_id)
    assert after.scores[0] == 58.0
    # nessun doppio conteggio: il reflight non crea una manche in più
    assert standings.compute_category(ev, "Q40").n_manches == len(before.scores)
    # annulla -> torna il valore vecchio
    assert ctl.undo_edit() is None
    assert p.score == 200.0 and not rs.applied


def test_change_target_and_reapply(ev):
    a = _idx(ev, "Q40", 1, 1)
    pid = ev.slots[a].pilot_id
    other = next(s for s in ev.slots if s.pilot_id == pid and s.category == "Q40" and s.manche == 2)
    _fly(ev.slots[a], "90.000", 200.0, 2)
    _fly(other, "70.000", 70.0, 0)
    ctl = _ctl(ev)
    ctl.create_reflight([a], _pos(ev))
    rs = _rr(ev)[0]
    _fly(rs, "58.000", 58.0, 0)
    assert other in reflight.target_choices(ev, rs)
    assert ctl.apply_reflight([(rs, other)]) is None
    assert other.score == 58.0 and ev.slots[a].score == 200.0
    # cambio idea: lo applico sul primo volo; il secondo torna com'era
    assert ctl.apply_reflight([(rs, ev.slots[a])]) is None
    assert other.score == 70.0 and ev.slots[a].score == 58.0
    # bersaglio non valido
    d = _idx(ev, "F3D", 1, 1)
    assert "non valido" in ctl.apply_reflight([(rs, ev.slots[d])])


def test_undo_create_and_blocked_when_flown(ev):
    i = _idx(ev, "Q40", 1, 1)
    _fly(ev.slots[i])
    ctl = _ctl(ev)
    n0 = ev.n_rounds
    ctl.create_reflight([i], _pos(ev))
    assert ctl.undo_edit() is None and ev.n_rounds == n0
    ctl.create_reflight([i], _pos(ev))
    _fly(_rr(ev)[0])
    assert "volato" in ctl.undo_edit()


def test_wire_line_keeps_category_and_uses_manche_100(ev):
    i = _idx(ev, "Q500", 1, 1)
    _fly(ev.slots[i])
    sent = []
    ctl = _ctl(ev)
    ctl._send = lambda line: sent.append(line) or True
    ctl.create_reflight([i], _pos(ev))
    ctl.jump_to(reflight.reflight_rounds(ev)[0])
    assert any(l.startswith("850,Q500,100,1,") for l in sent), sent


def test_save_load_roundtrip(ev, tmp_path):
    i = _idx(ev, "Q40", 1, 1)
    _fly(ev.slots[i], "90.000", 200.0, 2)
    ctl = _ctl(ev)
    ctl.create_reflight([i], _pos(ev))
    rs = _rr(ev)[0]
    _fly(rs, "58.000", 58.0, 0)
    ctl.apply_reflight([(rs, ev.slots[i])])
    p = tmp_path / "g.json"
    ev.save(p)
    e2 = Event.load(p)
    r2 = _rr(e2)[0]
    assert r2.is_reflight and r2.applied and r2.backup["score"] == 200.0 and r2.target_manche == 1
    json.loads(p.read_text())


def test_telegram_reflight_marker(ev):
    from pylonrace import telegram_bot as tb

    i = _idx(ev, "Q40", 1, 1)
    _fly(ev.slots[i])
    _ctl(ev).create_reflight([i], _pos(ev))
    txt = tb.format_start_line(_rr(ev), [])
    assert "(Reflight 1)" in txt and "Race : Q40" in txt
    assert "Reflight" not in tb.format_start_line(ev.round_slots(0), [])


def _ctl_rank(ev, tmp_path):
    ctl = RaceController(ev, None, None, scheduler=lambda ms, fn: None, clock=lambda: 0.0,
                         output_dir=lambda: tmp_path)
    calls = []
    real = ctl.generate_classifica
    ctl.generate_classifica = lambda notify=True: calls.append(1) or None
    return ctl, calls


def _complete_round(ctl, r):
    """Come se la centrale avesse mandato i risultati del round r."""
    ctl.race.current = r
    for s in ctl.race.round_slots(r):
        if not s.dummy:
            _fly(s)
    ctl._auto_classifica_after(ctl.race.round_slots(r)[0])


def test_auto_classifica_end_of_category_and_manche(ev, tmp_path):
    ev.settings.classifica_mode = "category"
    ctl, calls = _ctl_rank(ev, tmp_path)
    # round di Q500 manche 1: aggiorna solo quando l'ultimo round di Q500 manche 1 è completo
    rounds = [r for r in range(ev.n_rounds) if (ev.round_slots(r)[0].category, ev.round_slots(r)[0].manche) == ("Q500", 1)]
    for r in rounds[:-1]:
        _complete_round(ctl, r)
    assert calls == []
    _complete_round(ctl, rounds[-1])
    assert calls == [1]
    # a livello di manche serve che finiscano tutte le categorie
    ev.settings.classifica_mode = "manche"
    calls.clear()
    m1 = [r for r in range(ev.n_rounds) if ev.round_slots(r)[0].manche == 1]
    for r in m1[:-1]:
        _complete_round(ctl, r)
    assert calls == []
    _complete_round(ctl, m1[-1])
    assert calls == [1]
    # modalità manuale o a tempo: mai a fine round
    ev.settings.classifica_mode = "off"
    calls.clear()
    _complete_round(ctl, m1[-1])
    assert calls == []


def test_auto_classifica_after_reflight_confirmation(ev, tmp_path):
    ev.settings.classifica_mode = "manche"
    i = _idx(ev, "Q40", 1, 1)
    _fly(ev.slots[i], "90.000", 200.0, 2)
    ctl, calls = _ctl_rank(ev, tmp_path)
    ctl.create_reflight([i], _pos(ev))
    rs = _rr(ev)[0]
    _fly(rs, "58.000", 58.0, 0)
    ctl.apply_reflight([(rs, ev.slots[i])])
    assert calls == [1]


def test_old_files_with_timer_keep_timer_mode(ev):
    d = ev.to_dict()
    d["settings"].pop("classifica_mode")
    d["settings"]["auto_classifica"] = True
    assert Event.from_dict(d).settings.classifica_mode == "timer"
    d["settings"]["auto_classifica"] = False
    assert Event.from_dict(d).settings.classifica_mode == "off"
