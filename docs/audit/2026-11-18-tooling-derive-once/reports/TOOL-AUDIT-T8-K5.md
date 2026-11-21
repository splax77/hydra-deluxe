# TOOL-AUDIT-T8-K5 closure check (17 ids)

H = C:\Users\Patrick\Downloads\Hydra\hydra-test (main). G = C:\Users\Patrick\.claude\gates. M = C:\Users\Patrick\.claude\projects\...\memory.
Line numbers in the re-check reports are off for some long lines in agent-rules.md; I quote text and name the line I found it on today.

Summary: 14 PASS, 3 REOPEN (L-F27-27, L-F29-01, L-F29-18), 0 GONE.

### L-F26-15
**Verdict**: PASS (note is wrong; code wins)
**Homes read**: M\use-source-facts-not-proxies.md:13 "The last Expert drum note is only a backup when the metadata is missing". H\src\app\song_length.h:1-12 and :45-58 (three candidates: stated length, end event, last note's end over every instrument; kSongEndPadMs = 1000 at :37). H\CONTEXT.md:992-1010 "Song length" says the same as the code. H\tools\ch_mcp\bot_run.py:387-398 calls hydra_end.chart_song_length_ms.
**Why**: the code decides the length and CONTEXT.md agrees with it. Only the note is stale, and the fix is to edit its stale sentence (the note's other lesson, "ask whether a source already states it", stays). Both kinds hold.

### L-F27-23
**Verdict**: PASS (not a split)
**Homes read**: H\tools\ch_mcp\dump_facts.py:381 DEFAULT_BUILD = "6142" (which build a page fact belongs to when no heading names one); H\tools\ch_mcp\launch_patch.json:3 "build": "1.1.0.6142" (the build the launch byte patch fits); H\tools\ch_mcp\build_facts.py:69 current_build (reads the installed game's version file, hard-codes nothing); H\tests\test_docs_match_code.cpp:735 kBuild11 = "6142" (the test's name for the 1.1 build; the cited line 719 has moved). Searched: tools\ch_mcp, tests.
**Why**: three different questions, none of them answers "what build is installed" except build_facts.py, which reads the file. I agree.

### L-F27-27
**Verdict**: REOPEN
**Homes read**: H\tools\ch_probe\SPEC.md:86,106 ("0x0313ECB0 = 0.0110924370, the linear coefficient"; "never saved ... re-read live"); H\tools\ch_probe\constants.py:173-188 (normal "c1": 0x3140738, c2 0x3140658, c3 0x31406F8, c4 0x3140700; precision c1 0x31406B0, c2 0x3140620, c3 0x3140708, c0 0x3063FC0); H\docs\clone-hero\code\6142\hit_window_formula_20DDDA0.asm:12-30 and :36-52; H\docs\clone-hero\code\6142\hit_window_formula_20DDDA0.md:9-12; H\tools\ch_probe\engine.py:324-327 (reads these addresses live). Searched docs\clone-hero and tools\ch_probe for ECB0: only SPEC.md:106 holds it.
**Why (your question)**: SPEC.md's 0x0313ECB0 and constants.py's c1 at 0x3140738 are two different addresses, both labelled as the linear coefficient of the normal branch. They are not the same constant. In the 6142 disassembly the fall-through (bit clear, normal) branch multiplies by [0x31406b0] = 0.0110924369747899, [0x3140620] = 2.628539e-05, [0x3140708] = 170 and subtracts [0x3063fc0] = 20. The bit-set (precision) branch uses [0x3140738] = -0.04, [0x3140658] = 0.0004, [0x3140700] = 13 and [0x31406f8] = 12.4805915. So constants.py has the normal and precision tables swapped against the 6142 disassembly: its "normal c1" 0x3140738 is the -0.04 precision term, and 0.0111 sits at 0x31406B0, which constants.py lists under precision. 0x0313ECB0 appears in no 6142 listing; it may be from another build or a misread.
So "The code wins" does not hold: the code side (constants.py) disagrees with the 6142 asm, so the code is the side that is wrong (and engine.py reads these addresses live, so a probe would read the wrong constants). This is a doc-vs-code row whose code is itself contradicted by a second code source. It should go to the user, or be fixed against the asm (the disassembly is the source), not closed by "code wins".

### L-F28-06
**Verdict**: PASS (fix without asking), with a caveat for the fix wording
**Homes read**: G\wake.py:6 and G\stall.py:11 name docs/superpowers/plans/2026-11-16-agent-supervisor.md (`git show 80b310a9:docs/superpowers/plans/2026-11-16-agent-supervisor.md`). H main has no such file (ls docs\superpowers\plans; git ls-tree main; no deleting commit). It exists on branch plan/agent-supervisor (head 80b310a9 "Supervisor plan: record the user's answers and approval") and in worktree H\.claude\worktrees\plan-agent-supervisor. check.py: none in G (searched gates\*.py for "agent-supervisor": only wake.py and stall.py).
**Why**: the thing is nameable (an unmerged branch). The fix is to say so in the pointer (or the plan merges, which fixes it). The caveat: if the plan is meant to land on main later, the pointer becomes right on its own, so the fix should not reword it blindly.

### L-F28-13
**Verdict**: PASS (doc-vs-doc)
**Homes read**: H\docs\agents\brief-preamble.md:123 "Lead with the plain version. One idea per sentence. Gloss any jargon in one line right after it." C\model-postures.md:13 "Write plain English: lead with the plain version, one idea per sentence, no bullet walls of file:line references." (no gloss rule).
**Why**: both are docs; the kind holds. A weak split (an omission), which "Merge, I pick" covers.

### L-F28-15
**Verdict**: PASS (doc-vs-doc)
**Homes read**: H\docs\agents\agent-rules.md:41 "`CONTEXT.md` decides what a project word means"; :43 "If a definition's text and the code's usage disagree, that is a question for the user ... the agent changes neither"; H\docs\agents\rulings\README.md:34 "If the reading and `CONTEXT.md` disagree, `CONTEXT.md` wins and the session asks the user."; H\docs\agents\brief-preamble.md:99 ("change neither").
**Why**: all docs. Line 41 and the README speak of a ruling's reading against CONTEXT.md; line 43 speaks of CONTEXT.md text against code usage. These are close to two different cases, so it is a weak split, but the kind is right and the pick belongs in the fix plan.

### L-F28-16
**Verdict**: PASS (not a split)
**Homes read**: H\docs\agents\agent-rules.md:9 (rule 3: "In this rule a build means any piece of work ... The main session's compiling and test runs inside approved work need no separate yes (rule 14)"); :20 (rule 14: "In this rule a build means compiling, not rule 3's piece of work"); C\model-postures.md:3 ("its build still waits") and :11 ("build its branch (`hydra_build_pass`...)").
**Why**: agent-rules.md glosses the word in each rule and says outright that compiling needs no yes. The short forms lose the gloss but do not contradict.

### L-F28-18
**Verdict**: PASS (not a split)
**Homes read**: H\docs\agents\agent-rules.md:21 (rule 15 "If none does, the new owner goes in core"); :25 "Under an owner map, 'if none does, the new owner goes in core' above waits for the user ..."; H\docs\agents\owner-map.md:37 "Nobody writes a new owner for it".
**Why**: line 25 states which side wins once an owner map is in play, and owner-map.md agrees.

### L-F28-21
**Verdict**: PASS (doc-vs-doc)
**Homes read**: H\CLAUDE.md:9 ("A session or agent that learns a Clone Hero fact updates its page in the same commit"); H\docs\agents\brief-preamble.md:105 ("If not, put the fact in your report"); H\docs\clone-hero\README.md:80 ("An agent whose brief does not own that page puts the fact in its report").
**Why**: three docs; the short line omits the exception. Kind holds.

### L-F29-01
**Verdict**: REOPEN (the memory-vs-other half)
**Homes read**: C\model-postures.md:11 (build each executor branch "before review"); H\docs\agents\derive-once-review.md:198 (build the branch yourself, then the precheck); H\docs\agents\integrate.md:35 ("You do not build the joined branch"), :41 (main session builds the joined branch once), :46-56; M\one-build-per-wave.md:9-14; H\docs\agents\rulings\agents-and-workflow.md:8 (only "three builds at once ... one build per session"); H\docs\agents\agent-rules.md:9, :20.
**Why**: the doc-vs-doc half is fine (model-postures and derive-once-review build per branch before review, integrate.md builds the joined branch once). The memory half is not "Note is wrong": the note's rule "before launching work that will need builds, say how many builds it needs and get a yes" and "never plan one build per branch" has no doc or hook. agent-rules.md:9 says the opposite for compiling: "compiling and test runs inside approved work need no separate yes". I grepped agent-rules, model-postures, gates README, integrate and the rulings for "how many builds", "one build per", "per wave": nothing. integrate.md:41 holds only part of it (one build of a joined branch), and not the review-before-build order or the announce-the-count step. The user's click "One build per wave" is in a session card only (8e6a2908 has no card file in docs\sessions; no Confirmed line). Pruning the note drops a rule, so it should be "Give each a home" or a question to the user.

### L-F29-03
**Verdict**: PASS (doc-vs-doc)
**Homes read**: G\README.md:17 "The hook tests on this page need no build, so agents still run them."; H\docs\agents\brief-preamble.md:47 "Never run all the hook tests." and "Run only the tests your brief names."
**Why**: both docs, answering the same question differently (the README allows, the preamble narrows). Kind holds.

### L-F29-07
**Verdict**: PASS (not a split)
**Homes read**: H\docs\agents\brief-preamble.md:119 ("Comments may say why a rule exists"); H\docs\agents\rulings\code-and-engine.md:22 (cited as Confirmed, "not give a history lesson"; I did not re-open that line, the report's quote is taken as read, so this part is not independently checked).
**Why**: a reason for a rule and a history of the code are different things. I agree.

### L-F29-08
**Verdict**: PASS (doc-vs-doc)
**Homes read**: as L-F28-13 (brief-preamble.md:123, model-postures.md:13).
**Why**: same two lines, same kind.

### L-F29-10
**Verdict**: PASS (not a split)
**Homes read**: as L-F28-16 (agent-rules.md:9, :20; model-postures.md:3, :11).
**Why**: same question as L-F28-16.

### L-F29-11
**Verdict**: PASS (not a split, mild)
**Homes read**: H\docs\superpowers\plans\2026-10-25-hooks-from-scratch.md:11 "The new system does five jobs and nothing else."; G\README.md:3 "The hooks do six jobs ... where that plan and the code differ, the code wins."
**Why**: the plan is the earlier design, the README the current description and says which wins. Note: the README's "the code wins" names the plan, a doc, which still says five; the plan is still in the repo, so a reader of it meets "five". That is a stale plan, not a live split.

### L-F29-13
**Verdict**: PASS (doc-vs-doc)
**Homes read**: H\docs\agents\agent-rules.md:41 and :43, as L-F28-15.
**Why**: same split as L-F28-15.

### L-F29-18
**Verdict**: REOPEN (the memory-vs-other half; the doc-vs-doc half stands)
**Homes read**: as L-F29-01, plus H\docs\agents\integrate.md:35 (the integrator does not build; the row's wording "integrator builds" is wrong).
**Why**: the memory note is the same one-build-per-wave.md as L-F29-01 and fails the same test: it holds a rule (announce the build count and get a yes) that no doc or hook has. The row's own text does not even mention the note; the memory kind came from the re-checker's group. The doc-vs-doc part is the build timing between model-postures/derive-once-review and integrate.md, which "Merge, I pick" covers.

## Ids not reached
None.
