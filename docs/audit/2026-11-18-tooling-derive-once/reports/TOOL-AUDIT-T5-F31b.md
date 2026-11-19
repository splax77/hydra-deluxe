# TOOL-AUDIT-T5-F31b (re-send finder, agent aebf5d57d28342df4)

Read-only. Nothing in either repo changed. Short names: HY = C:\Users\Patrick\Downloads\Hydra\hydra-test, CL = C:\Users\Patrick\.claude, MEM = CL\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory, AR = HY\docs\agents, VT = C:\Users\Patrick\Downloads\Hydra\video-tools (outside the scope list; opened only because MEM:fc-video-workflow names it as the owner for I8-071).

Rows taken (from F31's "not reached / shallow" list): I7-170, I7-180, I7-208, I7-124, I7-161, I8-011, I8-016, I8-026, I8-036, I8-038, I8-071.
Hook test holding any of these homes equal: none found in CL\gates\tests (searched for the phrases of each row). The one hook test near them is CL\gates\tests\test_agent_files.py:16-21, which holds only the phrase lists the planner and reviewer files must contain ("Calls:", "never tried", "unchecked", "Premise map"); it holds I7-170's planner and reviewer homes, nothing else.

Confirmed rulings (the user's words) are marked RULING. Homes under `## Unconfirmed` and memory notes are homes.

---
## I7-170 Go request shows the call list (AR\premise-map.md:78)
Question: before a live run, what must the message that asks the user to go hold?
Homes:
- AR\premise-map.md:78 "Before a live run, the main session shows the user the step's call list. "The shape of a live run" on `docs/agents/clone-hero-mcp.md` says what that go request holds." Pointer only.
- AR\clone-hero-mcp.md:58 "Before the main session asks the user for a run, its message shows that step's `Calls:` line exactly as the plan holds it. The message then names each `(never tried)` call on that line, and each `unchecked` Premise map row the step uses. The user's go answers that message, not a bare step name. `docs/agents/premise-map.md` owns the shape of the call list and the map. No hook checks this message; the plan reviewer checks that every step has its `Calls:` line." This is the home (states the content).
- CL\agents\planner.md:13: planner writes a `Calls:` line per live step (a `### LS` heading), each call marked seen working or never tried. Says nothing about the go message.
- CL\agents\reviewer.md:13: reviewer checks each live step holds a `Calls:` line and no step uses an `unchecked` row as settled without naming it.
- AR\premise-map.md:54 section "The call list of a live step" owns the line's shape (the same page, :5 says it is the one home).
Adjacent, not the same question: MEM\trace-pass-inputs-before-live-runs.md:11 (list every value the pass line depends on before a live run).
Who acts: premise-map.md:78 and clone-hero-mcp.md:58 tell the main session; planner.md the planner; reviewer.md the plan reviewer. Edges: at the limit, n/a. Main vs agent: only the main session sends the go request; agents never. Card writers: no text reaches them. Worktree vs main: not stated.
Kinds: K1 (the call-list rule is stated on two pages, one a pointer), K5/K7 candidate: clone-hero-mcp.md:58 says in words "No hook checks this message"; the rule is prose with no check on the message itself, and only the plan-side `Calls:` line is checked (reviewer, and the phrase test above).
Splits: none.
RULING: none found speaking to it (searched AR\rulings for "Calls:", "go request", "call list").

## I7-180 Counter-example owner (AR\owner-map.md:31)
Question: which code owns "which side of the SP end a note sits on"?
Homes:
- AR\owner-map.md:31 "`SPSqueeze::needs_no_timing` (`src/core/model.h`) is the example: its comment says which side of the SP end a note sits on is "not this one"."
- HY\src\core\model.h:428-432 comment: "...The badges, hardest() and the Paths tab's sentences read it. Which side of the SP end the note sits on is core::paid_by_sp_walk's question, not this one." The quoted words match.
- HY\src\core\model.h:298 "The badges and the search read SPSqueeze::needs_no_timing, not this." (about backend_row_timing; a second pointer to the same split).
- HY\src\core\backend_value.h:26-28 comment on `paid_by_sp_walk(offset_ms)`: "The SP walk pays every chord at or before the SP end (the window is inclusive at the deactivation node), before any backend pricing happens." It claims the question itself. So the owner claim holds.
- HY\src\core\backend_value.h:30-31: a tick form of the same edge ("is this chord after the SP end?"), whose comment says it "agrees with paid_by_sp_walk wherever time runs forwards". Two code places answer the same edge, one by offset, one by tick. The comment claims agreement; I did not check whether a test holds them equal. Kind K6 candidate (comment names it).
- MEM\screen-words-not-code-names.md:11 uses `SPSqueeze::needs_no_timing` as a code name the user rejected in speech; not a home for the rule.
- MEM\sp-end-is-one-point-leeway-is-delgars.md:17 mentions `rate_activation` as an open conflict about the SP end (not read in depth; side note).
Who acts: scouts reading owner-map.md; no one else is told. Edges: none apply.
Kinds: K1 (doc quotes a code comment, and the two agree today).
Splits: none found. Searched AR (incl. rulings), CL agents/skills/gates/tests, CL\CLAUDE.md, HY\CLAUDE.md, MEM for the identifiers.
RULING: none.

## I7-208 Integrator report contents (AR\integrate.md:60)
Question: what must an integrator's report hold?
Homes:
- AR\integrate.md:56 "Report the wave branch's head commit hash and the range `main...<head>`." (the hand-off to the reviewer).
- AR\integrate.md:60 "Give the wave branch, its worktree, and its head commit hash. List each task branch you joined with its merge commit hash. Then, in plain sentences: the owners `main` gained since the fork, any task copy you folded into one of them, each conflict and how you resolved it, and the precheck's output. End with anything you stopped on."
- Only other mention of integrators: AR\brief-preamble.md:3 (lists integrators among agents the page binds); its last section (preamble "Your final report") gives a different default list (branch, worktree, commit hashes, summary, tests run with counts, numbers, stops) for any agent whose brief names none.
- No integrator agent file exists in CL\agents (executor, planner, reviewer, scout, card-writer only). No hook reads an integrator's report (searched agent_stop.py, review_dispatch.py for the fields).
Table: integrate.md:56 asks for head hash + range (to be passed on by the main session); :60 asks for head hash, worktree, each task branch's merge hash, owners gained, folds, conflicts, precheck output, stops; preamble default asks for commit hashes, test commands with pass counts. At the limit: n/a. Main vs agent: the main session takes the report; no check. Card writers: n/a. Worktree vs main: report names the worktree.
Kinds: K1 (two forms of the report inside one page; compatible); K7 candidate (report shape is prose only; no hook or test checks it).
Splits: the preamble default says "test commands with pass counts" while integrate.md:60 says "precheck's output" and agents may not run tests; not a conflict (integrator runs only precheck) but two lists for one report. The brief for an integrator decides which applies; nothing says.
RULING: none.

## I7-124 Audit set-up: calibration, order, corpus (AR\test-rubric.md:189-205)
Question: how is a test audit set up (calibration file, order of work, the engine record, the four mechanical checks)?
Homes:
- AR\test-rubric.md:189 "That page's calibration stage applies: before the auditors start, one agent gets a file whose bad tests are known, with no hint of what they are. Do not use `tests/test_sp_end.cpp`, `tests/test_sp_meter_curve.cpp` or any other file the examples page quotes... The answer key must hold at least one test whose framing is the bug..."
- AR\audit-method.md:44 "Before the finders start, one agent gets a file with known problems and no hint of what they are. It must find them. ... If the calibration agent misses a known problem, stop and report the miss to the user, and fix the brief before sending anyone else." and :46 "The calibration brief must not describe the known problems, even as an example." (and :158-160 a retelling of the 2026-10-03 failed first calibration, card 2026-10-03-b5aa7c31).
- AR\test-rubric-examples.md quotes the two files the rubric forbids as calibration files at lines 13, 29, 50, 64 and further (nine matches for the two file names): the rubric's "do not use" list is the examples page's quote list, stated by file name in two pages. If the examples page gains a file, the rubric's "or any other file the examples page quotes" covers it by words, but the two named files are a copy by name.
- AR\test-rubric.md:191 order "Display tests go first"; :193 "a handful of real charts"; :195 "Only the main session can run the first two (agent rule 14)"; :197 corpus-change check; :199 plant bugs from question 8; :201 display-vs-engine check "No such test exists today (question 6)"; :203 standing scan, with `tests/single_owner/special.cpp` and `flags_line` in `tests/single_owner/engine.h` (that function exists: engine.h:105).
- Rule 14's "agents never build / only the main session runs the tests" is restated at :195 (also in agent rules, preamble, reviewer.md:21, executor.md:11). Not a new value, a restatement.
- Planting bugs is also done by HY\tools\mutate (`first_use_table.md:7`: "planted bugs"); test-rubric.md:199 describes the same act by hand and does not point at the tool (K7 candidate: no home named for "plant bugs").
Table: audit-method says stop and report to the user on a calibration miss; test-rubric says nothing about a miss (it defers with "applies"). Test-rubric adds the file ban and the framing-is-the-bug requirement, which audit-method lacks (audit-method's own calibration used a source file, not tests). Main session vs agent: the main session plants bugs and builds; auditors read. Card writers: n/a. At the limit: n/a. Worktree: :197 throwaway worktree.
Kinds: K1 (calibration stage stated on two pages), K3 candidate (test-rubric applies the stage to tests but adds requirements the stage page does not hold), K7 candidate (plant-bugs and corpus-change checks: prose, nothing named as home; mutation tool exists).
Splits: none in values. The claim "No such test exists today (question 6)" was not checked in code.
RULING: AR\rulings\agents-and-workflow.md:8-9 (Confirmed) "any tests that require a build are done by the main session, not agents." (speaks to :195).
Test holding homes equal: none.

## I7-161 Quote only numbers the user can see (AR\this-machine.md:93-95)
Question: before quoting a stored number, what must an agent say, and where is what the Paths tab shows built?
Homes:
- AR\this-machine.md:95 "The user checks every number in the app... before you quote a stored number, say which path it comes from (the optimal path, a path within the limit, or the best all-0 path) and whether the app displays it. What the Paths tab shows is built by `build_path_buttons()` and `build_activations()` in `src/app/path_view.cpp`. ... (agents run neither: agent rule 14)".
- Code check: both functions exist, HY\src\app\path_view.cpp:225 (`build_activations`) and :529 (`build_path_buttons`). The owner pointer is true today.
- Other homes of the same duty: AR\agent-rules.md:18 (rule 12, changes the user can see); CLAUDE.md rule 11 (a question lists what the user will see). MEM\screen-words-not-code-names.md:3 "never offer my own arithmetic as a figure or a choice". Different duties on the same theme (what the user sees), not a copy of this one.
- The three path kinds named in :95 ("optimal path, a path within the limit, best all-0 path") are a list of kinds; I found no other doc that lists the same three outside CONTEXT.md/ui pages (not searched; side note).
Who acts: agents. Edges: main vs agent: the main session may read the app (hydra MCP, hydra_uitest), agents may not; this is rule 14 restated. Card writers: not reached by the text.
Kinds: K1 (rule 14 restated in a parenthesis). One text home for the duty.
Splits: none. RULING: none.

## I8-011 A chosen mockup's values are the design (MEM\approved-work-is-not-reasked.md:15)
Question: when the user has picked a mockup, may a plan offer today's look as an option?
Homes:
- MEM\approved-work-is-not-reasked.md:15 "A chosen mock's values are the design; today's look is never an option beside it."
- MEM\MEM index MEMORY.md:8 (new-screens note) "one design, no option toggles (10-31)"; MEM\new-screens-start-from-a-clickable-mockup.md:11,15,17 says a screen starts as a mockup the user clicks through and approves before a plan (same rule family, different part: how a design is reached; the "one design" phrase is the index line only).
- MEM\approved-work-is-not-reasked.md:11,13: the same note's two other cases (re-asking an approved build/task).
- MEM index MEMORY.md:5 "after a yes, a finding that makes the work look less needed is reported, never re-asked."
- AR\agent-rules.md:18 (rule 12: a visible change the user's words did not ask for is a question) and CLAUDE.md rule 12: the opposite direction (do not ship unasked visible change); and rule 3 "No work starts without the user's approval". No page states the "approved is not re-asked" rule; it lives in memory only (K7 candidate).
- CL\agents\planner.md: nothing on mockups (searched "mock").
RULING (Confirmed): AR\rulings\ui-and-display.md:9 "yes i approved the mockup and asked for option B. so why are you offering what we have today as an option?" (session ae1c19f5). Also :77 "why did you create huge blocks? that isn't the design i approved..." and :75 "go with your recommendations on all three. design approved."
Edge: AR\rulings\ui-and-display.md:123 (Unconfirmed section) says consistency of row layout matters more to the user than matching an approved mock; so "the mock's values are the design" has a stated exception in an unconfirmed line (note, not a ruling).
Kinds: K7 candidate (memory-only rule, no page or hook); K3 candidate between memory line 15 and the unconfirmed :123.
Splits: none in values.

## I8-016 Pricing a run that beats optimal on an affected chart (MEM\ch-disco-noflip-rule.md:15)
Question: how is a video run priced when it beats optimal and the chart carries `drums0dnoflip`?
Homes:
- MEM\ch-disco-noflip-rule.md:15 "When a video run beats optimal under both fill rules, grep the MIDI for `drums0dnoflip`; price the run against a scratch copy with the marker renamed (`drums0dNOFLIP`) and say it is priced under the bugged game rule." Only home of this procedure.
- HY\docs\clone-hero\02-note-list.md:253-260 (NL-21) states the game's behaviour and the ruling and D19 ("`drums0dnoflip` stays off"), but I found no recipe for pricing a run under the game's bugged rule there or in AR; the grep for "scratch copy" and "renam.*marker" in docs\clone-hero and AR found none. NL-23 (:273) counts charts that carry the marker.
- Other statements about a run that beats optimal (different cause, same symptom):
  - CL\skills\analyzing-sp-squeezes\SKILL.md:55 "a run that beats the engine's optimal is a version problem or a bug, never a real score, and a Clone Hero 1.0 run is priced with `--legacy-fills`."
  - VT\README.md:41-46 (checklist item 1) says the same in nearly the same words, and VT\README.md:710-727 ("When the run beats optimal"): usual cause is an older game version, rerun with `--legacy-fills`, "That is the whole recipe".
  - MEM\fc-video-workflow.md:11 says the README holds "what to do when a run beats optimal".
  - MEM\dmleaderboards-comparison.md:26-29 "Above-optimal scores are NORMAL, not a bug... many leaderboard scores were set on an older Clone Hero version... label them 'above optimal'; never flag as impossible/error."
  - AR\public-doc-style.md:17 "Rows above optimal are expected: many leaderboard scores were set on older Clone Hero versions..." (player-facing wording).
Table: ch-disco note: beats optimal under both fill rules -> check for the marker, rename, price on a scratch copy. SKILL.md:55 and VT README:41: beats optimal -> a version problem or a bug, price with `--legacy-fills`. VT README:710-727: no scratch database. dmleaderboards note: above optimal is normal; do not flag as error.
Kinds: K2/K3 candidate and a split (below), K1 (SKILL.md:55 copies VT README:41-46), K7 (the scratch-copy recipe is one memory line, with no tool or page named as its home).
Split: "a run that beats optimal is a version problem or a bug, never a real score" (SKILL.md:55, VT README:41) against "above-optimal scores are NORMAL, not a bug... never flag as impossible/error" (MEM\dmleaderboards-comparison.md:26-29). They are about different sources (a video run you are pricing versus leaderboard rows), but both say what to call a score above optimal and they give opposite verbs.
Split: the VT README says the whole recipe for a beating run is `--legacy-fills` with no scratch database; the ch-disco note adds a scratch copy with the marker renamed. Different causes (1.0 fill rule vs the dnoflip marker), but a reader of the README alone never hears of the second.
RULING (Confirmed): AR\rulings\clone-hero.md:53 "this is a bug in clone hero, not an intentional feature. it will be fixed in a future update." (session 1b0a7cd8) and :47 for the plain-Drums dynamics case; neither speaks to how a run is priced.

## I8-026 SP activation facts cited (MEM\check-an-edge-case-can-happen.md:11)
Question: how many bars of Star Power does an activation need, and how many remain after SP runs out?
Homes:
- MEM\check-an-edge-case-can-happen.md:11 "activation needs 2 bars and SP running out leaves 0 or 1 (`src/search/engine.cpp` `branch_activate` and `create_deactivated_path`)".
- Code owner of the 2: HY\src\core\timing.h:50-51 comment "Clone Hero's rule: an activation needs two banked bars of Star Power." with `kSpActivationBars = 2`. Used in engine.cpp:982-983, :1001 (`branch_activate`: `if (p.sp_bars < kSpActivationBars) return false;`), model.cpp:703, path_view.cpp:539, pb_text.cpp:72-74 (the user-facing message is built from the constant), replay.h:338.
- Code check of "0 or 1": engine.cpp `create_deactivated_path` (about :1099-1105) sets `c.sp_bars = sq ? 1 : 0;`, so the claim holds in code today. Its cited functions exist.
- Same number in other words, a different fact: SKILL.md:22-23 "SP extends +2 measures" and AR\test-rubric.md:21 "one Star Power bar lasts two measures": this is the length of a bar. Owner: `kMeasuresPerSpBar = 2` in HY\src\core\timing.h:268. docs\clone-hero\05 states it too (line 109, 129 per its Hydra-owner lines). The memory note states the activation count (2) in prose; SKILL and test-rubric state the bar length (2 measures); easy to mix.
Who acts: main session reading the note. Edges: none.
Kinds: K1 (value copied in memory; the code carries it only as the constant), plus a pointer that names two functions by name (drifts if renamed).
Splits: none. 
RULING (Confirmed): AR\rulings\questions-and-audits.md:9 "i dont understand why you always try to invent insane edge cases, this one isnt even possible in clone hero..." (the note's quote).

## I8-036 "Out of scope" lines (MEM\counts-on-screen-must-agree.md:24)
Question: may a plan carry an "Out of scope" list, and what does it need?
Homes:
- MEM\counts-on-screen-must-agree.md:24 "A plan's 'Out of scope (for the user to confirm)' line needs a recorded yes, or it is asked again."
- CL\agents\planner.md:13 "A promise with no task is a planning gap. Either give it a task or move it to an 'Out of scope' list for the user to approve. Never leave a promise only in the notes." (planner is told to keep an Out-of-scope list the user approves).
- MEM\search-web-before-finalizing-plans.md:11-15 and MEM index line 29: "fix 'pre-existing' items, don't list them"; "Do not leave a 'left alone' list of related defects; either fold them into the plan or ask."
- AR\rulings\docs-and-writing.md:19 (Unconfirmed section, which starts at :17): the user "treats a 'pre-existing, out of scope' list as a to-do list, not a disclaimer."
- AR\premise-map.md: no mention of scope lists (searched). AR\agent-rules.md rule 12 (CLAUDE.md rule 12): unasked visible change is a question.
- Archived copy (not scope): MEM\archive\never-keep-a-split-quietly.md:18.
Table: counts note: an out-of-scope line needs a recorded yes, or it is asked again. planner.md: put unplanned promises in an Out-of-scope list the user approves. search-web note: do not leave a "left alone" list; fold in or ask. Unconfirmed ruling: out-of-scope list = to-do list. Main vs agent: planner (agent) writes the list; main session gets the recorded yes. Card writers: n/a. 
Kinds: K3 candidate and split (below); K7 candidate (rule is memory-only; no hook or page checks the recorded yes).
Split: planner.md says a promise with no task may go on an "Out of scope" list for the user to approve; the search-web note and the unconfirmed ruling say not to keep out-of-scope or left-alone lists at all (fold in or ask). They can be read as compatible (user approves each item), but they tell the planner different default acts.
RULING: none Confirmed found speaking to it (docs-and-writing.md:19 is Unconfirmed).

## I8-038 "One owner per numbering" (MEM\counts-on-screen-must-agree.md:18)
Question: which code owns the numbering (line numbers) Claude reads in tool replies, and how many numberings may there be?
What "numbering" means here: the session card 2026-10-14-719dc0ea (AR: HY\docs\sessions\2026-10-14-719dc0ea.md, Roadblocks) says a failed `hydra_run` had two line numbers for one line (one from Hydra, one from the helper); the user rejected it. So the numbering is line numbers in the Hydra MCP tool replies.
Homes:
- MEM\counts-on-screen-must-agree.md:18 "The rule covers anything Claude or an agent reads (tool replies, logs), not only the GUI. One owner per numbering." Claims a rule; names no owner.
- HY\docs\adr\0030-hydra-owns-every-live-command-rule.md:50 table row "Line numbers | Script mode: the interpreter. Live mode: none from Hydra; the helper numbers the lines of a `hydra_run` script, the only numbering there"; :85-98 tells the story and says "a live reply names no line number, and the helper numbers only the lines of the script Claude sent it."
- HY\tools\hydra_ui_mcp.py:12-16 docstring "The helper owns ... the line numbers of a hydra_run script"; :691-695 comment ("live replies carry no line number, so 'stopped at line N' is the only one, counting the script's own lines"); :698 and :708 (`enumerate(lines, 1)`, `stopped at line {i} of {len(lines)}`).
- HY\tests\ui\uitest_commands.cpp:281-291, :432 (the script interpreter prints `!! failed at line %d` and `line %d: %s`, counting every line of its own script).
- AR\ui-testing.md:97 ("`!! failed at line N: <line>`" in script mode, `line N: <reason>` in the log), :229 (`hydra_run` "stops at the first failure with `stopped at line N of M`, counting the script's own lines"), :259; AR\doc-screenshots.md:13 (`!! failed at line N`).
- HY\tools\test_hydra_ui_mcp.py:773, :949, :956 hold the helper's "stopped at line N of M" shape (tests that name the string).
- Other "numbering" in scope, a different question: CL\tools\session_cards\number_rulings.py:13 and `numbering.json` (D-number numbering of rulings; its own file is the owner, per its docstring).
Table: ADR 0030:50 and hydra_ui_mcp.py say live mode has exactly one numbering (the helper's); script mode has the interpreter's (C++). So two numberers exist by design, one per mode; ui-testing.md:97 and :229 describe both forms (`failed at line N` vs `stopped at line N of M`).
Who acts: Claude reading tool replies; the MEM line addresses sessions. Edges: main vs agent: the helper is main-session only (agents run neither). Card writers: n/a.
Kinds: K8 candidate: the memory line says "One owner per numbering" and names none; the ADR names the owner per mode. K6 candidate: two code places number script lines (uitest_commands.cpp for script mode, hydra_ui_mcp.py for live mode), which the ADR states is the design.
Splits: none in value. Wording differs between the modes ("failed at line N" with the interpreter, "stopped at line N of M" with the helper).
RULING (Confirmed): AR\rulings\code-and-engine.md:30 (Confirmed section runs :7-60) "you think giving claude conflicting numbers is a good idea? that is just going to confuse the model" (session 719dc0ea). The same words as a note line are at :71 (Unconfirmed).

## I8-071 The two checklist rules (MEM\fc-video-workflow.md:15)
Question: where do "report the headline the moment engine and evidence agree" and "never rewrite the video" live?
Homes:
- MEM\fc-video-workflow.md:15 "Two checklist rules came out of it: report the headline the moment engine and evidence agree, and never rewrite the video. Both are now README checklist items 0 and 5." (README = VT\README.md, named at :11).
- VT\README.md:41-43 item 0 "Report the headline the moment you have it: the results-screen score against the engine's optimal for the right fill rule." and :56-57 item 5 "Never re-encode or rewrite the video for any reason." Both exist; the owner claim holds. F31/V31 only looked in HY\docs and did not find it because the README is in the separate video-tools repo (VT, git head 4e95395).
- VT\README.md:38 says "Eleven things to check". I did not count the items.
Notes: the memory line paraphrases both rules (K1 by words); item 5's "re-encode" and the note's "rewrite" differ slightly; the note's "headline ... engine and evidence agree" and the README's "the moment you have it" differ in the trigger (agree vs have). Mild K3 candidate on item 0.
Kinds: K1; K3 candidate (trigger wording differs).
Splits: none. RULING: none.

---
## Side notes
- VT\README.md:41-46 and CL\skills\analyzing-sp-squeezes\SKILL.md:55 carry the same sentence (above-optimal = version problem or bug; price 1.0 runs with `--legacy-fills`): a copy outside the scope list (VT) of a skill line.
- HY\src\core\backend_value.h:26-31: `paid_by_sp_walk` (by offset) and a tick-form "after the SP end" answer the same edge; the comment says they agree. Not checked for a test that holds them equal.
- MEM\sp-end-is-one-point-leeway-is-delgars.md:17 names an open conflict in `rate_activation` (src/core/squeeze_rating.cpp); not opened.
- AR\test-rubric.md:201 "No such test exists today (question 6)" not verified in the tests.
- No hook gaps noticed.
