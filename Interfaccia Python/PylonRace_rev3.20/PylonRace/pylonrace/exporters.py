"""Esportazioni: classifica (PDF/XLSX) e ordine di volo (PDF/XLSX)."""
from __future__ import annotations
from .i18n import tr

from pathlib import Path
from typing import List

from .models import COLOR_NAMES, Event
from .standings import CategoryStandings

FONT_DIR = Path(__file__).parent / "data" / "fonts"

# colori come nel foglio Excel
CUT1_RGB = "FFFFCC"   # 1 taglio: giallo chiaro
CUT2_RGB = "99FFFF"   # 2+ tagli: azzurro
BEST_RGB = "92D050"   # miglior tempo della categoria
COLOR_RGB = ("F4B6B6", "B7E4B0", "B7CBF0")   # rosso, verde, blu (tenui)
HEADER_RGB = "D9D9D9"
CAT_RGB = "FFC000"


def _fmt(v, nd=3):
    return "" if v is None else f"{v:.{nd}f}".rstrip("0").rstrip(".") if isinstance(v, float) else str(v)


def _register_fonts():
    from reportlab.pdfbase import pdfmetrics
    from reportlab.pdfbase.ttfonts import TTFont

    names = pdfmetrics.getRegisteredFontNames()
    if "PR-Sans" not in names:
        regular, bold = FONT_DIR / "DejaVuSans.ttf", FONT_DIR / "DejaVuSans-Bold.ttf"
        if regular.exists():
            pdfmetrics.registerFont(TTFont("PR-Sans", str(regular)))
            pdfmetrics.registerFont(TTFont("PR-Sans-Bold", str(bold if bold.exists() else regular)))
            pdfmetrics.registerFontFamily("PR-Sans", normal="PR-Sans", bold="PR-Sans-Bold",
                                          italic="PR-Sans", boldItalic="PR-Sans-Bold")
        else:  # ripiego
            return "Helvetica", "Helvetica-Bold"
    return "PR-Sans", "PR-Sans-Bold"


