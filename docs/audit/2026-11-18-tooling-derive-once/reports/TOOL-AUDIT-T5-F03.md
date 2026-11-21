# TOOL-AUDIT-T5-F03 (finder F03, agent a3ec402c353da118e)

Short names: M = the memory folder (C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory). H = C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout, HEAD af9dfac4). G = C:\Users\Patrick\.claude\gates. TR = docs/agents/test-rubric.md, TE = test-rubric-examples.md. Note: ripgrep skips the memory folder when searched from C:\Users\Patrick\.claude (git-ignored), so I searched M directly each time.

Rows are grouped by the question they share. Rows I did not reach are listed at the end.

## Group 1. How many charts does the test corpus (testdata/input) hold?
Rows: I7-126 (97 vs 98), I7-129 (CHECK(charts == 98)), I7-130 (shown() == 98), I8-115 ("about 115 charts").
Real count today: 98 song.ini files under H\testdata\input (116 chart-type files, counting variants).
Homes:
- H\docs\agents\ui-testing.md:48 "song folder = `testdata/input`, 98 charts" (states the value).
- H\docs\agents\test-rubric.md:11 "A 98th chart was added ... tests still expected 97" (a story, states both).
- TE:101 `CHECK(charts == 98);`, TE:113 `IM_CHECK_EQ(shown(), (size_t)98);` (quoted as bad examples).
- H\tools\test_hydra_ui_mcp.py:774 and :1070 `assertIn("charts=98", ...)` (a second hard-typed copy in a tool test).
- H\tools\mutate\test_run.py:65 "discovered 98 chart(s)" (fixture text).
- M\no-full-library-runs.md:11 "`testdata/input` (about 115 charts)".
- M\questions-describe-the-real-rule.md:24 "`[N/97]`".
- M\measure-small-explain-first.md:13 "326-chart run" (a different run, not the corpus).
What each tells its reader: ui-testing.md says the headless GUI starts with 98; the memory note tells a main session the corpus is about 115; the tool test fails if the GUI shows anything but 98. Main session vs agent: the memory note is read by the main session only, the docs by everyone. Card writers and worktree edges: not applicable to this value (a chart added in a worktree changes the number there, and the tool test in H would fail in that worktree).
Kinds: K2 (115 vs 98: the memory note drifted; 97 in the [N/97] example is also stale); K1 (98 in ui-testing.md and tools/test_hydra_ui_mcp.py, both agreeing today); K6 candidate (tools/test_hydra_ui_mcp.py types its own 98, and the C++ tests type theirs).
Splits: SPLIT 115 (M\no-full-library-runs.md:11) vs 98 (H\docs\agents\ui-testing.md:48, actual file count).
Test holding homes equal: none found in G\tests. The tool test (H\tools\test_hydra_ui_mcp.py:774) holds the GUI count to the typed 98, not to the folder.
Ruling: none found.

