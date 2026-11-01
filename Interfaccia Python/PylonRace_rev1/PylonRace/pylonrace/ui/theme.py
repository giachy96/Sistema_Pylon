"""Stile grafico dell'applicazione."""
from __future__ import annotations

from PySide6.QtCore import Qt
from PySide6.QtGui import QColor, QFont, QPalette
from PySide6.QtWidgets import QApplication

# colori delle tre piazzole: (sfondo tenue, bordo/accento, sfondo forte per riga corrente)
COLOR_BG = ("#FBE4E4", "#FADBD8", "#F5B7B1")
COLOR_ACCENT = ("#D64541", "#2E9E48", "#2F6FD1")
COLOR_STRONG = ("#F5B7B1", "#A9DFBF", "#AED6F1")
COLOR_SOFT = ("#FDF2F2", "#F1FAF3", "#F1F6FD")
REFUEL_BG = "#FFF3C4"
CUT1_BG = "#FFFFCC"
CUT2_BG = "#99FFFF"
BEST_BG = "#92D050"
OK = "#2E9E48"
WARN = "#C77700"
ERR = "#C0392B"
MUTED = "#6B7280"

QSS = """
* { font-family: "Segoe UI", "Noto Sans", "DejaVu Sans", sans-serif; font-size: 10pt; }
QMainWindow, QWidget#root { background: #C9CED6; color: #111827; }
QMenuBar { background: #B9BFC9; color: #111827; }
QMenuBar::item { padding: 4px 10px; background: transparent; color: #111827; }
QMenuBar::item:selected { background: #9FA7B4; }
QMenu { background: #F4F5F7; color: #111827; border: 1px solid #9CA3AF; }
QMenu::item:selected { background: #CFE0FB; color: #111827; }
QLabel, QCheckBox, QRadioButton { color: #111827; }
QCheckBox:disabled, QLabel:disabled { color: #6B7280; }
QTabWidget::pane { border: 1px solid #D1D5DB; border-radius: 6px; background: #E9ECF0; top: -1px; }
QTabBar::tab { padding: 9px 22px; margin-right: 2px; background: #B3BAC5; border: 1px solid #9CA3AF;
               border-bottom: none; border-top-left-radius: 6px; border-top-right-radius: 6px; color: #374151; }
QTabBar::tab:selected { background: #E9ECF0; color: #111827; font-weight: 600; }
QPushButton { min-height: 22px; padding: 7px 14px; border: 1px solid #9CA3AF; border-radius: 6px; background: #F8F9FA; color: #111827; }
QPushButton:hover { background: #E8EBEF; }
QPushButton:pressed { background: #E3E6EA; }
QPushButton:disabled { color: #6B7280; background: #D3D7DE; }
QPushButton#primary { background: #1F6FEB; color: white; border-color: #1A5FCC; font-weight: 600; }
QPushButton#primary:hover { background: #1A5FCC; }
QPushButton#primary:disabled { background: #A9C4F2; border-color: #A9C4F2; color: white; }
QPushButton#big { font-size: 15pt; font-weight: 700; padding: 16px 10px; }
QPushButton#nav { font-size: 14pt; font-weight: 700; padding: 18px 10px; }
QPushButton#cmdShow { background: #FFF3C4; border-color: #E0B000; font-weight: 600; }
QPushButton#cmdStart { background: #D6F5DD; border-color: #2E9E48; font-weight: 600; }
QPushButton#cmdStop { background: #FAD7D4; border-color: #D64541; font-weight: 600; }
QPushButton#danger { color: #B42318; }
QLineEdit, QComboBox { min-height: 24px; padding: 5px 8px; border: 1px solid #9CA3AF; border-radius: 5px; background: white; color: #111827; }
QComboBox QAbstractItemView { background: white; color: #111827; selection-background-color: #CFE0FB; selection-color: #111827; }
QLineEdit:focus, QComboBox:focus { border-color: #1F6FEB; }
QSpinBox, QDoubleSpinBox { padding: 4px 4px; min-height: 22px; max-width: 220px; background: white; color: #111827; }
QGroupBox { border: 1px solid #A5ACB8; border-radius: 8px; margin-top: 12px; padding: 10px 8px 8px 8px; background: #F4F5F7; }
QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; color: #374151; font-weight: 600; }
QTableView, QTableWidget { gridline-color: #E5E7EB; border: 1px solid #D1D5DB; border-radius: 4px;
                           selection-background-color: #CFE0FB; selection-color: #111827; alternate-background-color: #F3F4F6; color: #111827; }
QHeaderView::section { background: #D5D9E0; padding: 5px 6px; border: none; border-right: 1px solid #D9DCE1;
                       border-bottom: 1px solid #D1D5DB; font-weight: 600; color: #374151; }
QPlainTextEdit#log { background: #111827; color: #E5E7EB; border-radius: 6px; font-family: "Consolas", "DejaVu Sans Mono", monospace; font-size: 9pt; }
QFrame#card { border-radius: 10px; border: 2px solid #D1D5DB; background: white; }
QCheckBox { min-height: 22px; }
QScrollArea, QWidget#scrollInner { background: transparent; border: none; }
QLabel#h1 { font-size: 15pt; font-weight: 700; color: #111827; }
QLabel#muted { color: #6B7280; }
QStatusBar { background: #B9BFC9; color: #111827; }
QToolTip { padding: 4px; }
"""


def apply(app: QApplication) -> None:
    app.setStyle("Fusion")
    try:  # ignora il tema scuro di Windows: i colori sono definiti qui
        app.styleHints().setColorScheme(Qt.ColorScheme.Light)
    except Exception:
        pass
    pal = QPalette()
    pal.setColor(QPalette.Window, QColor("#C9CED6"))
    pal.setColor(QPalette.Base, QColor("#FFFFFF"))
    pal.setColor(QPalette.AlternateBase, QColor("#FAFAFB"))
    pal.setColor(QPalette.Text, QColor("#111827"))
    pal.setColor(QPalette.WindowText, QColor("#111827"))
    pal.setColor(QPalette.Button, QColor("#F8F9FA"))
    pal.setColor(QPalette.ButtonText, QColor("#111827"))
    pal.setColor(QPalette.ToolTipBase, QColor("#FFFFE1"))
    pal.setColor(QPalette.ToolTipText, QColor("#111827"))
    pal.setColor(QPalette.PlaceholderText, QColor("#6B7280"))
    pal.setColor(QPalette.HighlightedText, QColor("#FFFFFF"))
    for role in (QPalette.WindowText, QPalette.Text, QPalette.ButtonText):
        pal.setColor(QPalette.Disabled, role, QColor("#6B7280"))
    pal.setColor(QPalette.Highlight, QColor("#1F6FEB"))
    app.setPalette(pal)
    f = QFont(app.font())
    f.setPointSize(10)
    app.setFont(f)
    app.setStyleSheet(QSS)
