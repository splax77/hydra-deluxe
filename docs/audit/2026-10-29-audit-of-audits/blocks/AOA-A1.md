# AOA-A1: derivation audit 2026-10-03, "assumption" findings 302 to 326

Checkout: audit-of-audits at main 1fc55723. Scout agent id a6e6caa0874e6cfb2.

### Derivation audit 302: Dynamics tab percentages round down

- **Splits:** None as of the audit's own finding beyond "Dynamics tab truncates vs progress bars round": the two progress overlays (`src/ui/widgets.h:96`, `%.0f%%`) already rounded to nearest, so the only odd one out was the truncating Dynamics tab. Today every share goes through `format_percent` (`src/app/display_format.cpp:50`), and the progress overlay's `%.0f%%` also rounds to nearest, so both answer "nearest whole percent".
- **How it left:** fixed. Closure's words: "Phase 3 K5/O2: Dynamics percents round to nearest through format_percent (M_D 57dcf23)." (docs/audit/2026-10-03-fix-triage.json, row "302", `status_evidence`).
- **User's words:** "**30. Percentages.** (302, 51)" / "Today the Dynamics tab cuts decimals off, so 12 of 453 kicks reads 2% and 999 of 1000 reads 99%. The leaderboard page can say 100.00% for a score 10 points short of optimal." / "Recommended: round to nearest everywhere (3%, 100%, and 99.99% for the near miss)." (docs/audit/2026-10-04-phase-3-5-questions.md:142-144), answered as D48: "The user answered "all recommended" to all 33 questions in `docs/audit/2026-10-04-phase-3-5-questions.md`. Each question's "Recommended" line is the decision." (docs/audit/2026-10-03-fix-decisions.md:118) and "- The report items Q28 to Q31 take theirs." (fix-decisions.md, D48 list).
- **Code today:** `dynamics_share` (`src/app/dynamics_breakdown.cpp:159`) calls `format_percent(part, total, 0)`, which uses `percent_steps` = `(2 * mag + total) / (2 * total)` (`src/app/display_format.cpp:50-58`): 12 of 453 gives 3%, 999 of 1000 gives 100%. By reading. Other percent writers: `src/ui/widgets.h:96` progress overlay `%.0f%%` (nearest). The Preview solo percent `solo_percent` (`src/app/preview_view.cpp:654-660`) truncates on purpose to copy the game (see Side notes); it is a different question (the game's own number).
- **Verdict:** PASS
- **Why:** The question named the real behaviour (cut decimals, 2% and 99%) and the recommendation (round to nearest everywhere); the code now rounds to nearest through one formatter, and the other remaining rounding sites agree.

### Derivation audit 304: a note exactly 3.0 ms after the SP end

- **Splits:** None: one owner (`core::counted_without_squeeze`, `src/core/backend_value.h:41-43`). Every reader goes through it: `BackendSqueeze::summarystr` (`src/core/model.cpp:347,358`), `path_view.cpp:123,351`, `squeeze_rating.cpp:96`, `replay.cpp` via `core::paid_by_sp`. The edge is `offset_ms < leeway_ms`, strict, in all of them.
- **How it left:** a question the user answered in chat (no question-file entry; asked with a chat question tool). Closure's words in the triage: "phase 7 recheck (2026-10-04, D51): already fixed by step 2 T10/T11 (merged 7266643)." (docs/audit/2026-10-03-fix-triage.json, row "304", `status_evidence`). The real closure is D29.
- **User's words:** the chat question "Finding 304: a backend note at exactly +3.0 ms after the SP end?" answered "Keep strict (Recommended)" (card `docs/sessions/2026-10-04-bbf41dc4.md`:28), recorded as D29: "304: a backend note at exactly +3.0 ms stays uncounted (strict); record it and pin it with a test." (docs/audit/2026-10-03-fix-decisions.md:69). The question's subject is the exact edge, and it names the two outcomes (counted or not at exactly +3.0).
- **Code today:** `counted_without_squeeze(offset, leeway)` is `paid_by_sp_walk(offset) || offset < leeway` (`src/core/backend_value.h:41-43`); its comment reads "Exactly the leeway after it is not counted; the edge is strict (decision D29)." (backend_value.h:39-40). At exactly 3.0 with the default leeway 3.0 the plain row reads "Hard (uncounted)" (`src/core/model.cpp:358-360`). By reading.
- **Verdict:** PASS
- **Why:** The user was asked about this exact edge, chose strict, and the single shared test in the code is strict. The leeway number itself is the user's own setting.

### Derivation audit 305: early-fill badge on an activation that skips fills

- **Splits:** An activation with no required squeeze: its path button shows no timing, while its row can show an "early fill N ms" badge. The question text named exactly this pair. Today the badge side is `Activation::badge_timings` (words in `src/app/path_view.cpp:50-75`), the button side is the activation's required-only `difficulty()`; the badge shows an early fill always (D261).
- **How it left:** question. Closure's words: "Phase 3 O1/K2: the activation owns its "needs a squeeze" rule and the badge reads it (M_D 57dcf23)." (docs/audit/2026-10-03-fix-triage.json, row "305").
- **User's words:** "**10. Early-fill badge on an activation that skips fills.** (305)" / "Today an activation with no required squeeze can show a grey "early fill 20 ms" badge while its path button shows no timing. The 2026-09-27 plan said there should be no badge." / "Recommended: keep the badge and record the choice. That timing decides whether the first fill appears, which is what the skip count means." (docs/audit/2026-10-04-phase-3-5-questions.md:56-58), answered as D48: "- The early-fill badge stays and is recorded (Q10)." (docs/audit/2026-10-03-fix-decisions.md, D48 list). Later D261: "The user ruled, in chat on 2026-10-24: "the fix is to remove the exception and make the badge always show."" (fix-decisions.md:988).
- **Code today:** `activation_badges` (`src/app/path_view.cpp:69-76`) words every timing `Activation::badge_timings()` lists, early fill included; no skip-fill exception remains. By reading.
- **Verdict:** PASS
- **Why:** The question quoted both sides (badge shown, button shows no timing) and the plan that said the opposite; the user said keep the badge, and the code keeps it, later even for a fill with time to spare.

### Derivation audit 306: the fixed 10 ms band on backend rows (known REOPEN)

- **Splits:** Where does "Standard" end on the high side (after the SP end)? Plain row: under the leeway, 3 ms by default (`counted_without_squeeze`, `src/core/backend_value.h:41-43`, called from `src/core/model.cpp:358`) vs squeezed-out row: under 10 ms (`kBackendInnerBandMs`, `src/core/model.h:100`, used at `src/core/model.cpp:343,353`). The low side (-10) is the same number on both rows (`model.cpp:353` and `model.cpp:356`).
- **How it left:** fixed (the 10 got a name) after a question. Closure's words: "Phase 3 O1: kBackendInnerBandMs names the 10 ms backend band (M_D 57dcf23)." (docs/audit/2026-10-03-fix-triage.json, row "306").
- **User's words:** "**4. The ±10 ms band on backend rows.** (306, weighty)" / "Today a backend row turns "Easy SqOut" at +10 ms and "Easy" at -10 ms, whatever your hit window or leeway." / "Recommended: keep 10 ms as a named number. Players compare these labels with Clone Hero, and nothing in the game ties them to the leeway." (docs/audit/2026-10-04-phase-3-5-questions.md:30-34), answered as D48: "- The ±10 ms backend band stays a named constant, not tied to the leeway (Q4)." (docs/audit/2026-10-03-fix-decisions.md, D48 list). The text never says that the plain row's Standard ends at the leeway (3 ms) while the squeezed-out row's Standard SqOut ends at 10.
- **Code today:** `BackendSqueeze::summarystr` (`src/core/model.cpp:339-361`): a plain row at +5 ms (W=85, leeway 3) reads "Hard (uncounted)", the squeezed-out row at +5 ms reads "Standard SqOut (uncounted)". By reading.
- **Verdict:** REOPEN
- **Why:** Both edges are still in the code and the question described only the 10.
- **Examples:** Low end, -12 ms: plain `off < -band` gives "Easy", squeezed-out gives "Hard SqOut". High end, +80 ms with W=85: plain "Hard (uncounted)" (`off < w`), squeezed-out "Easy SqOut (uncounted)" (`off < w`). Gap, +5 ms: plain "Hard (uncounted)" (`counted_without_squeeze` is false since 5 is not < 3), squeezed-out "Standard SqOut (uncounted)" (`off < band` with band 10, tag from the same `counted_without_squeeze`). Same note, same question ("how far past the SP end"), two different labels.

### Derivation audit 307: an exact tie with the cap ceiling is not a clamp

- **Splits:** None: one owner. `ScoreGraph::extend_deacts` (`src/search/graph.cpp:296-318`) decides it with `const bool clamped = ceiling.ticks() < ext.ticks();`; the stored `clamp_tick` is only read elsewhere (`src/app/path_view.cpp:296-299` for the overfill text). The Preview meter has no clamp flag of its own (`build_sp_meter_curve`, `src/app/preview_view.cpp:124-`).
- **How it left:** a question the user answered with a batch "all recommended". Closure's words: "Read-only scan 2026-10-05 at 993df71: D51 call 6 keeps strict-earlier clamp counting; the graph.cpp comment states the tie rule." (docs/audit/2026-10-03-fix-triage.json, row "307").
- **User's words:** "**6. A phrase that fills the SP meter exactly to the cap: is that an overfill?** (307)" / "Today it isn't. The meter is full, nothing is lost, and no overfill note appears. No decision records this." / "Recommended: keep, and record it." (docs/audit/2026-10-04-phase-7-questions.md:40-42), answered as D51: "The user answered "all recommended" to every question in `docs/audit/2026-10-04-phase-7-questions.md`, including 4b. Each question's Recommended line is the decision." (docs/audit/2026-10-03-fix-decisions.md:156) and "6. A phrase that fills the meter exactly to the cap is not an overfill (Q6, 307)." (fix-decisions.md:166).
- **Code today:** at an exact tie `ceiling.ticks() < ext.ticks()` is false, so `clamped` is false and no overfill warning is stored (`src/search/graph.cpp:314`). By reading.
- **Verdict:** PASS
- **Why:** The question describes the real rule (a tie is not an overfill, no overfill note), and the code does exactly that.

### Derivation audit 308: Preview gauge cap for a song with no result yet

- **Splits:** None now. The finding's two sides (hard 4 vs Settings cap) are resolved in one place: `preview_tab.cpp` passes `app.settings.sp_cap` always (`src/ui/preview_tab.cpp:411-426`), and `build_unanalyzed_sp_meter_curve` uses `sp_meter_cap_of(sp_cap)` (`src/app/preview_view.cpp:96-112`).
- **How it left:** a question the user answered with a batch "all recommended". Closure's words: "Phase 3 K4a: an unanalyzed song's Preview gauge uses the Settings SP cap (M_D 57dcf23)." (docs/audit/2026-10-03-fix-triage.json, row "308").
- **User's words:** "**24. Gauge cap for a song with no result yet.** (308)" / "Today the gauge pins at 4 bars whatever your SP cap setting says." / "Recommended: it uses your Settings cap, the one the next analysis will run at." (docs/audit/2026-10-04-phase-3-5-questions.md:116-118), answered as D48: "Each question's "Recommended" line is the decision." (fix-decisions.md:118) and "- The Preview items Q22 to Q27 take theirs." (fix-decisions.md:134; Q24 is in that range).
- **Code today:** `src/ui/preview_tab.cpp:410-415` reads `const int sp_cap = app.settings.sp_cap;` for every selection, Ready record or not, with the comment "A chart with no record yet previews at the cap the next analysis will run at (D48, Q24)." By reading.
- **Verdict:** PASS
- **Why:** The question named the real behaviour (pinned at 4) and the answer is what the code does.

### Derivation audit 309: leaderboard "matched" status and the percent rule

- **Splits:** None: one owner (`collect_dm_rows`, `src/app/dm_report.cpp:80-128`). The status and the percent are decided together there; the page script and `tally_dm_rows` read the status back.
- **How it left:** a question the user answered with a batch "all recommended", plus an earlier chat answer for the speed rule. Closure's words: "Phase 3 K1b: the leaderboard chips read Under, At and Above optimal (M_D 57dcf23)." (docs/audit/2026-10-03-fix-triage.json, row "309").
- **User's words:** "**29. Leaderboard and fill-comparison labels.** (309, 312)" / "Today a leaderboard score 50,000 under optimal wears a "Matched" chip (it means "found in your library", not "equals optimal")." / "Recommended: "Under optimal", "At optimal" and "Above optimal" chips, with the summary line to match." (docs/audit/2026-10-04-phase-3-5-questions.md:138-140), answered as D48 (fix-decisions.md:118, "The report items Q28 to Q31 take theirs." at :135). The speed half: chat question "Leaderboard page: how should off-speed scores count?" answered "Own 'other speed' status (Recommended)" (card 2026-10-04-bbf41dc4.md:25), recorded as D25: "Off-speed leaderboard scores get their own "other speed" status and leave the Matched, Above optimal and Points left on table totals. Avg % already skips them." (fix-decisions.md:61).
- **Code today:** `src/app/dm_report.cpp:107-124`: `s.score == opt` gives At, above gives Above, else Under; `if (!base) row.status = kStatusOtherSpeed;` and the percent is filled only `if (base && opt > 0)`. By reading.
- **Verdict:** PASS
- **Why:** Both halves of the finding (status names, speed rule) have a user answer that names them, and the code matches both.

### Derivation audit 310: average multiplier of 0.000x for a path with no scoring notes

- **Splits:** None: one formula (`Path::avg_mult`, `src/core/model.cpp:845-850`, returns 0.0 when `basescore == 0`) and two callers (`src/app/path_view.cpp:430`, `src/store/record_store.cpp:585`). A number nobody chose, not a split.
- **How it left:** a bullet in a "confirm today's numbers" question, answered "all recommended". Closure's words: "phase 4 (merge ae60cc5, 2026-10-04): CONTEXT.md records the 0.000x average multiplier." (docs/audit/2026-10-03-fix-triage.json, row "310"). The question lists the id only in a bullet, not in its heading.
- **User's words:** "**33. Confirm today's numbers so they get recorded.** Nothing changes on screen if you say yes." / "- An average multiplier of 0.000x for a path with no scoring notes, which never happens in real charts (310)." / "Recommended: confirm all." (docs/audit/2026-10-04-phase-3-5-questions.md:168,170,185), answered as D48: "- Every number in Q33 is kept as it is and recorded. That includes the 10 ms Path limit on, 50 ms backend hide off, and both path cut-offs (248 and 260)." (fix-decisions.md:137).
- **Code today:** CONTEXT.md records it ("A path's average multiplier reads 0.000x when it has no scoring notes, instead of dividing by zero (`Path::avg_mult`); real charts never hit this (D48, Q33).", CONTEXT.md:445-448). `avg_mult` still returns 0.0. By reading.
- **Verdict:** PASS
- **Why:** The question showed the exact output (0.000x) and the case (no scoring notes), the user confirmed it, and the code matches.

### Derivation audit 311: negative path depth in the INI and CLIs

- **Splits:** None now. The finding's split (GUI clamps, INI and CLIs crash) is gone: one range table in `Settings` (`src/app/config.cpp:182-211`, `number("depth_value", &Settings::depth_value, 0, kNoCeiling)`) serves `load_file` (`config.cpp:226-240`), the boxes and the CLI (`tools/replay.cpp:189-194`, `Settings::clamp`).
- **How it left:** a question the user answered with a batch "all recommended". Closure's words: "phase 7 wave 2, merged to main c8c4196 (M7-2c, PV/SE2/TM, 2026-10-05)." (docs/audit/2026-10-03-fix-triage.json, row "311").
- **User's words:** "**14. A hand-edited setting outside its range.** (322, 311, 31, 56, 72, 138)" / "- `depth_value=-1` crashes the search." / "Recommended: clamp, matching the boxes." (docs/audit/2026-10-04-phase-7-questions.md:84-91), answered as D51: "14. A hand-edited setting outside its range is pulled to the nearest edge of the range its box enforces, both on load and on typing. The Preview volume is 0 to 100 everywhere (Q14: 322, 311, 31, 56, 72, 138)." (fix-decisions.md:180).
- **Code today:** `depth_value=-1` or `--depth -1` lands on 0 through `pull_into_range` (`src/app/config.cpp:226-232`, `std::clamp(value, lo, hi)`). By reading.
- **Verdict:** PASS
- **Why:** The question named depth_value=-1 and the clamp; the code clamps to 0 on every entry point.

### Derivation audit 312: fill comparison labels a chart by which database holds a record

- **Splits:** None now. `collect_fill_rows` labels by record, not by score (`src/app/fill_report.cpp:270-287`): `else if (!new_rec) only 1.0; else if (!old_rec) only 1.1; else In both`.
- **How it left:** a question the user answered with a batch "all recommended". Closure's words: "Read-only scan 2026-10-05 at 993df71: collect_fill_rows labels by which database holds a record, with an 'in both' label that tally_fill_rows counts (D50 item 2). Fixed by 1b603a1." (docs/audit/2026-10-03-fix-triage.json, row "312").
- **User's words:** "**29. Leaderboard and fill-comparison labels.** (309, 312)" / "In the fill comparison, a chart whose 1.0 record has no paths is listed under "Only in 1.1 db"." / "Recommended: ... Fill-comparison rows are labelled by which database holds a record." (docs/audit/2026-10-04-phase-3-5-questions.md:138-140), answered as D50: "2. In the fill comparison (finding 312), a chart with a record in both databases but paths in only one is listed under "In both", with "no score" on the empty side." (fix-decisions.md:149).
- **Code today:** as above; the comment at `fill_report.cpp:271-275` says "When both rules hold a record but only one kept a path, the chart is in both (D50 item 2)". By reading.
- **Verdict:** PASS
- **Why:** The question described the exact misfiled case and the answer matches the code.

### Derivation audit 313: the 60 ms early-fill window

- **Splits:** None today. The single constant `kEarlyFillWindowMs` no longer exists. Two named numbers replace it: `kEarlyFillReachMs = 85.0` (legality, `src/core/model.h:137`, read by `fill_refuses`, `model.h:309`) and `kEarlyFillSlackMs = 60.0` (the E mark, `model.h:140`, read by `is_e0`, `model.h:313-315`). `is_e_critical` calls `is_e0(e_offset, 0)` (`src/core/model.cpp:478`), so the in-house copy is gone.
- **How it left:** a question, then superseded. Closure's words: "phase 7 wave 3, merged to main f4e352b (M7-3b, DOC, 2026-10-05)." (docs/audit/2026-10-03-fix-triage.json, row "313").
- **User's words:** "**7. The early-fill window is 60 ms.** (313)" / "This is how early SP can be ready before a fill and still bring the fill. 60 ms comes from the library survey: the hardest early fill on 18,773 charts was 57.7 ms. No decision records it." / "Recommended: keep 60 ms, and record it with that reason." (docs/audit/2026-10-04-phase-7-questions.md:44-46), answered as D51: "60 is fine because it doesnt change anything. lots of paths were getting cluttered with early fills that will never matter so i tightened it from 85 to 60." (fix-decisions.md:158). Then D250: "1. **Which side.** Only the orange side goes to 85 ms (`fill_refuses`). The white side, where an activation gets the `E` mark with nothing to time (`is_e0`), stays at 60 ms, so D51 call 7's clutter does not come back." (fix-decisions.md:868), after the user's words "change the early fill search to allow orange timing up to 85ms" (fix-decisions.md:867).
- **Code today:** `fill_refuses(e_offset)` is `e_offset < -kEarlyFillReachMs` (85) and `is_e0` is `e_offset < kEarlyFillSlackMs && skips == 0` (60). By reading.
- **Verdict:** GONE
- **Why:** Commit efd1457f (2026-10-15, "Early fills reach 85 ms on the orange side; the E mark's slack stays 60 ms (D250)") split the one window into the legality reach (85) and the E slack (60), each with the user's chat words behind it.
- **Side picked without a ruling:** none; D250 names both numbers and both sides.

### Derivation audit 314: a beat is a quarter note in every meter

- **Splits:** What "a beat" is in 6/8: the Preview beat lines and the position text's beat number step by the time signature's bottom number, an eighth note in 6/8 (CONTEXT.md:731-737, "Beat"; `SongTiming` denominator in `src/core/timing.h:112-131`) vs the generated fill's 4-beat spawn deadline and half-beat distance, which count quarter notes in every meter (`kCh11FillLeadBeats * timing.tick_resolution()`, `src/search/graph.cpp:69`; `src/parse/song.cpp:374`). The CONTEXT.md entry records the difference as Clone Hero's own code ("both as Clone Hero's code does").
- **How it left:** a chat question the user answered "Not now" (D26), then a settled-by-decision status, then a later user ruling for the beat lines. Closure's words: "Read-only scan 2026-10-05 at 993df71: D26 keeps a quarter-note beat in every meter as unverified; ADR 0023 and CONTEXT.md record it." (docs/audit/2026-10-03-fix-triage.json, row "314").
- **User's words:** chat question "Live Clone Hero check for fill length, 6/8 beats and whether authored fills stop generation ..." answered "Not now" (card `docs/sessions/2026-10-04-bbf41dc4.md`:26), recorded as D26: "No live Clone Hero check now. Generated-fill length and 6/8 beats keep today's rule, written down as unverified against Clone Hero." (fix-decisions.md:63). Later, in the session 28fad19c, Claude told the user "The fill generator counts in quarter notes in every meter (proven). Clone Hero's beat lines, though, step by the bottom number, so 6/8 gets a line every eighth note" (that message was not kept) and asked "In 6/8 and other x/8 meters, Clone Hero draws a beat line every eighth note (step = resolution x 4 / bottom number). Hydra's Preview draws quarter notes in every meter. Match Clone Hero?" The user chose "Match CH (Recommended)" (card `docs/sessions/2026-10-12-28fad19c.md`:34, which keeps the label and the question's start; the full question text was not kept), recorded in fix-decisions.md:813 (D227) and "go with recommended for both." (card `docs/sessions/2026-10-12-28fad19c.md`:39).
- **Code today:** fill deadline `fill_start_tick - kCh11FillLeadBeats * tick_resolution()` (`src/search/graph.cpp:67-70`); CONTEXT.md:562-565 cites Clone Hero's code at 0x20DF7B7. Beat lines step by the denominator. By reading (the disassembly itself not re-read).
- **Verdict:** PASS
- **Why:** The 6/8 question was asked and answered by name, then the fill side was read from Clone Hero's code and the beat-line side was ruled by the user with both sides stated in the chat text before the question.

### Derivation audit 315: fill placement ties, truncation and drops

- **Splits:** None: one owner per rule (`Song::check_activations` and its two generators, `src/core/song.cpp:105-185`; `place_authored_fills`/`apply_fill_end`, `src/parse/song.cpp:342-388`). The 1.0 generator and the 1.1 generator both send a distance tie to the later chord (`song.cpp:150` `dist <= it->second.bestdist`).
- **How it left:** partly asked in chat, partly "code-only" recorded as is. Closure's words: "AF2 (D74), merged to main 9c3622d (2026-10-05): the generated-fill tie and whole-tick rules are recorded in ADR 0023 and CONTEXT.md, pointing at Song::check_activations, unverified against Clone Hero like D26's." (docs/audit/2026-10-03-fix-triage.json, row "315").
- **User's words:** authored fills: chat question "Fill B: place each authored fill on its own after the whole chart is read ..." answered "B1 (Recommended)" (card `docs/sessions/2026-10-04-bbf41dc4.md`:31), recorded as D30: "A tie goes to the later chord, and a fill with no chord in bounds is dropped." (fix-decisions.md:71); "one authored fill stops generation" is named in the D26 question the user answered "Not now" (card :26). The generated-fill tie and whole ticks: D74, "Code-only calls, approved as recommended: ... The generated-fill rules that were never written down (a distance tie goes to the later chord; distance and fill length are cut to whole ticks) are recorded as they are, unverified against Clone Hero like D26's (315)." (fix-decisions.md:326). The user saw these two rules named in a status message ("which chord wins a distance tie, and that distances get rounded to whole ticks"; that message was not kept) and answered "continue and commit when everything is done" (card `docs/sessions/2026-10-05-18216767.md`:17). That is not a question with options.
- **Code today:** the 1.1 generator now follows Clone Hero's disassembled code ("a tie going to the later chord", limit "plus one tick (inc at 0x20D2C91)", `src/core/song.cpp:176-183`), so for 1.1 the rule is a source fact. The 1.0 generator keeps the same tie rule with no source (`song.cpp:150`); ADR 0023 lists it as unchecked (docs/adr/0023-chart-rules-follow-clone-hero-code.md:98-99). By reading.
- **Verdict:** PASS
- **Why:** The rules have a user answer (D30, D26) or are now read from Clone Hero's code (1.1); only the 1.0 generator's tie rests on D74's "recorded as they are". See Side notes.

### Derivation audit 316: tied paths merge without comparing when their meter reached 2 bars

- **Splits:** None: one owner. `Engine::reduce_iteration_paths` (`src/search/engine.cpp:1409-1456`) builds the group key. For a waiting path the key holds `ready_class(p, ...)` (which upcoming fills the SP-ready time decides), not the raw `sp_ready_ms`. A question about a merge rule, not two parts answering differently. `skipped_e_offset` is still not in the key; each tied variant keeps its own stored facts (ADR 0022, `src/search/engine.cpp:1303`).
- **How it left:** a chat question the user answered. Closure's words: "Read-only scan 2026-10-05 at 993df71: reduce_iteration_paths puts ready_class in the group key for waiting paths; recorded as D40 in ADR 0014. Fixed by cd17920." (docs/audit/2026-10-03-fix-triage.json, row "316"). The real ruling is D35.
- **User's words:** "Q: Adopt the ready-time key? 7 best scores go up at cap 2, none go down, cap 4 and speed unchanged." chose "Adopt it (Recommended)" (card `docs/sessions/2026-10-04-a9a461da.md`:20, which keeps the label and the question's start; the rest of the question line was not kept), recorded as D35: "Adopt the cheaper ready-time key (`claude/s1-rk`). Tied paths now fold only when the same upcoming fills fall on the same side of their SP-ready cut-offs, so Hydra no longer keeps a path that misses a fill and drops one that gets it." (docs/audit/2026-10-03-fix-decisions.md:81). The question text I could read is one line plus the option label; the option descriptions are not in the transcript.
- **Code today:** `key_value = (key_value << 32) | ready_class(p, no_skips_ ? &passable : nullptr);` (`src/search/engine.cpp:1455`) with the comment "While waiting, also which upcoming fills its SP-ready time decides (ready_class), not the time itself" (:1411-1413). By reading.
- **Verdict:** PASS
- **Why:** The user was asked whether to adopt a key that includes the ready time, saw the measured effect, said yes, and the code has that key. See Side notes about the thin question text.

