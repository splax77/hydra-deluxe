# TOOL-AUDIT-T7-C23 (batch 22: S11 sorting, L11 ledger)
Paths: HY = C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout, current); G = C:\Users\Patrick\.claude\gates; AR = HY\docs\agents; MEM = the project memory folder; SC = C:\Users\Patrick\.claude\tools\session_cards.
Result: 24 PASS, 0 REOPEN, 0 GONE, none not reached. Weakest passes: L-F32-13, L-F33-18, L-F33-20.

### L-F32-11
**Verdict:** PASS (P)
**Homes read:** MEM\questions-describe-the-real-rule.md:13,20; AR\audit-method.md:36,146; AR\agent-rules.md:18 (rule 12).
**Why:** The note quotes the user's "same functionality" line as why a narrowing needs a question; audit-method.md:36,146 says narrowing the audit scope is its own question. Same rule on two subjects, nothing contradicts.

### L-F32-13
**Verdict:** PASS (P), weak
**Homes read:** MEM\trace-pass-inputs-before-live-runs.md (How to apply, third bullet); AR\premise-map.md:68 ("at most one `(never tried)` call"); AR\clone-hero-mcp.md:58.
**Why:** The note says mark each call "never tried", check each offline and show the list. It sets no cap and does not allow several in one step. premise-map.md sets the cap. A pointer to premise-map adds the cap; no statement conflicts.

### L-F32-14
**Verdict:** PASS (P)
**Homes read:** AR\brief-preamble.md:103,83; AR\audit-method.md:124,99; AR\derive-once-review.md:76; AR\doc-review.md:65; MEM\verify-before-claiming-a-source-is-wrong.md:22,28; MEM\trace-pass-inputs-before-live-runs.md (forced repro bullet); MEM\stored-versions-one-rule.md; MEM\open-the-source-before-offering-work.md:11,15; MEM\sp-end-is-one-point-leeway-is-delgars.md:16; AR\owner-map.md:43.
**Why:** The "nothing found proves nothing" wordings and the forced-repro wording all say the same thing. The stored-versions note points at src/store/stored_versions.h, which exists. No home contradicts another.

### L-F32-15
**Verdict:** PASS (C)
**Homes read:** ~\.claude\skills\analyzing-sp-squeezes\SKILL.md:10; C:\Users\Patrick\Downloads\Hydra\squeeze-tools\squeeze_scout.py:5,33-34 and README.md:9,61; HY\src\core\model.h:106,121.
**Why:** The skill, the scout tool and its README say kSqueezeWindowMs is in src/search/graph.h. It is at src/core/model.h:121 (grep of src: no declaration in graph.h). squeeze_scout.py:33 and README.md:61 also name kDefaultHitWindowMs, which does not exist in src (grep). The current name is kHitWindowMs at model.h:106 (85.0). Both are moved or missing names with a clear current place. Values (500, 85) agree.

### L-F32-16
**Verdict:** PASS (P)
**Homes read:** ~\.claude\skills\reverse-engineer-anything\SKILL.md:37,50-57,71,79,219-220; references\connection-and-recovery.md:54-56; AR\this-machine.md:25; skills\analyzing-sp-squeezes\SKILL.md:49,62.
**Why:** rea-agents@6.5.0 is the same in all homes. The approval rule is worded the same in SKILL.md and connection-and-recovery.md. The fcvideo rule has one home.

### L-F32-17
**Verdict:** PASS (P)
**Homes read:** MEM\mimalloc-link-order.md:11,15; HY\CMakeLists.txt:109,266,417,1095; HY\docs\development.md:236.
**Why:** The function hydra_use_mimalloc exists at CMakeLists.txt:1095 and every pointer to it is true. The only oddity is the dumpbin machine path in the note. It contradicts nothing.

