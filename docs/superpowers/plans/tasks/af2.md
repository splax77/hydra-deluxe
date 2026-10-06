Read `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md` first; it holds the rules every agent follows.

# Task AF2: one owner for the chart's note total, and the generated-fill rules written down (D74, findings 255 and 315)

Task id: AF2. Base: main at d05c9a6. Session for your trailers: 18216767-3c3e-4c72-8dd2-20d65e2f152c.

Make your worktree and first build with `pwsh -NoProfile -File C:\Users\Patrick\Downloads\Hydra\hydra-test\tools\new_worktree.ps1 -TaskId af2 -Base d05c9a6 -Target hydra_tests`. It makes `.claude\worktrees\af2` on branch `claude/af2` and builds inside the shared build slot. Work, build and commit only there. Later builds are warm: `.\build_cpp.ps1 -Target <target>` from the worktree.

The decision is D74's "code-only calls" paragraph in `docs/audit/2026-10-03-fix-decisions.md`. The findings are 255 and 315 in `docs/audit/2026-10-03-derivation-audit.md`; each row's `status_evidence` in `docs/audit/2026-10-03-fix-triage.json` says what a scan found still there on 2026-10-05. Read all three. D26 and D27 cover the related fill rules already recorded.

## Goal

The chart's note total (at the chosen difficulty and drum options) is worked out in one place, on `Song`, and the engine and the Dynamics tab read it. Every number shown or stored stays exactly the same. Separately, the two generated-fill rules nobody wrote down are recorded where the other fill rules live. Nothing that changes a score, a path, a stored byte or a stamp is in scope.

## What the code does today

Finding 255 names three counts. `ScoreGraph::build` (`src/search/graph.cpp`) puts each chord's note count on the graph edges, and `Engine::advance` (`src/search/engine.cpp`) adds them up into `Path::notecount`, which is stored (`src/store/path_codec.cpp`, and the summary's `notecount` column in `record_store.cpp`) and shown as the Notes column. `count_dynamics` (`src/app/dynamics_breakdown.cpp`) walks the song again, and `DynamicsBreakdown::played_total(bass2x)` gives the Dynamics tab's "of N". `replay_path` (`src/core/replay.cpp`) keeps a running combo; that is a running count over time, not the chart total, and stays as it is. `Song::sp_phrase_count` in `src/parse/song.h` is the shape of owner the finding proposes. The finding notes one known split: a 2x kick that shares a tick with a normal kick. Find out exactly how each count treats it before you change anything.

Finding 315's leftovers, per the scan: in `Song::check_activations` (`src/parse/song.cpp`), a distance tie between two chords goes to the later chord (`dist <= bestdist`), and the distance and the generated fill's length are cut down to whole ticks. Neither rule is written in `CONTEXT.md` or `docs/adr/0023-chart-rules-follow-clone-hero-code.md`, where the authored-fill rules (D22, D30) and the unverified ones (D26) already are.

## What changes

1. **A Song-level note count (255).** Add one function on `Song`, beside `sp_phrase_count`, that answers "how many notes does this chart have at this difficulty and these options". Use the same per-chord count the graph uses today, so the two cannot differ. The engine's stored `Path::notecount` comes from it instead of an edge sum, if every path always covers the whole chart. Check that first, with the code and a test; if some path can stop early, keep the edge sum and add a check that a full path's sum equals the Song count instead. The Dynamics tab's "of N" reads the Song count, or a test pins that `played_total` equals it on every corpus chart under both `bass2x` settings, whichever keeps one owner without moving a number. **If the Dynamics total and the Song count disagree on any chart today, stop and report the chart, the two numbers and why. That would be a visible change and needs the user.** Add a row to `tests/test_single_owner.cpp` for the question.
2. **The generated-fill rules written down (315).** Add one sentence each, in plain English, to the section of `docs/adr/0023-chart-rules-follow-clone-hero-code.md` that lists rules not verified against Clone Hero, and to `CONTEXT.md` where the fill rules live. The sentences name the function that owns each rule (`Song::check_activations`); per the preamble they point at the owner, they don't restate the code's comparison. Don't change the code of `check_activations`.

## Owned files (only these may change)

- `src/parse/song.h`, `src/parse/song.cpp` (the new count only; `check_activations` is read-only)
- `src/search/graph.h`, `src/search/graph.cpp`, `src/search/engine.h`, `src/search/engine.cpp` (only where the note count is summed or set)
- `src/app/dynamics_breakdown.h`, `src/app/dynamics_breakdown.cpp`
- `src/ui/dynamics_tab.cpp` only if the "of N" must read the new owner there (task AF1 changes one `count_label` call in this file at the same time; touch no other line, and say in your report if you touch it at all)
- `tests/test_song.cpp`, `tests/test_search.cpp`, `tests/test_dynamics_breakdown.cpp` (or whichever test file already holds `count_dynamics` cases; grep first), `tests/test_single_owner.cpp` (your row only)
- `CONTEXT.md`, `docs/adr/0023-chart-rules-follow-clone-hero-code.md`

If you need another file, stop and report which one and why.

## Test cases

Pin literals; never recompute the expected count in a test.

1. In `tests/test_song.cpp`: the new count on a small hand-written chart, pinned as a literal, including a 2x kick on the same tick as a normal kick, with 2x Bass on and off.
2. In `tests/test_search.cpp`: every path's `notecount` equals the Song count on one corpus chart (name it), pinned as the literal the run gives.
3. A corpus check across every chart under `testdata\input` (the same set `tests/test_search.cpp` or the corpus helpers already walk): the engine's count, the Song count and the Dynamics total agree, under both `bass2x` settings. If the check would be slow, say how long it takes in your report.
4. `-tc="single-owner*"` passes with your new row.

If the engine change replaces an edge sum, write case 2 first and keep its result from before the change as your "red" (it should already pass; say so). That shows the number didn't move.

## Test filters

- `build-cpp\Release\hydra_tests.exe` with `-sf=*test_song.cpp*`, `-sf=*test_search*`, the Dynamics test file's `-sf=`, and `-tc="single-owner*"`.

Nothing else. Never the full suite. The main session runs the corpus score comparison at merge.

## Not in this task

- The replay's running combo.
- Any change to how fills are placed. Finding 315 is recorded only.
- Task AF1's display fixes (8, 14, 55, 218), running at the same time in another worktree.

## Done when

- One Song function answers the note total, and the engine and the Dynamics tab read it or are pinned against it.
- The two fill rules are written in ADR 0023 and CONTEXT.md.
- The cases above pass, and no shown or stored number moves.
- `git diff --stat d05c9a6..HEAD` lists only owned files.

## Commits

One commit per step is fine. Trailers: `Task: AF2`, your agent id, the session above, and `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`. Report as the preamble says, and list the questions your change answers with each one's owner.
