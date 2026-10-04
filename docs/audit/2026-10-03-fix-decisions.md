# Decisions for the 2026-10-03 derivation fixes

These are your calls on findings from `2026-10-03-derivation-audit.md`, in the order you made them. The fix plans quote them. The triage table `2026-10-03-fix-triage.md` lists which findings still need a call.

## Step 1: engine facts

**D1 (finding 28), 2026-10-03.** When the SP cap clamps a window, the transfer scale is measured from the collecting note (the stored `clamp_tick`), not the activation. Why: the collecting note is the one that moves the SP end, as the Rolling in the Deep FC video proved. The engine already stores it (ADR 0013). This changes stored scales, so it needs a results-stamp bump. Finding 27's fix (the pre-SqIn SP end) is in the same function, so both go in together under one bump.

**D2 (finding 32), 2026-10-03.** The replay keeps its own chord-by-chord walk as a cross-check on the engine's totals. For "did Star Power pay this chord?" it asks the shared rule in `core/backend_value.h` instead of deciding it itself. The Preview's xN disc doubles only when SP actually paid something for the chord. So a squeezed-out chord that SP pays nothing shows the plain multiplier, and a partly paid one still shows doubled. No stored change.

**D3 (findings 90 and 89), 2026-10-03.** A tied variant folded into its leader while SP is still running takes the leader's closed activation from the fold point: its SP end, clamp note, collected phrases and backend rows. It keeps its own activation note. Why: from the fold point their futures are identical, so the leader's closed activation is the variant's real one. The search does not change. Finding 89 is fixed alongside: each variant stores its own leftover SP, and which paths are listed stays as it is. Results-stamp bump, shared with D1.

**D4 (finding 332), 2026-10-03.** The record may hold an explicit "unknown" transfer scale instead of a silent x1.00, and the details view then shows a short "transfer scale unknown" note and no eff. figures. But your condition is that this must never happen: "there should never be a situation when the scale cant be computed". So the plan must close every way of reaching it at the source: every activation gets a stored SP end, and a zero or negative measure length can't reach the engine (this ties to R7.6, negative `.chart` tempos). A test must prove that no fresh record stores "unknown". The "unknown" value is a guard that shows a bug, not an expected state.

**D5 (R7.10, with finding 30), 2026-10-03.** For an activation whose SP outlasts the song, the 500 ms backend window is measured from the SP end, the same as every other activation. Why: only notes that early activation plus a late hit could push out of SP matter, and those sit well inside 500 ms of the SP end. The engine's tail collection and the display share one "inside the squeeze window" check next to `kSqueezeWindowMs`. Nothing on screen or in stored records changes, because the display already filtered by this rule before saving.

**D6 (R7.6, finding 100; step-1 plan Q1), 2026-10-03.** Timing that makes a measure or beat last zero, negative or infinite time refuses the chart with a plain error naming the tick. A time signature with a top number of 0 is ignored in both formats, as `.chart` already does. This closes one of D4's sources.

**D7 (step-1 plan Q2), 2026-10-03.** A tied variant that takes its leader's closed activation (D3) also takes the SqIn or SqOut the leader closed the window with. Those variants' path strings may gain the + or −.

**D8 (finding 97; step-1 plan Q3), 2026-10-03.** Kept out of the step-1 plan. It gets its own plan after a corpus count of affected variants.

**D9 (finding 5; step-1 plan Q4), 2026-10-03.** On a late squeeze-in the Preview SP gauge refills at the old SP end, so it reads 0.875 at the phrase, matching the engine.

**D10 (step-1 plan Q5-Q10), 2026-10-03.** All answered as the plan recommends. Q8 was confirmed first on FC video: in four activations across Divine Inner Tension and Desecration Day, the game's disc drops from x8 to x4 within two frames of Hydra's SP end and stays plain until the next chord.

**D11 (Task 16 review), 2026-10-03.** The Preview score box shows the plain multiplier from the SP end on. That includes the 40 test-set chords that land under 2 ms past the SP end, which Hydra's 3 ms leeway still pays doubled.

**D12 (Task 3 review), 2026-10-03.** The refusal wording "Hydra can't analyze this chart because the tempo at tick N is not above 0 BPM. Fix that line in the chart file or download the song again." is approved, with the same sentence shape for time signatures and resolution. A `.mid` tempo of 0 microseconds per beat says instead that the tempo at tick N is infinite (0 microseconds per beat).

