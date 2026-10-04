# The squeezed-out phrase, the collected phrases and the rules are stored

> **Superseded 2026-09-27: Auto was removed.** The 2026-09-26 amendment below,
> about the Auto budget and the Auto ladder, no longer applies. Hydra still
> reads `auto_cap_ladder` and `auto_budget_s` in `hydra_rules.ini` but ignores
> them. The rest of this record stands.

Two facts about an activation were only ever known inside the search. The
first is which SP phrase it squeezed out. The second is which phrases it
collected while Star Power was running. The display layer needed both, so it
rebuilt them from the chart. That is the same drift ADR 0011 and ADR 0013
closed for the deactivation node and the cap-clamp anchor.

A record also never said which rules it was analyzed under. Once the user's
rule choices moved into hydra_rules.ini, a record from before an edit would
have read Ready while holding answers to a different question.

## The decision

The engine stamps both facts at copy-out. `Activation::sqout_tick` is the
phrase chord the path squeezed out when it took the SqOut branch. It was first
read off the deact edge's squeeze time; since D34 and D36 it is the chord the
engine offered that window (`sqout_phrase` in engine.cpp), written through
`Activation::set_sqout`.
`Activation::collected_phrase_ticks` is every phrase tick the path crossed on
the SP track, recorded in `advance()` and trimmed back past the squeezed-out
phrase in `create_deactivated_path`. A late-SqIn phrase and a cap-clamped
phrase both count, because the gauge received them.

The pather stamps `HydraRecord::rules_fingerprint` with the fingerprint of the
rules the run used. The fingerprint is FNV-1a 64 over one `name=value` line
per rule, each number written with 17 significant digits; a hash that lands
on 0 becomes 1, because 0 means "no usable rules" (`Rules::fingerprint()` in
src/core/rules.cpp; the user confirmed this format, D48, Q33).
The store writes it into the structure blob right after
the format version, so version and rules make one 12-byte head. That head is
what decides Ready, in C++ (`structure_is_current`) and in SQL
(`row_ready_sql()` in `src/store/record_store.cpp`). The rules part of both
compares one fingerprint, the store's `RulesStamp::fixed`.

Blob format 6, path-node format 4 and path-structure format 4 carry the three
fields.

## No fallback

A record written before this build has none of the three fields. It reads
Stale until it is re-analyzed. Nothing reconstructs the squeezed-out phrase or
the collected phrases from the chart, and nothing assumes an old record ran
under the default rules.

A record analyzed under other rules also reads Stale. It is not deleted. When
the rules are switched back, it reads Ready again.

When hydra_rules.ini is bad, the GUI opens its store with
`core::kNoRulesFingerprint`, which no rules value hashes to. Every row reads
Stale until the file is fixed, so nothing is shown as Ready under rules the
user did not choose.

## What this costs

Every stored record reads Stale once, after the upgrade. Every edit to
hydra_rules.ini makes the whole library Stale under the new rules.

The search pays one arena push per phrase crossed on the SP track, per path.
The limit was 3% on hydra_bench's corpus timing (best of two runs each, cap 4
and Auto at depth 4). The change was only committed within that limit; a run
past it stops for the user's call, and no cheaper variant exists.

## Amendment, 2026-09-26: the Auto budget and the Auto ladder (Superseded)

Superseded 2026-09-27 with Auto itself; kept as history. Today a record
carries one fingerprint, `Rules::fingerprint()`, and both Ready checks
(`structure_is_current` and `row_ready_sql()`) compare that one value. The
old Auto fingerprint survives only as `Rules::retired_auto_fingerprint()`,
which the store reads to delete the results Auto saved; no row carrying it
reads Ready. The names below are the code as it was then.

The first version put every rules field into one fingerprint, so any edit to
hydra_rules.ini made the whole library Stale. Two fields were over-reach
(user decision 7 of the 2026-09-26 audit plan).

The Auto time budget is a wall-clock limit. Two runs under the same budget
can settle on different rungs on a busy machine, so the fingerprint could
never promise a repeatable answer for it. It is in no fingerprint now. It
also had two homes: the search read `SearchSettings::time_budget_s` while the
fingerprint hashed `Rules::auto_budget_s`, so a run with no budget still
stamped 120 s. `Rules::auto_budget_s` is now the only home, and nullopt means
no budget.

The Auto ladder only changes what an Auto run does. A record now carries one
of two fingerprints: `Rules::fingerprint()` (every rule except the ladder and
the budget) for a fixed-cap run, and `Rules::auto_fingerprint()` (that plus
the ladder) for an Auto run. The store accepts either (`core::RulesStamp`),
in C++ (`structure_is_current`) and in SQL (then `kRowReadySql`, with
`IN (?, ?)`; today `row_ready_sql()`, which compares the one fixed
fingerprint). A ladder edit marked only Auto runs Stale.

The fingerprint's text changed, so every stored record reads Stale once more
after this lands. It ships with the record-format bump of the same plan,
which asks for the same one re-analysis.

## Amendment, 2026-10: the squeeze-out is stored once

