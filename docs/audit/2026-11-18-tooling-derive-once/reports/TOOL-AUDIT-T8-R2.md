# TOOL-AUDIT-T8-R2 (themes 14, 15, 17)
Paths: H = C:\Users\Patrick\Downloads\Hydra\hydra-test ; C = C:\Users\Patrick\.claude ; M = C\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory
None of my 20 ids appears in the T7 reports (none reopened by T7).

### L-F28-16 and L-F29-10 (same question)
**Verdict**: NOT A SPLIT
**Homes read**: H\docs\agents\agent-rules.md:9 "In this rule a build means any piece of work: writing code, editing hooks, or launching agents"; :20 "In this rule a build means compiling, not rule 3's piece of work." C\model-postures.md:3 "its build still waits" (rule-3 sense) and :11 "build its branch (hydra_build_pass...)" (compile sense).
One sentence: agent-rules.md itself glosses each rule's meaning of "build" in the rule's own text; the short forms (CLAUDE.md, model-postures) just do not repeat the gloss. Rule 3 answers "what needs the user's yes", rule 14 answers "who may compile".

### L-F29-01, L-F32-01, L-F29-18 (one split: when a branch is built)
**Verdict**: REAL (all three)
**Homes read**:
- C\model-postures.md:11 "When an executor returns, check its claims against the code, then build its branch (hydra_build_pass, one build at a time) and run its tests, before review."
- H\docs\agents\derive-once-review.md:198 "Build the branch and run the merge's tests yourself ... If the build or a test fails, send a fresh executor to the same branch".
- H\docs\agents\integrate.md:35 "You do not build the joined branch"; :41 "the main session builds the joined branch once"; :56 builds, then review of the wave branch.
- M\one-build-per-wave.md:9 "review each branch, then put the reviewed ones on one combined branch, build it once and run all their tests ... Never plan one build per branch."
**Kind**: doc-vs-doc (model-postures and derive-once-review: build each branch before review; integrate.md: join first, one build of the joined branch, then review). memory-vs-other (memory: review first, then one combined build, a failing branch comes back out).
**What changes**: with several branches in a wave, the main session either builds every branch before its review (several builds) or builds once on the joined branch (one build), and in the memory's version builds only after the reviews.
Notes: L-F29-18's wording "integrator builds" is wrong; integrate.md:35 says the integrator does not build. Its "roles" half is not a split: model-postures.md:5 lists five agent types and integrate.md is only a brief; nothing contradicts that. What is left is the build timing. Rulings agents-and-workflow.md Confirmed 2026-11-07 name only "three builds at once" and "one build per session", not a schedule (agrees with the row).

### L-F13-09
**Verdict**: REAL
**Homes read**: H\docs\agents\integrate.md:27 "Hold the last task's merge uncommitted until the precheck in step 4 is clean"; H\docs\agents\brief-preamble.md:71 "Commit before your 100th tool call, and after each finished step." integrate.md:3 says the preamble's rules "apply to you in full"; preamble:3 says if brief and page disagree, stop and report.
**Kind**: doc-vs-doc
**What changes**: an integrator following the preamble commits the last merge once it is resolved; following integrate.md it leaves it uncommitted through the precheck (and a hook stop or call cap in that window leaves nothing committed).

### L-F06-13
**Verdict**: NOT A SPLIT
**Homes read**: C\agents\scout.md:4-5 "model: sonnet", "tools: ... Edit, Write ..."; scout.md:9 "Change no files except your status line and files in your own scratch folder."; agent-rules.md rule 2 "Read-only scouts run on Sonnet or Haiku."
One sentence: Sonnet is inside "Sonnet or Haiku", and the scout prose itself allows writes to the status line and scratch folder, which need Edit/Write; the tool list is not claimed read-only anywhere.

### L-F07-01
**Verdict**: NOT A SPLIT
**Homes read**: C\agents\scout.md:4 "model: sonnet"; CLAUDE.md rule 2 / model-postures.md:5 "Sonnet or Haiku"; H\docs\agents\brief-preamble.md:67-69 trailer line plus "Use the model line your brief gives if it differs." (scouts do not commit).
One sentence: the pin is one choice inside the allowed set, and the Opus trailer text lets the brief override it. Whether to pin Haiku is a preference, not two files disagreeing.

### L-F07-09
**Verdict**: NOT A SPLIT
**Homes read**: brief-preamble.md:66-69 asks for "Agent: <your agent id>"; C\gates\ref_judge.py:40 TRAILER_KEYS, :242-252 missing_trailers checks only that each value holds an alphanumeric; ref_judge.py:339 hint text "<your agent id, or orchestrator>".
One sentence: the preamble says what to write, the hook checks only that a value is present; they do not give different answers to one question.
Hook gap (record only): ref_judge.py:242-252 does not check the Agent value is the committer's own id.

### L-F13-20
**Verdict**: NOT A SPLIT
**Homes read**: ref_judge.py:242-252 (no Co-Authored-By check; grep for "Co-Authored" in C\gates\*.py finds none); brief-preamble.md:67 "Co-Authored-By: Claude Opus 5.5" plus :69 "Use the model line your brief gives if it differs."
One sentence: the "session attribution reminder" side is a harness message, not text in a file, and the preamble already lets a brief change the model line.