# ----------------------------------------------------------------------------- classifica
def classifica_pdf(event: Event, results: List[CategoryStandings], path: Path) -> Path:
    from reportlab.lib import colors
    from reportlab.lib.pagesizes import A4, landscape
    from reportlab.lib.styles import ParagraphStyle
    from reportlab.lib.units import mm
    from reportlab.platypus import Paragraph, SimpleDocTemplate, Spacer, Table, TableStyle

    font, bold = _register_fonts()
    doc = SimpleDocTemplate(str(path), pagesize=landscape(A4), leftMargin=10 * mm, rightMargin=10 * mm,
                            topMargin=10 * mm, bottomMargin=10 * mm, title=event.settings.title)
    title = ParagraphStyle("t", fontName=bold, fontSize=14, leading=18, alignment=1, spaceAfter=2)
    cat_style = ParagraphStyle("c", fontName=bold, fontSize=12, leading=15, spaceBefore=6, spaceAfter=3)
    cell = ParagraphStyle("cell", fontName=font, fontSize=7.5, leading=9, alignment=1)
    cell_l = ParagraphStyle("cl", parent=cell, alignment=0)
    legend = ParagraphStyle("lg", fontName=font, fontSize=7, leading=9)

    story = []
    logo = FONT_DIR.parent / "logo.png"
    if logo.exists():
        from reportlab.platypus import Image as RLImage
        story.append(RLImage(str(logo), width=14 * mm, height=14 * mm))
    story += [Paragraph(event.settings.title, title),
             Paragraph("Legend: &nbsp;"
                       f'<font backColor="#{CUT1_RGB}">&nbsp;yellow = 1 cut&nbsp;</font> &nbsp; '
                       f'<font backColor="#{CUT2_RGB}">&nbsp;light blue = 2 cuts&nbsp;</font> &nbsp; '
                       f'<font backColor="#{BEST_RGB}">&nbsp;green = best time&nbsp;</font> &nbsp; '
                       "<strike>strikethrough</strike> = discarded manche", legend), Spacer(1, 4)]
    for cs in results:
        n = cs.n_manches
        head = ["POS", "NAT", "FAI", "NAME", "SURNAME"] + [f"M{i}" for i in range(1, n + 1)] + ["BEST", "TOT", "ABS"]
        data = [[Paragraph(f"<b>{h}</b>", cell) for h in head]]
        style = [
            ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#" + HEADER_RGB)),
            ("GRID", (0, 0), (-1, -1), 0.25, colors.grey),
            ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
            ("TOPPADDING", (0, 0), (-1, -1), 1.2),
            ("BOTTOMPADDING", (0, 0), (-1, -1), 1.2),
            ("LEFTPADDING", (0, 0), (-1, -1), 2),
            ("RIGHTPADDING", (0, 0), (-1, -1), 2),
        ]
        for r, row in enumerate(cs.rows, start=1):
            p = row.pilot
            cells = [
                Paragraph("" if row.place is None else str(row.place), cell),
                Paragraph(p.nation, cell),
                Paragraph(p.fai, cell),
                Paragraph(p.name, cell_l),
                Paragraph(p.surname, cell_l),
            ]
            for i in range(n):
                v = row.scores[i]
                txt = "" if v is None else _fmt(v)
                if v is not None and row.struck[i]:
                    txt = f"<strike>{txt}</strike>"
                cells.append(Paragraph(txt, cell))
                col = 5 + i
                c = row.cuts[i] or 0
                if v is not None and c == 1:
                    style.append(("BACKGROUND", (col, r), (col, r), colors.HexColor("#" + CUT1_RGB)))
                elif v is not None and c >= 2:
                    style.append(("BACKGROUND", (col, r), (col, r), colors.HexColor("#" + CUT2_RGB)))
            cells += [Paragraph("" if row.best is None else _fmt(row.best), cell),
                      Paragraph(_fmt(round(row.total, 3)) if row.total else "", cell),
                      Paragraph(str(row.abs_score) if row.total else "", cell)]
            if row.best_overall:
                style.append(("BACKGROUND", (5 + n, r), (5 + n, r), colors.HexColor("#" + BEST_RGB)))
            data.append(cells)
        label = f"{cs.category}   ({cs.discards} discard{'' if cs.discards == 1 else 's'})" if cs.discards else cs.category
        story.append(Paragraph(label, cat_style))
        fixed = [11 * mm, 12 * mm, 17 * mm, 22 * mm, 28 * mm]      # POS, NAT, FAI, NAME, SURNAME
        tail = [15 * mm, 16 * mm, 12 * mm]                        # BEST, TOT, ABS
        free = doc.width - sum(fixed) - sum(tail)                 # le colonne delle manche si adattano alla pagina
        widths = fixed + [min(13.5 * mm, free / max(n, 1))] * n + tail
        t = Table(data, colWidths=widths, repeatRows=1)
        t.setStyle(TableStyle(style))
        story.append(t)
        story.append(Spacer(1, 6))
    doc.build(story)
    return Path(path)


def classifica_xlsx(event: Event, results: List[CategoryStandings], path: Path) -> Path:
    from openpyxl import Workbook
    from openpyxl.styles import Alignment, Border, Font, PatternFill, Side

    wb = Workbook()
    ws = wb.active
    ws.title = "Ranking"
    thin = Side(style="thin", color="999999")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    ws["A1"] = event.settings.title
    ws["A1"].font = Font(bold=True, size=14)
    r = 3
    for cs in results:
        n = cs.n_manches
        ws.cell(r, 1, cs.category).font = Font(bold=True, size=12)
        ws.cell(r, 1).fill = PatternFill("solid", fgColor=CAT_RGB)
        for c in range(2, 6 + n + 3):
            ws.cell(r, c).fill = PatternFill("solid", fgColor=CAT_RGB)
        r += 1
        head = ["POS", "NAT", "FAI", "NAME", "SURNAME"] + [f"MANCHE {i}" for i in range(1, n + 1)] + ["BEST", "TOT", "ABS"]
        for c, h in enumerate(head, start=1):
            x = ws.cell(r, c, h)
            x.font = Font(bold=True)
            x.fill = PatternFill("solid", fgColor=HEADER_RGB)
            x.alignment = Alignment(horizontal="center")
            x.border = border
        r += 1
        for row in cs.rows:
            p = row.pilot
            vals = [row.place, p.nation, p.fai, p.name, p.surname] + row.scores + [row.best, row.total or None,
                                                                                  row.abs_score or None]
            for c, v in enumerate(vals, start=1):
                x = ws.cell(r, c, v)
                x.border = border
                x.alignment = Alignment(horizontal="center" if c != 4 and c != 5 else "left")
                if 6 <= c < 6 + n:
                    i = c - 6
                    if row.struck[i] and row.scores[i] is not None:
                        x.font = Font(strike=True)
                    cc = row.cuts[i] or 0
                    if row.scores[i] is not None and cc:
                        x.fill = PatternFill("solid", fgColor=CUT1_RGB if cc == 1 else CUT2_RGB)
                if c == 6 + n and row.best_overall:
                    x.fill = PatternFill("solid", fgColor=BEST_RGB)
            r += 1
        r += 1
    for col, w in zip("ABCDE", (7, 6, 10, 14, 18)):
        ws.column_dimensions[col].width = w
    wb.save(path)
    return Path(path)


