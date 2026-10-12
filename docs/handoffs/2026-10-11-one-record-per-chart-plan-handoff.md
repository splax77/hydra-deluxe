# Handoff: plan the one-record-per-chart change (D115), 2026-10-11

The design is done and approved in chat, piece by piece. Nothing is built. The next session writes the build plan, brings it to the user, and builds only after the user says yes (CLAUDE.md rule 3). Planning agents run on Fable (rule 2).

## Start here

Read the spec first: `docs/superpowers/specs/2026-10-11-one-record-per-chart-and-profile-design.md`. It was committed on main as 7cd003f7 and updated through the commit that adds this handoff; none of it is pushed. Its opening list holds every user decision, and its sections give the reasons.

In one paragraph: `hydra.db` replaces its `results` table with three tables. `profiles` has one row per settings combination ever analyzed. `records` has one row per chart and profile. `record_paths` holds each record's top 5 paths in plain columns, and the best path's row doubles as the summary. The path report then reads stored rows instead of analyzing the library each time it opens. On top of that, every question the user listed gets exactly one owner in the code, guarded by a single-owner scan rule (ADR 0029). The list: what is a chart, a record, optimal, a squeeze, a backend, what a note is worth, how a score is computed, the average multiplier, the score breakdowns, a tick, a tempo, a section, a time signature and a measure. Several labels change on screen, all chosen by the user.

The work started from `docs/handoffs/2026-10-11-path-report-speed-handoff.md`. That file and `docs/superpowers/plans/2026-10-11-profiler-flame-graph.md` are still untracked in the main checkout. They belong to the earlier session, so this session left them alone.

## Still in flight when this was written

The session's journal reported these subagents as active and unfinished:

```
agent-a5f924a798b2e7a71: unfinished, last tool call SubagentHandback, touched 22:41
agent-aa2b78f51f58d558c: unfinished, last tool call SubagentHandback, touched 22:58
```

Both had already handed back their final reports (tasks CH-meter-sections and CH-fill-deadline), and the spec uses both.

The journal does not list a third scout, task CH-sp-grid (agent a19cd3e5420efe386). The main session started it after the user chose Clone Hero's measure grid. Its report had not arrived when this handoff was written. Its last status line read: "found SP update fn 0x20f6800: drains SP(+0x68) by chart-tick delta (cur tick - prev tick), via 0x20f3750 time->tick | next: verify ctor constants, TS-independence, read Hydra side".

Its question is whether Clone Hero's Star Power drain counts measures on the same grid as its beat lines. The answer decides whether Hydra's Star Power length in measures (`plusmeasure`, `kMeasuresPerSpBar`) moves to the new grid. If the drain really runs on ticks and ignores the time signature, Hydra's two-measures-per-bar model may disagree with the game outside 4/4. That would be a new finding needing its own user decision. Don't assume either way. If the report never arrives, run the scout again from the brief in this session's transcript; the spec's section 9 names the question.

## What the plan has to cover

The spec is large, so the plan splits it into executor-sized tasks: one Opus executor each, inside 100 tool calls and 20 minutes (rule 7). Each task gets its own worktree and its own files. Independent tasks run in parallel waves (rule 6). One natural split follows. The planner owns the final one and must check file ownership, because several pieces touch the same files.

The **store** is the biggest piece and probably needs three tasks:

- the new tables and the open-time upgrade
- the one save function and the status lookup over profiles
- `ChartIndex`, the one owner of library charts, copies and naming

`Settings::profile()` is the one builder of a profile. Everything else waits on the store's interface, so the plan should fix that interface first.

The **report** reads stored rows. Its fill pass fills converted records, counting through `ChartIndex::count`. It shows the new labels and the "N charts" header. Its window keeps each cell's fitted text instead of fitting it every frame (the speed handoff's part 4).

The **callers** move to the new store: the batch, the click's save (which also marks an open report out of date), both comparison pages, `tools/bench.cpp` and `tools/compare_db.py`.

The **scoring, optimal and squeeze owners** are code-only:

- `HydraRecord::optimal_score()`
- one engine helper for the SP adds
- one "crosses a multiplier step" function
- one "cymbals only under Pro Drums" function
- the renames and comment fixes
- the test that pins `Engine::act_within_limit` to `Activation::within_ms_limit`

**Timing** moves to Clone Hero's rules:

- one "in force at tick" lookup each for tempo, time signature and section
- Clone Hero's measure grid inside `MeasureIndex`
- one grid walker shared by the measure lines and the position text
- the beat number counting the grid's beats
- the `check_activations` walk removed
- generated fills starting on the measure line, with the +1 tick and Clone Hero's first candidate
- Clone Hero's section-name list behind one function
- `hydra_replay`'s dump printing positions as the screen does
- the "span" and "measure line" renames

This piece moves scores. So before merge, `hydra_replay` on `testdata/input` lists every chart whose fills or score change, the user sees that list, and the results stamp is bumped (ADR 0018).

The **Analyze button** is independent and can go in the first wave. It follows the status chip, reads "Analyze shown (N)...", and touches only the library's model and toolbar. Its design is the side session's note `analyze-button-follows-chip.md`.

The **documents**:

- ADR 0030, which supersedes ADR 0026 in part
- decision D115's text in `docs/audit/2026-10-03-fix-decisions.md`
- ADR 0010's history for the fill rule
- the new `CONTEXT.md` entries
- the docs-match markers for the star factors and constants
- `docs/UserGuide.md` (a public page, so it goes through the doc review)

## Evidence and where it lives

Clone Hero 1.1.0.6142 lives at `C:\Clone Hero\1.1 june\Clone Hero`. The two scouts read its `GameAssembly.dll` statically. Their notes, annotated disassembly, helper scripts and the 2,718-entry section-name list (`section_table.tsv`) are copied to `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-fill-evidence-2026-10-11\`. The build ships that list as a data file. The 233 MB Il2CppDumper output stayed in this session's scratchpad, which the cleanup sweep removes after 7 days. The recipe to rebuild it is in the memory note `ch-star-cutoffs`.

The main session checked these key instructions against the disassembly:

| What | Address (RVA) |
|---|---|
| The fill deadline's start-tick compare | 0x20DDC20 |
| The fill flag clear | 0x20DDC33 |
| `[+0x300] = resolution << 2` | 0x20DF7B7 |
| The generator's `+1` on the distance limit | 0x20D2C91 |
| The generator's nearest-chord call | 0x20D2D11 |
| The jump of 4 after a placed fill | 0x20D2E5E |

Still inferred, not proven:

- the meaning of the `+0x42` byte ("SP not active")
- that the practice menu shows the stored section names unchanged
- what calls the two sibling fill-pruning methods (0x20DE4C0 and 0x20DF3E0)

The size figures come from scratch copies of the user's installed `hydra.db`:

- today's results table: 8,847,360 bytes for 37,696 rows
- the same rows in a lean layout: 1,884,160 bytes

The path rows' size is an estimate. The plan's first store task measures it on `testdata/input`.

## Rules the plan must keep

Agents run only their own tests. The main session runs the full suite once, at merge (rule 1). Each change gets one fresh Sonnet reviewer, who fixes its own findings and signs off (rule 5, D107). Merges on main that touch code need the derive-once review (`docs/agents/derive-once-review.md`). Public pages need the doc review. Stage files by name in the shared checkout. Every commit carries Task, Agent and Session trailers.

The user sees the result in the real app on a copy of their database before merge (memory `ui-changes-need-real-app-check`). That check fills every record once, about 18,848 analyses (the user's file holds 18,848 charts with results). It is a full library run, so ask for it in chat first, naming the run count and the time (rule 8).
