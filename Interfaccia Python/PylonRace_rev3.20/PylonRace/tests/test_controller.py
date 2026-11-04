import pytest
from PySide6.QtWidgets import QApplication

from pylonrace import draw, exporters, standings, telegram_bot
from pylonrace.controller import RaceController
from pylonrace.links import SimulatedCentral
from pylonrace.models import Event, Pilot


@pytest.fixture(scope="session")
def app():
    return QApplication.instance() or QApplication([])


class FakeSched:
    def __init__(self):
        self.q = []

    def __call__(self, ms, fn):
        self.q.append((ms, fn))

    def run(self):
        q, self.q = self.q, []
        for _, fn in q:
            fn()


class FakeClock:
    t = 100.0

    def __call__(self):
        return self.t


class FakeNotifier:
    def __init__(self):
        self.sent = []

    def send(self, text):
        self.sent.append(text)
        return True


class FakeVoice:
    def __init__(self):
        self.said = []

    def speak(self, text):
        self.said.append(text)


def make_event():
    ev = Event()
    for cat, n in (("Q40", 7), ("F3D", 3)):
        for i in range(n):
            ev.pilots.append(Pilot(f"{cat}-{i}", cat, f"Nome{i}", f"Cogn{i}è"))
    draw.generate_schedule(ev, 2, ["Q40", "F3D"], mode=draw.MODE_LIST_ORDER)
    return ev


@pytest.fixture
def rig(app):
    ev = make_event()
    sched, clock = FakeSched(), FakeClock()
    notifier, voice = FakeNotifier(), FakeVoice()
    ctl = RaceController(ev, notifier, voice, scheduler=sched, clock=clock)
    link = SimulatedCentral(sleep=sched, seed=1)
    link.open()
    ctl.set_link(link)
    logs = []
    ctl.log.connect(lambda m, lv: logs.append((lv, m)))
    return ctl, link, sched, clock, notifier, voice, logs


def test_first_round_sends_reset_then_names(rig):
    ctl, link, sched, clock, *_ = rig
    assert ctl.go_next()
    assert ctl.race.current == 0
    assert link.received == ["1400,,,,,,,,,"]            # subito il reset
    sched.run()                                           # dopo l'attesa i nomi
    assert len(link.received) == 2
    line = link.received[1]
    assert line.startswith("850,Q40,1,1,Nome0,Cogn0e,")   # accento rimosso per il display
    assert len(line) <= 98 and line.count(",") == 9


def test_central_buttons_work_only_after_first_round(rig):
    ctl, link, sched, clock, *_ , logs = rig
    link.press_next()                       # sel == 0 nell'Excel: ignorato
    assert ctl.race.current == -1
    ctl.go_next()
    sched.run()
    clock.t += 10
    link.press_next()                       # ora funziona
    assert ctl.race.current == 1
    clock.t += 10
    link.press_prev()
    assert ctl.race.current == 0


def test_results_flow_scoring_and_guard(rig):
    ctl, link, sched, clock, notifier, voice, logs = rig
    ctl.race.settings.telegram_on = True
    ctl.go_next()
    sched.run()
    got = []
    ctl.results_applied.connect(got.append)
    link.finish_round([("normal", 0), ("normal", 1), ("timeout", 0)], delay_ms=0)
    sched.run()
    assert got == [0]
    red, green, blue = ctl.race.round_slots(0)
    assert red.cuts == 0 and red.score == pytest.approx(float(red.total))
    assert green.cuts == 1 and green.score == pytest.approx(round(float(green.total) * 1.1, 3))
    assert blue.score == 200.0 and blue.kind == "timeout"
    # Telegram: messaggio di partenza + messaggio risultati
    assert any("Start Line" in m for m in notifier.sent)
    assert any("RESULTS" in m for m in notifier.sent)
    # guard di 5 s dopo la ricezione dei risultati
    assert ctl.go_next() is False
    clock.t += 6
    assert ctl.go_next() is True and ctl.race.current == 1


def test_partial_results_wait_for_all_three(rig):
    ctl, link, sched, clock, *_ = rig
    ctl.go_next()
    sched.run()
    lines = link.finish_round(delay_ms=0)
    sched.q.clear()
    link.line_received.emit(lines[0])
    link.line_received.emit(lines[2])
    assert ctl.race.round_slots(0)[0].score is None      # ne manca uno
    link.line_received.emit(lines[1])
    assert all(s.score is not None for s in ctl.race.round_slots(0))


def test_missing_total_needs_manual_entry(rig):
    ctl, link, sched, clock, *_ , logs = rig
    ctl.go_next()
    sched.run()
    for c in range(3):
        total = "" if c == 1 else "58.5"
        link.line_received.emit(f"55{c + 1}4," + ",".join(["5.8"] * 10) + f",{total}")
    mid = ctl.race.round_slots(0)[1]
    assert mid.score is None and mid.needs_check
    assert any(lv == "warn" and "Totale mancante" in m for lv, m in logs)
    ctl.edit_score(mid, 61.25)
    assert mid.score == 61.25 and mid.kind == "manual"
    ctl.edit_total(mid, "59.1")
    assert mid.score == pytest.approx(59.1)


