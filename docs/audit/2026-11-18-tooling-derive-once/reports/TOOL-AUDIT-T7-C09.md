# TOOL-AUDIT-T7-C09 (closure checker; batch item 8: S05 / L05, F13-02 .. F15-13)

Paths: HY = C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout, read as it is now), GATES = C:\Users\Patrick\.claude\gates, MEM = C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory, BP = HY\docs\agents\brief-preamble.md, UT = HY\docs\agents\ui-testing.md, TM = HY\docs\agents\this-machine.md, CH = HY\docs\agents\clone-hero-mcp.md.
Counts: PASS 30, REOPEN 2 (L-F14-01, L-F14-06), GONE 0. Not reached: none.

### L-F13-02
**Verdict:** PASS (A)
**Homes read:** GATES\status_append.ps1:5 ("The call shape: the brief preamble's "Status lines" section"); BP:19 ("The status-call check in ...progress.py owns the exact shape"); GATES\progress.py:22-27 (STATUS_PREFIX, _STATUS_CALL); C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md:100 (second full call form, no quote rule); GATES\settings.fragment.json:4 and live settings.json:10 (same allow text as STATUS_PREFIX); GATES\tests\test_docs_numbers.py:189 and test_progress.py:709,762 (tests hold BP and the call to STATUS_PREFIX).
**Why:** Every split is a comment, doc or brief against progress.py. The two code-side copies (settings allow rule and STATUS_PREFIX) read identically today, so they are not a split.

### L-F13-10
**Verdict:** PASS (B)
**Homes read:** MEM\every-brief-carries-status-rule.md (front matter says the dispatch gate denies briefs without the sentence; body says "No gate checks a brief for this sentence today"); GATES\review_dispatch.py (grep for "status" and "yet": nothing); grep of all GATES\*.py for status_append / "Status lines": only progress.py and stall.py, neither a brief check.
**Why:** The only split is a memory note against itself and the code.

### L-F13-12
**Verdict:** PASS (P)
**Homes read:** TM:30-43 (install table, vswhere -all fallback, GENERATOR_INSTANCE advice); HY\tools\find_cmake.ps1:6-25 (does the vswhere -all fallback); HY\build-cpp\CMakeCache.txt:771-773 (Visual Studio 18 2026, instance ...\18\BuildTools).
**Why:** All homes agree today. I did not run vswhere (no run allowed), so the table's version numbers are unchecked against the machine, but no file says otherwise.

### L-F13-13
**Verdict:** PASS (P)
**Homes read:** HY\tools\build_slot.ps1:1-3,75,195,199; HY\CLAUDE.md rule 14; HY\docs\agents\agent-rules.md:20; BP:57; GATES\percall.py:19-20 (BUILD_PASS_TOOL, says build_slot.ps1 owns what a pass allows); HY\tools\test_build_pass.ps1:117 ("all 3 build slots are taken").
**Why:** Three at once, main session only, one per session: every home agrees and build_slot.ps1 is the owner.

