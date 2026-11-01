"""Configurazione dell'applicazione (per PC, non legata alla singola gara).

Il token di Telegram resta qui e NON viene mai salvato nel file gara, così si può
condividere un file gara senza svelare le credenziali del bot.
"""
from __future__ import annotations

import json
import os
import sys
from dataclasses import asdict, dataclass
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

    @staticmethod
    def path() -> Path:
        return app_dir() / "config.json"

    @classmethod
    def load(cls) -> "AppConfig":
        try:
            d = json.loads(cls.path().read_text(encoding="utf-8"))
            cfg = cls()
            for k, v in d.items():
                if hasattr(cfg, k):
                    setattr(cfg, k, v)
            return cfg
        except (OSError, ValueError):
            return cls()

    def save(self) -> None:
        try:
            self.path().write_text(json.dumps(asdict(self), indent=1), encoding="utf-8")
        except OSError:
            pass
