# AOA-W2a: concept audit (2026-10-11), second wave: kept / by design / leave verdicts C1 and C2 did not reach

Paths below are relative to the checkout. "Audit dir" = docs/audit/2026-10-11-concept-audit. "Decisions" = docs/audit/2026-10-03-fix-decisions.md. All "Code today" claims are by reading unless a run is named; I ran nothing.

How I chose items: I grepped findings.md and findings/V1.md to V9.md for the trigger words, then read questions.md (the closure list at lines 207 to 219 and the question text for Q6, Q16 to Q21) to see how every M-id left. The M-ids named in questions.md as "unchanged / leave / no fix / not a copy" that neither C1 nor C2 covered are M16, M32, M34, M36, M37, M40, M48, M49, M67, M68, M70, M71. All twelve have a block below. I did not reach the second claims inside the M-ids C1 and C2 did cover, except where a block says so (see "Not reached").

### concept audit M04 (third sentence): the fill comparison page's empty state

- **Splits:** None new. The fill page says the same sentence as the report windows. The only split is the one C1 already wrote up: Library "No charts match your search." (src/ui/library_table.cpp:622) vs report windows "Nothing matches those filters." (src/app/html_page.h:43, drawn at src/ui/report_window.cpp:340).
- **How it left:** The finding recorded it as "no decision covers that" (questions.md:209); the fill page's copy was made a shared constant by D114 item 5 ("C++ injects every word, noun and colour the script writes").
- **User's words:** D114 item 5: "**Q5 (M07).** The fill page stays HTML. C++ injects every word, noun and colour the script writes; only the loops stay in the script." (Decisions:556). That makes the fill page's sentence the window's sentence by construction; it does not rule on the Library's.
- **Code today:** The fill page's script receives `kNothingMatches` as a placeholder: `empty.textContent = __NOTHING_MATCHES__;` (src/app/html_page.cpp:410), filled by `json_escape_into(nothing_matches, kNothingMatches)` (html_page.cpp:500). `kNothingMatches` is `"Nothing matches those filters."` (src/app/html_page.h:43). The window draws the same constant (src/ui/report_window.cpp:340). tests/single_owner/rules/app/concept-H3.json:32 owns it ("kNothingMatches and kSearchChartsHint in src/app/html_page.h"). The fill page's search hint is the same constant family (`kSearchChartsHint`, html_page.h:44 = "Search song, artist, or charter"). The fill page has no "Clear filters" button; the windows have one (report_window.cpp:341). Only the Library keeps its own sentence.
- **Verdict:** PASS (for this item only: the fill page is not a third wording). C1's REOPEN on the Library sentence stands untouched.
- **Why:** There are two wordings on screen, not three. The fill page and the two report windows share one constant; the Library is the odd one, as C1 found.

### concept audit M76 (side claim): the batch strip's time-left rule against the Preview's

