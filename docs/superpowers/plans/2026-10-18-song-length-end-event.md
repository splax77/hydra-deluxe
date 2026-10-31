# Song length: read the chart's end event

Date: 2026-10-18. Status: the user answered the four questions (below); the build waits for the user's yes. Nothing is built.

## The user's answers (2026-10-18)

The user picked these from the options in section 4:

1. Question 1: "Option C (Recommended)". With a stated length, `Min(E, S)`. With the stated length missing or dropped, E if there is one, else N.
2. Question 2: "Copy the pad (Recommended)". The length is the Question 1 answer plus 1,000 ms.
3. Question 3: "All instruments". N is the latest note end (start plus sustain) in any instrument section at any difficulty, as in Clone Hero. This is the plan's non-default choice. Section 7's "All tracks instead of the Expert drum note" applies, with three consequences. The 0fd29fa check ("a stated length that ends before N is dropped") now compares against this N. The Question 4 margin is measured from this N's tick. And the D-record records the wider parse instead of a departure. Chaotrope - Senescence (shortened) is the corpus chart this moves. Its N becomes 314,996 ms (the other instrument section, per section 4), which is past its stated 277,000 ms. So the stated length is dropped, and with no end event the length is 314,996 + 1,000 ms.
4. Question 4: "Copy the margin (Recommended)". E counts when `end_tick >= last_note_end_tick + Resolution`.

5. **One end everywhere (asked later the same day).** The user wrote: "the problem is that we have two different rules. there should one rule that every function uses. we already know how clone hero decides the end of the song, we don't need different places deriving different rules." Asked what the Preview does with the audio length it measures, the user chose "Ignore it (Recommended)". So `app::song_length_ms` is the one end of a song. The Paths timeline, the gold marks, the scrubber, the time box's total, the point where playback stops, and the SP meter all read it. The Preview's playback range stops being its own rule. `PreviewTransport::update_length_locked`'s "later of the last note and the audio's end" (D48, Q25: the audio tail stays playable) is replaced by the song length. `playhead_range` no longer takes the shorter of two ends, because there is only one. The measured audio end is never an input to the end. When a chart has no length at all (no stated length, no end event, no notes), the clock runs free as it does today. For Rime, every part then ends at 13:36.5, which is Clone Hero's end.

T2 is split in two by this answer. T2a is the rule in `song_length.cpp`, as T2 is written below. T2b makes the Preview read it. It owns `src/ui/preview_transport.h/.cpp`, `src/app/preview_view.h/.cpp` (`playhead_range`, `PlayheadRange` and their comments), `src/ui/preview_controller.cpp` if the load order needs it, and the preview tests. Before starting, it merges T3's branch, so it also owns the user guide's scrubber and time-box paragraphs and one added item in D255 that records answer 5 and what it replaces in D48. T2b's earlyStart is to fork from T2a's commit. The real-app check (T4) then expects the time box total, the scrubber's edge and the stop point all at the same time.

T1 is split in two because the wider parse roughly doubles it. Both halves own `src/parse/song.cpp`, so they run one after the other, not side by side. T1a is the end event, as T1 is written below. T1b stores `Song::last_note_end_tick` over every instrument section and difficulty. In .mid that means the lean filter in `src/parse/midi.cpp` keeps note messages on every instrument track, skipping BEAT, VENUE and EVENTS. In .chart, the reader takes every section's `N` lines. T1b's tests pin Neck Deep (bass past drums) and Chaotrope from the corpus, plus a synthetic sustain case where start plus length passes the last drum note. T1b owns `src/parse/midi.cpp` too. Its earlyStart is to fork from T1a's commit. T2 forks from T1b's commit, reads `last_note_end_tick` for both N and the margin, and keeps `last_note_start_ms` only where the Preview or another reader still needs it. The cost section must state the extra parse time on a click. The executor measures it on the corpus with the existing timing tools, never the library.

Input: `docs/handoffs/2026-10-18-song-length-end-event-handoff.md`. This plan answers its seven questions. Question 1 (what Clone Hero does) is answered by the two scouts in that handoff; section 2 below says which of their claims I could check myself and which I take from them.

