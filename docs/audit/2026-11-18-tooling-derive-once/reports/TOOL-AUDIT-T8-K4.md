# TOOL-AUDIT-T8-K4 closure check (18 ids, all reached)

Result: PASS 17, REOPEN 1 (L-F25-19), GONE confirmed 1 (L-F20-06, counted in the 17? no: PASS 16 + GONE 1 + REOPEN 1). Exact: PASS 16, GONE 1, REOPEN 1. Paths: H = C:\Users\Patrick\Downloads\Hydra\hydra-test, G = C:\Users\Patrick\.claude\gates, M = memory folder.

### L-F20-01
**Verdict**: PASS (weak)
**Homes read**: H/docs/agents/ui-testing.md:46 "The 98 test charts", :48 "98 charts"; H/docs/agents/agent-rules.md:14 "testdata/input (about 115 charts)"; H/tests/test_engine_sweeps.cpp:656 `CHECK(charts == 98)`; H/tests/ui/uitest_library.cpp:549 `shown() == 98`. find under testdata/input finds 116 .chart/.mid files in 6 top-level entries.
**Why**: Two docs give two counts for one corpus. Both docs: "Merge, I pick" holds. Which is right (98 loaded charts vs about 115 files) is for the fix plan.

### L-F20-04
**Verdict**: PASS (weak)
**Homes read**: ~/.claude/skills/analyzing-sp-squeezes/SKILL.md:55 "a run that beats the engine's optimal is a version problem or a bug, never a real score"; H/docs/agents/public-doc-style.md:17 "Rows above optimal are expected: many leaderboard scores were set on older Clone Hero versions"; M/dmleaderboards-comparison.md:24-27 "Above-optimal scores are NORMAL, not a bug".
**Why**: The skill judges one watched video run on a known version; the style page and note word leaderboard rows of unknown version. Both name the old-version cause. Different jobs, so exit 4 holds. A reader could still see "bug" against "not a bug", so the main session may want one sentence on the skill line.

### L-F20-06
**Verdict**: GONE confirmed
**Homes read**: G/agent_stop.py:225-227 `documents(git_dir, base, tip)` "base, the start the dispatch recorded"; G/README.md:51 "from the `base` the handover recorded to the new tip; it never asks git about `main`"; M/hooks-never-guess.md:13 describes the old merge-base start as the fault. Other merge-base calls (agent_stop.py:205, :212) are branch ancestry checks.
**Why**: Code, README and note now agree on the recorded base.

### L-F20-07
**Verdict**: PASS
**Homes read**: H/docs/agents/test-rubric-examples.md:9 "The Preview fix those four asked for has since landed. The Preview's own refill-tick function is gone"; same file :332-336 "It cannot hold one today ... the gauge disagrees with the record today ... the test to write in the same change as the Preview fix ... Today it would fail on Pitch Dark". grep of src, tests, tools finds no `refill_tick`.
**Why**: Same page says the fix landed and also that it is still to come; and the code has no such function. Both kinds covered: doc-vs-doc (Merge, I pick) and doc-vs-code (the code wins; the "would fail today" text is stale against code).

### L-F21-04
**Verdict**: PASS
**Homes read**: G/stall.py:294-295 "A move of main to a tip with no verdict is refused."; G/review_rules.py:419-431 `explain_main_move` returns "no-review-needed" when old == new or new is zero, "no-gate" when no marker is on either rev.
**Why**: The message states a rule wider than the deciding function. Code-vs-code, so "Merge, I pick" holds (the message is printed text, but it sits in a program file, not a doc).

### L-F21-09 (D49 missing-file piece only)
**Verdict**: PASS for that piece
**Homes read**: H/docs/audit/2026-10-03-fix-decisions.md:153 names `tools/derive_once_precheck.ps1`; find over H and ~/.claude finds no file by that name; git log shows it deleted in commit db87b55c "Delete the PowerShell precheck and its self-test"; H/tools/precheck/ holds the C++ checks (check_1..check_5, checks.cpp, decisions.cpp).
**Why**: The pointer moved to tools/precheck. "Fix without asking" holds for this piece. Note: it is a decision-log line, so the main session should say how to read it now (removed; git show db87b55c^:tools/derive_once_precheck.ps1) rather than rewrite history.

