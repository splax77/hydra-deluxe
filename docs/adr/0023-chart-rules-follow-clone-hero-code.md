# Chart rules follow Clone Hero's code, with the exceptions written here

Hydra reads a drum chart to decide which notes exist, which ones are
cymbals, ghosts or 2x kicks, and which ones pay Star Power. Until step 2
those rules came from the original Python port, and a few of them differed
from Clone Hero 1.1 in ways nobody had chosen.

In October 2026 Clone Hero 1.1's GameAssembly.dll was read statically (not
run) and each reading rule was compared with Hydra's. The evidence, with
code addresses, is in
`.superpowers/sdd/2026-10-03-step1-engine-facts/ch-evidence.md` (a local
working file, not checked in) and the step-2 briefs.

## The decision

Where Clone Hero's code answers a reading question, Hydra gives the same
answer, through one owner in `src/parse/song.cpp`. The rules that changed:

- Disco flip is read per difficulty. Each difficulty uses only its own
  `[mix N drums...]` markers, in both formats (Clone Hero 0x215C750,
  0x213D076, 0x2155050). The flip happens only with Pro Drums on, and that
  gate is written once.
- A 2x kick is read per difficulty: MIDI 59 Easy, 71 Medium, 83 Hard, 95
  Expert, and `.chart` N 32 in its own section (0x21555CD, 0x210D9C0). The
  2x Bass setting works at every difficulty, like Clone Hero's Double Kick
  modifier, which is its only gate (0x20D32B7).
- A Star Power phrase pays on the last chord with start <= tick < end. A
  zero-length phrase pays nothing, and a phrase that runs past the last note
  pays on that note (0x20D2440).
- An authored fill lands on a chord up to floor(resolution x slop) + 1
  ticks after the fill ends (0x20D0088-0x20D008D). The +1 applies on top of
  a `hydra_rules.ini` slop too.
- Each authored fill is placed on its own once every chord is read
  (0x20CFF60 calling 0x5DE030): on the last chord at or before its end but
  not before its start, or on the first chord after its end inside the
  window. The closer wins, and a tie goes to the later chord. A fill with
  neither is dropped.
- The MIDI dynamics tag counts only as `ENABLE_CHART_DYNAMICS` or
  `[ENABLE_CHART_DYNAMICS]`, compared exactly, and at a shared tick the file
  order decides (0x21557A5, 0x21557BB). This assumes Clone Hero's MIDI
  reader does not trim the text first; that was not checked.
- A `.chart` cymbal, ghost or accent marker with no note at its tick is
  skipped instead of failing the chart (0x215DDB0). The case of a chord at
  that tick without that colour was not traced.

Rules that were already Clone Hero's and are now written down:

- `.chart` ghosts and accents always count, and a `.chart` always reports
  dynamics on (0x213D620, 0x215DBD0). There is no tag in `.chart`.
- `.chart` gameplay events (solo, soloend, disco) match the raw text after
  trimming whitespace. A quoted `E "solo"` is not a solo (0x213CAF0,
  0x20FE510). Practice-section names strip one pair of quotes, because that
  is how the format writes them. There is no shared normaliser for the two,
  on purpose.

## Deliberate differences from Clone Hero

- `drums0dnoflip` turns flip off. Clone Hero's on/off check looks only at
  the 7th character of the text after `mix N `, so it reads `dnoflip` as on
  (0x215CD50). That is a Clone Hero bug, and Hydra does not copy it. It
  affects 48 library charts. For the same reason Hydra keeps its own on/off
  reading of `drumsNeasy`, `drumsNeasynokick` and `drumsd`, which Clone
  Hero's one-character rule reads differently on 5 chart-and-difficulty
  pairs.
- The backend leeway is Hydra's own rule. No such constant was found in the
  engine methods read; the 3 ms is Hydra's own setting. A note less than
  `backend_leeway_ms` (3 ms) after the SP end scores under Star Power; a
  note exactly that far after it does not (decision D29).
- When a fill has a chord on only one side of its end, Clone Hero's
  0x5DE030 takes that chord however far away. Hydra keeps the window's
  bound and drops the fill (decision D30). No measured chart scores
  differently either way.

## Not verified against Clone Hero

These keep Hydra's current rule and are written down as unchecked (decision
D26). A live Clone Hero session could settle them.

- A generated fill's length, when two or more meter changes fall between two
  chords. Hydra's walk reads the meter one change late there.
- How many ticks make a beat in compound meters like 6/8. Hydra counts a
  beat as a quarter note (the chart's resolution) everywhere, including the
  4-beat fill deadline.
- Whether one authored fill turns off generated fills for the whole chart.
- Whether Clone Hero carries its search position from one fill to the
  next (its fill search takes a start index).

## Consequences

Lower-difficulty Pro Drums scores change on about 640 library charts each,
mostly up, because charters mark disco only on Expert. A few dozen other
charts move for the other rules. Every saved result and every saved
Dynamics count is redone once.

A future Clone Hero version can change any of these. The addresses above
are where to look first.
