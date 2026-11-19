# TOOL-AUDIT-T5-W16 (second-round verifier, ledger L11, 27 rows)

Everything below is by reading. Nothing was run. Short names as in the ledger: HY = hydra-test worktree tooling-audit-records, AR = HY\docs\agents, G = C:\Users\Patrick\.claude\gates, MEM = the memory folder, SC = C:\Users\Patrick\.claude\tools\session_cards, SKILL = C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md. Rulings: only `## Confirmed` lines counted. Many finder line numbers had drifted by a few lines; where that matters it is said.

### L-F31-08
**Verdict:** CONFIRMED. The call line is now seen too.
**Homes as found:** HY\src\parse\song.cpp:90 `lights_lane_pad`, :149 `game_lane_pads`, :205 `light_drum_lanes`, call at :2204 (`light_drum_lanes(std::move(song.drum_lanes), song.sequence, reading.noteshuffle)`). HY\docs\clone-hero\02-note-list.md:611 says lines 203, 88, 147; :631 says "called at line 2009". Every page number is stale (by 2, 2, 2 and 195 lines). MEM\drum-lanes-d112-2026-10.md:11 says "called once in the loader tail `finish_loaded_song`" (no number, still true by the comment at song.cpp:2198-2204).
**Splits:** None in value. The page's line numbers disagree with the code.
**Ruling:** none found.
**Open:** none.

### L-F31-13
**Verdict:** CONFIRMED (all homes agree on who runs builds and tests), with line drift.
**Homes as found:** AR\doc-screenshots.md:19 ("the main session runs it, never an agent (agent rule 14)"); AR\agent-rules.md:7 (rule 1, not :1) and :20 (rule 14, not :14), :27 ("nothing checks rule 14's line on tests"); HY\CLAUDE.md:76; AR\brief-preamble.md:47,57,59; AR\ui-testing.md:17,27,151,348,362; AR\integrate.md:35,41,46; AR\derive-once-review.md:99,116,118; MEM\merge-tests-touch-changed-files.md:11; C:\Users\Patrick\.claude\agents\executor.md:3, reviewer.md:21. The pass refusal text is G\percall.py:143 ("Agents never build, so hydra_build_pass is refused to every agent"), not :109-110 as the row says. card-writer.md has no "build" text.
**Splits:** None in value. One wording difference between pages: brief-preamble.md:47 and derive-once-review.md:116 let an agent run `hydra_precheck.exe` from main; ui-testing.md:364 says the `hydra` MCP server runs the build of the checkout its helper sits in. Different subjects (precheck vs the MCP), so not a split.
**Ruling:** AR\rulings\agents-and-workflow.md:9 (Confirmed) "any tests that require a build are done by the main session, not agents." It names the test half; it is the one side that exists.
**Open:** nothing checks the test half (agent-rules.md:27 says so).

### L-F31-14
**Verdict:** CONFIRMED.
**Homes as found:** AR\test-rubric.md (text about `hydra_replay.exe dump` then `schedule`, "Agents may not run `hydra_replay` (agent rule 14), so an auditor asks the main session"); SKILL "It gets the path from `hydra_replay dump` (the repo's build-cpp\Release\hydra_replay.exe)" and "reconstructs each activation's SP ends engine-exactly"; MEM\sp-squeeze-mechanics.md (tells a reader to "invoke" the skill); squeeze_scout.py:334-349 is a Python rebuild of phrases and ends outside src (not read in full).
**Splits:** One by reading. The test-rubric page says an agent asks the main session for `hydra_replay` output. The skill says the tool runs `hydra_replay dump` itself and never says who may invoke it. They differ when an agent loads the skill: it is told to run a tool that calls `hydra_replay.exe`, which rule 14 (AR\agent-rules.md:20) forbids to agents. Also a second SP-end reconstruction exists in squeeze_scout.py (Python) beside the engine's; whether it agrees is not settled by reading.
**Ruling:** none found.
**Open:** agreement of the Python reconstruction with the engine needs a run (a main-session run).