## Group 2. Pitch Dark / Blight / bank / fill / combo-step example numbers
Rows: I7-127, I7-128 (Pitch Dark chord 243, tick 128520, 128400), I7-132 (Blight chord 228, ticks 64800, 65280), I7-133 (bank rise {13440, 14400}, engine.cpp line 2015), I7-134 (fill reach 7285, 7200 ms, 85 ms, FL-19, FL-23), I7-135 (combo steps 10/20/30, SC-05), I7-131 (loop floor acts > 1000, D43).
Homes (all in H\docs\agents):
- Pitch Dark values: TR:48-49 ("chord 243, tick 128520", "tick 128400"), TR:130, TR:175; TE:23 ("chord 243, at tick 128520"; "tick 128400").
- Blight: TE:74 only in these docs (also TE:336 names Blight).
- 13440/14400: TR:25, TR:185, TE:33, TE:42, TE:83-85.
- 7285 / 7200 ms / 85 ms: TE:179-181.
- Combo steps: TE:224-244, 255, 282-311 (SC-05 cited; 10/20/30 typed).
- Owner page for the combo steps: H\docs\clone-hero\06-scoring.md:53 "### SC-05 The multiplier is one plus a tenth of the streak, up to 4". Owner pages for the fills: H\docs\clone-hero\04-fills.md:192 (FL-19) and :232 (FL-23). The 4-beat fill lead in Hydra's code is src/search/graph.h:71 `kCh11FillLeadBeats = 4` (H\docs\clone-hero\07-internals.md:209 names it).
- In tools: H\tools\ch_probe\probe_songs.py:108 and tests\test_early_fill_walk.py:26 cite FL-19 as a pointer; the 65280 hits in tools\ch_probe\tests\*.json are chart data, not a copy.
What they tell the reader: TR and TE use the numbers as worked examples (bad examples to be judged). TR:48-49 and TE:23 tell the reader the engine's record says 128520 and the function used 128400, so both docs hold the same run's output twice, in the page that forbids copying one run's output (TR:23 "A number 'pinned from one run' is one" snapshot number). The combo steps in TE:226-228 are typed in a second place (they are an example the page says is "typed in SC-05").
Kinds: K1 (Pitch Dark 128520/128400 in TR:48,49,130,175 and TE:23; the combo steps 10/20/30 in TE and SC-05); K3 candidate for I7-133: TE:85 "src/search/engine.cpp line 2015" names a code line (a line pointer that I did not re-check against engine.cpp).
Splits: none found. I did not open the Pitch Dark or Blight charts, so whether 128520 and 65280 still equal the engine's record today is unchecked.
Any test holding homes equal: none in G\tests (the gates know nothing about these numbers). H\tools\check_pointers.py exists (tests at H\tools\test_check_pointers.py) but I did not read what it checks.
Ruling: none.
I7-131 (acts > 1000, D43): one home in the docs I searched (TE:123); D43 is in the decision log, which I was told to treat as the user's words; I did not open it.

