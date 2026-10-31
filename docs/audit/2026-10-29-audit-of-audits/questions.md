# Questions from the reopened audit closures

These are the questions for the user from the audit of the past audits. The Backend timings rating questions (M39, 306, 33, finding 34, and the 0 ms against leeway split) are not here; they went to the user first, with the design in `blocks/AOA-R2-design.md`. A drafting agent (aadb3cfe748cef780) checked every line below against the code at 1fc55723 rather than taking it from the scouts' blocks, and the main session edited Q3, Q4 and Q9 to take out reasons nobody had written down. Where a split came from an agent's choice and no ruling names both sides, I say so. Nobody chose any of these splits in a question that showed both sides; that is why each one is here. Your standing rule from 2026-10-29 is "yes everything should use one rule", and the recommendations follow it unless I say why not.

The groups: settings (4 questions), charts and song files (3), search and lists (1; Q9 was withdrawn in review, see "Dropped"), wording and small behaviour (5), numbers confirmed unseen (1), rare cases (1), hooks (2). Seventeen in all, numbered Q1 to Q18 with no Q9. Dropped items are at the end.

---

## Group 1: settings that take out-of-range values

What this group is about: a number typed in a box, written in `hydra_settings.ini` or `hydra_rules.ini`, or given on the command line, lands in different places depending on which door it came through.

### Q1. What should an SP cap below 1 become: 4, 1, or refused?

Three doors, three answers. In the settings file, `sp_cap=0` or junk becomes the default 4 (`src/app/config.cpp:203`: `number("sp_cap", &Settings::sp_cap, 1, kNoCeiling, Outside::DefaultFromFile)`; `pull_into_range` at :226-229 sends a below-floor file value to `defaults()`). In the Settings box, typing 0 becomes 1 (`src/ui/settings_bar.cpp:135`: `Settings::clamp(&Settings::sp_cap, cap)`). On the command line, `hydra_replay --cap 0` is refused (`tools/replay.cpp:170-174`: `if (in_range != cap) throw`).

Examples. `sp_cap=0` in the file: the app runs at 4 bars. Type 0 in the box: 1 bar, which never activates Star Power (the Paths tab says so). `sp_cap=-3` in the file: 4 bars; -3 in the box: 1 bar. `hydra_replay --cap 0`: an error. `sp_cap=5` is 5 bars through every door.

Where it came from: the file rule is the old "Auto" value kept as 4 after you dropped Auto (ui-redesign decision 6 said "leave the default as 4"); the box rule came from Q14's "nearest edge". Two of the three doors do have a record, written by the main session after your "go with recommended answers to everything" on 2026-10-04 and never shown to you as a question: the D51 addendum (fix-decisions.md:199) says "0, junk and `auto` still read as 4", and its code-only list (:201) says "`--cap 0` still refuses on the command line". No question ever showed you the three side by side. Q14 (D51) said every number setting is "pulled to the nearest edge ... when the file loads, and when you type it", which the file does not do for this key.

Options: (a) nearest edge everywhere, so 0 reads 1 through every door; (b) default 4 everywhere; (c) refuse everywhere (the file would need an error line); (d) keep the three as they are.

Recommendation: (a), one rule, and it is the rule D51 Q14 already states. Strongest case against: an old INI with `sp_cap=0` (the old Auto) would silently run at a 1-bar cap and show no activations; (b) avoids that trap, at the cost of the box and the file disagreeing with D51's words.

### Q2. When a setting is out of range in the file, nearest edge or the default?

D51 Q14 ruled "pulled to the nearest edge". The key table does that for most keys (`Outside::NearestEdge` is the default at `src/app/config.cpp:148`) but two keys go to the default instead: `number("depth_mode", ..., 0, 1, Outside::Default)` (:192) and `number("hit_window_ms", ..., 1.0, kNoCeiling, Outside::Default)` (:199). `pull_into_range` (:226-229) returns `defaults()` for those.

