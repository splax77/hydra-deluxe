# Phase 5 questions: the concept duplication audit

There are 21 questions here, drawn from the 84 findings in `findings.md`; every predicate quoted below was re-read in the code on 2026-10-11, and in every case the code matched the verifier's block. Three of the questions are bugs rather than wording: Q6 (M17, a Hard-mode comparison the toolbar refuses but Refresh builds anyway), Q7 (M29, the song panel keeps stale names after a rescan) and Q8 (M73, `nan` and `inf` load from hydra_rules.ini without an error). The code-only findings listed at the end proceed on your yes to the fix wave and need no answer; you can veto any by id.

Each question ends with a recommendation. Answering "recommended" to a question takes that line as the decision. The answers get recorded as D-records in `docs/audit/2026-10-03-fix-decisions.md`.

---

## A. The status words (M01, M02, M03, M05, M07), one batch per D113 item 7

These five decide the words and the table rules the Library, the comparison page (the DM page) and the fill page share. Answer them together; Q1 is the one that moves a count.

### Q1 (M01). What does the comparison page call a chart whose result is Stale?

The Library and the comparison page answer "does Hydra have a result for this chart" with different predicates, and the words clash on one input.

The Library's word comes from `status_label` in `src/ui/library_model.cpp` lines 67 to 74: `Ready` reads "Analyzed", `Stale` reads "Stale", `NotAnalyzed` reads "Not analyzed". The store sets `Stale` in `get_summaries` (`src/store/record_store.cpp` lines 1366 to 1374) whenever the winning row is not `ready()`, which is `current && same_rules` at line 370: analyzed by another Hydra build, or under other rules.

The comparison page's word comes from `collect_dm_rows` in `src/app/dm_report.cpp` lines 121 to 127: a score whose chart has no entry in `by_hash` reads `kStatusNotAnalyzed` ("not analyzed") when the library lists the chart, else "not in library". `by_hash` is `list_records`, and `list_records` drops every chart whose winner is not ready (`record_store.cpp` line 1498: `if (!keep[i] || !picker.rank(i).ready()) continue;`). So a Stale chart has no entry and reads "not analyzed". The comparison page also always reads at SP cap 4 (`dm_report.cpp` line 74), while the Library reads at the cap in Settings.

Two examples. A chart you never analyzed: the Library's Best path cell says "Not analyzed", the comparison page's Status cell says "not analyzed", and the two agree. A chart you analyzed under Hydra 2.5 and have not re-analyzed: the Library says "Stale" (its own colour, with a hover naming the build), while the comparison page says "not analyzed" and counts it in the "Not analyzed" tile. A third case splits them with no Stale row at all: a chart analyzed at cap 4 while Settings says cap 5 reads "Not analyzed" in the Library and "under optimal" on the comparison page.

The options. One: the comparison page gets the store's three-way status for charts with no listed record, and a Stale chart reads "stale" there too, which moves charts out of the "Not analyzed" tile into a new tile or the dropdown. Two: the comparison page keeps one bucket but renames it to a phrase that does not clash with the Library, such as "no current result", which is already the page's own definition in `dm_report.h` lines 39 to 41. Three: leave both as they are and record the comparison page's "not analyzed" as meaning "no current result".

Recommended: option two. The page's footer already defines its word that way, and the count stays whole. The strongest case against: a player who sees "Stale" in the Library and "no current result" on the comparison page still has to work out they are the same chart, which option one would make explicit at the cost of one more tile.

### Q2 (M02). Should the comparison page use one spelling per state, and what should the fill page's "in both" say when neither side has a score?

The comparison page writes each state by hand in six places with up to four spellings. "Not in library" is the table cell "not in library", the tile "Not in library", the dropdown and the row hover "Not in your library", and the counts phrase "not in your library" (`dm_report.cpp` lines 184 to 207, `dm_report_view.cpp` lines 164 to 194, `dm_report_window.cpp` line 99). They mean the same thing today; the question is whether "your library" or "library" wins, since the code fix puts the words in one table and that table needs one spelling per place.

The fill page has a second, real clash. `collect_fill_rows` (`src/app/fill_report.cpp` lines 230 to 240) sets `"in both"` whenever both databases hold a record and the pair is not two scores: that includes one scored side, and also both sides Ready with no paths. The tile for that status reads "Score on one side only" (line 147) and the subtitle says "with a score on one side only" (line 346), per D52. The dropdown says only "In both" (line 56).