## Group 3. Early-fill reach (85 ms hit window) and the 60 ms early-fill slack
Rows: I8-059 (kHitWindowMs 85), I8-060 (kEarlyFillSlackMs 60), I8-089 ("rule number (85)"), I8-194 (budget W*(1+r), W=85), I8-088 (labels as reasons, same note family), I7-134 (the 7285 example).
Owner in code: H\src\core\model.h:101-106 "The per-side hit window ... One fixed number for the search and the display alike: the squeeze budgets, the Rating words and the report's timing tiers read it" `constexpr double kHitWindowMs = 85.0;`. The slack: model.h:108-112 `kDefaultEarlyFillWindowMs = 60.0`, "the user's own number, 'default 60ms, supports 0ms to 85ms'", with `kMaxEarlyFillWindowMs` = the hit window (:115). `fill_refuses(e_offset, window_ms)` is at model.h:338.
Other homes:
- M\early-fill-orange-85-d250.md:11 "`kEarlyFillSlackMs` (60 ms), read by `is_e0`" and :3 description, M\MEMORY.md:22 "reach kHitWindowMs 85 / slack 60".
- ~/.claude\skills\analyzing-sp-squeezes\SKILL.md:18 "W=85" and :24 "`85*(1+r)`" (no owner named).
- H\tools\ch_probe\README.md:101 and SPEC.md:302 "Hydra's model uses a flat 85 ms hit window", "`kHitWindowMs = 85.0`".
- H\docs\agents\test-rubric-examples.md:179 "the 85 ms hit window".
- Old plans and handoffs: H\docs\superpowers\plans\2026-11-12-early-fill-live-check.md:253,296 and H\docs\handoffs\2026-11-12-early-fill-live-check-handoff.md:52 name `kEarlyFillSlackMs`.
Kinds: K8 and K2 on I8-060. `kEarlyFillSlackMs` does not exist in H\src (I searched the whole repo; only plans, handoffs and audit files name it). The code now has a setting `early_fill_window_ms` defaulting to `kDefaultEarlyFillWindowMs` (model.h:112, :985), and `fill_refuses` takes `window_ms` (model.h:338); the note's "kHitWindowMs ... read by fill_refuses" and "kEarlyFillSlackMs ... read by is_e0" no longer describe the code. I did not read `is_e0`'s current body. K1 for the 85 in the skill, the ch_probe pages and TE (the skill names no owner).
Splits: SPLIT M\early-fill-orange-85-d250.md:11 (orange side reads kHitWindowMs, white slack is a fixed constant kEarlyFillSlackMs 60) vs H\src\core\model.h:101-115, :137-143 (one early-fill setting `early_fill_window_ms`, default 60, on both sides, up to 85).
Confirmed rulings: H\docs\agents\rulings\code-and-engine.md:23 "change the early fill search to allow orange timing up to 85ms."; :24 chose "Orange only (Recommended)" to "Which side of the early-fill window should go to 85 ms?"; :49 "the hit window is +/- 85ms, the 70ms is an outdated figure. we can change that."; :57 (2026-11-15) "analysis settings panel, default 60ms, supports 0ms to 85ms". Also H\docs\agents\rulings\clone-hero.md:54 chose "85 ms both sides (Recommended)" (in the Confirmed section; the file's Confirmed header is at line 7, Unconfirmed at 67). Note :24 (orange side only gets 85) and :57 (a setting with one window, 0 to 85) are both Confirmed and describe different designs; the second is later.
Test holding homes equal: none in G\tests. The C++ test files are outside my scope (H\docs\audit\2026-10-31-stale-comments\questions\q3.md:77 mentions tests/test_model.cpp rows for `kHitWindowMs` and `kEarlyFillSlackMs`).
Edges: main session reads the note; agents never see it; the skill is read by whoever invokes it (a main session or agent).

## Group 4. How far Hydra looks for squeeze candidates (500 ms) and the "one check"
Rows: I8-161, I8-191 (also I8-195, I8-196 are in the same skill, not reached).
Owner in code: H\src\core\model.h:117-121 `kSqueezeWindowMs = 500.0` and :129-135 `within_squeeze_window`, comment "The one window check: the search graph, the engine's tail rows, the stored rows ... and hydra_replay all ask it." Callers: src/search/graph.cpp:160,329; src/search/engine.cpp:2044; src/core/model.cpp:845; src/core/sqout_chord.h:32,123 (so the graph is not the only caller).
Homes:
- M\sp-squeeze-mechanics.md:13-15 "kSqueezeWindowMs = 500 ms of the deactivation node (src/core/model.h; `within_squeeze_window` is the one check, used by src/search/graph.cpp)"; :3 description; M\MEMORY.md:38 "within 500 ms".
- ~/.claude\skills\analyzing-sp-squeezes\SKILL.md:10 "within 500 ms of a deactivation node (`kSqueezeWindowMs`, src/search/graph.h)".
- M\min-path-delta-proof.md:27 "500ms+ clearance around deacts" (a different use of 500).
Kinds: K8 (skill SKILL.md:10 names src/search/graph.h; the constant is in src/core/model.h:121, and graph.h only mentions it in a comment at :216); K1 (500 in three notes).
Splits: SPLIT SKILL.md:10 (constant in src/search/graph.h) vs M\sp-squeeze-mechanics.md:13 and H\src\core\model.h:121 (src/core/model.h). Same file also; the memory note's "used by src/search/graph.cpp" narrows the callers (the code lists more).
Confirmed rulings: none found for 500 ms.
Test holding homes equal: none in G\tests.

## Group 5. The 3 ms backend leeway and max tied paths
Rows: I8-158 (3 ms leeway), I8-063 (max_tied_paths default 4).
Owner in code: H\src\core\rules.h:32 `double backend_leeway_ms = 3.0;`, :38 `int max_tied_paths = 4;`, :6 comment "In hydra_rules.ini each field is set by its own name, e.g. `max_tied_paths = 4`." H\src\core\model.h:98-99 "The backend leeway edge is a user rule: core::Rules::backend_leeway_ms".
Homes: M\sp-end-is-one-point-leeway-is-delgars.md (:11 quotes "3ms"), M\MEMORY.md:7 "3 ms"; M\engine-tie-merge-prune.md:14 "`Rules::max_tied_paths` (src/core/rules.h, default 4, ...)" (names owner and states value); H\tools\mutate\first_use_table.md:45 mentions "the leeway" only.
Kinds: K1 (3 and 4 each stated in a memory note beside the owner). The rule.h owner claims in the notes match the code today (checked). Both notes name the owner, so each is a "pointer that also states the value", a home.
Splits: none.
Confirmed ruling: not in the files I searched. The note quotes the user (2026-10-29: "the 3ms rule is for whether a backend is included in the optimal score or uncounted"); I did not look up this line in rulings/.

## Group 6. Status-line interval (every 5 minutes) and the agent budget
Rows: I8-066 (every 5 minutes), I8-123 (100 calls or 20 minutes warning, 150 call cap).
Owner in code: G\progress.py:14-16 `CALL_WARN, CALL_STOP = 100, 150`, `MINUTES_WARN, MINUTES_STOP = 20, 30`, `STATUS_NAG_MINUTES, STATUS_DENY_MINUTES = 5, 10`.
Homes: H\CLAUDE.md rule 7 (not re-read in full here; the brief says it is short form); H\docs\agents\agent-rules.md:13 (rule 7: "every 5 minutes ... 100 tool calls ... 150 ... 20 minutes ... 30 minutes"); brief-preamble.md:13, :21, :73; derive-once-review.md:9, :110; doc-review.md:9, :100; G\README.md:124; M\every-brief-carries-status-rule.md:15 "every 5 minutes"; M\MEMORY.md:45; The reviewer-cap statement in I8-123 is in a note (not located again; the row says "the reviewer cap is 150 calls like everyone else").
Test holding homes equal: YES. G\tests\test_docs_numbers.py. `test_status_interval_is_the_nag_minutes` (:116) holds every "every N minutes" in CLAUDE.md, agent-rules.md, brief-preamble.md, derive-once-review.md and doc-review.md to `progress.STATUS_NAG_MINUTES`; `test_every_call_count_is_a_budget_number` (:110) holds each "N tool calls" to CALL_WARN or CALL_STOP; `test_agent_rules_rule_7_quotes_the_budget` (:122) and `test_preamble_quotes_the_budget` (:182) check rule 7 and the preamble quote the right numbers. The test's DOCS list (:25-26) does not include the memory notes, MEMORY.md, the agent type files (~/.claude\agents), gates\README.md (README is checked only for names, :176), skills, or this brief.
Kinds: K1 (5, 100, 150, 20, 30 stated in docs the test covers and in notes it does not); K5 candidate (briefs and docs spell the numbers out instead of pointing at progress.py; the test is the guard).
Splits: none.
Confirmed rulings: H\docs\agents\rulings\agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable"; :26 "the status updates should be brief but relayed in plain english". The 5-minute figure itself is in Unconfirmed line 43 (2026-10-04 "status every 10 calls/5 min, soft cap 100 calls (hand off), hard cap 150") which is the older rule (calls count), now replaced; this is an Unconfirmed line, so a home, not a ruling.
Edge: agent types that skip the preamble (card writers) get the status rule only from their own brief; M\every-brief-carries-status-rule.md:15 says "No gate checks a brief for this sentence today".

## Group 7. Does a hook check that a brief carries the status sentence?
Row: part of I8-066 (the note).
Homes: M\every-brief-carries-status-rule.md:3 description "the dispatch gate now denies briefs that lack it"; M\every-brief-carries-status-rule.md:15 "No gate checks a brief for this sentence today, so the brief has to carry it."
Kinds: K2/K8. The same note says both. G\review_dispatch.py contains no "status" text (searched), and G\tests\test_docs_numbers.py:31 lists `reviewer_dispatch_gate` as a retired hook word.
SPLIT: M\every-brief-carries-status-rule.md:3 (a dispatch gate denies briefs) vs M\every-brief-carries-status-rule.md:15 (no gate checks).

## Group 8. Batching and fan-out of agents (about 8 jobs per agent, 3-4 agents per round)
Rows: I8-008 (index), I8-090, I8-091.
Homes: M\lean-agent-fanout.md:3 (frontmatter), :20 "Batch small uniform jobs (merges, rewrites) about 8 per agent.", :21 "use 3-4 agents ... More agents per round makes 'stop when a round finds nothing' harder to reach"; M\MEMORY.md:39. The brief that sent me and the families.md/line F03 hold 53 rows for one finder, which is not what the note says (about 8 per agent); I only note it, not a home.
Confirmed ruling: none. The nearest user words are in rulings/agents-and-workflow.md:48, which sits under `## Unconfirmed` (line 32 onward): "do we really need 54 merge agents?" and "was running 8 agents per round really necessary?" (a home, not a ruling). That line says "8 agents per round" was too many; the note's number "about 8 jobs per agent" and "3-4 agents" are not in that line.
Kinds: K7 (a prose rule no hook or test checks, stated in three places in the memory folder, none named as home); K1.
Hooks: no hook in G or tool in H\tools reads these numbers (searched G\*.py, G\*.md, H\docs\agents).
Splits: none.

## Group 9. How many builds at once; one build per wave
Rows: I8-119 (one combined build per wave), I8-121 (the user's "3 builds at once?" quote).
Owner in code: H\tools\build_slot.ps1:71-75 "How many builds may run at once ... Decision D49 ... the user's limit of three is in docs/agents/rulings/agents-and-workflow.md." `$BuildSlotCount = 3`.
Homes of "three": H\CLAUDE.md:76 (rule 14) "At most three builds run at once"; H\docs\agents\brief-preamble.md:57; H\docs\agents\agent-rules.md:20 (long line, rule 14); H\tools\test_build_pass.ps1:114-117,141,199 (typed 3 in the test of the owner); M\one-build-per-wave.md:13 quote "3 builds at once? what? i didnt accept doing lots of builds"; M\archive\parallel-by-default.md:25 ("three at once across the machine").
Test holding homes equal: H\tools\test_build_pass.ps1 tests the owner with literal 3 (a second copy; it does not read $BuildSlotCount). None of G\tests holds CLAUDE.md or the preamble's "three" to build_slot.ps1.
Kinds: K1 (three in CLAUDE.md, preamble, agent-rules, the test). I8-121 is a quote of the user's reaction, not a limit (the row says so). I8-119 has one home (M\one-build-per-wave.md:11, description at :3). Nothing else states "one combined build per wave"; I searched H\CLAUDE.md, H\docs\agents\*.md, .claude\commands\*.md, tools, G, ~/.claude agents, M.
Confirmed rulings: H\docs\agents\rulings\agents-and-workflow.md:8 "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." and :9 "any tests that require a build are done by the main session, not agents." (a ruling found; both sit under `## Confirmed`).
Splits: none (the note says a wave gets one build; the ruling says one build per session at a time; they answer different questions).

## Group 10. Min nonzero path-score delta and related numbers
Rows: I8-104, I8-105 (w = v*d*m, v in {50,65}), I8-107 (18,610 charts; 49,719 gaps).
Homes: M\min-path-delta-proof.md (description :3, body :18-32) and M\MEMORY.md:44 "floor 5 witnessed / 20 usual / 10 real". The related fact "Multiples of 5 follow from 50, 65, 100 and 130 (SC-01, SC-02)" is in H\docs\clone-hero\06-scoring.md:283 (an owner for the point values; it says nothing about 5/10/20 floors).
Kinds: one home for the floors (the note) plus its index twin. I searched H\docs\clone-hero (counts), H\src (search for the "five-point" and floor wording), H\tools, H\docs\agents; nothing else states the floors. The note's own flag (per the inventory row I8-104) says its pointers into engine.cpp no longer hold.
Library size: the note says 18,610; M\ other notes say 18,811, 18,755, 19,000; see I8-107/118/171 below.

## Group 11. Library size cited in notes
Rows: I8-107 (18,610), I8-118 (18,811), I8-171 (18,755).
Homes: M\min-path-delta-proof.md:29 ("18,610 charts"); others per the inventory row. No home for the library's size in H\docs\agents (searched 18,811 / 18,610 / 18,755 / 19,000).
Kinds: K2 across the memory folder (the cited sizes differ), but each is a count in a story, as at a date. I did not check which count is right.
Splits: SPLIT 18,610 (min-path-delta-proof.md:29) vs 18,811 (I8-118's note) vs 18,755 (I8-171's note) for "how many charts are in the library"; no owner in code or DB was read.

## Group 12. Same goal failing twice means ask the user about the design
Row: I8-132.
Homes: M\question-the-design-before-patching.md:3, :11 "When the same goal fails twice and each fix works around the last failure, stop and put the design itself to the user, with the alternatives"; M\MEMORY.md:17 "2+ failed runs on one goal". Nearby but not the same: H\docs\agents\agent-rules.md:9 (a plan changing a display takes its design to the user first), and the preamble ("If your last two status lines show the same step and no new result, you are not making progress. Stop and report" brief-preamble.md:85).
Kinds: K7 for the memory rule (no hook checks it; two homes); K3 candidate for the preamble line 85 (two stuck status lines: the agent stops and reports; the note: the main session asks the user; different actors, similar trigger).
Splits: none. "Twice" (note body) vs "2+" (index) agree.
Hooks: none found in G for the note's rule (searched G\*.py,*.md for "twice", "same goal", "failed runs").

## Group 13. Timing, long-capture and CI rules in notes
Rows: I8-172 (timings come from hydra_bench), I8-095 (timing-run size), I8-097 (explain before a run), I8-027 (check long captures early), I8-174 (CI step timeout), I8-180 (decode ceilings).
Findings:
- I8-172: no other home in H\docs\agents or G for "a timing put to the user comes from hydra_bench" (searched docs\agents, gates). H\docs\agents\this-machine.md:91 and ui-testing.md:362 mention timings and hydra_bench but state a different rule (database size and build target). One home.
- I8-095: one home (note measure-small-explain-first.md:13 "326-chart run"). Nothing in G or H\docs\agents.
- I8-097: the note quotes a hook ("say what changed since the last run and why this one will end differently"); I found no such text in G (searched "what changed since", "will end differently", "bench_run" in G\*.py,*.md,*.json). So the quoted hook message does not exist in G today. K8 candidate: the note names a hook that I cannot find. (The hook may have been retired; unchecked.)
- I8-027: no other home found (searched docs\agents, docs\clone-hero, tools\ch_mcp, tools\ch_probe READMEs, G).
- I8-174: the note says "A CI step that has never run on a runner gets `timeout-minutes` of a few minutes on the step itself." H\.github\workflows\ci.yml:313 shows 15 min, :324 3 min, :345 5, :352 8, :359 3, :365 3, :369 3 on steps; fuzz.yml 15-60 min at job level. No doc in H\docs\agents states the rule (searched). One home for the rule; the YAML is the practice, not an owner.
- I8-180: one home in M (video-decode-ceilings.md, index :36). The video-tools repo is outside scope.
Kinds: single homes except I8-097 (K8 candidate).

## Rows not reached
I8-012, I8-050, I8-057, I8-080, I8-126, I8-148, I8-154, I8-163, I8-195, I8-196, I7-155, I7-189, I7-224, I7-226. (Not searched for other homes. I8-195 and I8-196 are SP-phrase mechanics in the skill; I did not look for twins in M\sp-squeeze-mechanics.md or docs\clone-hero.)
I7-131 and I7-133: only partly done (see group 2).

## Side notes
- H\docs\agents\agent-rules.md:13, G\tests\test_docs_numbers.py:26: the test covers five Hydra docs only; memory notes, ~/.claude\agents\*.md, model-postures.md and gates\README.md can drift from progress.py unnoticed.
- M\one-build-per-wave.md: the inventory row says its modified time is later than today's date (2026-11-19); the file's own text is dated 2026-11-18.
- H\tools\test_build_pass.ps1:114-117,141 types the build limit 3 itself instead of reading $BuildSlotCount in build_slot.ps1:75 (K6-style second copy in a test).
- Hook gap: the retired `reviewer_dispatch_gate` is still named in a memory note's description as if live (see group 7).