- **Splits:** What does "about 1:30 left" rest on, and when does it first appear? Batch strip: the mean time per finished chart so far times the charts left, shown once 3 charts have finished (`batch_eta_s`: `elapsed_s / completed * (total - completed)`, none when `completed < kEtaMinFinished`; src/ui/library_jobs.cpp:260-263; `kEtaMinFinished = 3`, src/ui/library_jobs.h:219) vs Song Preview load (and the startup screen, which follows it): a trailing one-second byte rate, shown once the job is 3.0 seconds old (`kTimeLeftAfterSeconds = 3.0`, src/ui/preview_load_job.h:90; `gated_time_left_text`, src/ui/preview_load_job.cpp:286-289; estimator `ByteRateClock::update`, preview_load_job.cpp:89, header comment preview_load_job.h:71-72, "the answer changes at most once a second"). Both print through the one formatter `time_left_text` (src/app/display_format.h:58-60).
- **How it left:** "by design and documented", classed K3 (V9.md:35: "The user sees "about 1:30 left" from both, from two different rules, by design and documented. I class this K3: no input is mis-answered by either."). Not in questions.md.
- **User's words:** For the batch gate: the approved 2026-09-27 UI design spec says "The estimate appears once three charts have finished." (docs/superpowers/specs/2026-09-27-ui-redesign-design.md:81). That is a spec the user approved, and it names the batch only. For the Preview side: D100 item 4, "**A time-left line, by the Song Preview's rule.** The recommendation was no time-left line; the user chose to show one. It reuses the Preview's rate clock (`ByteRateClock`), its words (`detail::time_left_text`) and its 3 s wait before the first estimate. Nothing new is invented." (Decisions:443). That rules for the startup screen to copy the Preview. Neither text mentions the other job's rule or says the batch and the Preview should differ. "Documented" is CONTEXT.md:266-272 ("Batch time left") and CONTEXT.md:299-300 ("The startup screen's time-left line follows the Song Preview's rule"); CONTEXT.md is a repo doc, not a user ruling. Searched: Decisions for "time left", "time-left", "three charts"; the three 2026-10-04 question files; questions.md; the session cards were not searched.
- **Code today:** Both rules are in the code exactly as above. The estimator also differs in kind: the batch uses the mean since start (so three fast cached charts followed by slow ones under-read for a long time), the Preview uses a window of the last second.
- **Verdict:** REOPEN
- **Why:** Two estimators and two gates (3 charts against 3 seconds) feed one user-visible sentence, and no ruling names both. The batch rule was approved in a design spec that never mentions the Preview; the Preview rule was ruled in D100 for the startup screen only. "Documented" is not a ruling.
- **Examples:** Both ends: a batch of 3 charts that each take 90 s shows nothing for 270 s; the Preview of a 300 MB .sng shows "about N left" at 3 s. Gap: a batch of 100 charts where the first 3 finish in 1 s each shows "about 0:03 left" at 3 s of elapsed time, though the next charts take 20 s each; the Preview's trailing window would have corrected within a second. Code's predicates: batch `completed < kEtaMinFinished` returns no estimate; Preview `elapsed_s < kTimeLeftAfterSeconds || !(time_left_s >= 0.0)` returns "".

### concept audit M26: does tools/bench.cpp use `app::load_scan_cache`?

- **Splits:** None. V3.md:127 "Disagree on: bench's missing try (a per-tool choice). Nothing else."
- **How it left:** code-only (C2 wrote this block as GONE and left one check open: whether bench uses the helper).
- **User's words:** D114 item 22 "Every finding the questions file lists as code-only is fixed with the owner its verifier proposed; none was vetoed." (Decisions:573). Nothing needed beyond that, because no split exists.
- **Code today:** Yes. tools/bench.cpp:153 `store ? app::load_scan_cache(*store) : store::ChartLibraryCache{};` and tools/bench.cpp:268 `cache = app::load_scan_cache(*cstore);`, then `discover_charts({folder}, app::ScanCallbacks{}, cache)` at bench.cpp:165 and :270. The two other `discover_charts({folder})` calls in bench (bench.cpp:69, :343) use the overload with no cache (src/app/analysis.cpp:723-725), which is a full scan with no store, so no library cache exists to read. Every other caller (src/cli/batch.cpp:163, src/ui/library_jobs.cpp:174, tests/test_analysis.cpp:132) also goes through `load_scan_cache` (src/app/analysis.cpp:430). By reading.
- **Verdict:** GONE
- **Why:** Same as C2's reasoning (commit ab00c8e7); the open check is now closed: bench reads the cache through the one owner.
- **Side picked without a ruling:** None. There was no differing answer to pick between.

### concept audit M62: does any command take a bare song.mid path, and is the MIDI-header check one answer?

- **Splits:** None between like questions. Different questions: "what format is the file I was handed" (extension: `chart_format_of`, src/parse/chart_files.cpp:9-14, any ".mid" is Mid) vs "which file in a folder is the chart" (`notes_file_format`, chart_files.cpp:19-24, only "notes.mid"/"notes.chart"). Where they differ is a file named `song.mid`: the library scan never lists it, and a command handed its path reads it as MIDI.
- **How it left:** code-only, optional ("one MIDI-header check", questions.md:219); the magic got one function.
- **User's words:** D114 item 22 (Decisions:573), as above. Nothing was asked about `song.mid`.
- **Code today:** (1) Commands that take a bare chart path: `hydra_replay` (tools/replay.cpp:372, :521, :603 call `load_songpath(a.chart, ...)`) and `hydra_bench` (tools/bench.cpp:87, :211, :424). `load_songpath` goes through `chart_format_of(path)` (src/parse/song.cpp:2051 `switch (chart_format_of(path))`). So a bare `song.mid` given to either tool loads as MIDI. The app never gets one: a library entry's notespath comes from the scan, which uses `pick_notes_file`/`notes_file_format` (chart_files.cpp:26-36); the Preview and the dynamics view open that same notespath (src/app/preview_source.cpp:352-353, src/app/dynamics_breakdown.cpp:189). No player-facing command takes a bare path. (2) The header check: `is_midi_header` (src/parse/midi.cpp:356-358, 4 or more bytes starting "MThd") is the one spelling, called by `walk_chunks` (midi.cpp:153) and by the .srb sniff (src/parse/song.cpp:2012). They do not give an identical answer for a stub of 4 to 13 bytes starting "MThd": the .srb sniff says MIDI (song.cpp:2012), then the reader throws `MidiError("not a MIDI file: missing MThd header")` because of the extra `size < 14` test at midi.cpp:153, so the error text is wrong for a file that does have the header. The decision (which reader) is the same; only the message text is off, on a damaged .srb entry.
- **Verdict:** PASS
- **Why:** The only path that disagrees with the scan is a developer-tool path no player meets, and Clone Hero itself reads only notes.mid/notes.chart, so the scan is the side that matters. The magic has one owner. The short-stub message is a small wording slip, listed under Side notes.