### Derivation audit 317: Auto cleanup recognises only default-ladder rows

- **Splits:** None today. The finding's two sides (default-ladder rows deleted, custom-ladder rows left Stale) no longer exist: `retired_auto_fingerprint` and `delete_auto_results` are not in `src/` (a search of `src` for `retired_auto` and `delete_auto_results` finds nothing).
- **How it left:** a "record as is" bullet in a batch question, answered "all recommended". Closure's words: "Read-only scan 2026-10-05 at 993df71: D51 call 13 records that custom-ladder Auto rows read Stale until re-analyzed." (docs/audit/2026-10-03-fix-triage.json, row "317").
- **User's words:** "**13. Record these as they are.** (129, 317, 321, 323, 325, 335)" / "- Old Auto results made with a custom cap ladder read Stale until re-analyzed. Only default-ladder ones are deleted at first start." / "Recommended: record all five as they are." (docs/audit/2026-10-04-phase-7-questions.md:72,76,80), answered as D51: "- Custom-ladder Auto rows read Stale until re-analyzed (317)." (fix-decisions.md:175).
- **Code today:** the first-start Auto cleanup is gone; an old library file is classified (`classify_library_file`, `src/store/library_file.h`) and set aside or refused by `OldFilePolicy`. By reading (`git show 59b77bc7`, commit message: "The copy upgrade, its recovery, the schema 2/3 rebuild, the Auto cleanup and retired_auto are gone.").
- **Verdict:** GONE
- **Why:** Commit 59b77bc7 (2026-10-14, "one owner for old library files (classifier, set-aside, refuse)", task D115-A1-6a) removed the cleanup, so there is no default-ladder versus custom-ladder difference left. An old file older than 2.2.0 is now set aside or refused whole, under the D115 design.
- **Side picked without a ruling:** the whole-file set-aside replaced both the delete-default-ladder-rows rule and the keep-custom-ladder-rows rule. I did not check whether the user's D115 words name Auto results; the cleanup's removal rode along inside the D115 one-record-per-chart build.

