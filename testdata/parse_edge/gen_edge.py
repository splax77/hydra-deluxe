"""Writes the crafted chart files that pin how the .chart and .mid readers
treat odd and broken input (the speedups plan, task P1).

Each file below is one case, and its comment says what it tests. The files,
list.txt (one file name per line, relative to this folder) and nothing else
are written here. Run it again and every byte comes out the same.

The expected digests beside them (expected_expert.tsv, expected_hard.tsv) were
first captured from the readers as they stood before P1, with hydra_bench
--parse. Later changes to the readers or the digest re-pinned some lines; git
log on the two files says which and why. tests/test_song.cpp reads them back
("the crafted edge files parse as the old readers did").

    py testdata/parse_edge/gen_edge.py
"""

import os
import struct

HERE = os.path.dirname(os.path.abspath(__file__))

# ---- .chart ---------------------------------------------------------------

SONG = """[Song]
{
  Name = "Edge"
  Resolution = 192
  Offset = 0
}
"""

SYNC = """[SyncTrack]
{
  0 = TS 4
  0 = B 120000
  1536 = TS 3 3
  1536 = B 140000
}
"""

EVENTS = """[Events]
{
  0 = E "section Intro"
  768 = E "section Verse 1"
  768 = E "prc_verse_1b"
  1536 = E "lighting (chase)"
}
"""

# A drum section with notes, cymbals, dynamics, a 2x kick, an SP phrase, an
# activation fill and a solo, so every op the reader knows runs at least once.
DRUM_BODY = [
    "0 = N 0 0", "0 = N 2 0", "0 = N 66 0",
    "192 = N 1 0", "192 = N 34 0",
    "384 = S 2 576", "384 = N 3 0", "384 = N 41 0",
    "576 = N 4 0", "576 = N 68 0", "576 = N 0 0", "576 = N 32 0",
    "768 = E solo", "768 = N 1 0", "768 = N 40 0",
    "960 = N 2 0", "960 = N 35 0", "960 = E soloend",
    "1152 = S 2 192", "1152 = N 3 0", "1152 = N 67 0",
    "1344 = N 4 0", "1344 = N 43 0",
    "1536 = S 64 384", "1536 = N 1 0",
    "1728 = N 2 0", "1728 = N 36 0",
    "1920 = N 0 0", "1920 = N 4 0",
    "2112 = N 1 0", "2112 = N 42 0",
    "2304 = N 3 0", "2304 = N 37 0",
    "2496 = N 0 0", "2496 = N 32 0",
]


def section(name, lines):
    return "[%s]\n{\n%s}\n" % (name, "".join("  %s\n" % l for l in lines))


def drums(name="ExpertDrums", body=None):
    return section(name, DRUM_BODY if body is None else body)


def base_chart(song=SONG, sync=SYNC, events=EVENTS, expert=None, hard=None, extra=""):
    parts = [song, sync, events]
    parts.append(drums("ExpertDrums", expert))
    parts.append(drums("HardDrums", hard))
    parts.append(extra)
    return "".join(parts)


def skipped(lines):
    return section("ExpertSingle", lines)


CHARTS = {}

# A well-formed chart: the control case every other file varies.
CHARTS["c01_base.chart"] = base_chart()

# A bad number in a section the reader does not use still refuses the chart.
CHARTS["c02_bad_number_skipped.chart"] = base_chart(extra=skipped(["0 = N 0 0", "192 = N x 0"]))
# An overflowing note value in an unused section.
CHARTS["c03_overflow_skipped.chart"] = base_chart(
    extra=skipped(["0 = N 0 0", "192 = N 99999999999 0"]))
# An overflowing length in an unused section (stoll, past 64 bits).
CHARTS["c04_overflow_length_skipped.chart"] = base_chart(
    extra=skipped(["192 = S 2 99999999999999999999"]))