Examples. `mslimit_value=900` reads 500 (the edge, as ruled). `hit_window_ms=0.5` reads 85 (the default), where the ruling gives 1. `depth_mode=7` reads 0 (the default), where the ruling gives 1. The hit window has no Settings box, so "the range its box enforces" never applied to it; the D51 addendum (fix-decisions.md:198) said depth_mode out of range reads as the default, but that line was written after the batch answer, not asked. One more ruling touches the hit window: D67 (fix-decisions.md:279) says a value that is not a clean number, such as `90abc`, falls back to the default 85. Junk reads as 0 before the range is applied (`config.cpp:242-244`), so under option (a) `90abc` would read 1, not 85; if you choose (a), say whether D67 still holds for junk.

Options: (a) nearest edge for every number key, as D51 Q14 says; (b) default for every number key; (c) keep the two exceptions.

Recommendation: (a). Strongest case against: a hit window of 1 ms, or a depth_mode pulled to 1, is a strange setting nobody wants, and the default is the safer landing; but then the ruling's words should change to say "default", not "nearest edge", and apply everywhere.

### Q3. Keep the five lower bounds in hydra_rules.ini as they are?

One loader, one table, but the bounds differ: `backend_leeway_ms` 0 or more, `max_tied_paths` 1 or more, `fill_cooldown_measures` 1 or more, `fill_max_distance_beats` 0 or more, `fill_land_slop_beats` 0 or more (`src/core/rules.cpp:76-81`). A value under its bound stops Hydra: `if (d < f.min) bad(path, f.name, v, "in range")` (`src/app/rules_file.cpp:39`).

Examples. `max_tied_paths=0` refuses to start. `backend_leeway_ms=0` loads (a backend note at exactly +0.0 ms still counts, one at +0.1 ms does not). `fill_cooldown_measures=0` refuses; `fill_land_slop_beats=0` loads.

Where it came from: the phase 7 question showed you one example ("max_tied_paths must be at least 1") and said "the User Guide will list all six", and the guide did not exist yet. You never saw the other four or the 0-versus-1 difference. The bounds were set by the executors who added each key.

Options: (a) keep all five as listed; (b) change any of them (say which).

Recommendation: (a), keep them, unless one looks wrong to you. These are five numbers nobody chose, one per key, set by the agents who added each key; you are seeing them together for the first time. Strongest case against: one rule ("0 or more everywhere, and 0 means off") is possible, but then `max_tied_paths=0` and `fill_cooldown_measures=0` need a meaning you would have to pick.

### Q4. A bad line in hydra_settings.ini is skipped; the same line in hydra_rules.ini stops Hydra. One rule?

Both files share the comment rule now (ruled, D51 Q15). Two differences remain. A line with no `=`: settings skips it (`src/app/config.cpp:262-263`, "A blank, comment-only or "="-less line is skipped."); rules throws `is not key = value` (`src/app/rules_file.cpp:91`). A whole-number key set to `abc`: settings reads 0 and then pulls it into range (`config.cpp:239-240`: `std::atoi(...)`, "Junk reads as 0"); rules refuses (`rules_file.cpp:52-54`: "a whole number").

Examples. `view_difficulty=Hard # practice`: Hard in both (ruled). A stray line `junk`: silently skipped in settings, an error in rules. `sp_cap=abc`: becomes 0 and then 4 in settings; `max_tied_paths=abc` stops Hydra with its file name and key.