Two examples. A chart scored under 1.0 and Ready with no paths under 1.1: the status is "in both", and "Score on one side only" is true. A chart Ready with no paths on both sides: the status is still "in both", and the tile counts it under "Score on one side only", which is false for it.

The options for the words: keep "your library" in the dropdown, hovers and counts phrase and "library" in the cell and tile (today), or pick one everywhere. The options for the fill page's no-score-either-side chart: leave the tile wording and accept the miscount, or give the tile and subtitle a wording that covers both cases ("in both, not both scored"), or give the both-unscored chart its own status.

Recommended: one spelling, "your library", in every place that is a sentence, with the bare table cell staying lower case; and the fill tile reworded to "In both, not both scored" so it is true for every chart it counts. The strongest case against: the fill page is made only by `hydra_fillcompare`, and the both-unscored case may never occur in your library, so the reword could be effort on a tile nobody reads.

### Q3 (M03). Should the fill page sort text the way the Library and the windows do, and should the Library sort on the text it shows?

Three sort rules order the same kind of column. The Library sorts on `searchable` strings, which `make_searchable` (`src/app/library_query.cpp` lines 362 to 370) builds as `fold_for_search(strip_rich_tags(raw))` from the stored text, with ties broken by title, folder, then file path (`library_model.cpp` lines 229 to 238). The report windows sort on the shown text folded the same way, by plain byte compare, with up to two sort keys and ties kept in row order (`src/app/report_view.cpp` lines 29 to 49; D103 item 16 chose this). The fill page's script sorts by the browser's `localeCompare` on the raw cell value with one key (`src/app/html_page.cpp` lines 332 to 339).

Two examples. "apple" against "Banana": all three put apple first. "Étude" against "Etude": the Library and the windows tie them, the fill page puts the accented one second. For the Path column the windows put "0 1 3 0+" before "0 1 3 0- 0" (bytes) and a browser puts the "-" row first; D103 item 23 measured that on 81 of 369 corpus rows for the old page.

The Library has its own split. Its cell shows `display_title(entry.title)`, which trims and may read "(unknown)", but its sort key is the untrimmed stored text, so a blank-title chart sorts at the top under "" while showing "(unknown)", and a stored "<unknown artist>" sorts under "<" while showing "(unknown)".

The options for the fill page: give each row a folded sort field from C++ and have the script compare bytes, so it orders like the windows; or leave `localeCompare` and record it as the page's own rule. The options for the Library: sort on the shown text (moves "(unknown)" rows to the "(" position and trims leading spaces), or keep sorting on the stored text.

Recommended: the fill page takes the folded field, and the Library sorts on the shown text. The strongest case against: the Library change moves rows you can see today, for charts with blank or tag-only names, and nothing has shown that any chart in your library has one.

### Q4 (M05). Should the fill page's count line say "1 of 1 chart"?

The windows write the count line through `count_line` (`report_view.cpp` lines 51 to 54): `group_thousands(shown) + " of " + counted(total, one, many)`, so a total of 1 reads "1 of 1 path" (D103 item 18). The Library heading writes "N charts" when nothing is hidden and "N of M charts" when something is (`src/ui/library_table.cpp` lines 60 to 69), through `charts_text`. The fill page's script writes `fmt(countOf(rows)) + ' of ' + fmt(countOf(ROWS)) + ' ' + PAGE.noun` (`html_page.cpp` lines 389 to 390) with the noun fixed at "charts", so one chart reads "1 of 1 charts".

The options: pass the noun as a singular and plural pair so the script follows `counted`, or leave the plural. Recommended: the pair, so D103 item 18 holds on every page. The case against: a one-chart fill comparison is rare and the page is a tool's output.

### Q5 (M07). Should the fill page stay an HTML page?

The fill page's script restates the windows' table rules in JavaScript: search, sort (Q3), count (Q4), the empty-state sentence and the tiles (`html_page.cpp` lines 303 to 402). It is the only page `html::page_template` has left (`fill_report.cpp` line 159). Its three colours match `chip_color` by hand (`src/ui/theme.cpp` against the CSS at `html_page.cpp` lines 117 to 141).

