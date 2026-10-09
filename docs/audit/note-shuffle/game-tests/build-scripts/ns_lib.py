"""Shared helpers for the Note Shuffle game-test charts: run both references, write songs."""
import json, os, struct, subprocess, sys, tempfile

REPO = "C:/Users/Patrick/Downloads/Hydra/hydra-test"
REF = {
    "flag": REPO + "/docs/audit/note-shuffle/scout-flag/shuffle_ref.py",
    "strings": REPO + "/docs/audit/note-shuffle/scout-strings/shuffle_ref.py",
}
HERE = os.path.dirname(os.path.abspath(__file__))
FFMPEG = "C:/ytdl/ffmpeg.exe"

K, R, YT, BT, GT, YC, BC, GC = 1, 2, 4, 8, 16, 32, 64, 128
NAME = {K: "kick", R: "red snare", YT: "yellow tom", BT: "blue tom", GT: "green tom",
        YC: "yellow cymbal", BC: "blue cymbal", GC: "green cymbal", 0: "NOTHING"}
COLOUR = {K: 0, R: 1, YT: 2, YC: 2, BT: 3, BC: 3, GT: 4, GC: 4}
FOLD = {YC: YT, BC: BT, GC: GT}


def order(chord, how):
    """chord: lane bits in file order. how: file, lane (K,R,Y,B,G), bit (by lane bit), reverse."""
    if how == "file":
        return list(chord)
    if how == "lane":
        return sorted(chord, key=lambda m: (COLOUR[m], m))
    if how == "bit":
        return sorted(chord)
    if how == "reverse":
        return sorted(chord, key=lambda m: (COLOUR[m], m), reverse=True)
    raise ValueError(how)