## 1. What this is about, in plain words

Hydra shows one length for a song. It drives the Preview's scrub bar, the Paths timeline and the SP meter. Today that length is the number the chart's metadata states (song.ini `song_length`, the .sng key, the .srb field), moved into chart time by the delay or Offset. When the metadata states none, or states one that ends before the last Expert drum note (commit 0fd29fa), the last drum note stands in.

Rime of the Ancient Mariner showed the hole. Its song.ini says 6:17 for a 13:41 song. The 0fd29fa check now gives 13:23.8 (the last drum note), but the chart itself says where it ends: an `[end]` event at 13:35.5. Clone Hero reads that event. Hydra does not read it anywhere today (the only "end" in `src` is the section-name table row).

Clone Hero's rule, from the decompiled game, now lives in docs/clone-hero/01-reading-a-chart.md (CR-46 for the length, CR-43 for the last note end, CR-41 for when an end event is usable). Hydra never opens audio (D75 item 5), so something has to stand in for "audio length". That is the user's call, asked in section 4.

## 2. What Clone Hero does, and what I checked

The handoff's scout section is the record. Agent rule 9 says check before repeating, so here is what the decompiled files in the scratchpad (`ch-song-end\out1.c`) let me confirm, and what I could not.

This session confirmed the load coroutine's shape in `out1.c` lines 161 to 198, and took from the scout and the main session which fold is Max, which is Min and what the constant is; the files in the scratchpad did not name them. What it found now lives in CR-46, CR-43 and CR-41 (docs/clone-hero/01-reading-a-chart.md). One reading has no row on the pages, so it stays here: line 139 shows the call `FUN_18035c110(song, 0, 0, 1, 0)`, which the second scout said leaves the .chart parse flag at 0, so `end` in a .chart is read in ordinary play.

Not checkable from the files: the body of the usability check (`FUN_18212c7e0` is called at line 179 but its decompile is not in the scratchpad), the text handlers and the setter. The second scout settled the two points that were open when this plan was commissioned, and the plan builds them in as settled facts:

1. The .chart flag. The bracket rule it guards now lives in CR-40. The flag itself has no row on the pages, so the scout's reading stays here: the flag at `settings+0x2C` is 0 on the play path, and only Clone Hero's library scan and command-line paths set it.
2. The margin now lives in CR-41, with the factor behind it in CR-42. At 100% speed, the speed Hydra works at, it is one beat.

Two things the scouts left unknown, and what this plan does with them:

- Whether the `[end]` match is case-sensitive. It is still open; CR-39 in docs/clone-hero/01-reading-a-chart.md holds it and sends it to docs/clone-hero/questions.md. The corpus cannot settle it: of 66 `notes.mid` files, 50 carry `[end]` on the EVENTS track and none carry any other spelling; of 31 `notes.chart`, 17 carry `E "end"`, none carry `E "[end]"`, and none carry a case variant. Hydra matches the exact spellings Clone Hero's string literals hold (`[end]` in .mid; `end` or `[end]` in .chart). A chart that spells it otherwise gets no end event, which is the conservative reading.
- How .sng and .srb deliver end events. This one is settled by Hydra's own code, not the game's: a .sng or .srb holds a verbatim `notes.mid` or `notes.chart`, and Hydra's loaders hand that inner file to the same two parsers (`load_songbytes_mid` / `load_songbytes_chart`, called from the container paths in `src/parse/song.cpp` around lines 2200 and 2227). Whatever the two parsers store on `Song` arrives for all four formats with no container code.