Where it came from: the settings file kept `atoi` from the first version; the rules file was written later with a strict loader (D114 Q8 asked only about the rules file, and its text called the settings side "by design", which was an agent's phrase, not yours).

Options: (a) both refuse with a plain error naming the file, key and value; (b) both skip bad lines quietly; (c) keep the split.

Recommendation: (a). A typo in a hand-edited file should say so once rather than silently run at a different cap. Strongest case against: the settings file is also written by Hydra itself and an older or newer version could leave a line the other does not know; today an unknown key is skipped (`config.cpp:265`), and that part should stay skip-only.

---

## Group 2: how Hydra reads charts and song files

What this group is about: two containers and two audio paths decide "what is this file" by different tests, and one fill rule has no source.

### Q5. A notes file with an odd name: .srb sniffs its bytes, .sng gives up. One rule?

In an `.sng`, only the exact names count: `if (low == "notes.mid") ... if (low == "notes.chart") ... return ChartFormat::None` (`src/parse/chart_files.cpp:21-26`), and with no match the load fails "No chart files found in SNG file." (`src/parse/sng.cpp:157`). In an `.srb`, an odd name falls through to the bytes: `else  // Unexpected filename: sniff the payload instead. is_mid = is_midi_header(...)` (`src/parse/song.cpp:2011-2012`).

Examples. `notes.mid` loads in both. No notes entry fails in both. An entry named `song.mid` holding real MIDI bytes: the `.srb` loads it, the `.sng` fails.

Where it came from: the chat batch you approved said "`.sng` and `.srb` would find their notes file the same way" (D27). The plan that built it kept the `.srb` sniff "as the fallback for a name outside the rule"; that was the planner's choice, and no question showed you the two outcomes. The audit found no real container with such a name.

Options: (a) both by name only; (b) both sniff bytes when the name is odd; (c) keep the split.

Recommendation: (a), the rule D27 said. Strongest case against: Clone Hero's own `.srb` bundles might carry odd names (this was never checked), and dropping the sniff could refuse a song the game plays; (b) is the safer one-rule answer if that worry is real.

### Q6. Which audio files play in the Preview: judged by name, or by bytes?

A song folder or `.sng` keeps a stem by its file name: `is_song_stem` is `is_audio_filename(filename) && stem_of(filename) != "preview"` (`src/app/preview_source.cpp:149-152`). An `.srb` keeps a stream by its bytes: `if (looks_like_audio(stream))` (`preview_source.cpp:224`), which has no name to test.

Examples. A folder with `guitar.ogg` holding real Vorbis plays. A folder with `notes.ogg` holding garbage is kept by name and then fails at open, so that stem is silently missing. `preview.ogg` in a folder is left out (it is a short clip); a `preview.wav` stream inside an `.srb` passes the byte test and is mixed into the song.

Where it came from: the concept audit's M64 paragraph said "no byte sniff for loose files" and you answered "go with recommended"; the paragraph never named these three outcomes. The split itself is older: the `.srb` path was written separately.

Options: (a) bytes everywhere (read the first bytes of each loose file in a folder or `.sng`, so `notes.ogg` holding garbage is dropped, and keep the "preview" name test for loose files, since an `.srb` stream has no name to test); (b) name everywhere (an `.srb` stream has no name, so this cannot be built; it is the same as keeping today's split); (c) keep as is.

Recommendation: (a). It costs one small read per file at Preview open and makes the folder and the `.srb` agree on what counts as a stem; a garbage `notes.ogg` is still not played, it just stops being counted as a stem that failed to open. Strongest case against: whether a real `.srb` ever carries a preview clip was not checked, so the only visible difference may never happen; "keep as is" costs nothing until one does.

### Q7. Keep Hydra's rule that one authored fill turns off all generated fills, until a live Clone Hero check?

Hydra's rule: `for (const SongTimestamp& ts : sequence) if (ts.has_activation()) return;  // chart already has fills.` (`src/core/song.cpp:106-107`). It runs before either generator is chosen (:112-115). Clone Hero's rule, unknown when this question was written, now lives in docs/clone-hero/04-fills.md (FL-27 and FL-28).

Examples. A chart with no authored fills gets generated ones at measure lines. A chart with fills on every phrase gets none, which nobody questions. The gap: a chart with one authored fill in the last chorus and none earlier gets exactly that one activation point; if the game generates fills around authored ones, the game would offer more. The other way round, a chart whose only authored fill lands on no chord is treated as having none and gets generated fills everywhere.

Where it came from: the rule is Hydra's from its first fill generator. D26 asked only "Live Clone Hero check for fill length, 6/8 beats and whether authored fills stop generation: when?" and you said "Not now". That deferred the check; it did not rule on the assumption, and D26 as recorded names only fill length and 6/8.

Options: (a) keep Hydra's rule and record it as an assumption until a live check; (b) run the live check now (one song in the game with a single authored fill, read through the `ch` MCP server); (c) change the rule now without checking.

Recommendation: (b), because this one can move a score and the check is one song. Strongest case against: you said "Not now" on 2026-10-04 and nothing new has come up; (a) with a written assumption is the honest holding position.

---

## Group 3: search and lists

What this group is about: the Library and the report windows say different things when nothing matches.

### Q8. When a search matches nothing, one sentence everywhere, and one hint in the box?

Library: "No charts match your search." (`src/ui/library_table.cpp:622`) and no button; its box hints "Search songs" (:205) with a "Clear search (Esc)" tooltip (:223). Report windows: "Nothing matches those filters." (`src/app/html_page.h:43`) with a "Clear filters" button (`src/ui/report_window.cpp:341`); the box hints "Search song, artist, or charter" (`html_page.h:44`), and the path report's says "Search song, artist, charter, or path notation" (`src/ui/path_report_window.cpp:164`).

Examples. Type `zzzz` in the Library: one sentence, press Esc to clear. Type it in the path report: a different sentence and a button that also resets the dropdown. The Library hint does not say what it searches; the windows' hints do, and differ between windows.

Where it came from: each surface was built in its own session (the windows under D103); the concept audit's M04 closure itself says "no decision covers" the sentences. The query language behind them is ruled (D56 item 1) and not asked here.

Options: (a) one sentence and one hint pattern everywhere, for example "Nothing matches." and "Search song, artist, charter" plus the extra field where a window has one; (b) keep each surface's own words; (c) one sentence, keep the per-window hints.

Recommendation: (a). Strongest case against: the Library has chips and the windows have a dropdown, so "filters" is true in one place and not the other; a shared sentence has to avoid both words, which is why (c) exists.

### Q9. Withdrawn in review: the same chart file in two folders is two rows, and you ruled that (D76, D77, D79)

Nothing to decide here. See "Dropped" at the end for the rulings that cover it.

---

## Group 4: wording and small behaviour differences

What this group is about: five small places where two parts of the app answer one question differently.

### Q10. One "about m:ss left" rule for the batch strip and the Preview?

Batch strip: average time per finished chart so far, times the charts left; shown once 3 charts have finished (`src/ui/library_jobs.cpp:261-262`: `if (... completed < kEtaMinFinished ...) return std::nullopt; return elapsed_s / completed * (total - completed);`, `kEtaMinFinished = 3` at `library_jobs.h:219`). Preview load and the startup screen: a byte rate over the last second, shown once 3 seconds have passed (`src/ui/preview_load_job.cpp:287`: `if (elapsed_s < kTimeLeftAfterSeconds || ...) return "";`, `kTimeLeftAfterSeconds = 3.0`, `preview_load_job.h:90`; the one-second window at `preview_load_job.cpp:97`). Both print through `time_left_text` (`src/app/display_format.h:58-60`).

Examples. Three 90-second charts: the strip shows nothing for 270 s. A 300 MB `.sng` in the Preview: "about N left" at 3 s. The gap: 100 charts where the first 3 are cached and finish in 1 s each: after 3 s the strip says "about 1:37 left" (3 s / 3 charts x 97 left) while the next 97 take 20 s each, over half an hour; the Preview's one-second window would correct itself within a second.

Where it came from: the batch rule is in the 2026-09-27 UI design spec you approved; the Preview rule was built separately and D100 item 4 copied it to the startup screen. Neither text mentions the other job.

Options: (a) one estimator and one gate for both (a trailing window, shown after 3 s); (b) the batch's mean-since-start for both; (c) keep both.

Recommendation: (a). Strongest case against: a batch's unit is charts, not bytes, and a per-chart rate over the last second is noisy when charts take 20 s each; a trailing window of the last few charts is the batch's equivalent, and it is a slightly different mechanism than the Preview's.

### Q11. The dot between facts: one space each side everywhere?

Almost every line uses `kFactSeparator = " \xC2\xB7 "`, one space each side (`src/app/display_format.h:16`). The batch strip's first dot has two: `line += "  \xC2\xB7  Now: " + s.current_title;` (`src/ui/library_dialogs.cpp:442`), and on the same line the next dot is the one-space form (:443). The Paths tab has a leading dot with no left space: `ImGui::TextDisabled("\xC2\xB7 %s", ...)` (`src/ui/paths_tab.cpp:501`).

Examples. Paths tab summary: "3 acts · 2 left". Batch strip: "Analyzing...  3 of 10  ·  Now: Title · Artist" (two gaps, then one). Paths tab backend row: "· Backends ..." as a bullet.

Where it came from: the strip's code comment says the wide dot "stays as the strip shows it"; that comment was written by the agent that added `kFactSeparator`, and the M46 paragraph you approved said only "the two odd spellings stay" without showing them.

Options: (a) `kFactSeparator` everywhere, including the strip; (b) keep the strip's wide dot (and the bullet) as they are.

Recommendation: (a) for the strip's dot; the Paths tab's leading bullet is a list marker, not a separator, and can stay if you agree it is a different thing. Strongest case against: purely cosmetic, and the wide gap on the strip matches the wide gap before its count.

### Q12. "Unknown option" with a capital and a colon, or "unknown option" plain?

`hydra_batch` and `hydra_fillcompare` print `Unknown option: %s` and exit 2 (`src/cli/batch.cpp:86`, `src/cli/fillcompare.cpp:65`). `hydra_replay` prints `unknown option %s` and then its usage (`tools/replay.cpp:768`). `hydra_precheck` throws `"unknown option " + a` (`tools/precheck/main.cpp:77`).

Examples. `hydra_batch --bogus`: "Unknown option: --bogus". `hydra_replay --bogus`: "unknown option --bogus" plus the usage block. `hydra_precheck --bogus`: "unknown option --bogus".

Where it came from: each tool was written in its own session; the M25 paragraph said "replay's lower-case text left alone" and you answered "go with recommended" without the two spellings in front of you.

Options: (a) one line from one helper for all four tools; (b) keep each tool's own.

Recommendation: (a). Strongest case against: replay and precheck are developer tools and a test already pins batch's spelling (`tests/test_cli.cpp:286`), so the change buys consistency only where few people look.

### Q13. Should the comparison window let go of its rows on close, as the path report does?

Path report: closing frees its rows and cancels a running build (`src/ui/app_state.cpp:954-963` `close_path_report`, via the window's close callback at `src/ui/path_report_window.cpp:274`; `drop_rows` at :65). Comparison window: no close callback is set (`src/ui/dm_report_window.cpp:135-157` sets row_click, refresh, try_again, cancel and compare_another only), so the slot keeps its result and reopen is instant.

Examples. Close and reopen the path report: the building bar again (about 3 s warm, 19 s cold on your library, D103 item 28). Close and reopen the comparison: the old table shows at once. The gap is memory while shut: about 1,100 leaderboard rows kept (one player's 1,147 scores in the D103 item 27 example) against the 68,037 path rows that caused the ruling.

Where it came from: D103 item 28 asked about the path report only, and its record says "The comparison window is unchanged", which was the recorder's note, not an option you chose.

Options: (a) both free on close; (b) both keep; (c) keep the split.

Recommendation: (a), one rule, since the comparison can only reopen through a new fetch anyway. Strongest case against: the comparison's rows are small, and freeing them removes an instant reopen for no memory you would notice.

### Q14. At SP cap 8, the toolbar refuses a comparison but the window's Refresh builds one. Which?

Toolbar: `why_not_comparable(app.settings.difficulty(), app.settings.sp_cap, ...)` with your real cap (`src/ui/library_toolbar.cpp:108`), so at cap 8 the button is greyed with "Needs SP cap 4, Clone Hero's rule: ...". Window's Refresh, Try again, Compare another: the job is given `settings.profile_at_clone_hero_cap()` (`src/ui/app_state.cpp:823-824`), always cap 4, so `collect_dm_rows` checks `profile.cap.exact` = 4 (`src/app/dm_report.cpp:63-65`) and builds. `settings_change_touches` ignores the cap (:238-242), so no out-of-date strip appears either.

Examples. Cap 4, Expert: both allow. Cap 8 with a comparison already open: the toolbar says no; Refresh builds a cap-4 comparison with no note that your Settings say 8. Difficulty Hard: both refuse (D114 item 6).

Where it came from: D114 Q6 fixed the difficulty half and its text called the cap half "deliberate"; that word was the auditor's, and no option named the cap.

Options: (a) the window follows the toolbar: Refresh at cap 8 refuses with the same sentence; (b) the toolbar follows the window: the button stays on at any cap and the page says it compares at cap 4; (c) keep the split.

Recommendation: (b). The page is always correct at cap 4, so refusing the button hides a comparison that works; the difficulty case is different because no Expert records exist for a Hard path. Strongest case against: (a) is the one-rule answer with no new text, and a player at cap 8 as a what-if may not expect a cap-4 answer.

---

## Group 5: numbers you confirmed without seeing them

What this group is about: D48 Q33 ("confirm today's numbers") listed topics, not values. The docs now cite that answer for each value below. Here are the values.

### Q15. Keep all of these as they are, or change any?

| Number | What it does | Where it lives |
|---|---|---|
| 0.6 | Overlay text boxes on the Preview image never shrink below 0.6 of full size; below that they overlap instead | `src/render/overlay_layout.h:157` `kOverlayMinScale = 0.6f` |
| 1e-3 | A railing edge whose two ends differ by less than this is drawn flat | `src/render/overlay_layout.cpp:91` |
| magenta | A malformed colour in the Preview config draws as opaque magenta | `src/render/preview_config.cpp:17` `kBadColor{1, 0, 1, 1}` |
| 50 ms | How often a leaderboard fetch checks Cancel | `src/net/http.cpp:134` `WaitForSingleObject(st.done, 50)` |
| 15 / 20 / 30 / 120 s | Leaderboard network timeouts: resolve, connect, send, receive | `src/net/http.cpp:185` `WinHttpSetTimeouts(..., 15000, 20000, 30000, 120000)` |
| 200 only | Any other HTTP status is a failure (a 204 or 301 counts as an error) | `src/net/http.cpp:227` `if (status != 200)` |
| 4096 frames | Audio is decoded and mixed in blocks of this size | `src/audio/decode.cpp:27`, `src/audio/stream_mix.h:90`, `vorbis_reader.cpp:28`, `ma_reader.cpp:166` |
| 120 ms | A zero-byte Opus packet counts as 120 ms of audio (the concealment length) | `src/audio/opus_reader.cpp:55` `frames_of_ms(120.0, kRate)` |
| 512 | Most packets read out of one Ogg page; more stops reading | `src/audio/opus_reader.cpp:403` `std::array<ogg_packet, 512>` |
| 0.5 s | MP3 seek points are built about this far apart | `src/audio/ma_reader.cpp:446` |
| 4 GiB | An MP3 larger than this gets no seek points (every seek decodes from the start) | `src/audio/ma_reader.cpp:332` `if (size > 0xFFFFFFFFull)` |
| 8% / 82% / 4.5% | The Preview loading bar's shares: read the file, open audio, build the scene | `src/ui/preview_load_job.cpp:31-33` |
| 1 s | The Preview bar's time-left answer changes at most once a second | `src/ui/preview_load_job.cpp:97` |
| 3 s | No time-left line before 3 s of loading | `src/ui/preview_load_job.h:90` |
| 0.005 | The engine reports progress every half percent | `src/search/engine.cpp:1822` `progress_reported_ + 0.005f` |
| 0.9 | The main search fills 90% of the bar; the all-0 pass fills the last 10% | `src/search/pather.cpp:36` `kMainProgressShare = 0.9f` |
| 6.0 s | A neutral status-bar line ("Saved") fades after 6 seconds; a problem line never fades | `src/ui/app_state.h:409` `kStatusFadeSeconds = 6.0`, read at `library_toolbar.cpp:28` |

None of these changes a score, a path or a stored record. Two items the audit named are gone and are not in the table: the 59.5 s "seconds to minutes" switch (time left is "about m:ss left" at every length now, `src/app/display_format.h:58-60`) and `kParkedLookups = 16` (removed in a82f74ee). One I could not find: the "64-byte Ogg tag window" from finding 327; `src/audio` has no such 64 and I did not search further.

Options: (a) keep all; (b) change the ones you name.

Recommendation: (a), and the docs that cite "D48, Q33" for them (CONTEXT.md:92, :305, :311-312, :448, :540, :624; ADR 0008:33; ADR 0019:97) should cite this answer instead. Strongest case against: "200 only" treats a redirect as an error, and the 6 s fade was never asked beside the 2 s "Copied!" badge; either could reasonably change.

---

## Group 6: rare cases

What this group is about: one edge that only a hand-made chart can reach.

### Q16. The SP end by tick and by milliseconds: merge them, pin them, or leave them?

Two tests decide "is this chord past the Star Power end". By tick: `after_sp_end(chord_tick, end_tick) { return chord_tick > end_tick; }` (`src/core/backend_value.h:36`), used by the graph and the replay. By time: `paid_by_sp_walk(offset_ms) { return offset_ms <= 0.0; }` (:30), used by the engine's pricing and the squeeze rating. The header comment says they agree "wherever time runs forwards". They disagree only when two neighbouring ticks round to the same millisecond, which needs a tempo near 1e14 BPM; `check_timing_maps` accepts any finite BPM above 0 (`src/core/song.cpp:44-46`), and a `.chart` can write `B 100000000000000000`. A `.mid` cannot reach it.

Examples. Every real chart: the two agree. A hand-made `.chart` at that tempo: a chord one tick after the end has an offset of 0 ms, so the time test says "inside" and the tick test says "after", and the two rows can price it differently. No library chart does this.

Where it came from: the two tests were written for two callers; the concept audit's M33 fixed part A (the engine re-asking the tick test) and left part B "untouched" with "needs the user".

Options: (a) one test (tick) everywhere, converting at one place; (b) refuse a chart whose tempo makes two ticks share a millisecond, so the two can never disagree; (c) leave both with a test that pins their agreement at a tempo change on the SP end tick.

Recommendation: (b). It is a one-line refusal in `check_timing_maps` with its own sentence, like the other timing refusals there, and it makes the comment's promise true by construction. Strongest case against: it adds a refusal for a chart nobody has; (c) costs less and still catches a drift.

---

## Group 7: hooks

What this group is about: two things the 2026-10-25 rebuild of `C:\Users\Patrick\.claude\gates` left out without naming them.

### Q17. Bring back the media guard (D189), or drop it on purpose?

D189 (2026-10-10) ruled: the main session may write ffmpeg or yt-dlp output under `C:\ytdl` freely; other output outside scratch asks first in the main session and is refused in a subagent. Today no hook checks any of that: a grep for `ffmpeg`, `yt-dlp` and `ytdl` over `C:\Users\Patrick\.claude\settings.json` and the hook code and docs in `C:\Users\Patrick\.claude\gates` (not its `state` folder, which holds old agent reports) finds nothing. The guard was live from 2026-10-10: it lived in the risky Bash gate and was merged into the hooks repo as 1380a37 ("Merge mediaout: hook overhaul R6 wave"). The 2026-10-25 switch (11b80b1) retired that gate. The rebuild plan's two "gone" lists (`docs/superpowers/plans/2026-10-25-hooks-from-scratch.md:86` and :208) name `bash_risky_gate` (line 86: "its real rules move to the ref judge and the delete rule") but never the media guard inside it, and your "approve as reommended" was over those lists. Neither the ref judge nor the delete rule looks at ffmpeg or yt-dlp output.

Examples. Main session writes yt-dlp output into `C:\ytdl`: D189 says free, today free (agree). A subagent runs ffmpeg over a file outside scratch and writes beside it: D189 says refuse, today nothing refuses. Main session writes ffmpeg output to `D:\x`: D189 says ask, today nothing asks.

Options: (a) build it as one small rule in the existing per-call hook (the delete rule's shape: literal paths only); (b) drop D189 and record that; (c) keep as is, which is (b) without the record.

Recommendation: (a). The ruling stands until you change it, and the rebuild's own rule was "no hook with no objective behind it", which this one has. Strongest case against: the rebuild planned to cut about 14,000 lines of hook code to about 2,200 for a reason (the plan's own estimate), and the delete rule already covers the worst case (a subagent destroying a file); a media write outside scratch is a mess, not a loss.

### Q18. Should a new or changed deny rule in the hooks run in shadow mode first?

The 2026-10-25 hook design audit proposed it as item 2 (`docs/audit/2026-10-25-hook-design-audit.md:80`): a new deny rule "first logs 'would have refused' for a few days without blocking", then someone reads the log, and only then may it deny. The rebuild plan has no shadow mode (grep for "shadow" in the plan, the gates folder and its README finds only an unrelated comment in `gates/tests/replay.py:109`). Its place was taken by the replay harness (item 3, built), which runs recorded real calls through old and new gates before a merge. The audit said each item "needs the user's yes"; you asked for a from-scratch plan and approved it, and the plan never mentioned shadow mode. The session card's open thread says the shadow-mode length was never answered.

Examples. Today: a new deny rule passes the replay harness on past calls and then denies live from its first day. With shadow mode: the same rule would log for a set number of days first and a reviewer would read the log.

Options: (a) no shadow mode; the replay harness is the check; (b) shadow mode for every new deny rule, with a length you name; (c) shadow mode only for rules that cannot be replayed (new tool shapes with no history).

Recommendation: (a). The replay harness answers the same question with real data before the merge instead of after it, and a shadow log needs someone to read it. Strongest case against: replay only covers call shapes that already happened; the first live day of the dispatch gate was exactly a shape nobody had recorded.

---

## Dropped, with the reason

- **D2: 60 (blank artist placeholder).** The code has one rule now: `artist_or_unknown` and `charter_or_unknown` run on every scan item (`src/app/analysis.cpp:715-716`), so a blank artist shows the placeholder like a blank charter. The question you saw named only the charter. No split remains to ask about; the main session may want to tell you in one line that the artist column changed with it.
- **D2: 67 (report-page search).** D56 item 1 names both sides in the words you approved: "Quotes and the Library's field prefixes (artist:, title:, charter:) are ordinary words on a report page. The Library keeps the full query language" (`docs/audit/2026-10-03-fix-decisions.md:232`). The code matches (`src/app/html_page.cpp:337-345`). Ruled, not a gap.
- **The changed-chart sentence (`src/ui/preview_tab.cpp:465`).** Confirmed both: the code says "Click the song again to see its path." and D51 call 18 says "Analyze it again to see its path." But D94 (2026-10-07, fix-decisions.md:418) records that you chose the "Click the song again" wording after the Analyze button went away, and the single-owner rule `tests/single_owner/rules/ui.json:128-133` pins D94's words. Ruled later, not a split. Small note: that rule's `must_match` list (:120) still carries the old D51 sentence as an accepted spelling.
- **U: 143's original case, and the row-identity choice (was Q9).** The "Wait for <title> to finish analyzing" panel is gone (a82f74ee) and the three row compares are one helper (`row_key`, `src/ui/library_model.h:81`, the notes path; each copy of a chart in two folders is its own row). The drafting pass asked whether those two folders should be one row, on the ground that no question had put it to you. Review found that you ruled it on 2026-10-05, in session `89d79ed1` (card `2026-10-05-89d79ed1`): you said "if the library count and the analyzed count disagree, that is a violation of derive once", then chose "Count every copy (Recommended)" (D76, fix-decisions.md:336-338); D77 extends it to the path report ("yes count every copy in the report too, and add the test") and D79 to the fill comparison. The library lists every folder, a chart in several folders is analyzed once, and every count includes every copy. Asking for one row per chart content would reverse D76. The scout for 143 searched only the question files and the decision log for "143", "two folders" and "Wait for", and never found these.
- **R7.36's parked lookups.** `kParkedLookups = 16` was removed in a82f74ee; the other two values are in Q15's table.
- **R7.32's 59.5 s switch.** Gone (`src/app/display_format.h:58-60` formats every length the same way); the shares, pacing and 3 s gate are in Q15.
- **327's 64-byte Ogg tag window.** Not found in `src/audio` or `src/net` at 1fc55723; I did not search further, so it is neither confirmed nor in the table.
