# Premise maps: what a plan's facts stand on

A premise is a fact a plan uses but does not build. "A kit on Player1 makes a drums player" is one. "Window Map has Star Power phrases" is another. Both once sat in a plan as flat sentences, nobody opened their source, and both were false, so live runs were lost to them. A plan already proves every promise it makes in its Promise map. The Premise map is the other half: it proves the facts the plan stands on.

This page is the one home of the map's shape, and of the shape of a live step's call list. The planner's instructions and the plan reviewer's instructions point here, the same way the preamble points at `docs/agents/handoff.md` for a handoff's Claims table.

## The rule

A fact either names its source or it is marked `unchecked`. An unchecked premise never carries into a task or a live step as settled. It becomes a task that checks it, or a question for the user. A task or live step that uses an unchecked premise names its row.

## The shape

A plan ends with one table, under a heading that is exactly `## Premise map`. Every fact the plan depends on gets a row. The table has four columns.

| Premise | Kind | Source | Status |
|---|---|---|---|
| one fact the plan depends on, in one sentence | exactly one of the six Kind words below | the exact place the fact was read, in backticks | `checked` and a date, or for an `unchecked` row the task or question that settles it |

Start each premise with a short id, such as `P1:`, so a task or step can name its row.

## Rules for the table

A row has one Kind. A fact with two sources of two kinds is two rows.

A row is either checked or unchecked. There is no half-checked status. If part of a fact was never opened, that part gets its own `unchecked` row.

**Source.** The Source cell names the exact place: a repo path with a line number or a block id, a game file path, a chart path, or a run's result folder. It holds at least one backticked token, because a cell with none is a shape error. It may hold several backticked tokens, with plain words around them.

**Status.** A row of any Kind but `unchecked` has a Status that starts with `checked` and a date, such as `checked 2026-10-31`. Words after the date may say what was opened. An `unchecked` row's Status names how it gets settled: either the id of a task in the plan (one with a `### <id>:` heading), or `question N`, where N is the number of an item under the plan's heading that starts `## Questions`.

## The six Kind words

Each word says what kind of thing the fact was read from.

- `code` is a file read directly, where the premise is what that file says or does. That covers Hydra source, a script or config in the hooks repo, an instruction or rules page the plan obeys or changes, a listing under `docs/clone-hero/code/`, and the game's `dump.cs`.
- `data` is a game data file, or a block on a Clone Hero page under `docs/clone-hero/` whose Label is "Proven in the game's code" or "Proven in a live run". A block labelled "Hydra assumes it, never checked" or "Unknown" is not a source. `docs/clone-hero/README.md` says what each label means.
- `chart` is a chart file.
- `run` is the result folder of a live run, kept under `hydra-data\ch-mcp-runs\`.
- `ruling` is the user's own words, quoted exactly. The quote must sit in a file under `docs/agents/rulings/`, in a session card's quoted words under `docs/sessions/`, or in the user's message as the main session quotes it in the reviewer's brief and confirms. A paraphrase of what the user wanted is not a ruling; a fact that rests on one is `unchecked`.
- `unchecked` is a fact with none of the sources above.

## What counts as a source

A premise about a file's own content is sourced by that file. "The planner's instructions say nothing about premises" is sourced by the planner's instruction file.

A premise about the world (the game, the code, a past run) is sourced by the thing itself. A file that only reports on it does not count. That means a handoff, another plan, a session card, a memory note, or a Clone Hero block with no proof label. A row whose only source is one of those, or nothing, is `unchecked`, and its Status must name a task in the plan or a question number.

## The call list of a live step

A live step is a heading that starts `### LS` followed by a `-` or a digit, such as `### LS-D0` or `### LS2`, inside a section whose heading starts `## The live steps`. A struck heading, one that starts `### ~~LS`, is not a step.

Under every step heading, before the next heading, one line starts with `Calls:`. It lists every tool call the step makes, in order. Each call carries one of two markers:

```
Calls: `ch_start` (seen: LS-D0 run 1), `ch_attach` (seen: LS-D0 run 1), `ch_kit_setup` (never tried), `ch_stop` (seen: LS-D0 run 1)
```

"Seen" means the call ran on this exact target before: the same player, the same route and the same chart. The run id names the step and run that showed it, such as `LS-D0 run 1`. Any call that has not run on this exact target is "never tried".

A step holds at most one `(never tried)` call. A step with two or more is a shape error, and its author splits it into steps of one new piece each. That way a failed run names the one piece that failed, instead of leaving a chain of suspects.

## Who checks a map

The planner writes the map and the call lists as it writes the plan.

The plan reviewer checks the facts twice. First it opens each `checked` row's source and confirms the fact is there; a row whose source does not say what the row says is a finding. Then, with the map closed, it reads each live step and each task cold and writes down every fact it depends on. A fact with no row is a finding, fixed by adding the row, as `unchecked` when the reviewer cannot source it.

The docs test, `tests/test_docs_match_code.cpp`, holds a plan to this shape at every merge to `main`; the comment above its Premise map check says which plans count. It checks the shape only. Whether a source says what its row says is the reviewer's job, because a test can tell that a file exists but not what a line in it means.

Before a live run, the main session shows the user the step's call list. "The shape of a live run" on `docs/agents/clone-hero-mcp.md` says what that go request holds.