# ----------------------------------------------------------------------------- ordine di volo
def _rounds_by_category(event: Event):
    """{categoria: {manche: [(round, [3 slot])...]}} nell'ordine del sorteggio."""
    out = {}
    for i in range(event.n_rounds):
        sl = event.round_slots(i)
        s0 = sl[0]
        out.setdefault(s0.category, {}).setdefault(s0.manche, []).append((s0.round, sl))
    return out


def ordine_volo_xlsx(event: Event, path: Path) -> Path:
    from openpyxl import Workbook
    from openpyxl.styles import Alignment, Border, Font, PatternFill, Side

    wb = Workbook()
    wb.remove(wb.active)
    med, thin = Side(style="medium"), Side(style="thin")
    center = Alignment(horizontal="center", vertical="center", wrap_text=True)

    def box(ws, rng, side):
        for row in ws[rng]:
            for c in row:
                c.border = Border(left=side, right=side, top=side, bottom=side)

    for cat, manches in _rounds_by_category(event).items():
        ws = wb.create_sheet(cat)
        ws.merge_cells("A1:S5")
        ws["A1"] = f"{event.settings.title}\n{cat}"
        ws["A1"].font = Font(bold=True, size=16)
        ws["A1"].alignment = center
        for k, (name, c0, c1) in enumerate(((tr("Rosso"), "B", "G"), (tr("Verde"), "H", "M"), (tr("Blu"), "N", "S"))):
            ws.merge_cells(f"{c0}6:{c1}7")
            ws[f"{c0}6"] = name
            ws[f"{c0}6"].alignment = center
            ws[f"{c0}6"].font = Font(bold=True, size=12)
            ws[f"{c0}6"].fill = PatternFill("solid", fgColor=COLOR_RGB[k])
            box(ws, f"{c0}6:{c1}7", thin)
        ws.merge_cells("A8:A9")
        ws["A8"] = tr("ROUND")
        ws["A8"].alignment = center
        ws["A8"].font = Font(bold=True)
        for c0, c1, txt in (("B", "D", tr("PILOTA")), ("E", "G", tr("MECCANICO")), ("H", "J", tr("PILOTA")),
                            ("K", "M", tr("MECCANICO")), ("N", "P", tr("PILOTA")), ("Q", "S", tr("MECCANICO"))):
            ws.merge_cells(f"{c0}8:{c1}9")
            ws[f"{c0}8"] = txt
            ws[f"{c0}8"].alignment = center
            ws[f"{c0}8"].font = Font(bold=True)
        box(ws, "A8:S9", thin)
        r = 10
        for m, rounds in manches.items():
            ws.merge_cells(f"A{r}:S{r + 1}")
            ws[f"A{r}"] = tr(f"Manche N:{m}")
            ws[f"A{r}"].font = Font(bold=True, size=12)
            ws[f"A{r}"].alignment = center
            box(ws, f"A{r}:S{r + 1}", med)
            r += 2
            for rnd, sl in rounds:
                ws.merge_cells(f"A{r}:A{r + 1}")
                ws[f"A{r}"] = rnd
                ws[f"A{r}"].alignment = center
                for k, s in enumerate(sl):
                    base = 2 + 6 * k
                    ws.merge_cells(start_row=r, start_column=base, end_row=r + 1, end_column=base + 2)
                    ws.merge_cells(start_row=r, start_column=base + 3, end_row=r + 1, end_column=base + 5)
                    ws.cell(r, base, f"{s.full_name}\n{s.pilot_id}").alignment = center
                    ws.cell(r, base + 3, s.mech).alignment = center
                box(ws, f"A{r}:S{r + 1}", thin)
                ws.row_dimensions[r].height = 18
                ws.row_dimensions[r + 1].height = 18
                r += 2
        from openpyxl.utils import get_column_letter

        for i in range(1, 20):
            ws.column_dimensions[get_column_letter(i)].width = 7.5
    if not wb.sheetnames:
        wb.create_sheet(tr("Ordine"))
    wb.save(path)
    return Path(path)


