# Owner maps: which existing code answers each question

An owner map is a table a scout writes before a plan is written. It lists each question the change touches, in game words, and the code that already answers it. The planner builds only from that table. This page is the one home of the map's shape and of the two stops that go with it. Other pages point here and do not copy it.

## Why it exists

On 2026-11-14 three plans in a row for the Preview's Star Power gauge invented rules the engine already had. Each planner pictured a design first, then searched for a function shaped like it, and wrote its own rule when none matched. The user asked: "so why do planners keep failing to find the rules we already have? why are they inventing their own rules? how can we prevent this in the future?" Then the user approved the fix: "Yes, write them".

The map splits finding from planning. A scout that only finds the owners has no design to bend the search toward. A planner that may only use the map's rows has nothing to invent with.

## When a plan needs one

Every plan that changes what Hydra decides or shows needs an owner map. That covers screens, the engine, scoring, the Path Builder, stored records and tools that report on them. A plan that only moves files, edits docs or changes hooks needs none. The main session decides, and says which in the planner's brief.

## Who writes it

A scout writes it before the planner starts, in its own run. The main session briefs the scout with the user's requirement, in the user's words, and nothing of its own about how to meet it. The scout returns the map, and the main session puts the map, unchanged, into the planner's brief. The planner copies it into the plan under `## Owner map`.

## The shape

One table, under a heading that is exactly `## Owner map`, with four columns.

| Question | Owner | Its own words | Status |
|---|---|---|---|
| one question, in game words, such as "where does this squeeze's Star Power end" | the function, field or stored value that answers it, with its file and line, in backticks | the owner's own comment or doc line, quoted, saying what it answers | `found`; `split` when two places answer it; `carry out` when the engine works it out but keeps it nowhere readable; or `none`, with the search that found nothing |

Start each question with a short id, such as `O1:`. A plan task names the rows it uses by id.

A question is in game words, never in the shape of a design. "Where does this squeeze's Star Power end" is a question. "Which function returns the tick where payment stops" is a design, because it picks the answer's shape before looking.

The owner's own words are what make a row checkable. If the owner's comment says it answers something else, it is not the owner of this question, however close its name looks. `SPSqueeze::needs_no_timing` (`src/core/model.h`) is the example: its comment says which side of the SP end a note sits on is "not this one".

A question can have only one owner. If two places answer it, the row names both and its Status is `split`. Merging them is the plan's first task (agent rule 15).

## The stops

**A row with Status `none` stops the work.** The scout reports it, and the main session takes it to the user before any plan is written. Nobody writes a new owner for it, because a missing answer to a game question is the user's call, not an agent's. The one exception is a value the engine already works out during its search and keeps nowhere a screen can read. There the row names the engine code that works it out, and its Status is `carry out`. The plan then adds a task that carries that same value out, filled by that same engine code.

**A result that contradicts the user's words stops the work.** This holds for the scout, the planner and the plan's reviewer. If the map, the code or the plan's own working leads somewhere the user's words or the handoff say it must not go, the agent stops and returns a report, not a plan. The report quotes the user's words, names what contradicts them, and gives the file and line. A finished plan never carries a list of the ways it contradicts its brief. A reviewer who finds one signs off `BLOCKED` and does not resolve it in passing. The user decides which side is right.

## What the planner may do with the map

The planner uses each row's owner as it is. It never builds a new value out of several owners. It never adds a condition to an owner, and never uses an owner for a question its own words do not claim. If a plan step needs a question the map lacks, the planner stops and asks the main session for the row; it does not search and fill the row itself.

## Who checks it

The plan's reviewer opens each owner and reads its own words against the row's question. A row whose owner does not claim that question is a finding. Then the reviewer reads the plan's steps with the map closed and lists every question they answer. A question with no row is a finding. So is a step that composes owners or adds a condition. The reviewer fixes a finding by naming the row's owner instead. If no owner exists, the reviewer signs off `BLOCKED` for the user, as "The stops" says.
