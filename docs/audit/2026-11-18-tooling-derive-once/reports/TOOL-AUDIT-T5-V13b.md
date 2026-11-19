# TOOL-AUDIT-T5-V13b (verifier of F13b). All claims by reading; nothing run.
HY = C:\Users\Patrick\Downloads\Hydra\hydra-test; GATES = C:\Users\Patrick\.claude\gates.

## I6-197 CMake refusal fix: CONFIRMED (K1, no split today)
this-machine.md:35-43 holds the install table, the "build-cpp cache names the 2026 Build Tools" line, and the `-DCMAKE_GENERATOR_INSTANCE="<folder>,version=<version>"` advice; it says Find-CMake falls back to `vswhere -all`. The finder's quotes match. I did not re-run vswhere or reopen CMakeCache/find_cmake.ps1 (those readings of the finder stay unchecked by me). Nothing holds the table equal to the machine.

## I6-198 shell not elevated: CONFIRMED
- this-machine.md ends its "not elevated" section with: "The user has ruled that no elevated window is opened." (quote matches.)
- grep -rniE "elevated|administrator" over HY\docs\agents\rulings returned nothing, so no `## Confirmed` line holds it. The words exist only in card/plan (finder's cites, not reopened by me). No ruling names either side.
- Memory note check-long-captures-early.md:15 says "Prefer non-invasive capture (ETW/..." (my output cut the rest). Split by reading: the note recommends ETW, this-machine.md says wpr is refused. A memory note is not the user's words.
- Nothing enforces it (finder's GATES grep; not repeated).

## Worktree folders: CONFIRMED
- delete_rule.py:14-18 WORKTREE_FOLDERS = HY\.claude\worktrees and C:\Users\Patrick\.claude\worktrees (its comment: "The worktrees folders of the two repos").
- derive-once-review.md:197 names the same two paths.
- cleanup_sweep.ps1:172-179: `<root>\.claude\worktrees` in any repo, plus `<root>\worktrees` only in the repo the hook lives in (~\.claude), "where the main session makes its task worktrees".
- git worktree list: 98 entries under hydra-test\worktrees, 3 under .claude\worktrees. So most Hydra task worktrees sit in a folder the delete rule and the doc do not list. Situation: an agent inside one of those 98 deleting its own build folder.
- new_worktree.ps1 not reopened.
- Ruling review-and-hooks.md:17 (2026-10-25, "Keep rule, move worktrees (Recommended)") says "a `worktrees\` folder" and "Ten Hydra worktrees sit beside the repo as wt-*". It names neither path, so it does not name both sides of the split.

## Three builds: CONFIRMED-CHANGED on two points
- tools\build_slot.ps1:195 refuses a second pass for the same session: "no pass: this session already holds one ... One build at a time per session." That matches the ruling's "one build per session". The finder's K3 worry is REFUTED by reading: the script enforces it. Count is BuildSlotCount (:197-207). I did not reread agents-and-workflow.md:8 (my sed printed the wrong line).
- State folder: build_slot.ps1:10,78 and bench_run.ps1:20 use ~\.claude\hooks\state\build_slots and \bench. Actual contents of ~\.claude\hooks\state today: only `build_slots`. The finder said hooks\state also holds agents, cleanup_sweep, posture and so on. That is wrong: those names are in gates\state. No GATES .py or .ps1 mentions hooks\state or build_slots. So there is no duplicate state home for one question; build locks just sit in a different folder from the gates' state. The finder's K6 candidate is REFUTED as stated. (`bench` folder is absent today.)
- The count of three in CLAUDE.md, agent-rules, brief-preamble, the script and test_build_pass.ps1: not reopened by me.

## Command-length limit: CONFIRMED (K1)
brief-preamble.md:33: "A command over 1,000 characters" and "A `git commit`'s own text does not count toward that limit", and it ends "`bash_windows_rule.py` in the gates folder owns the rule and the number." So the preamble does carry the commit exemption (the finder left this open). Test lines not reopened.

## Key route text: CONFIRMED
review_rules.py:54-55: KEY_ALLOW_TEXT = "allow key run"; _KEY_ALLOW = re.compile(escape + r"[.!]?", IGNORECASE); :86 fullmatch on stripped text. So "Allow key run." and "allow key run!" pass. clone-hero-mcp.md:29 names ch_key, ch_press, ch_play_chart with route keys and the hook; the "exactly" wording is the finder's quote and I saw only its start. ch_call forms not reopened. No ruling checked.

## "Up to four lines": OPEN
rulings\README.md:38 says "does these four things in order", a different thing. The "up to four lines" text at :41 was cut off in my output, not seen. I did not search ruling_rules.py.

## sync_settings launch path: CONFIRMED
sync_settings.py:20 `python.exe -I C:\Users\Patrick\.claude\gates\sync_settings.py`; README:110 uses C:\Users\Patrick\AppData\Local\Python\pythoncore-3.14-64\python.exe -I ... Both launch text, one bare name, one full path.

## Not reached
Reread of agents-and-workflow.md:8; test_build_pass.ps1:117; the 1,000 tests; new_worktree.ps1; the card and plan quotes; ruling_rules.py for four lines. A run (hook test) would be needed to show a worktree under HY\worktrees refused by delete_rule; not done.

## Splits listed
1. this-machine.md "has ruled" with no Confirmed line.
2. Memory note ETW vs this-machine.md wpr refusal.
3. delete_rule/derive-once-review list .claude\worktrees; 98 worktrees sit in HY\worktrees; cleanup_sweep lists that folder only for the hooks repo.
4. Key allow: doc "exactly" vs code accepts case, trailing . or !.
5. sync_settings launch: bare python.exe vs full path.
Refuted: build-state duplicate folders; one-build-per-session gap.
