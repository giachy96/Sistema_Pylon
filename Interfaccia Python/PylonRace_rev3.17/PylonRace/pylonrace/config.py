"""Configurazione dell'applicazione (per PC, non legata alla singola gara).

Il token di Telegram resta qui e NON viene mai salvato nel file gara, così si può
condividere un file gara senza svelare le credenziali del bot.
"""
from __future__ import annotations

import json
import os
import sys
from dataclasses import asdict, dataclass, field
from pathlib import Path


def app_dir() -> Path:
    if sys.platform.startswith("win"):
        base = Path(os.environ.get("APPDATA", Path.home()))
    else:
        base = Path(os.environ.get("XDG_CONFIG_HOME", Path.home() / ".config"))
    d = base / "PylonRace"
    d.mkdir(parents=True, exist_ok=True)
    return d


@dataclass
class AppConfig:
    port: str = ""
    baud: int = 9600
    dtr: bool = True
    telegram_token: str = ""
    telegram_chat_id: str = ""
    voice_id: str = ""
    voice_rate: int = 160
    last_project: str = ""
    output_dir: str = ""
    telegram_templates: dict = field(default_factory=dict)  # testi personalizzati dei messaggi
    ui_pt: float = 0.0   # dimensione del testo (0 = automatica)
    col_widths: dict = field(default_factory=dict)   # larghezze colonne scelte dall'utente (Piloti, Classifica)

    @staticmethod
    def path() -> Path:
        return app_dir() / "config.json"

    @classmethod
    def load(cls) -> "AppConfig":
        p = cls.path()
        for f in (p, p.with_suffix(".json.bak")):        # se config.json è rovinato si usa la copia
            try:
                d = json.loads(f.read_text(encoding="utf-8"))
                if not isinstance(d, dict):
                    continue
            except (OSError, ValueError):
                continue
            cfg = cls()
            for k, v in d.items():
                if hasattr(cfg, k):
                    setattr(cfg, k, v)
            return cfg
        return cls()

    def save(self) -> None:
        p = self.path()
        tmp = p.with_suffix(".json.tmp")
        try:
            tmp.write_text(json.dumps(asdict(self), indent=1), encoding="utf-8")
            if p.exists():
                try:
                    old = p.read_bytes()
                    json.loads(old.decode("utf-8"))
                    p.with_suffix(".json.bak").write_bytes(old)
                except (OSError, ValueError):
                    pass
            tmp.replace(p)
        except OSError:
            pass
