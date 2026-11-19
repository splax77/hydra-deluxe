# TOOL-AUDIT-T5-V10 (verifier of finder F10), all claims "by reading"

Checked the finder's headline candidates and splits against the files. Many K1 copy rows (typed Python path, gh path, `.claude\gates` path, 98 count) were only spot-checked; see "Not reached".

## Splits (finder's six, each checked)

1. Live-DB copy method: CONFIRMED. this-machine.md:81 "So copy `hydra.db-wal` and `hydra.db-shm` along with `hydra.db` whenever they are there. The bare `hydra.db` alone can miss the newest writes." ui-testing.md:195: "A plain copy of a database another program is writing can catch it halfway through a write, so the helper copies `hydra.db` with SQLite's online backup instead... whenever a `hydra.db-wal` sits beside it." hydra_ui_mcp.py:112-134 (`copy_database`: backup when -wal exists, plain copy when none) and compare_db.py:85 (backup API). Situation where they differ: the installed Hydra is open and writing. Closed app: all agree on a plain copy. Added nuance: ui-testing/helper also say a plain copy is right when no -wal exists. No Confirmed ruling names either side (none found by the finder; I did not re-grep rulings).
2. PowerShell version: CONFIRMED. this-machine.md:51 "PowerShell 7.6.9"; `pwsh` here reports 7.6.10. Moment: machine upgraded after the page's check date. Needs a run only to say when; not run.
3. hydra-deluxe-rename note: CONFIRMED. Description (line 3): "repo now patseguin1-commits/hydra-deluxe". Body (line 11): moved to "`splax77/hydra-deluxe` (current home...)". `git remote -v` origin is github.com/splax77/hydra-deluxe.git. Description is stale, body and remote agree.
4. SP-37 owner claim: CONFIRMED. sp-squeeze-mechanics.md description: "the game's SqIn/SqOut rule is on reference page 05 (SP-37)"; body: "The game's side of a squeeze is on docs/clone-hero/05-star-power.md (SP-37)". SP-37 (05-star-power.md:403-407) says "Label: Hydra assumes it, never checked" and "checked only against Hydra's code and full-combo videos, never against the game's code". Its Game code line lists only RVAs of SP-36 and SP-29. The ruling in the entry is about the hit window value only. The note's 500 ms is a different matter (the finder cited rulings/clone-hero.md:12; not re-read by me).
5. Render-check data: CONFIRMED-CHANGED, weak. Note line 3: "not done until it is seen in the real app on the user's library and ini ... render checks run on testdata/input, never a whole-library analysis" so the description itself holds both halves. Line 21 and ui-testing.md:11 agree: corpus for analysis-backed drawing, real copy for the Library view only. It is a wording tension inside one sentence, not a split between homes. Different questions: what to look at vs what to analyze.
6. `--replay` vs `--hydra-replay`: CONFIRMED-CHANGED. The skill line 18 (analyzing-sp-squeezes) speaks of squeeze_scout, whose own text (squeeze_scout.py:153) says "--replay at it". fcvideo.py:2906 uses `--hydra-replay`. So two different tools use two flag names for the same thing (K6/K2 across tools), not the skill disagreeing with its own tool. I did not open squeeze_scout's argparse.

## Other candidates

- K8 analyzing-sp-squeezes skill names memory note `gameplay-video-score-analysis`: CONFIRMED. SKILL.md:34 names it; the file exists only in memory/archive/ and MEMORY.md does not index it.
- K8 windows-installer.md "That is the user's design": CONFIRMED as unsupported. Note line 15 says it; grep of docs/ for `users-modify` returned no file (so no ruling, decision log or card text names it). Searched docs only, not session cards outside docs.
- K8 this-machine.md as home of machine paths: CONFIRMED. this-machine.md has no dumpbin or ffmpeg line (grep). Only mimalloc-link-order.md, video-decode-ceilings.md and stitched-chart-auto-fills.md hold those names in memory.
- K6 executor worktree: CONFIRMED. git-traps note line 16 types `git worktree add .claude\worktrees\<name> -b claude/<name> main`; tools/new_worktree.ps1 does the same job (header, branch claude/<TaskId>, default base main). The note does not name the script.
- K5/K7 executor isolation: CONFIRMED by reading. Only review_dispatch.py (line 149-150) contains an `isolation` check, and it denies reviewer dispatches; no other gate file mentions `isolation`. Executor half is prose only.
- K6 installer file name: CONFIRMED. hydra.iss:65 `HydraDeluxe-{#HYDRA_VERSION}-setup` and build_installer.ps1:128 `"HydraDeluxe-$version-setup.exe"` each spell the name. Situation: the script looks the file up by its own copy of the name; an edit of one breaks the other.
- K6 hydra_replay.exe default path: CONFIRMED in part. video-tools/replay.py:30 ends `\Release\hydra_replay.exe`; squeeze_scout.py references hydra_replay (path line not re-read).
- K8 "only place that knows the library folder's path" (hydra_ui_mcp.py:55): CONFIRMED-CHANGED. The path is also typed in compare_db.py:51 and :87, but those are docstring command examples for agents, not code paths. Also typed in this-machine.md:77 and ui-testing.md:350.
- K4 REA JAR/sigrok/WABT: not contradicted. this-machine.md grep for REA_/JAR/sigrok/WABT returned nothing. By reading the skill text I did not re-open.
- "98" count: CONFIRMED present: test-rubric.md:11 ("A 98th chart was added..., and tests still expected 97"), ui-testing.md (one "98 charts" hit in my count, finder says two lines), uitest_batch_reports.cpp has 5 lines with 98. Test holds the cpp to the corpus, not to the docs.
- video-decode-ceilings K8: PARTLY. Grep of video-tools/README.md for 3,000/1,900/3000/1900 found nothing, supporting the finder; I did not read the README table cells (the finder also did not).
- delete_rule.py WORKTREE_FOLDERS side note: CONFIRMED. Lists `...\hydra-test\.claude\worktrees` and `C:\Users\Patrick\.claude\worktrees` only; this audit's worktree is under `hydra-test\worktrees`. A hook gap exists there.

## Splits the finder missed
None new. One framing note: this-machine.md:81 (copy -wal and -shm as plain files) is itself the plain-copy method the helper says can catch a half-written database.

## Rulings
No Confirmed line was found by me naming both sides of any split. I re-read none of the rulings files; I rely on the finder's statement that none speaks to its rows.

## Not reached
The 20 typed copies of the Python path, gh path scope check, Visual Studio build numbers, hydra-deluxe-rename AppUserModelID, the one-reviewer note cap number, the REA skill variables, the dmleaderboards URL, handoff-name check, domain.md read order (Group A), doc_shots (Group B). These were taken from the finder unchecked.
