# The store keeps summaries; the engine gives the details

Think of hydra.db as a library card catalogue. It used to hold a photocopy of
every book as well: every path, every squeeze row, every tempo map. Now it
keeps only the cards, one per chart and settings, with the score, stars and
best path on them. When you open a song, Hydra asks the engine for the book
again. The engine writes the same book every time, in a few milliseconds.

This was user decision D87 (2026-10-07). The readers moved off the stored
details first, in storage tasks T1 to T4. The tables themselves go, and an
older file shrinks, with storage-T5.

## What the audit found

The storage audit (docs/handoffs/2026-10-06-storage-audit-handoff.md) measured
a copy of the installed database. It took 338 MB on disk: 292 MB of database
and a 46 MB write-ahead log (WAL), SQLite's journal file, which never shrank.

| What | Size | Share |
|---|---|---|
| Path nodes (the paths table and its index) | 202 MB | 69% |
| Tempo maps, names and lengths (songmeta) | 38 MB | 13% |
| Path links (path_refs and two indexes) | 25 MB | 9% |
| Summary rows (results) | 14 MB | 5% |
| The chart library (charts) | 9 MB | 3% |
| Kick dynamics counts (dynamics) | 4 MB | 1.5% |

Almost all of it was a saved copy of engine output. The engine re-analyzes a
chart in 1.8 ms at the median, 12.8 ms at the 99th percentile and 294 ms at
worst (Nirvana's "Endless, Nameless Setlist"). A re-analysis of 74 charts,
the biggest included, matched the stored rows exactly, in every table.

ADR 0009's share-once scheme never shared anything. Each of the 91,196 path
nodes was referenced exactly once, and the scheme still cost about 37 MB.

## The decision

hydra.db keeps three things. Each is something the chart file can't give at
the moment it is needed.

**The chart library (charts).** Names, file paths, and the size and date pair
a rescan uses to skip unchanged files. The library shows and sorts these
without opening any chart. A scan writes it (`rebuild_chart_library`), and
`kChartMetaStamp` says whether a scan's rows are still current.

**One summary row per chart and settings (results).** The score, stars,
counts, hardest timing and best path text, under the settings key ADR 0009
set up. The library sorts and filters on these, and so do the leaderboard
comparison and the fill comparison. Getting them any other way means
analyzing the whole library.

**meta.** A few labels on the file itself, such as the fill rule a batch ran
under (`engine_mode`) and the scan's stamp (`chart_meta_version`).

Everything else comes from the engine's normal analysis (`analyze_chart`, run
through `app::analyze_chart_file`). Nothing is worked out a second way:

- A click analyzes the song on a worker thread. The paths, the tempo map, the
  song's length (`app::analysis_song_length`) and the Dynamics count
  (`count_dynamics`) all come from that one analysis.
- A click saves the song's summary row when it is missing, Stale or different
  (D87 item 2). So the library row and the Paths tab always show the same
  numbers.
- A setting changed with a song open analyzes it again under the new settings
  (D90 item 1).
- The path report analyzes every library chart that has a Ready row, on all
  cores, and reuses the charts a batch just analyzed (D87 item 5). A chart
  whose file can't be read is left out, and the page names it (D89 item 1).
- The leaderboard comparison page and `hydra_fillcompare` follow the path
  report's library rule (D92). With the stored song table gone, a chart's name
  comes from the chart library. A database with results and no library stops
  both with D89 item 2's sentence (`report::kNoChartLibrary`), and a result
  whose chart the library doesn't list is left off the page. The app's own
  database is unchanged, since every result there is a library chart. A
  database built by `hydra_batch` with folder arguments has no library.
- `hydra_replay dump` always analyzes the chart and reads no database.

A row is Ready when its results stamp is current (`kResultsStamp`) and its
`rules_fp` column matches the rules in force. `rank_row` reads the row's
`hyversion` and `rules_fp` columns. The SQL twin, `row_readable_sql`, checks
the results stamp alone. Until storage-T5 the check read the rules from the
head of the row's stored path blob, together with a path format stamp. Every
row that carries the current results stamp has path format 7, because format 7
came before that stamp. The older rows, in older formats, are already Stale by
their results stamp and stay Stale. So no row changes state when the check
moves to the columns. `rules_fp_of` is what is left of the blob read. It runs
only while an older file upgrades, to fill the `rules_fp` column of a file
that has none yet (ADR 0014).

Two smaller rules come with this. Rows of charts that left the library are
deleted at each scan (D87 item 4, `delete_results_without_chart`). A chart
whose file changed since the scan is re-hashed on click before anything is
saved under it (D87 item 3, `reidentify_chart`).

## The upgrade