### concept audit M17 (cap row): Refresh builds a comparison the toolbar would refuse at SP cap 8

- **Splits:** May these settings be compared with the leaderboard when the SP cap is not 4? Toolbar: refused ("Needs SP cap 4, Clone Hero's rule: ...") because it passes `app.settings.sp_cap` (src/ui/library_toolbar.cpp:108-109) vs the window's Refresh / Try again / Compare another: allowed, because `launch_dm_report` hands the job `settings.profile_at_clone_hero_cap()` (src/ui/app_state.cpp:822-824), which is always cap 4, so `collect_dm_rows` passes `profile.cap.exact` = 4 to `why_not_comparable` (src/app/dm_report.cpp:63-64). `settings_change_touches` ignores the cap too (dm_report.cpp:238-243), so no out-of-date strip appears.
- **How it left:** "deliberate", inside the Q6 text: "The cap case is deliberate (the page forces cap 4, comment at lines 70 to 72); the difficulty case is not written down anywhere." (questions.md:71). Q6 asked only about the difficulty.
- **User's words:** Q6's options and Recommended name only the difficulty ("`collect_dm_rows` reads the difficulty back from the parsed chart mode ... Recommended: the first.", questions.md:73 and 75). Answered as D114 item 6: "**Q6 (M17).** `collect_dm_rows` reads the difficulty back from the parsed chart mode, so Refresh, Try again and "Compare another player..." refuse a non-Expert comparison with the toolbar's sentence, shown as a failed build." (Decisions:557). Nothing names the cap, and no other ruling does (searched Decisions for "cap 4" with "leaderboard", and the D92/D104 comparison items I did read).
- **Code today:** As in Splits. The comment now at src/app/dm_report.cpp:60-61 gives the reason for refusing a what-if cap ("a what-if cap's score would read as "above optimal" nonsense") and the page then lists the cap-4 profile whatever the setting is.
- **Verdict:** REOPEN
- **Why:** The user was told the cap case is "deliberate" but was asked nothing about it, and the difficulty ruling does not name it. The two entry points still answer "may I compare" differently at cap 8.
- **Examples:** Cap 4, Expert: both allow. Cap 8, Expert, a comparison already open: the toolbar button is greyed with "Needs SP cap 4 ..." while the window's Refresh builds, from cap-4 records, with no note that the Settings cap is different (the gap). The page itself is correct for cap 4 (this may be the better answer; the point is nobody chose it). Predicate: toolbar `why_not_comparable(difficulty, sp_cap, ...)` with the real cap vs window `why_not_comparable(mode.difficulty(), profile.cap.exact, ...)` with the forced cap.

### concept audit M40: how one squeeze or fill timing is written (whole ms against one decimal)