# A bad note value in the drum section.
CHARTS["c05_bad_number_drums.chart"] = base_chart(expert=DRUM_BODY + ["2688 = N q 0"])
# An overflowing note length in the drum section.
CHARTS["c06_overflow_drums.chart"] = base_chart(
    expert=DRUM_BODY + ["2688 = N 0 123456789012345678901"])
# A note value past int's range (10 digits) in the drum section.
CHARTS["c07_int_overflow_drums.chart"] = base_chart(expert=DRUM_BODY + ["2688 = N 4294967297 0"])

# A bad tempo in [Events], which no reader takes tempo from.
CHARTS["c08_bad_number_events.chart"] = base_chart(
    events=section("Events", ['0 = E "section Intro"', "768 = B fast"]))
# An overflowing meter in [Events].
CHARTS["c09_overflow_events.chart"] = base_chart(
    events=section("Events", ['0 = E "section Intro"', "768 = TS 99999999999"]))

# A tick line with a bad number in [Song].
CHARTS["c10_bad_number_song.chart"] = base_chart(
    song=SONG.replace("  Offset = 0\n", "  Offset = 0\n  0 = N z 0\n"))
# An overflowing tempo in [Song].
CHARTS["c11_overflow_song.chart"] = base_chart(
    song=SONG.replace("  Offset = 0\n", "  Offset = 0\n  0 = B 999999999999999999999\n"))

# Keys that are and are not ticks: a sign, leading zeros, trailing junk, a
# 19-digit tick, a 20-digit key past 64 bits, hex, an exponent, a lone minus
# and an empty key. Only the ones std::stoll reads whole are ticks.
CHARTS["c12_odd_keys.chart"] = base_chart(expert=DRUM_BODY + [
    "+2688 = N 0 0",
    "02880 = N 1 0",
    "3072abc = N 2 0",
    "1000000000000000000 = N 3 0",
    "99999999999999999999 = N 4 0",
    "0x10 = N 0 0",
    "1e3 = N 1 0",
    "- = N 2 0",
    "= N 3 0",
    "-192 = N 4 0",
    "Name = N 0 0",
])

# Number spellings that parse: a sign, leading zeros, trailing junk, 10 and
# 19 digits that still fit, negative values and lengths.
CHARTS["c13_odd_numbers.chart"] = base_chart(expert=DRUM_BODY + [
    "2688 = N +1 0",
    "2880 = N 0000000002 0",
    "3072 = N 3x 0",
    "3264 = N -0 0",
    "3456 = N 4 0000000000000000192",
    "3648 = N 1 -5",
    "3840 = S 2 +192", "3840 = N 2 0",
    "4032 = S 2 -100", "4032 = N 3 0",
    "4224 = N -1 0", "4224 = N 0 0",
    "4416 = S 64 1x", "4416 = N 4 0",
], extra=section("SyncTrack", [
    "0 = TS 4 +2", "0 = B 120000junk", "768 = TS 0", "1536 = TS 6 0003", "2304 = B 0150000",
]))

# Every section twice: the later copy replaces the earlier one.
CHARTS["c14_duplicate_sections.chart"] = base_chart(
    extra=drums("ExpertDrums", ["0 = N 1 0", "192 = N 2 0", "384 = N 3 0"]) +
    section("Song", ["Resolution = 480", "Offset = 0.25"]) +
    section("SyncTrack", ["0 = B 90000"]) +
    section("Events", ['0 = E "section Outro"']) +
    drums("HardDrums", ["0 = N 4 0", "480 = N 0 0"]))

# The drum section is never closed: the file ends inside it.
CHARTS["c15_unterminated_drums.chart"] = (
    SONG + SYNC + EVENTS + drums("HardDrums") +
    "[ExpertDrums]\n{\n" + "".join("  %s\n" % l for l in DRUM_BODY))
# A second copy of the drum section that is never closed: the first stays.
CHARTS["c16_unterminated_second_copy.chart"] = (
    base_chart() + "[ExpertDrums]\n{\n  0 = N 1 0\n  [HardDrums]\n  {\n  192 = N 2 0\n")
