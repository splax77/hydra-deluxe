"""The scout's copy of the all-toms tail, for the two scan scripts beside it.

This is a read-only scout, not Hydra. Hydra's owners are `convert_drum_pads`,
`midi_drum_chart_flags` and `drum_key_is_true` in src/parse/song.cpp, and the
plan docs/superpowers/plans/2026-11-12-mid-no-pro-toms.md says what the game
does. If the scout and those owners ever disagree, the owners are right and
this file is stale.

Each chart is read twice: once by the key rule main has today (`old`) and once
by the game's getter rule the plan adopts (`new`). A chart "moves" when the
two readings put it in a different place: another branch, or into the tail.
Only a chart with a yellow, blue or fourth-pad note can show a move, because
those are the pads the tail turns from cymbals to toms.
"""
import io
import re
import zlib

import mido

# Each difficulty's kick pitch, as kDifficultyChartCodes in src/parse/song.cpp
# spells it; the pads sit just above it.
KICK_PITCHES = (96, 84, 72, 60)
FIFTH_PAD_ABOVE_KICK = 5                       # kFifthPadAboveKick
FIFTH_PAD_PITCHES = {k + FIFTH_PAD_ABOVE_KICK for k in KICK_PITCHES}
CYMBAL_PAD_PITCHES = {k + d for k in KICK_PITCHES for d in (2, 3, 4)}  # yellow, blue, fourth pad
TOM_MARKER_PITCHES = {110, 111, 112}           # kYellowTomMarkerPitch .. kGreenTomMarkerPitch
DRUM_TRACK_NAMES = {'part drums', 'part drum'}  # kDrumsTrackNames in src/parse/midi.h

CHART_DRUM_SECTIONS = {'ExpertDrums', 'HardDrums', 'MediumDrums', 'EasyDrums'}
CHART_FIFTH_PAD = 5                            # chart_pad_color
CHART_CYMBAL_MARKERS = {66, 67, 68}            # chart_cymbal_marker_pad
CHART_CYMBAL_PADS = {2, 3, 4}

INT32_MIN, INT32_MAX = -2**31, 2**31 - 1
# .NET's white space for NumberStyles.Integer (AllowLeadingWhite and
# AllowTrailingWhite): tab to carriage return, and space.
DOTNET_WHITE = '\t\n\x0b\x0c\r '
HYDRA_SPACE = ' \t\r\n'


def old_key_read(value):
    """Main today: `drum_key_is_true` before the plan's T2."""
    return value is not None and value.strip(HYDRA_SPACE).lower() == 'true'


def new_key_read(value):
    """The game's getter, as the plan's "How the game reads the two keys"
    section and T2 have it; Hydra keeps its trim until Gap 2 is settled."""
    if value is None:
        return False
    m = re.fullmatch(r'[%s]*([+-]?[0-9]+)[%s]*' % (re.escape(DOTNET_WHITE), re.escape(DOTNET_WHITE)),
                     value)
    if m:
        n = int(m.group(1))
        if INT32_MIN <= n <= INT32_MAX:
            return n != 0
    return value.strip(HYDRA_SPACE).lower() == 'true'


def is_number(value):
    """Whether a key's text is a whole number, so the two readings can split."""
    return value is not None and re.fullmatch(r'\s*[+-]?[0-9]+\s*', value) is not None


def decode_text(raw):
    if raw.startswith(b'\xef\xbb\xbf'):
        return raw[3:].decode('utf-8', errors='replace')
    if raw.startswith(b'\xff\xfe') or raw.startswith(b'\xfe\xff'):
        return raw.decode('utf-16', errors='replace')
    try:
        return raw.decode('utf-8')
    except UnicodeDecodeError:
        return raw.decode('latin-1')


def read_song_ini_keys(raw):
    """The [song] section's keys, as read_song_ini_keys in src/parse/song.cpp
    takes them: lower-case keys, the last line of a key wins."""
    keys, in_song = {}, False
    for line in decode_text(raw).split('\n'):
        line = line.rstrip('\r \t').lstrip(' \t')
        if not line or line[0] in ';#':
            continue
        if line[0] == '[' and line[-1] == ']':
            in_song = line[1:-1].lower() == 'song'
            continue
        if not in_song or '=' not in line:
            continue
        k, v = line.split('=', 1)
        keys[k.lower().rstrip(' \t')] = v.lstrip(' \t')
    return keys


def sng_keys(pairs):
    """A .sng's metadata pairs as a key lookup: case ignored, the last pair wins
    (chart_drum_keys for sng metadata)."""
    keys = {}
    for k, v in pairs:
        keys[k.lower()] = v
    return keys


