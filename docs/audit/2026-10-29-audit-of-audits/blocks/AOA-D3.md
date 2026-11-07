# AOA-D3 closure checks (drift and double_reading findings, 2026-10-03 derivation audit)

Paths are relative to the checkout. "Audit" = docs/audit/2026-10-03-derivation-audit.md. Question files are docs/audit/2026-10-04-phase-3-5-questions.md (P35) and 2026-10-04-phase-7-questions.md (P7). Decisions are docs/audit/2026-10-03-fix-decisions.md (DEC). The user's "all recommended." for the phase 7 sheet is in session card b9839bd7 (cards/2026-10-04-b9839bd7.md:20: `D51: "all recommended." (22:22 UTC, approves every question on the sheet at its recommended answer)`). The phase 3-5 batch answer is recorded at DEC:118: `The user answered "all recommended" to all 33 questions`.

Counts: PASS 19, GONE 3 (106, R7.2, 133 tentative), REOPEN 0, not reached 0 (133 only partly verified, see its block).

### 2026-10-03 86: "every path" label has two constants

- **Splits:** Does `--paths N` (N between 100,000,000 and 1,000,000,000) read "every path" in the report subtitle: old threshold constant said yes (above 100,000,000) vs sentinel said no (only 1,000,000,000). Today only one constant is left: `src/app/report.cpp:325` `options.max_paths >= kEveryPathSentinel`, `src/app/report.h:162` `kEveryPathSentinel = 1000000000`. `kEveryPathLabelThreshold` appears nowhere in src.
- **How it left:** a "code-only call" listed in the phase 7 sheet. Closure's words: "**hydra_report:** `--paths 200000000` says "top 200,000,000 paths" and not "every path" (86)." (P7:155). Triage: "fixed".
- **User's words:** the sheet's heading "Code-only calls I'll make unless you say otherwise" (P7:145) with the line above (P7:155); answered as D51: "The code-only calls at the end of the sheet are approved as written. They cover findings 38, 59, 68, 78 to 85, 86, 98, 103, 108, 109, 170, 174, 198 and 202." (DEC:194), after "all recommended." (card b9839bd7:20).
- **Code today:** the label test is the single `>=` at report.cpp:325; 200,000,000 reads "top 200,000,000 paths per chart" by `hydra::counted` (report.cpp:327), by reading.
- **Verdict:** PASS
- **Why:** The ruling text names the exact input (`--paths 200000000`) and both outputs ("top 200,000,000 paths" not "every path"), and the code does that.

### 2026-10-03 88: Ready chart with no paths

- **Splits:** Is a Ready record with zero paths "analyzed"? Chip said yes (`src/ui/library_model.cpp:16` Ready -> Analyzed) vs `stars:`/`squeeze<=` and the leaderboard page said no. Today: chip stays Analyzed (library_model.cpp:16), `facts_of` returns no facts (library_model.cpp:43-45), DM page status `kStatusNoPaths = "no paths"` (`src/app/dm_report.h:40`), fill page prints 'no paths' (`src/app/fill_report.cpp:88`).
- **How it left:** a question to the user (P7 Q11).
- **User's words:** "**11. A Ready chart that kept no paths.** (88) Today it sits under the Analyzed chip, but `stars:` and `squeeze<=` never list it, and the leaderboard page calls it "not analyzed". Nobody has seen a real analysis produce one. Recommended: it stays Analyzed, and the filters treat it as having no facts. The leaderboard and fill pages say "no paths"." (P7:63-66), answered as D51 call 11: "A Ready chart with no paths stays under Analyzed. The filters see no facts for it, and the leaderboard and fill pages say "no paths" (Q11, 88)." (DEC:171).
- **Code today:** as above, by reading. `summarize_record` gives such a record no score (record_store.cpp:341).
- **Verdict:** PASS
- **Why:** The question named both sides (chip vs filters/leaderboard) with their wording, and the code does what the Recommended line says.

### 2026-10-03 90: path folded mid-SP keeps a stale SP end

