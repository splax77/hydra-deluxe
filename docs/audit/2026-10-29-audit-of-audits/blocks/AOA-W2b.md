# AOA-W2b: second-wave scout (agent a6383cbcddec97154)

Read-only. Short names used below:

- Cards dir = `docs/sessions/` (`<card>.md`). This scout also read the sessions' condensed transcripts; those were local working files and were not kept, so the lines below say where a detail came only from them.
- Fix decisions = `docs/audit/2026-10-03-fix-decisions.md`. Plan = `docs/superpowers/plans/2026-09-24-derivation-fixes.md` (removed; `git show 444631e6:docs/superpowers/plans/2026-09-24-derivation-fixes.md`).
- DA = `docs/audit/2026-09-24-derivation-audit.md`. CA = `docs/audit/2026-09-25-codebase-audit.md`.

The user's own typed and chosen words for 2026-09-24 are in ONE card: `docs/sessions/2026-09-24-da599242.md` (session da599242-9b0d-4c40-a47b-88badee1b17f); the full question texts were in that session's condensed transcript and were not kept. The 2026-09-26 audit answers are in `docs/sessions/2026-09-26-4908325e.md`; their full question texts were not kept either. AOA-S missed both because it only read the 2a88c065 card (the run of the plan), not the session that asked the questions.

## Item 1: DA "Checked and dropped"

### DA line 277: the two path-identity keys "answer different questions on purpose"

