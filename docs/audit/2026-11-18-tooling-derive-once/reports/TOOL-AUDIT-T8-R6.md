# TOOL-AUDIT-T8-R6 (19 rows T7 reopened)

Paths: H = C:\Users\Patrick\Downloads\Hydra\hydra-test (main), G = C:\Users\Patrick\.claude\gates, C = C:\Users\Patrick\.claude, MEM = C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory.
Tally: REAL 12 (F06-16, F08-09, F14-01, F17-02, F18-14, F21-04, F21-09, F22-04, F22-09, F26-04, F27-27, F32-10). NOT A SPLIT 7 (F04-13, F04-18, F05-06, F09-12, F14-06, F25-16, F25-18). GONE 0. Not reached: none.

### L-F04-13
**Verdict:** NOT A SPLIT
**Homes read:** C\skills\reverse-engineer-anything\references\javascript-applications.md:35-39 "When the connected schema advertises `max_heap_mb` and `analysis_timeout_ms`, ... Defaults are 1024 MiB and 300000 ms ... plus a 512 MiB"; SKILL.md:186 "20 rows"; references\native-and-artifacts.md:33 "samples 20 rows". Searched C (json, py, md) for max_heap and analysis_timeout: only that one skill line. The REA server's own defaults are not readable here.
The skill's own copies agree. The other "side" is a third-party server that is not text in any file I can open. The skill itself says the connected schema is the owner.

### L-F04-18
**Verdict:** NOT A SPLIT
**Homes read:** MEM\no-full-library-runs.md:17 "builds over all 18,811 charts" (a quoted question of 2026-10-08); MEM\min-path-delta-proof.md:29 "Library scan (a one-off tool, not kept): 18,610 charts"; MEM\stored-versions-one-rule.md:11 "the user's 18,755 results went Stale" (the 1.8.3 event); MEM\measure-small-explain-first.md:11 "a 19,000-chart uitest script" (a rounded run). H\docs\agents\rulings\code-and-engine.md:27 "about 18,848 charts" and ui-and-display.md:20 "your 19,610 charts" are quoted question text.
Each figure is a dated count of the library on one day, in a story about that day. None says how many charts the library holds as a rule, so nothing answers one question two ways.

### L-F05-06
**Verdict:** NOT A SPLIT
**Homes read:** H\tools\ch_mcp\machine_report.py:49-52 "They sit in [video], so game_files.get_game_setting, which reads [game], cannot read them."; :250-263 `_settings_lines` prints every framerate/vsync line it finds, any section. H\tools\ch_mcp\game_files.py:217-253 `_game_setting_line`/`get_game_setting`: one line in [game] only, raises on none or several.
One reads two report keys from [video] for a printout. The other reads one setting from [game] and refuses duplicates. They never answer for the same key or section.

### L-F06-16
**Verdict:** REAL
**Homes read:** C\model-postures.md:11 "act at once: look at its transcript tail and worktree log, then nudge it once with SendMessage ... If it does not recover, stop it with TaskStop and re-dispatch". G\stall.py:122-123 "If an agent is stuck, stop it with TaskStop and re-dispatch its remaining work." (same text at :143-144 for the time-limit notice). G\stall.py:460-462 (CHECK_IN_HEADER) "open its transcript, nudge it or stop it."
**Kind:** doc-vs-code (model-postures.md against the stall notice text).
**What changes:** a main session that follows the doc sends one nudge first; one that follows the stall notice stops the agent at once. The check-in header, a third text in the same file, allows both.

### L-F08-09
**Verdict:** REAL (narrow)
**Homes read:** G\sync_settings.py:39 `LIVE = r"C:\Users\Patrick\.claude\settings.json"`. G\tests\test_docs_numbers.py:46 `SETTINGS = os.path.join(os.path.dirname(GATES_DIR), "settings.json")` with its comment "The live settings.json sits at the root of the main checkout only ... so a worktree has none and its check is skipped"; :142-146 `test_live_settings_carry_the_fragment` skips when SETTINGS is missing.
**Kind:** code-vs-code.
**What changes:** on main both name one file. In a gates worktree sync_settings.LIVE still names the real file, but the test's path does not exist, so the test skips instead of checking the live file. The difference shows only in a worktree, and only in whether that test runs.

