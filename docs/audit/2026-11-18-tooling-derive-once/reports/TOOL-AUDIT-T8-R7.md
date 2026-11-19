# TOOL-AUDIT-T8-R7 (L-F21-09, the pieces nobody had read)

Paths: G = C:\Users\Patrick\.claude\gates, H = C:\Users\Patrick\Downloads\Hydra\hydra-test (main). The scout wrote only a stub file and gave its full report in its hand-back; the main session copied it here and spot-checked ref_judge.py:42, check_3.cpp:427-432 and definitions.h:20,25 against it.
Tally: GONE 1 (WATCHDOG), NOT A SPLIT 5. Not reached: none.

### L-F21-09 (WATCHDOG value)
**Verdict:** GONE
**Homes read:** G\ref_judge.py:42 `WATCHDOG = 60`, used at :502 and :505 (the "the judge ran over %d seconds" message and `threading.Timer(WATCHDOG, ...)`). A grep of ref_gate.sh, install_ref_gate.ps1, README.md and test_ref_judge.py for watchdog, "60 sec", seconds and timeout finds no second copy. The ledger's copy at ref_gate.sh:15-17 and install_ref_gate.ps1:13 is gone: ref_gate.sh:15-17 now holds the verdict check and its message, with no number. tests\test_ref_judge.py:420 only sets `ref_judge.WATCHDOG = 1` to force a timeout. Hydra's docs/agents, CLAUDE.md and development.md state no value.
**What changed:** the number lives in one place and nothing repeats it.

### L-F21-09 (preamble's summary of the ref gate)
**Verdict:** NOT A SPLIT
**Homes read:** H\docs\agents\brief-preamble.md:65 the gate "refuses any new commit that lacks a Task:, Agent: or Session: trailer", except merges; :71 it "refuses a move of main, or of any branch origin also has, that is not a plain step forward". G\ref_judge.py:99-103 (review lock), :135 and :169-171 (a non-descendant move refused on a shared branch or under open review), `judge_push_lines` :184-205 (the pushed case: a push whose new tip does not descend from the remote tip is refused with the same wording; deleting main on a remote is refused; the main-review rule is skipped when local main already passed it).
**Why:** both give the same rule. The preamble is a summary that leaves out the delete-main refusal, the review lock and the skip step; none contradicts it. The preamble also tells agents never to push.

### L-F21-09 (check_1 and check_2 hand-listed known copies)
**Verdict:** NOT A SPLIT
**Homes read:** H\tools\precheck\check_1.cpp:42-57 (the MIDI row "matches only the chunk tags ... when a row for it lands, this entry goes"); H\tests\single_owner\rules\tests.json:245-262 (the row exists; its pattern matches the chunk tags only). H\tools\precheck\check_2.cpp:52-61 (the two-hit budget row "does not read tests/ and does not match a bare 2.0"); H\tests\single_owner\rules\core.json:1047-1075 (no scope), H\tests\single_owner\engine.cpp:300 (an empty scope means src and tools only); its pattern needs a `*` product.
**Why:** each comment describes what its row covers today, and the row agrees. "This entry goes when the row lands" has no check behind it: a missing check, not a split. The "No row" notes at check_2.cpp:67 and :73 are also true.

### L-F21-09 (the "never blocks" registration)
**Verdict:** NOT A SPLIT
**Homes read:** G\agent_stop.py:12 "This hook only informs: it never blocks"; G\stall.py:4-8; posture.py's header; waiver.py:6; G\README.md:29. G\settings.fragment.json:207-219 registers SubagentStop for agent_stop.py with `"timeout": 20` and no `onFailure`; the blocking hooks carry `"onFailure": "block"` at :153, :169, :185. agent_stop.py:454 runs through `common.run(body, "loud")`, and common.py:55 says a loud crash prints a systemMessage and returns 0. tests\test_docs_numbers.py:178 pins no `onFailure` for the output-trim hook; no test does so for agent_stop.
**Why:** the registration and the code agree the hook never blocks. No test pins it for agent_stop.

### L-F21-09 (CODE_BRIEF and DOC_BRIEF paths)
**Verdict:** NOT A SPLIT
**Homes read:** G\review_rules.py:20 `CODE_BRIEF, DOC_BRIEF = "docs/agents/derive-once-review.md", "docs/agents/doc-review.md"`; both files exist in H\docs\agents\; H\CLAUDE.md names the same pages; doc-review.md:21 points at derive-once-review.md; agent_stop.py:270 tests `DOC_BRIEF in brief`.
**Why:** the paths match the real files. A rename of either page would break the hook. Line 435's `hydra_start('window')` mention is outside the six pieces and was not checked.

### L-F21-09 (D49 number lists in check_2 and check_3)
**Verdict:** NOT A SPLIT
**Homes read:** H\docs\audit\2026-10-03-fix-decisions.md:151-153 (D49: bodies under 30 tokens; a statement over up to 4 lines; 2-line reach for the typed 2.0; a cited decision on the line or the 3 lines above; never list 0, 1, 2; unit factors 1000, 1024, 1000000; 200 characters look-back). H\tools\precheck\definitions.h:20 `kBodyTokenFloor = 30`, :25 `kNamespaceLookBack = 200`; check_2.cpp:123-131 (`ln + 3`), :138-142 (2 lines either side); check_3.cpp:427-432 (`skip = {"0","1","2"}`, `unit_factors = {"1000","1024","1000000"}`), :490 "this line or the three above (D49)".
**Why:** every D49 number equals the code's, and each code comment cites D49. check_3.cpp:427-431 restates the numbers rather than only pointing at D49, which risks drift but is not a split today.