- **Splits:** "Are two paths of one record the same path?": the all-0 dedupe in `build_path_list` said same when score and short notation matched; the Preview overlay key said different when only the ms values differed (their own table, `docs/audit/2026-10-03-derivation-audit.md` finding 249, lines ~4750-4760: "only the ms values | same | different"). Today both sides call one function: `path_identity` (src/core/model.cpp:630, used at src/app/path_view.cpp:473-475 and src/app/preview_view.cpp:796).
- **How it left:** "Checked and dropped" with "on purpose": "The two path-identity keys (all-0 check versus Preview) answer different questions on purpose." (DA:277). The next audit (2026-10-03, finding 249) re-found it as a split and it was fixed.
- **User's words:** D50 (all recommended; Fix decisions:142 "The user answered "all recommended" to seven display choices D48 did not settle"), item 6 (Fix decisions:153): "Two paths are the same path when their score and every activation's tick and SP-end tick match (`path_identity`, finding 249). Measured on the 97 corpus charts: no all-0 list changes." The text states the new single rule and the measured effect; it does not list the old two rules as options.
- **Code today:** one rule, `path_identity` = score plus each activation's tick and deact tick (src/core/model.cpp:630-640), by reading. Commit f91b20e6 (2026-10-04) "One path identity for the all-0 dedupe and the Preview overlay key".
- **Verdict:** GONE
- **Why:** the "on purpose" claim was wrong (the 10-03 audit's own table showed the two keys disagreeing on ms-only differences) and the two keys are now one function.
- **Side picked without a ruling:** the new rule (score + activation ticks + SP-end ticks) is neither old rule. It sits under D50 item 6, which the user approved as "all recommended" with the rule written out, so a ruling exists for the new rule; there is no ruling that names the two old behaviours. Main session: decide whether the D50 wording is enough.

### DA line 31: the late-SqOut warning on a free backend row "on purpose"

- **Splits:** None that I can write as two displayed answers. The two sides answer different questions: the Points column answers "what does this row add to the path's score" (0 for a row at or past the leeway; `core::counted_without_squeeze`, src/core/backend_value.h:41-42) and the timing rating answers "how much does a late frontend hit endanger this squeezed-out row" (a free SqOut reads the late scale: `inside = row.squeezed_out ? core::paid_by_sp_walk(o) : ...` then `rate_note(o, inside, out.scales->post, ...)`, src/core/squeeze_rating.cpp:122-126). The same read of the code that DA cites is still there: model.h:352-353 "late ... free SqOuts, backend squeezes".
- **How it left:** a correction inside finding 1: "The comment at `src/core/squeeze_rating.cpp` lines 90 to 95 explains that warning on purpose: a late frontend hit can drag the SP end back over the free row and undo the squeeze-out." (DA:31).
- **User's words:** None found for the warning itself. Searched: Fix decisions (grep "late", "SqOut"), `docs/sessions/2026-09-24-da599242.md`, `docs/sessions/2026-09-24-8070412a.md`, and the full question texts of session da599242 (not kept). The nearest rulings are about the Points column ("0 + "(uncounted)" (Recommended)", which the card gives in words at `docs/sessions/2026-09-24-da599242.md`:50, and "0 points, no highlight (Recommended)" at :31), not the warning. The "on purpose" line was the audit writer's reading of a code comment, not a ruling.
- **Code today:** the old `late_backend_warns` flag no longer exists; `rate_activation` (src/core/squeeze_rating.cpp:99-132) rates a squeezed-out row past the SP end on the late side and gives it a scale figure when the scale moves, by reading. f4eebf9a (2026-09-24) rewrote only the plain-row edge; the squeezed-out branch is the same rule as before.
- **Verdict:** PASS
- **Why:** exit 4 holds on the one-sentence test, not on "on purpose": the Points column asks what the row scores (0), the rating asks what a late hit would do to the SP end (undo the squeeze-out). They are different questions. Note for the main session: what the squeezed-out ladder labels say (the "Standard SqOut" 10 ms edge) is the M39 split, handled separately; this PASS says nothing about it.

## Item 2: DA findings 38 and 39

### DA finding 38: Dynamics counts carry no parser version

- **Splits:** None: a stored table with no stamp (a stale-data risk).
- **How it left:** a question to the user, answered. Q (full text not kept): "The Dynamics tab's saved ghost/accent counts carry no version, so after a parser change they keep showing old counts. Add a version stamp so they're re-counted automatically?" chose "Add a stamp (Recommended)" (`docs/sessions/2026-09-24-da599242.md`:24). Then "The new version stamp on Dynamics counts: what should it track?" chose "Parser counter (Recommended)" (`docs/sessions/2026-09-24-da599242.md`:33). Plan decision 12 (Plan:57).
- **User's words:** as above, plus D87 (Fix decisions:388-389, 2026-10-07): "Path nodes, path links, structure blobs, tempo maps, song names and lengths (`songmeta`) and kick dynamics are no longer stored." The user "took the recommended answer to every question" (D87 heading text, Fix decisions:388).
- **Code today:** `src/store` has no dynamics table or stamp (grep "dynamics" over src/store finds nothing; `src/store/stored_versions.h` holds no dynamics stamp). The Dynamics tab counts from the parsed chart each time (`src/app/dynamics_breakdown.cpp`, called from `src/ui/app_state.cpp:460-462`), by reading. Commit 5a5c8faa (2026-10-07) "the store keeps summary rows only".
- **Verdict:** GONE
- **Why:** nothing is stored, so nothing can go stale; the stamp the user chose was built first and then retired with the whole table under D87.
- **Side picked without a ruling:** none.

### DA finding 39: scoring rules justified only by pointers to deleted Python

- **Splits:** None: a documentation gap (comments that cite deleted files).
- **How it left:** plan "Last, write down the reasons" (DA:297) and plan decisions 13 and 22 (Plan:58, :67). No ruling needed for a comment fix.
- **Code today:** the five scoring items DA lists are gone or no longer point at Python. The Auto cap ladder and the 120 s budget do not exist (commit eaf3b73e, 2026-09-27, "Remove Auto"; user's words "just drop auto completely, it's unnecessary and confusing. leave the default as 4." in `docs/sessions/2026-09-27-8fe109f3.md:26`). `src/store/path_binary.cpp` does not exist. `SPSqueeze` has no `operator==` (src/core/model.h; the `==` operators there are on ChordNote, Chord, TransferScale, BackendSqueeze, MultSqueeze, SpEndStep, HardestTiming). A grep for `hydata|hydra_app\.py|hymisc|hystore|native/hydra_search` over `src` finds 17 lines in 9 files, all UI layout or config-path notes (src/app/config.h:28, src/ui/app_state.h:1,6, src/ui/theme.cpp:42,59, and similar), the "harmless" kind DA itself names. By reading.
- **Verdict:** GONE
- **Why:** the scoring pointers were removed with their code; only layout comments still mention the old Python.
- **Side picked without a ruling:** none.

## Item 3: CA section 5 items 3, 5, 6, 7

All four were asked in chat on 2026-09-26 (session 4908325e-c9c7-4b77-a1a0-de265a45c992), 13:31-13:33 UTC, which AOA-S did not find. Cards: `docs/sessions/2026-09-26-4908325e.md` lines 23, 25, 26, 27; the full question texts were not kept. No D-record exists for them (the card lists "D: none").

### CA section 5 item 3: number boxes commit on leave

- **Splits:** None: a performance and behaviour choice with no two copies of one rule.
- **How it left:** a question the user answered, and the user chose the other option. CA:181 offered "Number boxes commit when you leave the box"; the user chose "Live, but cheap (Recommended)".
- **User's words:** Q: "Holding +/- on the score-range, ms-limit or SP-cap boxes rewrites the INI, re-queries the library and decodes the record every frame. How should those boxes behave?" chose "Live, but cheap (Recommended)" (`docs/sessions/2026-09-26-4908325e.md:23`; the full question text was not kept). Plan line 82 of `docs/superpowers/plans/2026-09-26-codebase-audit-fixes.md` (removed; `git show 444631e6:docs/superpowers/plans/2026-09-26-codebase-audit-fixes.md`): "The score-range, ms-limit and SP-cap boxes stay live exactly as today, but cheap. The INI is saved when you let go..."
- **Code today:** a held +/- changes the settings live (`app.edit_settings()`, src/ui/settings_bar.cpp:136,164), and the INI is written once no widget is active (`if (!ImGui::IsAnyItemActive()) app.flush_settings();`, src/ui/app_shell.cpp:474), by reading. Commit ddde90c3 (2026-09-26).
- **Verdict:** PASS
- **Why:** the question described what the code did, and the code does the chosen option.

### CA section 5 item 5: dropping pre-1.7 database migrations

- **Splits:** None: dead upgrade code.
- **How it left:** a question the user answered.
- **User's words:** Q: "Hydra still carries migrations that upgrade 1.5.x and 1.6.x databases. Since 1.8.1, the rows they produce can't be used anyway. Drop them?" chose "Drop them (Recommended)" (`docs/sessions/2026-09-26-4908325e.md:25`; the full question text was not kept). The audit's own wording (CA:183) names the visible effect ("would see "Not analyzed" instead of "Stale""), and the plan repeats it (codebase-audit-fixes plan line 84: "A 1.5 or 1.6 database shows "Not analyzed" instead of "Stale"").
- **Code today:** no migration code in `src` (grep "migrat" finds only the comments at src/store/record_store.cpp:369,1204 and src/store/record_store.h:111 about rows an older migration left with `ms_enabled = -1`), by reading.
- **Verdict:** PASS
- **Why:** the question named the user-visible effect in the audit text and the code matches the answer.

### CA section 5 item 6: retire ADR 0002

- **Splits:** None: a design record, not a rule in the code.
- **How it left:** a question the user answered.
- **User's words:** Q: "ADR 0002 (a recorded design decision) pins the report page byte-for-byte to hydra_report.py, a file deleted in the C++ cutover. Retire it so the three report pages can share one script and one stylesheet?" chose "Retire it (Recommended)" (`docs/sessions/2026-09-26-4908325e.md:26`; the full question text was not kept).
- **Code today:** `docs/adr/0002-sortable-page-fragments-canon-from-path-report.md` line 3: "_Superseded by ADR 0016 (2026-09): `hydra_report.py` is gone, nothing pins the page bytes, and the three report pages now share one stylesheet and one script._" `docs/adr/0016-report-pages-share-one-stylesheet-and-script.md` exists. Both files still exist on purpose (AOA-S's worry): "retire" here means marked superseded, which is what ADR practice does. By reading.
- **Verdict:** PASS
- **Why:** the question matches what was done.

### CA section 5 item 7: time budget out of the rules fingerprint

- **Splits:** None.
- **How it left:** a question the user answered.
- **User's words:** Q: "The rules fingerprint (the stamp that marks records Stale when rules change) includes the Auto ladder and the 120 s time budget. So editing either marks every record Stale, including fixed-cap ones they can't affect. What should count?" chose "Budget out, ladder Auto-only (Recommended)" (`docs/sessions/2026-09-26-4908325e.md:27`; the full question text was not kept).
- **Code today:** `rules_fields()` lists six fields and none is a budget or ladder (src/core/rules.cpp:73-83: backend_leeway_ms, sqout_rule, max_tied_paths, fill_cooldown_measures, fill_max_distance_beats, fill_land_slop_beats), by reading. AOA-S's grep found nothing because the budget was removed twice: from the fingerprint by commit 046cc0ba (2026-09-26) and with the whole Auto feature by eaf3b73e (2026-09-27, ruling above). `auto_cap_ladder` and `auto_budget_s` are only retired keys now (src/app/rules_file.cpp:24-25).
- **Verdict:** PASS
- **Why:** the question described the real fingerprint and the code does what was chosen (and then more, under the user's later "drop auto completely").

## Item 4: DA findings 33 to 37 (AOA-S gave PASS unread; code read now)

The rules file: `src/core/rules.h` and `src/core/rules.cpp`, read by `src/app/rules_file.cpp:load_rules_file`. `rules_fields()` (rules.cpp:73-83) is the one list the reader (rules_file.cpp, the `for (const core::RulesField& candidate : core::rules_fields())` loop in `load_rules_file`) and the fingerprint (rules.cpp:33-48) both walk.

### DA finding 33: a squeeze-out removes only the first note's share

- **Splits:** None. Two choices of one setting, both offered.
- **How it left:** asked; plan decisions 13 and 16 (Plan:58, :61).
- **User's words:** Q: "The 'squeeze-out removes only the first note's share' rule is a rule, not a number. What choices should the setting offer?" chose "first note / whole chord" (`docs/sessions/2026-09-24-da599242.md:28`; the full question text was not kept). Preceded by the typed "these are all arbitrary rules i decided for my purposes. make them configurable in a  config file." (card:25) to a question that names this rule (the card's line 25 names the rules it covered; the question text was not kept).
- **Code today:** `enum class SqOutRule { FirstNote, WholeChord };` (src/core/rules.h:22), default `SqOutRule sqout_rule = SqOutRule::FirstNote;` (rules.h:33), set from `first_note` or `whole_chord` (src/app/rules_file.cpp:70-72, src/core/rules.cpp:62-66). The only consumer is `const bool loses_sp = i == 0 || sqout_rule == core::SqOutRule::WholeChord;` (src/core/scoring.cpp:58), by reading.
- **Verdict:** PASS
- **Why:** the rule is a setting with both choices and today's default, as the user ruled.

### DA finding 34: the 3 ms backend leeway has no stated source (and the 10 ms edges, DA:223)

- **Splits:** (a) "How far past the SP end does a note still score under SP?" plain row: 3 ms, a setting `backend_leeway_ms = 3.0` (src/core/rules.h:32), used by `core::counted_without_squeeze` (src/core/backend_value.h:41-42) vs squeezed-out row's rating ladder: a fixed 10 ms, `kBackendInnerBandMs = 10.0` (src/core/model.h:100), used by `BackendSqueeze::summarystr` (src/core/model.cpp:343-352: `off < band ? "Standard SqOut"`). This is the M39 split. DA:223 says "The ±10 ms edges in `summarystr` are bare literals too. They only change label wording". The user's typed answer named only "the 3 ms leeway".
- **How it left:** the 3 ms part: asked and made a setting. The 10 ms part: no exit in this audit; it left via 2026-10-03 finding 306 and D48 question 4 (the known M39 story in `docs/agents/audit-method.md`).
- **User's words:** for the 3 ms: the question names it ("the 3 ms leeway"; the question text was not kept, and card `docs/sessions/2026-09-24-da599242.md`:25 says the answer covers the 3 ms leeway) and the typed answer is on that card at line 25. For the 10 ms edges: none that names the 3-versus-10 difference. `model.h:96-99` says "A fixed rule of its own, not the backend leeway (D48 Q4)", which cites the question that, per audit-method.md, described only the 10 ms edges.
- **Code today:** the leeway is a setting (above); the 10 ms band is still fixed in code, by reading.
- **Verdict:** REOPEN (the 10 ms half only; this is the already-known M39 item, so it is for the question the main session already owes the user. The 3 ms half is PASS.)
- **Why:** the user ruled the leeway a setting without being shown that the squeezed-out rating ladder uses a different, fixed 10 ms edge; the typed answer lists "the 3 ms leeway", not the second edge.
- **Examples:** note 12 ms before the SP end (-12): plain row `off < -band` gives "Easy", squeezed-out row `off < -band` gives "Hard SqOut" (they agree on the edge at -10, different words). Note 2 ms after the end (+2): plain row `core::counted_without_squeeze(off, 3.0)` gives "Standard", squeezed-out row `off < band` gives "Standard SqOut" (agree). Note 5 ms after the end (+5): plain row falls through to `if (off < w) return "Hard (uncounted)"`, squeezed-out row still says `off < band` "Standard SqOut" plus " (uncounted)" (they disagree: Hard versus Standard).

### DA finding 35: auto-generated fill constants have no source

- **Splits:** None. Numbers nobody sourced, now settings.
- **How it left:** asked; plan decision 13 (Plan:58). The authored-fill slop was asked separately.
- **User's words:** the typed answer above (`docs/sessions/2026-09-24-da599242.md`:25, which says it covers the fill constants; the question's words "the auto-fill constants" were not kept); Q: "The 1/32-beat slop that decides which chord an AUTHORED fill lands on (fill_lands_on_chord) isn't an auto-generated-fill constant. The audit lumped it in. Should it be a hydra_rules.ini value too?" chose "Yes, configurable" (card:32; the full question text was not kept).
- **Code today:** `fill_cooldown_measures = 4`, `fill_max_distance_beats = 0.5`, `fill_land_slop_beats = 1.0 / 32` (src/core/rules.h:44,45,51), read in src/core/song.cpp:154-156,184,223 and src/parse/song.cpp:1119,1874, by reading. `fill_length_measures` was retired by D115 (rules_file.cpp:26-29). One fill number is still fixed in code: `kCh10FillLengthMeasures = 0.5` (src/core/song.cpp:123), used only by the Clone Hero 1.0 generator (song.cpp:164); it is not in hydra_rules.ini. See side notes.
- **Verdict:** PASS
- **Why:** each named constant is a setting with today's default.

### DA finding 36: at most 4 tied paths are kept per score

- **Splits:** None.
- **How it left:** asked; plan decision 13.
- **User's words:** the typed answer (`docs/sessions/2026-09-24-da599242.md`:25, which says it covers the 4 tied paths) to a question that names "the 4-tied-paths limit" (the question text was not kept).
- **Code today:** `int max_tied_paths = 4;` (src/core/rules.h:38), passed into the engine (src/search/engine.cpp:2122, `static_cast<int32_t>(graph.rules().max_tied_paths)`), used at engine.cpp:1284, by reading.
- **Verdict:** PASS
- **Why:** a setting with today's default.

### DA finding 37: tie grouping leaves out SP-ready time

- **Splits:** "Do two tied paths with different SP-ready times fold into one?": the old group key said yes (no `sp_ready_ms` in it, DA:235) vs the order of fills that each ready time can still reach. Now the key holds the ready-time class.
- **How it left:** asked and fixed. D35 (Fix decisions:81): "Adopt the cheaper ready-time key (`claude/s1-rk`). Tied paths now fold only when the same upcoming fills fall on the same side of their SP-ready cut-offs, so Hydra no longer keeps a path that misses a fill and drops one that gets it."
- **User's words:** Q "Adopt the ready-time key? ..." chose "Adopt it (Recommended)" (`docs/sessions/2026-10-04-a9a461da.md:20`). The handoff text that defined the question (`docs/handoffs/2026-10-04-step1-and-gate-handoff.md`, line 37; removed, `git show 444631e6:docs/handoffs/2026-10-04-step1-and-gate-handoff.md`) says what the old key did: "Tied paths were folded without checking when each one's SP became ready. So Hydra could keep the path that misses a fill's early window, and drop the one that gets it."
- **Code today:** `Engine::ready_class` counts, per path, the upcoming fills it refuses and the fills over the ms limit, and the group key holds the result (src/search/engine.cpp:816-838, `return (refused << 16) | over;`). Commit cd179209 (2026-10-04) "Group waiting paths by which upcoming fills their ready time decides". By reading.
- **Verdict:** GONE
- **Why:** the key now includes the SP-ready consequences and the user said yes to that exact change.
- **Side picked without a ruling:** none.

## Item 5: the user's words behind plan decisions 3, 4, 5, 7, 9, 13

All six rulings exist. They were AskUserQuestion answers on 2026-09-24, 23:32-23:37 UTC (and 00:04 UTC on the 25th), session da599242. Source: `docs/sessions/2026-09-24-da599242.md` lines 16-33; the full question texts were not kept. For decisions 3, 4 and 5 AOA-S's "Side picked without a ruling" does NOT hold. For decision 7 as well.

### Plan decision 3: calibration fill sign

- **Splits:** "Which sign does the calibration fill show?": details line raw `e_offset` (-12.3 ms) vs header and report flipped (12.3).
- **How it left:** a question the user answered; fixed by c8100154 (2026-09-24).
- **User's words:** Q: "The calibration fill timing shows as -12.3ms on one line of the details panel and 12.3ms on the header and in the report. Which convention everywhere?" chose "Positive = hit early (Recommended)" (`docs/sessions/2026-09-24-da599242.md`:16; the full question text was not kept). Plan line 48: "3. Calibration fill sign: positive means hit early, everywhere."
- **Code today:** `av.early_fill = "Early fill: " + format_ms(*act.e_difficulty(/*verbose=*/true))` with the comment "Positive = hit early, the same sign as the report" (src/app/path_view.cpp:228-232), by reading.
- **Verdict:** GONE
- **Why:** both sides are in the question text, the user chose one, and the code shows one convention.
- **Side picked without a ruling:** none: a ruling names both (-12.3 ms raw vs 12.3 flipped).

### Plan decision 4: average multiplier rounding

- **Splits:** Song Details cut off at 3 decimals vs report `py_round3`.
- **How it left:** a question the user answered; fixed by c8100154.
- **User's words:** Q: "Song Details cuts the average multiplier off at 3 decimals; the report rounds it. Which one everywhere?" chose "Round (Recommended)" (`docs/sessions/2026-09-24-da599242.md`:17; the full question text was not kept). Plan line 49.
- **Code today:** `format_avg_mult(path.avg_mult())` with the comment "Rounded, not truncated, so it matches the report's mult column." (src/app/path_view.cpp:429-430), by reading.
- **Verdict:** GONE
- **Why:** both sides named, user chose, code matches.
- **Side picked without a ruling:** none.

### Plan decision 5: new record format, old records read Stale

- **Splits:** None: the user was asked about the cost.
- **How it left:** a question the user answered.
- **User's words:** Q: "Storing the squeezed-out note's tick and each activation's collected phrases means a new record format. Hydra's rule is that older records then read Stale until they're re-analyzed, which means re-analyzing the whole library once. Is that OK?" chose "Yes, bump and re-analyze (Recommended)" (`docs/sessions/2026-09-24-da599242.md`:18; the full question text was not kept). Plan line 50.
- **Code today:** the stamps live in src/store/stored_versions.h (ADR 0018); a record from an older stamp reads Stale. Not re-read in detail (not a split).
- **Verdict:** PASS
- **Why:** the question stated both the change and its cost, and the user said yes.

### Plan decision 7: 3 ms leeway wins for 2-3 ms plain backend rows

- **Splits:** "Is a plain backend row 2 to 3 ms past the SP end counted?": label and engine said yes (3 ms leeway) vs the rating said it needs a late frontend hit (past `kDifficultMs`, 2 ms).
- **How it left:** a question the user answered; fixed by f4eebf9a (2026-09-24).
- **User's words:** Q: "A plain backend row between 2 and 3 ms past the SP end: the table calls it 'Standard' and the engine counts it for free (3 ms leeway), but the rating treats anything past 2 ms as needing a late frontend hit and can raise the 'SP overfilled' warning. Which edge wins?" chose "3 ms everywhere (Recommended)" (`docs/sessions/2026-09-24-da599242.md`:20; the full question text was not kept). Plan line 52.
- **Code today:** `core::counted_without_squeeze(o, backend_leeway_ms)` in `rate_activation` (src/core/squeeze_rating.cpp:125) and in `is_frontend_decided` (squeeze_rating.cpp:93-96), by reading.
- **Verdict:** GONE
- **Why:** the question names both edges (2 ms and 3 ms) and the cap-clamped flag; the user chose.
- **Side picked without a ruling:** none. (This replaces AOA-S's request to confirm the plan's "already made" list: it did come from the user.)

### Plan decision 9: Preview honours audio delay and Offset after a live check

- **Splits:** None between Hydra parts: Hydra versus the game.
- **How it left:** a question the user answered, and the game check done.
- **User's words:** Q: "The Preview plays audio from 0 at chart time 0 and ignores song.ini 'delay' and the .chart 'Offset'. Test charts like Thornhill – Limbo (delay 1016) would play out of sync. What should happen?" chose "Check the game, then honor (Recommended)" (`docs/sessions/2026-09-24-da599242.md`:22; the full question text was not kept). The .sng and .srb question: Q "The Preview applies the audio delay/Offset rule only to folder charts. .sng and .srb charts always get 0 ... Should they get the same rule?" chose "Yes, same rule (Recommended)" (`docs/sessions/2026-09-26-4908325e.md:24`; the full question text was not kept). The game check was measured from the user's own recording (memory `ch-audio-delay-offset-rule`: Limbo `delay = 1016` measured +1018 ms).
- **Code today:** `sng_delay_ms` and `parse_delay_ms`, src/app/preview_source.cpp:110-179 (AOA-S, same lines), by reading.
- **Verdict:** PASS
- **Why:** the ruling is the user's chosen option and the check the option required was done.

### Plan decision 13: the six "arbitrary" rules become configurable

- **Splits:** None (see findings 33-37).
- **How it left:** typed by the user.
- **User's words:** "these are all arbitrary rules i decided for my purposes. make them configurable in a  config file." (`docs/sessions/2026-09-24-da599242.md:25`, 23:36 UTC), in answer to "Several rules have no written reason: the 3 ms leeway, 'a squeeze-out removes only the first note's share', the auto-fill constants, the 4-tied-paths limit, the Auto cap ladder and its 120 s budget. How should the reasons be found? Pick any that apply." (the question text was not kept). Follow-ups: "New hydra_rules.ini (Recommended)" (card:26), "Stamp + read Stale (Recommended)" (card:27).
- **Code today:** see findings 33-37. Auto's ladder and budget no longer exist (eaf3b73e). One gap: the 3 ms leeway's sibling, the 10 ms band, is not configurable (finding 34).
- **Verdict:** PASS
- **Why:** the user's words name the rules and the code does what they asked. The one hole is the 10 ms band, covered under finding 34.

## Item 6: hooks

### D190: "Let submit through" after a hard stop

- **Splits:** None: a hook behaviour, not two parts disagreeing.
- **How it left:** a question the user answered (2026-10-10). Fix decisions:739: "**D190 (session b5e8d95a), 2026-10-10.** chose "Let submit through (Recommended)" to "After an agent's hard stop ..." (17:53 UTC)".
- **User's words:** Q: "After an agent's hard stop (150 tool calls or 30 minutes), the progress gate lets through only the hand-back and git commands. A reviewer that hits the stop before running its submit command (derive_once_submit.ps1 or doc_review_submit.ps1) can't record its review, and you'd have to waive it. Should the two submit commands also pass after the hard stop?" chose "Let submit through (Recommended)" (`docs/sessions/2026-10-10-b5e8d95a.md:25`; the full question text was not kept).
- **Code today:** `C:\Users\Patrick\.claude\gates\progress.py` line 61: `exempt = data.get("tool_name") in REPORT_TOOLS or command.startswith("git ")`; at line 79 a spent budget denies everything else ("Only your final report, git commands and your status line get through now."). The two submit commands do not exist any more: no file of that name in `gates\`, and a grep over `gates` finds `derive_once_submit` and `doc_review_submit` only as retired words in tests (`tests\test_docs_numbers.py:22-23`, `tests\replay.py:71-76`). The reviewer's verdict is now recorded by `agent_stop.py` from the `SIGN-OFF: CLEAN`/`SIGN-OFF: BLOCKED` line in its final report (agent_stop.py:20), and the final report is a `SubagentHandback`, which `REPORT_TOOLS` always lets through. By reading.
- **Verdict:** GONE
- **Why:** the thing the user wanted (a reviewer that hit the hard stop can still get its verdict recorded) is true through the hand-back path, and the commands the question was about were retired by the 2026-10-25 rebuild.
- **Side picked without a ruling:** the retirement of the submit commands rests on the user's "approve as reommended" of the rebuild plan (plan line 86 lists `doc_review_submit_gate` as gone), not on a D190-specific ruling; there is no split, so I do not think this needs a question.

### Media guard (D189): every hook command read

- **Splits:** "Who may write ffmpeg or yt-dlp output outside scratch?": D189's chosen rule (main session free under C:\ytdl, subagents refused, other outputs asked in main) vs today's hooks, which check nothing.
- **How it left:** a question the user answered; the guard that kept it was built on branch ho-mediaout and never merged (AOA-S, card 2026-10-10-c3a986cf) [correction, 2026-10-30: it was merged into the hooks repo as 1380a37 on 2026-10-10]; the 2026-10-25 rebuild's dropped lists (plan lines 86 and 208) do not name it.
- **User's words:** D189 (Fix decisions:737): "chose "Main session free (Recommended)" to "The media guard has been off since the main session moved from Fable to Opus. ..." (17:26 UTC, C:\ytdl)". Full question (not kept; card `docs/sessions/2026-10-10-b5e8d95a.md`:24 keeps its start): "The media guard has been off since the main session moved from Fable to Opus. T4 turns it on for everyone. As it stands, every ffmpeg or yt-dlp command that writes outside scratch would ask you first in the main session, and be refused in a subagent. Today that means every download or re-encode into C:\ytdl, and every yt-dlp call without -o. How should C:\ytdl be treated?" chose "Main session free (Recommended)" (card:24).
- **Code today:** I read all nine hook entries in `C:\Users\Patrick\.claude\settings.json` (the file has only nine, not more than the 12 AOA-S saw: SessionStart `cleanup_sweep.ps1` and `session_cards_start.ps1`; UserPromptSubmit `posture.py`, `percall.py`, `waiver.py`; PreToolUse `*` -> `percall.py`, `Agent` -> `review_dispatch.py`, `Artifact|ExitPlanMode` -> `publish_gate.py`; SubagentStop `agent_stop.py`). Their arguments are in the `args` list, which AOA-S's command read did not show. A grep for `ffmpeg|yt-dlp|ytdl|yt_dlp` over `settings.json`, `gates\*.py`, `gates\*.ps1`, `gates\*.sh`, `gates\lib` and `gates\README.md` finds nothing. So no hook looks at ffmpeg or yt-dlp output. By reading.
- **Verdict:** REOPEN
- **Why:** the user ruled a rule and the rebuild dropped its guard without naming it; the rebuild plan's list of dropped hooks (plan:86, :208) does not mention it, and its approval ("approve as reommended") was over a list that did not show it.
- **Examples:** a subagent runs `ffmpeg -i clip.mp4 ... clip.mp4` over a file outside scratch: D189 says refuse, today nothing refuses. The main session runs `yt-dlp` writing into `C:\ytdl`: D189 says free, today also free (nothing checks), so this end agrees. The main session writes ffmpeg output to some other folder such as `D:\x`: D189 says ask first, today nothing asks (the gap).

### Hook design audit 2026-10-25, "What would stop the stream", items 2 to 6

For each: the audit's wording (docs/audit/2026-10-25-hook-design-audit.md:80-84), what happened, and an exit. None of these is a split; they are process proposals. The audit says (line 77): "None of them is built. Each one needs the user's yes before it starts." The user's reply was to scrap all hooks and rebuild (card `docs/sessions/2026-10-25-dee9c61e.md`: "make a new plan to get rid of every hook we have now and start over from scratch.") and later "approve as reommended".

- **Item 2, shadow mode for every new or changed deny rule.** The new plan (docs/superpowers/plans/2026-10-25-hooks-from-scratch.md) has no shadow mode: grep for "shadow" in the plan, the build handoff and `gates\README.md` finds nothing. Its place was taken by the replay proof (item 3). No user quote either way. Verdict: REOPEN as a small question, not a split: "do you want new deny rules to log first?" The card's open thread says the shadow-mode length was never answered. Exit today: nothing at all.
- **Item 3, replay real calls before merging a gate change.** Built: plan T10 (plan:124-132, :178) and the files `gates\tests\replay_extract.py`, `gates\tests\replay.py`, `gates\tests\reflog_moves.json` exist. Verdict: GONE (fixed). Side picked without a ruling: it is a stage of the plan the user approved "as recommended".
- **Item 4, helpers instead of parsing main-session commands (`merge_into.ps1`).** Not built as a helper. The merge case went the other way: the ref gate judges every move of `main` in git (`gates\ref_gate.sh`, `ref_judge.py`, `review_rules.py`), so no command shape is parsed; the other helper shape was kept for posts (`gates\publish.ps1`, `status_append.ps1`). Verdict: GONE (replaced by the git-side gate that the rebuild plan describes and the user approved as recommended). `tests\test_docs_numbers.py:23` lists `main_move_check` as a retired word.
- **Item 5, one owner per rule plus a docs-agree test.** Built: `progress.py` lines 14-17 hold the numbers ("The one home of these numbers"), and `gates\tests\test_docs_numbers.py` checks that CLAUDE.md and the agent docs quote them. Verdict: GONE (fixed).
- **Item 6, two-week freeze on new gates, then prune.** The prune happened wholesale: the rebuild replaced about 14,000 lines with about 2,200 (plan:21) and listed the retired hooks (plan:86). There is no freeze on new gates; the plan sets a 3,000-line cap instead (plan:257 of the replay row). Verdict: GONE for the prune; the freeze was never decided and has no ruling. It changes nothing the user sees, so I leave it as a side note, not a REOPEN.

## Counts

26 blocks: PASS 11, GONE 12, REOPEN 3.

PASS (11): DA:31 late-SqOut warning; CA 3; CA 5; CA 6; CA 7; DA 33; DA 35; DA 36; plan decision 5; plan decision 9; plan decision 13.
GONE: DA:277 path keys; DA 38; DA 39; DA 37; plan decisions 3, 4, 7; D190; hook audit items 3, 4, 5, 6.
REOPEN: DA 34 (the 10 ms half, M39); D189 media guard; hook audit item 2 (shadow mode, a small question).

## Not reached

None. Every item in the brief was done. Remaining caveat: for plan decision 5 I did not re-read the stamp code; it is not a split.

## Side notes

- `docs/audit/2026-10-25-hook-design-audit.md` items 2 and 6 and the card's "shadow-mode length ... never answered": no user ruling either way (see item 2 and 6 above).
- `src/core/song.cpp:123` `constexpr double kCh10FillLengthMeasures = 0.5;` is a fixed fill length for the Clone Hero 1.0 generator, outside hydra_rules.ini. Plan decision 13 said "the fill constants ... become configurable"; D115 retired the generated-fill length for the 1.1 generator but this constant remains. It is a Clone Hero 1.0 fact (ADR 0010), not a user choice; a comment or a ruling that says so would close the question. By reading.
- `docs/audit/2026-09-24-derivation-audit.md:17` says "Your two open questions from the handoff still stand". They were answered the same night: "0 + "(uncounted)" (Recommended)" and "Engine too (Recommended)" (`docs/sessions/2026-09-24-da599242.md` lines 15 and 50; the question texts were not kept). AOA-S's side note about missing D-records is answered: those answers are in this card, there is no D-record.
- The memory/cards pair for the 2026-09-26 audit answers (session 4908325e) has "D: none"; no D-record carries the CA section 5 answers. The words exist only in the card (`docs/sessions/2026-09-26-4908325e.md`); the condensed file that held the full question texts was not kept. If the main session wants the audit's closure checks to point at `fix-decisions.md`, those answers need a D-record.
- `docs/sessions/2026-09-24-da599242.md`:40 lists the 00:07 UTC answer "MIDI: follow mido on broken files" as replaced by the 00:33 UTC ruling (line 42); plan decision 25 holds the final form. Not checked further.
- Status line note: I wrote my first status line with 21:25 when the machine clock said about 21:20; the hook reads file time, so no effect.