**D13 (Task 1 review, audit finding 144), 2026-10-03.** A SqIn exactly on the old SP end (offset 0) is already collected for free, as the engine counts a note on the SP end as inside Star Power. One owner answers "is this SqIn free" for both the rating and the sentence. The sentence at offset 0 reads "no more than 0.0 ms late".

**D14 (Task 1 review), 2026-10-03.** A transfer multiplier within 1e-9 of 1 counts as exactly 1: no figure and no extra decimals. This guards against float noise. No test-set multiplier is that close to 1.

**D15 (Task 20 review), 2026-10-03.** The Preview SP gauge may differ from the old scan by up to one tick's worth of drain per stored SP-end step, inside a window only. Between windows it must match exactly. Why: the engine stores each SP end as a whole tick, so the old gauge's steepened last stretch and the new exact drain differ by at most 0.00066 bar. The readout shows one decimal, so nobody can see it.

**D16 (Task 8 review), 2026-10-03.** When two or more SqIns share one scale, the details line says "at each SqIn's SP end". A single SqIn still says "at the SqIn's SP end".

**D17 (Task 8 review), 2026-10-03.** The transfer-scale hover hint names the note SP is measured from: early and late hits scale differently when that note (the activation, or the collecting note when the cap clamps) or the SP end sits exactly on a signature or tempo change.

**D18 (Task 17 review, older bug), 2026-10-03.** An activation can't squeeze out a phrase that came before it started. Hydra allowed it, and since Task 5 that also stored an empty SP-end history. It is fixed in step 1, but only after a read-only count across the library shows how many charts and paths it touches. You see that count before any score change lands.

## Step 2: parser and Clone Hero rules

Evidence for these is Clone Hero 1.1's code, read statically (not run), in `docs/audit/ch-evidence.md` and the step-2 briefs in session bbf41dc4's scratchpad (`s2-*` folders). Numbers are from prototype builds on `claude/s1-t3` against scratch databases, each checked by a second agent.

