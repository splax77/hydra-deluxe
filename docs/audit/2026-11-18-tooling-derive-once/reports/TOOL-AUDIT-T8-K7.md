# TOOL-AUDIT-T8-K7

H = C:\Users\Patrick\Downloads\Hydra\hydra-test, G = C:\Users\Patrick\.claude\gates. Tally: PASS 8, REOPEN 0, GONE 1 (agreeing with R7). Not reached: none. I ran nothing; I read files and searched only.

### L-F09-12
**Verdict:** PASS
**Homes read:** H\docs\agents\ui-testing.md:193-195 (library folder is library_dir, else HYDRA_LIBRARY_DIR, else DEFAULT_LIBRARY_DIR in the helper; line 195 is a long line that says snapshot_library is the only code that knows the path). H\tools\hydra_ui_mcp.py:55 `DEFAULT_LIBRARY_DIR = r"C:\Program Files\Hydra"`, :140 "This is the only place that knows the library folder's path", :147 reads it. H\tools\bench.cpp:974-988: `installed_folders()` ends with `out.push_back("C:\\Program Files\\Hydra")`, and `refused_db` (:991-999) uses the list to refuse `--open` on a db inside the installed folder. H\tools\compare_db.py:51 and :87 (path in a docstring recipe a person types, nothing reads it).
**Search:** the Grep tool over src, tools, tests, scripts for "Program Files.{1,3}Hydra", all of .cpp .h .py .ps1. Hits: bench.cpp:986, compare_db.py:51 and :87, hydra_ui_mcp.py:55, tests\test_launch.cpp:31,35,38 (a launch-test fixture naming HydraPathBuilder.exe, not the library) and tests\test_live_protocol.cpp:64 (a negative example string for is_under_temp). No src or scripts hit.
**Why:** the doc claim "the only code that knows the path" is false: bench.cpp's decision about which folder `--open` refuses holds the same literal. That is doc-vs-code and the code wins, so the doc line is corrected. The two code copies (hydra_ui_mcp.py:55 and bench.cpp:986) answer one question, where the installed library lives, with two literals; bench.cpp also derives it from the ProgramFiles variables and the installer comment (hydra.iss). That is code-vs-code, "Merge, I pick". Fixing the doc does not open any guard gap. compare_db.py is only prose inside a docstring and does not add a third split. No split is missed by the two kinds.

### L-F26-04
**Verdict:** PASS
**Homes read:** memory relay-status-in-plain-english.md:15 ("Skip lines from test fixtures (names starting `test_`)"). G\stall.py:259-280 `_status`: it relays the status files of the agents in the session's agent records, with `common.status_path(aid, root)` under `root or STATE_ROOT`; no `test_` check is in stall.py (grep of stall.py for "test_" finds nothing). G\common.py:10 STATE_ROOT is the one live state folder. G\status_append.ps1:10-12 honours HOOKS_TEST_STATE_ROOT. G\tests\test_status_append.ps1:6-7, :52, :67 and the last line `Assert-LiveStateUntouched`: the test runs against a temp state folder and checks the live one is untouched. G\tests\replay.py:205-207 repoints STATE_ROOT to a temp folder. G\tests\test_ref_judge.py:616 and test_review_dispatch.py:644 patch STATE_ROOT to temp folders. G\tests\test_common.py:159 only asserts the value.
**Searches:** `ls state/status` has 860-odd files, none starting `test` (0 on a case-insensitive count of `^test_`; also none containing "test" in the name). All names are agent ids (a + 16 hex) or T-task names. The fixture agent names in the hook tests are `test-status-append-agent` and `test-status-append-norecord` (hyphens, in a temp folder). A grep of G\tests\*.py for quoted "test_..." ids found none.
**Why:** a hook-test agent's line cannot reach the relay: tests write to temp state, the live-state check guards that, the relay reads only agents a session recorded, and the fixture names do not even begin with `test_`. The rule is a filter that filters nothing anyone sees, so pruning it drops no rule. It is held by no doc or hook, but it governs nothing. The note's closure as "Note is wrong" holds.

### L-F32-10 (the test_ half)
**Verdict:** PASS
**Homes read:** the same as L-F26-04 (the exit is the same note line). The claimed-ruling half was K6 PASS and is not re-checked here.
**Why:** same as L-F26-04.

