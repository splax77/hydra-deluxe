# The store keeps one record per chart and profile, with its top paths

> **Supersedes [ADR 0026](0026-the-store-keeps-summaries-the-engine-gives-details.md)
> in part, 2026-10-15 (D115).** ADR 0026 still holds for what the engine
> gives: a click analyzes its song again, and the full analysis lives in
> memory only. What changed is what hydra.db keeps of an analysis, how the
> path report gets its rows, and what happens to an older library file.

Think of ADR 0026's hydra.db as a card catalogue with one card per book. Each
card spelled out the whole shelf address on it: the chart mode, the SP cap,
five lens numbers, the version and the rules. Now each shelf address is
printed once, on a shelf label (a profile), and each card names its shelf by
a small number. The card also carries the book's best five pages (the top
paths), so the path report can read the cards instead of rereading every book.

The design is `docs/superpowers/specs/2026-10-11-one-record-per-chart-and-profile-design.md`,
and the build followed `docs/superpowers/plans/2026-10-12-one-record-per-chart.md`.
The user's decisions are D115 in `docs/audit/2026-10-03-fix-decisions.md`.

## Why

The path report was the one slow thing Hydra did. On 2026-10-11 the main
session profiled the real app on the user's library
(`docs/handoffs/2026-10-11-path-report-speed-handoff.md`). Building the report
took 41.6 of the session's 62 CPU-seconds and about 17 s of wall time. It ran
37,691 analyses of 18,907 chart files, because ADR 0026 kept only summaries
and the report analyzed every chart again to get its top 5 paths.

The word "record" also had four meanings, and the report's building bar had
just drifted from its header (D103 item 25). The user asked for one record per
chart and for each of these questions to be answered in one place.

## The decision

hydra.db keeps four tables, plus `meta`.

**`charts`**, the chart library, is unchanged from ADR 0026. Each row is one
library row: one copy of a chart in one folder.

**`profiles`** holds one row per set of settings a record is filed under: the
chart mode, the SP cap, the five lens numbers (ms limit on and value, score
range unit and number, fill rule), the results stamp (ADR 0018) and the rules
fingerprint (ADR 0014). `store::Profile` in src/store/record_store.h is the
part the app chooses. The store adds the stamp and the fingerprint itself when
it files or looks up a record. There is no limit on profiles (user decision,
2026-10-11).

**`records`** holds one row per chart and profile, keyed by the chart's song
id (ADR 0028) and the profile's id: the path count, the best path's star
count and the chart's note count. `RecordRow` is its shape.

**`record_paths`** holds the record's kept paths, ranked from 1, at most
`kPathsPerRecord` of them: the path text, score, activation count, highest
skip, hardest ms, hardest early fill, average multiplier, squeezes in and out,
and the optimal flag. `PathRow` is its shape. They are plain columns, with no
blob and no codec, so any SQLite tool reads them and no decoder can drift from
the engine. That was ADR 0026's objection to stored paths. Rank 1 is the
summary the library sorts and filters on, so the summary is stored once.

One owner answers each question:

- `prepare_record` is the one place an analysis becomes a record. It walks
  `HydraRecord::all_paths`, ranks from 1 and checks the profile against the
  settings the analysis ran under.
- `RecordStore::save` is the one write, used by the click, the batch,
  `hydra_batch` and `tools/bench.cpp`. In one transaction it replaces the
  chart's record under its profile, deletes the chart's records under every
  profile whose results stamp this build cannot read, and deletes any profile
  left with no records. A record made under other rules but a readable stamp
  stays, because it reads Ready again if the rules come back (D51 call 8).
- `RecordStore::lookup` and `RecordStore::lookup_one` answer Ready, Stale or
  Not analyzed (below). The library's chips, the batch's skip list
  (`RecordStore::ready_song_ids`), the report and both comparison pages
  (`RecordStore::ready_records`) read through them.
- `Settings::profile` is the one builder of the current profile. The
  leaderboard comparison asks for `Settings::profile_at_clone_hero_cap` and
  the fill comparison for `Settings::profile_with_fills`. Nobody edits a cap
  or a lens field by hand.
- `ChartIndex` (src/store/chart_index.h) answers which charts the library
  lists, how many copies each has and which copy names it.
  `ChartIndex::count` is the one count of charts, and it counts library rows.