The options: keep the page as HTML and have C++ inject every word, noun and colour through placeholders so only the loops stay in the script (the verifier's proposal); or move the fill comparison into a window like the path report and the comparison, which deletes the script. Recommended: keep it HTML with injection, because `hydra_fillcompare` is a command-line tool and a window would need the app running. The case against: the sort and plural rules stay written twice for as long as the page exists, and a third difference can appear without a test catching it.

---

## B. Three findings that look like bugs

### Q6 (M17). Should Refresh, Try again and Compare another refuse a non-Expert comparison the way the toolbar does?

The gate is `why_not_comparable` (`src/app/dm_report.cpp` lines 33 to 49). Its first rule is `if (difficulty != Difficulty::Expert) return "Needs Expert: the leaderboard only has Expert scores."`. The toolbar button calls it with the real settings, `app.settings.difficulty()`, `sp_cap`, `legacy_fills`, `view_noteshuffle` (`src/ui/library_toolbar.cpp` lines 104 to 107), and greys the button with the sentence as its tooltip. `collect_dm_rows` calls the same gate with `Difficulty::Expert` and `kCloneHeroSpCap` as literals (`dm_report.cpp` lines 62 to 64), so only the fills and Note Shuffle rules can fire there. Refresh, Try again and "Compare another player..." reach `collect_dm_rows` through `request_dm_report` and `start_dm_report` without passing the toolbar.

Two examples. Settings at Expert, cap 4, 1.1 fills: the toolbar allows the comparison and Refresh builds it; the two agree. You build a comparison at Expert, then set Difficulty to Hard in the panel: the window shows "The settings changed since this report was built." with a Refresh button; Refresh builds a comparison of Expert leaderboard scores against your Hard paths, headed only by the footer "Actual scores from dmleaderboards.com against Hydra's optimal for Hard Pro Drums, 2x Bass.", while the toolbar button for the same settings is greyed with "Needs Expert". The cap case is deliberate (the page forces cap 4, comment at lines 70 to 72); the difficulty case is not written down anywhere.

The options: `collect_dm_rows` reads the difficulty back from the parsed chart mode (`mode->difficulty()`) so every entry point refuses with the same sentence, shown as a failed build; or the window's three buttons grey with the gate's sentence while the stale Expert page stays visible; or keep today's behaviour and record it.

Recommended: the first. The gate then gets the same four inputs from every caller. The case against: a player who deliberately wants to see how their Hard paths compare loses a page the app builds today, though it is a page the toolbar already says cannot be made.

### Q7 (M29). After a rescan, should the open song panel follow the library row?

`AppState::select` copies the whole `ChartLibraryEntry` into `selected` at click time (`src/ui/app_state.cpp` line 220). A rescan ends in `reload_after_scan` (lines 159 to 164), which calls `reload_library()` and nothing else, so `selected` keeps the clicked copy. The only patch after that is two fields, `md5` and `sig`, after an analysis finds the file changed (lines 395 to 399). The panel's own "Rescan library" button closes the panel first; the toolbar button and the Song folders' "Scan now" do not.

Two examples. A rescan that changes nothing: the panel and the row agree. A rescan after you edited a song.ini's `name` with the panel open: the table row shows the new title, the panel header beside it shows the old one until you click the row again, and a re-analysis under changed settings uses the old stated length for the song length (the panel's copy is what `ViewJob` reads).

The options: after a rescan the app re-reads the open chart's entry by `row_key`, the same lookup `select` does (lines 242 to 249), so the header changes under your eyes; or the rescan closes the panel the way the panel's own button does; or keep the clicked copy and record it.

Recommended: re-read the entry. It makes the library row the one owner of what the panel says. The case against: a header that changes while you are reading it is a small surprise, and the panel's own button already avoids the problem by closing first.

### Q8 (M73). Should hydra_rules.ini reject `nan`, `inf` and hex as numbers?

Three readers decide what text is a number. `parse_finite_number` (`src/core/strutil.cpp` lines 80 to 92) uses `std::from_chars` over the whole trimmed text and rejects anything not finite; the settings file's decimal keys use it. The settings file's whole-number keys use `std::atoi` (`src/app/config.cpp` line 179), where junk reads as 0 by design. The rules file uses `std::stod` and `std::stoi` with a whole-text check (`src/app/rules_file.cpp` lines 28 to 42), then `check_min` (lines 22 to 26), which tests `d < f.min` and `d == f.min`.