Two numbers in the handoff differ from mine, and mine come from a parse rather than a byte search: 50 + 17 = 67 corpus files carry an end event (the handoff's raw search found 52 `[end]` strings). No corpus file carries two end events, so the "first wins" rule from the scout's reading of the setter never bites here; Hydra copies it anyway (first in file order).

Other scout findings the plan relies on: `[music_end]` is not read (Rime's `[music_end]` at 13:26.6 plays no part); the end event is in chart time (the delay moves the clock's start, not the length); song.ini `song_length` is never read on the play path, but Clone Hero writes the audio's length into it when it is missing, which is why it usually equals the audio length.

## 3. The numbers for the worked examples

Every number below comes from reading the files. The corpus numbers come from a throwaway parse of the chart's own tempo map in this session (`.mid` note ticks from the PART DRUMS track, `.chart` from `[ExpertDrums]`; "all tracks" skips BEAT and VENUE). Rime's come from the handoff's table. m:ss is rounded to a tenth.

| Chart | Format | Last Expert drum note (N) | Stated length (S) | End event (E) | Last note end, all tracks |
|---|---|---|---|---|---|
| Iron Maiden - Rime Of The Ancient Mariner (Rh), user library | .mid | 803,783 ms (13:23.8) | 377,034 ms (6:17.0) | 815,463 ms (13:35.5), tick 781,320, 6,960 ticks past the last drum note | not measured (drums only) |
| Karnivool - Themata, `testdata/input/common/IB24/T2` | .mid | 331,915 ms (5:31.9) | 341,971 ms (5:42.0) | none | 331,916 ms (drums) |
| Allister - Overrated, `testdata/input/common/IB24/T1` | .mid | 143,684 ms (2:23.7) | 151,394 ms (2:31.4) | 150,098 ms (2:30.1), tick 182,520 | 143,733 ms (drums) |
| Thrice - Deadbolt, `testdata/input/common/IB24/T3` | .mid | 141,883 ms (2:21.9) | 185,087 ms (3:05.1) | 186,359 ms (3:06.4), tick 353,520 | 141,942 ms (drums) |
| Neck Deep - Beautiful Madness, `testdata/input/common/IB24/T3` | .mid | 176,842 ms (2:56.8) | 194,131 ms (3:14.1) | 192,079 ms (3:12.1) | 189,473 ms (PART BASS, 12.6 s past the drums) |

No chart in the corpus has an end event before its last note, nor one inside the one-beat margin (the smallest gap is about two beats, Yoink - Spirit Bomb). That case gets a synthetic test chart, not a corpus one. No delay is set on any of these five (Rime's song.ini has no `delay` line), so audio time and chart time agree for them; the delay tests already in `tests/test_song_length.cpp` cover the moved case.

## 4. The questions for the user

These change D75 item 1 and item 2, so they are the user's call. Each one quotes the rule as the code would read it and shows both ends.

### Question 1. What stands in for Clone Hero's audio length?

Clone Hero needs the audio length and Hydra will not open audio. Three ways to fill the gap. In all three, N is the last Expert drum note's start, S is the stated length moved into chart time, and E is the end event when it is usable (its tick at least one beat past the last note). The 0fd29fa check stays: an S that ends before N is dropped.

**Option A, the stated length is the audio.** Copy Clone Hero's formula with S in the audio's seat: `Min(E, Max(S, N))`, and `Max(S, N)` alone with no E. Because an S that ends before N is dropped, `Max(S, N)` is S when S is present and N when it is not. So the rule reads: `S present: Min(E, S); S absent: N`. The end event only ever shortens a stated length. Note what this means for Rime: its S is dropped, so the length is N, 13:23.8, exactly what 0fd29fa gives today. Option A never reaches Rime's `[end]`.

**Option B, the end event first.** The handoff's suggestion: `E present: E; else S present: S; else N`. The end event wins whenever it is usable, even past the stated length. For Thrice this gives 3:06.4 where Clone Hero gives 3:05.1 plus the pad: the audio there is shorter than the end event, and Clone Hero caps at the audio.

**Option C, Clone Hero's formula, and the end event alone when the stated length is gone.** `S present: Min(E, S); S absent: E if present, else N`. This is Option A whenever the stated length is trustworthy, so it matches Clone Hero exactly in the ordinary case (Clone Hero's audio length is the song.ini length in nearly every chart, because the game wrote it). When the stated length is missing or proved wrong, the chart's own end event is the best fact left, and Option C uses it. Rime gets 13:35.5.

Worked numbers, without the +1 s pad (Question 2 adds it). "Clone Hero" is the game's own formula with the song.ini value as its audio length, except Rime, whose real audio length (821,464 ms, the Preview's time box) is known; the pad is included in that column because the game always adds it.