def run_one(kind, notes, instrument, timeout=8):
    with tempfile.NamedTemporaryFile("w", suffix=".json", delete=False, dir=HERE) as f:
        json.dump({"instrument": instrument, "notes": notes}, f)
        p = f.name
    try:
        r = subprocess.run([sys.executable, "-I", os.path.join(HERE, "run_refs.py"), kind, REF[kind], p],
                           capture_output=True, text=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        return "HANG"
    finally:
        os.unlink(p)
    if r.returncode != 0:
        raise RuntimeError(r.stderr)
    return [tuple(x) for x in json.loads(r.stdout)]


def predict(chords, *, how="lane", num=1, den=1, pro=True):
    """chords: [(tick, [lane bits in file order])]. Ticks are scaled by num/den before the
    shuffle. Returns {"flag": res, "strings": res}; res is "HANG" or [(orig tick, [out masks
    in game order])]."""
    notes, back = [], {}
    for t, ch in chords:
        st = t * num // den
        assert st * den == t * num, "scale must be exact"
        back[st] = t
        for m in order(ch, how):
            notes.append([st, FOLD.get(m, m) if not pro else m])
    out = {}
    for kind in REF:
        res = run_one(kind, notes, 9 if pro else 6)
        if res == "HANG":
            out[kind] = "HANG"
            continue
        per = {}
        for t, m in res:
            per.setdefault(back[t], []).append(m)
        out[kind] = [(t, per[t]) for t, _ in chords]
    if out["flag"] != out["strings"]:
        raise RuntimeError(f"the two references disagree: {out}")
    return out["flag"]


def pads_text(masks):
    pads = sorted({m for m in masks if m != K}, key=lambda m: (COLOUR[m], m))
    kick = K in masks
    words = [NAME[m] for m in pads]
    if kick:
        words = ["kick"] + words
    return " + ".join(words) if words else "nothing"


def sets(res, n=None):
    if res == "HANG":
        return "HANG"
    rows = res if n is None else res[:n]
    return [(t, frozenset(ms)) for t, ms in rows]


# ---------- writers ----------

def chart_text(name, res, chords, bpm=120):
    lines = ["[Song]", "{", f'  Name = "{name}"', '  Artist = "Hydra note shuffle test"',
             '  Charter = "Hydra"', "  Offset = 0", f"  Resolution = {res}", '  Genre = "Test"',
             '  MediaType = "cd"', '  MusicStream = "song.ogg"', "}",
             "[SyncTrack]", "{", "  0 = TS 4", f"  0 = B {bpm * 1000}", "}",
             "[Events]", "{", "}", "[ExpertDrums]", "{"]
    code = {K: 0, R: 1, YT: 2, YC: 2, BT: 3, BC: 3, GT: 4, GC: 4}
    marker = {YC: 66, BC: 67, GC: 68}
    for t, ch in chords:
        for m in ch:
            lines.append(f"  {t} = N {code[m]} 0")
        for m in ch:
            if m in marker:
                lines.append(f"  {t} = N {marker[m]} 0")
    lines.append("}")
    return "\n".join(lines) + "\n"


def _vlq(n):
    out = [n & 0x7F]
    n >>= 7
    while n:
        out.append((n & 0x7F) | 0x80)
        n >>= 7
    return bytes(reversed(out))


def _track(events):
    """events: [(tick, seq, bytes)] -> MTrk chunk; sorted by (tick, seq)."""
    events = sorted(events, key=lambda e: (e[0], e[1]))
    body, last = b"", 0
    for t, _, data in events:
        body += _vlq(t - last) + data
        last = t
    body += _vlq(0) + b"\xff\x2f\x00"
    return b"MTrk" + struct.pack(">I", len(body)) + body


def mid_bytes(name, res, chords, bpm=120):
    """Type-1 .mid: tempo track, then PART DRUMS. Within a tick, note-ons are written in
    the chord's list order (file order). Y/B/G are cymbals unless a tom marker covers them."""
    t0 = [(0, 0, b"\xff\x03" + _vlq(len(name)) + name.encode()),
          (0, 1, b"\xff\x51\x03" + struct.pack(">I", 60000000 // bpm)[1:]),
          (0, 2, b"\xff\x58\x04\x04\x02\x18\x08")]
    pitch = {K: 96, R: 97, YT: 98, YC: 98, BT: 99, BC: 99, GT: 100, GC: 100}
    tom = {YT: 110, BT: 111, GT: 112}
    tn = "PART DRUMS"
    ev = [(0, 0, b"\xff\x03" + _vlq(len(tn)) + tn.encode())]
    off = max(1, res // 16)
    seq = 1
    for t, ch in chords:
        for m in ch:  # tom markers first, so they cover the note
            if m in tom:
                ev.append((t, seq, bytes([0x90, tom[m], 100]))); seq += 1
        for m in ch:
            ev.append((t, seq, bytes([0x90, pitch[m], 100]))); seq += 1
        for m in ch:
            ev.append((t + off, seq, bytes([0x80, pitch[m], 0]))); seq += 1
            if m in tom:
                ev.append((t + off, seq, bytes([0x80, tom[m], 0]))); seq += 1
    hdr = b"MThd" + struct.pack(">IHHH", 6, 1, 2, res)
    return hdr + _track(t0) + _track(ev)


def song_ini(name, length_ms):
    return ("[song]\n"
            f"name = {name}\n"
            "artist = Hydra note shuffle test\n"
            "charter = Hydra\n"
            "genre = Test\n"
            "year = 2026\n"
            "diff_drums = 0\n"
            "pro_drums = True\n"
            "delay = 0\n"
            f"song_length = {length_ms}\n")


def write_song(folder, name, res, chords, fmt, bpm=120):
    os.makedirs(folder, exist_ok=True)
    last = chords[-1][0]
    length_ms = int(last / res * 60000 / bpm) + 4000
    if fmt == "chart":
        with open(os.path.join(folder, "notes.chart"), "w", encoding="utf-8", newline="\n") as f:
            f.write(chart_text(name, res, chords, bpm))
    else:
        with open(os.path.join(folder, "notes.mid"), "wb") as f:
            f.write(mid_bytes(name, res, chords, bpm))
    with open(os.path.join(folder, "song.ini"), "w", encoding="utf-8", newline="\n") as f:
        f.write(song_ini(name, length_ms))
    ogg = os.path.join(folder, "song.ogg")
    if not os.path.exists(ogg):
        subprocess.run([FFMPEG, "-y", "-loglevel", "error", "-f", "lavfi", "-i",
                        "anullsrc=r=44100:cl=stereo", "-t", f"{length_ms / 1000:.3f}",
                        "-c:a", "libvorbis", "-q:a", "0", ogg], check=True)
    return os.path.join(folder, "notes." + fmt)