Two examples. `fill_length_measures = 4`: all three readers give 4. `fill_length_measures = nan`: `std::stod` on MSVC accepts "nan" and uses all three characters, so `to_double` returns NaN; both `check_min` comparisons are false for NaN, so the value loads with no error, every rule that compares against it goes one way or the other silently, and the rules fingerprint stamps stored results with a NaN spelling. The same holds for `inf` and `0x10` (which reads as 16). This breaks the promise in `rules_file.h` lines 3 to 5 that a typo never silently runs. The settings file never hits this: its readers give 0 for the same text.

The options: `to_double` calls `parse_finite_number` and reports the rules file's usual error for anything it rejects, and `to_int` rejects the same spellings; or keep `stod` and add a finite check only; or leave it. Recommended: the first, so there is one grammar for a number in a hand-edited file. The case against: a rules file that loads today would start reporting an error, though only one that holds `nan`, `inf` or a hex number, which nobody writes on purpose.

---

## C. Wording and stored text

### Q9 (M15). Should hydra_batch's header use the panel's names for the settings?

The panel, the settings button, the batch confirm and the reports call the settings "SP cap", "Score range", "Path limit" and "1.0 fills" (`src/ui/settings_bar.cpp`, `src/ui/library_dialogs.cpp` lines 60 to 123). `describe_settings` (`src/app/config.cpp` lines 354 to 361) writes the CLI's words: `depth` as unit then number ("scores 4"), `cap` as "4 bars", `timing` as the ms or "none"; `cli/batch.cpp` lines 184 to 189 label them "Depth", "SP cap", "Timing cap" and "Fill rule". `tests/test_config.cpp` pins the CLI words on purpose, while `config.h` lines 219 to 226 calls `describe_settings` "the one place they are spelled", which is not true of the GUI.

Two examples. SP cap 4: the confirm says "4 bars (Clone Hero's rule)", the CLI says "SP cap     : 4 bars", the report subtitle says "SP cap 4 bars"; the noun agrees. Score range 2000 points: the settings button says "Score range 2,000 points", the CLI says "Depth      : points 2000".

The options: the CLI prints the GUI's names and number forms ("Score range : 2,000 points", "Path limit : off"); or the CLI keeps its own labels and the header comment is corrected. Either way the code-only part merges `score_range_value` and `within_label` into one function and the "bar"/"bars" noun into one place, with no printed word moving.

Recommended: the CLI takes the GUI's names, so one setting has one name on every surface (the "one word, one meaning" rule). The case against: anything that parses hydra_batch's header text, such as a script of yours, would need its labels updated.

### Q10 (M24). Which order and gloss should the batch's finished counts use?

Both surfaces print the same three numbers from `BatchProgress`. The GUI strip writes them through `batch_counts` (`library_dialogs.cpp` lines 54 to 58): "<n> analyzed · <n> failed · <n> skipped (already had a result)". The CLI writes "Analyzed <n>, skipped <n> already stored, <n> failed in <s>s." (`cli/batch.cpp` lines 260 to 263). The numbers never disagree, because neither recounts; only the order and the gloss for skipped differ.

Example: a run over 22 charts with one already stored and none failing reads "21 analyzed · 0 failed · 1 skipped (already had a result)" in the strip and "Analyzed 21, skipped 1 already stored, 0 failed in 40.0s." in the terminal.

The options: one function words the three counts and both print it, with the GUI's order and gloss; or with the CLI's; or each keeps its own and only the numbers share an owner (already true). Recommended: the GUI's words, because "already had a result" says what skipped means. The case against: the CLI line gets longer by a few characters and a terminal reader may prefer the shorter form.

### Q11 (M72). What should the command-line tools print when hydra_rules.ini is bad?

One state has three wordings. The app shows `kAnalysisOffSentence` (`src/ui/app_state.h` lines 279 to 281): "hydra_rules.ini has an error, so analysis is off until the file is fixed and Hydra is restarted.", with the loader's raw text under it. `kRulesFile` in `src/app/user_messages.cpp` lines 48 to 50 reads "hydra_rules.ini has a line Hydra can't read. Fix or delete that line, then restart Hydra." and is reached by no production path: every `load_rules_file` call sits in a `catch (RulesFileError&)` that stores or prints `e.what()` raw. The four tools print the raw text alone and return 2 (`cli/batch.cpp` lines 113 to 115 and three like it).

