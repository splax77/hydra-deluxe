# A chart's id is Clone Hero's song id

Hydra keys every chart, and every saved result, by one id: the hyhash. For a
folder chart that was always the MD5 of its notes file, which is also the id
Clone Hero gives the song. For a `.sng` or `.srb` it used to be the MD5 of the
whole container. Clone Hero never uses that number, so the dmleaderboards
comparison could never match a `.sng` or `.srb` song. The user's decision
(2026-10-09) made the hyhash Clone Hero's own song id for every kind of chart.

## The rule

- A folder chart's id is the MD5 of its notes file. This is unchanged.
- A `.sng`'s id is the MD5 of the notes file inside it, after unmasking. The
  same `sng_read_notes` that the note loader uses picks that file, so the id
  and the notes always come from one file.
- An `.srb`'s id is the 16-byte checksum its metadata stores, right after the
  song length. Hydra reads it and never recomputes it.

`song_id_read` in src/app/analysis.cpp is the one owner of this rule. The
library scan and `hash_chart_file` both call it.

## The evidence

Clone Hero writes every song's id into its song cache
(`AppData\LocalLow\srylain Inc_\Clone Hero\songcache.bin`). On 2026-10-09 that
cache held 54 `.sng` and 30 `.srb` songs.

- For all 54 `.sng` songs, the cached id was the MD5 of the notes file inside.
  The whole-file MD5 matched none of them.
- For all 30 `.srb` songs, the cached id was the stored checksum. The
  whole-file MD5 matched none of them.
- The stored checksum equals the MD5 of the notes stream in 26 of the 30. The
  other four (Good Grief Retreat, No Known Suspects, Stigma and Troopers of
  the Stars) differ. That is why an `.srb`'s id is read, not recomputed.

dmleaderboards uses the same ids. With the new rule, both No Known Suspects
(`c87b09d0…`) and Galaxies in Harmony (`416daca3…`) joined real players'
scores in `collect_dm_rows`.

## What it cost

The scan's stamp, `kChartMetaStamp` in src/store/stored_versions.h, went from 2
to 3. So the next scan reads every chart file once more. Every `.sng` and
`.srb` song gets a new id, and the scan drops their saved results, because no
chart has the old ids any more. Those songs read "Not analyzed" until they are
analyzed again. The user chose this over carrying the old results across to the
new ids. Results for folder charts are kept.

Two smaller changes come with it:

- A `.sng` with no notes file, or an `.srb` whose metadata ends before its
  checksum, has no id. The scan now lists it as an error. Before, it showed up
  as a library row that failed when it was clicked.
- A `.sng` and an unpacked folder copy of the same notes file now share one
  id. They show as one chart with two copies, as Clone Hero sees them.

Scanning a `.sng` also got faster. The scan reads only its header and its notes
file, not the audio around them.

## Rejected

**A second stored id.** Hydra could have kept the whole-file MD5 and stored
Clone Hero's id beside it. The dm join, the comparison's row click and its "not
in library" check would each have had to translate between the two ids. So
would any later join on Clone Hero's id, such as reading Clone Hero's own
`scoredata.bin`. That would mean one chart with two ids, and every join on
Clone Hero's id doing the translation again.
