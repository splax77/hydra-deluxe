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

Clone Hero writes every song's id into its song cache
(`AppData\LocalLow\srylain Inc_\Clone Hero\songcache.bin`). The checks below
were run on 2026-10-09 and 2026-10-10. Each one looks for a chart's 16 id bytes
anywhere in the cache, not in a decoded id field.

- The cache listed 54 `.sng` songs. For all 54, the MD5 of the notes file
  inside was in the cache. The whole-file MD5 was in it for none of them.
- The game ships 30 `.srb` files. For all 30, the stored checksum was in the
  cache. The whole-file MD5 was in it for none of them.
- For a sample of 300 folder charts (the first 300 the probe met when walking
  the user's song folder), the MD5 of the notes file was in the cache for all
  300.
- The stored checksum equals the MD5 of the notes stream in 26 of the 30. The
  other four (Good Grief Retreat, No Known Suspects, Stigma and Troopers of
  the Stars) differ. That is why an `.srb`'s id is read, not recomputed.

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