- **Splits:** Where does a tied variant's last activation end: its own fold-time snapshot vs the leader's closed activation. Today `Engine::close_folded_act` (`src/search/engine.cpp:1563`) copies the leader's `deact_edge`, `sqout_phrase`, `final_sp_end` (engine.cpp:1569-1572) to the variant.
- **How it left:** a question to the user in chat (call 3 of the step-1 walk-through).
- **User's words:** Q "Call 3 (finding 90): how should a tied path folded mid-Star-Power get its SP end?" with options "Give the variant the leader's closed activation (recommended)" and "Stop folding while SP is running", user chose "Use leader's activation (Recommended)" (card `docs/sessions/2026-10-03-cebf3006.md`:21 keeps the question and the chosen label; the option text was not kept). Recorded as D3: "A tied variant folded into its leader while SP is still running takes the leader's closed activation from the fold point: its SP end, clamp note, collected phrases and backend rows." (DEC:11).
- **Code today:** engine.cpp:1700-1704 calls `close_folded_act` for `var.open_sp`, by reading.
- **Verdict:** PASS
- **Why:** Both options were written out in the question and the user picked one; code does the picked one.

### 2026-10-03 93: Paths row "N bars"

- **Splits:** What the row's "N bars" means: bars banked at activation (`act.sp_meter`) vs the larger amount SP actually lasts. The words side is now settled: `src/app/path_view.h:74` `int sp_bars = 0;             // bars banked when you activate`, `docs/UserGuide.md:259` "banked when you activate".
- **How it left:** question (P35 Q9).
- **User's words:** "**9. "Bars" on an activation row.** (93) Today a row says "2 bars" when you banked 2 bars at the activation. A phrase collected during Star Power can make it last as long as 3 would. Recommended: keep the number and fix the words: "bars banked when you activate". That is what your meter shows when you hit the activation, and the gauge already shows the spend." (P35:52-55), D48: "A row's bars mean "banked when you activate" (Q9)." (DEC:127).
- **Code today:** by reading, as above.
- **Verdict:** PASS
- **Why:** The question named both numbers (2 banked vs 3 lasted) and the code and guide say "banked".

### 2026-10-03 94: 2W squeeze budget edge rules

- **Splits:** Is a gap of exactly 2W (170.0 ms at W=85) inside the budget: details view said inside vs report tier said Beyond. Today `tier_for` uses `*ms <= *t.cutoff` (`src/app/report.cpp:91-92`), the Insane+ cutoff is `nominal_budget_ms(w)` (`src/core/squeeze_rating.cpp:182`), and `effective_backend_ms` calls `squeeze_budget_ms` (squeeze_rating.cpp:62-63), so one formula (squeeze_rating.cpp:66-68).
- **How it left:** question (P35 Q3).
- **User's words:** "**3. A timing exactly on a tier edge.** (34, 94) Today a path at exactly 2.0 ms has no orange warning on the Paths tab but reads Hard in the report. A gap of exactly 170.0 ms reads Beyond in the report but has no "over budget" line in the details. After: both places agree. Recommended: a timing on the edge counts as inside, so 2.0 ms is not difficult and 170.0 ms is Insane+. That's how the Paths tab has always read it." (P35:25-29), D48: "A timing on a tier edge counts as inside (Q3)." (DEC:121).
- **Code today:** by reading; `Normal` cutoff `kDifficultMs` inclusive (squeeze_rating.cpp:177-180).
- **Verdict:** PASS
- **Why:** Both sides and both edge values (2.0 and 170.0) were in the question.

### 2026-10-03 95: tie limit counts per side

- **Splits:** How many tied paths at one score are kept: per side of the Path limit (up to 8 at limit 4) vs one count. Today key = `(score << 1) | apart` with `apart = over_limit_pass && node >= 0` (`src/search/engine.cpp:1259-1260`): complete paths inside and over the limit share one key; only still-running over-limit paths keep their own.
- **How it left:** question (P7 Q1), plus a refinement D55 item 5.
- **User's words:** "**1. With the tie limit at 4, may Hydra keep 8 tied top-score paths?** (finding 95, changes results) Today the tie limit counts separately for paths inside your Path limit and paths over it. So "keep 4 tied paths" can keep 4 of each, which is 8. After: one count per score. When some tied paths are inside the limit and some are over, the inside ones come first. Recommended: one count per score, as the User Guide already promises." (P7:13-17), D51 call 1 (DEC:161); D55 item 5 (DEC:229) "One count per score (D51 call 1) must not bring back over-limit paths below the best score."
- **Code today:** by reading, engine.cpp:1243-1284.
- **Verdict:** PASS
- **Why:** Question named the 4-versus-8 split, and code gives one count for finished paths.

### 2026-10-03 98: empty record's best score

