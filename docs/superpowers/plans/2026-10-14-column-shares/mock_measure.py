"""Measure the user's real library in Hydra's fonts and derive column shares.

Reads a COPY of the installed hydra.db (C:\\Program Files\\Hydra). Writes
measure.json beside the db copy. No analysis runs; it only reads stored rows.
"""
import json
import math
import re
import sqlite3
import sys
from pathlib import Path

from PIL import ImageFont

D = Path(sys.argv[1])
FONT_PX = 18.0  # app_shell.cpp kFontSize, UI scale 1.0
USUAL = 0.99    # column_widths.cpp kUsualShare

# Current view settings (hydra_settings.ini): Expert, Pro Drums, 2x Bass, no
# Note Shuffle, sp_cap 4, ms limit on at 10, depth mode 0 value 4, no 1.0 fills.
MODE = "Expert Pro Drums, 2x Bass"
CUR = dict(sp_cap=4, ms_enabled=1, ms_value=10, depth_mode=0, depth_value=4, legacy_fills=0)


def imgui_font(path):
    # ImGui sizes a font so ascent - descent equals the pixel size
    # (stbtt_ScaleForPixelHeight). Find the em size that gives that in PIL.
    ref = ImageFont.truetype(str(path), 1000)
    asc, desc = ref.getmetrics()
    em = FONT_PX * 1000.0 / (asc + desc)
    return ImageFont.truetype(str(path), em), em


MAIN, MAIN_EM = imgui_font(D / "ShipporiAntiqueB1-Regular.ttf")
MONO, MONO_EM = imgui_font(D / "CourierPrime-Regular.ttf")

_cache = {}


def width(text, mono=False):
    key = (text, mono)
    w = _cache.get(key)
    if w is None:
        w = math.ceil((MONO if mono else MAIN).getlength(text)) if text else 0
        _cache[key] = w
    return w


RICH = re.compile(r"</?(color|size|b|i|u|s|sub|sup)(=[^<>]*)?>", re.I)


def clean(s):
    return RICH.sub("", s or "").strip()


def title(s):
    t = clean(s)
    return "(unknown)" if not t or t == "<unknown title>" else t


def group(n):
    return f"{n:,}"


def usual(ws):
    if not ws:
        return 0
    s = sorted(ws)
    rank = max(1, math.ceil(USUAL * len(s)))
    return s[rank - 1]


con = sqlite3.connect(D / "hydra.db")
con.text_factory = lambda b: b.decode("utf-8", "replace")
charts = con.execute("select md5,name,artist,charter,folder from charts").fetchall()
res = {}
q = ("select hyhash,hyversion,bestpath,score,actcount,maxskip,hardest_ms,avgmult,notecount,"
     "sqin_count,sqout_count from results where chartmode=? and sp_cap=? and ms_enabled=? and "
     "ms_value=? and depth_mode=? and depth_value=? and legacy_fills=?")
for r in con.execute(q, (MODE, *CUR.values())):
    prev = res.get(r[0])
    if prev is None or r[1] > prev[1]:
        res[r[0]] = r
newest = max(r[1] for r in res.values())

lib = []
for md5, name, artist, charter, folder in charts:
    r = res.get(md5)
    if r is None:
        best, status = "Not analyzed", "new"
    elif r[1] != newest:
        best, status = "Stale", "stale"
    else:
        best = (group(r[3]) + "  " + r[2]) if r[3] is not None else r[2]
        status = "ready"
    lib.append(dict(title=title(name), artist=title(artist), charter=clean(charter),
                    folder=folder or "", best=best, status=status))

LIB_COLS = [("Title", "title", False), ("Artist", "artist", False), ("Charter", "charter", False),
            ("Folder", "folder", False), ("Best path", "best", True)]


def column_stats(rows, cols):
    out = []
    for header, key, mono in cols:
        ws = [width(r[key], mono) for r in rows]
        out.append(dict(header=header, key=key, mono=mono, header_w=width(header),
                        usual=max(usual(ws), width(header)), widest=max(ws + [width(header)]),
                        widths=ws))
    return out


lib_stats = column_stats(lib, LIB_COLS)

# Path report rows: one per chart's stored best path in the current mode
# (the report's best-only view). Early fill isn't in the results table; its
# header "Early fill (ms)" is wider than any "-123.4 ms" value, so the header
# sets that column's width.
W = 85.0  # hit_window_ms


def tier(ms):
    if ms is None:
        return "No squeezes"
    for name, cut in [("Normal", 2.0), ("Hard", W / 2), ("Extreme", W), ("Insane", 1.5 * W),
                      ("Insane+", 2 * W)]:
        if ms <= cut:
            return name
    return "Beyond 170 ms"


rep = []
by_md5 = {c[0]: c for c in charts}
for md5, r in res.items():
    if r[1] != newest or r[3] is None:
        continue
    c = by_md5.get(md5)
    if c is None:
        continue
    rep.append(dict(song=title(c[1]), artist=title(c[2]), charter=clean(c[3]), mode=MODE,
                    path=r[2], score=r[3], acts=r[4], skip=r[5], ms=r[6], mult=r[7],
                    notes=r[8], sqin=r[9], sqout=r[10]))
rep.sort(key=lambda x: -x["score"])
for i, x in enumerate(rep):
    x["#"] = group(i + 1)
    x["Score"] = group(x["score"])
    x["Acts"] = str(x["acts"])
    x["Max skip"] = str(x["skip"])
    x["Hardest ms"] = f"{x['ms']:.1f} ms" if x["ms"] is not None else "—"
    x["Timing"] = tier(x["ms"])
    x["Early fill (ms)"] = ""
    x["Avg multiplier"] = f"{x['mult']:.3f}"
    x["SqIn"] = str(x["sqin"])
    x["SqOut"] = str(x["sqout"])
    x["Notes"] = group(x["notes"])

REP_TEXT = [("Song", "song", False), ("Artist", "artist", False), ("Charter", "charter", False),
            ("Mode", "mode", False), ("Path", "path", True)]
REP_NUM = ["Score", "Acts", "Max skip", "Hardest ms", "Timing", "Early fill (ms)",
           "Avg multiplier", "SqIn", "SqOut", "Notes"]
rep_text = column_stats(rep, REP_TEXT)
CHIP_PAD = 12  # chip outline padding, both sides
num_w = {"#": max(width("#"), max(width(x["#"]) for x in rep))}
for h in REP_NUM:
    pad = CHIP_PAD if h == "Timing" else 0
    num_w[h] = max(width(h), max(width(x[h]) + pad for x in rep))

out = dict(
    font=dict(main_em=MAIN_EM, mono_em=MONO_EM, live_forever=width("Live Forever")),
    library_rows=len(lib), ready=sum(r["status"] == "ready" for r in lib),
    new=sum(r["status"] == "new" for r in lib), stale=sum(r["status"] == "stale" for r in lib),
    report_rows=len(rep), newest_version=newest,
    lib=[{k: v for k, v in s.items() if k != "widths"} for s in lib_stats],
    rep=[{k: v for k, v in s.items() if k != "widths"} for s in rep_text],
    rep_num=num_w,
)
json.dump(out, open(D / "measure.json", "w"), indent=1)
json.dump(dict(lib=lib, rep=rep, lib_w={s["key"]: s["widths"] for s in lib_stats},
               rep_w={s["key"]: s["widths"] for s in rep_text}),
          open(D / "rows.json", "w", encoding="utf-8"))
print(json.dumps(out, indent=1))