def ordine_volo_pdf(event: Event, path: Path) -> Path:
    from reportlab.lib import colors
    from reportlab.lib.pagesizes import A4, landscape
    from reportlab.lib.styles import ParagraphStyle
    from reportlab.lib.units import mm
    from reportlab.platypus import PageBreak, Paragraph, SimpleDocTemplate, Spacer, Table, TableStyle

    font, bold = _register_fonts()
    doc = SimpleDocTemplate(str(path), pagesize=landscape(A4), leftMargin=10 * mm, rightMargin=10 * mm,
                            topMargin=10 * mm, bottomMargin=10 * mm, title=tr("Ordine di volo"))
    h1 = ParagraphStyle("h1", fontName=bold, fontSize=14, leading=18, alignment=1)
    h2 = ParagraphStyle("h2", fontName=bold, fontSize=11, leading=14, spaceBefore=6, spaceAfter=2)
    cell = ParagraphStyle("c", fontName=font, fontSize=8, leading=10)
    story = []
    first = True
    for cat, manches in _rounds_by_category(event).items():
        if not first:
            story.append(PageBreak())
        first = False
        story += [Paragraph(f"{event.settings.title} — {cat}", h1), Spacer(1, 4)]
        for m, rounds in manches.items():
            story.append(Paragraph(tr(f"Manche {m}"), h2))
            data = [[tr("Round")] + [tr("{}: pilota / meccanico").replace("{}", tr(COLOR_NAMES[k])) for k in range(3)]]
            style = [("GRID", (0, 0), (-1, -1), 0.25, colors.grey),
                     ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#" + HEADER_RGB)),
                     ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
                     ("TOPPADDING", (0, 0), (-1, -1), 1), ("BOTTOMPADDING", (0, 0), (-1, -1), 1)]
            for k in range(3):
                style.append(("BACKGROUND", (1 + k, 0), (1 + k, 0), colors.HexColor("#" + COLOR_RGB[k])))
            for rnd, sl in rounds:
                data.append([str(rnd)] + [
                    Paragraph(f"<b>{s.full_name}</b> ({s.pilot_id})<br/><font size=7>{s.mech}</font>", cell) for s in sl
                ])
            t = Table(data, colWidths=[16 * mm] + [87 * mm] * 3, repeatRows=1)
            t.setStyle(TableStyle(style))
            story.append(t)
    if not story:
        story.append(Paragraph(tr("Nessun ordine di volo generato"), h1))
    doc.build(story)
    return Path(path)


def griglia_xlsx(path, title, head, rows, fills):
    """Esporta la griglia della scheda Gara così come è visualizzata (righe filtrate, colori inclusi)."""
    import openpyxl
    from openpyxl.styles import Alignment, Font, PatternFill
    from openpyxl.utils import get_column_letter

    wb = openpyxl.Workbook()
    ws = wb.active
    ws.title = tr("Griglia")
    ws["A1"] = title
    ws["A1"].font = Font(bold=True, size=14)
    for c, h in enumerate(head, 1):
        cell = ws.cell(3, c, h)
        cell.font = Font(bold=True, color="FFFFFF")
        cell.fill = PatternFill("solid", fgColor="1F3A8A")
        cell.alignment = Alignment(horizontal="center")
    for r, (row, fl) in enumerate(zip(rows, fills), 4):
        for c, (v, color) in enumerate(zip(row, fl), 1):
            if isinstance(v, str):
                try:
                    v = float(v.replace(",", ".")) if v.replace(",", ".").replace(".", "", 1).lstrip("-").isdigit() and c > 7 else v
                except ValueError:
                    pass
            cell = ws.cell(r, c, v if v != "" else None)
            cell.alignment = Alignment(horizontal="left" if c in (5, 7) else "center")
            if color:
                cell.fill = PatternFill("solid", fgColor=color.lstrip("#").upper())
    widths = [8, 9, 8, 5, 26, 7, 24] + [8] * (len(head) - 10) + [9, 7, 9]
    for i in range(1, len(head) + 1):
        ws.column_dimensions[get_column_letter(i)].width = widths[i - 1] if i - 1 < len(widths) else 9
    ws.freeze_panes = "A4"
    wb.save(path)