- **Splits:** Best score of a record with no paths: `hydra_batch` "-" vs hydra_replay JSON and hydra_bench 0. Today replay JSON `score` is null (`tools/replay_json.cpp:150-151`), bench prints "-" (`tools/bench.cpp:107-109`), batch prints "-" (`src/cli/batch.cpp:206`).
- **How it left:** a code-only call on the P7 sheet.
- **User's words:** "An empty record's best score reads null in its JSON and "-" in text (98). hydra_bench does the same." (P7:153), approved by DEC:194 (list includes 98) after "all recommended." The line names the new values; the old 0 is not written in the sentence.
- **Code today:** as above, by reading.
- **Verdict:** PASS
- **Why:** Both outputs the user sees (null, "-") are named, the old 0 is implied; it is dev-tool output only. Thin but sufficient.

### 2026-10-03 99: report numbers grouped by browser locale

- **Splits:** Thousands separator on report pages: `toLocaleString()` (browser locale) vs C++ `group_thousands` (comma). Today the only `toLocale` call in src is `src/app/html_page.cpp:318` `toLocaleString('en-US')`.
- **How it left:** question (P35 Q12).
- **User's words:** "**12. Counts next to nouns, and commas.** (14, 99) ... In a German-language browser the report pages print 1.234 under a subtitle saying 1,234. Recommended: one rule everywhere: singular at exactly 1, commas from 1,000, whatever the browser's language." (P35:66-69), D48: "Counts are singular at 1 with commas from 1,000, whatever the browser's language (Q12)." (DEC:130).
- **Code today:** by reading (grep `toLocale` over src: one hit).
- **Verdict:** PASS
- **Why:** Both separators (1.234 vs 1,234) were in the question.

### 2026-10-03 100: zero time signature

- **Splits:** A time signature with top number 0: .chart skipped it vs .mid crashed. Today `apply_timesig` (`src/core/song.cpp:58-59`) `if (numerator == 0) return;` for both formats.
- **How it left:** question in chat (step-1 plan Q1), recorded D6.
- **User's words:** Chat Q "Q1: A chart whose timing makes a measure or beat last zero, negative or infinite time (e.g. a negative .chart tempo). Refuse the chart, or ignore the bad line?" chose "Refuse with error (Recommended)" (card `docs/sessions/2026-10-03-cebf3006.md`:25 keeps the question's start and the chosen label; the rest of the question text was not kept). The written plan Q1 Recommended: "refuse the chart with a plain error naming the tick. The one exception is a time signature with a top number of 0, which is ignored in both formats, as `.chart` already does." (`docs/superpowers/plans/2026-10-03-step1-engine-stores-what-displays-guess.md`, line 66; removed, `git show 444631e6:docs/superpowers/plans/2026-10-03-step1-engine-stores-what-displays-guess.md`). D6: "A time signature with a top number of 0 is ignored in both formats, as `.chart` already does." (DEC:17).
- **Code today:** song.cpp:58-59, by reading.
- **Verdict:** PASS
- **Why:** The Recommended line names both formats. Caveat for the main session: the session's record (now card `docs/sessions/2026-10-03-cebf3006.md`:25) keeps only the chat question's first sentence, and the option text was not kept, so I could not confirm the user saw the zero-numerator exception inside the option text itself; the written plan's Q1 holds it.

### 2026-10-03 101: .sng preview clip in the mix

- **Splits:** Is `preview.*` part of the song mix: loose folder dropped vs .sng kept. Today `is_song_stem` (`src/app/preview_source.cpp:149-151`) drops stem "preview" and both container walks call it (preview_source.cpp:124 and 161).
- **How it left:** question (P7 Q21).
- **User's words:** "**21. Which files count as song audio.** (74, 101) Today a loose folder's preview.ogg is left out of the mix, but a .sng's preview.ogg is mixed into the song. ... Recommended: preview clips are left out everywhere, and both checks use the one "is this playable audio" rule." (P7:119-121), D51 call 21 (DEC:187).
- **Code today:** by reading. I did not open line 124's function name to confirm it is the .sng walk.
- **Verdict:** PASS
- **Why:** Both sides in the question; one predicate now.

### 2026-10-03 106: "records across" in batch and report

