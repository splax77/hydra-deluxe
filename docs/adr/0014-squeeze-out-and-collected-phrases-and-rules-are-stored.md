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

The engine stamps both facts at copy-out. `Activation::sqout_tick` is the deact
edge's `sqinout_time` when the path took the SqOut branch.
`Activation::collected_phrase_ticks` is every phrase tick the path crossed on
the SP track, recorded in `advance()` and trimmed back past the squeezed-out
phrase in `create_deactivated_path`. A late-SqIn phrase and a cap-clamped
phrase both count, because the gauge received them.

The pather stamps `HydraRecord::rules_fingerprint` with the fingerprint of the
rules the run used. The store writes it into the structure blob right after
the format version, so version and rules make one 12-byte head. That head is
what decides Ready, in C++ (`structure_is_current`) and in SQL
(`kRowReadySql`).

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

## Amendment, 2026-09-26: the Auto budget and the Auto ladder

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
in C++ (`structure_is_current`) and in SQL (`kRowReadySql`, now
`IN (?, ?)`). A ladder edit marks only Auto runs Stale.

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

The squeeze window is 500 ms around an SP end. Usually that is far shorter
than one SP bar (two measures), so the window never reaches back past the
activation. At a very fast tempo or a very short measure it can. The engine
then offered a squeeze on a phrase the activation had banked. As a
squeeze-out, it cost a bar the activation never had and left it with no SP
end at all. As a squeeze-in, it had no step to relabel and broke the whole
search (D18).

The rule lives once, in `core/sqout_chord.h`: `activation_can_squeeze` is
true only for a phrase chord after the activation chord.

Tied paths. The search folds running paths that share an SP end, because the
end decides their future. Now the activation can decide it too: a path that
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

One gap stays open at those extreme tempos. The search groups running paths
without the phrases their window already squeezed in, so a tied variant can
take its leader's squeeze of the next phrase where alone it would squeeze the
first (the early_sqin_twice test chart). Normal charts never reach it.

An early squeeze-in's step is the step on the squeezed-in chord. That step is
Collected, or Clamped when the cap pinned the end on that phrase. It is never
the Activation step. A lone path never meets an SqIn step there since D34; a
folded variant still can at extreme tempos (the gap above), and the step
stays SqIn. `is_sqin_step` in the engine states that rule once.

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

None of these fire on any of the 19,343 library charts. The charts in
`testdata/input/test_fast_tempo` are fuzzed at those tempos. They exist only
to keep these rules from breaking again.

Scores change only on charts where 500 ms spans an SP bar. On such a chart,
either a window reaches a banked phrase, or a late squeeze-in's new end lands
before its phrase (D32). None of the 97 corpus charts has one, and a count
across 19,343 library charts found none either. Old records of such a chart
keep their wrong squeeze until they are analyzed again, and the format 7
bump makes every record read Stale anyway.