| Chart | Today (0fd29fa) | Option A | Option B | Option C | Clone Hero (pad in) |
|---|---|---|---|---|---|
| Rime (S dropped: 6:17 < N) | 803,783 (13:23.8) | 803,783 (13:23.8) | 815,463 (13:35.5) | 815,463 (13:35.5) | 816,463 (13:36.5) |
| Karnivool - Themata (no E) | 341,971 (5:42.0) | 341,971 | 341,971 | 341,971 | 342,971 (5:43.0) |
| Allister - Overrated (E before S) | 151,394 (2:31.4) | 150,098 (2:30.1) | 150,098 | 150,098 | 151,098 (2:31.1) |
| Thrice - Deadbolt (E after S) | 185,087 (3:05.1) | 185,087 | 186,359 (3:06.4) | 185,087 | 186,087 (3:06.1) |
| Synthetic: no S, E usable, N = 100 ms, E = 2,000 ms | 100 | 100 | 2,000 | 2,000 | 2,000 + pad (audio unknown) |
| Synthetic: E before N (ignored), S = 50 ms, N = 100 ms | 100 | 100 | 100 | 100 | audio + pad |

The two ends of each option: Allister is the end that shortens (all three options agree, and all move Hydra off today's number). Thrice is the end that sits past the stated length (A and C keep the stated length as Clone Hero does; B follows the event). Rime is the end whose stated length is gone (A keeps the drum note; B and C take the event).

**Recommendation: Option C.** It is Clone Hero's own formula wherever the stated length can be trusted, and it is the only option that both fixes Rime and keeps Thrice where Clone Hero keeps it. Option A is the strict copy but leaves Rime at the drum note, which is the problem the user opened. Option B is simpler to state but departs from the game whenever the audio is shorter than the end event, and the corpus shows that happens often: in 20 of the 50 `.mid` charts with an end event, E sits past S, most by 1 ms (the charter put the event at the audio's end and the game rounded `song_length` down), Thrice by 1.3 s, August Burns Red by 0.9 s, Hannes Grossmann by 0.9 s.

Whichever option the user picks, the plan below is the same code with a different predicate in one function (`app::song_length_ms`) and different pinned numbers in its tests.

### Question 2. Copy the +1 s pad?

Clone Hero adds one second after everything: the song ends, and the results screen comes, one second after the end event (or the audio, or the last note). Should Hydra's length carry the same second?

The rule with the pad: `length = (the Question 1 answer) + 1,000 ms`. Without: the Question 1 answer as it stands.

What it moves. On the Paths timeline every activation mark slides left by the share one second is of the song (about 0.4% on a four-minute song), and the end label may name the next measure. The SP meter's curve closes one second later. The scrub bar ends at the shorter of the song length and the playback end (`playhead_range`), and the playback end is the audio's end when the song has audio, so for a chart whose song.ini equals its audio length the bar does not move at all; it moves only where the audio runs past the length. Every pinned length in the tests moves by 1,000 (`tests/test_song_length.cpp`, `test_app_click.cpp` lines 648 and 665, `test_preview_controller.cpp` lines 210 and 211, `test_preview_boxes.cpp` near line 856), which is a mechanical edit.

Both ends: with the pad, Karnivool reads 5:43.0 and Clone Hero's song ends at 5:43.0; without it, 5:42.0, which is the audio's end as the charter recorded it.

**Recommendation: copy the pad.** With Option C and the pad, Hydra's number is Clone Hero's number in the ordinary case (song.ini + 1 s), and "where the song ends in Clone Hero" is the one meaning the length then has everywhere. The pad is one named constant in `song_length.cpp`, so the user's no is a one-line difference.

### Question 3. The last note over all tracks, or the last Expert drum note?