- **Splits:** How many decimals does the same timing get? Activation badge and verbose notation: whole ms, nearest (`format_ms_whole`, src/core/model.cpp, `std::lround`, then " ms") vs squeeze sentence, `(eff. ...)` suffix, tooltip, early-fill line, path buttons and path-report columns: one decimal (`format_ms`, src/app/display_format.cpp:24, `%.1f ms`). One SqIn at +12.6 reads "13 ms" on the badge and "12.6 ms" on the button (V5.md:69). Also sign: the badge shows a free SqIn at -316 as "-316 ms" (free-squeeze fallback, D80) while the sentence says "no more than 316.0 ms late" (it takes the absolute value).
- **How it left:** "leave" (questions.md:219 "M40: leave."); V5.md:70 "Disagree on: rounding and sign, on purpose (display_format.h 18-25 states the split)."
- **User's words:** The question the user answered: "**2. How does a squeeze timing print?** (4, 18, 36, 154) Today one squeeze can read "13 ms" on the activation badge, "12 ms" when you copy the path, and "12.6 ms" on the path button. ... After: whole-ms text rounds to nearest everywhere, so the badge and the copied text agree. One-decimal text is unchanged. ... Recommended: yes." (docs/audit/2026-10-04-phase-3-5-questions.md:20-23). Answered as D48: "The user answered "all recommended" to all 33 questions" (Decisions:118) with "Whole-ms text rounds to nearest everywhere (Q2)." (Decisions:120). The follow-up D57 item 1: "Every one-decimal millisecond figure is written with a space ("163.5 ms"), matching whole numbers ("171 ms") and the report pages. One formatter answers it." (Decisions:237).
- **Code today:** `display_format.h:25-33` ("A whole-ms timing ("163 ms") is format_ms_whole in core/model.h") still has both formatters; callers as V5 lists. By reading.
- **Verdict:** PASS
- **Why:** The question the user saw shows both sides in its own example ("13 ms" on the badge, "12.6 ms" on the button) and says one-decimal text is unchanged, so the whole-versus-one-decimal split is a chosen one. The sign difference ("-316 ms" against "no more than 316.0 ms late") is the same fact written as a number and as a sentence, which the question did not name; I count it as two phrasings, not two answers.

### concept audit M49 (with M47 and M50's shared end-of-song claim): "different numbers on purpose (D75)"

- **Splits:** When does the last note start? The Preview's `last_note_ms(scene)` (src/app/preview_view.cpp:285-288, last drawn note at the chosen difficulty) vs the song length's backup `last_note_start_ms(song)` (src/app/song_length.cpp:22-27, Expert with 2x Bass), V6.md:70: "different numbers on purpose (D75: the song length is the whole chart's, not the drawn mode's)".
- **How it left:** "M49 (last note): leave unless touched." (questions.md:219); V6.md:71 "Disagree on: none for one Song; by design for different difficulty or 2x settings."
- **User's words:** D255 (session c433b17f, 2026-10-18), item 5: "**One end everywhere.** Asked later the same day, the user wrote: "the problem is that we have two different rules. there should one rule that every function uses. we already know how clone hero decides the end of the song, we don't need different places deriving different rules." ... So `app::song_length_ms` is the one end of a song. The Preview's playback stops there, and the time box's total, the scrubber's edge and the gold marks end there too (`PreviewTransport::length_ms`)." (Decisions:909). It amends D75 items 1 and 2 (Decisions:898).
- **Code today:** `song_length_ms` no longer calls `last_note_start_ms`; it uses `song.last_note_end_tick` over all instruments and the end event (src/app/song_length.cpp:44-69). `last_note_start_ms` has no caller in src/ or tools/ (grep over src, tools, tests: only its definition, its declaration src/app/song_length.h:39 and one test comparison tests/test_preview_scene.cpp:91). `last_note_ms(scene)` is no longer a playback range input either (only boxes and tests). By reading.
- **Verdict:** GONE
- **Why:** Commit(s) behind D255 (the "song length reads the chart's end event" change, plan docs/superpowers/plans/2026-10-18-song-length-end-event.md) removed the second consumer, so the two "last note" answers no longer feed one displayed length.
- **Side picked without a ruling:** None for the end of the song: the user's own words (above) chose one rule. `last_note_start_ms` is now dead code and its header comment (song_length.h:36 "It agrees with last_note_ms of the Preview's scene") is a stale claim; see Side notes. M47's and M50's questions (Q17, Q18; D114 items 17, 18) were answered before D255 and D255 item 5 replaced the playback range they described ("This replaces D48's playback range (Q25)", Decisions:909 end); another scout should recheck those two against D255.

### concept audit M16: which of a record's key parts does the store check against the record