### L-F09-12
**Verdict:** NOT A SPLIT
**Homes read:** H\docs\agents\ui-testing.md:195 "`snapshot_library` is the only code that knows the library folder's path." H\tools\hydra_ui_mcp.py:55 `DEFAULT_LIBRARY_DIR = r"C:\Program Files\Hydra"`, :140 "This is the only place that knows the library folder's path". H\tools\compare_db.py:51,87 and H\docs\agents\this-machine.md:77 name the path. Searched tools, src, scripts and *.ps1 for "Program Files\Hydra" and "Program Files/Hydra": only those three .py hits.
The claim is about the MCP helper's code, and the constant sits in that same module. compare_db.py names the path only inside its docstring recipe for a person to type; it reads nothing. this-machine.md is a doc saying where the file lives. No other program reads the folder.

### L-F14-01
**Verdict:** REAL
**Homes read:** H\docs\agents\agent-rules.md:14 "the checked-in chart corpus in `testdata/input` (about 115 charts)". H\docs\agents\ui-testing.md:48 "song folder = `testdata/input`, 98 charts". H\tests\test_engine_sweeps.cpp:656 `CHECK(charts == 98);`. Counted now: 98 song.ini files and 116 .chart/.mid files under H\testdata\input. MEM\questions-describe-the-real-rule.md:24 "Strip the `[N/97]` prefix" (what the batch tool prints; I did not run it, so 97 is unchecked).
**Kind:** doc-vs-code (agent-rules.md against the test); doc-vs-doc (agent-rules.md against ui-testing.md); memory-vs-other (the 97 in the note, if the tool prints 98).
**What changes:** someone sizing a test run from agent-rules.md plans for about 115 charts. Someone reading ui-testing.md or the test gets 98. The 116 files vs 98 folders is probably the source of the 115.

