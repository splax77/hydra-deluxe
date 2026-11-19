# TOOL-AUDIT-T7-C20 closure check (sorting S10, ledger L10)

Checked against current files. Hydra files read from the main checkout (C:\Users\Patrick\Downloads\Hydra\hydra-test); hook files from C:\Users\Patrick\.claude\gates (G); model-postures.md (MP) at C:\Users\Patrick\.claude\model-postures.md. Short names: AR = docs/agents/agent-rules.md, BP = brief-preamble.md, DOR = derive-once-review.md, IN = integrate.md, CH = clone-hero-mcp.md, GR = G\README.md, PC = Hydra CLAUDE.md.

Counts: PASS 25, REOPEN 0, GONE 0. Not reached: none.

### L-F28-01
**Verdict:** PASS (P)
**Homes read:** G\percall.py:20 and :142-145 (refusal text); tools\build_slot.ps1:75 `$BuildSlotCount = 3` and :87 `$BuildPassHowTo`; tools\hydra_ui_mcp.py:874 `def hydra_build_pass`; AR:20 and PC rule 14 "At most three builds run at once"; BP:57; MP:11.
**Why:** The tool name, the count of three and the rule "agents never build, hand back" read the same in every home. Only the refusal sentence is worded three ways, with the same meaning. A pointer changes no behaviour.

### L-F28-03
**Verdict:** PASS (P)
**Homes read:** G\stall.py:30-33 (STATUS_HEADER "Relay the claim to the user briefly, in plain English. When the evidence disagrees..."); MP:11 "Relay the claim to the user briefly, in plain English, and add a second sentence only when the evidence disagrees."
**Why:** Same instruction. stall.py adds "open the agent's transcript" and MP:11 says reading a transcript needs no approval; no conflict.

### L-F28-04
**Verdict:** PASS (A)
**Homes read:** AR:27 ("The notice repeats at 20 and 30 minutes ... An agent that has not handed back 30 minutes after its first call gets one more notice"); G\stall.py:22-24 (STALL_SECONDS 600, LIMIT_SECONDS, STALL_NOTICES) and :129-144 `_past_limit` (`now - first <= LIMIT_SECONDS` skips, so the notice comes only after more than 30 minutes).
**Why:** The only split is a doc sentence ("30 minutes after") against the hook's `<=` at exactly 30 minutes. Doc against a program file.

### L-F28-05
**Verdict:** PASS (P)
**Homes read:** G\stall.py:27 `CTX_THRESHOLD = 500000` and :36 CTX_TEXT "fired at 500K (50% of the window)"; MP:15 "Keep this conversation under 500K tokens".
**Why:** All three say 500K. No disagreement; the 500K is a typed copy in three places, replaceable with a pointer.

### L-F28-07
**Verdict:** PASS (P), with a note
**Homes read:** G\common.py:53-76 `run` ("closed" denies on a crash, "loud" prints a systemMessage, both return 0); GR:95-100 (same two modes, exit 0, onFailure: block for a hook that cannot start); G\cleanup_sweep.ps1:30-31 and :665-666, :735-743; G\session_cards_start.ps1:23-25 and :355-371 ("fail loudly, never freeze", exit 0 always). G\settings.fragment.json registers only these two .ps1 files as hooks (lines 79, 91), and both only inform.
**Why:** The sorting line says "both warn and exit 0". That is not the whole of it: Python hooks that can refuse deny on a crash. But the README states the same two modes, and the PowerShell hooks are informers, so they match the "loud" mode. No two homes answer differently.

### L-F28-08
**Verdict:** PASS (A)
**Homes read:** G\cleanup_sweep.ps1:33-35 (names -ScratchRoot, -StateRoot, -ClaudeRepo, -ClaudeHome, -Foreground) against its param block at :37-46 (also -Worker, -Cwd, -SessionId); G\session_cards_start.ps1:27-31 ("-ToolsDir ... -DataDir ... -CardsDir ... -StateRoot ... The test uses all four") against its param block at :33-39 (also -BudgetSec); G\tests\test_h53_session_cards_start.ps1:94 passes -BudgetSec.
**Why:** The header comments list fewer parameters than the param blocks, and "all four" is false against the test. Comment against code in the same programs.

### L-F28-10
**Verdict:** PASS (C)
**Homes read:** Hydra .derive-once-gate and .doc-review-gate (name `hooks\lib\derive_once_rules.ps1` and `hooks\lib\doc_review_rules.ps1`); C:\Users\Patrick\.claude\hooks holds only `state`, `hooks\lib` does not exist; G\review_rules.py:18-24 (markers, BUILTIN_CODE, PUBLIC_DOCS), :181-186 `_is_code`, :419-480 `explain_main_move`.
**Why:** Moved file named by the markers. The current place is clear: G\review_rules.py (`_is_code` and BUILTIN_CODE for code paths, PUBLIC_DOCS and `explain_main_move` for documents).