- **Splits:** None. V2.md:122: "Kind: K0 ... This is a missing stored fact, not a second copy of a rule." The record carries cap, ms limit, fill rule and a rules fingerprint, so `prepare_row` checks those; it cannot check chart mode or score-range mode and value because the record does not carry them.
- **How it left:** "M16: not a copy (K0), no fix." (questions.md:219).
- **User's words:** None needed for a non-split. The finding's claim that matters is V2.md:124 "the score range only widens which paths are kept, so a wrong band is not a wrong score".
- **Code today:** `prepare_row` (src/store/record_store.cpp, near line 546 per V2; I did not re-read it this pass) and `HydraRecord` (src/core/model.h) as V2 describes. D115 later made the key a profile (store::Profile with `cap.exact`, `lens`, seen at src/app/dm_report.cpp:63); I did not check whether the profile layout now stores the chart mode with the record, so this item is by the finding's reading, not today's code.
- **Verdict:** PASS (weak; see Why)
- **Why:** A missing check is not two answers to one question. The claim that a wrong score-range band is "not a wrong score" is the finding's, and I did not verify it; D115 (one record per chart and profile) rewrote the key, so a fresh scout should re-read `prepare_row` if the main session cares.

### concept audit M32: what an accent or ghost earns (category 50 against dynamics_bonus 65)

- **Splits:** None. Two different quantities. The `accent` and `ghost` categories are always the pad-independent 50 at 1x (src/core/scoring.cpp:49-50 and 65-66); `CategoryScores::dynamics_bonus` is the note's paid value less a plain note's on the same pad, multiplier included (scoring.cpp:83: `value.paid - note_value(plain, combo_multiplier).paid`), so a cymbal accent is 65 at 1x. scoring.h:39-46 states the split.
- **How it left:** "M32: no fix." (questions.md:219); V4.md:44 "Fix: none; any relabel is "needs the user" (display text)."
- **User's words:** None needed. Related ruling: Q16 / D114 item 16 "The Paths tab's "Notes:" label stays as it is." (Decisions:567).
- **Code today:** `dynamics_bonus` is written (scoring.cpp:83), copied (src/core/replay.cpp:155) and read only by tools/replay.cpp:404 (JSON) and tests. No UI reads it (grep over src and tools). So the user never sees the 65. By reading.
- **Verdict:** PASS
- **Why:** The two numbers answer different questions ("what is the accent category worth" and "what does this note earn over a plain one") and only the first reaches a screen.

### concept audit M34: which chart rows fall inside an SP end's squeeze window

- **Splits:** None. One owner, `within_squeeze_window` (src/core/model.h:126), strict at 500 ms.
- **How it left:** "M34 ... unchanged." (questions.md:219); V4.md:88 "Kind: K3 ... Fix: none."
- **User's words:** None needed (callers of one owner).
- **Code today:** Every selection site calls the owner: src/search/graph.cpp:155 and :324, src/search/engine.cpp:1990, src/core/sqout_chord.h:32 and :123, src/core/model.cpp:827. src/app/path_view.cpp:80 only mentions it in a comment. No hand-written 500 ms comparison turned up in that grep. By reading.
- **Verdict:** PASS
- **Why:** The four loops differ only in how they walk their data; the rule is asked in one place.

### concept audit M36: how many paths are tied under a root

- **Splits:** None. The engine's running `tied_count` answers "may this variant still fold under the limit" mid-search; `Path::recount_tied_paths` (src/core/model.cpp:649-656) answers "what is stored and shown" after the tree exists. V4.md:142 "The engine's running number answers a different question in the middle of the search ... The runtime check is the right tool here."
- **How it left:** "M34, M36, M37: unchanged." (questions.md:219).
- **User's words:** None needed. The runtime check V4.md:141 names (`rebuild` throws if the two disagree, engine.cpp 2055-2063) is the guard; I did not re-read that range.
- **Code today:** `recount_tied_paths` still at src/core/model.cpp:649 (per grep of the finding; not re-read). By the finding's reading.
- **Verdict:** PASS
- **Why:** Two different questions (mid-search limit test, final stored count) with a built-in assertion that they agree.

### concept audit M37: is every timing of this activation inside the ms limit

