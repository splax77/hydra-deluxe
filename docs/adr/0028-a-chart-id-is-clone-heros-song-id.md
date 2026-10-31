# A chart's id is Clone Hero's song id

Hydra keys every chart, and every saved result, by one id: the hyhash. For a
folder chart that was always the MD5 of its notes file (MD5 is a 32-digit
fingerprint of a file's bytes), which is also the id Clone Hero gives the song.
For a `.sng` or `.srb` it used to be the MD5 of the whole container. Clone Hero never uses that number, so the dmleaderboards
comparison could never match a `.sng` or `.srb` song. The user's decision
(2026-10-09) made the hyhash Clone Hero's own song id for every kind of chart.

## The rule

- A folder chart's id is the MD5 of its notes file. This is unchanged.
- A `.sng`'s id is the MD5 of the notes file inside it, after unmasking (a
  `.sng` stores each file scrambled with a simple XOR mask). The same
  `sng_read_notes` that the note loader uses picks that file, so the id and
  the notes always come from one file.
- An `.srb`'s id is the 16-byte checksum its metadata stores, right after the
  song length. Hydra reads it and never recomputes it.

`song_id_read` in src/app/analysis.cpp is the one owner of this rule. The
library scan and `hash_chart_file` both call it.

## The evidence

The song-cache checks behind this rule, run on 2026-10-09 and 2026-10-10 and
partly re-run on 2026-10-30, now live in docs/clone-hero/01-reading-a-chart.md
(CR-10 to CR-12), which also says why an `.srb`'s id is read, not recomputed.
This section's full earlier text is this file at commit 5fbe71ae.

dmleaderboards uses the same ids. A one-off check on 2026-10-10 scanned copies
of No Known Suspects (`c87b09d0…`) and Galaxies in Harmony (`416daca3…`) into
a scratch database, then ran `collect_dm_rows` against two real players' live
scores. Both songs joined. The check used a temporary test that was not kept.

## What it cost

The scan's stamp, `kChartMetaStamp` in src/store/stored_versions.h, went from 2
to 3. So the next scan reads every chart file once more. Every `.sng` and
`.srb` song gets a new id, and the scan drops their saved results, because no
chart has the old ids any more. Those songs read "Not analyzed" until they are
analyzed again. The user chose this over carrying the old results across to the
new ids. Results for folder charts are kept.

Two smaller changes come with it:

- A `.sng` with no notes file has no id. The scan now lists it as an error.
  Before, it showed up as a library row that failed when it was clicked.
- An `.srb` whose metadata ends before its checksum has no id either, and the
  scan now lists it as an error too. Before, it was a normal library row that
  loaded fine, because nothing read the checksum.
- A `.sng` and an unpacked folder copy of the same notes file now share one
  id. Hydra shows them as one chart with two copies.

Scanning a `.sng` also reads less. The scan reads only its header and its notes
file, not the audio around them. Nobody has timed the difference.

## Rejected

**A second stored id.** Hydra could have kept the whole-file MD5 and stored
Clone Hero's id beside it. The dm join, the comparison's row click and its "not
in library" check would each have had to translate between the two ids. So
would any later join on Clone Hero's id, such as reading Clone Hero's own
`scoredata.bin`. That would mean one chart with two ids, and every join on
Clone Hero's id doing the translation again.
