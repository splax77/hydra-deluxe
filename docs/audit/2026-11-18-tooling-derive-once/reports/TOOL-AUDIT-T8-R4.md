# TOOL-AUDIT-T8-R4: re-check of 23 theme-20 rows (first half)

Read on 2026-11-18. Hydra = main checkout. Hooks repo = C:\Users\Patrick\.claude. None of these ids was reopened by T7 (searched reports\TOOL-AUDIT-T7-C*.md for each id: no hit). All ids reached.

Tally: REAL 11 (L-F03-02, L-F03-20, L-F07-02, L-F07-17, L-F10-23, L-F11-04, L-F14-10, L-F15-15, L-F17-16, L-F18-18, L-F18-22). NOT A SPLIT 12 (L-F02-02, L-F02-13, L-F02-20, L-F03-08, L-F05-07, L-F05-08, L-F05-12, L-F07-10, L-F13-24, L-F14-11, L-F16-19, L-F18-21). GONE 0.

### L-F02-02
**Verdict**: NOT A SPLIT.
**Homes read**: gates\tests\test_docs_numbers.py:6 "reads the Hydra docs from the folder in the GATES_HYDRA_ROOT environment variable, or from the Hydra main checkout when that is unset"; :70 `return os.environ.get("GATES_HYDRA_ROOT") or DEFAULT_HYDRA_ROOT`; gates\README.md:154 "It reads the Hydra main checkout unless `GATES_HYDRA_ROOT` names another folder."
One side only: the test's code and its two descriptions agree. Whether the test should also read a branch is a design wish, not two homes answering differently. Searched the test, the README and Hydra docs\agents for a claim that the docs check sees a branch edit: none.

### L-F02-13
**Verdict**: NOT A SPLIT.
**Homes read**: docs\agents\audit-method.md:104 "'five or fewer per message' is the main session's own suggestion"; docs\superpowers\plans\2026-11-08-player-audit.md:102 "Questions go out five or fewer per message"; ...2026-11-08-clone-hero-docs-gaps-search.md:187 "Splits reach the user five or fewer per message"; ...2026-11-14-test-suite-audit.md:194 "five or fewer per message is the main session's own reading of 'small'".
The method says the number is a suggestion; the two 11-08 plans are those audits' own plans choosing that number. They answer different questions (what the method requires vs what that plan does). The newest plan agrees with the method. (The brief's file names player-audit.md and docs-gaps-search.md are the two plans above.)

### L-F02-20
**Verdict**: NOT A SPLIT.
**Homes read**: gates\README.md:118 "An edit to a watched settings file reaches every running session on its next call ... The probe saw this for a project settings file and infers it for the user's `settings.json`, which it did not touch."; README.md:190 "An edit to a watched settings file reaches a running session on its next call; an edit to a `--settings` file does not."; gates\tests\probe_results.md (probe 6): "The user's own ~\.claude\settings.json was not touched".
Line 118 itself states the inference. All three agree on strength.

### L-F03-02
**Verdict**: REAL.
**Homes read**: C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:10 "(`kSqueezeWindowMs`, src/search/graph.h)"; C:\Users\Patrick\Downloads\Hydra\squeeze-tools\squeeze_scout.py:34 `HYDRA_WINDOW_MS = 500.0     # kSqueezeWindowMs, src/search/graph.h`; src\core\model.h:121 `constexpr double kSqueezeWindowMs = 500.0;` (src\search\graph.h:216 only mentions it in a comment). squeeze_scout.py:32 also names `kDefaultHitWindowMs`; model.h has `kHitWindowMs` (:106) and no `kDefaultHitWindowMs`.
**Kind**: moved-pointer (graph.h -> src/core/model.h). Second line: code-vs-code (squeeze_scout.py types 500.0 and 85.0 by hand outside the repo; equal to model.h today).
**What changes**: someone sent to graph.h for the window will not find its definition, and a change in model.h does not reach the scout's typed 500.0 and 85.0.

### L-F03-08
**Verdict**: NOT A SPLIT.
**Homes read**: memory\no-full-library-runs.md:17 "builds over all 18,811 charts" (an event on 2026-10-08); memory\min-path-delta-proof.md:29 "Library scan (a one-off tool, not kept): 18,610 charts" (work dated 2026-08-17, line 21).
Each is a dated count from one past event; neither claims the current library size. Different dates, so the library grew.

