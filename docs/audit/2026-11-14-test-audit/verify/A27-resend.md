Agent aa3ab98f758d588e5, task T5-V-RESEND-A27, session 16892967-2cf8-4437-a7cd-e3dad250f621. Slice A27, the 28 records the first verifier (`verify/A27.md`) did not reach. I audited and verified none of them before. Every mark is "by reading". I ran nothing.

Opened for this file: all of `tests/test_search_target.cpp` and `tests/test_song_input.cpp`; `src/search/pather.cpp` 90-520 (the keep rules, `run_target`, `target_on_graph`, `best_finish`, `full_pin_mismatch`); `src/parse/song.cpp` 425-540, 590-700, 1403-1480, 1800-2110; `src/core/model.cpp` 270-310 and 525-545; `src/core/strutil.cpp` 70-110; `src/app/preview_source.cpp` 285-370; `src/app/preview_view.cpp` 560-612; `src/parse/midi.cpp` 440-500; `src/parse/section_names.cpp` 11-30; `src/app/ini_errors.h` 30-60; `tests/record_fixtures.h` 326-415; `docs/clone-hero/01-reading-a-chart.md` CR-19, CR-24, CR-26; the rubric. Not opened: `windows_text`, `path_named`, `fuzz_chart`'s whole generator, `main_search_options`, `Path::recount_tied_paths`.

Numbers: 28 records checked. I agree with the verdict word on 26 and dispute 2 (ST4 and ST6, both to Rewrite). On SI4 I agree with Investigate but correct its reason, and on ST9, ST10 and ST11 I add a reason; those stay agreed.

## Findings across records