Example: a line `nope = 1` in the file. The app shows the sentence above and then `<path>: unknown key "nope"`; `hydra_batch` prints only the second line.

The options: the tools print the app's sentence with "and Hydra is restarted" dropped, then the raw line; or the tools print a tool-specific one-liner ("hydra_rules.ini has an error:" then the raw line); or the tools stay raw. In every option the dead `kRulesFile` text is deleted and the app's sentence moves into `user_messages.cpp`, which its header says owns every error sentence. Recommended: the second, because "restart Hydra" is wrong advice for a tool. The case against: three wordings become two, not one.

### Q12 (M81). One sentence for "a batch is running"?

The status line says `kBatchRunningStatus` = "A batch is running." (`app_state.h` line 473) when a running batch turns away a scan or a chart-mode switch. The greyed Scan and Analyze buttons say "Busy: a batch is running." in their tooltip (`library_toolbar.cpp` line 53). Same predicate, `batch_running()`, same kind of moment (a refusal). The panel's "Stop the batch to change these." and the confirm's forewarning are different sentences for different jobs and stay.

The options: the tooltip calls the status constant ("A batch is running."), or the constant takes the tooltip's form ("Busy: a batch is running."), or both stay. Recommended: the plain one in both places. The case against: the existing scan row lists the tooltip under `must_not_match`, so its author chose to leave it; nothing is wrong today.

### Q13 (M82). Should the song panel use the toolbar's "analysis is off" sentence?

With `rules_error` set, the toolbar and the empty path report window show `kAnalysisOffSentence` ("hydra_rules.ini has an error, so analysis is off until the file is fixed and Hydra is restarted.") with the file's error under it. The song panel shows its own literal, "Analysis is off until hydra_rules.ini is fixed and Hydra is restarted." (`src/ui/details_panel.cpp` lines 142 to 145), with no error text. Both are on screen at once whenever a song is selected.

The options: the panel calls the constant and shows the error too; the panel calls the constant without the error; the panel keeps its shorter line. Recommended: the constant without the error, since the toolbar above already shows the error and the panel is narrower. The case against: two lines on screen reading the same words is repetition a shorter panel line avoids.

### Q14 (M83). Should the status line carry the raw error, and should the comparison's failure sentence match the path report's?

When the batch's path report fails, the strip (`library_dialogs.cpp` lines 540 to 542) and the window (`path_report_window.cpp` line 162 via the frame) show "The path report could not be built.", then the plain message, then the raw error dimmed. The status line, which speaks only after the strip is dismissed, shows `set_problem("The path report could not be built. " + path_report.message)` (`app_state.cpp` line 793) with no raw error. The comparison window's failure sentence is "Could not build the report." (`dm_report_window.cpp` line 123).

Example: a report build that fails on a bad chart reads three lines in the strip and one joined line in the status bar; the comparison for a server error reads "Could not build the report." where the path report would read "The path report could not be built."

The options for the status line: add the raw error, or keep the short form. The options for the comparison: "The comparison could not be built." to match the shape, or keep "Could not build the report." In every option the three identical literals become one constant. Recommended: keep the status line short (it is one line by design) and reword the comparison to match the shape. The case against: a reword touches a uitest's expected text for no change in meaning.

### Q15 (M23). Should every chart name be trimmed before it is stored, whatever the file format?

The song.ini reader trims its lines, so a value with spaces around it is stored trimmed (`read_song_ini_keys`, `src/app/analysis.cpp`). The .sng reader stores the value bytes as they are (`sng_read_metadata`, `src/parse/sng.cpp` lines 77 to 99) and so does the .srb reader. All three then pass through `title_or_unknown`, `artist_or_unknown` and `charter_or_unknown` (`analysis.cpp` lines 728 to 730), which test only for an empty string. Screens trim on display through `clean_name` (`src/parse/song.cpp` line 531).

Two examples. A charter "Foo" in any format: stored "Foo", shown "Foo". A charter " " (one space) in a .sng: stored " ", shown blank; the same in a song.ini: stored "<unknown charter>", shown as that placeholder. A padded title differs only in the stored bytes, since `display_title` trims.

