import os
from pathlib import Path

import pytest

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PySide6.QtCore import QCoreApplication, Qt
from PySide6.QtWidgets import QApplication, QMessageBox


@pytest.fixture(scope="session")
def qapp():
    from pylonrace.ui import theme

    app = QApplication.instance() or QApplication([])
    theme.apply(app)
    return app


@pytest.fixture
def win(qapp, tmp_path, monkeypatch, sample_path):
    monkeypatch.setenv("XDG_CONFIG_HOME", str(tmp_path / "cfg"))
    from pylonrace.config import AppConfig
    from pylonrace.importer import import_xlsm
    from pylonrace.ui.main_window import MainWindow

    cfg = AppConfig()
    cfg.output_dir = str(tmp_path / "out")
    w = MainWindow(cfg, tmp_path / "gara.json")
    w.ctl._schedule = lambda ms, fn: fn() if ms == 0 else w._deferred.append(fn)
    w._deferred = []
    ev = import_xlsm(sample_path)
    ev.settings.guard_seconds = 0
    ev.settings.reset_wait_seconds = 0
    for s in ev.slots:      # parti da una gara senza risultati
        s.clear_result()
    w._load_event(ev, tmp_path / "gara.json")
    w.show()
    yield w
    w.close()
    w.deleteLater()
    QApplication.processEvents()


def _connect_sim(w):
    w.port_box.setCurrentIndex(w.port_box.findData("__SIM__"))
    w.toggle_connection()
    assert w.link.is_open


def test_startup_and_tabs(win):
    assert win.tabs.count() == 5
    assert win.race_tab.proxy.rowCount() == 342
    assert win.pilots_tab.model.rowCount() == 53
    assert "Modena" in win.title_label.text()
    for i in range(win.tabs.count()):
        win.tabs.setCurrentIndex(i)
        QApplication.processEvents()


def test_full_round_via_simulated_central(win):
    _connect_sim(win)
    assert win.race_tab.sim_box.isVisibleTo(win)
    win.race_tab.btn_next.click()
    assert win.ctl.race.current == 0
    assert "Q500" in win.race_tab.round_label.text()
    assert win.link.received[0] == "1400,,,,,,,,,"
    # la centrale manda i risultati (le tre righe) -> punteggi
    win.link.finish_round([("normal", 0), ("normal", 1), ("timeout", 0)], delay_ms=0)
    QApplication.processEvents()
    QCoreApplication.processEvents()
    sl = win.ctl.race.round_slots(0)
    assert all(s.score is not None for s in sl)
    assert sl[2].score == 200.0
    # il pulsante AVANTI della centrale fa avanzare il round
    win.link.press_next()
    assert win.ctl.race.current == 1
    win.link.press_prev()
    assert win.ctl.race.current == 0
    # la classifica si aggiorna
    assert win.stand_tab.tabs.count() >= 1
    win.disconnect_link()
    assert not win.link


def test_edit_cell_updates_score_and_standings(win):
    from pylonrace.ui.tables import C_SCORE, C_TOTAL

    m = win.race_tab.model
    s = win.ctl.race.slots[0]
    assert m.setData(m.index(0, C_TOTAL), "60.000")
    assert s.score == 60.0
    assert m.setData(m.index(0, C_SCORE), "61,5")
    assert s.score == 61.5 and s.kind == "manual"
    assert not m.setData(m.index(0, C_SCORE), "abc")
    assert win.ctl.has_any_result()
    win.stand_tab.rebuild()
    assert win.stand_tab.tabs.count() >= 1


def test_draw_tab_generates(win, monkeypatch):
    monkeypatch.setattr(QMessageBox, "question", lambda *a, **k: QMessageBox.Yes)
    monkeypatch.setattr(QMessageBox, "warning", lambda *a, **k: QMessageBox.Ok)
    win.draw_tab.spin.setValue(4)
    win.draw_tab.generate()
    n_cat = len({p.category for p in win.ctl.race.pilots})
    assert win.ctl.race.n_rounds > 0
    assert max(s.manche for s in win.ctl.race.slots) == 4
    assert "piloti" in win.draw_tab.report.toPlainText()


def test_add_pilot_logic_and_persistence(win, tmp_path):
    from pylonrace.models import Event, Pilot

    ev = win.ctl.race
    n = len(ev.pilots)
    ev.pilots.append(Pilot(ev.next_pilot_id("Q500"), "Q500", "Nuovo", "Pilota"))
    win.pilots_tab.reload()
    win.ctl.save_now()
    assert len(Event.load(tmp_path / "gara.json").pilots) == n + 1
    assert win.pilots_tab.model.rowCount() == n + 1


def test_screenshots(win, tmp_path):
    out = Path(os.environ.get("PYLON_SHOTS", tmp_path / "shots"))
    out.mkdir(parents=True, exist_ok=True)
    _connect_sim(win)
    win.ctl.race.settings.guard_seconds = 0
    win.resize(1900, 960)
    win.race_tab.btn_next.click()
    win.link.finish_round([("normal", 0), ("normal", 1), ("normal", 0)], delay_ms=0)
    win.ctl.race.settings.guard_seconds = 0
    win.race_tab.btn_next.click()
    win.race_tab.btn_next.click()
    for i, name in enumerate(["gara", "piloti", "sorteggio", "classifica", "impostazioni"]):
        win.tabs.setCurrentIndex(i)
        QApplication.processEvents()
        win.grab().save(str(out / f"{name}.png"))
    assert (out / "gara.png").stat().st_size > 5000