- Real input exists that the records said did not. `testdata/input/common` holds 98 `song.ini` files and 44 `.chart` files. 63 of the song.ini files have a `delay` line: 61 are `delay = 0`, one is `delay = 689`, one is `delay = 1016`. All 44 charts have an `Offset`: 43 are `Offset = 0`, one is `Offset = 0.25`. So SI7 lines 151 and 153's accepted values and SI8 line 171's 1016 are values real files hold (the corpus's `delay = 1016` and `Offset = 0.25`). The refused texts (500ms, nan, inf) have no real file in the corpus; no real input exists for them. The record for SI8 says the corpus has no song.ini; that is true only of `records-screen/fixture_songs`, not of `testdata/input/common`.
- Real input also exists for a tagged name. Ten corpus song.ini files hold `charter = <color=#e02222>Blood</color>line` (for example `testdata/input/common/IB24/T5/Satyr - Whelmed/song.ini` line 5). SI13 line 353 types that exact text as a title; SI17's real example is this string, not `<color=red> Hoph2o </color>` (line 379 is typed). No corpus title or artist holds tags; its `loading_phrase` lines do, which no display test here reads.
- No corpus `.mid` holds `_ENDOFSONG` (grep over `testdata/input`, binary search for the text, zero hits), so SI2's capture needs a chart outside the corpus. No corpus `.mid` track is named `PART DRUM` (SI3).
- The tick-list overload of `search_target` has no caller in `src/` or `tools/` (I grepped `search_target(` in both; the nine hits all pass a graph or a window list). It bears on ST1, ST2, ST3 and ST7 together, as the first verifier said: one question to the user, not four.
- Split, new: `ST9`, `ST10` and `ST11` build their graph as `ScoreGraph graph(song, kCloneHeroSpCap)` (lines 405, 427, 456). Production builds it with `search_graph` (`pather.cpp:261-265`): a cap of `graph_build_cap(...)` (as tall as the song has phrases to bank), the fill rule and the user's rules. Three tests state the search's behaviour on a graph the app never builds. The test name quoted in the comment at `pather.cpp:522-524` ("a 4-bar graph built at the song's phrase count stores the same paths") says the two agree for the cap; the fill rule and rules are the defaults here. One question: should these tests call `search_graph`.
- Split, from the records and confirmed: `display_artist` reads the old title placeholder `<unknown title>` as missing (it calls `display_title`, `song.cpp:680-682`), `display_charter` does not (`song.cpp:685-688`). No test names which is wanted. I confirmed the code.
- Split, new: song.ini `delay` is a whole number in the game (CR-26, "Proven in the game's code", `01-reading-a-chart.md` line 269), while `read_ini_delay_ms` reads it through `parse_finite_number` (`preview_source.cpp:112-114`, `strutil.cpp:85`), which accepts `0.25` and `1e3`. `ini_errors.h:45` has `parse_whole_number`, which is the whole-number rule for the other ini keys, and the delay does not use it. The record raised this on SI6 only; it belongs on SI8, whose test never feeds a fractional delay.
- Split, from the records: a bad note number leaves as `std::invalid_argument` (`word_stoi`, `song.cpp:1403-1410`), a bad Resolution as a plain `ChartTimingRefused` sentence (`chart_resolution`, `song.cpp:2038-2053`). Confirmed in the code. Also `Resolution = 480abc` reads 480 (`std::stoll`, same function) while `Offset = 500ms` is absent (`parse_finite_number`). Confirmed.
- No record proposes a saved expected-output file. I checked every Rewrite and Investigate line: none does.

## test_search_target.cpp (11 records)

### ST1 (:104). Agree: Rewrite.
Q2 by reading: `keep_target_paths` (`pather.cpp:249`) is called only by `run_target` (:305); the tick overload (:387-391) has no src or tools caller. Q8 by reading: 121 and 124 fail if `took_pinned` (:202) stops being exact; 128 holds because a tick that is no fill node comes back as a path with fewer activations (comment at :235-238) and `keep_target_paths` then drops every path. 113, 114-117 and 119 are snapshots (any change). One caution on the rewrite the record proposes ("the kept set equals the unfiltered search's paths whose ticks equal the named ones"): filtering by tick equality restates `took_pinned` in the test, a copy of the owner's rule (rubric Q5). Keep 121/124 as the soundness half. Say the completeness half through a rule the test does not copy, for example that the kept set is not empty when a path with the named ticks exists in the full search's output.

### ST2 (:141). Agree: Rewrite.
Q2 by reading: `lone_pricing` has four other callers in tests (`test_fast_tempo.cpp:59`, `test_fills_and_skips.cpp`, `test_replay.cpp:285`, `test_tied_variants.cpp:247, 589`), so moving 183-189 out of this file to a test of the helper is right. Q8: 181 holds only if `rescue_variants` (`pather.cpp:122-134`) copies the whole walk; 188-189 test the helper, not `search_target`. Not read: `windows_text`.

### ST3 (:198). Agree: Rewrite.
Q8 by reading: 213 and 216 fail if a tied variant under a kept root keeps a wrong tick (`keep_qualifying_variants`, `pather.cpp:154-167`). 210 holds (`promoted` gets one entry per kept root, `keep_paths_where` :181, :195). 220 is a snapshot of `tied_pathcount`. A rewrite that states `tied_pathcount()` as "1 plus the returned variants" restates `recount_tied_paths`, which I did not open; the record should say which function owns the count before it proposes that line.

### ST4 (:225). Dispute: Rewrite, not "Investigate: claim not checked only".
I traced the whole list by hand through `keep_paths_where`, `rescue_variants` and `adopt_unshared` (`pather.cpp:122-198`). Every check holds: root 1 stays with variants 3 then 4, 3's `var_point` is 2, 4 has none, `tied_pathcount` is 3; root 6 is dropped, 7 leads with 8 tied at `var_point` 2, `promoted` is `{false, true}`. The fault in the test itself is lines 253 and 265: they type `2`. The rule (`adopt_unshared`, `pather.cpp:141-147`) is "one past the parent's walk", and the number only follows from the fixture's two ticks. Write it as `k1.walk_activations().size()` and `k7.walk_activations().size()`. The record's Q3 ("claim not checked", capture of a real chart) also stays; with a fault in the test itself the verdict is Rewrite.

### ST5 (:274). Agree: Rewrite.
Q3 by reading: the record's answer meets the brief's form (real input: the `testdata/input` corpus; independent owner: the stored record the main search wrote). Note the independence is partial: both sides come from `run_search`. Q8: 285 fails if `full_pin_mismatch` (:480-515) finds a path, total, SP meter or skips difference or a tied variant; 287 and 289 repeat the same `search_target` call a second time (line 286 runs the search that 285 already ran) and add only `windows_text`. `if (song.is_empty()) continue;` (:279) is a silent skip; `paths > 0` is a floor. The rewrite (count skips, require zero) holds.

### ST6 (:299). Dispute: Rewrite, not Investigate.
Two faults in the test itself. First, lines 332-333 flip the squeeze-out with `pins[1].squeezed_out ? std::nullopt : std::optional<int64_t>(22848)`. The path `0+- 0-` (kept list in ST1: `0+- 0- 6950`) has a squeeze-out on its second window, so the engine's `squeezed_out` is set and only the `nullopt` branch ever runs. The typed `22848` branch is dead; the comment at 327-329 promises both. Second, 12864, 22848 and the three reason words are typed (the reason words are a header contract, `pather.h:124-127`; they are fine). I traced `target_on_graph` (`pather.cpp:338-375`) for both cases: the binary search gives prefix 1 and tick 12864 in both; reason "window_end" when the end is one tick after the activation, "sqout" when the end is pinned but the squeeze-out answer is flipped. Those hold. Engine record for the real-chart half: `records-engine/index.md` line 25, Believer path 4 ("1 0- 1"), second activation: act tick 52992, `sqout_tick` 57600, one SqOut at 0 ms. The record shows the path has a squeeze-out; no run of `hydra_replay target` on it exists, so the failure words are unchecked on a real chart ("claim not checked", capture: that run with the squeeze-out answered plain).

### ST8 (:370). Agree: Rewrite.
Q3 by reading: Allies is in the corpus (`testdata/input/common/Summer Blast _25 Setlist/Tier 3/Mutemath - Allies/notes.chart`); it has no engine record, so the typed 49920 and 22272 have no independent source. I traced the search: `failed_tick` is `windows[realized].act_tick` (`pather.cpp:356-357`); moving window 1's end gives prefix 1 and moving window 3's gives prefix 3 (binary search lines 349-354). The rewrite in the record (`pins[3].act_tick`, `pins[1].act_tick`) is exactly what the code returns. 375 (`pins.size() == 4`) is a precondition on the chart.

### ST9 (:397). Agree: Rewrite, with a second reason added (see the split).
The record's reason holds (silent skip :402, floor :413, plain options built from `best_finish_options`). Add: the graph at :405 is not the production graph (finding above), and `plain.target_act_ticks.reset()` removes a vector that `pin_windows` set to an empty list (`pather.cpp:279`), so the test reaches the one difference that matters, an empty list against none. That is the claim and it holds on the real corpus; fine. I record the verdict as agree Rewrite; the dispute is only that the graph build belongs in the rewrite. (Counted as agree below.)

### ST10 (:420). Agree: Investigate, claim not checked only.
By reading: 436 fails if `pin_windows` drops the first tick, 441 and 442 fail if the free-after search declines every fill after the pin (`best_finish_options` sets `free_after`, `pather.cpp:383`). Allies has no engine record; capture: its best path's windows and the paths a one-window pin returns. Also the graph split above.

### ST11 (:451). Agree: Investigate, claim not checked only.
By reading: the loop's chart is the first in `corpus::chart_paths()` order (sorted notespaths, `corpus_util.h:56-68`) whose best path has a window failing a 0 ms limit. I cannot say which that is without a run; capture: its name. 480 fails if a window chosen after the pins breaks the limit; 487 fails if the hard limit stops dropping the unpinned path. `Activation::within_ms_limit` (`model.cpp:532-536`) is the engine's own test, not a display's. Graph split above.

### ST12 (:500). Agree: Investigate, claim not checked only.
I traced the hand-built list through `keep_pinned_paths` (`pather.cpp:232-245`) and `took_pinned` (:202-208): in prefix mode paths 1, 2 and the rescued variant 4 stay (labels `{1, 2, 4}`, promoted `{false, false, true}`), 3 is dropped, 5 activates between the pins, 6 stops short; in exact mode only path 2 stays. All checks hold. Q2: `keep_prefix_paths` has one src caller, `best_finish` (`pather.cpp:416`), reached from `path_costs.cpp:67` and `pb_coach.cpp:95, 157, 254`. Capture needed: a real path that activates between the pins.

## test_song_input.cpp, SI1 to SI9

### SI1 (:34). Agree: Rewrite.
By reading: `MidiFile::events_track` (`midi.cpp:496-500`) returns the first track whose name equals "EVENTS" ignoring case (CR-19, "Proven in the game's code"), so lines 52-55 hold. Tick 1000 at 120 BPM and 480 per beat is 1041.67 ms, tick 2000 is 2083.33 ms, as the comment says; `build_time_box` (`preview_view.cpp:569-609`) takes the last section at or before the rounded tick. `practice_sections` is read by one consumer in `src` (`preview_view.cpp:401`), so there is no second owner of "which section is in force". Q8: 52 fails if a second events track is read; 61 and 63 fail if the box lags or leads. 1041.7 and 2083.4 are typed raw results (working in a comment).

### SI2 (:68). Agree: Investigate, claim not checked only.
By reading: `marker_name` (`section_names.cpp:19-27`) vetoes a .mid body holding `_ENDOFSONG`. 77 fails if the veto is lost; 80 is covered by 77 (a missing veto adds a section, `sections.size()` changes). No corpus `.mid` holds the text (zero hits), so the capture is a chart from outside the corpus.

### SI3 (:85). Agree: Investigate, claim not checked only.
`kDrumsTrackNames` is walked by `is_drums_track_name` (`midi.cpp:447-450`) and `MidiFile::drums_track` (:490-493). The name rule is also asserted in `test_midi.cpp:201-213` (four spellings, and four non-names). `docs/clone-hero/` has no page rule for the "PART DRUM" name (grep: CR-16 names only "PART DRUMS"); D252 (`fix-decisions.md:1034`) holds the game-code evidence. No corpus `.mid` has the name. Q8: 92 fails if the track is not read; 94 and 95 add that the notes match; 94's two chords are the fixture's own.

### SI4 (:99). Agree: Investigate, with the record's premise answered; the verdict becomes Investigate: claim not checked only.
The record asks whether the throw can be reached from a file. It can. `Chord::apply_cymbal`, `apply_ghost` and `apply_accent` throw `ChartFileError` for a marker with no note (`model.cpp:288-307`), the .chart parser calls them for every marker line (`song.cpp:1619-1621`), and `run_phase` catches the error per op and drops the marker (`song.cpp:1994-2002`). The comment at `model.cpp:288-293` says so. So the throw is the mechanism SI5 relies on, not an internal contract behind a guard: SI4 and SI5 are one rule in two layers. SI4's claim is on a hand-built chord, which is the Chord's own contract; the real input is SI5's stray modifier, for which no chart is named. Lines 103-105 fail if a marker on a missing note stops throwing (then `run_phase` adds a phantom mark); 106-107 are fine.

### SI5 (:111). Agree: Rewrite.
By reading, with `note_optype` (`song.cpp:1934-1963`): notes 66, 41 and 35 on tick 0 are dropped by the per-op catch; 384's lone cymbal marker has no chord (pads count zero, no chord emitted, `song.cpp:2016`); at 768 the cymbal marker is kept. `Chord::code` (`model.cpp:108-133`): `.n...` is a plain red, `..N..` is a yellow flagged lane. Typed code strings are the fault. Q8 by reading: 123 fails if a phantom chord appears; 124 if a stray marker changes the red note; 126 if the real cymbal is lost.

### SI6 (:130). Agree: Investigate, claim not checked only.
I checked every case against `parse_finite_number` (`strutil.cpp:85-97`): `0.25`, ` -250 ` (trim first), `+500` (plus removed), `1e3` read; empty and blank are empty; `500ms`, `0.25s`, `5 5`, `0x1F4` (stops at `x`) leave `end != last`; `nan`, `NaN`, `inf`, `-inf` parse then fail `isfinite`; `1e999` is an out-of-range error; `soon` an invalid-argument error; `+-5` is refused by the sign check. All 17 hold. Callers in src (re-grepped): `ini_errors.h:45`, `preview_source.cpp:114`, `song.cpp:2082`, `song_length.cpp:19`, `rules_file.cpp:45`. The game-versus-Hydra gap on fractional `delay` (CR-26) is the split above.

### SI7 (:143). Agree: Investigate, claim not checked only, with real input now named.
`song.cpp:2078-2082` reads Offset through `parse_finite_number`. The corpus holds `Offset = 0.25` once (line 151's first value) and `Offset = 0` 43 times. `Offset = 1` and `-0.5` and the three refused texts have no real file. Capture: load that one real chart and read `chart_offset_s`.

### SI8 (:160). Agree: Investigate, claim not checked only, with real input now named.
`read_ini_delay_ms` (`preview_source.cpp:291-299`) returns nothing for an unreadable ini and otherwise `parse_delay_ms` (:112-114). The corpus has `delay = 1016` once, `delay = 689` once, `0` 61 times, so line 171 is a real value. Add the CR-26 split above (a fractional delay is accepted here; the game takes a whole number). 175 `fs::remove_all(dir)` runs after the CHECKs: a failed CHECK in doctest does not throw (CHECK, not REQUIRE), so the folder is removed.

### SI9 (:180). Agree: Investigate.
I read the code behind each block. Blank line outside a section: `find_section_header` fails, `ChartFileError` "expected a [section] header" (`song.cpp:1867-1868`). A second `=` ends the value (`song.cpp:1811-1818`). `garbage` and `=` have no tick key, so they are properties and add nothing; `N 1` has two words, so it is no note (`classify_chart_line`, `song.cpp:1467`, needs three words); `N x 0` throws `std::invalid_argument` from `word_stoi`; a section that never closes never reaches the drum list. Generic `E` events: quotes come off, inner spaces stay, so `"section  Intro  "` gives the name ` Intro  ` with a leading and trailing space (line 243), and the game's `.chart [Events]` reader trims the text first (`song.cpp:1840-1842` comment). Line 243 therefore pins a name with outer spaces where the game's reader trims; the .mid twin of this is open as Q-84 (`questions.md:228`). No test or page says what the game does on the .chart side. The record's verdict and split stand.

## test_song_input.cpp, SI12 to SI19

Shared, by reading (`song.cpp:590-700`, `song.h:100-150`): `display_title` is `title_or_unknown(clean_name(..))`; `display_artist` is `name_or_unknown(display_title(..), kUnknownArtist)`; `display_charter` is `name_or_unknown(clean_name(..), kUnknownCharter)`; `display_album` is `clean_name`. `clean_name` is `trim(strip_rich_tags(..))`. Every assertion in SI12 to SI19 holds against this code (I read each one). They are all typed-in, typed-out checks of the one text owner; the callers (record's list, `app/report.cpp`, `ui/library_model.cpp` and the others) read the same owner, so there is no display owner to compare against. The verdict for all eight is bounded by one thing: no capture of a Library row or a details panel with a tagged or missing name exists in `records-screen/` (it holds Believer, Beg and fixture runs only). I agree with Investigate, claim not checked only, for all eight. The capture needed is one Library row and one details panel for a song with a tagged charter; the corpus has the input (`charter = <color=#e02222>Blood</color>line`, ten songs).

- SI12 (:346). Agree. 347 fails if tags stop being stripped, 348 if blanks stop reading as missing.
- SI13 (:351). Agree. Line 353 is a real charter string, typed as a title.
- SI14 (:357). Agree. 359 fails if the old placeholder `<unknown title>` shows (`song.cpp:600-604`). Real input would be an older library database; none is in the repo that I searched.
- SI15 (:362). Agree. The test's name says `display_title` but its body tests `display_artist`: a naming fault, a Rewrite only of the name.
- SI16 (:368). Agree. 374 compares with the typed text `"<unknown artist>"` where `kUnknownArtist` exists (`song.cpp:692`, the constant the owner uses): a typed copy of a production constant. The same on 390 for `"<unknown charter>"`.
- SI17 (:377). Agree. Real example is the Blood/line charter, not Hoph2o.
- SI18 (:382). Agree. D266 item 2 says the charter reads `(unknown)` like the artist and title.
- SI19 (:393). Agree. Line 400 pins that the artist's placeholder text stays on an album; no ruling asks for it.

## Disputed, in one place

- ST4: Rewrite, because 253 and 265 type the `2` the rule gives as the parent's walk length.
- ST6: Rewrite, because the second branch of 332-333 never runs for this path and the typed 22848 is dead.
- SI4 (verdict word agreed, reason corrected): the record asks whether the throw can be reached from a file. It can, and the parser's per-op catch drops it.

## Counts

28 records for the 28 names in the first verifier's "Not reached" list: 11 in `test_search_target.cpp` (ST1 to ST6 and ST8 to ST12; ST7 was reached by the first verifier), SI1 to SI9 and SI12 to SI19 in `test_song_input.cpp`. 26 verdicts agreed, 2 disputed (ST4, ST6).

Records not reached: none.
