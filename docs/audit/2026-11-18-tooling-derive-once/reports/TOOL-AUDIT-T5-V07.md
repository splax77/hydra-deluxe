# TOOL-AUDIT-T5-V07 (verifier of F07). All claims "by reading"; nothing run.
Gates = C:\Users\Patrick\.claude\gates; H = C:\Users\Patrick\Downloads\Hydra\hydra-test.
Checked the finder's 10 split lines. The other families were not re-opened (listed at the end).

## B1 CONFIRMED (K2)
review_dispatch.py:155-159: `if not keys: if not _gated(...): return None`, then deny. _gated (:132-137) is true only when the repo has a local main and a review marker on it. README:39 says "The hook refuses a brief with no commit". DOR:200 says "A hook refuses a reviewer dispatch with no key". Both are unconditional. Situation: a repo with no review marker on main passes a keyless brief. No Confirmed ruling names both sides (review-and-hooks.md :19 and :20 are about sign-off lines and fixes).

## B2 CONFIRMED (K2)
_key_problem (:108-110) refuses "no local main branch"; README:39 lists it. DOR:17 and DOR:200 list no key, key on main, unknown key, more than one commit, and not the no-local-main case. Situation: a repo with no local main.

## C1 CONFIRMED-CHANGED (K2)
The ui-testing.md table (:213-229) has 17 rows (hydra_start through hydra_run), not 16. TOOL_NAMES in tools/test_hydra_ui_mcp.py:51-55 has those 17 plus hydra_build_pass, so 18, not 17. hydra_ui_mcp.py:874 defines hydra_build_pass. Same split: ui-testing.md:209 says the table "is a copy" of the owner list and lacks the tool. The test (:724) holds the live list to TOOL_NAMES only, not to the doc.

## E1 CONFIRMED (K6/K2)
tests/replay.py:19-20: "because the main-session route above does not reach that rule". percall.py:126-129 `_main_session` runs `bash_windows_rule.check(data)`; README:11 says the same. Situation: a main-session shell call. I read replay's header only, not its routing code; a replay run would settle whether the code matches the header (not run).

## F1 CONFIRMED with a caveat (K8)
status_append.ps1:5 reads "The call shape: the brief preamble's 'Status lines' section."; :9 says progress.py owns how a write is recognised. Preamble:19 is cut off in my view (it begins "Type the line out in full..." and names the progress hook's refusal). I did not see the finder's exact sentence "The status-call check in `progress.py` owns the exact shape", so that half is the finder's word.

## F2 CONFIRMED (K6)
common.py:113-116 check_id refuses "", ".", "..", and any of / \ :. progress.py:26 _STATUS_CALL accepts only [A-Za-z0-9_-]+. status_append.ps1 accepts a name only if an agents-folder record `<name>.json` exists. Situation: an id like "a.b" passes check_id and cannot be written as a status call. Whether real ids ever have that shape is not settled by reading.

## I1 CONFIRMED-CHANGED (K3)
analyzing-sp-squeezes SKILL.md:64: "Note whether the fleet hooks held: one background executor per task, no nested background workers." Preamble:77: "Never use run_in_background... You have no helper agents." The skill line is a check-and-note, not an instruction to launch background executors, so "treats as expected" overstates it. What is real: "fleet hooks" has no definition anywhere I saw, and "background executor" may mean a main-session background dispatch or an agent's own run_in_background; reading cannot tell.

## J1 CONFIRMED (K2)
delete_rule.py:17-18 WORKTREE_FOLDERS holds two exact paths (hydra-test\.claude\worktrees, C:\Users\Patrick\.claude\worktrees); _roots (:48-55) and _instead use only those. MP:5 says "under a `worktrees` folder (the delete rule trusts only those)". Preamble:43 gives the two paths and also "any worktree directly inside a `worktrees` folder of either repo". This session's worktree, hydra-test\worktrees\tooling-audit-records, is in neither list. Ruling review-and-hooks.md:17 (under ## Confirmed, which starts :7) says "a worktree that sits in a `worktrees\` folder", naming no paths; it does not name the two-paths-vs-any-folder sides.

## Scout model CONFIRMED (K3)
AG\scout.md:4 `model: sonnet`; CLAUDE.md rule 2 says "Read-only scouts run on Sonnet or Haiku." Haiku would need a dispatch-time override; whether the harness allows one is open (a run would settle).

## Two Confirmed status lines CONFIRMED (K2 between rulings)
review-and-hooks.md :33 "demand status every 10 tool calls and reject the next call after 15" and :34 "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes"; both 2026-09-04, session 48cdbb34, both under ## Confirmed (## Unconfirmed starts :37). progress.py:16 follows :34. Neither line names the other, so no ruling covers both sides.

## Side note 1 CONFIRMED
tests/replay.py:41 `LINE_BUDGET = 3100`; review-and-hooks.md:13 Confirmed "Raise cap to 3,050". Values differ; I looked for no later ruling.

## Splits the finder missed
None. One correction: C1 counts (17 in the doc, 18 in server and test).

## Not reached (unverified, finder's words only)
F-A model copies and Opus alias, F-D (CH MCP, NOTHING_OPEN, KIT_PLAYER), F-G memory-note claims, F-H trailers, F-I rows other than I1, F-K CLAUDECODE, F-L probes, F-M, F-N, side notes 2-6, B3 (archive excluded).
