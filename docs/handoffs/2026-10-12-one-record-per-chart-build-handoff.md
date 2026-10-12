# Handoff: build the one-record-per-chart change (D115), 2026-10-12

The plan is written, committed and fully answered. Nothing is built. The next session starts the build, but only after the user says yes to the plan. On 2026-10-12 the session asked "Should I start wave 1?", and the user answered by asking for this handoff, not with a yes (CLAUDE.md rule 3).

## Start here

Read the plan first: `docs/superpowers/plans/2026-10-12-one-record-per-chart.md`. It was committed on main as cfedc30d, and the user's answers went in with the commit after it. It holds the wave order, the overrides to the parts, the shared files, the main session's code-only decisions and the user's ten answers. Where it and a part disagree, the plan wins.

Each task's full detail is in one of four part files in `docs/superpowers/plans/2026-10-12-one-record-per-chart/`. `part-A1-store.md` covers the store, `part-A2-report-callers.md` the report, its callers and the Analyze button, `part-B1-scoring.md` the scoring owners, and `part-B2-timing.md` the timing rules. `analyze-button-design.md` is the side session's design note for the button. Four Fable planners wrote the parts, and every name in them was read at main 47dd4706. An executor greps again before it edits.

The design behind all of it is the spec, `docs/superpowers/specs/2026-10-11-one-record-per-chart-and-profile-design.md`. One of the user's answers changes the spec, so trust the plan over the spec on these points:

- The timing change bumps the results stamp to "2.8.0" and removes `fill_length_measures`, which changes the rules fingerprint. So nothing the upgrade converts can read Ready.
- The store has no "readable but old" stamp. `row_readable_sql()` in `src/store/record_store.cpp` is built from `kResultsStamp.accepted`. So the spec's "drop rows with an unreadable stamp" would have dropped all 37,696 of the user's rows.
- The user chose to keep old rows as Stale records instead. The `from_summary` column, the "does this record still need its paths" function and the report's fill pass, with its "Saving paths" bar, are all dropped.

Nothing is pushed. Main holds the spec, the earlier handoffs and the plan commits.

## Still in flight when this was written

The session's journal reported these subagents as active and unfinished:

```
agent-a449306f9e574421d: unfinished, last tool call SubagentHandback, touched 09:32
agent-a5165afcb0c14696b: unfinished, last tool call SubagentHandback, touched 09:32
agent-a93a2eb6af1162a72: unfinished, last tool call SubagentHandback, touched 09:30
agent-afe5a9fe728098315: unfinished, last tool call SubagentHandback, touched 09:32
```

All four are the planners: A1, A2, B1 and B2, in that order of ids, a93a2… being A1. Each one had already handed back its final report, and its part file is committed. None of them changed code. Nothing else is running.

`docs/handoffs/2026-10-11-path-report-speed-handoff.md` and `docs/superpowers/plans/2026-10-11-profiler-flame-graph.md` are still untracked in the main checkout. They belong to an earlier session, so this session left them alone.

## The first steps

1. Get the user's yes on the plan.
2. Dispatch wave 1 together, eight Opus executors, each in its own worktree: A1-0, A2-1, A2-5, B1-1, B1-2, B1-3, B1-5, and B2-1 on a new branch `d115-timing`. Each brief starts with `docs/agents/brief-preamble.md`. Then it gives the task id, session id, part file, the plan's overrides for that task, the owned files and the exact shared region (the plan's "Shared files" section), the tests it may run and the status rule.
3. If the first status lines show cold builds running past 8 minutes, hold back the last two dispatches of the wave, as the plan says.

## Things the plan asks the main session to do by hand

At the timing merge, the main session runs the score-check recipe from part B2 on `testdata/input` only and shows the user `changed.md`. Then it makes three edits. It bumps `kResultsStamp` to "2.8.0", with `accepted` holding only that stamp. It re-pins the four digests in `tests/test_perf_digest.cpp`, which fold the engine's output and are expected to change. And it re-pins the rules fingerprints in `tests/test_rules.cpp`.

A1-5 moves `tests/test_perf_digest.cpp` off `PreparedRow`, and its four digests must not change. That is the proof that the store's move changed no engine output.

After the timing merge, the main session edits `C:\Users\Patrick\Downloads\Hydra\video-tools\replay.py` line 213 to read the dump's new `label` (user's answer 10). That is a separate git repo.

A1-4's baseline entries in `tests/single_owner/baseline.json` need dates that stay inside the scan's 14-day limit (`kBaselineMaxDays`) until A1-5 removes them.

## Rules the build must keep

- Agents run only their own tests. The main session runs the full suite once per merge (rule 1).
- Each change gets one fresh Sonnet reviewer, who fixes its own findings and signs off (rule 5, D107). Code merges on main also need the derive-once review.
- `docs/UserGuide.md` is a public page and goes through the doc review in D1.
- Stage files by name in the shared checkout. Every commit carries Task, Agent and Session trailers; the main session's own commits use `Agent: orchestrator`.

The real-app check before the push needs a batch over a copy of the user's library, about 18,848 analyses (spec section 1). That is a full library run, so ask for it in chat first, with that count and a time (rule 8). The 2026-10-11 profile ran 37,691 analyses in about 17 s of wall time. One profile is about half of that, and Defender may double it.