Clone Hero's "last note end" is the largest start plus sustain over every instrument and difficulty. Hydra's backup (D75 item 2) is the last Expert drum note's start, and Hydra parses only the drum track. The rule as the code would read it: `N = the start of the last chord in the Expert drums chart parsed with 2x kick` (today) against `N = the latest note end in any instrument section at any difficulty` (the game).

Where it differs: Neck Deep's bass runs 12.6 s past its drums; in the corpus 14 of 97 charts have another instrument ending after the drums (10 `.mid`: Area 11, fanclubwallet, Neck Deep, Hail The Sun, Phish, Mike Orlando, Don Broco, Rush, Sufferer, Venetian Snares; 4 `.chart`: Mastodon, Nickelback, Chaotrope, The Fall of Troy). N only reaches the screen when the stated length is missing or dropped and no end event helps. In 13 of the 14 the stated length is past both readings, so nothing would change. The exception is Chaotrope - Senescence (shortened), `testdata/input/common/Summer Blast _25 Setlist/Tier 5`: one of its other instrument sections (it has ExpertSingle, HardSingle and ExpertDoubleBass beside ExpertDrums) runs to 314,996 ms, past its stated 277,000 ms and past its last drum note at 270,896 ms, and it has no end event. Clone Hero's length there is `Max(audio, 314,996) + 1 s`, about 5:16; Hydra's, under any option with the drum note, is 277,000 ms (4:37), the stated length. The margin check also uses N: an end event must sit one beat past it. Across the corpus no end event sits within a beat of either reading of N.

Cost of the game's reading: the `.mid` lean reader would have to keep note messages for every instrument track instead of the drum track alone, and the `.chart` reader every `N` line of every section, on every click that works out a length. That is a wider parse for a value that, in this corpus, never changes the number shown.

**Recommendation: keep the last Expert drum note, and write the departure into the D-record** as a known difference, like D75's "Hydra shows what the charter wrote". One corpus chart in 97 (Chaotrope) would read shorter than in the game, and it is a drum tool showing a drum chart's end. If the user wants the game's reading, it is one more field on `Song` filled by the readers (T1 grows) and the predicate reads it instead of N; the rest of the plan stands.

### Question 4. An end event before the last note

Clone Hero ignores an end event less than one beat past the last note's end, and the length falls back to `Max(audio, last note) + 1 s`. The handoff asked whether Hydra should ignore it the same way a too-short stated length is ignored. The answer from the game is yes, with a sharper edge than "before the last note": the margin is one beat. The rule as the code would read it: `E counts when end_tick >= last_note_tick + song.tick_resolution()`. Equal counts. One tick less does not.

Both ends: an `[end]` at the last note's tick plus 480 (Resolution 480) counts; at plus 479 it is ignored and the length is S (or N with no S). This is not really a user choice, since it copies the game, but the user said to confirm it. **Recommendation: copy the margin as written**, with the beat taken from `Song::tick_resolution()` and no new number.

## 5. Where the code stands, and what changes (questions 4 to 7 of the handoff)

### Where the event is read (question 4)

Neither parser keeps global text events on `Song` today. `Song` (`src/parse/song.h`, around line 221) holds `practice_sections`, filled from the same two places that would see an end event:

- `.mid`: pass 3 of `load_songbytes_mid` (`src/parse/song.cpp` lines 1412 to 1426) walks the EVENTS track's text metas, takes the text inside `[...]` and asks `section_name_of`. The lean reader keeps those text metas (`MidiFile::lean`, `src/parse/midi.cpp` line 422 keeps texts for the drums and events tracks), so the notes-only reparse that `chart_song_length_ms` makes sees them too, at no new cost.
- `.chart`: the `[Events]` branch of the line reader (`src/parse/song.cpp` lines 1831 to 1836) classifies each line; `classify_chart_line` hands back the event body with its quotes stripped (lines 1545 to 1554). The end event would be taken in the same branch.
- `.sng` and `.srb`: the inner file goes through those two functions (section 2), so nothing to add.