### L-F22-04
**Verdict**: PASS
**Homes read**: G/lib/hook_input.ps1:1 "the one place a hook reads its input", :6 "decodes them as UTF-8, dropping a leading byte-order mark"; G/common.py:59,64 `sys.stdin.buffer` then `.decode("utf-8")` (no BOM drop); G/wake.py:92; G/ref_judge.py:510.
**Why**: Comment claims one place; Python hooks read and decode on their own. Code decides, no guard gap.

### L-F22-09
**Verdict**: PASS
**Homes read**: ~/.claude/tools/session_cards/writer-brief.md (rule 4) "D115 is a name with no entry, so never cite it"; H/docs/audit/2026-10-03-fix-decisions.md:704 "**D115 (one record per chart and profile)..."; ~/.claude/tools/session_cards/common.py:92 `DECISION_ENTRY = re.compile(r"^\*\*D(\d+)\b", re.M)`.
**Why**: The check accepts D115 because the log has the entry; the brief is wrong. Code wins.

### L-F22-13
**Verdict**: PASS
**Homes read**: H/.derive-once-gate:2-3 "decided by Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1)"; ~/.claude/hooks holds only `state`; grep over ~/.claude (py, ps1, md, json) finds the names only in archived memory, old cards and H/.superpowers progress notes; G/review_rules.py:21-22 `BUILTIN_CODE`, used when the marker has no `gate:` lines.
**Why**: The pointer names a function and file that are gone; the rule is now `BUILTIN_CODE` in review_rules.py. Fix without asking holds.

### L-F22-24
**Verdict**: PASS (weak)
**Homes read**: ~/.claude/tools/session_cards/writer-brief.md:100 (full `status_append.ps1 <your agent id> "HH:MM done <n> of <m> cards | next: <session>"` command); ~/.claude/model-postures.md:11 "A brief names no status file and no id; it carries only the preamble's status rule."; H/docs/agents/brief-preamble.md:15.
**Why**: The brief file exists (the R2 miss is confirmed wrong). Two docs word what a brief says about the status line differently. The conflict is thin: the brief names the helper, not a status file, and leaves the id to the progress hook. Doc-vs-doc "Merge, I pick" holds, but it is weaker than the fold note suggests.

### L-F24-09
**Verdict**: PASS
**Homes read**: H/docs/agents/derive-once-review.md:126-140 report sections (Questions this change answers, Findings, Visible changes, Precheck lines dropped, Notes, Touched, Scan rows added, Where I looked); H/docs/agents/doc-review.md:110-113 (Restatement, Findings, Numbers checked, References opened, ...); derive-once-review.md:90 "you follow that page for every document in the range".
**Why**: Neither page says which list a combined report uses. Doc-vs-doc.

### L-F24-11
**Verdict**: PASS (weak)
**Homes read**: H/docs/agents/audit-method.md:50 "a table of inputs and answers for each home, with the edges in. Edges are values like an empty list, exactly 0 ..."; briefs/T5-finder.md:33-41 "a table of what each home tells its reader to do, with these edges in: exactly at the limit, the main session against an agent ...".
**Why**: Method page speaks of finders across the code; the T5 brief is the finder brief for a docs-and-hooks audit and its edges fit docs. Different homes, different edges. Exit 4 holds, though audit-method.md is the page every audit brief follows, so the main session may prefer a one-line pointer in it.

### L-F24-12
**Verdict**: PASS
**Homes read**: H/docs/agents/brief-preamble.md:3 (brief names task id, session id, owned files, worktree, scratch folder, tests); ~/.claude/model-postures.md:5 (task id, session id, files, tests, status rule; no worktree or scratch) and :11; H/docs/agents/audit-method.md:120 (id, session id, files or batch, caps, status rule); M/every-brief-carries-status-rule.md (description says "the dispatch gate now denies briefs that lack it", body says no gate checks).
**Why**: Three docs list three different brief contents (doc-vs-doc). The note's rule "every brief carries the status rule" is held by the preamble, model-postures:5 and audit-method:120, so pruning the note's wrong gate claim drops no rule. Both kinds hold.

