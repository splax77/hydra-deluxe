"""Turn measure.json into whole-percent shares, fit rates, and three mock PNGs."""
import json
import math
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

D = Path(sys.argv[1])
M = json.load(open(D / "measure.json"))
R = json.load(open(D / "rows.json", encoding="utf-8"))

FONT_PX = 18
MAIN = ImageFont.truetype(str(D / "ShipporiAntiqueB1-Regular.ttf"), M["font"]["main_em"])
MONO = ImageFont.truetype(str(D / "CourierPrime-Regular.ttf"), M["font"]["mono_em"])
ASC = MAIN.getmetrics()[0]

# ImGui defaults the theme keeps: cell padding 4 px each side, scrollbar 14,
# window padding 8. Row height 22 = 18 + 2 * 2 (the 22 px row pitch).
PAD, SCROLL, WINPAD, ROW_H = 4, 14, 8, 22
BG, ALT, HEAD = (37, 37, 37), (43, 43, 43), (47, 46, 49)
WHITE, DIM, GOLD, GREY = (255, 255, 255), (145, 145, 145), (250, 210, 0), (145, 145, 145)
CAP_BG, CAP_FG = (20, 20, 20), (200, 200, 200)


def whole_percents(values):
    """Largest-remainder rounding to whole percents that add up to 100."""
    total = sum(values)
    raw = [100.0 * v / total for v in values]
    floor = [math.floor(x) for x in raw]
    left = 100 - sum(floor)
    for i in sorted(range(len(raw)), key=lambda i: raw[i] - floor[i], reverse=True)[:left]:
        floor[i] += 1
    return floor


def split(shares, avail, header_w):
    """The design: each shown text column gets its share of the room, never
    below its header."""
    total = sum(shares)
    return [max(h, math.floor(avail * s / total)) for s, h in zip(shares, header_w)]


def fit_rate(widths_px, col_w):
    inner = col_w  # the column width is the text room; padding is outside it
    return sum(w <= inner for w in widths_px) / len(widths_px)


def ellipsize(text, w, font):
    if font.getlength(text) <= w:
        return text
    dots = "…"
    lo, hi = 0, len(text)
    while lo < hi:
        mid = (lo + hi + 1) // 2
        if font.getlength(text[:mid].rstrip() + dots) <= w:
            lo = mid
        else:
            hi = mid - 1
    return text[:lo].rstrip() + dots


def draw_table(img, x0, y0, cols, rows, sort_col=None, sort_desc=False):
    """cols: dicts with header, width, get(row)->(text,color), font, right."""
    d = ImageDraw.Draw(img)
    total_w = sum(c["width"] + 2 * PAD for c in cols)
    d.rectangle([x0, y0, x0 + total_w, y0 + ROW_H - 1], fill=HEAD)
    x = x0
    for i, c in enumerate(cols):
        d.text((x + PAD, y0 + 2), c["header"], font=MAIN, fill=WHITE)
        if i == sort_col:
            hx = x + PAD + MAIN.getlength(c["header"]) + 6
            cy = y0 + ROW_H // 2
            tri = [(hx, cy + 3), (hx + 8, cy + 3), (hx + 4, cy - 3)] if not sort_desc else \
                  [(hx, cy - 3), (hx + 8, cy - 3), (hx + 4, cy + 3)]
            d.polygon(tri, fill=WHITE)
        x += c["width"] + 2 * PAD
    y = y0 + ROW_H
    for k, r in enumerate(rows):
        if k % 2:
            d.rectangle([x0, y, x0 + total_w, y + ROW_H - 1], fill=ALT)
        x = x0
        for c in cols:
            text, color = c["get"](r)
            shown = ellipsize(text, c["width"], c["font"])
            tx = x + PAD
            if c.get("right"):
                tx = x + PAD + c["width"] - c["font"].getlength(shown)
            if c.get("chip") and text:
                tw = c["font"].getlength(shown)
                d.rounded_rectangle([tx - 1, y + 2, tx + tw + 11, y + ROW_H - 3], radius=9,
                                    outline=color)
                tx += 5
            d.text((tx, y + 2), shown, font=c["font"], fill=color)
            x += c["width"] + 2 * PAD
        y += ROW_H
    return total_w


def caption(img, lines):
    d = ImageDraw.Draw(img)
    h = 8 + 24 * len(lines)
    d.rectangle([0, 0, img.width, h], fill=CAP_BG)
    for i, line in enumerate(lines):
        d.text((10, 6 + 24 * i), line, font=MAIN, fill=CAP_FG)
    return h + 6


# ---------------------------------------------------------------- Library
lib = M["lib"]
lib_keys = [c["key"] for c in lib]
lib_shares = whole_percents([c["usual"] for c in lib])
lib_rows = R["lib"]
order = sorted(range(len(lib_rows)), key=lambda i: lib_rows[i]["title"].casefold())
start = next(k for k, i in enumerate(order) if lib_rows[i]["title"][:1].upper() == "A")
SHOW = 42
show = [lib_rows[i] for i in order[start:start + SHOW]]


def lib_get(key):
    def get(r):
        if key == "best":
            return r["best"], GOLD if r["status"] == "ready" else GREY
        return r[key], WHITE
    return get