The plan adds one stored fact: `std::optional<int64_t> Song::end_event_tick`, the tick of the first end event in the file, set once by each parser. The spelling is one constant, `kEndEventBody = "end"`, beside the section-name helpers; the `.mid` reader compares the bracket-stripped text with it (brackets required, as the game requires them), and the `.chart` reader strips one pair of brackets if present and compares the rest. `song_length_ms` reads the tick and converts it with `song.timing().timecode(tick).ms()`, the same conversion every note gets.

### Time bases (question 5)

Three values, two bases, kept apart in the code and named in the tests:

- The stated length S is audio time. It is moved into chart time by `audio::chart_ms_of_audio_ms(S, preview_audio_offset_ms(meta.delay_ms, chart_offset_s))` (`src/audio/frames.h` line 39). This is what `song_length_ms` does today and it does not change.
- The last note N and the end event E are chart time already: ticks through the tempo map. The margin check is made in ticks, before any conversion, because that is how the game makes it.
- The pad, if taken, is added last, in chart ms.

Tests pin a delayed stated length against an undelayed end event (a 60 ms delay, S = 150 ms audio = 90 ms chart, E at 120 ms chart) so a future edit cannot quietly move E through the delay.

### What else moves (question 6)

Nothing stored. `store::SongLength` (`src/store/record_store.h` line 174) says the click shows it and nothing stores it; the only length in the database is the scan's `stated_length_ms` column (`record_store.cpp` line 803), which is the song.ini value, not the song length, and does not change. So `kChartMetaStamp` stays at 4 (the scan reads the same facts) and `kResultsStamp` stays (no analysis output changes; the end event is not in the record). The batch works out no length. CONTEXT.md's Song length paragraph says "the store keeps one per song", which is stale against `record_store.h`; T2 corrects it.

Three displays read the one length and move together: the Paths timeline (`build_activations` in `src/app/path_view.cpp` lines 241 and 421: mark positions through `song_fraction` and the end measure label), the Preview's scrub bar (`playhead_range` in `src/app/preview_view.cpp` line 690: the bar ends at the shorter of the song length and the playback end) and the SP meter (`close_curve`, `preview_view.cpp` line 71). None of them changes; the number they are handed does. For Rime the bar's right edge moves from 13:23.8 to 13:35.5 (plus the pad if taken), the playhead no longer parks at the edge at 6:15, and the activations after 6:17 spread out along the bar.

The Preview's playback range stays a separate question: `PreviewTransport::update_length_locked` (`src/ui/preview_transport.cpp` line 26) is the later of the last note and the audio's end (D48), and audio past the song's length still plays. Untouched.

Both click paths go through the one owner: `app::analysis_song_length` (`src/app/analysis.cpp` line 857, read by the library click into `viewed.song_length_ms`) and the Preview's `read_preview_notes` (`src/ui/preview_load_job.cpp` line 81) both call `chart_song_length_ms`. The owner rows in `tests/single_owner/rules/app.json` ("How long is this song?", line 1261, and "What does a failed length read leave in the store?", line 1295) stay true; T2 adds one row so the end event is read in one place.

### Records (question 7)

A new D-record amending D75 items 1 and 2, in `docs/audit/2026-10-03-fix-decisions.md` at the next free number after D254 (the executor checks the file). It records the user's answers to the four questions in the user's words, the Clone Hero rule with the RVAs from the handoff, the corpus counts from section 2, and the known departures (drum note instead of all tracks if the user keeps it; the stated length in the audio's seat). CONTEXT.md's Song length paragraph, the header comment and function comments in `src/app/song_length.h`, and the two user guide paragraphs that say where a length comes from (`docs/UserGuide.md` line 195, the Paths timeline, and line 262, the scrubber) all follow. The user guide edit is public text: public doc style applies and the doc-review gate fires when main moves.

## 6. The build, in executor-sized tasks

Each task fits one Opus executor inside 100 tool calls and 20 minutes. Each names the files it owns and runs only its own tests on `testdata/input` plus synthetic charts; the Rime chart is read only in T4's real-app check (it is the user's library, so no committed test can depend on it). Branch names and worktrees are the main session's to assign; T2 forks from T1's commit, T3 runs beside T1 from the start.

