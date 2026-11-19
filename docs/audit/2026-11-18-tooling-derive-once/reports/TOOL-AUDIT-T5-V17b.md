# TOOL-AUDIT-T5-V17b: verifier of F17b (all claims "by reading"; I ran no hook or test)

GR = C:\Users\Patrick\.claude\gates. Live settings = C:\Users\Patrick\.claude\settings.json. I loaded the live and fragment `hooks` blocks as JSON (a data read) and compared them.

## Side note 1: live settings.json equals settings.fragment.json
CONFIRMED. The two `hooks` blocks are equal as parsed data. All 11 entries listed, with matcher, timeout and onFailure: SessionStart x4, UserPromptSubmit, PreToolUse `*` (timeout 10, block), PreToolUse `Agent` (20, block), PreToolUse `Artifact|ExitPlanMode` (20, block), PostToolUse `Bash|PowerShell`, SubagentStop `*`, Stop. Only the three PreToolUse entries carry `onFailure: block`. No test compares the real files. Open run: `sync_settings.py` would report a difference, but nothing differs today.
Note: the files must be read as UTF-8; a default (cp1252) read fails. Not a split.

## I3-060 Crash handling: CONFIRMED
Homes, all open:
- Code mode: `common.run(body, "closed")` at publish_gate.py:135, delete_rule.py:187, percall.py:176, review_dispatch.py:178. `"loud"` at agent_stop.py:297, posture.py:55, prompt.py:67, shell_output.py:201, waiver.py:50. prompt.py:30 passes a variable `mode` (an extra place the finder did not list). common.py:56-69 defines the two modes: closed denies with "Gate hook crashed, so the call is refused".
- Registration: `onFailure: block` on `*` (percall), `Agent` (review_dispatch), `Artifact|ExitPlanMode` (publish_gate); the live file and the fragment agree.
- README:95-100 says in words that closed is for hooks that can refuse, loud is for hooks that only inform, and "The refusing hooks carry `onFailure: block` in their registration for that case."
- test_docs_numbers.py:163 asserts the shell_output registration has no `onFailure`.
Situation where the homes differ: none today. A crash inside the script is handled by the code mode. A script that cannot start is handled only by `onFailure`. delete_rule is "closed" with no registration of its own (the finder says it is a library of percall; I did not open that import).
Splits: none. Nothing ties the code-mode list to the `onFailure` list except that one negative test. No Confirmed ruling speaks to it beyond agents-and-workflow.md "agents should fail loudly" (not a ruling on this question; I did not re-open that file).

## I3-065 Which tools are gated: CONFIRMED
Homes: publish_gate.py:112 (ExitPlanMode with plan) and :117-119 (Artifact with action None, "" or "publish", a file_path, and asset not True); live and fragment matcher `Artifact|ExitPlanMode`; README:63 and :49 in words; doc-review.md:17 and :21 in words.
Situation where the homes differ: an Artifact call with another action or an asset upload reaches the code through the matcher and is let through; README:63 and doc-review.md:17 say only "publish", which fits. A tool name added to the matcher alone, or to the code alone, would split them; no test ties the two.
Splits: none today.

## I3-063 How size is counted: CONFIRMED
Homes: publish_gate.py:14 `SMALL_LINES, SMALL_CHARS = 20, 2000`, with the comment "The user's decisions of 2026-10-10"; :29-33 the count (CRLF and CR folded to LF, UTF-16 units over 2000 is large, then non-blank lines over 20 is large, latin-1 fallback on decode failure). README:63 and doc-review.md:17 point to the file with no number. test_publish.py:107-109 pins 20 lines and 2000 characters.
Confirmed rulings: review-and-hooks.md has "only large items over 20 lines should go for review" (2026-10-10, session b5e8d95a). That names the 20-line side only. A grep of docs\agents\rulings\*.md for "2000" or "2,000" found only an unrelated clone-hero line. So the 2000-character half and the counting method are in code only; the code comment calls them the user's decision, which is a claim, not evidence. The "old PowerShell gate" the comment cites is not in GR (the folder has publish.ps1, a different script; I did not read it to see whether it holds the count).
Splits: none found.

## I3-045 Comment-only change: CONFIRMED-CHANGED (small)
Home: review_rules.py:345 `change_is_comments_only` with its docstring; README:55 restates the cases in words and names that function and `_c_code` as owners. Single code owner; agrees today.
Change from the finder: the test line numbers are off. The tests are at test_review_rules.py:702, :706 and :719 (the finder's 702-719 is right for these three; it also lists the run of the real function, which is right). Other callers of the function are at :559, :700 and :791. No change in verdict.
Splits: none. No Confirmed ruling found.

## Splits list
None found for any of the four. Gaps that are not splits: no test ties the matcher to the code (I3-065); no test ties code modes to `onFailure` (I3-060); the 2000-character half has no Confirmed ruling (I3-063).