### L-F06-15 and L-F24-12 (one split)
**Verdict**: REAL
**Homes read**:
- H\docs\agents\brief-preamble.md:3 "It names your task id, the session id, your owned files, your worktree or checkout, your scratch folder and the tests you may run." (no status rule, no caps)
- C\model-postures.md:5 "the task id, the session id, the files it owns, the tests it may run and the status rule." (no worktree, no scratch folder); :11 "A brief names no status file and no id; it carries only the preamble's status rule."
- H\docs\agents\audit-method.md:120 "the agent's task id, the session id, its files or batch, its caps, and the status rule;"
- M\every-brief-carries-status-rule.md: every prompt tells the agent to write status lines "with the helper status_append.ps1".
**Kind**: doc-vs-doc; memory-vs-other (memory has the brief name the helper, model-postures.md:11 says the brief names no status file; preamble:15 and progress.py:88 already give the helper and the id, so that practical gap is small).
**What changes**: a main session briefing from model-postures leaves out the worktree and scratch folder that the preamble says a brief names; one briefing from audit-method adds caps and leaves out tests, worktree and scratch.
Note: the memory note's description says "the dispatch gate now denies briefs that lack it", but its own body says "No gate checks a brief for this sentence"; a grep of C\gates for status wording in the dispatch hooks finds no such check.

### L-F22-24
**Verdict**: GONE
**Homes read**: H\docs\agents\writer-brief.md does not exist (git ls-files finds none; grep -rln "writer-brief" in H\docs and CLAUDE.md finds none). progress.py:57-58, 88 still gives the id and helper line at the first call. The remaining sides, model-postures.md:11 and the memory note, point the same way; what is left of the difference is whether the brief names the helper path, already counted in L-F06-15 above.

### L-F28-21
**Verdict**: REAL
**Homes read**: H\CLAUDE.md:9 "A session or agent that learns a Clone Hero fact updates its page in the same commit as the work that found it"; H\docs\agents\brief-preamble.md:105 "If your brief owns that page, update it ... If not, put the fact in your report"; H\docs\clone-hero\README.md:80 "An agent whose brief does not own that page puts the fact in its report, and the main session adds it."
**Kind**: doc-vs-doc (preamble and README agree with each other; only the CLAUDE.md short line omits the exception).
**What changes**: an executor whose brief does not own the page edits it (CLAUDE.md) or reports the fact (preamble, README).

### L-F02-12
**Verdict**: NOT A SPLIT
**Homes read**: rulings\review-and-hooks.md:22 (2026-10-10 answer; card H\docs\sessions\2026-10-10-b5e8d95a.md:8,35 calls it the "doc-review 20-line and 2,000-character rule"); C\gates\publish_gate.py:14 "SMALL_LINES, SMALL_CHARS = 20, 2000" and :31-33 apply it to text sent for doc review. H\tools\precheck\items.h:50 kLargeRangeLines = 400 with items.cpp:102-106 "This is a warning, not a gate."
One sentence: the 20-line ruling is applied by the doc publish gate (so the row's "no code applies it" is wrong), and the 400 is a different thing, a split-the-code-range warning.

### L-F04-07, L-F07-16, L-F21-06 (one split)
**Verdict**: REAL
**Homes read**: C\gates\tests\replay.py:41 "LINE_BUDGET = 3100  # the user's hard budget for hook code; only the user changes it". rulings\review-and-hooks.md:13 (under ## Confirmed) 'chose "Raise cap to 3,050"'. Lines 41 and 44 (under ## Unconfirmed) say 3,100 ("rase cap to 3100"; "LINE_BUDGET stays 3100"). Confirmed :11 (2026-10-31) "im not too concerned about the exact number".
**Kind**: ruling-vs-other (code constant 3100 against the only Confirmed number, 3,050); the code comment also says the user set it, and 3100 is backed only by Unconfirmed lines.
**What changes**: hook code between 3,050 and 3,100 lines passes the test but is over the Confirmed budget.

### L-F29-07
**Verdict**: NOT A SPLIT
**Homes read**: brief-preamble.md:119 "Comments may say why a rule exists, or what it deliberately leaves out"; rulings\code-and-engine.md:22 (Confirmed) "comments exist to explain the current code, not give a history lesson".
One sentence: one allows a reason for a rule, the other bans a history of how the code came to be; a "why" is not a history lesson.

### L-F31-06
**Verdict**: NOT A SPLIT
**Homes read**: rulings\ui-and-display.md:56 (Confirmed, 2026-10-17) "the lane on the screen does not match the notes that are in the chart..."; :57 (Confirmed, same date and session 65b16299) "clone hero only draws the blue. this will be an intentional deviation from the game. it looks better with both."; H\src\parse\song.cpp:192-196 "With it on, a lane lights every pad its shuffled notes land on (every_lane_pad). That is a deliberate deviation ... (D254)"; M\drum-lanes-d112-2026-10.md:13.
One sentence: line 56 is the user's complaint and line 57, a second Confirmed line, is the user's own answer choosing both pads lit; the code and memory follow line 57. (The file does not show which of the two same-day lines came first; the memory note says the complaint came first.)

Ids not reached: none.