### T1. The parsers store the end event

Owns `src/parse/song.h`, `src/parse/song.cpp`, `tests/test_song_parse.cpp`, `tests/test_midi.cpp`. earlyStart: none; it starts at the user's yes.

Add `Song::end_event_tick` (optional tick, comment pointing at the owner that reads it, `app::song_length_ms`) and the one spelling constant. In the `.mid` pass 3, when the bracket-stripped text equals the constant and no tick is stored yet, store the tick. In the `.chart` `[Events]` branch, strip one pair of brackets from the event body if both are present, compare, store the first. Nothing else reads it in this task.

Tests, each named here so the reviewer can find them:
- `midi: [end] on the events track is the end event, first one wins` (test_midi.cpp, built with `testmidi::smf_tracks`, `text_event("[end]")`; two `[end]` texts, the earlier tick stored; `[END]` and `end` without brackets not stored; an `[end]` on PART DRUMS not stored).
- `midi: the lean read keeps the end event` (both_reads agree; `load_songbytes_mid` under a notes-only reading stores the same tick).
- `.chart: E "end" and E "[end]" in [Events] are the end event` (test_song_parse.cpp, `testchart::chart_bytes` with an `Events` section; both spellings; `E "End"` not; an `E "end"` in `[ExpertDrums]` not; the file order unsorted so the first in file order is pinned).

Run: `build-cpp\Release\hydra_tests.exe -sf=test_midi.cpp -sf=test_song_parse.cpp`.

### T2. The length rule reads it

Owns `src/app/song_length.h`, `src/app/song_length.cpp`, `tests/test_song_length.cpp`, `tests/single_owner/rules/app.json`, `CONTEXT.md` (the Song length paragraph only), and, if the pad is taken, the pinned lengths in `tests/test_app_click.cpp`, `tests/test_preview_controller.cpp` and `tests/test_preview_boxes.cpp`. earlyStart: fork from T1's commit as soon as `Song::end_event_tick` exists on it; do not wait for T1's review.

`song_length_ms` grows one predicate written the way the user answered Question 1, plus `end_event_counts(song)` (the Question 4 margin, in ticks, reading `Song::tick_resolution()`), plus `kSongEndPadMs` if Question 2 is yes. The last note's tick comes from the same chord `last_note_start_ms` already finds (`SongTimestamp::timecode.ticks()`), so the two stay one lookup; refactor `last_note_start_ms` to hand back the timestamp rather than add a second walk. The `ExpertChart` callback and the notes-only reparse stay as they are; the reparse already carries the events track (section 5). As built, T2a removed both (and `chart_song_length_ms` with them): the last note's end (N) is stored on the `Song` by the parsers over every instrument and difficulty, so it no longer depends on how the chart was read and `song_length_ms(meta, song)` needs no second parse. Comments in `song_length.h` say where each rule lives and name the D-record; no history in them.

Add one owner row to `app.json`: "Where does the chart's end event sit, and does it count?", owner `end_event_counts` in `src/app/song_length.cpp`, pattern `end_event_tick` read outside `song_length.cpp` and the two parser writes, with the parser lines as owner lines. Update the note on the "How long is this song?" row to name the end event. Correct the CONTEXT.md sentence about the store.

