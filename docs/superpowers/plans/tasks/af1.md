Read `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md` first; it holds the rules every agent follows.

# Task AF1: the audit's last display leftovers (D74 items 1 to 4)

Task id: AF1. Base: main at d05c9a6. Session for your trailers: 18216767-3c3e-4c72-8dd2-20d65e2f152c.

Make your worktree and first build with `pwsh -NoProfile -File C:\Users\Patrick\Downloads\Hydra\hydra-test\tools\new_worktree.ps1 -TaskId af1 -Base d05c9a6 -Target hydra_tests`. It makes `.claude\worktrees\af1` on branch `claude/af1` and builds inside the shared build slot. Work, build and commit only there. Later builds are warm: `.\build_cpp.ps1 -Target <target>` from the worktree.

The decision is D74 in `docs/audit/2026-10-03-fix-decisions.md`. The findings are 8, 14, 55 and 218 in `docs/audit/2026-10-03-derivation-audit.md`; each row's `status_evidence` in `docs/audit/2026-10-03-fix-triage.json` says what a scan found still there on 2026-10-05. Read all three.

## Goal

Four small display fixes the user approved. Each one makes a screen read a fact from the owner that already exists, instead of its own copy. No score, path, stored record or stamp changes.

## What the code does today, and what changes

1. **Hover teal (218, D74 item 1).** `src/ui/theme.h` keeps `kButtonHoveredColor` (0,104,104) and `kFrameHoveredColor` (0,100,100). `src/ui/theme.cpp` uses the second for `FrameBgHovered` and `HeaderHovered`. After the change there is one hover teal, 0,104,104, used by all three slots. Keep one constant; don't alias a second name to it unless something outside theme needs the old name. `tests/test_single_owner.cpp` has a row about these colours (around line 888); update it to match one owner.
2. **DMBot names on the leaderboard page (8, D74 item 2).** `collect_dm_rows` in `src/app/dm_report.cpp` (around lines 237-249) copies DMBot's `song_name`, `artist` (and charter, if present) raw when the leaderboard knows the song, and runs Hydra's own names through `display_title` / `display_artist` otherwise. After the change, both branches go through the same owners in `src/parse/song.h`. Don't add a new strip function.
3. **Scan dialog counts and the count_label wrapper (14, D74 item 3).** The scan modal in `src/ui/library_dialogs.cpp` (around lines 218-224) prints "(%d found)" and "%d unchanged since last scan, reused." with raw `%d`. After the change those counts use `group_thousands` or `counted` from `src/core/model.h`, so 1234 reads 1,234. `count_label` (library_dialogs.cpp line 64, declared in `src/ui/library_parts.h`) only forwards to `counted`; delete it and have its callers (`src/ui/dynamics_tab.cpp`, `tests/test_batch_text.cpp`) call `counted` directly. If the scan modal's text is built inline in render code, put the text in a small function a unit test can call, beside where the batch confirm's text is already built (grep `test_batch_text.cpp` for the pattern).
4. **The "1.0 fills" tooltip (55, D74 item 4).** `src/ui/settings_bar.cpp` (around lines 101-107) writes its own sentences about the 1.1 and 1.0 deadlines. After the change it reads exactly: "Spawn drum fills by Clone Hero 1.0's rule instead of 1.1's. A fill only appears if your Star Power was ready in time. " + `fill_rule_description(FillDeadlineRule::Ch11)` + " " + `fill_rule_description(FillDeadlineRule::Ch10)` + " For runs played on 1.0; current Clone Hero plays by 1.1." The rule names inside come from `fill_rule_name` as today. The scan also noted that no test checks the batch confirm's "Fills" line; add one in `tests/test_batch_text.cpp` that pins it as a literal.

## Owned files (only these may change)

- `src/ui/theme.h`, `src/ui/theme.cpp`
- `src/app/dm_report.cpp` (`collect_dm_rows` only)
- `src/ui/library_dialogs.cpp`, `src/ui/library_parts.h`, `src/ui/dynamics_tab.cpp` (the `count_label` call only)
- `src/ui/settings_bar.cpp` (the "1.0 fills" help text only)
- `tests/test_dm_report.cpp`, `tests/test_batch_text.cpp`, `tests/test_single_owner.cpp` (your rows only)
- `tests/ui/uitest_library.cpp` only if an existing script there pins the old tooltip or scan-dialog text

If you need another file, stop and report which one and why.

## Test cases with red lines

Write each red first, then green, and keep the red line. Pin text as literals.

1. In `tests/test_dm_report.cpp`: "a leaderboard row's DMBot names lose their Clone Hero tags". A DMBot song record whose name and artist carry a tag (use a tag the existing `display_title` tests already use). The row's song and artist are the cleaned text. Red line: the raw text today.
2. In `tests/test_batch_text.cpp`: the scan dialog's two count lines at 1234 read "1,234". Red line: whatever today's text gives (a compile failure counts if the function is new; quote it).
3. In `tests/test_batch_text.cpp`: the batch confirm's Fills line, pinned as a literal for both rules. No red needed; it pins today's behaviour.
4. The tooltip: if a unit test can reach the help text, pin the full sentence. If only a GUI script can, check `tests/ui/uitest_library.cpp` for one that reads it and update that; otherwise say so in your report.
5. `tests/test_single_owner.cpp`: `-tc="single-owner*"` passes with your updated hover row.

Existing tests that must pass unchanged: `-sf=*test_dm_report*`, `-sf=*test_batch_text*`, `-sf=*test_report*`.

## Test filters

- `build-cpp\Release\hydra_tests.exe` with `-sf=*test_dm_report*`, `-sf=*test_batch_text*`, `-sf=*test_report*` and `-tc="single-owner*"`.
- `build-cpp\Release\hydra_uitest.exe --test <name>` only for a script you changed (build with `.\build_cpp.ps1 -Target hydra_uitest` first).

Nothing else. Never the full suite, never `hydra_uitest --all`.

## Not in this task

- Any other colour, count or sentence. Finding 255 (note totals) and 315 (fill rules docs) are task AF2, running at the same time in another worktree.

## Done when

- The four changes above are in, each reading its existing owner.
- The cases above pass with their red lines recorded, and the named existing tests pass.
- `git diff --stat d05c9a6..HEAD` lists only owned files.

## Commits

One commit per item is fine. Trailers: `Task: AF1`, your agent id, the session above, and `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`. Report as the preamble says, and list the questions your change answers with each one's owner.
