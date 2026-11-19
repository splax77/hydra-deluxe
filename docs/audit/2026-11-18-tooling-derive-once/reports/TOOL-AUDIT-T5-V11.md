# TOOL-AUDIT-T5-V11 (verifier of finder F11). All claims "by reading"; nothing run.
H = C:\Users\Patrick\Downloads\Hydra\hydra-test ; G = C:\Users\Patrick\.claude\gates

## Checked (the 5 K8 rows and the 7 splits; other rows were not reached)

### I9-061 / S1 worktree folders: CONFIRMED
- G/delete_rule.py:17-18 WORKTREE_FOLDERS = `H\.claude\worktrees` and `C:\Users\Patrick\.claude\worktrees`.
- G/cleanup_sweep.ps1:174-178 Get-SweepPrefixes: `<root>\.claude\worktrees` in any repo, plus `<root>\worktrees` in the hooks' own repo. Different set.
- H/tools/new_worktree.ps1:29 makes `<main>\.claude\worktrees\<TaskId>`.
- G/README.md:136 "Keep it under a `worktrees` folder: the delete rule trusts only those"; ~/.claude/model-postures.md:5 same words (the finder cited it as G/model-postures.md; the file is at C:\Users\Patrick\.claude\model-postures.md, the finder's path is wrong, the text is right).
- Situation where they differ: a worktree at `H\worktrees\<task>` (this audit's own) is outside both delete_rule folders; the docs' "a `worktrees` folder" wording would suggest it passes. Whether the hook then refuses is a run to settle (a delete from that folder by a subagent).
- Ruling: none read as naming both sides.

### I9-017 / I9-192 / I10-034 / S3 build-slot and bench folder: CONFIRMED-CHANGED
- Typed `.claude\hooks\state` at H/tools/build_slot.ps1:10 and :78, H/tools/bench_run.ps1:8-9 and :20, H/tools/compare_db.py:43. Confirmed.
- The finder said `C:\Users\Patrick\.claude\hooks` no longer exists. At my read it does exist: `hooks\state\build_slots` was made today (folder 21:21, build_slots written 21:30). So the default in build_slot.ps1 is live, not dead: locks go to hooks\state\build_slots while G\state (agent_stop.jsonl, agents, posture, ...) has no build_slots folder. The cleanup sweep (prunes only G\state subfolders) does not reach hooks\state. The split is real and stronger: two state roots in use.
- Situation: any build pass (slot locks) and bench run.

### I10-072 / S5 which Visual Studio install: CONFIRMED-CHANGED
- find_cmake.ps1:10-17: `vswhere -latest -products * -requires ...VC.Tools.x86.x64`; only if that returns nothing, `-all -prerelease -products *` (the finder said it prefers -latest then -all: right). Also tries PATH first (:6-7).
- H/build_cpp.ps1:60-63 (the file is at the repo root, not tools/; the finder wrote only "build_cpp.ps1"): `-all -prerelease ... -format json`, sorted by installationVersion descending, newest; comment says `-latest` skips an install with a pending update.
- ci.yml:326: `vswhere -latest -all -products * -property installationPath`.
- Change: find_cmake handles the pending-update case itself (fallback), so find_cmake and build_cpp differ only when a complete older install and a newer complete install both exist: -latest = newest complete one, build_cpp = newest by version among all. Plus ci.yml third form. Differ: an install with a pending update, or two installs.

### S4 HYDRA_MUTANT: CONFIRMED (meaning left open)
- C++ H/tools/mutate/hydra_mutant.h:41-53: unset gives -1; empty, non-number, or negative aborts with a message.
- Python H/tools/mutate/py_mutate.py:63: `int(os.environ.get('HYDRA_MUTANT') or 0)`: unset and empty both 0; a non-number raises ValueError; a negative is accepted. Its own comment (py_mutate.py:55-56) says "none when unset", which the code does by 0, not by a none value.
- Open: whether switch 0 is ever a real switch number (I grep'd rewrite.py/run.py/hydra_mutant.h for numbering and found no statement). A run or a read of rewrite.py's numbering settles it.

### I9-176 / I9-175 harness proof test: CONFIRMED (K8)
- G/tests/lib/harness.ps1:11 "Its proof is tests\test_h39_harness.ps1." and :21 "Only test_h39_harness.ps1 changes them". grep of G for h39 hits only those two lines; `ls G/tests | grep h39` empty. The named proof file is missing.

### I9-136 realapp.jsonl: CONFIRMED (K8)
- H/tools/hydra_ui_mcp.py:78-81 defines REALAPP_FILE = "realapp.jsonl" ("The hooks repo's ref gate reads it, so its field names are a contract").
- G/review_rules.py:44-46: comment "Hydra's tools/hydra_ui_mcp.py owns its name (REALAPP_FILE) and its fields (realapp_record); this file only reads them", then types `REALAPP_FILE = "realapp.jsonl"`. Two literals in two repos; the comment says one owns it. No test found by me holding them equal (I searched only test names in the finder's report; I did not search the tests).

### I10-055 library folder claim: CONFIRMED (K8)
- hydra_ui_mcp.py:17 "snapshot_library, the only code that knows that folder's path" and :140 "This is the only place that knows the library folder's path".
- H/tools/bench.cpp:975-986 reads the Program Files variables and also pushes `C:\\Program Files\\Hydra`. So bench.cpp also knows it. (src/ not searched by me either.)

### I10b-100 / S6 replay.cpp vs replay.h: CONFIRMED (K8)
- H/tools/replay.cpp:28-29 "The scoring itself lives in core/replay.h so this tool and tests/test_replay share one implementation."
- H/src/core/replay.h:1-12 "It owns no scoring rule" and points at chord_score_table (core/scoring.h).
- The "share one implementation" half is not tested by me (finder says tests include core/replay.h; not rechecked).

### I10b-058 / S7 "ONE place addresses live": CONFIRMED (K8)
- tools/ch_probe/constants.py:3-7 "This is the ONE place the numbers live. Every other module imports from here; nothing else hard-codes an address."
- Hard-coded addresses elsewhere, all read: ch_mcp/agent/menu.ts:54 `PLAYER_SCREEN_FRAME_RVA = 0x32F590`; ch_mcp/launch_patch.json:8 `0x3968D0`, :11 `0x396EC5`; ch_mcp/start_candidates.json:19 rva `0x8D090`. None of those four values is in constants.py (grep of that file empty for them). Different folder (ch_mcp vs ch_probe), so a reader could say the claim is about ch_probe only; the sentence says "nothing else".

### I10b-084 / S2 decisions: CONFIRMED
- derive-once-review.md:45 lists "a line under `## Confirmed` in `docs/agents/rulings/`" as a decision home.
- H/tools/precheck/decisions.h:55-63 lists docs/adr/*.md, CONTEXT.md, plan "User decisions" and "Test limits" sections, and decision-log D-records; `grep -c rulings` gives 0 in decisions.cpp and decisions.h. The doc says Git does not check the other places so the reviewer does. "Test limits" is read by the precheck and is not in the doc's list on line 45.

## Splits added
- S3 stronger form: two live state roots (G\state and `.claude\hooks\state`), the second recreated today by the build slot.
- S8 (new, small): model-postures.md lives at C:\Users\Patrick\.claude\, not in gates; the finder's row I9-061 and I9-121 path "G/model-postures.md" is wrong. Not a split of rules, a citation error.
- S9 (new, from reading): py_mutate.py's comment says "none when unset" while the code uses 0; C++ uses -1 (inside S4).

## Rulings
No `## Confirmed` ruling was opened by me for these rows; I did not search docs/agents/rulings. I make no claim on rulings beyond the finder's "none".

## Not reached
All other rows of F11 (I8-237, I8-238, I9-001, I9-018, I9-023, I9-038, I9-068/069, I9-072, I9-080, I9-107, I9-110, I9-121, I9-151, I9-161, I9-178, I9-184/185, I10-001 to I10-021, I10-048, I10-056 to I10-058, I10-073 to I10-079, I10b-015 to I10b-025, I10b-064, I10b-105 other than S4, I10b-110), and I8-255 (finder did not reach).
