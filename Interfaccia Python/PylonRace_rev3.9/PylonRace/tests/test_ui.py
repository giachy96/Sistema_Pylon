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
    assert "Modena" in win.title_label.toolTip()
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
    win.resize(1500, 800)
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


def test_export_grid(win, tmp_path, monkeypatch):
    import openpyxl
    from PySide6.QtWidgets import QFileDialog

    out = tmp_path / "griglia.xlsx"
    monkeypatch.setattr(QFileDialog, "getSaveFileName", lambda *a, **k: (str(out), ""))
    win.race_tab.cmb_cat.setCurrentIndex(win.race_tab.cmb_cat.findText("F3E"))
    win.race_tab._export_grid()
    ws = openpyxl.load_workbook(out).active
    assert ws.cell(3, 5).value == "Pilota"
    cats = {ws.cell(r, 1).value for r in range(4, ws.max_row + 1)}
    assert cats == {"F3E"}
    assert win.race_tab.proxy.rowCount() == ws.max_row - 3


def test_flight_triplet_colors(win):
    from pylonrace.ui import theme

    win.ctl.race.current = 5
    m = win.race_tab.model
    for k in range(3):
        b = m.data(m.index(15 + k, 4), Qt.BackgroundRole)
        assert b.color().name().upper() == theme.COLOR_FLIGHT[k].upper()


def test_telegram_templates_dialog(win, monkeypatch):
    from pylonrace.ui.telegram_dialog import TelegramTemplatesDialog

    dlg = TelegramTemplatesDialog(win.ctl, win.cfg.telegram_templates)
    dlg.edits["start"].setPlainText("CIAO {gara} {rosso}")
    assert "CIAO" in dlg.prev.toPlainText()
    dlg.accept()
    assert win.cfg.telegram_templates == {"start": "CIAO {gara} {rosso}"}
    assert win.ctl.tg_templates is win.cfg.telegram_templates


def _proxy_index(win, source_row):
    return win.race_tab.proxy.mapFromSource(win.race_tab.model.index(source_row, 4))


def test_move_pilot_flow(win, monkeypatch):
    monkeypatch.setattr(QMessageBox, "question", lambda *a, **k: QMessageBox.Yes)
    monkeypatch.setattr(QMessageBox, "warning", lambda *a, **k: QMessageBox.Ok)
    t = win.race_tab
    ev = win.ctl.race
    i = next(k for k, s in enumerate(ev.slots) if (s.category, s.manche, s.round) == ("Q40", 1, 1))
    j = next(k for k, s in enumerate(ev.slots) if (s.category, s.manche, s.round) == ("Q40", 1, 4))
    n1, n2 = ev.slots[i].full_name, ev.slots[j].full_name
    t.btn_mv_pilot.click()
    assert t.banner.isVisibleTo(t) and not t.btn_mv_round.isEnabled()
    t._table_clicked(_proxy_index(win, i))
    assert i in t.model.marked
    t._table_clicked(_proxy_index(win, j))
    assert (ev.slots[i].full_name, ev.slots[j].full_name) == (n2, n1)
    assert not t.banner.isVisibleTo(t) and t.btn_undo.isEnabled()
    t.btn_undo.click()
    assert (ev.slots[i].full_name, ev.slots[j].full_name) == (n1, n2) and not t.btn_undo.isEnabled()


def test_move_pilot_refused_for_other_category(win, monkeypatch):
    warned = []
    monkeypatch.setattr(QMessageBox, "warning", lambda *a, **k: warned.append(a[2]))
    t, ev = win.race_tab, win.ctl.race
    i = next(k for k, s in enumerate(ev.slots) if s.category == "Q40")
    j = next(k for k, s in enumerate(ev.slots) if s.category == "F3D")
    t.btn_mv_pilot.click()
    t._table_clicked(_proxy_index(win, i))
    t._table_clicked(_proxy_index(win, j))
    assert warned and "categorie diverse" in warned[0]
    assert t._mode == "pilot"          # resta in attesa di un'altra scelta
    t._cancel_move()
    assert not t.banner.isVisibleTo(t)