### L-F21-09 (WATCHDOG value)
**Verdict:** GONE (agree with R7)
**Homes read:** G\ref_judge.py:42 `WATCHDOG = 60`, used at :502 and :505. G\ref_gate.sh:10-18 has the verdict check and a message with no number. Grep of G for "WATCHDOG" finds only ref_judge.py and tests\test_ref_judge.py:420 (sets it to 1 to force a timeout). Grep of ref_gate.sh, install_ref_gate.ps1 and README.md for watchdog, "60 s", "sixty" finds nothing. Grep of H\docs\agents\*.md and CLAUDE.md finds no judge-time number.
**Why:** one number, one place, no second copy.

### L-F21-09 (preamble's summary of the ref gate)
**Verdict:** PASS (NOT A SPLIT)
**Homes read:** H\docs\agents\brief-preamble.md:65 and :71. G\ref_judge.py:41 `TRAILER_KEYS = ("Task","Agent","Session")`, :130-136 (shared = main or origin has it; a non-ancestor move on a shared or reviewed branch gets rewrite_reason), :184-205 `judge_push_lines` (a push that is not a descendant is refused; deleting main refused; the main rule skipped when local main passed).
**Why:** the preamble is a shorter statement of the same rule; nothing it says contradicts the judge.

### L-F21-09 (check_1 and check_2 hand-listed known copies)
**Verdict:** PASS (NOT A SPLIT)
**Homes read:** H\tools\precheck\check_1.cpp:42-57 (MIDI varlen entry, "matches only the chunk tags"); H\tests\single_owner\rules\tests.json:245 ("Which test helper writes MThd/MTrk chunks?"); H\tools\precheck\check_2.cpp:52-61 (two-hit budget entry, "does not read tests/ and does not match a bare 2.0"); H\tests\single_owner\rules\core.json:1047-1075 (the row; no `scope` line inside it, grep of that range finds none); H\tests\single_owner\engine.cpp:299-303 (an empty scope means src and tools only).
**Why:** each comment says what its row covers today and the row agrees. "When a row lands, this entry goes" is a to-do with no check behind it; that is a missing check, not two answers.

### L-F21-09 (the "never blocks" registration)
**Verdict:** PASS (NOT A SPLIT)
**Homes read:** G\agent_stop.py:12 "This hook only informs: it never blocks a hand-back"; G\settings.fragment.json:207-220 SubagentStop for agent_stop.py with `"timeout": 20`, no `onFailure` (blocking hooks carry `"onFailure": "block"`, per R7 at :153,:169,:185; I did not reopen those lines).
**Why:** the code and the registration agree that it does not block. No test pins this for agent_stop; a missing test, not a split.

### L-F21-09 (CODE_BRIEF and DOC_BRIEF paths)
**Verdict:** PASS (NOT A SPLIT)
**Homes read:** G\review_rules.py:20 names docs/agents/derive-once-review.md and docs/agents/doc-review.md; both files exist in H\docs\agents\ (listed). H\CLAUDE.md names the same two pages.
**Why:** the paths match real files. The hook would break if a page were renamed, but today they agree.

### L-F21-09 (D49 number lists in check_2 and check_3)
**Verdict:** PASS (NOT A SPLIT)
**Homes read:** H\docs\audit\2026-10-03-fix-decisions.md:151-153 (D49: under 30 tokens; up to 4 lines; 2-line reach; decision cited on the line or the 3 lines above). H\tools\precheck\definitions.h:20 `kBodyTokenFloor = 30`, :25 `kNamespaceLookBack = 200`. H\tools\precheck\check_2.cpp:123-131 (up to 4 lines, `ln + 3`), :138-142 (2 lines either side). H\tools\precheck\check_3.cpp:425-431 (`skip = {"0","1","2"}`, `unit_factors = {"1000","1024","1000000"}`, both citing D49) and :490 ("this line or the three above (D49)").
**Why:** every number matches D49 and each code comment cites it. I did not read the D49 lines for the 0/1/2 and unit-factor lists or the 200 look-back in the decision text (the lines I read at :151-153 do not show them; R7 cites them there as well). Those three numbers are unchecked against the decision, though the code comments cite D49 for them. If this matters, the main session reads the rest of D49 before leaving the closure.