### L-F31-15
**Verdict:** CONFIRMED (draw place vs gap measure, not proven a split).
**Homes as found:** MEM\derive-display-from-engine-truth.md:23 ("every SP event tied to a squeeze is drawn at that chord. The search's bookkeeping (the nominal end, the node where it switches the end, the stored bank tick) is not where the squeeze happens"); SKILL ("A SqOut gap is measured against the pre-collection end, not the final deactivation node"); HY\src\core\model.h:688 `nominal_end()` ("the first step's end"), :629 `sqout_tick`; model.cpp:719,747,793.
**Splits:** One, unproven. The memory note says the nominal end is not where a squeeze happens (about drawing); the skill measures SqOut gaps from the pre-collection end (about the player's timing gap). They differ if "pre-collection end" and `nominal_end()` are the same tick: the note forbids it as a draw point, the skill uses it as a measure point. Whether the skill's value equals `nominal_end()` is not read.
**Ruling:** none found.
**Open:** whether the skill's "pre-collection end" is `nominal_end()`; needs reading the scout tool or a run.

### L-F31-16
**Verdict:** CONFIRMED-CHANGED. Most sub-claims hold; one is wrong.
**Homes as found:**
- Wrong sub-claim: AR\test-rubric.md:21-25 does NOT copy the 10/20/30 multiplier steps. It defines rule numbers and uses 85 ms and "two measures". The copy is AR\test-rubric-examples.md:192-203 (quotes the old test), :244 ("10, 20, 30 and the multipliers 1 to 4"), :288; owner HY\src\core\timing.cpp:12-17 `to_multiplier` (<10:1, <20:2, <30:3, else 4); HY\docs\clone-hero\06-scoring.md:55 (SC-05) states the rule; HY\tests\test_timing.cpp:73-90 holds it as a test with names x2_from=10, x3_from=20, x4_from=30.
- Held: planner.md:19 and reviewer.md:17 near-copy owner-map.md's "use each owner as it is, never combine owners..." rule; AR\owner-map.md:37 "Nobody writes a new owner for it".
- Held: AR\brief-preamble.md:111 says "`docs/agents/owner-map.md`, "The stops", is the one home of this rule" while spelling it out in the line.
- Held: MEM\derive-display-from-engine-truth.md:21 names `Chord::activation_note`, `kSpActivationBars`, `sp_ready_ms`, `app::song_length_ms` (exists: HY\src\app\song_length.h:61).
- Held: dmleaderboards note (MEM:47-50) and HY\src\net\dmbot_client.h:35-36 `kBaseSpeedPercent = 100`; both say only 100% scores are compared.
- Not re-opened: issue-tracker.md:20,25; handoff.md:45 (does name `py -I tools/check_handoff.py`, same command as brief-preamble.md:97).
- domain.md:11 says "proceed silently" for missing CONTEXT/ADR files; domain.md:49 says surface an ADR contradiction, not override silently. Different subjects.
**Splits:** None found. The only wording clash is "one home of this rule" vs the rule being spelled out at brief-preamble.md:111 (K8-light).
**Ruling:** none re-opened.
**Open:** the memory note with no owner (I8-038) is L-F31-26's.

### L-F31-27
**Verdict:** CONFIRMED.
**Homes as found:** HY\src\core\model.h:121 `constexpr double kSqueezeWindowMs = 500.0;` and :130-134 (strictly inside, `fabs(...) < kSqueezeWindowMs`). SKILL says "(`kSqueezeWindowMs`, src/search/graph.h)" for the 500 ms value: wrong file. MEM\sp-squeeze-mechanics.md:3 and the body say 500 ms and name src/core/model.h (right). MEM\one-build-per-wave.md front matter reads `modified: 2026-11-19T01:56:25.740Z`, which is after today's date (2026-11-18) in UTC; may be a local evening in a western time zone, so it is a note, not a fault.
**Splits:** None. The 500 value agrees.
**Ruling:** none found.
**Open:** none.

### L-F32-02
**Verdict:** CONFIRMED.
**Homes as found:** HY\tools\build_slot.ps1:75 `$BuildSlotCount = 3`, :80 `$BuildPassUnclaimedSeconds = 120`, :21 env var `HYDRA_BUILD_PASS`. Prose copies of "three": HY\CLAUDE.md:76; AR\agent-rules.md:20; AR\brief-preamble.md:57; HY\docs\development.md:114 ("at most three builds"), :143 ("The limit applies only when `CLAUDECODE` is 1"). That scope is not stated in CLAUDE.md or agent-rules.md. "120" appears as prose only in AR\rulings\agents-and-workflow.md:35 (under `## Unconfirmed`, starts :32) ("2 minutes, 120 seconds"); no doc page states it. Comments naming build_slot.ps1 as owner: HY\tools\hydra_ui_mcp.py:20-24, HY\tools\new_worktree.ps1:7-8. G\percall.py refusal text is at :143.
**Splits:** None beyond L-F32-01.
**Ruling:** AR\rulings\agents-and-workflow.md:8 (Confirmed) "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." It states "three" and "one per session"; it does not name the CLAUDECODE scope or 120 s.
**Open:** none.

### L-F32-04
**Verdict:** CONFIRMED.
**Homes as found:** MEM\min-path-delta-proof.md "MultSqueeze::gain() (src/core/model.cpp) is display-only; its one consumer is multsqueeze_views in src/app/path_view.cpp." HY\src\core\model.cpp:455 region (declared model.h:488 `int gain() const`; model.cpp:10 includes scoring.h for `multsqueeze_gain`). Only caller found: HY\src\app\path_view.cpp:141 and :144 (inside `multsqueeze_views`, :136); path_view.h:42 field `gain` with comment "MultSqueeze::gain()"; path_view.cpp:161 sums `s.gain` from the view.
**Splits:** None.
**Ruling:** none found.
**Open:** searched src and tools only (grep of MultSqueeze/multsqueeze_gain); tests not searched.

### L-F32-05
**Verdict:** CONFIRMED.
**Homes as found:** SKILL (kick 95 counts as a completion note in 2x Bass mode; "Count a note at the phrase-off tick: excluded"); HY\src\parse\song.cpp:431 `close_sp_phrase` (comment :421-430: phrase covers `start <= t < end`, last chord inside gets the phrase); :888-890 `is_handled_note` (`note == kick2x` returns true); :881 comment. Scout side: squeeze_scout.py:334-349 (playable = pitches 96-100, plus 95 when 2x; phrase as half-open [start, off), comment "Hydra's parser closes the phrase in its 'Pre' phase"). Same rule in two places (Python scout and song.cpp); the skill gives no pointer to song.cpp.
**Splits:** None by reading (both are half-open at the note-off tick, both include kick 95 only in 2x mode).
**Ruling:** none found.
**Open:** whether the scout's playable set equals song.cpp's handled pitches for every format (.chart too): not read.

### L-F32-06
**Verdict:** CONFIRMED-CHANGED. A real split the row did not list.
**Homes as found:** MEM\names-from-data-not-recall.md:15 "Every proper name in a deliverable must trace to data in hand (files, DB, web lookup)"; MEM:11 says the real name "found by web search"; SKILL "Every proper name in a report traces to a file, the database or a cited lookup, or prints as `unknown`"; AR\derive-once-review.md:76 "Every name comes from the repo, not memory." Grep for "proper name" in HY\CLAUDE.md, AR\brief-preamble.md, AR\public-doc-style.md and the five agent files found nothing. I8-109 (Feeble Words a full drum tacet) lives only in the memory note (last paragraph).
**Splits:** One. Memory note and skill accept a web or cited lookup as a source; the reviewer page accepts only "the repo". They differ for a name found only by web search (exactly how Feeble Words was found): fine in the first two, not in the reviewer rule.
**Ruling:** none found.
**Open:** none.

### L-F32-07
**Verdict:** CONFIRMED.
**Homes as found:** MEM\new-screens-start-from-a-clickable-mockup.md: description says "before any plan or build"; body says the user "approves it before a plan is written"; last body paragraph says "A mockup shows the one recommended design, with no option switches ... never as toggles on the page. (The second mockup's four toggles were accepted; ten was too many.)". MEM\MEMORY.md:8 says "one design, no option toggles". MEM\approved-work-is-not-reasked.md:15 and MEM\one-time-scale-per-screen.md:15 point to it. Grep of HY\CLAUDE.md, AR\*.md and the agent files for "mockup" found nothing.
**Splits:** None between homes. Wording gap inside the note: "no option switches" vs "four toggles were accepted".
**Ruling:** AR\rulings\ui-and-display.md:24 (Confirmed section is :7-101) "this is way too many options. im not going to bother checking different combinations of 50 different boxes" (2026-10-31). It supports "ten was too many"; it does not name the four-accepted side. :9 "yes i approved the mockup and asked for option B. so why are you offering what we have today as an option?".
**Open:** none.

### L-F32-10
**Verdict:** CONFIRMED (both parts).
**Homes as found:** MEM\relay-status-in-plain-english.md cites 'The user chose this on 2026-11-16 ("Claim, plus d...' and the lines on skipping test_ fixture lines. G\stall.py:26 `STATUS_MAX_LINES = 12`; :30-33 `STATUS_HEADER` ("Relay the claim to the user briefly, in plain English. When the evidence disagrees, add one sentence that says so"); stall.py has no "test_" skip (grep of `test_` in stall.py: nothing). C:\Users\Patrick\.claude\model-postures.md:11 repeats "Relay the claim to the user briefly, in plain English, and add a second sentence only when the evidence disagrees." Grep of HY\docs for "Claim, plus" finds only audit ledger files, no ruling or card.
**Splits:** None between hook and posture (same words). Open record gap: the "Claim, plus disagreement" pick has no Confirmed line.
**Ruling:** AR\rulings\agents-and-workflow.md:26 (Confirmed) "the status updates should be brief but relayed in plain english" (2026-09-05). It names the plain-English side only, not the "second sentence on disagreement" side.
**Open:** none.

### L-F32-11
**Verdict:** CONFIRMED.
**Homes as found:** MEM\questions-describe-the-real-rule.md:13 holds "same functionality" and :20 the "narrowing is its own question" bullet; AR\audit-method.md:36 "Narrowing it is its own question to the user" (scope of an audit); :146 heading "A question narrower than the code (D51 and D80)"; AR\agent-rules.md rule 12 (line 18) is a different rule (visible change). "Same functionality" is in no AR page, HY\CLAUDE.md or agent file; only in the memory note and rulings.
**Splits:** None (different subjects: a question vs an audit's scope).
**Ruling:** AR\rulings\code-and-engine.md:47 (Confirmed, section :7-60) "hydra is expected to keep the same functionality it has today. your plan needs to ensure that." Names the keep-functionality side only.
**Open:** none.

### L-F32-12
**Verdict:** CONFIRMED.
**Homes as found:** MEM\search-web-before-finalizing-plans.md:11,13,15 (search GitHub / Stack Overflow before finalizing; "Do not leave a 'left alone' list of related defects; either fold them into the plan or ask."); AR\audit-method.md:36 "Leave nothing in a "pre-existing, left alone" pile." (D119); AR\rulings\docs-and-writing.md:19 sits under `## Unconfirmed` (starts :17). Grep of planner.md, premise-map.md and owner-map.md for "prior art", "web search", "left alone" found nothing.
**Splits:** None here (the planner "Out of scope" list clash is L-F31-25).
**Ruling:** none under Confirmed.
**Open:** none.

### L-F32-14
**Verdict:** CONFIRMED.
**Homes as found:** "A search that finds nothing proves nothing": AR\brief-preamble.md:103, AR\audit-method.md:124 (names the preamble as source), AR\derive-once-review.md:76, AR\doc-review.md:65, MEM\verify-before-claiming-a-source-is-wrong.md (wording "only proves 'not in what I searched', never 'absent' or 'wrong'"). Forced repro: AR\brief-preamble.md:83 and MEM\trace-pass-inputs-before-live-runs.md:19. Stored versions: MEM\stored-versions-one-rule.md only; grep of HY\docs\agents, HY\CLAUDE.md, HY\docs\development.md for kResultsStamp / stored_versions found nothing; owner file HY\src\store\stored_versions.h exists. MEM\open-the-source-before-offering-work.md:11,15 single home. MEM\sp-end-is-one-point-leeway-is-delgars.md:16 (check what each side answers before calling it a split) with AR\audit-method.md:99.
**Splits:** None.
**Ruling:** none found.
**Open:** none.

### L-F32-16
**Verdict:** CONFIRMED.
**Homes as found:** C:\Users\Patrick\.claude\skills\reverse-engineer-anything\SKILL.md:50-57 (read-only dry-run plan, "obtain approval before setup writes configuration"); references\connection-and-recovery.md:54-56 (same rule in near words: "Obtain approval for writes when that concrete scope is not already authorized"). `rea-agents@6.5.0` in SKILL.md:37,51,56,71,79; AR\this-machine.md:25 also names it (REA entry); neither says which owns the version. SKILL.md:219-220 ("ask which app before searching") single home. SKILL (analyzing-sp-squeezes) one-session-edits-fcvideo rule is its last tips line (:62 by finder; text found); the "Never modify the Hydra repo" line is :49.
**Splits:** None.
**Ruling:** none found.
**Open:** none.

### L-F32-17
**Verdict:** CONFIRMED.
**Homes as found:** MEM\mimalloc-link-order.md:11 ("`hydra_use_mimalloc` in CMakeLists.txt owns the linking (docs/development.md)"); HY\CMakeLists.txt:109, :266, :417, :846 mention it and :1095 `function(hydra_use_mimalloc)`, called :1113; HY\docs\development.md:231-236 (names `hydra_use_mimalloc` at the bottom of CMakeLists). MEM note also pins `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\dumpbin.exe` (a machine path with a compiler version); grep of AR\this-machine.md for dumpbin / 14.44 / MSVC found nothing.
**Splits:** None.
**Ruling:** none found.
**Open:** whether that dumpbin path exists today: not checked.

### L-F32-18
**Verdict:** CONFIRMED.
**Homes as found:** G\settings.fragment.json registers `UserPromptSubmit` -> `prompt.py` and `PostToolUse` (Bash|PowerShell) -> `shell_output.py` (about :124-133 and :190-200); the live C:\Users\Patrick\.claude\settings.json carries the same blocks (about :134-143, :200-210). G\README.md:23 ("`shell_output.py` is the one hook on PostToolUse"), :33 ("`prompt.py` is the one hook on UserPromptSubmit. It runs `posture.py`, `percall.py` and `waiver.py` in one Python process"), :53 and :83/:162 (git shim and installer text). G\prompt.py:21 `FIELD_CAP = 10000` (used :54,:57; row said :20). G\install_ref_gate.ps1:12 `$marker = '# git-ref-gate shim'`.
**Splits:** None.
**Ruling:** none found.
**Open:** none. Held to words by G\tests (not opened here).

### L-F32-19
**Verdict:** CONFIRMED.
**Homes as found:** G\session_cards_start.ps1:43 `$ClaimHours = 2`, :45 `$RemovalReminderMb = 500`, :47 `$ReminderGapHours = 24` (comments "The user's choice (task SC-9, 2026-10-14)"), header :13-17 repeats them by variable name; :268 and :289 hook text says a commit that changes only docs/sessions "needs no reviewer". C:\Users\Patrick\.claude\agents\card-writer.md:9 sends the writer to SC\writer-brief.md "which replaces the repo's docs/agent..." (preamble). G\tests\test_h53_session_cards_start.ps1:9 "The thresholds themselves live only in session_cards_start.ps1".
**Splits:** None.
**Ruling:** AR\rulings\agents-and-workflow.md:12 (Confirmed) "use haiku for these simple transcript readers"; :15 "continue, and raise the cap to 60." Neither is the 2-hour window, the 500 MB, or the 24 h. Grep of AR\rulings for "2 hours", "two hours" found nothing; the "user's choice (SC-9)" comment is a claim.
**Open:** none.

### L-F32-20
**Verdict:** CONFIRMED-CHANGED. One cited line is contradicted by other hooks.
**Homes as found:** HY\tools\hydra_ui_mcp.py:753-760 (server instructions: "Refs below the toolbar need **/ (the search box is **/##search)") and AR\ui-testing.md:107,123,217,218 say the same. HY\tools\ch_mcp\quiet.py:35 `COMPILER_NAMES` (used :119), :49 `REPORTED_NAMES = ("Spotify.exe", "MsMpEng.exe")`; machine_report.py:43 `ALWAYS_LISTED_NAMES = quiet.COMPILER_NAMES + quiet.REPORTED_NAMES` (reuses, not a copy). AR\ui-testing.md:99 ends: "no hook reads shell text, so the rule against scrolling the library without the user's yes is the only thing that binds there."
**Splits:** One new. ui-testing.md:99 says no hook reads shell text. But AR\brief-preamble.md:43 says a hook checks every recursive or forced delete "you run from a shell", and G\percall.py's whole-library guard reads shell text for the library flag (see L-F32-08). They differ for any claim of the form "no hook reads shell text": true for a wheel or key message sent from a script to Hydra's window, false for delete and library-flag shell calls.
**Ruling:** none Confirmed. AR\rulings\clone-hero.md:72 ("raise the limit to 30%") is under `## Unconfirmed` (starts :67).
**Open:** none.

### L-F32-22
**Verdict:** CONFIRMED.
**Homes as found:** G\README.md:15: "The check reads the structured tool name only, never shell text, because an agent's shell looks like a main session's". HY\Directory.Build.targets (tracked in git): target `HydraBuildPassCheck` runs `tools\build_slot.ps1 -Check` and errors, under condition `'$(CLAUDECODE)' == '1'`, for Visual Studio projects ("It stops a Visual Studio project that was started without going through build_cpp.ps1, such as a hand-run cmake or MSBuild"). HY\docs\development.md:138-141 says the same ("A second lock stops a build that skips build_cpp.ps1 ... a hand-run `cmake --build` is refused"), and :143 says the limit applies only when `CLAUDECODE` is 1.
**Splits:** None in value.
**Ruling:** AR\rulings\agents-and-workflow.md:8 (Confirmed) "agents are never allowed to start their own builds, only the main session". Names the rule, not the route.
**Open:** whether any build route is left (a non-Visual Studio generator, or a shell with CLAUDECODE cleared) needs a run. Hook gap: a gap may exist at the build-pass check (G\README.md:15 and percall.py); it is covered only by the second lock named above.

### L-F33-05
**Verdict:** CONFIRMED (device names, and now the prose homes).
**Homes as found:** HY\tools\ch_mcp\kit_tools.py:127-129 (comment cites 2026-10-30 "Switch it off (Recommended)" and "Switch it off too (Recommended)"; `KIT_DEVICES = ("keyboard", "mouse")`); HY\tools\ch_mcp\agent\kit.ts:199 `type Device = "keyboard" | "mouse"`, :380 `setDeviceOn("keyboard", false)`. AR\clone-hero-mcp.md:15 (server backs up only the registry key; `backup` in game_files.py owns it; no game file is backed up), :19 (registry key `rewired.reg`), :21 (settings.ini one line set), :25 (user's own keyboard and mouse), :60; HY\tools\ch_mcp\README.md:75,81 (same backup text) and :89 (keyboard and mouse "as the user ruled on 2026-10-30"). Same text in the `ch` MCP server instructions. HY\tools\ch_mcp\game_files.py:77 `REG_CAP_S = 30.0`, used :87,:91.
**Splits:** None in value. The three prose homes agree with each other and with the code.
**Ruling:** AR\rulings\clone-hero.md:8 (Confirmed, heading :7) 2026-11-13 chose "Leave the line changed" for settings.ini; :9 "stop messing with the game's save file. just let it stay." The 2026-10-30 rulings (keyboard/mouse off; "Cap at 30 s") are cited in code and docs but found nowhere under AR\rulings (grep for "Cap at 30|Switch it off|Leave the line changed": only the 2026-11-13 hit).
**Open:** none.

### L-F33-07
**Verdict:** CONFIRMED.
**Homes as found:** SC\prune_list.py:23 `PROTECTED_FOLDERS = ("memory", "session-cards")`; SC\common.py:34 `DATA_ROOT = os.path.join(PROJECTS_ROOT, PROJECT_NAME, "session-cards")`; SC\test_session_cards.py:61, :276, :948, :1085 repeat the names; SC\writer-brief.md:25 states the data folder as a full path. SC\migrate.py:26-27 `SOURCES = "Sources"` with comment "The block the user dropped on 2026-10-31"; SC\common.py:49 comment "the user dropped it on 2026-10-31"; test_session_cards.py:1173 `OLD_CARD_TAIL`. G\session_cards_start.ps1:45 (`500`), :47 (`24`); G\tests\test_h53_session_cards_start.ps1:230,233 use 499.9 and 500.0, :241 uses a 25 hour back-date (own copies, as the finder said).
**Splits:** None.
**Ruling:** none found (grep of AR\rulings for "Sources block", "Sources": nothing relevant). "The user dropped it" is a claim.
**Open:** none.

### L-F33-13
**Verdict:** CONFIRMED.
**Homes as found:** G\wake.py:25 `WATCH_SUFFIX = ".wake.json"  # which watch is the newest for a session`; :30-34 `claim` docstring "An older watch sees the new token on its next pass and ends quietly, so watches never pile up."; :37-39 `_still_newest`. G\README.md:31 describes `wake.py` (stop hook, scans every `stall.SCAN_EVERY` seconds) but grep for "newest" or "pile" in README finds no sentence on the token. G\tests\test_wake.py:145-152 tests the behaviour.
**Splits:** None.
**Ruling:** none found.
**Open:** none.

### L-F33-15
**Verdict:** CONFIRMED-CHANGED (line numbers; claims hold).
**Homes as found:** AR\test-rubric-examples.md:263 holds the old test's place and commit ("(tests/test_timing.cpp:72-82) and its commit, 3b699581"), not :210 (:210 is the Q5/Q8 judgement text). Commit 3b699581 exists: "Checkpoint: complete C++ port through Phase 6 alongside the Python oracle". The G1 removal rule is at :218: "If a later reader finds an answer in that record that is wrong, G1 comes off this page; it does not stay with a note." :218 also says the test "replaced 'timing: to_multiplier thresholds' (B14)". HY\tests\test_timing.cpp:73 `TEST_CASE("timing: the combo multiplier steps up every ten notes and stops at x4")` matches the G1 title (:216). History told twice: AR\test-rubric.md:13-15 ("An earlier version of this page ...") and AR\test-rubric-examples.md:338-343 ("What was removed from this page").
**Splits:** None.
**Ruling:** none found.
**Open:** nothing checks the page against the test name (grep of tests\*.cpp for "test-rubric-examples": no hit). The G1 removal rule has no check.

### L-F33-20
**Verdict:** CONFIRMED (items spot-read; pointers true).
**Homes as found:** G\sync_settings.py:8-17 (what the script owns; replaced whole vs merged lists). G\ruling_rules.py:108 `CHECK_COMMAND = "python.exe -I C:/Users/Patrick/.claude/gates/ruling_rules.py --check"`; copy G\settings.fragment.json:6 `PowerShell(python.exe -I ... --check *)` and live settings.json:12 same; G\tests\test_docs_numbers.py:137 holds fragment == CHECK_COMMAND, :142 `test_live_settings_carry_the_fragment` which `skipTest`s when no settings.json beside gates (a worktree). HY\CLAUDE.md "A finished plan or handoff leaves the repo ..." paragraph present. MEM\ytdl-folders-not-a-project-matter.md:11 ("Don't raise C:\ytdl\videos or C:\ytdl\tracks being empty"); grep of HY\CLAUDE.md and AR\*.md for "ytdl": no hit (no hook or doc). reverse-engineer-anything SKILL.md:341-342 and references\connection-and-recovery.md:84 and javascript-applications.md:173 all repeat the `close_binary` advice. HY\tools\build_slot.ps1:143-146 `Test-BuildSlotLive`; HY\tools\test_build_pass.ps1:162-233 tests live/expired/dead owner.
**Splits:** None.
**Ruling:** none found.
**Open:** the `close_binary` copies were matched by phrase only.

### L-F33-21
**Verdict:** CONFIRMED.
**Homes as found:** HY\tools\build_slot.ps1:10 comment "Each slot is a lock file, C:\Users\Patrick\.claude\hooks\state\build_slots\" and :78 `$BuildSlotDefaultDir = 'C:\Users\Patrick\.claude\hooks\state\build_slots'`. That folder exists. The gates' own state folder is C:\Users\Patrick\.claude\gates\state (holds agent_stop.jsonl, agents, cleanup_sweep, posture, refusals.jsonl, reviews.jsonl, rulings.jsonl, session_cards). Grep of G\*.py, G\README.md, AR\*.md and HY\docs\development.md for "build_slots" found no mention outside build_slot.ps1.
**Splits:** None (two state folders, one per owner).
**Ruling:** none found.
**Open:** none.

## Ids not reached
None. All 27 rows were reached.

Summary counts: CONFIRMED 23, CONFIRMED-CHANGED 4 (L-F31-16, L-F32-06, L-F32-20, L-F33-15), REFUTED 0.
