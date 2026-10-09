"""Build song F (a 2x kick and a normal kick on one tick, decision D105) as a .mid and a .chart,
then show which chords Hydra reads with 2x Bass off and on.

usage: py -I build_f.py
"""
import json, os, struct, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ns_lib import REPO, _track, _vlq, song_ini, FFMPEG

OUT = REPO + "/docs/audit/note-shuffle/game-tests/songs"
EXE = REPO + "/build-cpp/Release/hydra_replay.exe"
RES, BPM = 480, 120
# One chord every half second from 2.0 s. Each chord lists its kicks in file order:
# "n" is a normal kick (MIDI 96, .chart N 0), "x" a 2x kick (MIDI 95, .chart N 32). Every chord has a red snare.
LAYOUT = ["n", "", "nx", "", "xn", "", "x", "", "n"]
CHORDS = [(1920 + 480 * i, kicks) for i, kicks in enumerate(LAYOUT)]


def mid_bytes(name):
    t0 = [(0, 0, b"\xff\x03" + _vlq(len(name)) + name.encode()),
          (0, 1, b"\xff\x51\x03" + struct.pack(">I", 60000000 // BPM)[1:]),
          (0, 2, b"\xff\x58\x04\x04\x02\x18\x08")]
    tn = "PART DRUMS"
    ev, seq = [(0, 0, b"\xff\x03" + _vlq(len(tn)) + tn.encode())], 1
    pitch = {"n": 96, "x": 95}
    for t, kicks in CHORDS:
        notes = [pitch[k] for k in kicks] + [97]
        for p in notes:
            ev.append((t, seq, bytes([0x90, p, 100]))); seq += 1
        for p in notes:
            ev.append((t + 30, seq, bytes([0x80, p, 0]))); seq += 1
    return b"MThd" + struct.pack(">IHHH", 6, 1, 2, RES) + _track(t0) + _track(ev)


def chart_text(name):
    lines = ["[Song]", "{", f'  Name = "{name}"', '  Artist = "Hydra note shuffle test"',
             '  Charter = "Hydra"', "  Offset = 0", f"  Resolution = {RES}", '  Genre = "Test"',
             '  MediaType = "cd"', '  MusicStream = "song.ogg"', "}",
             "[SyncTrack]", "{", "  0 = TS 4", f"  0 = B {BPM * 1000}", "}",
             "[Events]", "{", "}", "[ExpertDrums]", "{"]
    code = {"n": 0, "x": 32}
    for t, kicks in CHORDS:
        for k in kicks:
            lines.append(f"  {t} = N {code[k]} 0")
        lines.append(f"  {t} = N 1 0")
    lines.append("}")
    return "\n".join(lines) + "\n"


def write(folder, name, fmt):
    os.makedirs(folder, exist_ok=True)
    length_ms = int(CHORDS[-1][0] / RES * 60000 / BPM) + 4000
    path = os.path.join(folder, "notes." + fmt)
    if fmt == "mid":
        open(path, "wb").write(mid_bytes(name))
    else:
        open(path, "w", encoding="utf-8", newline="\n").write(chart_text(name))
    open(os.path.join(folder, "song.ini"), "w", encoding="utf-8", newline="\n").write(song_ini(name, length_ms))
    ogg = os.path.join(folder, "song.ogg")
    if not os.path.exists(ogg):
        subprocess.run([FFMPEG, "-y", "-loglevel", "error", "-f", "lavfi", "-i", "anullsrc=r=44100:cl=stereo",
                        "-t", f"{length_ms / 1000:.3f}", "-c:a", "libvorbis", "-q:a", "0", ogg], check=True)
    return path


def hydra_kicks(path, bass2x):
    r = subprocess.run([EXE, "score", "--chart", path, "--prodrums", "1", "--bass2x", "1" if bass2x else "0"],
                       capture_output=True, text=True, check=True)
    j = json.loads(r.stdout)
    return {c["tick"]: any(n["color"] == "Kick" for n in c["notes"]) for c in j["chords"]}


for folder, fmt in (("NS F1 2x kick shared tick mid", "mid"), ("NS F2 2x kick shared tick chart", "chart")):
    path = write(os.path.join(OUT, folder), folder, fmt)
    for b in (False, True):
        k = hydra_kicks(path, b)
        row = ["kick" if k.get(t) else "-" for t, _ in CHORDS]
        print(f"{folder} | Hydra, 2x Bass {'on ' if b else 'off'}: {row}")
print("written (file order):", [k or "-" for _, k in CHORDS])