It landed with storage-T5, and D100 (2026-10-07) changed how it works. The
first time the new Hydra opens an old file, it writes a fresh, small file with
only the kept tables and swaps it in for the old one. It runs only when the
old paths table exists, so a second open does nothing. No lasting backup copy
is left (D100 item 1, which replaces D87 item 7's in-place upgrade). The copy
lives in the `RecordStore` constructor in src/store/record_store.cpp.

Think of moving house. The first version threw things out of the old house
room by room and then shrank the house. The new one packs only the boxes
being kept, carries them to a new house, and hands back the old keys.

**Why a copy.** The first version dropped the paths, path_refs, songmeta and
dynamics tables inside the old 292 MB file, then ran VACUUM (SQLite's command
that rewrites a file at its new size). Task T0 timed that at 0.41 s on a
freshly made copy (below). On a cold disk, such as the first start after a
reboot, the same work took about 4.4 s when it was timed for D100, because
SQLite read the pages it was throwing away. The window sat frozen and
unpainted the whole time. The copy reads only the charts, results and meta
tables and never touches a page of the dropped ones. A prototype of it took
0.39 s on a cold disk. The merged code is timed at merge with the "upgrade timing" test case
in tests/test_store.cpp, and that number is the one to quote. The copy also
needs less free disk space: only the new file's size, where VACUUM needed up
to the old file's size.

**The steps.** Hydra writes the kept rows into a new file beside the old one,
hydra.db.upgrading. Each row is copied value for value, so it keeps its id
and its stamps (ADR 0018). The new file gets this build's tables and indexes.
Rows that have a score and no stars are left out of the copy. An older build
wrote them before the stars column existed; filling the stars needed the
details, and a click or a batch writes the row again. Once the copy is
complete and on disk, Hydra closes the old file and swaps the two names,
through a third name, hydra.db.old, which it then removes. Last, it opens the
new hydra.db as usual. The exact order, and the checks that stop the swap when
another program holds the file open, are in record_store.cpp.

**If it stops part-way.** A crash, a power cut or a closed window can stop the
upgrade at any moment. The steps are ordered so the disk then holds either the
whole old file or the whole new one, under a name the next start recognizes.
Before it opens anything, the next start looks at which of the three names
exist and finishes or reverses the swap. That rule lives in
src/store/upgrade_files.cpp, which only moves files and never reads inside a
database. When hydra.db exists it is always whole, so any leftover is thrown
away. Closing the window during the copy stops it at the next row and removes
the new file; hydra.db is untouched and the next start begins again (D100
item 5).

Why two renames and not one "replace" rename: on NTFS a replace is a single
step, but on exFAT or FAT32 Windows does it as a delete and then a rename. A
crash between the two would lose hydra.db. Two renames are safe on every file
system, because the next start knows the one moment when hydra.db is briefly
missing.

**What the user sees.** The open runs on a worker thread (`StoreOpenJob` in
src/ui/library_jobs.cpp), so the window keeps drawing. A fast open shows an
empty window. An open still running after 0.15 s shows the startup screen,
with a bar that counts rows copied out of rows to copy, both counted from the
file itself (D100 items 2 to 4 and 7). The command-line tools have no window to
freeze. They still open the file before doing anything else, and get the
faster upgrade too.

**If it fails.** A failure before the swap leaves hydra.db as it was. Hydra
shows the startup message box (D72 item 1) with the library-file sentence for
`ErrorKind::DatabaseUpgrade` from src/app/user_messages.cpp over the raw
error, then closes (D100 item 6). A failure to open the new file after the
swap reads like any failed open (D72 item 2).

Task T0 measured the first, in-place version on a copy of the real database
(docs/handoffs/2026-10-07-storage-measurements.md). The file went from
292,372,480 bytes to 13,033,472 bytes, about 13.0 MB.

Every open also sets SQLite's journal_size_limit (`kJournalSizeLimitBytes`,
4 MB; user decision D93). SQLite then cuts the log back to that size after
each full checkpoint, so it never sits at 46 MB again. Firefox ships the same
limit for the same reason (Mozilla bug 1820478).

## Rejected

**Compressing the blobs.** zlib would have taken the path nodes from 145 MB
to about 49 MB, and zstd with a trained dictionary to about 37 MB. It is
still a stored copy of engine output, with a codec and a stamp to keep in step
with the engine.

**The trimmed codec.** A tighter number encoding plus dropping the dead and
duplicated fields would have brought the nodes to about 72 MB. Every field
left is still a second copy of a fact the engine owns, read by a decoder that
can drift from it.

**Share-once with integer ids.** Raw 16-byte hashes and integer ids would have
cut the scheme's 37 MB to about 6 MB. But the scheme never shared a single
path, so the right size for it is zero.

SQLite itself stays. In the one side-by-side size test the audit found, only
RocksDB came out smaller, and only because it compresses by default.

## What this costs

Opening a song runs the analysis: about 2 ms for a typical chart and about
0.3 s for the slowest. A chart still analyzing after 0.15 s shows the
"Analyzing chart..." box with its Cancel button (D87 item 6).

A chart whose file is missing can't show its paths. Its library row and
summary stay.

The path report analyzes the library each time it is built. T0 estimated
about 2.4 s for the whole library, on the 8 workers a batch uses on a
16-core machine.

An older Hydra that opens an upgraded file finds no path details in it.

Several older records say a fact is "stored by the engine": ADRs 0011, 0013,
0014 and 0021. That now means the engine stamps the fact on the record when it
analyzes. The record lives in memory and is never saved.

## What this supersedes

Each record below carries a "Superseded in part" line that points here. Their
text is otherwise left as it was written.

ADR 0009 keeps its keying by settings, but its paths stored once are gone.
ADR 0014's rules fingerprint lives in the `rules_fp` column, not in a blob.
ADR 0017's and ADR 0022's statements about the stored layout are history.
ADR 0018 loses four stamps: the path format stamp, the two dynamics stamps and
the song length stamp. ADR 0021's history and transfer scales are stamped by
the engine at analysis and never stored.