### L-F03-20
**Verdict**: REAL.
**Homes read**: memory\search-before-clicking-a-song.md:11 "Never scroll through the user's library to reach a song or a spot without asking in chat first ... That covers every method"; memory\render-checks-use-the-corpus.md:19 "Scroll to the middle of the list, click a song, compare before and after, close, and compare again." User's words, rulings\ui-and-display.md:27 (inside ## Confirmed, lines 7-101): "this should never happenw tihout my approval." Guard tests\ui\uitest_commands.cpp:185-231 refuses only title lookups and page keys; docs\agents\ui-testing.md:99 "no hook reads shell text, so the rule against scrolling the library without the user's yes is the only thing that binds there".
**Kind**: memory-vs-other (two memory notes); ruling-vs-other (the render note's step vs the Confirmed line); homeless-rule (no hook behind the no-scroll rule for shell-sent wheel messages).
**What changes**: an agent doing a render check from the render note scrolls the list by itself; an agent following the other note and the ruling asks in chat first.

### L-F05-07
**Verdict**: NOT A SPLIT.
**Homes read**: tools\ch_mcp\menu_tools.py:45-51 "So Blue is a guess: live step LS-K3, which was to check it, was struck before it got that far ... `SEARCH_NEXT_ITEM = KIT_PREFIX + "blue"`"; rulings\clone-hero.md:81 records LS-K3 pressing Blue then Yellow with the read staying 0/3.
One home, and it says itself the value is unverified. Searched docs\agents, docs\clone-hero and tools\ch_mcp for another statement of which pad scrolls: none.

### L-F05-08
**Verdict**: NOT A SPLIT.
**Homes read**: rulings\ui-and-display.md:27 (Confirmed) "scrolling thorugh the entire library for minutes to reach the song you want"; tools\ch_mcp\menu_tools.py:38-39 `SEARCH_DOWN_CAP = 10` "How many scroll presses the search step makes looking for the target song" (Clone Hero's list), used at :389-403.
The ruling is about Hydra's library, as the note recording it says; the code is ten presses in the game's menu. No text in a file says the ruling covers the game's list; that reading is the row's own framing.

### L-F05-12
**Verdict**: NOT A SPLIT.
**Homes read**: tools\capture\report_shot.ps1:5-9 "the click is measured from the client area's right edge. 80 is 2,560 - 2,480 on the user's maximized window." and `$ClickFromRight = 80`; src\ui\library_toolbar.cpp:130,140 `report_w`, `place_control_at_right_edge(report_w)`; src\ui\widgets.h:197.
The script answers "where to click"; the code answers "where the button is drawn". Both say the right edge, and the script's own comment says the 80 is a measurement. No text claims a different position.

### L-F07-02
**Verdict**: REAL (two splits left of the three the row named).
**Homes read**: memory\archive\agent-worktree-base-is-origin-main.md:15 "Reviewers still use `isolation: worktree` as `docs/agents/derive-once-review.md` says"; docs\agents\derive-once-review.md:200 "leave `isolation` out ... the hook refuses a reviewer dispatch that sets `isolation`"; gates\review_dispatch.py:161-162 `if ti.get("isolation"): return common.deny(...)`; gates\README.md:39 same refusal.
Gone from the row: the old "fewer refusals" gap (DOR:17 and README:39 now list the two-line form). Still differing: README:39 and DOR:17 say a brief lacking the two lines is refused with no condition; review_dispatch.py:169 `if not keys and not labelled and not _gated(cwd): return None` lets such a brief pass in a repo with no review marker on main (neither Hydra nor the hooks repo).
**Kind**: memory-vs-other (archived note vs doc and code); doc-vs-code (README:39 and DOR:17 vs review_dispatch.py:169, unmarked repos only).
**What changes**: a session reading the archived note launches reviewers with isolation and is refused; and a brief with no lines is refused by the docs' account but passes in an unmarked repo.

### L-F07-10
**Verdict**: NOT A SPLIT.
**Homes read**: skills\analyzing-sp-squeezes\SKILL.md:64 "Note whether the fleet hooks held: one background executor per task, no nested background workers."; docs\agents\brief-preamble.md:77 "Never use run_in_background ... You have no helper agents".
The skill line is something to note after a run (what a main session's fleet did); the preamble tells an agent what it may do in its own shell. "Fleet hooks" is defined nowhere I searched (~\.claude gates and skills, Hydra docs\agents, CLAUDE.md; only old session transcripts use the phrase).

### L-F07-17
**Verdict**: REAL (only through the archived note; the other parts of the row are refuted or agree).
**Homes read**: the archived isolation note and the refusal, as in L-F07-02. Refuted or agreeing parts: MAX_BRIEF_FOLDERS is gone (no hit in review_dispatch.py; the Review repo line is read at :22); docs\development.md:143 "The limit applies only when `CLAUDECODE` is 1" matches tools\build_slot.ps1:90 `$env:CLAUDECODE -eq '1'`; gates\percall.py:18 `{"mcp__ch__ch_play_chart": "keys"}` equals tools\ch_mcp\game_tools.py:71 `ROUTE_KEYS = "keys"` (same value, two typed copies, no test ties them); ~\.claude\model-postures.md:5 "A brief may name a commit in the other repo if it writes that repo's root folder" agrees in effect with README:39 "in this repo or the other one".
**Kind**: memory-vs-other (same split as L-F07-02).
**What changes**: same as L-F07-02; this row adds nothing else real.

### L-F10-23
**Verdict**: REAL.
**Homes read**: docs\agents\ui-testing.md:294 "`report_window_flow_tests()`, which `register_tests` calls last"; tests\ui\uitest_tests.cpp:34-35 `register_entries(report_window_flow_tests()); register_entries(command_tests());`. Memory one-reviewer-rule-d107.md:15 "the reviewer cap is 150 calls" equals gates\progress.py:14 `CALL_WARN, CALL_STOP = 100, 150` (agrees today; a typed copy).
**Kind**: doc-vs-code.
**What changes**: the page says the flow tests register last; the code registers command_tests after them. ui-testing.md:296 says order does not matter, so a reader loses only the accuracy of the pointer.

### L-F11-04
**Verdict**: REAL.
**Homes read**: gates\tests\lib\harness.ps1:11 "Its proof is tests\test_h39_harness.ps1." and the comment on `$script:HarnessShell` "Only test_h39_harness.ps1 changes them, to prove the failure paths". gates\tests holds no such file (it has test_h52_cleanup_sweep.ps1, test_h53_session_cards_start.ps1, test_status_append.ps1 and the .py tests). The file exists only inside old worktrees (~\.claude\worktrees\*\hooks\tests\). Searched gates for other users of HarnessShell / HarnessDrainMs: only harness.ps1.
**Kind**: moved-pointer (the named file is gone; no current place found in gates).
**What changes**: the harness says a test proves it and nothing does; the failure paths its HarnessShell and HarnessDrainMs variables exist for run in no test.

### L-F13-24
**Verdict**: NOT A SPLIT.
**Homes read**: gates\README.md:5 "The rules every agent follows (status lines, commits, trailers, tests) live in the Hydra repo's docs/agents/brief-preamble.md. This page does not repeat them."; README.md:19 (delete rule), :23 (output trim), :25, :27 (Bash shapes) describe how each hook works; brief-preamble.md:43 states the delete rule for agents.
On the delete rule both say the same thing (literal absolute path inside scratch or a `worktrees` folder). The README describes hook behaviour; line 5 is about the agents' working rules. Same facts twice, not two answers. I compared the delete rule only.

### L-F14-10
**Verdict**: REAL (same split as L-F03-20, same two notes).
**Homes read**: memory\render-checks-use-the-corpus.md:19 "Scroll to the middle of the list, click a song"; memory\search-before-clicking-a-song.md:11 "Never scroll ... without asking in chat first"; rulings\ui-and-display.md:27 (Confirmed); guard tests\ui\uitest_commands.cpp:185-231 covers only title lookups and page keys.
**Kind**: memory-vs-other; ruling-vs-other; homeless-rule (no hook for shell-sent wheel or key messages; ui-testing.md:99 says so).
**What changes**: same as L-F03-20.

### L-F14-11
**Verdict**: NOT A SPLIT.
**Homes read**: docs\agents\clone-hero-mcp.md:23 "The game window is never brought to the front ... The one launch that cannot ask is the spawn form ... The count is information, not a pass condition: the user ruled on 2026-10-31 that the game taking focus briefly does not matter"; memory\ch-runs-must-not-need-focus.md:6 "the run may not need the game window in front or take the keyboard", :16 "Brief focus is not a failure", and its Traps paragraph "never show the window" (about SW_SHOWNOACTIVATE un-minimizing); rulings\clone-hero.md:34 and :80 hold the focus quote: line 34 is under ## Confirmed (lines 7-66), line 80 under ## Unconfirmed (from line 67).
Doc, memory note and ruling agree: brief focus is allowed, a run must not depend on focus.

### L-F15-15
**Verdict**: REAL.
**Homes read**: skills\reverse-engineer-anything\SKILL.md:37-41 lists 15 ids ending "`commandcode`, `vscode`, `grok_build`, `omp`, or `grok_bot`"; skills\reverse-engineer-anything\references\connection-and-recovery.md:46-50 lists 23, adding `qwen_code`, `trae`, `pi`, `hermes`, `qoder`, `droid`, `zcode`, `dsh`.
**Kind**: doc-vs-doc.
**What changes**: for a client such as `qwen_code`, SKILL.md does not list it among supported ids (so it reads as unsupported), while the reference page names it as an id for `doctor --client`.

### L-F16-19
**Verdict**: NOT A SPLIT.
**Homes read**: tools\ch_mcp\build_facts.py:69-80 `current_build` reads `Clone Hero_Data\version.json` (today "v1.1.0.6142-final"); build_facts.py:97-100 `same_build` "version.json writes a leading `v` and dump_facts.py's facts do not, so one is dropped from each"; dump_facts.py:27 and README.md:27 type `--version` by hand (dump_facts.py:994-995 "a dump.cs needs --version", since a dump.cs holds none). The "not attached" texts (frida_attach.py:439, frida_tools.py:61,97, kit_tools.py:1268) are messages for different states.
The typed value is the only input a dump has; the reader copes with the spelling difference in code.

### L-F17-16
**Verdict**: REAL.
**Homes read**: docs\agents\rulings\README.md:19-20 "`## Unconfirmed` holds the lines sessions copied from memory notes on 2026-10-31, untouched."; docs\agents\rulings\review-and-hooks.md:37-44 (`## Unconfirmed`) holds lines dated 2026-11-12 and 2026-11-06 (e.g. "Exempt git commit", "yes block those too"), which cannot be 2026-10-31 copies.
**Kind**: doc-vs-doc.
**What changes**: the README describes the section as a frozen 10-31 copy; it also holds later session-written lines, so a reader trusting the README treats newer lines as old untouched memory copies.

### L-F18-18
**Verdict**: REAL (the doc-vs-hook half only; the two literals agree).
**Homes read**: gates\review_rules.py:49 `REALAPP_FILE = "realapp.jsonl"` and tools\hydra_ui_mcp.py:81 `REALAPP_FILE = "realapp.jsonl"` (same value, two typed copies). docs\agents\ui-testing.md:11-13: case 1 "Behaviour and text. `hydra_uitest` ... takes the place of launching `Hydra.exe`", and case 2 asks for the real-app check only for "A layout, a width, spacing, alignment, or what the screen does when a song opens or closes". gates\README.md:55 "A move of Hydra's `main` that changes UI code needs a real-app check as well as its review"; review_rules.py:31 `UI_CODE = re.compile(r"(?i)^src/ui/")`, skipping only comment-only changes. ui-testing.md:235 does name the gate on "merges that change `src/ui/`".
**Kind**: doc-vs-code.
**What changes**: for a text-only change under src/ui, the page's three cases say the headless check is enough; the hook refuses the merge until a real-app record exists.

### L-F18-21
**Verdict**: NOT A SPLIT.
**Homes read**: docs\agents\derive-once-review.md:52 "D13 approved 'free', not the sentence 'no more than 0.0 ms late'"; docs\audit\2026-10-03-fix-decisions.md:35 "D302 reverses this", :1680 "This reverses D13".
The review page uses D13 as an example of a ruling not covering the wording built on it. It does not say squeezes at 0 are free today, so no reader of the page acts differently from a reader of D302.

### L-F18-22
**Verdict**: REAL.
**Homes read**: docs\agents\derive-once-review.md:57-64 "the ratio 1.1067 ... 158,224.76 + 1.1067 x (24.12 + 3.00) = 158,254.77 ms ... The game stored 158,268.99 ms, which is 14.21 ms away"; docs\clone-hero\05-star-power.md:437 "priced the end read after chord 466 with that step's own ratio, 1.3926 ... planned it at 158,262.53 ms. The game read 158,268.4 ms, 5.8 ms later". The run files both cite, hydra-data\ch-mcp-runs\2026-11-13\pitch-dark-ratios-prep\schedule.json, chords[466].reads: ratio 1.3926146644312014, planned_end_ms 158262.5305185332; pitch-dark-ratios-run1\press-1.csv:470 end read 158268.36. schedule.json holds no 1.1067, 158254 or 158268.99 (grep count 0).
**Kind**: doc-vs-doc.
**What changes**: the review page's worked example gives a ratio, a plan and a gap that the run's own files do not hold; a reviewer following it learns a gap of 14.21 ms for chord 466 where the star-power page and the data give 5.8 ms.

## Ids not reached
None.