The options: trim every name field once in `discover_charts` before the fallbacks, which changes stored titles for padded values and needs a `kChartMetaStamp` bump so charts are re-read on the next scan; or leave the stored bytes and record that only song.ini trims. Recommended: trim once, so one blank charter reads the same from every format. The case against: a stamp bump re-reads every chart's metadata on the next scan, for a case nobody has shown exists in your library.

### Q16 (M31). Should the Paths tab's "Notes:" line say what it leaves out?

Two different quantities share one owner each and are exact inverses, so this is not a merge. `category_scores` (`src/core/scoring.cpp`) splits a note's 1x value into base, accent and ghost; `Path::chart_base_score` adds them back. The Stars tab's "Base score" (`src/ui/stars_tab.cpp` line 27) prints the sum. The Paths tab's "Notes:" (`src/app/path_view.cpp` line 429) prints `score_base` alone, then "Accent Notes:" and "Ghost Notes:" (lines 433 to 434).

Two examples. A plain red snare: Notes 50, Base score 50. A lone accented snare: Notes 50, Accent Notes 50, Base score 100, for one note worth 100. The three Paths lines add up to the Stars line on every path.

The options: leave the labels; or relabel "Notes:" as something like "Notes (before dynamics):" so it is not read as the notes' value. Recommended: leave them, with the User Guide's existing "Base score" wording as the explanation. The case against: a player who adds up the tab by hand sees "Notes: 50" for a 100-point note and has no line telling them why.

---

## D. Behaviour choices

### Q17 (M47). May the scrubber be dragged past where playback stops?

Playback stops at L, `length_ms_` = the later of the last drawn note and the audio's end (`src/ui/preview_transport.cpp` lines 26 to 32), and `seek_ms` clamps to L when L > 0 (lines 77 to 79). The overlay boxes pin to L through `shown_ms` (`src/app/preview_view.cpp` lines 491 to 494). The slider spans [0, E], where E is `scrub_end_ms`: the stated song length when `has_song_length` accepts it, else L (lines 655 to 658; D75 item 4). The thumb is `min(now, E)` (lines 660 to 664).

Two examples. A chart whose audio outlasts its stated length: E < L, so the slider ends at the song length and playback runs on with the thumb parked at the end, as D50 item 4 and D56 item 3 chose. A chart whose song.ini states a length longer than both the audio and the last note: E > L, the slider spans past L, a drag to the right end calls `seek_ms(E)`, the transport clamps to L, and next frame the thumb snaps back short of the edge. That case is not in any decision. A third, deliberate edge: with no notes and no audio L is 0, the clock free-runs and the boxes read 0:00.

The options: the slider's range becomes `min(E, L)` so it never spans what a seek cannot reach, with the marks still placed by E; or the slider keeps E and the thumb is allowed to park at E while playback has stopped at L; or leave it. Recommended: the first, with `shown_ms`, the thumb and the slider reading one range value from the transport. The case against: the gold marks and the Paths timeline are placed as fractions of E, so a slider ending at L sits under marks that use a different length, which is the alignment D75 item 4 set out to get.

### Q18 (M50). Should the labels tell "where playback ends" apart from "the song's length"?

Three ends have one owner each and the split is deliberate (D48 Q25, D70, D75). Playback end L (above) is shown by the clock "m:ss / m:ss" and the "<position> of <length>" measure line (`preview_view.cpp` lines 505 to 512). Song length S, `song_length_ms` (`src/app/song_length.cpp` lines 28 to 40: the stated length moved into chart time, else the last Expert-with-2x note), is the scrubber's right edge, the gold marks and the Paths timeline's end measure (`path_view.cpp` lines 408 to 411). The audio end A ends the beat grid.

Example where they agree: no stated length, Expert with 2x Bass on, audio ending at the last note. Example where they differ: audio that outlasts the stated length, or a Preview on Hard whose last drawn note is earlier than the Expert chart's: the time box's "of" measure says one bar number and the Paths tab's end measure says another, for the same chart, with nothing on screen saying why.

The options: leave the words; or label the time box's total "playback" (or the Paths end "song length") so a player can tell which end they are reading. No merge is proposed. Recommended: leave the words, because D56 item 3 already corrected the User Guide to explain the clock. The case against: the guide explains it, the screen does not.