### Derivation audit 319: charts with no time signature read as 4/4

- **Splits:** None now. The finding's three spellings of the 4/4 default are one: `kDefaultTimeSigNumerator`/`kDefaultTimeSigDenominator` (`src/core/song.h:94-95`), written by `Song::Song` through `apply_timesig` (`src/core/song.cpp:77-79`), and read by the Preview's `PreviewTimeSig` (`src/app/preview_view.h:198-199`). A `.chart` `TS n` with no exponent is n/4 by the format's own rule (`src/parse/song.cpp:1198-1203`).
- **How it left:** a chat question (small-items batch) the user answered. Closure's words: "Read-only scan 2026-10-05 at 993df71: The 4/4 default is two constants in song.h written through Song::Song; the Preview reads them. Fixed by f4a7d2c." (docs/audit/2026-10-03-fix-triage.json, row "319").
- **User's words:** "Q: Approve the small-items batch as recommended (60, 61, 258/319, 331, R7.5, R7.7, 77 fixes; 141 and 320 recorded only)?" chose "Approve all (Recommended)" (card `docs/sessions/2026-10-04-bbf41dc4.md`:27, which keeps the label and the question's start), with the batch text "- **One owner for the default 4/4 (findings 258 and 319).** No output changes." (the batch text was not kept), recorded as D27: "one owner for the default 4/4 (258, 319)" (docs/audit/2026-10-03-fix-decisions.md:65).
- **Code today:** as above; the default is 4/4 in one place. By reading.
- **Verdict:** PASS
- **Why:** The question named the default (4/4) and its one-owner fix; no second spelling of the default is left in the code.