Tests in `test_song_length.cpp`, with the fixtures already there (`chart_with_ini`, `testchart::section`, `testsng::make_sng`); each pins a literal from one run:
- `song length: an end event after the last note caps a longer stated length` (Allister's shape: S 200,000, E 150,000 chart ms, result E).
- `song length: an end event past the stated length leaves it` (Thrice's shape; result S under Options A and C, E under B; the test pins whichever the user chose).
- `song length: with the stated length dropped, the end event stands in` (Rime's shape: S 50 ms < N 100 ms, E 2,000 ms; result E under Option C, N under A).
- `song length: with no stated length, the end event stands in` (and N when there is no event either).
- `song length: an end event inside one beat of the last note is ignored` (Resolution 192: E at last tick + 192 counts, + 191 does not; both ends pinned).
- `song length: the end event is chart time, the stated length is audio time` (the 60 ms delay case from section 5).
- `song length: a .sng carries its inner file's end event` (make_sng around a `.chart` with `E "end"`).
- If the pad is taken: every existing case in this file moves by 1,000 and one case pins the pad on an end event, a stated length and a last note each.

Run: `hydra_tests.exe -sf=test_song_length.cpp -sf=test_app_click.cpp -sf=test_preview_controller.cpp -sf=test_preview_boxes.cpp` and the single-owner scan (the test under `tests/single_owner`; the executor takes its filter name from that folder's README).

### T3. The records

Owns `docs/audit/2026-10-03-fix-decisions.md` (the new D-record only), `docs/UserGuide.md` lines 195 and 262, and a one-line status update at the top of `docs/handoffs/2026-10-18-song-length-end-event-handoff.md`. earlyStart: at the user's yes, in parallel with T1; it needs only the user's answers, which the main session pastes into the brief word for word.

The D-record follows the form of D250 and D75: the question, the measurements with their sources, the Clone Hero rule with RVAs, the user's calls numbered, and what this replaces in D75 (items 1 and 2; item 3, 4 and 5 stand). The user guide paragraphs say, in second person and without the decision's nickname, that the timeline runs to the chart's own end event when it has one, else the stated length, else the last Expert drum note, and that Hydra never opens the audio for it; the scrubber paragraph keeps its sentence about playback. The executor checks every label and number in those paragraphs against `song_length.cpp` before it commits (public doc style).

Run: `hydra_tests.exe -sf=test_docs_match_code.cpp` if that test names the user guide's length paragraphs (the executor greps; if it does not, no test).

### T4. The real-app check, before the merge to main

Done by the main session or a read-only Sonnet scout after T1 and T2 sit on one integration branch. It is a check, not a build, and it has a cap: 20 minutes, then report what was seen.

Headless first, on the corpus (`hydra_start("headless")` or `hydra_uitest` with the Preview opened on Allister - Overrated and Thrice - Deadbolt): read the Paths tab's timeline end label and the Preview's time box against the table in section 4, through `hydra_text` and `hydra_dump`.

Then the real window on a snapshot copy of the user's library (`hydra_start("window")`, which copies the db and ini and never opens the real ones; a branch build is driven the way the MCP-server memory describes, with the worktree's helper). Open Rime in the Preview. Expected: the scrub bar's right edge at 13:35.5 (13:36.5 with the pad), the playhead reaching the right edge only at the end, the activations after 6:17 spread along the bar instead of piled at the edge, and the time box still 13:41.5 (the audio's end, which this change does not touch). Take one screenshot for the user. Rendering is checked on the corpus charts, not the library (memory "Render checks use the corpus"); the library copy is read only for Rime's numbers.

Nothing in this plan runs over the whole library.

## 7. What a different answer changes

- Option A or B instead of C: the predicate in `song_length_ms` and the pinned results of two tests in T2. Nothing else.
- No pad: drop `kSongEndPadMs`, and the pinned numbers outside `test_song_length.cpp` stay as they are. T2 shrinks.
- All tracks instead of the Expert drum note: T1 also stores a `last_note_end_tick` over every instrument section (the `.mid` lean filter must then keep note messages on every instrument track, and the `.chart` reader every section's `N` lines), T2 reads it for both N and the margin, and the D-record records the wider parse instead of the departure. T1 roughly doubles; the rest stands.
- The margin is not a choice; it copies the game.

## 8. Prior art

The GuitarGame_ChartFormats docs (TheNathannator, `docs/Chart-File-Formats/mid-format/Tracks/Global-Events.md`) describe `[end]` only as "the end point of a song" and the .chart overview says events may carry brackets or not; neither says what a game does with it, which is why the decompile was needed. YARG and Moonscraper read `[end]` as the song's end too, by those docs' account, but this plan rests on Clone Hero's own code, not on their behaviour.