- **Splits:** The batch line counted every stored row; the report counted current-setting Ready rows. Today neither line exists in that form: `git log -S"Store now holds"` shows commit f772582d removed it; report subtitle is `counted(out.songs, "chart", "charts") + " — " + shown + " — " + cap_label` (`src/app/report.cpp:333-334`); `src/cli/batch.cpp` prints no store count.
- **How it left:** Q32 approved a rewording ("hydra_batch's closing line says "rows for every setting"", P35:543), then later D79 item 2.
- **User's words:** D79 item 2: "hydra_batch's "Store now holds N records across M songs, rows for every setting" line is removed. That number was every stored row at every setting, not a count of library charts." (DEC:354), where "The user took the recommended answer to all four questions" (DEC:352).
- **Code today:** by reading and `git show f772582d` (message says "the Store now holds line is gone").
- **Verdict:** GONE
- **Why:** f772582d (2026-10-05, D78 B2-B7 batch work, ruled as D79 item 2) deleted the batch line, so there is no second count to disagree with the report's.
- **Side picked without a ruling:** none; D79 item 2 names the removal.

### 2026-10-03 126: Preview re-parses a changed chart

- **Splits:** Stored path drawn on a freshly parsed (changed) chart vs the stored record. Today the Preview shows a message when the chart changed: `src/ui/preview_tab.cpp:465` "This chart changed since it was analyzed. Click the song again to see its path."
- **How it left:** question (P7 Q18).
- **User's words:** "**18. The chart file changed since the library scan.** (126) Today the Preview draws the stored path on the new notes, so activations can land on the wrong notes. Recommended: open the chart, hide the path overlay, and show one line: "This chart changed since it was analyzed. Analyze it again to see its path."" (P7:107-110), D51 call 18 (DEC:184).
- **Code today:** by reading preview_tab.cpp:465 only; I did not open the md5 comparison or overlay hiding.
- **Verdict:** PASS
- **Why:** Question named both behaviours; the message exists. The last sentence differs from the ruled wording ("Click the song again" vs "Analyze it again"); see side notes.

### 2026-10-03 129: headline stars from stored cache

- **Splits:** None today: score from the record and stars from the `results.stars` column agree; a latent drift guarded by the results stamp. Details headline reads the summary (`src/ui/details_panel.cpp:132`).
- **How it left:** recorded as-is (P7 Q13).
- **User's words:** "The library's star count comes from the stored summary, which a re-analysis refreshes." (P7:74), D51 call 13: "The stars column is a cache of `path_stars` (129)." (DEC:174).
- **Code today:** by reading details_panel.cpp:132.
- **Verdict:** PASS
- **Why:** The finding showed no live disagreement; the ruling names the cache and the code is the cache.

### 2026-10-03 133: hardest timing stored vs recomputed

- **Splits:** The library `squeeze<=N` filter reads the stored column vs report/Paths button recomputing. Today the report reads `p.hardest_ms` (`src/app/report.cpp:54`) from a stored path row (`src/store/record_store.cpp:302`), and the library reads the summary column (`src/store/record_store.cpp:351`, `src/app/library_query.cpp:420`). The Paths-tab button side (`build_path_buttons`) I did not re-open.
- **How it left:** triage "fixed" ("phase 7 wave 1, merged to main 6c65208"). No question cites 133: I searched P35/P7/phase-6 headings and the concept-audit questions for the id and found none; DEC has no 133.
- **User's words:** None found; searched the three phase question files, `docs/audit/2026-10-11-concept-audit/questions.md` and DEC for "133".
- **Code today:** report and library now read the stored column, by reading.
- **Verdict:** GONE (tentative)
- **Why:** The two sides I checked both read the stored column. The Paths-tab button was not rechecked, so the main session may want one more look.
- **Side picked without a ruling:** the stored column won (report.cpp:54, record_store.cpp:302); no ruling found, though the triage owner was marked "undecided".

### 2026-10-03 R7.1: "time left" wording

- **Splits:** "about 1:30 left" (batch strip) vs "about 2 min left" (Preview loader). Today both call `app::time_left_text` (`src/app/display_format.h:58-59`, "about " + `format_duration` + " left"): batch at `src/ui/library_dialogs.cpp:453`, Preview at `src/ui/preview_load_job.cpp:286-288`. The Preview keeps its 3 s gate (`gated_time_left_text`).
- **How it left:** question (P35 Q20).
- **User's words:** "**20. Time text.** (318, R7.1) ... The batch strip says "about 1:30 left" while the Preview loader says "about 2 min left". Recommended: the clock reads 1:00.000. Both use the batch strip's "about m:ss left" form, and the Preview keeps its 3-second wait before showing it." (P35:98-100), D48 (DEC:133 "Q15 to Q21 take their recommended forms").
- **Code today:** by reading.
- **Verdict:** PASS
- **Why:** Question showed both wordings and the 3 s wait; the code matches. The two different estimate sources (chart average vs byte rate) are different inputs and the question left them alone.

