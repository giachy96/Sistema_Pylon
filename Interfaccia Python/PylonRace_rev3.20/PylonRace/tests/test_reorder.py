import pytest

from pylonrace import reorder
from pylonrace.controller import RaceController
from pylonrace.importer import import_xlsm


@pytest.fixture
def ev(sample_path):
    e = import_xlsm(sample_path)
    for s in e.slots:
        s.clear_result()
    return e


def _first_index(ev, cat, manche, rnd):
    return next(i for i, s in enumerate(ev.slots) if (s.category, s.manche, s.round) == (cat, manche, rnd))


def test_swap_rules(ev):
    i = _first_index(ev, "Q40", 1, 1)
    j = _first_index(ev, "Q40", 1, 4)
    assert reorder.check_swap(ev, i, j) is None
    other_cat = _first_index(ev, "F3D", 1, 1)
    assert "categorie diverse" in reorder.check_swap(ev, i, other_cat)
    other_manche = _first_index(ev, "Q40", 2, 1)
    assert "manche diverse" in reorder.check_swap(ev, i, other_manche)
    assert reorder.check_swap(ev, i, i)
    ev.slots[j].score, ev.slots[j].total = 60.0, "60"
    assert "risultati" in reorder.check_swap(ev, i, j)


def test_swap_moves_pilots_keeps_labels(ev):
    i, j = _first_index(ev, "Q40", 1, 1), _first_index(ev, "Q40", 1, 4)
    a, b = ev.slots[i], ev.slots[j]
    na, nb, ma, mb = a.full_name, b.full_name, a.mech, b.mech
    labels = [(s.category, s.manche, s.round, s.color) for s in ev.slots]
    reorder.swap_pilots(ev, i, j)
    assert (a.full_name, a.mech, b.full_name, b.mech) == (nb, mb, na, ma)
    assert labels == [(s.category, s.manche, s.round, s.color) for s in ev.slots]


def test_move_round_across_categories_keeps_numbers(ev):
    n = ev.n_rounds
    src = 30 if ev.round_slots(30)[0].category != ev.round_slots(0)[0].category else 40
    moved = [s for s in ev.round_slots(src)]
    labels = [(s.category, s.manche, s.round) for s in moved]
    final = reorder.final_index(src, 1, after=False)       # prima del round 1
    assert reorder.check_move_round(ev, src, final) is None
    reorder.move_round(ev, src, final)
    assert ev.n_rounds == n
    assert ev.round_slots(final) == moved
    assert [(s.category, s.manche, s.round) for s in ev.round_slots(final)] == labels   # numeri invariati


def test_move_round_blocks(ev):
    ev.current = 5
    assert "in corso" in reorder.check_move_round(ev, 5, 8)
    assert "prima del round in corso" in reorder.check_move_round(ev, 8, 2)
    assert reorder.check_move_round(ev, 8, 6) is None        # subito dopo il corrente: ammesso
    ev.round_slots(9)[0].score = 61.0
    assert "risultati" in reorder.check_move_round(ev, 9, 12)
    ev2_current_follows = ev.round_slots(5)[0]
    reorder.move_round(ev, 8, 6)
    assert ev.round_slots(ev.current)[0] is ev2_current_follows


def test_move_round_current_pointer_follows_when_moving_across(ev):
    ev.current = 4
    first = ev.round_slots(4)[0]
    reorder.move_round(ev, 2, 7)          # un round non ancora volato che stava prima del corrente
    assert ev.round_slots(ev.current)[0] is first


def test_conflict_detection(ev):
    sl = ev.round_slots(0)
    sl[1].mech = sl[0].full_name
    sl[0].dummy = sl[1].dummy = False
    assert reorder.conflicts_in_round(ev, 0)


def test_controller_undo_and_resend(ev):
    sent = []

    class L:
        is_open = True

        def send_line(self, text):
            sent.append(text)
            return True

    ctl = RaceController(ev, scheduler=lambda ms, fn: fn(), clock=lambda: 0.0)
    ctl.link = L()
    i, j = _first_index(ev, "Q40", 1, 1), _first_index(ev, "Q40", 1, 4)
    n1, n2 = ev.slots[i].full_name, ev.slots[j].full_name
    assert ctl.swap_pilots(i, j) is None and ctl.can_undo
    assert (ev.slots[i].full_name, ev.slots[j].full_name) == (n2, n1)
    assert ctl.undo_edit() is None
    assert (ev.slots[i].full_name, ev.slots[j].full_name) == (n1, n2) and not ctl.can_undo
    # undo di un round
    src = 30
    first = ev.round_slots(src)[0]
    assert ctl.move_round(src, 1) is None
    assert ev.slots[3] is first
    assert ctl.undo_edit() is None
    assert ev.slots[3 * src] is first
    # undo bloccato se nel frattempo sono arrivati risultati
    ctl.swap_pilots(i, j)
    ev.slots[i].score = 50.0
    assert "risultati" in ctl.undo_edit()
    assert ctl.can_undo


def test_swap_in_current_round_resends_names(ev):
    sent = []
    ctl = RaceController(ev, scheduler=lambda ms, fn: fn(), clock=lambda: 0.0)
    ctl._send = lambda line: sent.append(line)
    ev.current = _first_index(ev, "Q40", 1, 1) // 3
    i, j = _first_index(ev, "Q40", 1, 1), _first_index(ev, "Q40", 1, 4)
    ctl.swap_pilots(i, j)
    assert sent and sent[0].startswith("1400") and sent[1].startswith("850")