### L-F14-06
**Verdict:** NOT A SPLIT
**Homes read:** H\docs\agents\ui-testing.md:354 "The live window takes no input from the real mouse (the user, 2026-10-29: "my mouse doesnt work in it")."; H\docs\agents\rulings\ui-and-display.md:43 (under ## Confirmed, line 7) "the window is jsut in the library list and my mouse doesnt work in it". Searched src, tools\hydra_ui_mcp.py, tests\ui for mouse, NoMouse, real mouse: nothing states or denies it. H\src\ui\main.cpp:34-36 only says --live takes commands from the channel.
The doc and the quote agree on what the user saw. No file says the opposite, so there are not two answers. Whether it is a bug or intended is a product question, not a split.

### L-F17-02
**Verdict:** REAL
**Homes read:** H\docs\agents\clone-hero-mcp.md:29 "unless the user has typed exactly `allow key run` in chat". G\review_rules.py:50-55 `_KEY_ALLOW = re.compile(re.escape("allow key run") + r"[.!]?", re.IGNORECASE)` with the comment "matched the same way" as the library phrase (:50 "a trailing . or ! is allowed"). H\tools\ch_mcp\server.py:148,466,477 say "typed allow key run".
**Kind:** doc-vs-code.
**What changes:** read as written, the page says "Allow key run." does not count. The hook lets it through. Same edge as L-F17-01 (the library phrase).

### L-F18-14
**Verdict:** REAL (one piece; the rest agree)
**Homes read:** C\agents\reviewer.md:11 "at 100 calls or 20 minutes you stop fixing ... list every unfixed finding under a Notes heading in your report, each with the plan line it breaks (CLAUDE.md rule 5)". H\docs\agents\derive-once-review.md:110 "Write each finding you did not fix under Notes, with what is left to do." H\CLAUDE.md rule 5 holds no plan-line text. C\tools\session_cards\writer-brief.md:101 "At 20 minutes or 100 tool calls, stop starting new cards: finish the one in hand and report." G\progress.py:14-15 (100/150, 20/30), :92-100 warn "Start wrapping up", :129 refuse.
The writer-brief and the hook use the same numbers; one says stop starting work, the other warns first and refuses later. That is wording, not a conflict. "100 calls" vs "100 tool calls" is wording too.
**Kind:** doc-vs-doc (reviewer.md vs derive-once-review.md); moved-pointer (reviewer.md cites CLAUDE.md rule 5 for a plan-line requirement rule 5 does not hold).
**What changes:** a reviewer following reviewer.md writes each unfixed finding with the plan line; one following derive-once-review.md writes only what is left to do. No program reads either.

### L-F21-04
**Verdict:** REAL
**Homes read:** G\stall.py:280-281 "A move of main to a tip with no verdict is refused." G\review_rules.py:419-432 `explain_main_move`: returns "no-gate" when no code or doc marker is on either rev, "no-review-needed" when old == new or new is zero; other allows "verdict", "carry", "waiver".
**Kind:** code-vs-code (message string in one hook against the decision in another).
**What changes:** a main session trusting the message thinks every unreviewed move of main is refused. In a repo with no marker, or a move needing no review, it is allowed.

### L-F21-09
**Verdict:** REAL (one piece only)
**Homes read:** H\docs\audit\2026-10-03-fix-decisions.md:153 names "`tools/derive_once_precheck.ps1`: helper bodies under 30 tokens are matched by name only ...". `ls H\tools | grep -i -E "precheck|derive"` finds only the folder `tools\precheck` (C++ files check_1..check_5, etc.). G\ref_judge.py:42 holds `WATCHDOG = 60` per T7, not re-read by me. The picker limits, check kinds and step 8 agree per T7; I did not recompute them.
**Kind:** moved-pointer (D49 names a .ps1 that is gone; the checks now live under tools\precheck).
**What changes:** a reader following D49 looks for a file that is not there. The rest of the row (preamble vs pushed case, hand-listed known copies, "never blocks") I did not show to be a split; they are not reached.

### L-F22-04
**Verdict:** REAL
**Homes read:** G\lib\hook_input.ps1:1 "the one place a hook reads its input", :2-6 (UTF-8, drops a leading byte-order mark). G\common.py:59 `sys.stdin.buffer`, G\wake.py:92 `sys.stdin.buffer`, G\ref_judge.py:510 `sys.stdin.buffer.read().decode("utf-8","replace")` (git lines, other input). Only the PowerShell reader drops the BOM.
**Kind:** doc-vs-code (a comment against the Python hooks).
**What changes:** someone fixing input decoding in the "one place" fixes only PowerShell hooks; Python hooks read and decode on their own.

### L-F22-09
**Verdict:** REAL (one piece: D115)
**Homes read:** C\tools\session_cards\writer-brief.md:78 "D115 is a name with no entry, so never cite it". H\docs\audit\2026-10-03-fix-decisions.md:704 "**D115 (one record per chart and profile), 2026-10-11 to 2026-10-15.**" (matches check.py's D-number entry pattern per T7, common.py:92). The "2 to 6" tags (writer-brief.md:77) and "0 to 8" lines (:85) are brief-only guidance with no other home.
**Kind:** doc-vs-code (brief against the decision log the check reads).
**What changes:** the brief tells a card writer not to cite D115; the check accepts it because the log has an entry.

### L-F25-16
**Verdict:** NOT A SPLIT
**Homes read:** H\docs\agents\ui-testing.md:237-253 `commands` = "how many command lines Hydra answered ... timeout lines of a one-call timeout included"; "The record shows that a window was opened and used on that commit". G\review_rules.py:405 requires dirty False, commands int >= 1, commit on the branch.
The page defines what is counted and what the record shows. It states no minimum. The gate sets one. Two statements, no contradiction. Whether one command is strong enough is a gate-strength choice that the page does not take a side on (L-F27-18 frames the same point).

### L-F25-18
**Verdict:** NOT A SPLIT
**Homes read:** H\docs\agents\clone-hero-mcp.md:53 "The chart for the same-pad question is `C:\Clone Hero\songs\Hydra Probe\Edge Walk Pair`, Edge Walk with one more kick 100 ms after its last one." H\tools\ch_probe\probe_songs.py:63-68 `DEFAULT_OUT`, `EDGE_WALK = "Edge Walk"`, `NAME_PREFIX = "Hydra Probe - "`; SONGS (line 338) lists no Pair song. H\tools\ch_probe\tests\edge_walk_pair_schedule.json:1 uses the same "Edge Walk Pair" path. The folder exists on disk (`ls "C:\Clone Hero\songs\Hydra Probe"`). Searched tools and docs for "Walk Pair".
The doc and the test schedule agree. probe_songs.py's "Hydra Probe - Edge Walk" is the listed song name for the plain Edge Walk folder, a different chart. No repo program writes the Pair folder, but nothing says otherwise.

### L-F26-04
**Verdict:** REAL
**Homes read:** MEM\relay-status-in-plain-english.md "Skip lines from test fixtures (names starting `test_`)" and "a second sentence only when the evidence disagrees". G\stall.py:30-33 STATUS_HEADER "When the evidence disagrees, add one sentence that says so" (no skip rule). C\model-postures.md:11 "add a second sentence only when the evidence disagrees" (no skip rule). MEM\MEMORY.md:46 "one plain sentence per agent". Grepped stall.py, common.py, progress.py, wake.py, agent_stop.py for `test_` and fixture filters: none.
**Kind:** memory-vs-other (the `test_` skip has no hook or doc line); homeless-rule (the skip lives only in the note). The second sentence matches the hook, so that part is not a split; the MEMORY.md index line is a shorter note-against-note.
**What changes:** a session following the note drops `test_` agents' lines from the relay; one following the hook or model-postures relays every agent.

### L-F27-27
**Verdict:** REAL
**Homes read:** H\tools\ch_probe\SPEC.md:104-108 "the originating session did read the resolved doubles ... (e.g. `0x0313ECB0 = 0.0110924370`, the linear coefficient ...). Those resolved reads were never saved to disk"; :88 `window = (0.0110924*t - 2.628539e-05*t^2)*170 - 20`. H\tools\ch_probe\constants.py:175-180 `"c1": 0x3140738` (first formula constant, normal branch); H\docs\clone-hero\code\6142\hit_window_formula_20DDDA0.asm:41 reads `[0x3140738] dbl=-0.04`, :46 `[0x31406f8] dbl=12.4805915`. 0x0313ECB0 appears in SPEC.md only (searched tools and docs\clone-hero).
**Kind:** doc-vs-code.
**What changes:** SPEC.md puts the linear coefficient at one address with 0.0111; constants.py and the 6142 disassembly put the first formula constant at another address with -0.04. Whoever re-reads the coefficient live uses a different address depending on the side. The two may come from different builds; SPEC.md says the value was never saved.

### L-F32-10
**Verdict:** REAL
**Homes read:** MEM\relay-status-in-plain-english.md "The user chose this on 2026-11-16 ("Claim, plus disagreement")". H\docs\sessions\2026-11-16-e816e44f.md:24 `chose "Claim, plus disagreement" to "Relay wording: your filed words are ..." (23:19 UTC)`. H\docs\agents\rulings\agents-and-workflow.md: `## Confirmed` (line 7) holds only :26 "the status updates should be brief but relayed in plain english"; grep for "Claim, plus" across H\docs\agents\rulings finds nothing. The `test_` skip has no home (see L-F26-04).
**Kind:** claimed-ruling (the note says the user chose; the card records a click, no Confirmed line); memory-vs-other (the `test_` skip).
**What changes:** the note and model-postures.md act on a choice that no filed ruling backs. The 2026-09-05 filed words say only "brief but relayed in plain english".

## Not reached
None.