The squeeze-out used to be stored twice: as `sqout_tick`, and as a SqOut
entry in the squeeze list with its own offset. Now the node stores
`sqout_tick` only. The SqOut's offset is its row's `offset_ms`. The squeeze
list stores SqIn offsets only. `Activation::set_sqout` is the one writer, and
`Activation::sqout_row` the one reader. The SP end the offset was measured
from is `deact_tick()`, read from the SP-end history (ADR 0021).

`collected_phrase_ticks` is no longer a stored field either. Each collected
phrase is a step in the SP-end history, and `collected_phrase_ticks()` reads
them back (ADR 0021).

## Amendment, 2026-10: a squeeze needs a phrase collected after the activation

A squeeze moves a phrase the player collects while Star Power runs. A
squeeze-out hits the phrase's last note late, so it lands after SP ends and
banks a bar. A squeeze-in hits it early, to extend SP. A phrase at or before
the activation chord was banked before SP started. So no activation can
squeeze it either way.

The squeeze window is 500 ms<!-- default: kSqueezeWindowMs --> around an SP
end (`kSqueezeWindowMs`). Usually that is far shorter
than one SP bar (two measures), so the window never reaches back past the
activation. At a very fast tempo or a very short measure it can. The engine
then offered a squeeze on a phrase the activation had banked. As a
squeeze-out, it cost a bar the activation never had and left it with no SP
end at all. As a squeeze-in, it had no step to relabel and broke the whole
search (D18).

The rule lives once, in `core/sqout_chord.h`: `activation_can_squeeze` is
true only for a phrase chord after the activation chord.

Tied paths. The search folds running paths that share an SP end, because the
end decides their future (one exception since D44, in "Tied paths whose
clamps came from different ends" below). Now the activation can decide it too: a path that
collected a phrase can squeeze it, and one that banked it cannot. So the
search groups running paths by one more key. The key is the banked phrase, if
any, that a later SP end of this activation could still hold in its window.
`core::banked_phrase_in_reach` works it out by asking
`activation_can_squeeze`. On a normal chart the key is "none" for every
activation, so the groups are unchanged.

### A phrase is squeezed in only once (D34)

Two SP ends can sit one tick apart. plusmeasure rounds down, so both move one
bar on to the same tick. Both then offered the same phrase to the same path.
The path squeezed it in at the first end, then in or out again at the second.
That printed an extra "+" (Thrice - Deadbolt at cap 4, SoundHaven - Triad at
cap 2) for a squeeze that never moved anything.

The rule now: an SP end offers the first phrase in its 500 ms window that the
running window can still squeeze. A phrase banked before the activation is
skipped (D18). So is a phrase this window already squeezed in. If nothing is
left, the SP end is a plain one. The banked case used to stop at the first
phrase; it now moves on to the next one too, so both cases follow the same
rule.

It lives in one function, `core::offered_phrase` in `core/sqout_chord.h`.
The graph lists every phrase in each SP end's window on its deactivation
edge (`squeeze_window_phrases`), because the edge is shared by every path.
The engine picks from that list with `offered_phrase`; "already squeezed in"
means the window holds an SqIn step on that phrase. The engine remembers
which phrase each window squeezed out, since it is no longer always the
window's first. `hydra_replay` asks the same function. A stored path tells it
which phrases each window squeezed in; a typed window says none.

On the library at caps 2 to 4 this changes exactly 7 listed paths on those
two charts, each losing one "+", and no score. The next-phrase offer never
fires there. It needs 500 ms to span an SP bar, so only the hand-made
4,000 BPM test charts reach it.

D36 replaced the early side of this rule. At those extreme tempos "the first
phrase in the window" let a window squeeze out an older phrase while keeping
a newer one's step, which no player can do: the newer phrase is hit later,
so it is hit after Star Power ran out too. The record then named an end one
bar early. It also let a tied variant take its leader's squeeze (the search
grouped paths without the phrases their window had squeezed in), and let one
of two SP ends a tick apart take the other's path.

### The newest phrase, at the end it moved (D36)

An SP end D offers a window at most one phrase (`core::offered_phrase`):

- Early side: the window's newest phrase, and only when collecting it moved
  the window's end from D. The graph decides once, per phrase and pending
  end, whether that end keeps a node that lists the phrase
  (`SpExtension::sqout_node`); the engine copies it into the step
  (`EndNode::sqout_at`). `offered_phrase` skips a phrase the window already
  squeezed in, so a phrase is still squeezed in only once. A banked phrase
  never has a step, so D18 holds too.
- Late side: when the window's end is D, the first phrase after D that it
  has not squeezed in. This side changed too. Before D36 an older phrase in
  D's window could block it: the old rule offered the first phrase the window
  could squeeze, and when that was an early one a path ending at D got
  nothing, never the late phrase behind it. Now the late phrase is offered
  whatever sits before D.

