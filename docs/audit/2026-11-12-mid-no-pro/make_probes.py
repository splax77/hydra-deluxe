"""Make probes G, H and J for the plan's live steps LS1, LS2 and LS3
(docs/superpowers/plans/2026-11-12-mid-no-pro-toms.md).

Each probe is probe F's chart: a four-pad .mid with no tom marker and no
fifth pad, 125 BPM at 480 ticks a beat, so one tick is one millisecond. Each
probe changes one thing from probe F:

  G  song.ini writes pro_drums = 1                         (LS1)
  H  song.ini writes pro_drums = True with a trailing blank (LS2)
  J  a disco section over the two red and three yellow notes, no keys (LS3)

Usage:  py -I make_probes.py <parent folder> [G] [H] [J]

Each probe goes in its own folder, "Mid No Pro <letter>", inside the parent
folder. With no letters it makes all three. It refuses a probe folder that
already exists, before it writes anything. The audio is copied from the
Edge Walk Pair probe, as probe F's was; it is only read.
"""
import json
import os
import shutil
import sys

import mido

AUDIO_SRC = r'C:\Clone Hero\songs\Hydra Probe\Edge Walk Pair\song.ogg'

KICK, RED, YELLOW, BLUE, GREEN = 96, 97, 98, 99, 100
# Expert's disco markers, as the readers match them (is_disco_on_marker and
# is_disco_off_marker in src/parse/song.cpp).
DISCO_ON, DISCO_OFF = '[mix 3 drums0d]', '[mix 3 drums0]'

# Probe F's notes: 29 kicks, then 2 red, 3 yellow, 4 blue and 5 green.
NOTES = [(3000 + 500 * i, KICK) for i in range(29)]
_t = 17500
for _pitch, _count in ((RED, 2), (YELLOW, 3), (BLUE, 4), (GREEN, 5)):
    for _ in range(_count):
        NOTES.append((_t, _pitch))
        _t += 500
LAST = NOTES[-1][0]

# The disco section opens 500 ms before the first red note and closes 250 ms
# after the last yellow one, half way to the first blue note.
_first_red = min(t for t, p in NOTES if p == RED)
_last_yellow = max(t for t, p in NOTES if p == YELLOW)
DISCO_SPAN = (_first_red - 500, _last_yellow + 250)

PROBES = {
    'G': {'ini_extra': 'pro_drums = 1\n', 'disco': False,
          'note': 'probe F notes; song.ini pro_drums = 1 (LS1)'},
    'H': {'ini_extra': 'pro_drums = True \n', 'disco': False,
          'note': 'probe F notes; song.ini pro_drums = True with a trailing blank (LS2)'},
    'J': {'ini_extra': '', 'disco': True,
          'note': 'probe F notes; a [mix 3 drums0d] section over the 2 red and 3 yellow '
                  'notes; no pro_drums or five_lane_drums (LS3)'},
}


def build_mid(letter, disco):
    mid = mido.MidiFile(type=1, ticks_per_beat=480)
    tempo = mido.MidiTrack()
    tempo.append(mido.MetaMessage('track_name', name=f'Probe {letter}', time=0))
    tempo.append(mido.MetaMessage('set_tempo', tempo=480000, time=0))
    tempo.append(mido.MetaMessage('time_signature', numerator=4, denominator=4, time=0))
    tempo.append(mido.MetaMessage('end_of_track', time=0))
    mid.tracks.append(tempo)

    # (tick, order, message): a note-off sorts before a text, a text before a
    # note-on on the same tick.
    events = []
    for tick, pitch in NOTES:
        events.append((tick, 2, mido.Message('note_on', note=pitch, velocity=100)))
        events.append((tick + 60, 0, mido.Message('note_off', note=pitch, velocity=0)))
    if disco:
        events.append((DISCO_SPAN[0], 1, mido.MetaMessage('text', text=DISCO_ON)))
        events.append((DISCO_SPAN[1], 1, mido.MetaMessage('text', text=DISCO_OFF)))
    events.sort(key=lambda e: (e[0], e[1]))
    drums = mido.MidiTrack()
    drums.append(mido.MetaMessage('track_name', name='PART DRUMS', time=0))
    now = 0
    for tick, _, msg in events:
        drums.append(msg.copy(time=tick - now))
        now = tick
    drums.append(mido.MetaMessage('end_of_track', time=0))
    mid.tracks.append(drums)
    return mid


def song_ini(letter, extra):
    return ('[song]\n'
            f'name = Hydra Probe - Mid No Pro {letter}\n'
            'artist = Hydra ch_probe\ncharter = ch_probe\ngenre = Test\nyear = 2026\n'
            'diff_drums = 0\ndelay = 0\n'
            f'song_length = {LAST + 3000}\n'
            f'{extra}')


def main(argv):
    if len(argv) < 2:
        sys.exit('usage: make_probes.py <parent folder> [G] [H] [J]')
    parent = argv[1]
    letters = [a.upper() for a in argv[2:]] or sorted(PROBES)
    for letter in letters:
        if letter not in PROBES:
            sys.exit(f'unknown probe {letter!r}; known: {", ".join(sorted(PROBES))}')
    if not os.path.isdir(parent):
        sys.exit(f'refusing: parent folder {parent} does not exist')
    if not os.path.isfile(AUDIO_SRC):
        sys.exit(f'refusing: audio {AUDIO_SRC} not found')
    targets = {L: os.path.join(parent, f'Mid No Pro {L}') for L in letters}
    for dst in targets.values():
        if os.path.exists(dst):
            sys.exit(f'refusing: {dst} exists')

    for letter, dst in targets.items():
        spec = PROBES[letter]
        os.makedirs(dst)
        build_mid(letter, spec['disco']).save(os.path.join(dst, 'notes.mid'))
        shutil.copyfile(AUDIO_SRC, os.path.join(dst, 'song.ogg'))
        ini = song_ini(letter, spec['ini_extra'])
        # newline='' keeps the trailing blank on probe H's line as written.
        with open(os.path.join(dst, 'song.ini'), 'w', encoding='utf-8', newline='') as f:
            f.write(ini)
        manifest = {'song': f'Hydra Probe - Mid No Pro {letter}', 'resolution': 480,
                    'bpm': 125.0, 'note': '1 tick = 1 ms; ' + spec['note'],
                    'notes': [{'time_ms': t, 'pitch': p} for t, p in NOTES]}
        if spec['disco']:
            manifest['disco'] = {'on_ms': DISCO_SPAN[0], 'off_ms': DISCO_SPAN[1],
                                 'on_text': DISCO_ON, 'off_text': DISCO_OFF}
        with open(os.path.join(dst, 'manifest.json'), 'w', encoding='utf-8') as f:
            json.dump(manifest, f, indent=1)
        print(f'== {dst}: {len(NOTES)} notes, last at {LAST}')
        print('-- song.ini (repr of each line):')
        for line in ini.splitlines(keepends=True):
            print('   ' + repr(line))


if __name__ == '__main__':
    main(sys.argv)
