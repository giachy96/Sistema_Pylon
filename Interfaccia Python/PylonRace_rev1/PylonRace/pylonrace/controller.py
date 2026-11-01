"""Controllo gara: selezione del round, dialogo con la centrale, risultati, notifiche.

Equivale ai moduli VBA SelezioneAvanti / SelezioneIndietro / inviodati / lettura_dati /
DecodeStringone, ma senza polling del foglio: gli eventi arrivano dal collegamento seriale.
"""
from __future__ import annotations

import time
from pathlib import Path
from typing import Callable, Dict, Optional

from PySide6.QtCore import QObject, QTimer, Signal

from . import protocol, scoring, standings, telegram_bot
from .models import Event, Slot
from .voice import announce_text


def _qt_schedule(ms: int, fn: Callable) -> None:
    QTimer.singleShot(max(0, int(ms)), fn)


class RaceController(QObject):
    changed = Signal()              # qualcosa nell'evento è cambiato (aggiorna le viste)
    round_changed = Signal(int)     # nuovo round corrente
    results_applied = Signal(int)   # risultati ricevuti per il round indicato
    log = Signal(str, str)          # messaggio, livello: info|warn|error|tx|rx
    classifica_ready = Signal(str)  # percorso PDF generato

    PENDING_TIMEOUT = 90.0  # secondi: dopo tanto tempo un set parziale di risultati viene scartato

    def __init__(self, race: Event, notifier=None, voice=None, scheduler=None, clock=time.monotonic,
                 output_dir: Optional[Callable[[], Path]] = None, parent=None):
        super().__init__(parent)
        self.race = race
        self.notifier = notifier
        self.voice = voice
        self._schedule = scheduler or _qt_schedule
        self._clock = clock
        self._output_dir = output_dir or (lambda: Path.cwd())
        self.link = None
        self.project_path: Optional[Path] = None
        self._guard_until = 0.0
        self._pending: Dict[int, protocol.ResultLine] = {}
        self._pending_t = 0.0
        self._seq = 0
        self._save_pending = False
        self._auto_timer = QTimer(self)
        self._auto_timer.timeout.connect(self._auto_classifica)

    # ------------------------------------------------------------------ evento / file
    def set_event(self, race: Event, path: Optional[Path] = None) -> None:
        self.race = race
        self.project_path = path
        self._pending.clear()
        self._seq += 1
        self.apply_auto_classifica()
        self.changed.emit()
        self.round_changed.emit(race.current)

    def request_autosave(self) -> None:
        if self._save_pending or not self.project_path:
            return
        self._save_pending = True
        self._schedule(400, self.save_now)

    def save_now(self) -> None:
        self._save_pending = False
        if self.project_path:
            try:
                self.race.save(self.project_path)
            except OSError as exc:
                self.log.emit(f"Salvataggio non riuscito: {exc}", "error")

    # ------------------------------------------------------------------ collegamento
    def set_link(self, link) -> None:
        if self.link is not None:
            try:
                self.link.line_received.disconnect(self.on_line)
                self.link.error.disconnect(self._on_link_error)
            except (RuntimeError, TypeError):
                pass
        self.link = link
        if link is not None:
            link.line_received.connect(self.on_line)
            link.error.connect(self._on_link_error)

    def _on_link_error(self, msg: str) -> None:
        self.log.emit(msg, "error")

    @property
    def connected(self) -> bool:
        return bool(self.link and self.link.is_open)

    def _send(self, line: str) -> bool:
        if not self.connected:
            self.log.emit("Centrale non collegata: comando non inviato", "warn")
            return False
        ok = self.link.send_line(line)
        if ok:
            self.log.emit(line, "tx")
        return ok

    # ------------------------------------------------------------------ selezione round
    def _guard_active(self) -> bool:
        if self._clock() < self._guard_until:
            self.log.emit("Aspetta qualche altro secondo prima del prossimo round (reset della centrale)", "warn")
            return True
        return False

    def go_next(self, source: str = "pc") -> bool:
        ev = self.race
        if ev.n_rounds == 0:
            self.log.emit("Nessun ordine di volo: generalo dalla scheda Sorteggio", "warn")
            return False
        new = 0 if ev.current < 0 else ev.current + 1
        if new >= ev.n_rounds:
            self.log.emit("Ultimo round raggiunto", "info")
            return False
        if self._guard_active():
            return False
        return self._select(new, backward=False, source=source)

    def go_prev(self, source: str = "pc") -> bool:
        ev = self.race
        if ev.current <= 0:
            return False
        if self._guard_active():
            return False
        return self._select(ev.current - 1, backward=True, source=source)

    def jump_to(self, index: int) -> bool:
        if not (0 <= index < self.race.n_rounds):
            return False
        if self._guard_active():
            return False
        return self._select(index, backward=False, source="pc")

    def reset_selection(self) -> None:
        self.race.current = -1
        self._pending.clear()
        self._seq += 1
        self.round_changed.emit(-1)
        self.changed.emit()
        self.request_autosave()

    def _round_pilots(self, slots) -> list:
        return [(s.name, s.surname) for s in slots[:3]]

    def _select(self, index: int, backward: bool, source: str) -> bool:
        ev, st = self.race, self.race.settings
        ev.current = index
        self._pending.clear()
        self._seq += 1
        seq = self._seq
        slots, refuel = ev.round_slots(index), ev.refuel_slots(index)
        s0 = slots[0]
        where = {"central": "dalla centrale", "pc": "dal PC"}.get(source, source)
        self.log.emit(
            f"{'Indietro' if backward else 'Avanti'} {where}: {s0.category} manche {s0.manche} round {s0.round}", "info"
        )

        # 1) soft reset della centrale e, dopo l'attesa, invio dei nomi (inviodati del VBA)
        self._send(protocol.reset_line())

        def send_names():
            if self._seq != seq or ev.current != index:
                return  # nel frattempo è cambiato il round
            code = protocol.CODE_NAMES_BACK if (backward and st.back_uses_750) else protocol.CODE_NAMES_FWD
            line = protocol.encode_command(code, s0.category, s0.manche, s0.round,
                                           self._round_pilots(slots), st.ascii_fold)
            self._send(line)

        self._schedule(int(st.reset_wait_seconds * 1000), send_names)

        # 2) Telegram e voce
        if st.telegram_on and self.notifier:
            self.notifier.send(telegram_bot.format_start_line(slots, refuel, backward))
        if st.voice_on and self.voice:
            self._announce(slots, refuel, seq)
            self._schedule(int(st.second_read_seconds * 1000),
                           lambda: self._announce(slots, refuel, seq) if self._seq == seq else None)

        self.round_changed.emit(index)
        self.changed.emit()
        self.request_autosave()
        return True

    def _announce(self, slots, refuel, seq) -> None:
        if self._seq != seq or not self.voice:
            return
        self.voice.speak(announce_text([s.full_name for s in slots], [s.full_name for s in refuel]))

    # ------------------------------------------------------------------ comandi diretti
    def _command(self, code: int) -> bool:
        slots = self.race.current_slots()
        if slots:
            s0 = slots[0]
            line = protocol.encode_command(code, s0.category, s0.manche, s0.round,
                                           self._round_pilots(slots), self.race.settings.ascii_fold)
        else:
            line = protocol.encode_command(code)
        return self._send(line)

    def send_show(self) -> bool:
        return self._command(protocol.CODE_SHOW)

    def send_start(self) -> bool:
        return self._command(protocol.CODE_START)

    def send_stop(self) -> bool:
        return self._command(protocol.CODE_STOP)

    # ------------------------------------------------------------------ ricezione
    def on_line(self, line: str) -> None:
        self.log.emit(line, "rx")
        parsed = protocol.parse_line(line)
        if parsed is None:
            return
        if isinstance(parsed, protocol.ButtonEvent):
            if self.race.current < 0:
                self.log.emit("Pulsante della centrale ignorato: avvia prima il primo round dal PC", "warn")
                return
            if parsed.command == "next":
                self.go_next("central")
            else:
                self.go_prev("central")
            return
        if self.race.current < 0:
            self.log.emit("Risultati ricevuti ma nessun round selezionato: ignorati", "warn")
            return
        now = self._clock()
        if self._pending and now - self._pending_t > self.PENDING_TIMEOUT:
            self._pending.clear()
        self._pending[parsed.color] = parsed
        self._pending_t = now
        if len(self._pending) == 3:
            self._apply_pending()

    def _apply_pending(self) -> None:
        ev = self.race
        slots = ev.current_slots()
        if len(slots) < 3:
            self._pending.clear()
            return
        for color, res in self._pending.items():
            scoring.apply_result(slots[color], res)
        self._pending.clear()
        self._guard_until = self._clock() + ev.settings.guard_seconds
        idx = ev.current
        self.log.emit(f"Risultati ricevuti: {slots[0].category} manche {slots[0].manche} round {slots[0].round}", "info")
        for s in slots:
            if s.needs_check:
                self.log.emit(f"Totale mancante per {s.full_name}: inserire il punteggio a mano", "warn")
        if ev.settings.telegram_on and self.notifier:
            self.notifier.send(telegram_bot.format_results(slots))
        self.results_applied.emit(idx)
        self.changed.emit()
        self.request_autosave()

    # ------------------------------------------------------------------ modifiche manuali
    def edit_lap(self, slot: Slot, lap_index: int, text: str) -> None:
        text = text.strip()
        slot.laps[lap_index] = "" if text == "/" else text
        if slot.kind == "":
            slot.kind = "manual"
        if slot.total:
            scoring.recompute(slot)
        else:
            slot.cuts = scoring.count_cuts(slot.laps)
        self._edited()

    def edit_total(self, slot: Slot, text: str) -> None:
        slot.total = "" if text.strip() == "/" else text.strip()
        if slot.kind == "":
            slot.kind = "manual"
        scoring.recompute(slot)
        self._edited()

    def edit_score(self, slot: Slot, value: Optional[float]) -> None:
        slot.score = value
        slot.kind = "manual" if value is not None else ""
        slot.needs_check = value is None and bool(slot.total)
        if slot.cuts is None and value is not None:
            slot.cuts = scoring.count_cuts(list(slot.laps) + [slot.total])
        self._edited()

    def clear_result(self, slot: Slot) -> None:
        slot.clear_result()
        self._edited()

    def _edited(self) -> None:
        self.changed.emit()
        self.request_autosave()

    # ------------------------------------------------------------------ classifica
    def has_any_result(self) -> bool:
        return any(s.score is not None for s in self.race.slots)

    def generate_classifica(self, notify: bool = True) -> Optional[Path]:
        from . import exporters

        if not self.has_any_result():
            self.log.emit("Nessun dato per generare la classifica", "warn")
            return None
        st = standings.compute_all(self.race)
        out = Path(self._output_dir())
        out.mkdir(parents=True, exist_ok=True)
        path = out / "Classifica.pdf"
        try:
            exporters.classifica_pdf(self.race, st, path)
        except Exception as exc:
            self.log.emit(f"Classifica PDF non generata: {exc}", "error")
            return None
        self.log.emit(f"Classifica aggiornata: {path}", "info")
        if notify and self.race.settings.telegram_on and self.notifier:
            self.notifier.send(telegram_bot.format_ranking(self.race.settings.classifica_link))
        self.classifica_ready.emit(str(path))
        return path

    def apply_auto_classifica(self) -> None:
        st = self.race.settings
        self._auto_timer.stop()
        if st.auto_classifica:
            self._auto_timer.start(max(1, int(st.classifica_minutes)) * 60_000)

    def _auto_classifica(self) -> None:
        self.generate_classifica(notify=True)