def test_move_round_flow(win, monkeypatch):
    monkeypatch.setattr(QMessageBox, "warning", lambda *a, **k: QMessageBox.Ok)
    monkeypatch.setattr(QMessageBox, "exec", lambda self: 0)
    monkeypatch.setattr(QMessageBox, "clickedButton", lambda self: self.buttons()[1])   # "Dopo questo round"
    t, ev = win.race_tab, win.ctl.race
    moved = ev.round_slots(30)
    t.btn_mv_round.click()
    t._table_clicked(_proxy_index(win, 3 * 30))
    assert t.model.marked == {90, 91, 92}
    t._table_clicked(_proxy_index(win, 3 * 2))          # dopo il round di indice 2
    assert ev.round_slots(3) == moved
    t.btn_undo.click()
    assert ev.round_slots(30) == moved


def test_reflight_flow_in_ui(win, tmp_path, monkeypatch):
    from pylonrace import reflight
    from pylonrace.ui import reflight_dialog as rd

    _connect_sim(win)
    win.race_tab.btn_next.click()
    win.link.finish_round([("normal", 0), ("normal", 1), ("timeout", 0)], delay_ms=0)
    QApplication.processEvents()
    ev = win.ctl.race
    assert not win.race_tab.btn_rf_ok.isEnabled()
    # finestra di creazione: scelgo il pilota in timeout (blu) del primo round
    dlg = rd.NewReflightDialog(win.ctl, win)
    assert dlg.table.rowCount() == 3
    dlg.table.selectRow(2)
    dlg._add()
    assert len(dlg.picks) == 1 and "Verranno creati" in dlg.preview.text()
    dlg.resize(900, 560)
    out = Path(os.environ.get("PYLON_SHOTS", tmp_path / "shots"))
    out.mkdir(parents=True, exist_ok=True)
    dlg.grab().save(str(out / "reflight_nuovo.png"))
    dlg._accept()
    assert ev.n_rounds % 1 == 0 and reflight.reflight_rounds(ev)
    r = reflight.reflight_rounds(ev)[0]
    assert r == 1 and ev.round_slots(r)[0].manche == 100
    assert "Reflight RF1" in win.race_tab.cmb_man.itemText(win.race_tab.cmb_man.count() - 1)
    # volo il reflight: alla fine si apre la conferma (qui la accetto in automatico)
    shown = {}

    def fake_exec(self):
        shown["rows"] = self.table.rowCount()
        self.grab().save(str(out / "reflight_conferma.png"))
        self._accept()
        return 1

    monkeypatch.setattr(rd.ConfirmReflightDialog, "exec", fake_exec)
    win.race_tab.btn_next.click()
    assert ev.current == 1
    assert "Reflight RF1" in win.race_tab.round_label.text()
    win.link.finish_round([("normal", 0), ("normal", 0), ("normal", 0)], delay_ms=0)
    QApplication.processEvents()
    QCoreApplication.processEvents()
    QApplication.processEvents()
    assert shown.get("rows") == 1
    target = ev.slots[2]
    assert target.score is not None and target.score != 200.0          # sostituito il timeout
    assert not win.race_tab.btn_rf_ok.isEnabled()
    assert win.ctl.can_undo


def test_pdf_telegram_flag_in_settings_classifica_group(win):
    st = win.ctl.race.settings
    assert st.classifica_pdf_telegram is True
    # sta nel gruppo "Gara" delle Impostazioni (con il link classifica), non in quello Telegram
    grp = win.set_tab.pdf_tg.parentWidget()
    assert win.set_tab.link.parentWidget() is grp
    win.set_tab.apply()                      # salvare le impostazioni non lo cambia
    assert st.classifica_pdf_telegram is True and win.set_tab.pdf_tg.isChecked()
    win.set_tab.pdf_tg.setChecked(False)
    assert st.classifica_pdf_telegram is False
    win.set_tab.apply()
    assert st.classifica_pdf_telegram is False