def library_layout(shown_keys, table_w):
    idx = [lib_keys.index(k) for k in shown_keys]
    avail = table_w - SCROLL - 2 * PAD * len(idx)
    widths = split([lib_shares[i] for i in idx], avail, [lib[i]["header_w"] for i in idx])
    fits = [fit_rate(R["lib_w"][lib_keys[i]], w) for i, w in zip(idx, widths)]
    return idx, avail, widths, fits


results = {}
WIN_W = 2560
for name, keys, table_w in [("closed", lib_keys, WIN_W - 2 * WINPAD),
                            ("open", ["title", "artist", "best"], 1016)]:
    idx, avail, widths, fits = library_layout(keys, table_w)
    results[name] = dict(avail=avail, cols=[dict(header=lib[i]["header"],
                                                 share=lib_shares[i],
                                                 width=w, fit=f)
                                            for i, w, f in zip(idx, widths, fits)])
    cols = [dict(header=lib[i]["header"], width=w, get=lib_get(lib_keys[i]),
                 font=MONO if lib[i]["mono"] else MAIN) for i, w in zip(idx, widths)]
    shown_total = sum(c["share"] for c in results[name]["cols"])
    for c in results[name]["cols"]:
        c["of_shown"] = round(100 * c["share"] / shown_total)
    if name == "closed":
        img_w, lines = WIN_W, [
            f"Library, song panel closed. Your library sorted by title, rows {start + 1:,}-{start + SHOW:,} of {len(lib_rows):,}.",
            "   ".join(f"{c['header']} {c['share']}% = {c['width']} px, fits {c['fit']:.1%} of rows"
                       for c in results[name]["cols"])]
    else:
        img_w, lines = 1024, [
            "Library, song panel open: the 1,016 px Library pane.",
            f"Rows {start + 1:,}-{start + SHOW:,} of {len(lib_rows):,}. Charter and Folder hidden;",
            "the other three re-split the pane in the same proportions:"] + [
            f"   {c['header']}: {c['share']}% of 100 -> {c['of_shown']}% of the pane = {c['width']} px, fits {c['fit']:.1%} of rows"
            for c in results[name]["cols"]]
    h = 8 + 24 * len(lines) + 6 + ROW_H * (SHOW + 1) + 12
    img = Image.new("RGB", (img_w, h), BG)
    top = caption(img, lines)
    draw_table(img, WINPAD, top, cols, show, sort_col=0)
    img.save(D / f"mock_library_{name}.png")

# ---------------------------------------------------------- Path report
rep = M["rep"]
rep_keys = [c["key"] for c in rep]
rep_shares = whole_percents([c["usual"] for c in rep])
num = M["rep_num"]
NUM_ORDER = ["Score", "Acts", "Max skip", "Hardest ms", "Timing", "Early fill (ms)",
             "Avg multiplier", "SqIn", "SqOut", "Notes"]
n_cols = 1 + len(rep) + len(NUM_ORDER)
fixed = num["#"] + sum(num[h] for h in NUM_ORDER)
avail = WIN_W - 2 * WINPAD - SCROLL - 2 * PAD * n_cols - fixed
rep_widths = split(rep_shares, avail, [c["header_w"] for c in rep])
rep_fits = [fit_rate(R["rep_w"][k], w) for k, w in zip(rep_keys, rep_widths)]
results["report"] = dict(avail=avail, fixed=fixed,
                         cols=[dict(header=c["header"], share=s, width=w, fit=f)
                               for c, s, w, f in zip(rep, rep_shares, rep_widths, rep_fits)],
                         numbers={h: num[h] for h in ["#"] + NUM_ORDER})

TIER_COL = {"Normal": (90, 200, 120), "Hard": (230, 200, 60), "Extreme": (240, 140, 50),
            "Insane": (230, 70, 70), "Insane+": (200, 70, 200), "Beyond 170 ms": (160, 90, 220),
            "No squeezes": DIM}
cols = [dict(header="#", width=num["#"], get=lambda r: (r["#"], DIM), font=MAIN, right=True)]
for c, w in zip(rep, rep_widths):
    k = c["key"]
    color = WHITE if k in ("song", "path") else DIM
    cols.append(dict(header=c["header"], width=w, font=MONO if c["mono"] else MAIN,
                     get=(lambda k, color: lambda r: (r[k], color))(k, color)))
for h in NUM_ORDER:
    if h == "Timing":
        cols.append(dict(header=h, width=num[h], font=MAIN, chip=True,
                         get=lambda r: (r["Timing"], TIER_COL[r["Timing"]])))
    else:
        cols.append(dict(header=h, width=num[h], font=MAIN, right=True,
                         get=(lambda h: lambda r: (r[h] if r[h] else "—", WHITE))(h)))
rows = R["rep"][:SHOW]
h = 60 + ROW_H * (SHOW + 1) + 20
img = Image.new("RGB", (WIN_W, h), BG)
top = caption(img, [
    f"Path report, best paths, sorted by score (highest first). Rows 1-{SHOW} of {len(R['rep']):,}. "
    f"Number columns at their widest value ({fixed} px together). Early fill shows dashes: its values aren't in the stored summaries.",
    "   ".join(f"{c['header']} {c['share']}% = {c['width']} px, fits {c['fit']:.1%}"
               for c in results["report"]["cols"]),
])
draw_table(img, WINPAD, top, cols, rows, sort_col=6, sort_desc=True)
img.save(D / "mock_path_report.png")

json.dump(results, open(D / "results.json", "w"), indent=1)
print(json.dumps(results, indent=1))