### L-F32-18
**Verdict:** PASS (P)
**Homes read:** G\settings.fragment.json:124-133,190-200; C:\Users\Patrick\.claude\settings.json:143,210; G\prompt.py:18-21; G\README.md:23,33; G\install_ref_gate.ps1:12.
**Why:** Live settings and the fragment register prompt.py and shell_output.py the same way. README:33 names FIELD_CAP and prompt.py:21 sets 10000. Marker text matches.

### L-F32-19
**Verdict:** PASS (P)
**Homes read:** G\session_cards_start.ps1:13-17,43,45,47,199,215,235; grep of HY\CLAUDE.md and AR\*.md for "2 hours" and "24 hours" (no copies).
**Why:** The thresholds ($ClaimHours=2, $ReminderGapHours=24, $RemovalReminderMb=500) are in one script and its own comments. No doc states a different number.

### L-F32-20
**Verdict:** PASS (A)
**Homes read:** AR\ui-testing.md:99 ("no hook reads shell text"); G\percall.py:14,46-55,146-163 (_library_refusal reads the shell command text for the whole-library flag; delete_rule.check, bash_windows_rule.check and media_rule.check also read shell calls); AR\brief-preamble.md:43 ("A hook checks every ... delete you run from a shell").
**Why:** The doc line is a doc, the other side is hook program code, and the code reads shell text. Code wins. The doc's narrower point still holds: grep of G for scroll/wheel finds no hook that stops a scroll script. The fix is a rewording of the line.

