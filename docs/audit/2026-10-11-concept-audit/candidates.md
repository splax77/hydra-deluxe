# Phase 4: merged candidates

The main session merged the 130 candidate blocks from `groups/*.md`, `inventory-C1.md` and `inventory-C2.md` into 84 ideas. Each line is one idea with two or more homes. "From" names every scout block that raised it, including the "also seen" blocks; the verifier reads all of them before reading the code. No candidate here is graded yet. The kind each scout guessed is in its block, not here, so a verifier forms its own view.

The ids are M01 to M84 ("merged"), so they don't clash with the scouts' own ids. Candidates are batched by the files they read, one batch per verifier.

## V1: the reports and their tables

- **M01.** Does Hydra have a result for this chart, and which status word does each page show? From G1a-1, G1b-4, C1-1, C2-2.
- **M02.** How many spellings does one state have ("not in library", "in both" and the rest)? From G1a-2.
- **M03.** How is a text column sorted, and where do ties and missing values go? From G1a-3, G1b-1, G1b-2, C1-2 (sort part), C2-7.
- **M04.** What does the search box match, and what must the user type? From G1b-3, C1-2 (search part).
- **M05.** How is a "shown of total" count line written, and what does it count? From G1a-4, G1b-5.
- **M06.** The small view helpers (`text_column`, `ms_key` against `optional_key`) copied between the two window views. From G1a-5.
- **M07.** The report table written again in the fill page's JavaScript, and the words and colours it shares with the windows. From G1b-6, G1a-8, G11b-4.
- **M08.** "Keep listed records whose chart the library lists": the SQL join against the in-memory `library_lists` test. From G1a-7, G10-3.
- **M09.** What a report window remembers between frames and forgets on close, and who closes it. From G1b-7, C2-8, G11a-8, and also-seen G1b-7 in G11a.
- **M10.** What "Clear filters" resets, and what "no filter" means. From G1b-8.

## V2: which settings matter, and how they are named

- **M11.** Which settings does each report page read, and which make an open report out of date (`settings_change_touches`, `hit_window_ms`)? From G1a-6, G3-3, and also-seen G3-3 in G11a.
- **M12.** Which settings pick a stored result: the chart mode, cap and lens as three loose parts. From G3-1, G2-6, G10-5, and also-seen G3-1 in G4b and G11a.
- **M13.** The ten Analysis panel settings spelled by hand in six or more places, including `settings_at_defaults`. From G3-4, and also-seen G3-4 in G11b.
- **M14.** What 0 and 1 mean for the score-range unit, kept as an enum and two ints. From G3-5, and also-seen G3-5 in G4b and G11b.
- **M15.** What each analysis setting is called in words, on each surface (`describe_settings` and its rivals). From G3-6, C1-3, and also-seen G3-6 in G11b.
- **M16.** Which of a record's key parts the store checks against the record. From G3-7.
- **M17.** Can this chart mode be compared with the leaderboard, and is this SP cap Clone Hero's? (toolbar passes the real difficulty, `collect_dm_rows` passes Expert; check whether Refresh or Try again reaches it on a Hard mode). From G3-2, G11b-3, and also-seen G3-2 in G11b.
- **M18.** Which search settings change a stored analysis (`settings_key` against `RecordKey`)? From G4b-4.
- **M19.** Which settings make a Preview "another song", which only a new overlay, and which reading of a chart is asked for (does a rules-only change rebuild)? From G6a-3, C2-5, and also-seen G6a-3 in G6b and G11a.
- **M20.** Which columns make one result row's identity, and where each is spelled. From G10-4.

## V3: a chart's identity, the library and the store

- **M21.** The shapes of one scanned chart (`ScanItem`, `ChartLibraryEntry`, `ChartCacheEntry`, `ChartMeta`) and the `charts` table readers. From G2-1, G2-2, G2-3, G10-2, C2-1.
- **M22.** Why a result is Stale: `LibraryRow` copies `SummaryLookup`, the two flags in three types, and the hand-written "did it change" test. From G1b-9, G10-6.
- **M23.** Which metadata key means which field for .ini, .sng and .srb, and how values are trimmed. From G2-4, G9-8, and also-seen G2-4 in G7b.
- **M24.** What counts a batch reports, and who words them. From G2-5, and also-seen G2-5 in G11a and G11b.
- **M25.** What a CLI tool does to start, against the app. From G2-7.
- **M26.** Scan the folders against the last scan's cache, then save as the library: written in the CLI and in `ScanJob::run`. From G10-1.
- **M27.** The fill rule a result ran under, stored twice (known, partly fixed). From G10-7.
- **M28.** "Begin, do the work, commit, roll back if still open" written five times. From G10-8.
- **M29.** The chart open in the song panel: a by-value copy patched by hand (stale after a rescan?). From G11a-9.

## V4: scoring and the search engine