# A section left open swallows the next header and its lines as its own.
CHARTS["c17_header_inside_section.chart"] = (
    SONG + SYNC + "[Events]\n{\n  0 = E \"section Intro\"\n" + drums("ExpertDrums") +
    drums("HardDrums"))

# Ticks out of order, one tick's lines split across the section, unsorted
# events and a sync track written backwards.
CHARTS["c18_unsorted_ticks.chart"] = base_chart(
    expert=list(reversed(DRUM_BODY)) + ["0 = N 4 0", "576 = N 37 0"],
    hard=DRUM_BODY[10:] + DRUM_BODY[:10],
    events=section("Events", ['1536 = E "section C"', '0 = E "section A"',
                              '768 = E "section B2"', '768 = E "section B1"']),
    sync=section("SyncTrack", ["1536 = B 140000", "0 = B 120000", "0 = TS 4"]))

# CRLF line ends, a stray CR in a line, tabs and trailing spaces.
CHARTS["c19_crlf.chart"] = base_chart().replace("\n", "\r\n").replace(
    "192 = N 1 0", "192\t=\tN 1 0   ").replace("576 = N 4 0", "576 = N 4\r 0")

# Disco markers for each difficulty, bracketed and bare, with a digit, the
# noflip spelling and near misses.
DISCO = DRUM_BODY + [
    "2688 = E mix_3_drums0d", "2688 = N 1 0", "2688 = N 2 0",
    "2880 = E [mix 3 drums]", "2880 = N 2 0",
    "3072 = E mix.2.drums1d", "3072 = N 3 0",
    "3264 = E [mix_2_drumsdnoflip]", "3264 = N 1 0",
    "3456 = E mix_3_drums9d", "3456 = N 2 0",
    "3648 = E mix_3_drumsdnoflip", "3648 = N 3 0",
    "3840 = E mix_3_drums00d", "3840 = N 2 0",
    "4032 = E \"mix_3_drums0d\"", "4032 = N 1 0",
    "4224 = E mix_3_drums", "4224 = N 2 0",
]
CHARTS["c20_disco.chart"] = base_chart(expert=DISCO, hard=DISCO)

# No [Song] section at all.
CHARTS["c21_missing_song.chart"] = SYNC + EVENTS + drums() + drums("HardDrums")
# [Song] without a Resolution.
CHARTS["c22_missing_resolution.chart"] = base_chart(song=section("Song", ['Name = "x"', "Offset = 0"]))
# A Resolution with trailing junk, and an Offset that is not a number.
CHARTS["c23_resolution_junk.chart"] = base_chart(
    song=section("Song", ["Resolution = 480junk", "Offset = 500ms"]))
# A Resolution with no digits.
CHARTS["c24_bad_resolution.chart"] = base_chart(song=section("Song", ["Resolution = abc"]))

# A file whose first line is not a section header.
CHARTS["c25_bad_header.chart"] = "Song\n{\n}\n" + base_chart()
# Headers with junk around the brackets and brackets inside the name.
CHARTS["c26_header_junk.chart"] = (
    "x[Song]y\n{\n  Resolution = 192\n}\n" + SYNC + "[[Events]]\n{\n  0 = E \"section Q\"\n}\n" +
    "  [ExpertDrums] trailing\n{\n" + "".join("  %s\n" % l for l in DRUM_BODY) + "}\n" +
    drums("HardDrums"))

# Lines with nothing after '=', no '=', two '=', a one-word note, a bare
# event, a quoted section with inner spaces, and a property in the drum
# section.
CHARTS["c27_empty_values.chart"] = base_chart(expert=DRUM_BODY + [
    "2688 =", "2688", "2880 = N 0", "2880 = E", "3072 = N 1 0 = 5",
    "3264 = E \"section  Two  Spaces\"", "Speed = 3", "3456 = N 2 0",
], events=section("Events", ['0 = E "section Intro"', "192 = E section Bare",
                             '384 = E "prc_"', "576 =", "768 = E \"\"", "960 = E \"section \"",
                             "Name = E \"section Prop\""]))

