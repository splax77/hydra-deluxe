# TOOL-AUDIT-T9-K4 closure check (18 ids)

Gates = C:\Users\Patrick\.claude\gates. Hydra = C:\Users\Patrick\Downloads\Hydra\hydra-test. Read 2026-11-19.
Result: 15 PASS, 3 REOPEN (L-F12-12, L-F13-03, L-F13-11), 0 GONE. All 18 reached.

### L-F12-10
**Verdict:** PASS
**Homes read:** Gates\percall.py:14 `LIBRARY_FLAG = "--whole-library"`, checked at :48; Hydra src\cli\batch_scope.h:10 `kWholeLibraryFlag`, `batch_scope()` returns `Refused` when the flag and folders are both absent, and its error says "A run over the whole library needs --whole-library"; src\cli\batch.cpp:82,95-97 uses it; docs\agents\this-machine.md:91 (a whole-library `--redo` needs the allow).
**Why:** exit 4 holds. hydra_batch cannot start a library run without the flag, so the docs' "any run over the whole library" and the hook's flag check answer one question. Searched only hydra_batch's entry (batch.cpp, batch_scope.h); I did not scan other CLIs for a library-wide mode.

### L-F12-12
**Verdict:** REOPEN
**Homes read:** Gates\shell_output.py:20-22 (`BUILD_TEST_COMMANDS` still lists `mutation_probe.ps1` and `hydra_precheck`); Hydra tools\ has build_slot.ps1 and new_worktree.ps1 but no mutation_probe.ps1; brief-preamble.md:35,47 (agents may run hydra_precheck, may not run the test programs).
**Why:** it left by "Merge, I pick", which needs both sides to be the same kind. Here one side is code (the tail list) and the other is rule text. Also the two do not answer one question: the tail list says which commands get an exit-code line, the rule text says which an agent may run. The only real defect is a stale code entry (`mutation_probe.ps1`, a script that no longer exists). Should be exit 4 for the may-run/may-not-run part, plus a mechanical fix of the stale name; not a merge pick.

### L-F12-15
**Verdict:** PASS
**Homes read:** Gates\bash_windows_rule.py:173,178 ("the user's ruling"); Gates\README.md:19 ("The user ruled on 2026-10-31 that ... losing battle") and :27 ("the user ruled on 2026-11-06 'Exempt git commit'"); Hydra docs\agents\rulings\review-and-hooks.md (Confirmed is lines 7-36, Unconfirmed starts at :37; "Exempt git commit" is :40, under Unconfirmed; the Bash-hook "Yes, build it" line :41 is also Unconfirmed).
**Why:** the claims are still there. Grep of rulings\ for "losing battle" finds nothing; the quote is only in docs\sessions\2026-10-31-6777b546.md:25 (a card, which closes nothing). The Confirmed 2026-10-31 line (:11) says something else (number versus efficiency). "Drop the claim" holds.

### L-F13-01
**Verdict:** PASS
**Homes read:** Gates\ref_judge.py:38-39 `PINNED` (four settings incl. trailer.separators), :53-54 drops GIT_CONFIG_PARAMETERS/COUNT, sets GIT_NO_REPLACE_OBJECTS and GIT_GRAFT_FILE; Gates\review_rules.py:61-62 (three pins, six env drops), :138-139; Hydra tools\check_handoff.py:25-26,39-40 (same three and six).
**Why:** all code, one question (what git is pinned to), answered three ways. "Merge, I pick" fits.