### Q19 (M66). Should a loose stem another program is writing still play?

`file_byte_source` (`src/core/winstr.cpp` lines 238 to 243) opens with `FILE_SHARE_READ | FILE_SHARE_WRITE`, with a comment saying why: a file another program holds open still reads. `stream_md5` opens with `_wfopen "rb"`, which shares the same way. `MappedFile::open` (`src/audio/mapped_file.cpp` lines 21 to 23) opens with `FILE_SHARE_READ` only, and no comment says why. By the Win32 rule a file another program holds open for writing then fails to open. `map_song_stems` swallows the throw (`src/audio/song_audio.cpp` lines 19 to 24), so the stem is silently left out of the mix.

Two examples. A stem nobody else has open: every reader opens it. A loose drums.ogg a DAW is writing while you open the Preview: the chart file and a .sng's stems still read; the loose stem is dropped and the Preview plays without it, with no message.

The options: one open helper in `winstr` that shares writers, used by the mapping too, so the stem plays; or keep the stricter share for mapped files with a comment stating the reason (a mapped file another program shrinks can fault on read); or keep it and add a message when a stem is dropped. Recommended: keep the stricter share and write the reason down, because a mapping is a different thing from a buffered read. The case against: it is the one read of a song file with a different rule, and the reason is a guess until someone writes it.

### Q20 (M74). Should the settings file be written with the report page's write-then-rename?

`write_report_file` (`src/app/report_files.cpp` lines 89 to 117) writes `<path>.tmp` and renames it over the target, so a reader never sees a half page. `Settings::save_file` (`src/app/config.cpp` lines 217 to 239) opens the target with `trunc` and writes in place. The library file swap in `store/upgrade_files.cpp` has its own two-rename recovery.

Example: Hydra is killed mid-write. A report reader finds the old page or the new one; a settings reader finds a truncated file, which loads as defaults for every key after the cut, so settings past that point are lost silently.

The options: lift the write-then-rename into one helper and have `save_file` call it (the settings folder gains a transient `.tmp`); or leave in-place writing with a one-line comment that it is a choice. Recommended: the helper, because settings are rewritten often and a lost setting has no message. The case against: on FAT and exFAT the replacing rename is a delete then a rename, so a stop between them loses the whole file instead of its tail, as the upgrade code's own comment warns.

### Q21 (M77). When a cancel lands on a job that already finished, keep the result or drop it?

Two landing rules order the same two inputs differently. `land_report` (`src/ui/app_state.cpp` lines 752 to 771) tests `job.ok()` first, so a finished report lands Ready even if the cancel flag is set; `cancel_dm_report` says so in words ("One that already finished has its result: keep it, not the cancel."). `update_view_job` (lines 361 to 367) tests `job->is_cancelled()` first, so a click's analysis that finished and then got a Cancel is dropped and the panel says "Analysis cancelled."

Two examples. A Cancel pressed mid-run: both paths say Cancelled. A Cancel pressed after the work is complete but before the result lands: the path report keeps its rows and shows them; the song panel throws the finished analysis away. For the click that window is real (between the last cancel check and the return, `dynamics_for_settings` can re-read the chart). The batch has a third spelling: a Stop pressed on the frame the last chart finishes marks the run "Stopped" and builds no path report, though every chart ran.

The options: keep both orders, with the click's exception written beside one shared `outcome()` so the difference is visible (today's behaviour, code-only); or make the click follow the reports (a finished analysis is shown even after Cancel); or make the reports follow the click. Recommended: keep both; the panel's rule is backed by `close_details` ("nothing is saved for a song whose analysis didn't finish"). The case against: two orders for one question is exactly the kind of split this audit exists to remove, and the "Stopped with every chart done" edge stays.

---

## Already decided, no question