# Tempo and meter lines inside the drum section (they never run there), and
# a phrase that ends on a tick holding only such a line.
CHARTS["c28_timing_in_drums.chart"] = base_chart(expert=DRUM_BODY + [
    "2688 = S 2 192", "2688 = N 1 0", "2880 = B 60000", "2880 = TS 7 3", "3072 = N 2 0",
    "3264 = TS 2", "3456 = B 0",
])


# ---- .mid -----------------------------------------------------------------

def varlen(n):
    out = [n & 0x7F]
    n >>= 7
    while n:
        out.append(0x80 | (n & 0x7F))
        n >>= 7
    return bytes(reversed(out))


def header(fmt=1, ntracks=2, division=480):
    return b"MThd" + struct.pack(">IHHh", 6, fmt, ntracks, division)


def chunk(tag, body):
    return tag + struct.pack(">I", len(body)) + body


def meta(delta, kind, payload):
    if isinstance(payload, str):
        payload = payload.encode("latin-1")
    return varlen(delta) + bytes([0xFF, kind]) + varlen(len(payload)) + payload


def note(delta, status, pitch, vel):
    return varlen(delta) + bytes([status, pitch, vel])


def end_of_track(delta=0):
    return varlen(delta) + b"\xFF\x2F\x00"


def tempo_track(us_per_beat=500000, name="tempo"):
    return chunk(b"MTrk",
                 meta(0, 0x03, name) +
                 meta(0, 0x58, bytes([4, 2, 24, 8])) +
                 meta(0, 0x51, us_per_beat.to_bytes(3, "big")) +
                 meta(1920, 0x58, bytes([3, 2, 24, 8])) +
                 meta(0, 0x51, (400000).to_bytes(3, "big")) +
                 end_of_track())


# Note-on/off pairs over Expert (96-100, 2x 95) and Hard (84-88, 2x 83),
# with markers (solo 103, flam 109, toms 110-112, SP 116, fill 120), ghost
# and accent velocities, and every off spelled both ways.
def drum_events():
    ev = []
    t = 0

    def at(tick, data):
        nonlocal t
        ev.append(varlen(tick - t) + data)
        t = tick

    at(0, bytes([0xFF, 0x01]) + varlen(23) + b"[ENABLE_CHART_DYNAMICS]")
    at(0, bytes([0x99, 116, 100]))
    for i, tick in enumerate(range(0, 480 * 16, 240)):
        lane = i % 5
        vel = (1, 127, 100, 64, 127)[i % 5]
        at(tick, bytes([0x99, 96 + lane, vel]))
        at(tick, bytes([0x99, 84 + lane, 100]))
        if i % 4 == 0:
            at(tick, bytes([0x99, 95, 100]))
            at(tick, bytes([0x99, 83, 100]))
        if i == 6:
            at(tick, bytes([0x99, 110, 100]))
        if i == 9:
            at(tick, bytes([0x89, 110, 0]))
        if i == 10:
            at(tick, bytes([0x99, 103, 100]))
            at(tick, bytes([0x99, 109, 100]))
        if i == 14:
            at(tick, bytes([0x99, 103, 0]))
            at(tick, bytes([0x89, 109, 64]))
        if i == 3:
            at(tick, bytes([0x99, 116, 0]))
        if i == 20:
            at(tick, bytes([0x99, 120, 100]))
        if i == 24:
            at(tick, bytes([0x89, 120, 0]))
        if i == 16:
            at(tick, bytes([0x99, 116, 100]))
        if i == 18:
            at(tick, bytes([0x89, 116, 0]))
        if i == 12:
            at(tick, bytes([0xFF, 0x01]) + varlen(15) + b"[mix 3 drums0d]")
            at(tick, bytes([0xFF, 0x01]) + varlen(15) + b"[mix 2 drums0d]")
        if i == 22:
            at(tick, bytes([0xFF, 0x05]) + varlen(14) + b"[mix 3 drums0]")
            at(tick, bytes([0xFF, 0x01]) + varlen(14) + b"[mix 2 drums0]")
        if i == 5:
            at(tick, bytes([0xFF, 0x04]) + varlen(15) + b"[mix 3 drums0d]")
            at(tick, bytes([0xFF, 0x08]) + varlen(4) + b"prog")
            at(tick, bytes([0xFF, 0x09]) + varlen(3) + b"dev")
            at(tick, bytes([0xFF, 0x06]) + varlen(6) + b"marker")
            at(tick, bytes([0xFF, 0x07]) + varlen(3) + b"cue")
            at(tick, bytes([0xFF, 0x02]) + varlen(1) + b"c")
        at(tick + 60, bytes([0x89, 96 + lane, 0]))
        at(tick + 60, bytes([0x99, 84 + lane, 0]))
        at(tick + 60, bytes([0xB9, 7, 100]))
        at(tick + 60, bytes([0xC9, 3]))
        at(tick + 60, bytes([0x99, 60 + lane, 100]))
    return b"".join(ev)