def read_sng(raw):
    """(metadata pairs, notes name, notes bytes) of a .sng, by src/parse/sng.cpp.
    The notes file is notes.mid when there is one, else notes.chart
    (pick_notes_file); None when there is neither."""
    mask = raw[10:26]
    meta_len = int.from_bytes(raw[26:34], 'little')
    pos = 34
    count = int.from_bytes(raw[pos:pos + 8], 'little')
    pos += 8
    pairs = []
    for _ in range(count):
        kl = int.from_bytes(raw[pos:pos + 4], 'little'); pos += 4
        k = decode_text(raw[pos:pos + kl]); pos += kl
        vl = int.from_bytes(raw[pos:pos + 4], 'little'); pos += 4
        v = decode_text(raw[pos:pos + vl]); pos += vl
        pairs.append((k, v))
    pos = 34 + meta_len + 8
    count = int.from_bytes(raw[pos:pos + 8], 'little')
    pos += 8
    entries = []
    for _ in range(count):
        nl = raw[pos]; pos += 1
        name = raw[pos:pos + nl].decode('utf-8', errors='replace'); pos += nl
        length = int.from_bytes(raw[pos:pos + 8], 'little')
        offset = int.from_bytes(raw[pos + 8:pos + 16], 'little')
        pos += 16
        entries.append((name, length, offset))
    pick = None
    for name, length, offset in entries:
        if name.lower() == 'notes.mid':
            pick = (name, length, offset)
            break
        if name.lower() == 'notes.chart':
            pick = (name, length, offset)
    if pick is None:
        return pairs, None, None
    name, length, offset = pick
    data = raw[offset:offset + length]
    key = bytes(mask[j % 16] ^ j for j in range(256))
    notes = bytes(b ^ key[i & 0xff] for i, b in enumerate(data))
    return pairs, name, notes


def read_srb(raw):
    """(notes name, notes bytes, is_mid) of an .srb, by src/parse/srb.cpp:
    stream 1 is the metadata, stream 2 the notes. An .srb carries no song.ini
    keys."""
    d1 = zlib.decompressobj(-15)
    meta = d1.decompress(raw[16:])
    rest = d1.unused_data
    notes = zlib.decompressobj(-15).decompress(rest)
    name = ''
    if len(meta) >= 8:
        n = int.from_bytes(meta[4:8], 'little')
        name = decode_text(meta[8:8 + n])
    low = name.lower()
    is_mid = low == 'notes.mid' or (low != 'notes.chart' and notes[:4] == b'MThd')
    return name, notes, is_mid


def mid_facts(data):
    """What the tail asks of a .mid's drum track, or None when it has none."""
    mid = mido.MidiFile(file=io.BytesIO(data))
    for track in mid.tracks[1:]:
        names = [m.name for m in track if m.type == 'track_name']
        if not any(n.lower() in DRUM_TRACK_NAMES for n in names):
            continue
        ons = [m.note for m in track if m.type == 'note_on' and m.velocity > 0]
        return {
            'fifth_pad_note': any(p in FIFTH_PAD_PITCHES for p in ons),
            'pro_mark': any(p in TOM_MARKER_PITCHES for p in ons),
            'cymbal_pads': sum(1 for p in ons if p in CYMBAL_PAD_PITCHES),
            'expert_cymbal_pads': sum(1 for p in ons if p in (98, 99, 100)),
        }
    return None


def chart_facts(text):
    """What the tail asks of a .chart's drum sections, or None when it has none."""
    facts, section, found = {'fifth_pad_note': False, 'pro_mark': False, 'cymbal_pads': 0,
                             'expert_cymbal_pads': 0}, None, False
    for line in text.splitlines():
        s = line.strip()
        if s.startswith('['):
            section = s.strip('[]')
            found = found or section in CHART_DRUM_SECTIONS
            continue
        if section not in CHART_DRUM_SECTIONS:
            continue
        m = re.match(r'\d+\s*=\s*N\s+(\d+)', s)
        if not m:
            continue
        v = int(m.group(1))
        if v == CHART_FIFTH_PAD:
            facts['fifth_pad_note'] = True
        if v in CHART_CYMBAL_MARKERS:
            facts['pro_mark'] = True
        if v in CHART_CYMBAL_PADS:
            facts['cymbal_pads'] += 1
            if section == 'ExpertDrums':
                facts['expert_cymbal_pads'] += 1
    return facts if found else None


def placement(fmt, facts, keys, read):
    """(five-lane branch, tail) for one reading of the keys. A .chart takes only
    five_lane_drums from the keys, and its tail changes nothing in Hydra."""
    five_key = read(keys.get('five_lane_drums'))
    pro_key = read(keys.get('pro_drums')) if fmt == 'mid' else False
    fifth = facts['fifth_pad_note'] or (five_key if fmt == 'mid' else False)
    pro = facts['pro_mark'] or pro_key
    five_lane = (fifth and not pro) or five_key
    tail = fmt == 'mid' and not five_lane and not fifth and not pro
    return five_lane, tail


def judge(fmt, facts, keys):
    """The chart's placement today and after the plan, and why it moves."""
    old = placement(fmt, facts, keys, old_key_read)
    old = (old[0], False)  # main has no tail
    new = placement(fmt, facts, keys, new_key_read)
    reasons = []
    if new[1] and facts['cymbal_pads']:
        reasons.append('tail')
    if old[0] != new[0]:
        reasons.append('branch')
    number_keys = [k for k in ('pro_drums', 'five_lane_drums') if is_number(keys.get(k))]
    return {'old': old, 'new': new, 'moves': bool(reasons), 'reasons': reasons,
            'number_keys': number_keys}