M04 (the search box): the Library's query language against the pages' every-word match is D56 item 1; the empty-state sentences differ per module and no decision covers that, but no merge is proposed. M09 (what a report window forgets on close): D103 item 28 says the path report frees its rows and "the comparison window is unchanged". M27 (the fill rule stored twice): finding 54, kept by design; the reader side is already fixed and the remaining reader is code-only. M39 (two squeeze ladders, "Hard" and "Insane"): D50 item 1 keeps both and says so in the report footer; the code still matches. M60 (blank title, artist, charter): D27 stores the placeholders, D50 item 5 and D56 item 2 show "(unknown)" for title and artist, and the charter deliberately differs; the code matches. M61 part (a) (the generated-fill meter walk in `check_activations`): D26 keeps today's rule as unverified against Clone Hero, and the code still has it; part (b), the time box's meter label at one tick on a mid-measure change to a shorter meter, is cosmetic and the verifier proposes leaving it, so no question; deleting the dead `bpm_pos` walk is code-only.

## Dropped from the candidate set as code-only

M41 ("free" squeeze): the only proposed change is renaming the internal `is_free`; the on-screen "Free" stays, so no answer is needed. M69 (decode errors): the proposed fix is a contract comment saying which readers report `failed()`; a message for a damaged stem would be a new sentence nobody asked for, so it is not put to you here.

## Code-only findings, with the owner each verifier proposed

These proceed on your yes to the fix wave. Veto by id.

M06 (view helpers): `report_view.h` gains one `optional_key` and one `text_column`. M08 (listed-records loops): one "listings the library lists" helper for the two loops; the SQL stays. M10 ("all" choice): the choice lists own their "all" entry as a named constant. M11 (settings a page reads): each generator states its settings once; smallest step adds `hit_window_ms` to the path report's list. M12 (chart mode, cap, lens): one struct for the record key, per its block. M13 (the panel's settings list): one list of the analysis fields. M14 (score-range unit): `Settings::lens()` passes `search_depth_mode() == Points`. M16: not a copy (K0), no fix. M18 (`settings_key`): moves to the tests, its only user. M19 (Preview song key): one `Settings::reading()` for the four parse choices. M20 (row identity): one list of identity columns in the store. M21 (scanned chart's fields): one struct, per its block. M22 (why Stale): `LibraryRow::shows_same_as(lookup)` beside `facts_of`. M25 (tool start-up): `app::load_tool_rules`, replay's lower-case text left alone. M26 (scan against cache): `discover_charts` takes the cache by reference, `app::load_scan_cache` owns the swallowed failure. M28 (transactions): `RecordStore::in_transaction`. M30 (running total): the engine reads `score_total`. M32: no fix. M33 part A: the engine reads `sq->late` instead of re-asking `after_sp_end`; part B untouched. M34, M36, M37: unchanged. M35 (graph builders): one builder expression, per its block. M38 (the "4 beats" sentence): the number comes from the constant, sentence unchanged. M40: leave. M42 (group names): a `group_name(Group)` table only with the next rename. M43 (SQ_IN/SQ_OUT): `SqueezeKind` used directly. M44 (deact tick): `Activation::deact_tick`, the pass-through deleted. M45 (squeeze-out cost): delete the dead branch. M46 (the " · " separator): one `kFactSeparator`; the two odd spellings stay. M48 (SP running): `sp_window_contains(now)` only if touched. M49 (last note): leave unless touched. M51 (usable length): the tab asks `has_song_length`. M52 (has activations): `PreviewController::has_activations()`; note the verifier says the jump buttons would grey during a load where today they are live and do nothing, a visible change it calls a dead click. M53 (box tops): one `overlay_tops`. M54 (chart changed): `ChartNow::differs_from(md5)`. M55 (scene assembly): `build_track_state` ends with `rebuild_overlay_fields`. M56 (`PreviewNote::solo`): dropped as a dead copy. M57 (lane bits): `lane_bit_rank` read off `lane_bit`. M58 (cymbal pads): the `.chart` parser asks `allows_cymbals`. M59, M62, M63, M65: small helpers (one "run one op" helper, one MIDI-header check, `is_container(ChartFormat)`, one `fits` pair in core). M64 (stem pre-filter): one table of the five formats; no byte sniff for loose files. M67, M68, M70, M71: leave or move test helpers out of `src/audio`. M75 (path cuts): `trim_trailing_separators` in `core/winstr`. M76 (job clocks): optional `elapsed_since`. M78 (Cancel on a report): optional `ReportSlot::mark_cancelled()`. M79 (cancelled throw): one `Cancelled` error type in core. M80 (may a batch start): `AppState::can_analyze()`. M84 (modal names): two named ids in `library_parts.h`.