### L-F13-03
**Verdict:** REOPEN
**Homes read:** brief-preamble.md:35 ("Above 30,000 characters it arrives as Claude Code's own notice ..." and "`shell_output.py` ... owns the number and the lists"); Gates\shell_output.py:14-15 (`CAP = 4000`; the file's header says it is the one home of its own numbers); Gates\tests\probe_results.md:127,223 (the measured 30,000 limit); Gates\README.md:25.
**Why:** it left by "Give each a home", which is for a rule that lives only in prose or a note. This one is not homeless: the 30,000 already has a home in the preamble and measurements in probe_results.md. The defect is that the preamble says shell_output.py "owns the number" while the code owns only 4,000 (30,000 is Claude Code's own limit). That is a doc pointing at the wrong owner, so it is a fix of the doc's pointer (code-wins kind), not a rule needing a home.

### L-F13-04
**Verdict:** PASS
**Homes read:** brief-preamble.md:43 (six commands "such as", ends "`delete_rule.py` ... is the one home"); Gates\delete_rule.py:32-35 (longer list: switch, checkout ., restore ., branch -D, stash clear, worktree remove); Gates\settings.fragment.json:59-64 (`ask`: rm, git clean, git reset --hard, Remove-Item).
**Why:** two splits. The preamble's short list against the code is doc-versus-code and the preamble already calls delete_rule.py the one home, so "The code wins" (point the doc at it) fits and opens no gap. The agent-refused list against the main-session ask list is code versus settings config, one question answered two ways, so "Merge, I pick" fits. Both splits are covered.

### L-F13-05
**Verdict:** PASS
**Homes read:** Gates\delete_rule.py:14-18 (only Hydra\.claude\worktrees and C:\Users\Patrick\.claude\worktrees); Gates\cleanup_sweep.ps1:174-178 (`<root>\.claude\worktrees` in any repo, plus `<root>\worktrees` in the repo the hook lives in); brief-preamble.md:43.
**Why:** the split is still there today (the delete rule has not yet been changed). "Trust both" answers exactly this split.

### L-F13-07
**Verdict:** PASS
**Homes read:** brief-preamble.md:71 (forbids amend, rebase, reset, moving a branch, and says the gate refuses main and branches origin has); Hydra docs\agents\integrate.md:31,50; Gates\ref_judge.py:174 `shared = ref == MAIN or current("refs/remotes/origin/" + name) is not None`, plus a refused "branch that is under review" (:131).
**Why:** the preamble already states the gate's real scope, so the docs and the hook do not contradict. What is left is an instruction with no hook for an agent's own local branch, which is the theme 3 case ("add a hook only where cheap and exact, else instruction"). Passes. Minor: the preamble does not mention the "under review" branches the gate also refuses.

### L-F13-11
**Verdict:** REOPEN
**Homes read:** Hydra docs\agents\this-machine.md:49 ("The user has ruled that no elevated window is opened."); grep of docs\agents\rulings\ for elevated/administrator/admin finds no line; docs\sessions\2026-11-07-7d216054.md:11,21 (card only); memory check-long-captures-early.md:15 and MEMORY.md ("prefer ETW over hooks"); docs\agents has no page on ETW.
**Why:** the claim half ("Drop the claim") holds: no Confirmed line says it. But the row has more splits that "Drop the claim" does not cover: (a) the memory note recommends ETW/WPR while this-machine.md:49 says wpr is refused from agent shells (note against doc); (b) the ch MCP's own Frida hooks are the opposite of "prefer ETW over hooks". L-F13-11 is not among the rows Q2 ("Note is wrong") names, and "Note is wrong" would itself fail for (a), since the note's rule "prefer non-invasive capture" has no doc home (grep of docs\agents for ETW finds only unrelated hits). Needs the note split placed.

### L-F13-21
**Verdict:** PASS
**Homes read:** brief-preamble.md:27 (Read tool), :77 (foreground only); Gates grep for run_in_background finds only agent_stop.py:100 (comment: a task is background because it was started so "or moved there after running too long") and shell_output.py:75 (no tail on background); grep of percall, bash_windows_rule, shell_output, media_rule, delete_rule, progress for head/tail/sed/cat finds no refusal.
**Why:** both rules are prose with no hook, and nothing in the repo says otherwise. agent_stop handling background tasks is not a contradiction, because a command can be backgrounded after running long. "Give each a home" (instruction in the preamble, hook only if cheap) holds. Note: this session's own tool text allows cat/sed through Bash; that is outside the repo's homes.

### L-F13-22
**Verdict:** PASS
**Homes read:** Gates\percall.py:14,48; Gates\review_rules.py:51 (`LIBRARY_ALLOW_TEXT = "allow library run"`) and tests at test_review_rules.py:839-847 (case and a trailing . or ! tolerated); Hydra src\cli\batch_scope.h; this-machine.md:91; CLAUDE.md rule 8 ("asked for in chat first").
**Why:** runs half: exit 4 holds (same as L-F12-10, and the `--redo` case is covered because a bare library run is refused). Phrase half: "The code wins" holds, because the hook is stricter than rule 8, so fixing the doc to point at the hook opens no way past the gate.

### L-F14-08
**Verdict:** PASS
**Homes read:** Hydra tools\ch_mcp\game_tools.py:953-972 ("give all four of the run's settings, or none"); tools\ch_mcp\compare_scorestats.py:202 (keys on checksum, speed, no_fail only).
**Why:** two code paths, one question, different answers. "Merge, I pick" fits.

### L-F15-14
**Verdict:** PASS
**Homes read:** brief-preamble.md:77; Gates\agent_stop.py:100 and its `_BACKGROUND_STARTED` readers; Gates\shell_output.py:75; same greps as L-F13-21 (no hook refuses run_in_background); Gates\media_rule.py:6 `MEDIA_ROOT`.
**Why:** the row's splits are the background rule (prose with no hook, no contradiction, as in L-F13-21) and tests copying hook values (no split stated). "Give each a home" holds.

### L-F16-01
**Verdict:** PASS
**Homes read:** Gates\delete_rule.py:3-6 (a quoted delete is not read); Gates\shell_output.py:45-52 `_unquoted`; Gates\media_rule.py:14-15,31,51.
**Why:** all code, one question (is a quoted first word a command), different answers. "Merge, I pick" fits. The "by the user's ruling" comment at bash_windows_rule.py:178 belongs to theme 1 (see L-F12-15), not to this row.

### L-F16-02
**Verdict:** PASS
**Homes read:** Gates\delete_rule.py:29,75-77 (cuts at an operator anywhere, `a;b` cut inside the word); Gates\media_rule.py:18-27 (cuts only where an operator stands alone).
**Why:** both code, different cut. "Merge, I pick" fits.

### L-F16-03
**Verdict:** PASS
**Homes read:** Gates\delete_rule.py:17-35,58-63; Gates\settings.fragment.json:59-64; brief-preamble.md:43; Gates\README.md:19 (not re-read for its list).
**Why:** code against settings config (refuse list against four ask patterns), plus a docs-against-docs short list against the long list. "Merge, I pick" covers both kinds. The verifier's side note (worktrees under Hydra\worktrees not allowed) is the folder question, which Q11 / L-F13-05 "Trust both" covers; it is not a separate split of this row.

### L-F16-07
**Verdict:** PASS
**Homes read:** Gates\progress.py:22 (`gates` only); Gates\tests\replay.py:280 (`(hooks|gates)`).
**Why:** both code. Still differ today. "Merge, I pick" fits.

### L-F16-14
**Verdict:** PASS
**Homes read:** Gates\ref_judge.py:38-39 (trailer.separators too); Gates\review_rules.py:61-62; Hydra tools\check_handoff.py:25-26; tools\effort_test_faults.py:97 (quotePath only).
**Why:** all code, still differ. "Merge, I pick" fits.

## Ids not reached
None.

## Gaps noted
- The exits.md, log.md and ledger files I read came from the worktree copy of the audit folder; main has no docs\audit\2026-11-18-tooling-derive-once yet.
- I started one recursive grep over the whole Hydra tree by mistake; it ran in the background and finished on its own (exit 0). Nothing is left running.
