"""Lingua dell'interfaccia: ogni testo passato a tr() deve avere la traduzione inglese."""
import ast
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def _keys():
    keys = set()
    for fn in (ROOT / "pylonrace").rglob("*.py"):
        tree = ast.parse(fn.read_text(encoding="utf-8"))
        for n in ast.walk(tree):
            if isinstance(n, ast.Call) and isinstance(n.func, ast.Name) and n.func.id in ("tr", "trc") and n.args:
                a = n.args[0]
                if isinstance(a, ast.Constant) and isinstance(a.value, str):
                    k = a.value
                elif isinstance(a, ast.JoinedStr):
                    k = "".join(v.value if isinstance(v, ast.Constant) else "{}" for v in a.values)
                else:
                    continue
                if n.func.id == "trc":
                    k = f"{n.args[1].value}|{k}"
                keys.add((k, f"{fn.name}:{n.lineno}"))
    return keys


def test_every_text_has_english_translation():
    from pylonrace.i18n_en import EN
    missing = sorted({(k, where) for k, where in _keys() if k not in EN})
    assert not missing, missing


def test_placeholders_match():
    from pylonrace.i18n_en import EN
    for k, v in EN.items():
        if "{gara}" in k:
            continue
        assert k.count("{}") == len(re.findall(r"\{\d*\}", v)), k


def test_tr_italian_is_identity_and_english_patterns():
    from pylonrace import i18n
    i18n.set_language("it")
    assert i18n.tr("Connetti") == "Connetti"
    i18n.set_language("en")
    try:
        assert i18n.tr("Connetti") == "Connect"
        assert i18n.tr("F3D · Manche 4 · Round 5") == "F3D · Manche 4 · Round 5"
        assert i18n.tr("Avanzamento: 3 / 9 round") == "Progress: 3 / 9 rounds"
        assert i18n.tr("Eliminare Mario Rossi (Q500)?") == "Delete Mario Rossi (Q500)?"
        assert i18n.tr("testo sconosciuto") == "testo sconosciuto"
        assert i18n.color_letter(1) == "G" and i18n.ordinal(2) == "2nd"
    finally:
        i18n.set_language("it")


def test_english_window_starts(tmp_path):
    """Avvio completo in inglese in un processo separato (le etichette di modulo dipendono dalla lingua)."""
    code = f"""
import os, sys
os.environ["QT_QPA_PLATFORM"] = "offscreen"; os.environ["XDG_CONFIG_HOME"] = r"{tmp_path}"
sys.path.insert(0, r"{ROOT}")
from pylonrace.i18n import set_language; set_language("en")
from pathlib import Path
from PySide6.QtWidgets import QApplication
app = QApplication([])
from pylonrace.config import AppConfig
from pylonrace.ui.main_window import MainWindow
w = MainWindow(AppConfig(), Path(r"{tmp_path}") / "g.json")
tabs = [w.tabs.tabText(i) for i in range(w.tabs.count())]
assert tabs == ["Race", "Pilots", "Draw", "Ranking", "Settings"], tabs
assert w.btn_conn.text() == "Connect"
print("OK")
"""
    r = subprocess.run([sys.executable, "-c", code], capture_output=True, text=True, timeout=60)
    assert "OK" in r.stdout, r.stderr[-2000:]


def test_language_setting_saved(tmp_path, monkeypatch):
    monkeypatch.setenv("XDG_CONFIG_HOME", str(tmp_path))
    from pylonrace.config import AppConfig
    c = AppConfig()
    assert c.language == "it"
    c.language = "en"
    c.save()
    assert AppConfig.load().language == "en"