The search's group key on SP nodes gains one word: the end where the newest
phrase can still be squeezed out, while it is ahead. Paths at one node with
one SP end and one such end have the same newest phrase, so they face the
same offers. A squeeze-out now gives back exactly its own step, and the
rebuild throws if a closed window's record ends anywhere but its
deactivation node. T10's clamp-origin guard (finding 37) was this rule
for Clamped steps only, so it went with the edge's `sqout_time` and
`clamped` fields.

`hydra_replay` asks the same function. A stored window that ended plainly had
its end at D, so only the late side applies. A typed window has no history,
so it accepts either side's chord.

An early squeeze-in's step is the step on the squeezed-in chord: the
window's newest step. That step is Collected, or Clamped when the cap pinned
the end on that phrase. It is never the Activation step. A folded variant's
step there gets the leader's SqIn label at the close (`close_folded_act`).
`is_sqin_step` in the engine states that rule once.

### Tied paths whose clamps came from different ends (D44)

When the meter is full, collecting a phrase pins the SP end to the cap's
ceiling past that phrase. That ceiling is the same tick whichever end the
phrase moved. So two paths can tie on score and on their SP end, yet have
reached it from different ends. At extreme tempos a squeeze-out of that
phrase is on offer at the old end, and each path may only take it back to
its own old end (finding 37, D29).

Folded as ties, the variant would follow its leader's squeeze choices and
lose its own squeeze-out. T10 first stopped that with a guard of its own:
`reduce_group` kept two such paths apart when their clamps came from
different ends. The user approved that departure from the plan in D44. It
left one gap: the group key did not hold the end a clamp moved, so the lower
path could still be pruned before its own squeeze-out node.

D36's rule (above) now covers this case, and T10's guard is gone. A clamped
step is one more step that moved the SP end. It remembers that end when the
end can give the phrase back, and the group key holds it while it is ahead.
So two paths whose clamps came from different ends sit in different groups
while either can still squeeze. Neither folds into the other, and neither is
pruned against the other before its squeeze-out node, which closes T10's
gap too. Normal charts never reach any of this: the offer needs an SP bar
inside the 500 ms squeeze window.

### Two more extreme-tempo crashes (D32)

At 2,000 to 4,000 BPM the search could still crash in two ways, one in the
graph and one in the engine. Both come from an SP bar shorter than the
500 ms window. A late squeeze-in moves the SP end one bar past the old end,
and at these tempos that new end can land before the phrase it squeezed.
Three changes fix them:

- The graph used to add the new end's node only when it reached the phrase.
  By then the end was behind it. The graph now adds that node when it moves
  the SP end (`sqin_end_by_phrase` in graph.cpp).
- A path now ends where its SP end says. Before, a squeezable phrase in the
  window kept the path in SP past its own end (`deactivation_type`).
- A phrase a late squeeze-in already spent is not offered again. The path
  ends SP before reaching it, and hitting it later adds nothing to the meter.

The engine tracks two kinds of phrase that are still ahead of a path but
already paid for. A spent phrase is one a late squeeze-in took: its extension
is in the SP end, so reaching it adds no step and no bar. A banked phrase is
one a late squeeze-out took: its bar is already in the meter, so reaching it
adds nothing, and an edge that passes without it hands the bar back until its
own edge adds it again. They are separate fields (`spent`, `banked_ahead`),
and the spent ones always come first in chart order. Since D34 one window can
spend two late phrases that both lie past its final SP end. Its SqOut sibling
then leaves SP with two spent phrases and one banked bar ahead. With the two
kinds kept apart, the meter can never go below zero off SP. If it does, the
engine throws instead of skipping a phrase.

None of these fire on any of the 19,343 library charts. The charts in
`testdata/input/test_fast_tempo` are fuzzed at those tempos. They exist only
to keep these rules from breaking again.

Scores change only on charts where 500 ms spans an SP bar. On such a chart,
either a window reaches a banked phrase, or a late squeeze-in's new end lands
before its phrase (D32). None of the 97 corpus charts has one, and a count
across 19,343 library charts found none either. Old records of such a chart
keep their wrong squeeze until they are analyzed again, and the format 7
bump makes every record read Stale anyway.

### Three numbers the extreme-tempo rules use (D40)

The user recorded these as they are on 2026-10-04 (decision D40). None
changes a score on the library.

- The banked phrase in reach looks one tick before the first SP end that
  can squeeze (`banked_phrase_in_reach`). plusmeasure rounds down, so the
  real end can come out a tick early; the extra tick keeps the reach wide
  enough. It decides which tied running paths fold at extreme tempos. D36's
  follow-up may revisit it.
- An SP end exactly on its phrase's tick counts as reaching that phrase
  (`sqin_end_by_phrase` uses `<=`). The graph then adds that end's node
  when it moves the end, as for an end before the phrase.
- The search's group key packs each path into 64 bits
  (`reduce_iteration_paths` and `ready_class` in engine.cpp). While SP
  runs: the SP end must lie within ±2^46 ticks and the banked-phrase
  ordinal must be at most 65,535. While waiting: the meter must be under
  2^30, and each of `ready_class`'s two fill counts gets 16 bits. A chart
  past any of these fails to analyze with an error. It never folds paths
  wrongly. These widths come with D35's ready-time key.