### Derivation audit 320: `.chart` files always count ghosts and accents

- **Splits:** None: one place (`ChartParser::parse` sets `song.dynamics_enabled = true`, `src/parse/song.cpp:1893`). The MIDI side needs the tag (`dynamics_enabled_` set by `[ENABLE_CHART_DYNAMICS]`, `song.cpp:780-781`), and that difference is Clone Hero's own (below), not a Hydra choice.
- **How it left:** a chat question the user answered (recorded only). Closure's words: "Read-only scan 2026-10-05 at 993df71: D27 records that .chart ghosts and accents always count; CONTEXT.md states it." (docs/audit/2026-10-03-fix-triage.json, row "320").
- **User's words:** the batch text "**Finding 320.** `.chart` ghosts and accents always count, as in Clone Hero." (the batch text was not kept), "Q: Approve the small-items batch as recommended (60, 61, 258/319, 331, R7.5, R7.7, 77 fixes; 141 and 320 recorded only)?" chose "Approve all (Recommended)" (card `docs/sessions/2026-10-04-bbf41dc4.md`:27), recorded as D27: "Recorded only: ... `.chart` ghosts and accents always count (320)." (docs/audit/2026-10-03-fix-decisions.md:65).
- **Code today:** `.chart` dynamics are always on (`src/parse/song.cpp:1890-1893`). ADR 0023 now cites Clone Hero's code for it: "dynamics on (0x213D620, 0x215DBD0). There is no tag in `.chart`." (docs/adr/0023-chart-rules-follow-clone-hero-code.md:62). By reading (the disassembly not re-read).
- **Verdict:** PASS
- **Why:** The user was told the rule in plain words with its reason (as in Clone Hero), and the code and the ADR agree.

