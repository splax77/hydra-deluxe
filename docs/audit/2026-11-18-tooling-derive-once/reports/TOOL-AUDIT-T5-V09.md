# TOOL-AUDIT-T5-V09 (verifier V09, finder F09). All verdicts "by reading". Card tools live in C:\Users\Patrick\.claude\tools\session_cards (not the Hydra repo).

## Q1 status file path: CONFIRMED (K1, K6)
common.py:10 `STATE_ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "state")`; :123-124 `status_path` = `<root>\status\<id>.md`. status_append.ps1:11-14 `$StateRoot = 'C:\Users\Patrick\.claude\gates\state'`, `Join-Path $StateRoot 'status'` and 'agents'. Preamble line 13 repeats the full path. Difference: common is file-relative, the helper is fixed, so a gates copy in a worktree would differ (a run would settle it). Not opened: doc-review.md:9, derive-once-review.md:9 and the test lines. Split: none today.

## Q2 worktree roots: CONFIRMED (K1, K2, K6), split S1
delete_rule.py:14-18 `WORKTREE_FOLDERS = (...hydra-test\.claude\worktrees, C:\Users\Patrick\.claude\worktrees)`. cleanup_sweep.ps1:174-178 `Get-SweepPrefixes`: `<root>\.claude\worktrees` for any repo, plus `<root>\worktrees` for the .claude repo only. README:136 names only `C:\Users\Patrick\.claude\worktrees\<task>`. derive-once-review.md:197 names both delete_rule paths. Hydra tools\new_worktree.ps1:6,29 builds `<main>\.claude\worktrees\<TaskId>`. Difference: a worktree at hydra-test\worktrees\<task> (this audit's own) is in neither delete_rule root nor the sweep list. The sweep accepts `.claude\worktrees` in any repo; delete_rule only the two. Ruling review-and-hooks.md:17 (2026-10-25, chose "Keep rule, move worktrees (Recommended)") speaks of "a worktree that sits in a `worktrees\` folder"; it does not name the lists, so it does not name both sides.

## Q3 Clone Hero fact location: CONFIRMED (K1, K3)
Hydra CLAUDE.md: "updates its page in the same commit ... never in a memory note". Card writers: writer-brief.md:88 "The `write-back:` line"; card-writer.md:9 says the writer brief replaces the repo preamble. Different actor and action for card writers. Preamble:105 and clone-hero README:80 not re-opened. No split.

## Q4 plain-English home: CONFIRMED (K1, K8-adjacent)
doc-review.md:81: "That file is the one home of the style rules; read it ... In short, check that jargon is glossed on first use, sentences are short, and no wall of file:line bullets ..." (restates). Preamble:123 states the rule too. model-postures.md is at C:\Users\Patrick\.claude\model-postures.md (not in Hydra); its line 13 not opened. No split.

## Q5/Q6/Q7 repo, version, machine facts: partly CONFIRMED
CMakeLists.txt:21 `project(Hydra VERSION 3.1.2`. this-machine.md:17 carries the release tag rule "v plus the version"; I did not see the literal 3.1.2 in my cut of line 17, so the copy is unconfirmed. stall_harness.py:122-124: `# The PowerShell lookup docs/agents/this-machine.md gives for cdb's folder.` then `_CDB_LOOKUP = "(Get-AppxPackage -Name Microsoft.WinDbg).InstallLocation"`: a code copy naming a page as its source (K6/K1). The `rea-agents@6.5.0` pin was not found by my grep of this-machine.md (line 25 mentions REA only): that part is NOT CONFIRMED; the skill copy was not opened. I6-205 and I6-193 not opened.

## Q8 card tools: CONFIRMED (K6, K1, K3, K7), splits S2, S3
check.py:316 and prepass.py:461 both `repos = a.repo or [common.HYDRA_REPO, common.CLAUDE_REPO]`. common.py:258-260 owns staging; session_cards_start.ps1:259,270-271 rebuild `Join-Path $cardData 'batches'` and `'staging'`. batch.py:63 also builds `os.path.join(a.out, "batches")` (a third home the finder missed; I found no batches function in common.py). Local rule: common.py:272-277 `marked_local` (header says yes); :280-289 `is_local_only` docstring "Every script asks here, never by its own test."; check.py:232-239 `keeps_local` = `common.marked_local(header) or common.is_local_only(out, entry)`. Two tests combined, so the comment is wrong. Ruling: common.py:263-269 "The user ruled on 2026-10-31 ..."; writer-brief.md:16 "The user ruled on four kinds of content on 2026-10-31". My grep of docs/agents/rulings/ for "locally", "local card", "not about the project" returned nothing, so no `## Confirmed` line found (search limited to those words). Edge: the card writer marks `local: yes`; the check also keeps a card local if its session already is.

## Q9 settings sync / shared code: partly CONFIRMED (K1, K7)
gates README:156: "It runs every test only when it touches what every hook shares: `common.py`, `lib\`, `settings.fragment.json` or `tests\lib\`." agent-rules.md:7 and model-postures.md:11 were not seen with that list (my cut of the lines stopped early): unverified. Marker copy: README:138 names `gates/`, `agents/`, `tools/session_cards/`; the marker file holds the same three `gate:` lines (K1, confirmed).

## Side note K8 (hooks\lib): CONFIRMED-CHANGED
Hydra .derive-once-gate: "Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1)"; .doc-review-gate: "hooks\lib\doc_review_rules.ps1". Finder said C:\Users\Patrick\.claude\hooks does not exist: wrong. It exists, holding only `state\build_slots`; `hooks\lib` does not exist. Real owners: gates\review_rules.py (BUILTIN_CODE line 22, markers line 18, `_is_code` line 180) and gates\publish_gate.py:12 `MARKER = ".doc-review-gate"`. The wrong-owner claim stands.

## Not reached / not verified
I6-148, I6-205, I6-206, I6-215, I6-193, I5-176; Q10 smaller rows (scout scope, agent_cost.py, handoff, media guard); memory-note homes; the test-line claims (test_docs_numbers.py, test_delete_rule.py); agent-rules.md:7 and model-postures.md:11 list copies; this-machine.md "3.1.2" and rea pin copies.

## Splits (mine)
S1 worktree roots (three lists, this audit's worktree path in none). S2 local-card ruling has no Confirmed line found. S3 check.py combines two local tests while common.py says every script asks one place.