### L-F25-16
**Verdict**: PASS (weak)
**Homes read**: H/docs/agents/ui-testing.md:246 `commands` "how many command lines Hydra answered ... timeout lines of a one-call timeout included", and the limit paragraph "shows that a window was opened and used on that commit"; G/review_rules.py:405 requires `dirty` False, `commands` an int >= 1, commit on the branch.
**Why**: The page defines the field and states no minimum; the gate sets one. No contradiction. Gap to flag, not a split: a session with a single timeout line passes the gate while the page calls the record "opened and used". That is a gate-strength question (see L-F27-18), not this row.

### L-F25-18
**Verdict**: PASS
**Homes read**: H/docs/agents/clone-hero-mcp.md:53 names `C:\Clone Hero\songs\Hydra Probe\Edge Walk Pair`; H/tools/ch_probe/tests/edge_walk_pair_schedule.json:1 uses the same path; H/tools/ch_probe/probe_songs.py:63-68 `EDGE_WALK = "Edge Walk"`, `NAME_PREFIX = "Hydra Probe - "`; the folder exists on disk (ls of C:\Clone Hero\songs\Hydra Probe lists Edge Walk and Edge Walk Pair). Searched tools and docs for "Walk Pair": hits are the schedule, the doc, docs/audit/2026-11-12-mid-no-pro/make_probes.py (reads the folder's song.ogg) and plans.
**Why**: Doc and schedule agree and the folder exists. The audit row (exits.md:487) compared the doc to the plain Edge Walk name, a different chart. No program writes the Pair folder, but nothing claims one does.

### L-F25-19
**Verdict**: REOPEN (weak)
**Homes read**: H/tools/ch_mcp/dump_facts.py:381 `DEFAULT_BUILD = "6142"`, used :643 for a page with no build heading; H/tests/test_docs_match_code.cpp:730-735 comment "else 1.1; tools/ch_mcp/dump_facts.py --tsv picks the same" then `kBuild11 = "6142"`, and :778 "else 6142"; H/tools/ch_mcp/build_facts.py:69-81 `current_build` (installed version); H/tools/ch_mcp/launch_patch.json:3 "1.1.0.6142".
**Why**: R5 is right that the installed-build reader and the patch file answer other questions. But the dump tool and the docs test answer the same question ("which build does a facts span with no build heading mean") with two separate "6142" literals, and the test's own comment says it must "pick the same". No test holds them equal (grep of DEFAULT_BUILD in tests finds none). They agree today, so this is a code-vs-code copy with nothing to keep it true, not exit 4. Should be "Merge, I pick" (code-vs-code), or fold into the first-line.

### L-F26-02
**Verdict**: PASS
**Homes read**: H/docs/agents/audit-method.md:97 "Give one example from each end, and one from the gap where the two sides disagree."; H/docs/agents/doc-review.md:61 "Each option has a concrete example from each end."; M/questions-describe-the-real-rule.md:16 "one example from each end of its range".
**Why**: Doc-vs-doc on the gap example. The note's rule is held in both docs, so "Note is wrong" holds for the memory side.

### L-F26-13
**Verdict**: PASS (weak)
**Homes read**: M/comments-explain-current-code.md "Code comments explain the current code: what a constant or rule means and who reads it. They never tell its history."; H/docs/agents/brief-preamble.md:119 "A comment names where a rule lives; it never restates the rule. ... Comments may say why a rule exists, or what it deliberately leaves out"; H/docs/agents/rulings/code-and-engine.md:22 (Confirmed) "comments exist to explain the current code, not give a history lesson". Grep of docs/agents for "history" near "comment" finds no instruction page carrying the no-history rule.
**Why**: The note and preamble differ on whether a comment states what a constant means. The Confirmed ruling backs only the no-history half, and no agent-facing page or hook states it. So the fix must keep the no-history half (give it a home, or leave it in the note); pruning the note whole would drop an agent rule. Main session should word the fix as correcting the "what a constant means" half only.

## Ids not reached
None.