### 2026-10-03 R7.2: "Open automatically"

- **Splits:** Hint says finished batches vs setting also opened every leaderboard comparison. Today the setting is read once, `src/ui/app_state.cpp:1015` (`settings.auto_open_report`, batch path report only); `grep auto_open src` shows no read in `dm_jobs.cpp`. Hint: `src/ui/library_dialogs.cpp:552` "Open the path report as soon as it's built."
- **How it left:** question P7 Q23 ("keep the behaviour and fix the words to "Open each report in your browser as soon as it's built."", P7:131, DEC:189), then superseded by D103 item 13: "**"Open automatically" covers only the path report.** Its hint becomes "Open the path report as soon as it's built."" (DEC:464).
- **User's words:** D103 was a user-asked redesign ("The user asked for the path report to move from a generated HTML page into Hydra", DEC:452); commit 49799760 "Both reports live in memory; nothing writes or opens a page".
- **Code today:** by reading.
- **Verdict:** GONE
- **Why:** 49799760 removed the browser page; the comparison no longer opens anything on this setting, so the sides no longer differ. The `src/app/config.h:187-190` comment still says it "covers every report ... ReportJob and DmReportJob both read it" (stale; see side notes).
- **Side picked without a ruling:** none; D103 item 13 names "only the path report".

### 2026-10-03 R7.3: scan during a batch

- **Splits:** Toolbar Scan off during batch vs "Scan now"/"Rescan library" started a scan. Today one `can_scan()` (`src/ui/app_state.cpp:564-566`: `!settings.chartfolders.empty() && !scan_job && !batch_running()`), used by the toolbar (`library_toolbar.cpp:69`) and `start_scan` (app_state.cpp:627-630) which sets `kBatchRunningStatus` "A batch is running." (`app_state.h:500`).
- **How it left:** question (P7 Q24).
- **User's words:** "**24. Scanning during a batch.** (R7.3) Today the toolbar's Scan button is off during a batch, but "Scan now" and "Rescan library" elsewhere still start a scan. That scan's window then covers the batch strip. Recommended: block both during a batch, with the status line saying "A batch is running."" (P7:133-135), DEC:190.
- **Code today:** by reading.
- **Verdict:** PASS
- **Why:** Both doors named; code blocks both with that exact status line.

### 2026-10-03 R7.5: Offset vs delay number strictness

- **Splits:** `.chart` Offset text "500ms" read as 500 s and "nan" accepted vs song.ini/.sng delay dropped. Today both use `parse_finite_number` (`src/core/strutil.cpp:80-92`, whole text must be a number): `src/parse/song.cpp:1825` and `src/app/preview_source.cpp:113-114`.
- **How it left:** approved in a small-items batch in chat (D27).
- **User's words:** "**Offset and delay numbers (R7.5).** Today `"500ms"` is read as 500 seconds, and `nan` is accepted. One strict number helper would read both. No library file is affected." (the batch text was not kept); Q "Approve the small-items batch as recommended (60, 61, 258/319, 331, R7.5, R7.7, 77 fixes; 141 and 320 recorded only)?" chose "Approve all (Recommended)" (card `docs/sessions/2026-10-04-bbf41dc4.md`:27). D27: "one strict number helper for `.chart` Offset and song.ini delay (R7.5)" (DEC:65).
- **Code today:** by reading.
- **Verdict:** PASS
- **Why:** Wording is thin (it names the .chart misreading, not that a unit on a delay was already dropped) but the rule asked, one strict reader for both, is what the code does. A unit-suffixed `.chart` Offset now counts as absent (song.cpp:1820-1822).

### 2026-10-03 R7.6: negative .chart tempo

- **Splits:** Lookups assume ms never falls as ticks rise vs a `.chart` tempo of 0 or below. Today `check_timing_maps` refuses it: `src/core/song.cpp:44-46` `if (!std::isfinite(bpm) || bpm <= 0.0) throw ... "is not above 0 BPM"`.
- **How it left:** question in chat, D6.
- **User's words:** "Q1: A chart whose timing makes a measure or beat last zero, negative or infinite time (e.g. a negative .chart tempo). Refuse the chart, or ignore the bad line?" chose "Refuse with error (Recommended)" (card `docs/sessions/2026-10-03-cebf3006.md`:25; the rest of the question text was not kept). D6 (DEC:17): "Timing that makes a measure or beat last zero, negative or infinite time refuses the chart with a plain error naming the tick." Also D4's condition: "there should never be a situation when the scale cant be computed" (DEC:13).
- **Code today:** by reading.
- **Verdict:** PASS
- **Why:** The question named the negative tempo and both options; code refuses. The option "clamp" from the audit was not offered, but "ignore the bad line" was the alternative put to the user.

### 2026-10-03 R7.8: FLAC header length 0

- **Splits:** Preview trusts the header length (silent stem) vs old mixer decoded to the end. Today `src/audio/ma_reader.cpp:184` comment "A header length of 0 is counted (CountedLength)".
- **How it left:** question (P7 Q19).
- **User's words:** "**19. A FLAC whose header says its length is 0.** (R7.8) ... Recommended: read the file once on open to count its length, so it plays. Every other file keeps today's fast open." (P7:111-113), DEC:185.
- **Code today:** by reading the comment only; I did not read the counting code.
- **Verdict:** PASS
- **Why:** Both behaviours named; the code names the counted length. Only the 0-length case was asked; the "header promises more than it decodes" row in the audit table was not (side note).

### 2026-10-03 R7.9: Opus after a decode error

- **Splits:** Seek back after a damaged Opus stem: stays silent vs every other format revives. Today `src/audio/opus_reader.cpp:282-285`: "No early return on failed_: a seek to before the damage plays again".
- **How it left:** question (P7 Q20).
- **User's words:** "**20. A damaged Opus stem after you scrub back.** (R7.9) Today a damaged Opus stem goes quiet at the damage and stays quiet after any scrub back, until you reopen the chart. Every other format comes back when you scrub to before the damage. Recommended: Opus behaves like the others." (P7:115-117), DEC:186.
- **Code today:** by reading.
- **Verdict:** PASS
- **Why:** Both sides named; code matches.

### 2026-10-03 R7.10: 500 ms window anchor for SP past the chart end

- **Splits:** Tail backend rows measured from the song's last note (graph) vs from the SP end (display). Today the engine measures each tail row against `final_sp_end` and filters with `within_squeeze_window` (`src/search/engine.cpp:1985-1990`).
- **How it left:** question in chat (call 5), D5.
- **User's words:** Q "Call 5 (R7.10): for SP that outlasts the song, measure the 500 ms backend window from where?" chose "From the SP end (Recommended)" (card `docs/sessions/2026-10-03-cebf3006.md`:23), options as written in the session ("Measure from the SP end (recommended)" / "Measure from the last note."; the option text was not kept). D5: "the 500 ms backend window is measured from the SP end, the same as every other activation." (DEC:15).
- **Code today:** by reading engine.cpp:1980-1993.
- **Verdict:** PASS
- **Why:** Both anchors were in the options and the code uses the chosen one.

<!-- next -->

## Not reached

None. Weak spots to recheck: 133 (Paths-tab button side not re-opened), 101 (line 124 not confirmed as the .sng walk), 126 (md5 comparison and overlay hiding not read), R7.8 (counting code not read).

## Side notes

- `src/app/config.h:187-190`: the comment on `auto_open_report` says it "covers every report Hydra builds ... ReportJob and DmReportJob both read it (D51 call 23)". Code (`src/ui/app_state.cpp:1015`) and D103 item 13 (DEC:464) say only the path report. The comment describes a rule that no longer exists.
- `src/ui/preview_tab.cpp:465`: "...Click the song again to see its path." vs the ruled text "Analyze it again to see its path." (P7:109, DEC:184). I did not find a later ruling for the change.
- `src/app/path_report_view.cpp:107-108`: the Timing column help says "Beyond means more than twice the hit window", a fixed-prose copy of 2W beside `nominal_budget_ms` (squeeze_rating.cpp:70-72). Same answer today; would drift if the budget formula changed.
- Finding 95: still-running over-limit paths keep their own tie key (engine.cpp:1259, D55 item 5); this was a ruled refinement, not a leftover.