**D19 (findings 11 and 250), 2026-10-03.** Disco flip is per difficulty: each difficulty reads only its own `[mix N drums...]` markers, in both formats, as Clone Hero does (0x215C750, 0x213D076, 0x2155050). `drums0dnoflip` stays off (Hydra does not copy Clone Hero's bug). Clone Hero's one-letter on/off rule (A+) is not adopted. The "flip only with Pro Drums" gate moves to one place. Effect: Expert never changes; Hard, Medium and Easy Pro Drums scores change on about 640 library charts each, mostly up, because charters mark disco only on Expert and their lower difficulties already have the hi-hat on yellow. Needs `kDynamicsCountStamp` and the shared results stamp.

**D20 (findings 10, 12 and 255), 2026-10-03.** Each difficulty reads its own 2x kick: MIDI 59 Easy, 71 Medium, 83 Hard, 95 Expert, and `.chart` N 32 in its own section (Clone Hero 0x21555CD, 0x210D9C0; its only gate is the Double Kick modifier at 0x20D32B7, at every difficulty). The 2x Bass box works at every difficulty, like Clone Hero's Double Kick. The Dynamics tab stops reading Expert's 2x kicks below Expert, so the phantom rows and swallowed kicks go away, and one kick total feeds both "All kicks" and Totals. Effect: 12 library charts' Hard or Medium scores change with 2x on; lower-difficulty record keys gain "2x Bass". No key fallback: this ships with step 1's results-stamp bump, which re-analyzes everything anyway. Needs `kDynamicsCountStamp`.

**D21 (finding 21), 2026-10-03.** One phrase-end helper for both parsers, with Clone Hero's rule (0x20D2440): start <= tick < end; a zero-length phrase pays nothing; a phrase running past the last note pays on that note. Effect: 20 Expert charts and 1 Hard chart lose 2,160 to 19,180 points that Clone Hero never pays (largest: Muse - Feeling Good). Shared results stamp.

**D22 (finding 315, the +1), 2026-10-03.** The fill landing window is floor(resolution x slop) + 1 ticks, as Clone Hero computes it (0x20D0088-0x20D008D). The +1 applies on top of a user's `hydra_rules.ini` slop too. Effect: 2 Thrice charts gain 460 to 720. Whether each fill is placed independently (Fill B) is a separate decision, pending its measurement. Shared results stamp.

**D23 (finding 344), 2026-10-03.** The written `kResultsStamp` rule names `src/parse`: a parser change that alters chart output forces re-analysis. Step 2 reuses step 1 Task 22's value and does not bump twice.

**D24 (finding 64), 2026-10-03.** The dynamics tag counts only in Clone Hero's two exact spellings, `ENABLE_CHART_DYNAMICS` and `[ENABLE_CHART_DYNAMICS]` (0x21557A5, 0x21557BB), and at a shared tick file order decides. Why: Clone Hero doesn't recognise a one-bracket tag, so no player can earn those ghost and accent bonuses. Effect: Last Chance to Reason - Programmed for Battle's Expert optimal drops from 620,730 to 607,580, the most the game can pay (caveat: assumes Clone Hero does not trim the text first). When the tag comes after marked notes, the Dynamics tab says "Dynamics enabled: from <time> on (N earlier markings ignored by Clone Hero)" (3 library charts); this adds two stored facts per difficulty (`kDynamicsBlobStamp`).

**D25 (finding 222), 2026-10-03.** Off-speed leaderboard scores get their own "other speed" status and leave the Matched, Above optimal and Points left on table totals. Avg % already skips them.

**D26 (findings 53, 314, open half of 315), 2026-10-03.** No live Clone Hero check now. Generated-fill length and 6/8 beats keep today's rule, written down as unverified against Clone Hero.

**D27 (small owners), 2026-10-03.** Approved as recommended: blank artist or charter shows the placeholder through one helper next to `title_or_unknown` (60; 56 charter cells); one container-entry rule (61); one owner for the default 4/4 (258, 319); a `.chart` modifier with no note is skipped, as Clone Hero does at 0x215DDB0 (331); one strict number helper for `.chart` Offset and song.ini delay (R7.5); `.mid` practice sections sorted (R7.7; 3 charts); probe tools use the measured 171.43 ms cap (77). Recorded only: gameplay events match raw text and section names strip quotes, with no shared normaliser (141); `.chart` ghosts and accents always count (320).

**D28 (hit-window plan), 2026-10-03.** Steps 1-3 of `docs/superpowers/plans/2026-09-25-hit-window-testing.md` are answered from Clone Hero's code and `poll_windows.csv` (cap 171.43 ms, floor 96.29 ms, both gaps used). Steps 4-6 are shelved.

**D29 (findings 37 and 304, after step 1 merges), 2026-10-03.** 37: the SP-end extension during Star Power comes only from `extend_deacts`, which stops at a full meter, as Clone Hero does (0x20F6800); counted after the merge. 304: a backend note at exactly +3.0 ms stays uncounted (strict); record it and pin it with a test.

**D30 (finding 315, Fill B), 2026-10-03.** Authored fills are placed one by one after the whole chord list is read, as Clone Hero does (0x20CFF60 calling 0x5DE030). Each fill takes the last chord at or before its end, but not before its start, or the first chord after its end within the slop. A tie goes to the later chord, and a fill with no chord in bounds is dropped. Why: today Hydra forgets a fill when the next fill starts before any chord reaches the first fill's end, so its path skip counts can say one fewer fill than the game shows. Clone Hero's unbounded one-side fallback (B2) is not adopted. Effect: no score changes anywhere measured. The best path's fill counts change on 2 to 5 library charts per difficulty (e.g. Dirge Within - Forever the Martyr, `1+ 0 2` to `2+ 0 2`). Open: the search takes a start index, so Clone Hero may carry position between fills; not traced. Shared results stamp.

**D32 (early squeeze-out review), 2026-10-03.** Both crashes that only appear at extreme tempos (2,000 to 4,000 BPM) get fixed in step 1. One is Task 17's "a folded variant holds a step past its fold". The other is the older "search reached a broken state". Each currently makes a chart fail to analyze. Neither appears in the library. Scores on real charts must not change.

**D31 (step-2 plan wording, Q1-Q5 and the review), 2026-10-03.** Approved as the plan proposes. The 2x Bass tooltip, shown at every difficulty, reads "Include the chart's 2x kicks, like Clone Hero's Double Kick.", and the UserGuide's 2x lines change to match. The Dynamics tab's "2x kicks: X of Y kick notes" line keeps counting every charted kick. The late dynamics tag is stored as a time in milliseconds, so the tab can say "from m:ss", and one earlier marking reads in the singular. The leaderboard page gets an "Other speed" filter option, a stat row and a status-help sentence, the Points-left cell dims on off-speed rows, the summary adds ", N at other speeds" only when there are some, and one UserGuide sentence says the same. The UserGuide's `fill_land_slop_beats` line says the +1 tick applies and that it covers only fills written in the chart.

**D33 (Task 22 stamps), 2026-10-03.** Step 1 and step 2 ship in release 2.1.0. `kResultsStamp` becomes "2.1.0", so every analysis made before it reads Stale and is redone once. The app version in CMakeLists.txt changes at release time, not in Task 22.

**D34 (extreme-tempo review, double squeeze-in), 2026-10-04.** A phrase can be squeezed in only once. When the first phrase in an SP end's squeeze window was already squeezed in (or banked, D18), the next phrase in that window is offered instead, or nothing if there is none. Measured on the library at caps 2 to 4: 0 score changes; 7 listed paths on 2 charts (Thrice - Deadbolt cap 4, SoundHaven - Triad cap 2) lose a "+" for a squeeze that never happens. The cause is two SP-end points one tick apart.

**D35 (ready-time group key), 2026-10-04.** Adopt the cheaper ready-time key (`claude/s1-rk`). Tied paths now fold only when the same upcoming fills fall on the same side of their SP-ready cut-offs, so Hydra no longer keeps a path that misses a fill and drops one that gets it. Measured on 19,343 library charts: 7 best scores higher at cap 2, none lower, cap 4 identical, no speed change. Ships under the 2.1.0 stamp with step 1.

**D36 (extreme-tempo gaps), 2026-10-04.** The two 2,000+ BPM gaps (a squeezed-out first phrase drops the second phrase's SP-end step; folded paths copy the leader's squeeze-in of the next phrase) and the twin SP-end points one tick apart get one follow-up branch after step 1 lands on `main`. It is measured on the library and shown to the user before it merges. No library chart reaches either gap.

**D37 (cleanups), 2026-10-04.** Approved, none visible on screen: derive the 2x-kick pitch as kick − 1 and drop its column; track the Clone Hero evidence file under `docs/audit/`; remove `MPhase::PreTimestamp` and the stale test comment; fix the batch-strip-drift flake if `69e5ed7` doesn't already; step-1 review notes (drop the Task 17/19 duplicate test, Task 20 test passes `cfg.rules`, Task 7 exact-tick test and fill list in `hydra_replay dump`, multi-worker batch-strip test); hooks read input as UTF-8 through one shared helper, status-file readers included. The review rule is switched on; steps 1 and 2 merge to `main` when every check is green and the gate review passes.

**D38 (folded path's own early-fill figure), 2026-10-04.** When tied paths fold, each keeps its own early-fill facts (offset, passed fills, E0 status), so a folded alternative's view shows its own "Early fill" line, not the leader's. Found by the D35 review; it predates D35. No score changes. Ships in step 1 under the 2.1.0 stamp.

**D39 (sizing an open file), 2026-10-04.** The single-owner scan says one place works out a file's size. The five spots that size a file they already hold open (four `ImFileGetSize` lines and `MappedFile::open`) call a new winstr helper for "the size of this open file", so those five leave the scan's known-copies list. (The three `midi.cpp` entries stay until audit R7.11's own fix.) Nothing visible changes. This replaces audit R7.25's "acceptable" for sizing an open file; R7.25's other half (the Preview loader sizes each loose stem twice per load) stays open on the fix list. One exception, named line by line in the scan rule: `list_dir` keeps the size the folder listing already reports, for the rescan cache's change check, because opening every library file just to size it would slow every scan. Its answer matched the handle size in review, including on a file still open and growing.
D38 addendum, 2026-10-04: the folded path's own passed-fill count also shows in its name. On the 97 test charts at default settings nothing changes; at cap 4, depth 40, 10 ms limit one path of 3,068 does (The Agonist - Thank You, Pain, "2 0 2" becomes "2 0 5", its own five passed fills). No score changes. The user said keep it.
D37 addendum, 2026-10-04 (test limits from the flake fixes, approved under "fix the flaky test"): the work-pool hang test fails when no round finishes for 30 s; the 20,000-chart filter test checks the fastest of five runs against its existing 20 ms; a cancelled Preview load may open at most 8 MB (two 4 MB report steps) after the cancel and must stop within 2 s (ten times the old 200 ms). None of these is an app setting or changes anything on screen.

**D40 (step-1 review, numbers in comments), 2026-10-04.** Recorded as they are, no behaviour change. The banked-phrase reach looks one tick further than the plain SP end, to absorb `plusmeasure` rounding; it decides which tied paths fold at extreme tempos (ADR 0014; D36 may revisit it). An SP end exactly on its phrase's tick counts as reaching that phrase (ADR 0014). The group key's packing limits (SP end within ±2^46 ticks, banked-phrase ordinal at most 65,535, meter under 2^30, 16 bits per fill count) refuse a chart past them with an error rather than fold paths wrongly (D35's record).

**D41 (finding 138, Path limit), 2026-10-04.** The Path limit box's ±500 ms ceiling is the engine's squeeze window. The box reads `kSqueezeWindowMs`, and the squeeze-window scan drops its "different setting" exemption. Nothing on screen changes.

**D42 (audit R7.30, test reference copies), 2026-10-04.** Tests may not keep frozen copies of old code as a reference. `reference_replay_path` (test_replay.cpp), `old_scan` (test_preview_view.cpp) and `reference_build` (test_track_state.cpp) are deleted; the tests that used them check fixed expected values on hand-built charts instead. D15's "may differ from the old scan" becomes a pinned expected gauge, with the same one-tick-per-step tolerance inside a window.

**D43 (step-1 review, test limits), 2026-10-04.** Approved as test limits, none an app setting or visible: `acts > 1000` and `skipped > 100` (test_search.cpp); variant floors 10, 200 and 200 with skip pins 0, 0 and 4, `changed == 9`, `wl.size() > 50` (test_replay.cpp; the last two go with D42 if their tests do); seeds 1-48, `analyzed == 48`, `variants > 50` and the fuzz generator's ranges (test_fast_tempo.cpp); the 60 s batch-step deadline and three workers with a 10-frame, 2 ms settle (uitest_batch_reports.cpp); the 300 random probe times (test_preview_view.cpp); the scan floors `files > 50` and `owner_uses >= 3` (test_model.cpp); the "uncapped" cap of 999 (test_store.cpp).

**D44 (T10 lands), 2026-10-04.** T10 (finding 37 under D29) merges after step 2, behind its derive-once review, with the Step 9 stamp check re-run just before. Counted at caps 2 to 4, all difficulties: 0 changes on the 97 test charts and the 206 short-measure library charts. Only the hand-made 2,400 BPM test charts change, losing "+" marks for squeeze-ins that never happen (e.g. cap 4 `clamped_sqin_a` best "0+++" becomes "0++", both 13,350). Approved departures from the plan: tied paths whose SP ends the cap clamped in different places stay apart in `reduce_group`, so each keeps its own squeeze-out; the `clamped_sqin` fold test is re-pinned and renamed; the engine reads offered (chord, end) pairs off the graph's choice lists; one comment in `sqout_chord.h` is corrected.

**D45 (`hydra_replay target`), 2026-10-04.** A targeted search returns the paths that have exactly the named activations and drops only those missing one, instead of reporting the whole set unrealizable when any one path lacks an activation. It builds its graph with `graph_build_cap`, like the main search. Dev tool output only; the app and stored results don't use it. The fast-tempo test then uses the shared lone-pricing helper.

**D46 (UTF-8 hook leftovers), 2026-10-04.** All three fixed: `deletion_shrink_gate`'s worktree `.git` read names UTF-8; the project's inline Stop hook in `.claude/settings.local.json` reads its input through the shared helper; and when the shared helper is missing or throws, gates fail closed (ask) instead of allowing the call.
D46 amendment, 2026-10-04: fail loudly, never freeze. When the shared helper is missing or broken, gates deny (with a reason naming the helper and the error) instead of asking, because an unanswerable "ask" inside a subagent hangs the call. Hooks that cannot refuse (UserPromptSubmit, PostToolUse, Stop, trackers) show a visible warning (systemMessage) as well as the stderr line. handoff_gate learns which event it runs on from its registration, so it denies tool calls but never blocks a prompt.
D43 addendum, 2026-10-04: test_replay's "SP points add up window by window" sweep checks the first 120 windows of the longest corpus chart (0.5 s; all 2,000 cost 5.6 s per run). Approved as a test limit.
D45 addendum, 2026-10-04: every path `search_target` returns, tied variants included, has exactly the named activations. A variant that skipped one is dropped whether its root is kept or dropped. When a root is dropped, its qualifying variants are rebuilt as standalone paths; the first leads and the rest are its tied variants, in the engine's order.
D45 addendum, placement (2026-10-04, follows the addendum's rule): when a dropped variant sits under a kept root, its own qualifying variants stay under that root, in the engine's order, as for promoted paths. `search_target` also reports which returned paths were promoted, so test oracles never treat a promoted copy as an unfolded root.
D43 addendum, 2026-10-04: step 1's hand-made tests analyze at "keep the top 40 scores" (`test::scores_settings`), wide enough that their small charts keep every tied variant. Approved as a test limit; not an app setting.
D44 addendum, 2026-10-04 (T10 on the step 1+2 base): real charts still show 0 changes (97 test charts and the 206 short-measure library charts, caps 2 to 4, all difficulties). On the 48 generated 2,000 to 4,000 BPM test charts, T10 finds squeezes the old search missed when a full meter pins SP's end to the cap's ceiling: best scores rise on seed 37 cap 2 (11,950 to 13,150) and seed 35 caps 2, 3 and 4 (+200, +400, +600); the replay prices each new path at its score on both builds. Elsewhere on those charts fake squeeze marks go and a few lower paths gain real ones (seed 5 gains "0- 0+-", "0- 0++", "0- 0-"; best 7,550 unchanged). The user said land it; the seed-5 tests are re-pinned. T10 ships in 2.1.0 and shares its results stamp.

**D47 (step-2 review, .chart note order), 2026-10-04.** A `.chart` whose drum notes are written out of tick order is read in tick order: the reader settles chord order once, before the phrase rule (D21) and the fill rule (D30), so a phrase's pay-out and a fill's landing always pick the same "last" chord. None of the library's 4,338 loose `.chart` files is out of order, so no saved result changes; the `.sng`/`.srb` containers are checked when the fix lands. Clone Hero's own ordering is not in the evidence file.
D36 addendum, 2026-10-04 (measured, user said land it): one rule in `core::offered_phrase` — an SP end offers a window at most one phrase: before the end, only the window's newest phrase, at the SP end that collecting it moved; after the end, when the window ends here, the first phrase after it the window hasn't squeezed in (the old rule let an older phrase block it — the design had said that side was unchanged). It replaces T10's clamp-origin guard. Whole library (about 19,900 charts, caps 2 to 4, four difficulties): every stored record byte-identical, including squeeze offsets and backend rows; Thrice - Deadbolt and SoundHaven - Triad identical; speed unchanged within run-to-run noise. Hand-made 2,000 to 4,000 BPM test charts only gain: clamped_sqin_a 12,950 to 13,150 (cap 2), 13,350 to 13,550 (cap 3), 13,350 to 13,950 (cap 4); clamped_sqin_b 12,750 to 12,950 (caps 3 and 4); seed 36 9,550 to 9,950 (caps 3 and 4); some same-score labels gain a "+" or a free "-". Ships under 2.1.0.

## Phases 3 to 5: display, docs, shell path

**D48 (phase 3-5 questions), 2026-10-04.** The user answered "all recommended" to all 33 questions in `docs/audit/2026-10-04-phase-3-5-questions.md`. Each question's "Recommended" line is the decision. In short:
- Tied top-score paths are all optimal, in the report too (Q1).
- Whole-ms text rounds to nearest everywhere (Q2).
- A timing on a tier edge counts as inside (Q3).
- The ±10 ms backend band stays a named constant, not tied to the leeway (Q4).
- The report footer is reworded (Q5).
- Uncounted SqOut rows get the "(uncounted)" tag (Q6).
- The "Tightest squeeze" tile becomes "Hardest ms" (Q7).
- The timeline is orange only for difficult rows (Q8).
- A row's bars mean "banked when you activate" (Q9).
- The early-fill badge stays and is recorded (Q10).
- Note names follow the Dynamics wording (Q11).
- Counts are singular at 1 with commas from 1,000, whatever the browser's language (Q12).
- Fill rules are named "Clone Hero 1.0" or "CH 1.0" from one owner, and "(legacy)" goes (Q13).
- Titles are cleaned, with "(unknown)" when nothing is left (Q14).
- The library and settings-bar items Q15 to Q21 take their recommended forms.
- The Preview items Q22 to Q27 take theirs. Position is measured against the last note, the beat lines run to the audio end, teal means SP running, and a chord on the playhead counts as hit.
- The report items Q28 to Q31 take theirs.
- The nine wording fixes in Q32 are approved.
- Every number in Q33 is kept as it is and recorded. That includes the 10 ms Path limit on, 50 ms backend hide off, and both path cut-offs (248 and 260).

None of these changes a score, a path or a stored record. The results stamp stays "2.1.0".

**D49 (developer-tool tuning numbers, M0 review), 2026-10-04.** The user chose to record these as they are. They control only how two developer tools behave; none touches a score, a display or a record.
- `tools/new_worktree.ps1` and its shared build-slot script: at most 3 cold builds at once (the approved phase 3-5 plan's "Cold builds start at most three at a time"), a free-slot check every 15 s, and a "still waiting" line every 60 s.
- `tools/derive_once_precheck.ps1`: helper bodies under 30 tokens are matched by name only, not by body; a C++ statement split over up to 4 lines is read as one; a typed 500 counts only within 2 lines of a squeeze, window, SqIn/SqOut, leeway, deact or fabs( word, and the same 2-line reach applies to a typed 2.0 beside a hit window (the 500 rule later moved into a scan row, leaving the 2.0 rule as the reach's only user); and a decision cited on the line or the 3 lines above it is the one the number must appear in.
- Also in the precheck (added after the M0 round-2 review, same kind of tool tuning): the numbers 0, 1 and 2 are never listed as needing a decision, because they are counts, first and second, and off-by-one, not thresholds; 1000, 1024 and 1000000 next to `*` or `/` are unit factors (ms to s, KiB), not limits; and check 1 looks back 200 characters before a function's opening brace for a `namespace` keyword, to tell a free helper from a member.
- The precheck's run on main is the range run, which prints nothing on a clean merge. A whole-tree run lists every copy already in the tree, as backlog for the audit, not as something a branch added.

**D50 (phase 3 display follow-ups), 2026-10-04.** The user answered "all recommended" to seven display choices D48 did not settle, raised by the phase 3 task briefs and reviews. Each one changes only text or placement on screen; none touches a score, a path or a stored record.
1. The path report's footer (finding 2) reads: "Timing tiers measure how big each squeeze is, in steps of your hit window. The Paths tab's row labels measure how far a hit lands from the Star Power end, so the two can differ. 'Beyond' means past the N ms window."
2. In the fill comparison (finding 312), a chart with a record in both databases but paths in only one is listed under "In both", with "no score" on the empty side.
3. An empty fill comparison (finding 105) says: "Nothing is analyzed under these settings (SP cap N, <mode>) in either database. Analyze with these settings, or change them."
4. The Preview scrubber's right edge is the last note (finding 9), so activations sit at the same fraction as on the Paths timeline. Playback runs on into the audio tail with the thumb parked at the right end; the tail cannot be dragged into.
5. An artist made only of Clone Hero tags reads "(unknown)", by the same rule as titles (finding 8).
6. Two paths are the same path when their score and every activation's tick and SP-end tick match (`path_identity`, finding 249). Measured on the 97 corpus charts: no all-0 list changes.
7. On the Paths timeline (finding 3), a difficult activation keeps its orange outline; any other badged activation gets a grey outline matching its grey badge.

**D51 (phase 7), 2026-10-04.** The user answered "all recommended" to every question in `docs/audit/2026-10-04-phase-7-questions.md`, including 4b. Each question's Recommended line is the decision. The user added two notes in their own words.
- On 4- and 5-note chords (Q3, findings 49 and 50): "those should never appear in a real drums chart. but it's valid in clone hero so hydra should account for it correctly." So `MultSqueeze` follows `to_multiplier` for every chord size, and shows the real gain.
- On the early-fill window (Q7, finding 313): "60 is fine because it doesnt change anything. lots of paths were getting cluttered with early fills that will never matter so i tightened it from 85 to 60." So `kEarlyFillWindowMs` stays 60 ms. It was tightened from 85 ms to drop early fills that never matter.

The calls in short:
1. The tie limit is one count per score. When some tied paths are inside the Path limit and some are over, the inside ones lead (Q1, 95; changes stored results).
2. A path that ties the best score stays shown even when it is over the Path limit, and the guide says so (Q2, 330).
3. See the 4- and 5-note chord note above (Q3, 49 and 50; changes stored results).
4. A path with nothing to time shows no timing figure. A squeeze-out at exactly 0.0 ms still needs timing, and the all-0 list follows D13 (Q4, 22; may change stored all-0 lists, and the counts go to the user before the merge). One duplicate hides only itself, not the whole "Best all-0 path" section (Q4b, 324).
5. A required early fill counts against the Path limit like a squeeze, and the words say "hardest squeeze or required early fill" (Q5, 23).
6. A phrase that fills the meter exactly to the cap is not an overfill (Q6, 307).
7. See the early-fill window note above (Q7, 313).
8. Results made under other rules are kept, and read Ready again when the rules match (Q8, 65).
9. Song length is stored per difficulty (Q9, 62).
10. The first copy the scan lists names a duplicate chart, and a batch analyzes each chart once (Q10, 63).
11. A Ready chart with no paths stays under Analyzed. The filters see no facts for it, and the leaderboard and fill pages say "no paths" (Q11, 88).
12. Each analysis rewrites the stored tempo map, and the scan cache carries its own version stamp (Q12, 343 and 345).
13. Recorded as they are (Q13). Six items:
    - The stars column is a cache of `path_stars` (129).
    - Custom-ladder Auto rows read Stale until re-analyzed (317).
    - The scan reads .sng and .srb names from the first 1 MB (321).
    - `squeeze<N` means "at most N" (323).
    - The six hydra_rules.ini lower bounds stay as they are (325).
    - The scoring values: cymbal +15, solo 100 per note, multiplier steps at combos 10, 20 and 30 (335).
14. A hand-edited setting outside its range is pulled to the nearest edge of the range its box enforces, both on load and on typing. The Preview volume is 0 to 100 everywhere (Q14: 322, 311, 31, 56, 72, 138).
15. A `#` comment after a value in hydra_settings.ini is ignored, and the hit window may hold a decimal (Q15, 66 and 140).
16. A 1-bar SP cap stays allowed, and the Paths tab says "A 1-bar cap can never activate Star Power." (Q16, 139).
17. Jumping back after the audio ends brings the sound back (Q17, 73).
18. A chart file that changed since it was analyzed opens with the path overlay hidden and one line: "This chart changed since it was analyzed. Analyze it again to see its path." (Q18, 126).
19. A FLAC whose header length is 0 is counted by decoding it once on open (Q19, R7.8).
20. A damaged Opus stem comes back after a scrub to before the damage, like every other format (Q20, R7.9).
21. Preview clips are left out of the song mix everywhere, and one "is this playable audio" rule (`audio::sniff_format`) decides (Q21, 74 and 101).
22. The SP gauge measures a mid-measure meter-change tick under the engine's side, the earlier section (Q22, 58).
23. "Open automatically" keeps its behaviour. Its hint reads "Open each report in your browser as soon as it's built." (Q23, R7.2).
24. "Scan now" and "Rescan library" are blocked during a batch, and the status line says "A batch is running." (Q24, R7.3).
25. Errors carry their kind, and each kind has one plain sentence. A Preview asset problem reads "Reinstall Hydra" (Q25, 193).
26. The batch reports its own analyzed, skipped and failed counts (Q26, 142).

The code-only calls at the end of the sheet are approved as written. They cover findings 38, 59, 68, 78 to 85, 86, 98, 103, 108, 109, 170, 174, 198 and 202. In particular, the six old probe scripts are deleted.

**D51 addendum (phase 7 wave 1 launch), 2026-10-04.** The wave 1 brief writers raised open questions that D51 didn't spell out. The user answered "go with recommended answers to everything". Each brief's "Decided at launch" section holds the full text.
- **ST1 (65), stored layout.** The results table's unique key gains the rules fingerprint. Schema 4 rebuilds the table the way `add_fill_rule_column` does: a `rules_fp` column filled from `substr(structure,5,8)` joins the key. Every row, result id and blob is kept, so nothing is re-analyzed. This is what lets a rules-A row and a rules-B row sit side by side.
- **SE1, `depth_mode` in a hand-edited file.** Any value other than 1 reads as scores. A 2 has always searched by scores, so it keeps doing that. The nearest-edge rule from call 14 covers the ranged number fields, not this switch.
- **SE1, the SP cap.** 0, junk and `auto` still read as 4. That is the earlier user rule with its own test. The minimum cap of 1 lives in `config.h`'s key table.
- **E2, the how-to line.** When several notes must go on one side of the step, the line names them all: "Hit [YellowCym] and [BlueCym] last." The cheap side reads "first.". " or " stays only for notes tied in value.
- **Code-only calls, approved as recommended.** Each wave 1 task adds its own rows at the end of `tests/test_single_owner.cpp`, joined at M7-1. T1's result writer lives in `replay_json`, `--cap 0` still refuses on the command line, and `hydra_bench` is diffed by hand. AU1 pins only the FLAC zero-length case and stops if libopus accepts the bad packet. PS1 moves the byte check behind `audio::sniff_format` into `hydra_core`, so the Preview's dependency on it is declared. PR1 deletes `schedule_hit`, moves `window_verdict` into `watch_window.py`, and uses one 5 s stall limit. PR2 reads a `hydra_replay dump`, and the main session adds its README sentence at M7-1.

**D52 (phase 3, fill comparison counts), 2026-10-04.** The user chose the recommended answer. D50 item 2 puts a chart with a score on only one side under "In both". The fill page's subtitle, its tiles and hydra_fillcompare's "Compared N charts: …" line now count those charts as "N with a score on one side only", with a matching tile. The parts add up to the total again. No score, path or record changes.