def drum_track(name="PART DRUMS", extra_front=b"", body=None):
    return chunk(b"MTrk", meta(0, 0x03, name) + extra_front +
                 (drum_events() if body is None else body) + end_of_track())


def events_track(name="EVENTS"):
    return chunk(b"MTrk",
                 meta(0, 0x03, name) +
                 meta(0, 0x01, "[section Intro]") +
                 meta(1920, 0x06, "[prc_chorus_1]") +
                 meta(0, 0x01, "[section Verse]") +
                 meta(960, 0x05, "[section Lyric]") +
                 meta(0, 0x03, "[section Name]") +
                 meta(0, 0x01, "section NoBrackets") +
                 end_of_track())


def other_track(name="PART GUITAR", body=b""):
    return chunk(b"MTrk", meta(0, 0x03, name) +
                 note(0, 0x90, 96, 100) + note(240, 0x80, 96, 0) + body + end_of_track())


def mid(tracks, fmt=1, division=480):
    return header(fmt, len(tracks), division) + b"".join(tracks)


MIDS = {}

# A well-formed file: tempo track, drums for every difficulty, events.
MIDS["m01_base.mid"] = mid([tempo_track(), other_track(), drum_track(), events_track()])

# Two drum tracks: only the first is read.
MIDS["m02_two_drum_tracks.mid"] = mid([
    tempo_track(), drum_track(), drum_track(body=note(0, 0x99, 97, 100) + note(480, 0x89, 97, 0)),
    events_track()])

# Late and extra names: "notes" at tick 0 then "PART DRUMS" later, a track
# named "Drums" after its "PART DRUMS", and an EVENTS track that is named
# twice.
MIDS["m03_late_names.mid"] = mid([
    tempo_track(),
    chunk(b"MTrk", meta(0, 0x03, "notes") + note(0, 0x99, 98, 100) +
          meta(480, 0x03, "PART DRUMS") + meta(0, 0x03, "Drums") + drum_events() + end_of_track()),
    drum_track(name="Drums"),
    chunk(b"MTrk", meta(0, 0x03, "EVENTS") + meta(0, 0x03, "PART DRUMS") +
          meta(0, 0x01, "[section Late]") + end_of_track()),
])

# A meta over the 1,000,000-byte cap in a track the song parser never reads.
OVERSIZE = varlen(0) + bytes([0xFF, 0x01]) + varlen(1000001) + b"x" * 16
MIDS["m04_oversize_meta_other.mid"] = mid([
    tempo_track(), other_track(body=OVERSIZE), drum_track(), events_track()])
# The same, inside the drum track after its notes.
MIDS["m05_oversize_meta_drums.mid"] = mid([
    tempo_track(), drum_track(body=drum_events() + OVERSIZE), events_track()])