### L-F13-14
**Verdict:** PASS (P), with a note
**Homes read:** GATES\bash_windows_rule.py:16 (MAX_CHARS = 1000), :172-174; BP:33; GATES\README.md (limit text); GATES\tests\test_docs_numbers.py:192-196 (holds BP's "1,000" to MAX_CHARS).
**Why:** The number agrees everywhere and a test holds it. The commit-text exemption wording in BP:33 does differ from the code, but that is its own row (L-F15-12, below).

### L-F13-15
**Verdict:** PASS (A)
**Homes read:** CH:29 ("typed exactly `allow key run`"); GATES\review_rules.py:52-55,86 (KEY_ALLOW_TEXT, regex allows any case and a trailing . or !, fullmatch on stripped text); GATES\percall.py:15-18,58-80 (_key_call, ch_call forwarding); GATES\README.md:13,37.
**Why:** The split is CH:29 (and README:13's fuller list of ch_call forms) against review_rules.py and percall.py, which do the matching. CH:29 and README:13 differ from each other only by README naming the ch_call forms, and percall._key_call is the code that settles it. The memory note's "ask first" agrees with CH:29 ("ask the user in chat first"), so there is no memory-note split.

### L-F13-16
**Verdict:** PASS (P)
**Homes read:** GATES\ruling_rules.py:100-101 (APPROVE, DECLINE, QUESTION); HY\docs\agents\rulings\README.md:36-44 (question text, both labels, "up to four lines"); HY\CLAUDE.md:41; HY\docs\agents\agent-rules.md:33; grep of ruling_rules.py for four/4: no hit.
**Why:** Text and labels match ruling_rules.py. "Up to four lines" has no code owner but nothing disagrees with it.

### L-F13-18
**Verdict:** PASS (P)
**Homes read:** GATES\ruling_rules.py:107-108 (CHECK_COMMAND); GATES\settings.fragment.json:6 and live settings.json:12 (same allow rule); GATES\README.md:61; GATES\tests\test_docs_numbers.py:137-140; HY\docs\agents\rulings\README.md:38.
**Why:** The fragment and live settings carry the same rule and a test holds it to CHECK_COMMAND; the rulings README gives a pointer without the interpreter, which is not a disagreement.

### L-F13-19
**Verdict:** PASS (P)
**Homes read:** HY\tools\check_handoff.py:7 and :32; HY\docs\agents\handoff.md:39-55; HY\.claude\commands\handoff.md:12; BP:97; HY\tools\precheck\main.cpp:4-12; HY\docs\agents\derive-once-review.md:24,99,124; HY\docs\agents\integrate.md:37,46; GATES\README.md:145.
**Why:** The handoff command is the same string in all four homes, the status words and MANUAL_KINDS match, the precheck forms match main.cpp's usage, and the hook-test command matches between README:145 and derive-once-review.md:124.

### L-F14-01
**Verdict:** REOPEN (should be A plus B, or a question; the A row misses a memory-note side)
**Homes read:** HY\docs\agents\agent-rules.md:14 ("about 115 charts"); UT:46,48 ("98 charts"); HY\tests\test_engine_sweeps.cpp:656 (CHECK(charts == 98)); HY\tests\ui\uitest_batch_reports.cpp:389 ("Analyze 98 charts"); find testdata/input -name song.ini gives 98; MEM\questions-describe-the-real-rule.md:24 ("Strip the `[N/97]` prefix").
**Why:** The doc-against-code side holds (agent-rules.md says 115, code and UT say 98). But the ledger row also lists a memory note saying 97, which is still in the note now. A is only valid when no split side is a memory note; the memory-note half is category B. Also HY\docs\adr\0010 and HY\docs\audit\note-shuffle\step0\FINDINGS.md still say 115 (doc against code, same A fix). The sorting file's sides column left the memory note out.

### L-F14-04
**Verdict:** PASS (A)
**Homes read:** TM:62-63 (cdb with `-y <build-cpp\Release>`); HY\tools\ch_probe\experiments\stall_harness.py:118-121 (CDB_COMMAND = "~*kc 40; qd") and :191-195 (launch_cdb passes -pv -p, no -y).
**Why:** A doc line against the script that runs it. The harness's comment saying it copies the doc is a code comment, still code side.

### L-F14-05
**Verdict:** PASS (A)
**Homes read:** UT:14-20 (bare `.\build_cpp.ps1 -Target hydra_uitest`, with a pointer to development.md); HY\build_cpp.ps1:13-14 ("a build needs a build pass, in HYDRA_BUILD_PASS, and stops before configure without one"); HY\tools\build_slot.ps1:207-208 (prints the build line with HYDRA_BUILD_PASS set).
**Why:** The doc example omits the pass the build script requires. The finder's unverified quote is now confirmed by build_cpp.ps1:13-14.

### L-F14-06
**Verdict:** REOPEN (a question, not P)
**Homes read:** UT:354 ("The live window takes no input from the real mouse (the user, 2026-10-29: "my mouse doesnt work in it")"); HY\docs\agents\rulings\ui-and-display.md:43 ("the window is jsut in the library list and my mouse doesnt work in it"); searched src\ui, src\app, tests\ui\uitest_live.cpp and tools\hydra_ui_mcp.py for real-mouse suppression (mouse, NoMouse, ignore): no code states it.
**Why:** The ruling line is a complaint. The doc turns it into a design fact ("takes no input from the real mouse") that no code I searched backs. Whether it is a bug or intended is a user decision, so "no disagreement" does not hold.

### L-F14-07
**Verdict:** PASS (A)
**Homes read:** CH:31 ("No tool opens the Online menu, Settings, Practice or the control mapper."); HY\tools\ch_mcp\menu_tools.py:191-234 (online_refusal, item_problem kit-only, load_steps); CH:35.
**Why:** The doc states an outcome and the code enforces a narrower mechanism; doc against code only.

### L-F14-09
**Verdict:** PASS (P)
**Homes read:** GATES\review_rules.py:47-49 (REALAPP_FILE = "realapp.jsonl"; comment says hydra_ui_mcp.py owns it); HY\tools\hydra_ui_mcp.py:80-81 (same literal).
**Why:** Same value in two code homes; the fix points one at the other.

### L-F14-13
**Verdict:** PASS (P)
**Homes read:** HY\docs\agents\doc-screenshots.md:5-7; HY\tools\doc_shots.ps1:2; HY\docs\development.md:980 (all `pwsh -NoProfile -File tools\doc_shots.ps1`); HY\src\ui\library_dialogs.cpp:133,136-137 ("defaults", settings_button_label); HY\tests\test_batch_text.cpp:80-82; HY\tests\single_owner\rules\ui.json:598; HY\tools\doc_shots\shots.txt:20.
**Why:** Command and label agree in every home; tests hold the label to the owner.

### L-F14-14
**Verdict:** PASS (A)
**Homes read:** CH:78 (names only the version-string check; says the launch agent checks no build); HY\tools\ch_mcp\build_facts.py:25-65 (version file); HY\tools\ch_mcp\build_stamp.py:210 (check of built agent JS); callers frida_tools.py:402 and method_watch.py:437.
**Why:** Doc lists one gate where the code has two (the second only checks the built agent JS, not the game build).

### L-F14-15
**Verdict:** PASS (P)
**Homes read:** UT:112-135 (label cheat-sheet); HY\src\ui and HY\tests\ui source. I extracted all 171 backticked labels in UT:117-133 with a script and looked for each label fragment as text in src and tests\ui.
**Why:** Every label fragment was found, except example numbers and file paths. No disagreement found. Substring presence is a weak check, but no file disagrees.

### L-F14-16
**Verdict:** PASS (P)
**Homes read:** UT:31-40; HY\tools\hydra_ui_mcp.py (flags); HY\tests\ui\uitest_commands.cpp:111,316 (split_bar); HY\tools\ch_mcp\kit_tools.py:737 (FRAME_PROBE_SECONDS = 5.0); CH:51; HY\.mcp.json (hydra with `-I`, ch without).
**Why:** Spot checks of the listed rows found copies only, no disagreement. I did not open all 25 source items; this is the weakest PASS in the batch.

### L-F14-17
**Verdict:** PASS (P)
**Homes read:** UT:44,48 (dated timing and canned alice/111); TM:7 and GATES\README.md:114 (same python.exe path); HY\.claude\worktrees\plan-agent-supervisor\docs\agents\issue-tracker.md (git-ignored, identical to docs\agents\issue-tracker.md now per diff).
**Why:** Dated numbers and copies only; the stale copy the finder saw is now identical and ignored.

### L-F15-02
**Verdict:** PASS (P)
**Homes read:** MEM\git-traps-in-shared-checkout.md:20; GATES\ref_judge.py:448 (MERGE_HEAD used for a different question).
**Why:** Single home, no split.

### L-F15-03
**Verdict:** PASS (P)
**Homes read:** HY\docs\agents\integrate.md:13-15,27; GATES\README.md:160 (`--ff-only` by the main session); BP:111; HY\docs\agents\owner-map.md:35.
**Why:** `--no-ff --no-commit` (integrator) and `--ff-only` (main session) are for different actors; no home contradicts another.

### L-F15-04
**Verdict:** PASS (P)
**Homes read:** MEM\stitched-chart-auto-fills.md (points to FL-27, FL-28, FL-30); HY\docs\clone-hero\04-fills.md:276,286,306-310; HY\docs\clone-hero\02-note-list.md:153.
**Why:** The note's pointer matches the page; the "merge rules no page holds" list is a gap the note names, not a disagreement.

### L-F15-05
**Verdict:** PASS (P)
**Homes read:** MEM\engine-tie-merge-prune.md:22; MEM\fc-video-workflow.md:19; MEM\MEMORY.md:34; C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:58; C:\Users\Patrick\Downloads\Hydra\video-tools\README.md:53; C:\Users\Patrick\Downloads\Hydra\squeeze-tools\README.md:11.
**Why:** All say never widen the band, same reason. (The ledger's squeeze-tools path is wrong; the folder is at C:\Users\Patrick\Downloads\Hydra\squeeze-tools.)

### L-F15-06
**Verdict:** PASS (P)
**Homes read:** MEM\hooks-never-guess.md:9-16; HY\docs\superpowers\plans\2026-10-25-hooks-from-scratch.md:108.
**Why:** The rule has no second home that says anything different.

### L-F15-07
**Verdict:** PASS (P)
**Homes read:** MEM\mimalloc-link-order.md:3,13; HY\src\app\allocator.h:28; HY\tests\test_library_jobs.cpp:739,770; HY\tests\ui\uitest_engine.cpp:157; HY\docs\development.md:231-236.
**Why:** A manual check beside automatic tests: two routes to the same fact, no value in disagreement.

### L-F15-08
**Verdict:** PASS (P)
**Homes read:** C:\Users\Patrick\Downloads\Hydra\video-tools\frames.py:74 (NEVER_NVDEC = {"h264"}), :215 (CROP_ALIGN = 16); video-tools\README.md:88,96,508-510; MEM\video-decode-ceilings.md:11-19; analyzing-sp-squeezes\SKILL.md:59.
**Why:** h264 never to the card, half speed with --speed 0.5, all agree.

### L-F15-09
**Verdict:** PASS (B)
**Homes read:** MEM\windows-installer.md:19 ("`gh release create --target` needs the full SHA; a short one gives HTTP 422."); HY\docs\development.md:1011 (publish.ps1 with `'--target','main'`); MEM\windows-installer.md:16 and HY\installer\build_installer.ps1:78-91 (staging; agrees).
**Why:** The split is a memory note against a doc command.

### L-F15-10
**Verdict:** PASS (B)
**Homes read:** MEM\no-full-library-runs.md:15 ("No hook enforces it yet."); GATES\percall.py:14,48 (LIBRARY_FLAG enforced).
**Why:** Memory note against the hook.

### L-F15-11
**Verdict:** PASS (C)
**Homes read:** HY\src\core\model.h:121 (`constexpr double kSqueezeWindowMs = 500.0;`); HY\src\search\graph.h:216 (comment only); skills\analyzing-sp-squeezes\SKILL.md:10 and C:\Users\Patrick\Downloads\Hydra\squeeze-tools\README.md:9 (both say src/search/graph.h); HY\docs\clone-hero\05-star-power.md:417,419 (say model.h line 111).
**Why:** A moved/wrong file and line; the current place is model.h:121 (within_squeeze_window at :134, kHitWindowMs near :105-106, so the page's other numbers have drifted too).

### L-F15-12
**Verdict:** PASS (A)
**Homes read:** BP:33 ("A `git commit`'s own text does not count toward that limit"); GATES\bash_windows_rule.py:25,134-148,172-174; GATES\tests\test_bash_windows_rule.py:106-116 (`git --no-pager commit` and `"git" commit` still count).
**Why:** Doc wider than the code; the code still charges those two spellings.

### L-F15-13
**Verdict:** PASS (P)
**Homes read:** GATES\bash_windows_rule.py:211-225 (tags), :235; GATES\common.py:47 (`reason[:300]`); GATES\README.md:81; GATES\percall.py:47,146,167,181 (SHELL_TOOLS use); GATES\progress.py:20.
**Why:** Tags, the 300 cut and the tool sets are literal copies, not disagreements.

## Ids not reached
None.