def test_stale_names_not_sent_when_round_changed(rig):
    ctl, link, sched, clock, *_ = rig
    ctl.go_next()
    clock.t += 10
    ctl.go_next()                 # due avanti prima che scada l'attesa
    sched.run()
    names = [l for l in link.received if l.startswith("850")]
    assert len(names) == 1 and ",Q40,1,2," in names[0]


def test_voice_announces_and_repeats(rig):
    ctl, link, sched, clock, notifier, voice, logs = rig
    ctl.race.settings.voice_on = True
    ctl.go_next()
    assert voice.said and "At the start line" in voice.said[0] and "At the refueling" in voice.said[0]
    sched.run()   # esegue anche la seconda lettura
    assert len(voice.said) == 2


def test_direct_commands_carry_current_round(rig):
    ctl, link, sched, clock, *_ = rig
    ctl.send_show()
    assert link.received[-1] == "2000,,,,,,,,,"
    ctl.go_next()
    sched.run()
    ctl.send_start()
    assert link.received[-1].startswith("3000,Q40,1,1,Nome0,")
    ctl.send_stop()
    assert link.received[-1].startswith("6000,")


def test_not_connected_warns(app):
    ctl = RaceController(make_event(), scheduler=FakeSched(), clock=FakeClock())
    logs = []
    ctl.log.connect(lambda m, lv: logs.append(lv))
    assert ctl.go_next()           # il round avanza comunque, ma niente invio
    assert "warn" in logs


def test_telegram_formats():
    ev = make_event()
    sl = ev.round_slots(0)
    txt = telegram_bot.format_start_line(sl, ev.refuel_slots(0))
    assert "Start Line" in txt and "Race : Q40" in txt and "Refueling" in txt
    assert "WARNING" in telegram_bot.format_start_line(sl, [], backward=True)
    sl[0].laps = [str(5 + i) for i in range(10)]
    sl[0].total, sl[0].cuts, sl[0].score = "95", 0, 95.0
    res = telegram_bot.format_results(sl)
    assert "LAP: 10 14" in res and "Score: 95.0" in res


def test_telegram_notifier_queue_and_errors():
    out, done = [], []
    import threading

    ev = threading.Event()

    def poster(url, data):
        out.append((url, data))
        ev.set()
        return 200, "ok"

    n = telegram_bot.TelegramNotifier("TOKEN", "-100", on_result=lambda ok, m: done.append(ok), poster=poster)
    assert n.send("ciao")
    assert ev.wait(2)
    assert out[0][0].endswith("/botTOKEN/sendMessage") and out[0][1] == {"chat_id": "-100", "text": "ciao"}
    nn = telegram_bot.TelegramNotifier("", "", on_result=lambda ok, m: done.append(ok), poster=poster)
    assert nn.send("x") is False


def test_exports(tmp_path, app):
    ev = make_event()
    # qualche risultato
    for i, s in enumerate(ev.slots[:12]):
        s.score, s.cuts, s.laps = 55.0 + i, i % 3 == 0 and 1 or 0, [f"5.{i}"] * 10
    res = standings.compute_all(ev)
    pdf = exporters.classifica_pdf(ev, res, tmp_path / "c.pdf")
    xl = exporters.classifica_xlsx(ev, res, tmp_path / "c.xlsx")
    ov = exporters.ordine_volo_xlsx(ev, tmp_path / "o.xlsx")
    op = exporters.ordine_volo_pdf(ev, tmp_path / "o.pdf")
    for p in (pdf, xl, ov, op):
        assert p.exists() and p.stat().st_size > 1000, p
    assert pdf.read_bytes()[:4] == b"%PDF"
    import openpyxl

    wb = openpyxl.load_workbook(ov)
    assert wb.sheetnames == ["Q40", "F3D"]


def test_save_load_roundtrip(tmp_path):
    ev = make_event()
    ev.slots[0].score, ev.slots[0].laps[3] = 61.2, "5.1-P2"
    ev.current = 2
    p = tmp_path / "gara.json"
    ev.save(p)
    ev2 = Event.load(p)
    assert ev2.to_dict() == ev.to_dict()
    assert ev2.slots[0].laps[3] == "5.1-P2" and ev2.current == 2