- **Splits:** None. Every comparison goes through `timing_within_limit` (src/core/model.h:283, `ms <= limit_ms`), `fill_within_limit` (model.h:329-330) and the squeeze's own `within_limit`; `Activation::within_ms_limit` (src/core/model.cpp:536-539) and the engine's `act_within_limit` differ only in what they loop over. The Library's squeeze filters also call the owner (src/app/library_query.cpp:424, :431).
- **How it left:** "unchanged" (questions.md:219).
- **User's words:** None needed. Edge "a timing on a tier edge counts as inside" is D48 Q3 ("A timing on a tier edge counts as inside (Q3).", Decisions:121), which the `<=` matches.
- **Code today:** As above, by reading.
- **Verdict:** PASS
- **Why:** One owner, with the boundary answered the way D48 Q3 says.

### concept audit M48: is Star Power running at this moment

- **Splits:** None today. Both sides take their window from `PreviewActivation::sp_window()`; the boxes test `struck_at(now, s) && now < e` (src/app/preview_view.cpp 241-247 per V6.md:40) and the highway sweeps the same window half-open (render/track_state.cpp 78-96). V6.md:52 "Disagree on: none today; a change of `struck_at` to a strict `<` ... would split them."
- **How it left:** "M48 (SP running): `sp_window_contains(now)` only if touched." (questions.md:219).
- **User's words:** None needed (agrees on every input including both edges). Not re-read in code this pass.
- **Verdict:** PASS (by the finding's reading; I did not re-open preview_view.cpp 241-247 or track_state.cpp)
- **Why:** The window has one owner and the edge rule "start inclusive, end exclusive" is spelled twice with the same answer at both edges.

### concept audit M67, M68, M70, M71: the audio readers' bookkeeping, frame counts, seek and mix rules

- **Splits:** None a player meets.
  - M67 (four readers repeat cursor members): "Kind: K3 ... Fix: none needed." (V8.md:8).
  - M68 (`read_all` against `StreamMix` when a decoder yields more frames than `length_frames()` says, V8.md:39): only `StreamMix` is production. `read_all` (src/audio/decode.cpp:21) is reached only through `decode_audio` and `decode_stem` (decode.cpp:40-50); a grep of src/ and tools/ finds no caller outside decode.cpp for either (the grep matched other functions of other names, not these two).
  - M70 (`CountedLength::seek` against `Mp3Reader::seek`): the two disagree on how far they decode forward, not on where a successful seek lands (V8.md "Every row agrees on where a successful seek lands").
  - M71 (`mix_stems` against `StreamMix`): `mix_stems` (src/audio/mixer.cpp:69, header mixer.h:51) has no caller in src/ or tools/; its only mention elsewhere is a comment in stream_mix.h:60. It is a test oracle kept in src.
- **How it left:** "M67, M68, M70, M71: leave or move test helpers out of `src/audio`." (questions.md:219).
- **User's words:** None needed for non-splits. The user-visible fact would be a stem that decodes longer or shorter than promised; V8.md:49 says "Nothing visible changes today because no reader is known to produce D > L on a real file", which I did not test (needs a file; no run).
- **Code today:** As above, by reading.
- **Verdict:** PASS
- **Why:** Each pair is either two decoders' internals with no shared question, or a production path against a test-only reference. The "no real file hits D > L" claim is the finding's and stays unproven; it would need a one-stem test (V8.md:50).

### Note on the 2026-10-11 side-notes.md (five items; each needs an exit)

docs/audit/2026-10-11-concept-audit/side-notes.md says each "goes to the user at the end of phase 5 as a possible follow-up, after someone checks it in the code" (line 3). I checked each against the code today.

1. **`Playhead::read_frames` returns `n`, not the frames its source gave.** Not found by the name. There is no `Playhead` class in src/audio any more (no playhead.h or playhead.cpp; the only `Playhead` mentions are in src/audio/player.h, src/audio/stream_mix.h and src/ui/preview_load_job.h). I did not open them. Exit: **none on record**; a fresh scout should read those three files for the current method and its header comment. Verdict: REOPEN (no exit, claim unchecked).
2. **Several audio headers say "throws std::runtime_error" where the code throws a `KindedError`.** Today src/audio/device.h:40 and :48 say "Throws std::runtime_error" and src/audio/device.cpp:61 and :78 do throw `std::runtime_error`, so those two agree. I did not check the other audio headers. Verdict: REOPEN for the other headers, unchecked; low weight (no displayed word).
3. **`MappedFile` opens with `FILE_SHARE_READ` only.** Exit: a question and an answer. Q19 (questions.md:181-187) and D114 item 19 "**Q19 (M66).** Mapped audio files keep the stricter share mode, and a comment states the reason at the open." (Decisions:570). The comment is there: src/audio/mapped_file.cpp:21-23. Verdict: PASS. The side note's own text also says "The user said leave it: no notice (2026-10-11)."
4. **Process: agents got their id from the progress hook late or never.** Not a Hydra finding. In this run the hook named my id on the reply to my first tool call, as docs/agents/brief-preamble.md:9 promises. Verdict: GONE for this run; the earlier failures are not reproducible by reading, and the claim needs the hook's own tests (gates repo), so I leave it for the main session.
5. **`prev_tick` starts at 0 in src/parse/note_shuffle.cpp.** Still true: `int64_t prev_tick = 0;` (note_shuffle.cpp:140), `same_chord = nt.tick == prev_tick` (:146). A first non-kick note at tick 0 reads as "same chord", so `pat` stays `cur_pat` = 0 (:147-152) and the chord pattern is never recorded for that first chord (the `cur_pat = pat` update at :175-177 only runs for a new chord). Effect: a following chord with the same shape would not copy the first chord's pad. Whether the game does the same is **not checked**: docs/audit/note-shuffle/game-tests has D1 (first chord at tick 0, the frozen-game case, tests/test_note_shuffle.cpp:137-147) and D2 (the same notes at 4.0 s), but no case with a first chord at tick 0 followed by a same-shaped chord and a game run. Needs a live game run to settle (audit-method.md says it stays on the open list). Verdict: REOPEN (no exit, needs the run).

<!-- next -->

## Not reached

- Second claims inside M-ids C1 and C2 already covered: I did not read M03 (V1.md:85 "The windows follow the Library on purpose (D103 item 16)", answered by Q3, D114 item 3), M05 (V1.md:172 "The fill page counts library copies, the windows count rows; deliberate (D76, D79)"), M06 (V1.md:201 the empty `posted` sinking while an empty `song` sorts as text, "recorded (D103 items 16 and 17)"), M07 (V1.md:235 the same colour token meaning three things, "a per-report choice (K3)"), M17's "is this SP cap Clone Hero's" half (V2.md:161, agrees today), V3 M23 (V3.md:70, " " charter) beyond what Q15 / D114 item 15 says, V3 M21's `sig` blank (V3.md:29 "deliberate and owned once"), V4's "Ch10 sentence carries no number on purpose" (V4.md:174), V5 M44 or M45 dead-branch notes, V7's finding 53 "recorded as kept" (V7.md:55, covered by C1's M61 block), V7 M66's second claim (V7.md:120-122 "the name test is a documented cheap pre-filter"; M64 is C2's), V8 M72/M73's "junk policy deliberate and documented" (V8.md:151, answered by Q8 and D114 items 8, 24), V8 M74's "kept on purpose for stored keys" (V8.md:195), V9 M77, M81 to M83 (answered by Q21, Q12 to Q14).
- The Side notes lists at the end of each V file (V1 to V9) were not read, except the M32 correction, which is in M32's block.
- M16, M36, M48 are PASS by the finding's own reading only (I did not re-open their code); a fresh scout should do the re-read if wanted.

## Side notes

- src/app/song_length.h:36-39: the comment "It agrees with last_note_ms of the Preview's scene" is about a function (`last_note_start_ms`) with no caller in src/ or tools/ today (grep). Dead code with a comment that restates a rule (brief-preamble.md "A comment names where a rule lives; it never restates the rule").
- src/parse/midi.cpp:153-154: a 4 to 13 byte file starting "MThd" gets "not a MIDI file: missing MThd header" though the header is there (the `size < 14` test fails first). Only reachable from a damaged .srb entry or a hand-made file. Wording slip.
- src/app/dm_report.cpp:238-243 and src/ui/app_state.cpp:822-824: changing the SP cap in Settings neither marks an open comparison out of date nor changes what Refresh builds (see the M17 cap-row block). Consistent with itself; flagged only because the toolbar refuses at the same setting.
- questions.md:71 (Q6) told the user a row was "deliberate" inside a question that asked about another row; the same pattern as M39. Worth checking the other 20 questions for sentences of the form "X is deliberate" that the user's "go with recommended" would not cover (C1's side note says the same about batch answers).
- D255 item 5 replaced the playback range described in D114 items 17 and 18 (Q17, Q18). Those two answers (scrubber spans the shorter of song length and playback end; end labels stay) were given before D255 and may be moot or contradicted now. Not checked.
