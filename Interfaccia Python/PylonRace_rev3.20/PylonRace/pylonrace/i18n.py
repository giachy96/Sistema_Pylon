"""Lingua dell'interfaccia (italiano / inglese).

La lingua si sceglie in Impostazioni → Lingua e vale dal prossimo avvio: `set_language` va chiamata
prima di importare l'interfaccia, perché alcune etichette sono costanti di modulo.
I testi del programma sono scritti in italiano e passano da `tr()`; in inglese vengono cercati nel
dizionario (`i18n_en.EN`). I testi con parti variabili (f-string) sono registrati con `{}` al posto
di ogni parte variabile e vengono riconosciuti a runtime.
Non passano da qui: messaggi Telegram, annunci vocali, classifica (sempre in inglese).
"""
from __future__ import annotations

import re
from typing import Dict, List, Optional, Tuple

LANGUAGES = {"it": "Italiano", "en": "English"}
_lang = "it"
_patterns: Optional[List[Tuple[re.Pattern, str]]] = None


def set_language(lang: str) -> None:
    global _lang
    _lang = lang if lang in LANGUAGES else "it"


def language() -> str:
    return _lang


def _table() -> Dict[str, str]:
    from .i18n_en import EN
    return EN


def _compiled() -> List[Tuple[re.Pattern, str]]:
    global _patterns
    if _patterns is None:
        out = []
        for key, val in _table().items():
            if "{}" in key:
                parts = [re.escape(p) for p in key.split("{}")]
                out.append((re.compile("^" + "(.*?)".join(parts) + "$", re.S), val))
        # i modelli più lunghi (più specifici) prima
        out.sort(key=lambda kv: -len(kv[0].pattern))
        _patterns = out
    return _patterns


def tr(text: str) -> str:
    """Traduce un testo dell'interfaccia nella lingua scelta (in italiano lo restituisce com'è)."""
    if _lang == "it" or not isinstance(text, str) or not text:
        return text
    t = _table()
    if text in t:
        return t[text]
    for rx, val in _compiled():
        m = rx.match(text)
        if m:
            args = [t.get(g, g) for g in m.groups()]
            try:
                return val.format(*args)
            except (IndexError, KeyError, ValueError):
                return text
    return text


def trc(text: str, context: str) -> str:
    """Come tr, per parole con più significati: la chiave inglese è "contesto|testo"."""
    if _lang == "it":
        return text
    return _table().get(f"{context}|{text}", tr(text))


def ordinal(n: int) -> str:
    if _lang == "it":
        return f"{n}ª"
    return {1: "1st", 2: "2nd", 3: "3rd"}.get(n, f"{n}th")


def color_letter(i: int) -> str:
    return ("RVB" if _lang == "it" else "RGB")[i]
