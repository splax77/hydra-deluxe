# A squeeze needs a phrase collected after the activation

A squeeze-out means hitting an SP phrase's last note late, so it lands after
Star Power ends and banks a bar instead of extending SP. A squeeze-in is the
same note hit early to extend SP. Both move a phrase the player collects
while Star Power runs. A phrase at or before the activation chord was banked
before SP started, so no activation can squeeze it either way.

The search's squeeze window is 500 ms around an SP end. Normally that is far
shorter than one SP bar (two measures), so the window never reaches back past
the activation. At a very fast tempo or with a very short measure it can. The
engine then offered a squeeze on a phrase the activation had banked: a
squeeze-out that cost a bar it never had (and, with the squeeze-out trimming
the SP-end history, left the activation with no SP end at all), or a
squeeze-in that had no step to relabel and broke the whole search.

## The decision

`core/sqout_chord.h` owns the rule, once: `activation_can_squeeze(act_tick,
chord_tick)` is true only for a phrase chord after the activation chord.

- The graph still names one chord per SP end (`sqout_chord(song, sp_end)`),
  since its deactivation edges are shared by every activation.
- The engine asks `activation_can_squeeze` before it treats that chord as
  squeezable (`deactivation_type`). When the activation banked it, the SP end
  is a plain one: SP ends there only if the path's end is exactly there.
- `hydra_replay` asks `sqout_chord(song, sp_end, act_tick)`, so it refuses a
  typed squeeze-out on a banked phrase and warns about no such chord.

When the graph's one chord was banked, the activation squeezes nothing at
that SP end, even if a later phrase sits in the same window. Finding that
case needs two phrases within a second and an activation between them, at a
tempo where 500 ms spans an SP bar; we leave it.

## Tied paths

The tie-fold groups running paths by their SP end, on the grounds that the
end decides the future. With this rule the activation tick can decide it too:
a path that collected a phrase can squeeze it out, one that banked it cannot.
So the engine groups running paths by one more value, the act edge's
`banked_phrase_ordinal`: which banked phrase (if any) a later SP end of this
activation could still hold in its window (`core::banked_phrase_in_reach`).
It is 0 on every activation of a normal chart, so their groups are unchanged.

## Consequences

Scores change only on charts where 500 ms spans an SP bar and a window
reaches a banked phrase. On the 97-chart corpus there is no such chart: no
score or path changes. Old records of such charts keep their wrong squeeze
until they are analyzed again.