### L-F28-11
**Verdict:** PASS (P)
**Homes read:** G\session_cards_start.ps1:267-268 (comment: a commit on main that changes only docs/sessions needs no reviewer, but every commit needs trailers); G\review_rules.py:21-22 BUILTIN_CODE (src, tests, tools, third_party, installer, .github, CMakeLists.txt, CMakePresets.json, build_cpp.ps1), :181-186 `_is_code`, :440-443 (no code path changed gives "no-review-needed"), :468-470 (documents gated only if PUBLIC_DOCS or the marker); Hydra .derive-once-gate holds only prose, so the built-in list applies.
**Why:** docs/sessions/ matches neither BUILTIN_CODE nor PUBLIC_DOCS, so the comment is true today. The comment only restates the rule and could point at `explain_main_move`. (The ledger said the finder did not trace this; I did.)

### L-F28-12
**Verdict:** PASS (P)
**Homes read:** C:\Users\Patrick\.claude\tools\session_cards\check.py:37 `CATCHERS = ("main session", "hook notice", "user")` and :62-63; writer-brief.md:91 (same three phrases).
**Why:** Same three values. (check.py also cites a supervisor plan; that is a separate row, L-F28-06.)

### L-F28-14
**Verdict:** PASS (B)
**Homes read:** PC:1 "Ask questions if something is unclear."; C:\Users\Patrick\.claude\projects\...\memory\ask-only-game-or-permission-questions.md (limits questions to game or permission); AR:15 (a plan's made-up rule is not put to the user) and audit-method.md:91-104 (how a question is written; no limit on topics); grep of docs\agents\rulings for "game or permission", "fact about the game", "if something is unclear": no hit.
**Why:** The one split is a memory note against PC:1. No other doc narrows questions to game-or-permission (AR:15 narrows only what a plan may invent). No ruling holds either side.

### L-F28-22
**Verdict:** PASS (P)
**Homes read:** G\percall.py:142-145 (refuses only the tool named in BUILD_PASS_TOOL); AR:27 ("`build_cpp.ps1` then refuses any build with no pass"); build_cpp.ps1:15 and :72-76 (needs a pass under Claude Code).
**Why:** No two homes disagree. The hook gap (a build through another route is stopped by build_cpp.ps1, not by percall.py) is already stated in AR:27.

### L-F29-02
**Verdict:** PASS (P)
**Homes read:** PC rule 14; AR:7 and AR:20; BP:47; DOR:116; MP:9; C:\Users\Patrick\.claude\agents\executor.md:11 and reviewer.md:21; GR:17.
**Why:** All say agents may run `hydra_precheck.exe` from main and may not run `hydra_tests.exe`, `hydra_uitest.exe` or `hydra_replay.exe`. GR:17 itself says no hook enforces the test part. No disagreement.

### L-F29-05
**Verdict:** PASS (P)
**Homes read:** PC:71 (rule 9); AR:15; BP:95 and :103; MP:13; handoff.md:3; DOR:76; doc-review.md:65; audit-method.md:124; owner-map.md:3 and :35; BP:109-111.
**Why:** The rule 9 sentences and the "a search that finds nothing proves nothing" sentences are word-for-word or near copies and agree. BP:111 points at owner-map.md "The stops" and that heading exists.

### L-F29-09
**Verdict:** PASS (B)
**Homes read:** AR:9 ("The main session's compiling and test runs inside approved work need no separate yes (rule 14)"); MP:3 (nothing starts without the yes, a code-only change's "build" still waits, meaning the piece of work); memory one-build-per-wave.md ("before launching work that will need builds, say how many builds it needs and get a yes"); docs\agents\rulings\ grep for "one build per wave" and "lots of builds": no hit.
**Why:** The split is a memory note against AR:9. The memory note quotes the user but is not a filed ruling. MP:3 uses "build" in rule 3's sense, so it does not side with the memory note.

### L-F29-12
**Verdict:** PASS (P), with a note
**Homes read:** BP:5 (seven things hooks enforce, "a few"); AR:27 ("Hooks check four of these" plus the stall notice); GR:3 and GR:116 (one registration per event); G\stall.py:22-24.
**Why:** The lists cover different scopes (BP:5 hook behaviours an agent meets; AR:27 which of the fifteen rules a hook keeps). No value or claim in one contradicts the other. A pointer to GR as owner would need no choice of value. Note for the main session: BP:5 omits the build-pass refusal and key-route check, but BP:5 says "a few", so it does not claim completeness.

### L-F29-14
**Verdict:** PASS (A)
**Homes read:** DOR:66 ("`retime_from_end` ... moves each later press by the read end minus the planned end, one for one"); tools\ch_probe\experiments\play_chart.py:361-369 `own_end_now_ms` (own_stored_end_ms + conversion[side] * (read_end_ms - read_stored_end_ms)) and :440-442 (aim_ms moved by own_now_ms - own_end_ms).
**Why:** The doc says one for one; the code scales by the file's conversion. They differ whenever the conversion is not 1. Doc against code.

### L-F29-15
**Verdict:** PASS (A)
**Homes read:** DOR:57-66 (the Pitch Dark example: the tool takes the squeeze's ratio from `SPSqueeze::transfer` for chord 466's read, and the fix would call `transfer_scale_between`); tools\replay_schedule.cpp:436-437 and :635-640 (ratio from `sq.transfer` / `act.transfer_post`, used for the squeeze's own `g.ratio`), :653-659 (read end after the anchor scaled by `step_transfer_scale(act, *rs, timing)`); src\core\squeeze_rating.h:53-73 and squeeze_rating.cpp:12-37 (`step_transfer_scale` calls `transfer_scale_between`).
**Why:** The doc's worked example describes a path the code no longer takes: the read after the anchor now uses `step_transfer_scale`. Doc against code.

### L-F29-17
**Verdict:** PASS (A)
**Homes read:** IN:43-44 (scan `-tc="single-owner*"`, docs `-sf=*docs_match_code*`); DOR:99 (`-tc="single-owner*"`); PC:57 and AR:7 (`-tc=docs match code*`, `-sf=*single_owner*`); tools\precheck\test_pick.cpp:787-791 (prints `always: "-tc=docs match code*"` and `always: "-sf=*single_owner*"`).
**Why:** IN and DOR name the scan and docs tests with the opposite filter forms from the ones the precheck prints. Doc prose against the precheck code that writes the filters. Test files exist under tests\single_owner\ and tests\test_docs_match_code.cpp.

### L-F29-19
**Verdict:** PASS (A), with a note
**Homes read:** BP:77 ("Never use run_in_background"); DOR:5 and doc-review.md:5 (no background jobs); G\agent_stop.py:1-5, :46-51, :66-97, :293-321 (an agent that stops to wait on background work is marked parked, not finished); G\stall.py:238-242 (shows "parked since ... waiting on its own background work"); G\progress.py:77-79; G\shell_output.py:75 (skips a call that has run_in_background).
**Why:** The prose bans backgrounding; the hooks treat it as an ordinary state and track it. Doc against hook code. Note: the hooks also cover commands the platform moves to the background after a timeout (agent_stop.py:49-51), so the code does not by itself say a deliberate run_in_background is allowed. That reads as doc against code, not code against code, so A holds.

### L-F29-20
**Verdict:** PASS (P)
**Homes read:** audit-method.md:19-26 (four exits), :34-36, :44 (commit 707c285 exists and holds src/core/squeeze_rating.cpp, confirmed by `git ls-tree`), :116-127; PC:33; docs\sessions\2026-10-11-8bffee2b.md exists; test-rubric.md:189; DOR:106.
**Why:** PC:33 and test-rubric.md:189 agree with audit-method.md. DOR:106 holds no four-exit list, so there is no second copy to disagree. No split.

### L-F30-01
**Verdict:** PASS (P)
**Homes read:** PC rule 14; AR:20; BP:47 and :57; DOR:116; ui-testing.md:27 and :185; doc-screenshots.md:19; executor.md:11; reviewer.md:21; GR:17; G\percall.py:142-145.
**Why:** Every page names the same three programs the same way and agrees agents may run only `hydra_precheck.exe`. Only the pass tool is hook-refused; GR:17 says so itself. No disagreement.

### L-F30-02
**Verdict:** PASS (A)
**Homes read:** AR:14 ("any run over the user's whole song library (for example `hydra_batch --whole-library`, or a whole-library compare or timing run) ... `allow library run`"); G\percall.py:14 `LIBRARY_FLAG = "--whole-library"` and :44-53 `_library_refusal` (refuses only a shell command whose text holds the flag); G\review_rules.py:50-52 and :81-82 `is_library_allow`.
**Why:** The doc claims any whole-library run needs the typed allow; the hook tests only the flag text. Doc against hook code.

### L-F30-03
**Verdict:** PASS (P)
**Homes read:** G\review_rules.py:51 `LIBRARY_ALLOW_TEXT = "allow library run"`; G\tests\test_review_rules.py:839-845, test_progress.py:101 (sets the constant from the real module) and :427-:584, test_review_dispatch.py:401-428, test_posture.py:197-246; AR:14.
**Why:** The constant and every literal are the same text today. A test typing the literal would go red if the constant changed; that is not a disagreement. (Line numbers in the ledger have moved; the files and text are unchanged.)

### L-F30-05
**Verdict:** PASS (P)
**Homes read:** clone-hero-mcp.md:43 ("ends with `ch_stop`'s restore lines"); premise-map.md:61 (example Calls line ends with `ch_stop`); tools\ch_mcp\game_files.py:174-209 (the restore lines); CH:15 (what is restored).
**Why:** The rule is stated at CH:43, and two older plans repeat it with the same meaning. Nothing contradicts it and no second answer exists.

### L-F30-07
**Verdict:** PASS (P)
**Homes read:** G\review_rules.py:23-24 PUBLIC_DOCS (README.md, docs/README.md, docs/UserGuide.md, docs/differences-from-public-hydra.md); PC:45 "the four public pages"; public-doc-style.md:3 (same four pages plus release notes); doc-review.md:17 (same four paths); all four files exist.
**Why:** Same four pages everywhere.

## Ids not reached
None.
