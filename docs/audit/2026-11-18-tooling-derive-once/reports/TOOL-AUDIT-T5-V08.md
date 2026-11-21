# TOOL-AUDIT-T5-V08 (verifier V08, checking finder F08). All claims "by reading". Gates folder = C:\Users\Patrick\.claude\gates.

I checked the 14 questions at the points that carry the weight, not every row id. Items I did not re-open are listed at the end.

## Q1 status line / state folder: CONFIRMED
- status_append.ps1:11 `$StateRoot = 'C:\Users\Patrick\.claude\gates\state'` (override only by HOOKS_TEST_STATE_ROOT, :12). common.py:10 `STATE_ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "state")`; status_path common.py:123-124. Two code homes decide the folder (K6). Same folder today; they differ when gates runs from a copy (common derives from its own file, status_append is fixed).
- progress.py:22 STATUS_PREFIX has the same call text as preamble :15. Preamble :13, derive-once-review.md:9 and doc-review.md:9 repeat the status path; writer-brief.md:100 repeats the call (K1).
- I1-055: CONFIRMED-CHANGED. The charset `[A-Za-z0-9_-]+` is at progress.py:26. status_append.ps1:36-41 does not hold a charset; it checks that an agent record `<name>.json` exists. So that is a different test, not a fourth copy of the charset.
- K8: common.py:3 "The folder layout and the record formats are described in gates/README.md." I did not find a layout section in the README parts I read (not the whole README).

## Q2 delete roots: CONFIRMED, with a stronger split
- delete_rule.py:17-18 WORKTREE_FOLDERS = `...\hydra-test\.claude\worktrees` and `C:\Users\Patrick\.claude\worktrees`. Preamble :43 and derive-once-review.md:197 name the same two (K1).
- cleanup_sweep.ps1:174-178: `<root>\.claude\worktrees` for any repo, plus `<root>\worktrees` only for the repo the hook lives in (~/.claude). So delete_rule and the sweep both decide the folder list (K6).
- `git worktree list` in Hydra shows worktrees under `hydra-test\.claude\worktrees\` AND under `hydra-test\worktrees\` (for example batch-rows-br1). The folder `hydra-test\worktrees` exists. It is in neither delete_rule nor the sweep's Hydra prefixes. The finder said the used ones "sit in worktrees\"; only half right: both forms are in use today.
- Situation where the homes differ: an agent in a worktree cut under `hydra-test\worktrees` cannot rm -rf its build folder, and the sweep never removes that worktree. Preamble :43 says "A worktree cut anywhere else is not a root", so text and code agree; the split is between code and practice.
- Ruling: docs\agents\rulings\review-and-hooks.md:17 (Confirmed, 2026-10-25): "Keep rule, move worktrees (Recommended)" to "...a worktree that sits in a `worktrees\` folder. Ten Hydra worktrees sit beside the repo as `C:\Users\Patrick\Downloads\Hydra\wt-*` instead." It names a `worktrees\` folder in general. It does not name `hydra-test\worktrees` against `hydra-test\.claude\worktrees`, so it does not name both sides of this split.

## Q3 media root: CONFIRMED single home
media_rule.py:6 `MEDIA_ROOT = r"C:\ytdl"`. I found no second code home (searched gates py, ps1, README by name). No kind.

## Q4 shell-output: CONFIRMED (K1)
shell_output.py:26-28 SAVED_FOLDER "shell-output", FALLBACK_FOLDER "tool-results", NAME_DIGITS 12. README.md:23 repeats "a `shell-output` folder inside the session's scratch folder". Test literals not re-opened.

## Q5 plan/measurement pointers: CONFIRMED-CHANGED
Pointers are not homes. Only the 8 card-tool copies of the removed-plan string would be K1; I did not recount the eight. A removal note is what the Hydra docs rule asks for, so it is not drift by itself.

## Q7 gate markers: CONFIRMED (K1/K6)
review_rules.py:18 `CODE_MARKER, DOC_MARKER = ".derive-once-gate", ".doc-review-gate"`. publish_gate.py:12 `MARKER = ".doc-review-gate"` (used :47). Same text today; no split.

## Q8 review-key repo: CONFIRMED single code home; approval claim not backed
review_dispatch.py:68-69: "one exception the user approved on 2026-10-24". Confirmed lines dated 2026-10-24 in Hydra rulings: review-and-hooks.md:48 ("just move everything to c++"), tests-and-runs.md:15,16,33, ui-and-display.md:47-51. None is about a review key judged in another repo. K8 possible (a code comment cites an approval I could not find under `## Confirmed`; the decision log and cards were not searched).

## Q9 Python path: CONFIRMED (K1/K6)
`pythoncore-3.14-64\python.exe` is in install_ref_gate.ps1:10, publish.ps1:16, README.md, settings.fragment.json (9) and ../settings.json (9), tests/test_shell_output.py. Counts equal. this-machine.md and derive-once-review.md:124 not re-opened. No split today.

## Q10 git shim: CONFIRMED (K1/K6)
ref_judge.py:31-32 ALLOW/DENY/SELFTEST_OK. ref_gate.sh:15-16 repeat `ref-gate-verdict: allow` / `deny`. install_ref_gate.ps1:12-13 repeat `# git-ref-gate shim` and `ref-judge selftest: allow ok, deny ok`. Same text today.

## Q12 / Q13 partial
cleanup_sweep.ps1:597-600 names `stall`, `posture`, `*.json` subfolders: CONFIRMED (K1/K6). The K4 claim (stall.py builds the transcript path, agent_stop.py reads it from input) and posture.py were not opened.

## Q14 session cards: CONFIRMED in part
- C:\Users\Patrick\.claude\tools\session_cards\common.py:33-40 owns PROJECT_NAME, DATA_ROOT, DEFAULT_MEMORY, CARDS_ROOT.
- Hydra tools\agent_cost.py:31 and tools\check_pointers.py:30 spell the same project folder name again: CONFIRMED (K6).
- session_cards_start.ps1:274 `$rel = 'docs/sessions'` vs common.py:40 CARDS_ROOT (`docs`, `sessions`): CONFIRMED (K6, same value).
- session_cards_start.ps1:21 names state_prune.ps1; the file does not exist in gates: CONFIRMED K8.
- Other Q14 rows (batch names, tags.txt, check.py regex, writer-brief paths) not opened.

## Splits (own lines)
1. Hydra worktrees under `hydra-test\worktrees\` are in neither delete_rule.WORKTREE_FOLDERS nor cleanup_sweep's Hydra prefixes (cleanup_sweep.ps1:177 gives `worktrees` only to ~/.claude). Hydra worktrees under `hydra-test\.claude\worktrees\` are in both. Both forms are in use. No Confirmed line names both sides.
2. status_append.ps1:11 fixed live path vs common.py:10 path derived from its own file: same value for the live gates folder; differs only for a copy.
No other value split found.

## Not reached / not re-opened
Q6, Q11, K4 in Q12, posture.py (Q13), test literals named by the finder, derive-once-review.md body, this-machine.md, README beyond the lines quoted, the 8-copy count in Q5, Q14 batch/tags/check rows.