### Derivation audit 321: scan reads .sng and .srb names from only the first 1 MB

- **Splits:** None today. The scan's 1 MB capture (`kSngHeadCapture`) is gone. The scan reads a `.sng` head through `sng_read_head` (`src/parse/sng.cpp:115-128`), which grows the read until the whole file table is in hand, and a `.srb` through `srb_read_metadata`, the same reader the loader uses (`src/parse/srb.cpp:189-195`; its `kSrbMaxMetadata = 1 << 20` is one shared bound for scan and load, `src/parse/srb.h:45-47`).
- **How it left:** a "record as is" bullet in a batch question. Closure's words: "Read-only scan 2026-10-05 at 993df71: D51 call 13 records the 1 MB name capture for .sng/.srb; stored_versions.h cites it." (docs/audit/2026-10-03-fix-triage.json, row "321").
- **User's words:** "- The scan reads a .sng or .srb's title and artist from its first 1 MB." / "Recommended: record all five as they are." (docs/audit/2026-10-04-phase-7-questions.md:72,77,80), answered as D51: "- The scan reads .sng and .srb names from the first 1 MB (321)." (docs/audit/2026-10-03-fix-decisions.md:176).
- **Code today:** `git log -S kSngHeadCapture -- src` shows the last change at 36be3a70 (2026-10-10, "A chart's id is Clone Hero's song id for .sng and .srb too"); `src/app/analysis.cpp:293,441` read `sng_read_head(...)`. By reading.
- **Verdict:** GONE
- **Why:** Commit 36be3a70 removed the scan's 1 MB cut-off for `.sng` and the scan and loader now share one reader for each container.
- **Side picked without a ruling:** the whole-metadata side won for `.sng` (the scan now reads as much as the file table needs); D51 call 13 had recorded the 1 MB cut-off as the rule. The user's words behind the removal are not in the commit subject.

