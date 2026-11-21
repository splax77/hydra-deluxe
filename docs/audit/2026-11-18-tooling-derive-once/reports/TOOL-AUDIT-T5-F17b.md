# TOOL-AUDIT-T5-F17b (re-send of F17)

Abbreviations: GR = C:\Users\Patrick\.claude\gates; HR = C:\Users\Patrick\Downloads\Hydra\hydra-test; LS = C:\Users\Patrick\.claude\settings.json (live); FR = GR\settings.fragment.json; PG = GR\publish_gate.py; RR = GR\review_rules.py.

Rows taken (from F17 line 174, "Rows not reached with full search"): I3-060, I3-065 (the live-settings side F17 skipped), I3-063, I3-045 (F17 searched these only by name and phrase). The unopened home was the live settings.json. F17's own findings on these rows (Q10, Q12, Q15) are not repeated except where I add to them.

Confirmed rulings that speak: review-and-hooks.md:22 "opus, retire, retire, minutes only, only large items over 20 lines should go for review, retire..." (the 20 lines only; no Confirmed line gives 2000 characters). agents-and-workflow.md:18 "agents should fail loudly. freezing is unacceptable". review-and-hooks.md:27 (every published document gets a fresh-agent review). All read in F17's report; I did not re-open the ruling files.

## I3-060 Hook failure mode: what a crash does
Question: when a hook crashes or cannot start, does the call stop, or only a message print?
Homes:
- PG:135 `common.run(check, "closed")`; GR\delete_rule.py:187, percall.py:176, review_dispatch.py:178 also "closed". Informing hooks use "loud": agent_stop.py:297, posture.py:55, prompt.py:67, shell_output.py:201, waiver.py:50.
- LS lines 163, 179, 195 and FR lines 153, 169, 185: `"onFailure": "block"` on percall.py (matcher `*`), review_dispatch.py (`Agent`), publish_gate.py (`Artifact|ExitPlanMode`). No other registration carries it; live hook list read in full.
- GR\README.md:95-100: "closed" is for hooks that can refuse, "loud" for hooks that only inform; "The refusing hooks carry `onFailure: block` in their registration for that case." README:190: "A hook with `onFailure: block` whose program is missing blocks the call."
- GR\tests\test_docs_numbers.py:163 asserts the shell_output registration has no `onFailure` ("the trim only informs, so a failed start must not block a call").
What each home tells its reader: the mode in code and `onFailure` in the registration are two separate settings for one rule ("a refusing hook fails closed"). Code mode covers a crash inside the script; `onFailure` covers a script that cannot start. README:100 states the pairing in prose.
Edges: at the limit n/a. Main vs agent: all of these hooks run for both; none checks agent type here. Card writers: percall.py is registered on `*`, so it runs for them too. Worktree vs main: the registration is user-level, not per checkout.
Kinds: K6-adjacent / K1. The "refusing" list is stated three times: code mode strings (7 places), registration `onFailure` (3 places), README:97-100. They agree today (percall, review_dispatch, publish_gate carry both; delete_rule is "closed" but is a library called by percall, so it needs no registration of its own). Nothing holds the code-mode list and the `onFailure` list equal; the only test is the one negative check at test_docs_numbers.py:163, for shell_output only.
Splits: none found. Live LS and FR hook lists are identical in event, matcher, program, timeout and onFailure (compared all 11 entries).
Tests that hold homes equal: GR\tests\test_sync_settings.py compares a temp live copy against a temp fragment (its docstring says the real live file is never read), so no test compares the real LS to FR. GR\sync_settings.py:1-12 is the owner of how FR reaches LS ("Which part of the live file the fragment owns ... `merged` and `first_difference`"); the script exits 1 with the difference when out of sync. LS and FR are two copies of one registration (K1, equal today).