# Format 0: one track holding the tempo, the name and the drums.
MIDS["m06_format0.mid"] = mid([chunk(b"MTrk",
    meta(0, 0x03, "PART DRUMS") + meta(0, 0x51, (500000).to_bytes(3, "big")) +
    meta(0, 0x58, bytes([4, 2, 24, 8])) + drum_events() +
    meta(0, 0x01, "[section Inline]") + end_of_track())], fmt=0)

# SMPTE time division.
MIDS["m07_smpte.mid"] = mid([tempo_track(), drum_track()], division=-7680)

# A file cut off inside the drum track.
_full = mid([tempo_track(), drum_track(), events_track()])
MIDS["m08_truncated.mid"] = _full[: len(mid([tempo_track()])) + 600]

# Running status carried across a meta event, and a data byte at a track's
# start with no status to run from.
_rs = (varlen(0) + bytes([0x99, 96, 100]) + varlen(0) + bytes([97, 100]) +
       meta(240, 0x01, "[mix 3 drums0d]") + varlen(0) + bytes([98, 100]) +
       varlen(0) + bytes([99, 1]) + varlen(240) + bytes([96, 0]) + varlen(0) + bytes([100, 127]))
MIDS["m09_running_status_after_meta.mid"] = mid([
    tempo_track(),
    chunk(b"MTrk", meta(0, 0x03, "PART DRUMS") + _rs + drum_events() + end_of_track()),
    chunk(b"MTrk", varlen(0) + bytes([0x40, 0x40]) + meta(0, 0x03, "EVENTS") + end_of_track()),
])

# An unknown chunk between tracks, one cut short at the end, and a header
# longer than six bytes.
MIDS["m10_unknown_chunk.mid"] = (
    b"MThd" + struct.pack(">IHHh", 8, 1, 3, 480) + b"\x00\x00" + tempo_track() +
    chunk(b"XFIH", b"\x01\x02\x03\x04\x05") + drum_track() + events_track() + b"MTr")

# A status byte the reader cannot size (0xF1) stops the drum track there;
# a sysex before it is skipped by its length.
MIDS["m11_bad_status.mid"] = mid([
    tempo_track(),
    drum_track(body=drum_events() + varlen(0) + bytes([0xF0]) + varlen(3) + b"\x01\x02\xF7" +
               note(10, 0x99, 97, 100) + varlen(0) + bytes([0xF1, 0x00]) +
               note(10, 0x99, 98, 100)),
    events_track()])

# A tempo of 0 microseconds per beat.
MIDS["m12_zero_tempo.mid"] = mid([tempo_track(us_per_beat=0), drum_track(), events_track()])

# Data bytes past 127 (clipped as mido clips them), a note-on with velocity
# 0 as the off, short tempo and meter metas, and tempo metas in the drum
# track, which only the first track's count.
MIDS["m13_clipped_and_short_metas.mid"] = mid([
    chunk(b"MTrk", meta(0, 0x51, b"\x07\xA1") + meta(0, 0x58, b"\x04") +
          meta(0, 0x51, (600000).to_bytes(3, "big")) + meta(0, 0x58, bytes([5, 3])) +
          end_of_track()),
    drum_track(extra_front=meta(0, 0x51, (100).to_bytes(3, "big")) +
               varlen(0) + bytes([0x99, 0x60, 0xFF]) + varlen(0) + bytes([0x99, 0xE1, 0x40]) +
               varlen(120) + bytes([0x99, 0x60, 0x00])),
    events_track()])


def main():
    names = []
    for name, text in CHARTS.items():
        with open(os.path.join(HERE, name), "wb") as f:
            f.write(text.encode("latin-1"))
        names.append(name)
    for name, data in MIDS.items():
        with open(os.path.join(HERE, name), "wb") as f:
            f.write(data)
        names.append(name)
    with open(os.path.join(HERE, "list.txt"), "w", newline="\n") as f:
        f.write("".join(n + "\n" for n in names))
    print("%d files" % len(names))


if __name__ == "__main__":
    main()