### Derivation audit 322: setting value ranges enforced in only some places

- **Splits:** Where does a hit window under 1 land when read from hydra_settings.ini? "The nearest edge of the range" (the user's rule, which gives 1) vs the default 85 (`number("hit_window_ms", &Settings::hit_window_ms, 1.0, kNoCeiling, Outside::Default)`, `src/app/config.cpp:196-197`; `pull_into_range` sends it to `defaults()` at `config.cpp:226-232`). Two other keys also deviate and are covered: `depth_mode` out of range reads as the default (D51 addendum, fix-decisions.md:199) and `sp_cap` below 1 reads 4 from the file (fix-decisions.md:200).
- **How it left:** a question the user answered with a batch "all recommended". Closure's words: "phase 7 wave 2, merged to main c8c4196 (M7-2c, PV/SE2/TM, 2026-10-05)." (docs/audit/2026-10-03-fix-triage.json, row "322").
- **User's words:** "**14. A hand-edited setting outside its range.** (322, 311, 31, 56, 72, 138)" / "After: every number setting has one allowed range, the one its box already enforces. A value outside it is pulled to the nearest edge when the file loads, and when you type it." / "Recommended: clamp, matching the boxes. The volume range becomes 0 to 100 everywhere, including Ctrl+click typing." (docs/audit/2026-10-04-phase-7-questions.md:84,90,91), answered as D51: "14. A hand-edited setting outside its range is pulled to the nearest edge of the range its box enforces, both on load and on typing. The Preview volume is 0 to 100 everywhere (Q14: 322, 311, 31, 56, 72, 138)." (fix-decisions.md:180).
- **Code today:** `hit_window_ms=0.5` reads 85 (the default), not 1. Before the key table, the loader kept any value above 0 (`if (v > 0) s.hit_window_ms = v;`, `git show 50fd7468^:src/app/config.cpp` line 83), so 0.5 used to be accepted. The hit window has no box, so "the range its box enforces" does not apply to it. By reading.
- **Verdict:** REOPEN
- **Why:** The question promised "pulled to the nearest edge" for every number setting; the code sends a hit window under 1 to the default instead. The effect is tiny (a hand-typed hit window under 1 ms), but the words do not match the rule.
- **Examples:** Low end: `hit_window_ms=0.5` reads 85 (default) under `value < lo` with `Outside::Default`; the user's words give 1. High end: `mslimit_value=900` reads 500 (`number("mslimit_value", ..., -kWindowMs, kWindowMs)`), as the words say. Gap: `hit_window_ms=0` reads 85 as before; `hit_window_ms=0.9` reads 85 where the rule gives 1.

### Derivation audit 323: library search `squeeze<N` means "at most N"

- **Splits:** None today. `squeeze<` and `squeeze<=` are now different (`q.squeeze_max_strict = op == "<";`, `src/app/library_query.cpp:276-278`), matching `>` and `>=`.
- **How it left:** a "record as is" bullet in a batch question, then changed by a later user ruling. Closure's words: "phase 7 wave 3, merged to main f4e352b (M7-3b, DOC, 2026-10-05)." (docs/audit/2026-10-03-fix-triage.json, row "323").
- **User's words:** "- `squeeze<20` means "at most 20"." / "Recommended: record all five as they are." (docs/audit/2026-10-04-phase-7-questions.md:78,80), answered as D51 (fix-decisions.md:177). Then D266: "1. **`squeeze<` is strict, like `>`.** The user chose "Strict, like >". ... Now `squeeze<20` drops a chart at exactly 20 ms and keeps one at 19.9 ms. `squeeze<=20` still keeps 20 ms" (docs/audit/2026-10-03-fix-decisions.md:1028), from "a question whose options showed what the code really did" (fix-decisions.md:1026).
- **Code today:** `squeeze<20` drops exactly 20 ms; the Search tips say "squeeze<20 leaves out exactly 20 ms." (`src/app/library_query.cpp:484-486`). By reading.
- **Verdict:** GONE
- **Why:** Commit 08688ca3 (2026-10-29, "Make squeeze<N strict, like squeeze>N (D266)") made `<` strict after the user chose it in chat, which replaced D51 call 13's record-as-is.
- **Side picked without a ruling:** none; D266 names the user's choice "Strict, like >".

### Derivation audit 324: one duplicate hides the whole "Best all-0 path" section

- **Splits:** None now. The finding's two tests (the display hides the whole section on any duplicate; the engine skips the all-0 search when the best path is already all-0) are consistent by construction: the display dedupes by `path_identity` per path (`src/app/path_view.cpp:469-478`) and the engine shortcut uses the same `is_allzero` plus limit test (`src/search/pather.cpp:46-56`, whose comment says "the display's identity dedupe agree by construction").
- **How it left:** a question the user answered with a batch "all recommended". Closure's words: "phase 7 wave 2, merged to main 199fec4 (M7-2a, E3, 2026-10-05)." (docs/audit/2026-10-03-fix-triage.json, row "324").
- **User's words:** "**4b. One duplicate hides the whole "Best all-0 path" section.** (324, from phase 6)" / "Today, if one all-0 variant matches a path already listed above it, the Paths tab hides the whole "Best all-0 path" section. Any other all-0 variants that aren't duplicates disappear with it." / "Recommended: hide only the duplicate, and keep the other variants visible." (docs/audit/2026-10-04-phase-7-questions.md:32-34), answered as D51: "One duplicate hides only itself, not the whole "Best all-0 path" section (Q4b, 324)." (docs/audit/2026-10-03-fix-decisions.md:164).
- **Code today:** `if (!listed) view.allzero.push_back(z);` per path, and the section shows when any survives (`src/app/path_view.cpp:472-480`). By reading.
- **Verdict:** PASS
- **Why:** The question described the real rule, the user chose the fix, and the code hides only the duplicate.

### Derivation audit 325: hydra_rules.ini lower bounds

- **Splits:** None (one loader, `load_rules_file`, `src/app/rules_file.cpp:36-67`, driven by the field table `rules_fields`, `src/core/rules.cpp:75-82`). The finding's visible asymmetry was that a leeway of 0 is accepted but a tied-path count of 0 is refused. Today: `backend_leeway_ms` 0 or more, `max_tied_paths` 1 or more, `fill_cooldown_measures` 1 or more, `fill_max_distance_beats` 0 or more, `fill_land_slop_beats` 0 or more (the sixth, `fill_length_measures`, was retired).
- **How it left:** a "record as is" bullet in a batch question. Closure's words: "phase 7 wave 3, merged to main f4e352b (M7-3b, DOC, 2026-10-05)." (docs/audit/2026-10-03-fix-triage.json, row "325").
- **User's words:** "- The six lower bounds in hydra_rules.ini. One example: max_tied_paths must be at least 1. The User Guide will list all six." / "Recommended: record all five as they are." (docs/audit/2026-10-04-phase-7-questions.md:79-80), answered as D51: "- The six hydra_rules.ini lower bounds stay as they are (325)." (docs/audit/2026-10-03-fix-decisions.md:178).
- **Code today:** the User Guide now lists every bound in an "Allowed values" column (docs/UserGuide.md, the rules table: "`max_tied_paths` ... A whole number, 1 or more", "`backend_leeway_ms` ... 0 or more"). By reading.
- **Verdict:** REOPEN
- **Why:** The question showed one of the six bounds and pointed at a guide that did not yet exist; the user never saw the other bounds or the 0-versus-1 difference the finding was about. The text also says "six" in the bullet and "all five" in the Recommended line.
- **Examples:** One end: `max_tied_paths=0` is refused (`if (d < f.min) bad(...)`, min 1, `src/app/rules_file.cpp:39`). The other end: `backend_leeway_ms=0` and `fill_max_distance_beats=0` are accepted (min 0). Gap: the same "0" is refused for `fill_cooldown_measures` (min 1) and accepted for `fill_land_slop_beats` (min 0). With `backend_leeway_ms=0` a backend note exactly at +0.0 ms still counts because `paid_by_sp_walk` is `offset_ms <= 0.0`, but one at +0.1 ms does not.

### Derivation audit 326: hydra_bench corpus mode has no timing limit

- **Splits:** What timing limit does a hydra_bench run use? Corpus mode: none, `settings.ms_filter = std::nullopt;` (`tools/bench.cpp:227`) vs folder mode: the GUI default through `gui.to_analysis_settings()` (`tools/bench.cpp:68-74`; the GUI default turns the limit on at 10 ms). A developer tool; no stored record or app display reads it.
- **How it left:** fixed (relabelled), through the approved plan. Closure's words: "phase 4 (merge ae60cc5, 2026-10-04): hydra_bench corpus label reads cap4 d4 no-ms from kCloneHeroSpCap." (docs/audit/2026-10-03-fix-triage.json, row "326").
- **User's words:** the approved plan row "| C3a Replay and bench text | 107, 110, 326 | ... | The bench reads "cap4 d4 no-ms" from `kCloneHeroSpCap`. |" (`docs/superpowers/plans/2026-10-04-phases-3-5.md`, line 67; removed, `git show 444631e6:docs/superpowers/plans/2026-10-04-phases-3-5.md`); the user approved that plan (D49 calls it "the approved phase 3-5 plan", docs/audit/2026-10-03-fix-decisions.md:142). No user question about the no-limit choice itself was found. I searched the three 2026-10-04 question files and fix-decisions.md for "326" and "no-ms" and found only that plan row.
- **Code today:** corpus mode prints `cap%d d%d no-ms` and runs `bench(kCloneHeroSpCap, 4)` (`tools/bench.cpp:222,241`); its comment says why it is fixed: it is the before/after yardstick (`tools/bench.cpp:211-214`). By reading.
- **Verdict:** PASS
- **Why:** The two modes answer different questions (a fixed speed yardstick versus "how long does the GUI's default take on this folder"), the difference is now in the printed label, and the plan that fixed the label was approved. The no-limit choice has no ruling of its own, but the tool touches no score, record or app display.

## Not reached

None. All 23 ids have a block. The weakest PASS verdicts are 316 (the transcript holds only a one-line question; the option texts are not in the transcript) and 315 (the generated-fill tie and whole-tick rules were closed by "continue" on a status message, then partly overtaken by Clone Hero's own code for the 1.1 generator).

## Side notes

- The tie rule for 1.0 generated fills (`dist <= it->second.bestdist`, a distance tie goes to the later chord) at `src/core/song.cpp:150` has no Clone Hero source and no question that named it; docs/adr/0023-chart-rules-follow-clone-hero-code.md:98-99 lists it as unchecked. Only the 1.1 generator cites a disassembly address (`src/core/song.cpp:176-183`).
- `src/app/preview_view.h:532` says the Preview's `sp_cap` is "the viewed record's own sp_cap", but `src/ui/preview_tab.cpp:410-415` passes `app.settings.sp_cap` for every selection ("The meter's ceiling is the Settings cap, Ready record or not"). A comment that names the wrong source.
- `solo_percent` (`src/app/preview_view.cpp:654-660`) truncates (`static_cast<int>(percent)`) while every other percent in the app rounds to nearest (`format_percent`). It copies the game's solo box on purpose (D251, fix-decisions.md:~880), so this is a different question, but a reader of finding 302's "no single rule" could trip on it.
- In 6/8 the "Fill spawn deadline" tooltip says "4 beats" (CONTEXT.md:556-558, `kCh11FillLeadBeats`) while the position text's beat number counts eighth notes (CONTEXT.md "Beat" entry, :731-737). CONTEXT.md says so in words, but the player-facing tooltip may not (not checked).
- The Q13 question text says "The six lower bounds" in the bullet and "record all five" in the Recommended line (docs/audit/2026-10-04-phase-7-questions.md:79-80). The same batch answer settled both.
- Finding 316's `skipped_e_offset` (set on the first passed-over fill, `src/search/engine.cpp:1017`) is not in the merge key. Each tied variant stores its own copy, so I saw no loss, but I did not run a chart.
- Finding 321's `.srb` metadata bound `kSrbMaxMetadata = 1 << 20` (`src/parse/srb.h:47`) is still a 1 MB cut-off, now shared by scan and load. A `.srb` whose metadata exceeds 1 MB is refused by both the same way; nothing disagrees.