## I3-065 Which tool calls are gated by publish_gate
Question: which tool calls need a fresh-agent review before they go through?
Homes:
- PG:112 `tool == "ExitPlanMode" and inp.get("plan")`; PG:117-119 `tool == "Artifact" and inp.get("action") in (None, "", "publish") and inp.get("file_path") and inp.get("asset") is not True`; PG:3 docstring "PreToolUse on Artifact and ExitPlanMode".
- LS:184 and FR:174 matcher `"Artifact|ExitPlanMode"` (equal; live file read).
- GR\README.md:63 "refuses to show a plan with ExitPlanMode, or publish an Artifact page, that no fresh agent signed off"; README:49 mentions Artifact pages as HTML documents; README:190 "ExitPlanMode's input fields are `plan` and `planFilePath`" (copy of the field names PG:112-113 reads).
- HR docs\agents\doc-review.md:17 "Four steps are gated: ... an Artifact publish; a plan shown with ExitPlanMode; and text posted to GitHub. A short text passes the last three with no review; `publish_gate.py` in the gates folder owns that limit." doc-review.md:21 "That covers public pages, release notes, plans and Artifact pages alike."
- GR\tests\test_publish.py:77,95,164,195,216 build calls with `ExitPlanMode`/`Artifact`; also tests\probe_results.md, replay.py, replay_extract.py mention the names (not read in depth).
What each home says: PG decides on input fields (plan present; action absent/empty/"publish"; file_path present; asset not true). The matcher decides which tool names reach PG at all, so a tool name added to the matcher but not to PG:112/117 passes silently, and the reverse never runs. README:63 and doc-review.md:17 describe the two tools in words and do not mention the `action` and `asset` conditions, so an Artifact action other than publish (or an asset upload) is gated by PG as "not gated", yet doc-review.md:17 says "an Artifact publish", which fits. doc-review.md:17 counts "text posted to GitHub" as the same gate; PG handles that by the `--check` path, not by this matcher.
Edges: at the limit: n/a here (limit is I3-063). Main vs agent: the matcher has no agent test, so an agent's ExitPlanMode or Artifact is gated the same. Card writers: same. Worktree: PG reads the repo marker from the call's cwd (F17 Q11).
Kinds: K1 (tool names in matcher, PG and two docs); K6-adjacent: two code places list the gated tools (matcher in settings, `if/elif` in PG), nothing ties them. K1 for README:190 field names against PG:112-113. F17 already noted the "from the old hooks\doc_review_tool_gate.ps1" claim; I re-checked that no `hooks\` folder or that .ps1 exists in GR now.
Splits: none found.
Test holding homes equal: none for matcher against PG; test_publish.py checks PG's behaviour for each tool only.

## I3-063 How the size is counted
Question: how are lines and characters counted when deciding a text is small?
Homes: PG:14 `SMALL_LINES, SMALL_CHARS = 20, 2000`; PG:29-33 the counting (non-blank lines, UTF-16 units, "as the old PowerShell gate counted them"); PG:103 the refusal message formats the same two constants (not a copy); PG:5 "the small-text rule has this file as its one home"; README:63 "`publish_gate.py` is also the one home of the short-text limit" (pointer, no number); doc-review.md:17 "A short text passes the last three with no review; `publish_gate.py` ... owns that limit" (pointer, no number). tests\test_publish.py:107-109 `test_20_lines_and_2000_characters_pass` pins both limits (it states 20 and 2000 itself, a second statement of the numbers in a test, not a source).
Searched: all GR .py/.md/.ps1, HR docs\agents\*.md, agents\, model-postures.md, memory\*.md, tools\session_cards; terms "non-blank", "UTF-16", "code units", "2000", "2,000", "20 lines", "twenty", "small", "short". The only other "2000" hits are unrelated (test padding, shell_output CAPS).
What each says: only PG states a rule. Docs point. The "20 lines" half is the user's own words (review-and-hooks.md:22, Confirmed); the 2000-character half and the counting method (non-blank, UTF-16, latin-1 fallback) are not in any Confirmed line; PG:13 attributes the numbers to "The user's decisions of 2026-10-10". The comment "as the old PowerShell gate counted them" points at a script that no longer exists in GR, so it cannot be checked.
Edges: exactly at the limit: PG uses `>` for both, so 20 lines and 2000 units pass (test_publish.py:107). Main vs agent, card writers: no agent test. Worktree: n/a.
Kinds: K1 only for the test; otherwise single home with pointers (as F17 found). Possible K7-ish: the 2000-character half has no Confirmed line, only a comment claiming the user decided it (not evidence).
Splits: none.

## I3-045 Comment-only change reading
Question: when does a changed file count as "comments and blank lines only", so the real-app check skips it?
Homes: RR:346 `change_is_comments_only` docstring "True when path changes from old to new in comments and blank lines only"; RR:262-363 the algorithm and `_c_code` (rows I3-045 text); GR\README.md:55 states the rule in prose: "A file whose change touches only comments and blank lines is skipped, even a comment at the end of a code line ... Its code, read the way a compiler reads it with the comments gone, must be the same at both ends, line for line. `change_is_comments_only` in `review_rules.py` owns that test, `_c_code` beside it is the one reader of C and C++ comments, and anything they cannot read counts as a real change." README:55 also says "a later comment-only change is skipped the same way" (applies the rule to the record check).
Searched: GR .py/.md; HR docs\agents\*.md (including ui-testing.md and derive-once-review.md, no comment-only text found there); memory notes; agents\; handoff.md; terms "comment-only", "only comments", "comments and blank lines", "comment at the end", "comment line". Memory note comments-explain-current-code.md is about writing comments, not this rule.
What each says: README:55 restates the rule's behaviour in words (end-of-line comment counts as comment, unreadable counts as change); RR holds the algorithm. README names RR as owner, so it is a pointer-plus-value home. The "mode change, added/deleted file, non-C file, diff with no hunks counts as real change" parts are in RR only.
Tests: GR\tests\test_review_rules.py:702 `test_block_comment_edit_needs_no_record`, :706 `test_one_code_line_among_comments_counts`, :719 `test_commenting_out_code_counts` (and more in the 640-730 range, not all read) run the real function; they hold behaviour, not a second copy.
Edges: main vs agent, card writers, worktree: the rule is path-and-diff only; no agent test. At the limit: "a comment at the end of a code line" is the boundary README names; RR owns it.
Kinds: K1 (README:55 describes the rule's cases in words next to the owner pointer; agree today). Single code owner (RR).
Splits: none.

## Side notes
- LS and FR are equal today in hooks, event by event; no test compares the real files. Only sync_settings.py's exit code reports a difference when someone runs it.
- LS carries no `permissions` comparison in my work; I read only the `hooks` block.
- A way past a hook, general words: a hook registered with a matcher only sees the tool names in that matcher; a gated action reachable under a tool name outside the matcher is not seen. Not examined further.
- Read-only: I changed no repo file; this file is in the scratch folder.