- **M30.** Which six score categories a path has, and how many places spell them out. From G4a-1, C2-3, and also-seen G4a-1 in G4b.
- **M31.** How a chord's chart base score relates to the stored "base" category. From G4a-2.
- **M32.** What an accent or ghost earns. From G4a-3.
- **M33.** Is this chord after the SP end? (`paid_by_sp_walk`, `after_sp_end`, the engine's second call). From G4a-4, G4b-1, and also-seen G4a-4 in G4b.
- **M34.** Which chart rows fall inside an SP end's squeeze window. From G4b-2.
- **M35.** How the search graph is built for a set of settings (three builders). From G4b-3.
- **M36.** How many paths are tied under a root. From G4b-5.
- **M37.** Is every timing of an activation inside the ms limit? From G4b-6.
- **M38.** The fill-deadline "4 beats" rule computed in code and restated in a user sentence. From G4b-7.

## V5: paths and squeezes on screen

- **M39.** Who words the Rating cell, what "(uncounted)" means, and which word rates a squeeze ("Hard", "Insane" with different edges). From G5-1, C1-4.
- **M40.** How one squeeze or fill timing is written for the user. From G5-2.
- **M41.** What "free" means for a squeeze. From G5-3.
- **M42.** What each path group is called on screen. From G5-4, C1-5.
- **M43.** Which kind of squeeze this is: the engine's SQ_IN and SQ_OUT ints against `SqueezeKind`. From G5-5, and also-seen G5-5 in G4b.
- **M44.** How an activation's deactivation tick is read. From G5-6.
- **M45.** Does squeezing out this row cost points? From G5-7.
- **M46.** The " · " separator between facts on a line. From G5-8.

## V6: the Preview

- **M47.** Where the playhead can be, and what the screen shows outside that (the clamps). From G6a-1, G8-7, and also-seen G6a-1 in G8.
- **M48.** Is Star Power running at this moment? (G6b found the tables agree; confirm). From G6a-2, and also-seen G6a-2 in G6b.
- **M49.** When does the last note start? From G6a-4.
- **M50.** What "the end of the song" is, and which end each label shows. From G6a-5, and also-seen G6a-5 in G8.
- **M51.** Is this length one the timeline can use? From G6a-6.
- **M52.** Does the drawn path have activations? From G6a-7.
- **M53.** Where each text box sits on the Preview image. From G6a-8, and also-seen G6a-8 in G6b.
- **M54.** Has the chart file changed since its record was analyzed? From G6b-1.
- **M55.** Two ways to assemble a Preview scene and its highway timeline. From G6b-2.
- **M56.** Is `PreviewNote::solo` a second home of "is this note in a solo", and is it read at all? From G6b-3.

## V7: parsing and chart files

- **M57.** The order of a drum pad's lane bits. From G7a-1.
- **M58.** When is a drum note a cymbal, and what is one drum note to each layer? From G7a-2, C1-7, and also-seen G7a-2 in G6b.
- **M59.** What happens to one tick's events when one is malformed, and where the 2x bass step sits. From G7a-3, and also-seen G7a-3 in G7b.
- **M60.** What the library shows for a song with no title, artist or charter. From G7a-4.
- **M61.** Which tempo and meter section is in force at a tick (`check_activations`' own walk, the Preview's time-signature label). From G7a-5, G7b-5, and also-seen G7a-5 in G7b.
- **M62.** How Hydra decides a chart's notes format. From G7b-1.
- **M63.** Is this path a container (.sng or .srb) or a plain chart file? From G7b-2, C1-8, C2-4.
- **M64.** Is this entry a playable, usable song stem (by name, by bytes, channels and rate)? From G7b-3, G8-5.
- **M65.** Do n bytes fit at this position in the buffer? From G7b-4.
- **M66.** How a song file is opened for reading, the share mode, and what the user sees when it can't be. From G2-8, G9-3, and also-seen G2-8 in G7b, G8 and G9.

## V8: audio, ini files and file helpers

- **M67.** The readers' cursor bookkeeping and the 4096-frame block. From G8-1, G8-8.
- **M68.** How many frames a stem has, and who decides. From G8-2.
- **M69.** What a decode error does, and who finds out. From G8-3.
- **M70.** Getting to a frame by decoding forward and discarding. From G8-4.
- **M71.** The mixing rule written twice (`mix_stems` and `StreamMix`), and the plain `runtime_error` throws. From G8-6.
- **M72.** What the user reads when hydra_rules.ini is bad. From G9-1.
- **M73.** What text counts as a number in a hand-edited ini file (does "nan" pass `check_min`?). From G9-2.
- **M74.** How a file is written so a reader never sees half of it. From G9-6.
- **M75.** Path-string helpers: where a path is cut at its last separator. From G9-7.

## V9: background jobs and the app's messages

- **M76.** The progress, elapsed-time and time-left machinery the jobs hand-roll. From G6b-4, G11a-5, C1-6.
- **M77.** Is this job running or finished, and how does it become Ready, Cancelled or Failed? From G11a-1, G11a-2, C2-6.
- **M78.** What Cancel does to a report build, and what the slot says afterwards. From G11a-3.
- **M79.** How "this job was cancelled" is thrown and worded. From G9-5.
- **M80.** May a batch start or an analysis start now, and how many charts the Analyze button covers. From G11a-4, G11b-1.
- **M81.** How "a batch is running" is worded. From G11a-6, G11b-7.
- **M82.** How "analysis is off" is worded. From G11a-7.
- **M83.** Where user-facing failure sentences live, and how the path report's outcome is worded (strip, status line, window). From G9-4, G11a-10, G11b-2, G11b-6, and the "path report is ready" lead in G11a.
- **M84.** How a modal is named, and who opens it. From G11b-5.