def test_telegram_custom_templates_and_safe_placeholders():
    ev = make_event()
    sl = ev.round_slots(0)
    t = {"start": "VIA {gara} M{manche} R{round}: {rosso} / {verde} / {blu} - {sconosciuto}",
         "ranking": "Classifica! {link}"}
    txt = telegram_bot.format_start_line(sl, ev.refuel_slots(0), templates=t)
    assert txt.startswith("VIA Q40 M1 R1: ") and "{sconosciuto}" in txt     # segnaposto errato lasciato com'è
    assert telegram_bot.format_ranking("", t) == "Classifica!"
    assert "RESULTS" in telegram_bot.format_results(sl, t)                  # chiavi non personalizzate: predefinito
    # testo vuoto = predefinito
    assert "Start Line" in telegram_bot.format_start_line(sl, [], templates={"start": "  "})


def test_controller_uses_custom_templates():
    sent = []

    class N:
        def send(self, text):
            sent.append(text)
            return True

    ev = make_event()
    ev.settings.telegram_on = True
    ctl = RaceController(ev, notifier=N(), scheduler=FakeSched(), clock=FakeClock())
    ctl.tg_templates["start"] = "PARTENZA {gara}"
    ctl.go_next()
    assert sent and sent[0].startswith("PARTENZA Q40")


def test_telegram_send_document_and_ranking_pdf(tmp_path, app):
    import threading

    # 1) il notificatore invia il file con sendDocument (nome, tipo e didascalia)
    got, ev_done = [], threading.Event()

    def doc_poster(url, data, path):
        got.append((url, data, path))
        ev_done.set()
        return 200, "ok"

    n = telegram_bot.TelegramNotifier("TOKEN", "-100", doc_poster=doc_poster)
    f = tmp_path / "Classifica.pdf"
    f.write_bytes(b"%PDF-1.4 test")
    assert n.send_document(f, "Classifica aggiornata")
    assert ev_done.wait(2)
    assert got[0][0].endswith("/botTOKEN/sendDocument")
    assert got[0][1] == {"chat_id": "-100", "caption": "Classifica aggiornata"} and got[0][2] == str(f)
    assert telegram_bot.TelegramNotifier("", "").send_document(f) is False

    # 2) la classifica generata dal controller parte come PDF (o come solo testo se l'opzione è spenta)
    class Fake(FakeNotifier):
        def __init__(self):
            super().__init__()
            self.docs = []

        def send_document(self, path, caption=""):
            self.docs.append((str(path), caption))
            return True

    ev = make_event()
    for i, s in enumerate(ev.slots[:12]):
        s.score, s.cuts, s.laps = 55.0 + i, 0, [f"5.{i}"] * 10
    ev.settings.telegram_on = True
    fake = Fake()
    ctl = RaceController(ev, fake, FakeVoice(), scheduler=lambda ms, fn: None, output_dir=lambda: tmp_path)
    p = ctl.generate_classifica()
    assert fake.docs and fake.docs[0][0] == str(p) and not fake.sent
    ev.settings.classifica_pdf_telegram = False
    ctl.generate_classifica()
    assert len(fake.docs) == 1 and fake.sent


def test_classifica_tab_telegram_button_sends_pdf(tmp_path, app):
    from pylonrace.ui.classifica_tab import StandingsTab

    class Fake(FakeNotifier):
        ready = True

        def __init__(self):
            super().__init__()
            self.docs = []

        def send_document(self, path, caption=""):
            self.docs.append(str(path))
            return True

    ev = make_event()
    for i, s in enumerate(ev.slots[:12]):
        s.score, s.cuts, s.laps = 55.0 + i, 0, [f"5.{i}"] * 10
    fake = Fake()
    ctl = RaceController(ev, fake, FakeVoice(), scheduler=lambda ms, fn: None, output_dir=lambda: tmp_path)
    tab = StandingsTab(ctl, lambda: tmp_path)
    assert ev.settings.telegram_on is False          # il pulsante manda comunque, è un invio esplicito
    tab._telegram()
    assert len(fake.docs) == 1 and fake.docs[0].endswith("Classifica.pdf") and not fake.sent
    ev.settings.classifica_pdf_telegram = False
    tab._telegram()
    assert len(fake.docs) == 1 and len(fake.sent) == 1


def test_overwritten_results_can_be_undone(rig):
    ctl, link, sched, clock, notifier, voice, logs = rig
    ctl.go_next()
    sched.run()
    link.finish_round([("normal", 0), ("normal", 0), ("normal", 0)], delay_ms=0)
    sched.run()
    first = [(s.total, s.score) for s in ctl.race.round_slots(0)]
    assert not ctl.can_undo                      # primo volo: niente da annullare
    clock.t += 100
    link.finish_round([("timeout", 0), ("timeout", 0), ("timeout", 0)], delay_ms=0)
    sched.run()
    assert all(s.score == 200.0 for s in ctl.race.round_slots(0))
    assert any("sovrascritti" in m for lv, m in logs if lv == "warn")
    assert ctl.undo_edit() is None
    assert [(s.total, s.score) for s in ctl.race.round_slots(0)] == first