## Ready, Stale and Not analyzed

A chart is Ready when it has a record under the current profile: the chart
mode, cap and lens match the settings, the results stamp is one this build
accepts, and the rules fingerprint is the rules in force. It is Stale when it
has no Ready record but has one under a sibling profile, the same mode, cap
and lens with another stamp or other rules; `RecordLookup` says which. It is
Not analyzed otherwise. A record under another mode, cap or lens never makes a
chart Stale. Switching back to settings analyzed before shows those records
Ready at once.

## The path report reads stored paths

The path report is one database read of the current profile's Ready records,
kept to the charts the library lists (`report::listed_records`). It analyzes
nothing and opens no chart file. It lists the current profile only, so its
Mode column and the row click's mode switch are gone, and a chart mode switch
marks an open report out of date (user decisions, 2026-10-12). A click that
saves a record marks both open reports out of date, as a finished batch does.

A record keeps at most `kPathsPerRecord` (5) paths, so the report never shows
more than that per chart.

## Older library files

ADR 0026's upgrade copied an old file's kept tables into a new file and swapped
the two. That copy is deleted. The first open by this build now sorts a file
into one of five kinds with `classify_library_file` (src/store/library_file.h):
Fresh, Current, Convertible, TooOld or Unreadable.

**From 2.2.0 to 2.7.0 (Convertible).** The file is converted in place, in one
transaction, by `RecordStore::convert_results`. Each row of the old `results`
table becomes a record under its profile, whatever its results stamp, and its
old summary columns become the record's rank-1 path. Song ids are lowercased.
A row with a score and no stars is left behind, as ADR 0026's copy left it.
The old `results` table and the file-wide fill-rule label (`engine_mode`) are
deleted. The startup screen counts the rows converted.

Every converted record reads Stale. This change bumped the results stamp to
2.8.0 (generated fills, Star Power's length and the beat grid moved; ADR 0010's
note of 2026-10-15), and the converted records carry the old stamp. So after
the update the library reads Stale everywhere, as after any stamp bump, until
the user analyzes again (user decision, 2026-10-12, question 1). A converted
record never needs its paths filled in, because it can never read Ready: the
report lists Ready records only, and every Ready record was written whole by a
click or a batch.

**From before 2.2.0 (TooOld).** The file is not converted. Each open names
what it wants with `OldFilePolicy`; there is no default. The app sets the
file aside: `set_aside_old_library` (src/store/upgrade_files.cpp) renames
hydra.db and its side files to hydra.db.pre-2.2.0, replacing an earlier
set-aside file only after the move has worked. The app then starts a fresh
library, shows the notice once and scans. The command-line tools refuse the
file and leave it alone. Both sentences are in src/app/user_messages.cpp, and
the cut-off is `kOldestConvertibleVersion` (user decisions, 2026-10-14).

## What this costs

The library file grows by the stored paths. The spec estimated about 150 to
200 bytes per record and 3 to 4 MB per profile across the user's 18,848
analyzed charts (spec section 3: the stored best path averaged 12 characters
and a record kept 4.08 paths on the user's file). That is an estimate, not a
measurement of the shipped layout.

Every saved result reads Stale once after the update, and the whole library
must be analyzed again before the report shows anything.

A library file from before 2.2.0 loses its results, and its library is
scanned again. The user chose to cut those files off (2026-10-14).

An older Hydra that opens a converted file finds no `results` table in it.

## What this supersedes

ADR 0026 carries a "Superseded in part" note that points here, and a dated
note on its "The upgrade" section. Its text is otherwise left as written. Its
keeping of summaries only, its copy-and-swap upgrade, its file-wide fill-rule
label in `meta` and its "the path report analyzes the library each time"
are history. Its rule that the engine gives the details stands.

ADR 0010's file-wide fill-rule label is retired; its dated note of 2026-10-15
says so.

## Rejected

**Paths as JSON in the records row.** A separate table was chosen (user
decision, 2026-10-11): plain columns any SQLite tool reads, and no encoder or
decoder to keep in step with the engine.

**Filling a converted record's paths when the report opens.** The spec first
kept the old rows Ready and had the report analyze each converted record once
to fill its paths. The stamp bump made every converted record Stale, so that
pass could never run on real data. It was dropped, with its "Saving paths"
bar (user decision, 2026-10-12).