### L-F32-22
**Verdict:** PASS (P)
**Homes read:** G\README.md:15; HY\docs\development.md:136-141; HY\Directory.Build.targets:1-11.
**Why:** README says the pass check reads the tool name only. development.md and Directory.Build.targets say a second lock covers a hand-run build. They agree. Hook gap exists at the README:15 pass check (it reads no shell text, so an agent's shell can start a build past it, though Directory.Build.targets covers MSBuild); no steps given.

### L-F33-01
**Verdict:** PASS (P)
**Homes read:** G\cleanup_sweep.ps1:49,51,54,56,454,457; G\common.py:15; HY\tools\hydra_ui_mcp.py:600,770; AR\ui-testing.md:213.
**Why:** 7 days, 24 h, 1 h, 30 days, 100 retries and 10 ms agree everywhere. Two prose copies say "a week" with no owner named, and the retry pair agrees.

### L-F33-05
**Verdict:** PASS (P)
**Homes read:** HY\tools\ch_mcp\kit_tools.py:8,129; HY\tools\ch_mcp\agent\kit.ts:199,380-381; HY\tools\ch_mcp\game_files.py:77,91; AR\clone-hero-mcp.md:15,19,21,25,27,47; HY\tools\ch_mcp\README.md:89.
**Why:** Device names (keyboard, mouse), the 30 s cap and the backup text agree across code and prose. clone-hero-mcp.md:15 quotes the 2026-11-13 ruling that is filed. The unfiled 2026-10-30 rulings cited in code comments are a separate finding (L-F32-21), not a split here.

### L-F33-07
**Verdict:** PASS (P)
**Homes read:** SC\prune_list.py:23,69; SC\common.py:34; SC\writer-brief.md:25; SC\migrate.py:27; SC\test_session_cards.py:1173; G\session_cards_start.ps1:45,47.
**Why:** "session-cards" and "memory" are repeated literals with the same values. The thresholds sit in one script. Nothing disagrees.

### L-F33-09
**Verdict:** PASS (P)
**Homes read:** SC\prune_list.py:1-9,23,62-74; SC\common.py:34; SC\check.py:38 (CARD_FILE: 8-hex card name); G\session_cards_start.ps1:16,346-350.
**Why:** CARD_FILE matches a card file name and listable() matches a full session id. They answer different questions. The protected folder names agree.

### L-F33-11
**Verdict:** PASS (C)
**Homes read:** G\session_cards_start.ps1:21; G\cleanup_sweep.ps1:16,28,592,658; ls of G (no state_prune.ps1).
**Why:** The comment names state_prune.ps1, which does not exist. The owner is Invoke-StatePrune at cleanup_sweep.ps1:592. The current place is clear.

### L-F33-13
**Verdict:** PASS (P)
**Homes read:** G\wake.py:25-34; G\README.md:31; G\tests\test_wake.py:145-152.
**Why:** README says an older watch that finds a newer token ends quietly so watches never pile up. wake.py:30-34 says the same, and the test checks the behaviour.

### L-F33-14
**Verdict:** PASS (P)
**Homes read:** AR\audit-method.md:108,110,166-169; grep of AR\*.md and HY\CLAUDE.md for "triage" (no other home).
**Why:** One rule (the fixing merge updates the row), told once as a rule and once as a history. The two tellings agree (192 of 396).

### L-F33-15
**Verdict:** PASS (P)
**Homes read:** AR\test-rubric-examples.md:210,216-218,336-342; HY\tests\test_timing.cpp:73; `git log -1 3b699581` (subject matches).
**Why:** The commit exists and its subject is what the page says. The G1 test name is at test_timing.cpp:73. The removal rule is stated once. Only the finder's line numbers were off.

### L-F33-16
**Verdict:** PASS (P)
**Homes read:** AR\this-machine.md:3; HY\CLAUDE.md:9; AR\brief-preamble.md:91-95,105; AR\agent-rules.md:15.
**Why:** this-machine.md:3 covers lines on the machine page; CLAUDE.md:9 and preamble:105 cover Clone Hero pages. Different questions. The preamble contains no rule about this-machine.md, so nothing conflicts.

### L-F33-17
**Verdict:** PASS (P)
**Homes read:** HY\tools\ci_build_folder.ps1:51; HY\tools\test_ci_build_folder.ps1:61,95; HY\docs\development.md:882-890; HY\.github\workflows\ci.yml:92,466-469.
**Why:** The 2000-01-01 date appears in the script and its test. The test fails if the date changes. development.md names the script as the owner.

### L-F33-18
**Verdict:** PASS (P), weak
**Homes read:** HY\tools\hydra_ui_mcp.py:41-42,459,566,636; AR\ui-testing.md:280.
**Why:** REPLY_MARGIN_S = 15.0 has one home and is used for both the reply wait and the exit wait. No home states another number, and no doc copies the value. This is one number with two jobs, not a split in value. Naming a second constant of 15 changes no behaviour.

### L-F33-19
**Verdict:** PASS (A)
**Homes read:** HY\tools\ch_mcp\README.md:83; AR\clone-hero-mcp.md:60; HY\tools\ch_mcp\bot_run.py:115-120,750-761,795-802.
**Why:** README:83 says the copy is only for the bot's launch and a person-played song is not copied. human_refusal (bot_run.py:120) refuses human=True with no song folder, so every human launch has a folder, cap_s is set, and wait_stats copies. clone-hero-mcp.md:60 says the same as the code. One side is a README, the other is code.

### L-F33-20
**Verdict:** PASS (P), spot-read
**Homes read:** G\ruling_rules.py:108; G\settings.fragment.json:6; ~\.claude\settings.json:12; G\tests\test_docs_numbers.py:137-141; G\sync_settings.py:8-17; HY\tools\build_slot.ps1:143,159,254 and tools\test_build_pass.ps1 (named only); HY\CLAUDE.md:57; the ytdl note in MEM; reverse-engineer-anything SKILL.md:341-342, javascript-applications.md:173, connection-and-recovery.md:84.
**Why:** CHECK_COMMAND is the same in all three homes and a test holds them equal. The SKILL pointers read the same close_binary advice. The rest are single homes. I did not open test_sync_settings.py or the full test_build_pass.ps1 range.

### L-F33-21
**Verdict:** PASS (P)
**Homes read:** HY\tools\build_slot.ps1:10,78; ls C:\Users\Patrick\.claude\hooks\state (build_slots) and G\state; grep of docs and G for "build_slots".
**Why:** Two state folders exist, one per owner. build_slot.ps1 names its own path in two places that agree.

## Not reached
None.
