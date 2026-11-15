# Test rubric: examples

These are real tests from this repo, judged by the questions in `docs/agents/test-rubric.md`. "Q3" means question 3 on that page. Every quoted line was copied from the file at the commit that last changed this page; line numbers are from that commit and will drift. Where a neighbouring line was not read again for this page, the example says so.

Thirteen bad examples follow. The first five are the ones earlier audits missed: the test's own account of what should happen is the fault. Read those first. There is no good-examples list, because no test in this repo passes every question yet; the last section says what a good test looks like.

## Bad examples: the framing is the bug

### B1. A display test that pins the display's own rule

"sp meter curve: a late squeeze-in refills at the old SP end", `tests/test_sp_meter_curve.cpp` lines 420 to 422:

```cpp
    CHECK(sp_meter_bars_at(c, 7000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(1.0));   // the refill, at the old end
    CHECK(sp_meter_bars_at(c, 7250.0) == doctest::Approx(0.875)); // the late phrase
```

It fails Q2, Q3 and Q6. The claim (Q1) is that the gauge refills at the old SP end, a quarter measure before the phrase the player squeezes in. `Activation::refill_tick` in `src/core/model.cpp` works that moment out, and the only caller in `src/` is the Preview (`src/app/preview_view.cpp` line 428). The search never asks for it. So it is a game rule that only a display uses.

Q3 shows what that costs. On Pitch Dark (`testdata/input/test_pitch_dark_ratio`), the engine's record for the first activation has its squeeze-in step on chord 243, at tick 128520. `refill_tick` hands the gauge tick 128400 instead, the end in force before that step. The gauge refills at a moment the record never named. The test types that moment in milliseconds, so it holds the gauge to the wrong place.

The rewrite takes the expected side from the record. For each step in the activation's `sp_end_steps`, the gauge changes at the tick that step names, and it changes nowhere else. No millisecond and no bar reading is typed. The values 7000 ms and 0.875 do not survive. The finding against `refill_tick` is reported on its own line (derive-once, kind A), and the test rewrite does not close it.

### B2. The same rule pinned as a raw number, twice more

"SP end history: a late squeeze-in is a SqIn step on its phrase", `tests/test_sp_end.cpp` lines 484 to 485:

```cpp
    // The bar arrives at the old end: the player hits the phrase early.
    CHECK(act.refill_tick(1) == 13440);
```

"refill_tick: a late squeeze-in's bar arrives at the old end", `tests/test_model.cpp` line 1183:

```cpp
    CHECK(a.refill_tick(2) == 15360);  // past the end in force: the old end
```

They fail Q4 first: a reader cannot tell what 13440 is. The answer is in a comment above `make_late_sqin_song`, in `tests/record_fixtures.h`. They fail Q2 and Q3 for the reason in B1, and Q10 because three tests pin one rule.

An earlier version of this page judged line 485 and proposed naming the number: `old_end` in place of 13440. That keeps the wrong claim and makes it easier to read. Naming a value is never a fix for a claim nobody checked.

The rewrite removes both lines. Line 483 of the first test already checks what the engine recorded: the squeeze-in step sits on the phrase's own chord. That check has its own Q4 fault, raw ticks, and is rewritten with the step's chord named as the fixture's third phrase.

### B3. A check that cannot fail, under a comment that says it guards something

"SP end history: every corpus activation is consistent", `tests/test_sp_end.cpp` lines 788 to 790:

```cpp
                    // SP never runs dry inside a window: every step takes
                    // effect at or before the end in force.
                    CHECK(act.refill_tick(k) <= prev.end_tick);
```

It fails Q8. For a squeeze-in step, `refill_tick` returns the smaller of the step's tick and `prev.end_tick`. So for the one case the comment is about, the line compares a value with something it was already cut down to. It cannot go red there. An earlier version of this page listed this line among the test's sound properties.

The rewrite states the behaviour on the record's own fields. For every step that is not a squeeze-in, the step's tick is at or before the end in force. For a squeeze-in step, the squeeze the record lists for it is the one that carries the gap, and the check says so by comparing the step with that squeeze.

### B4. A gauge test that never asks the record where the squeeze-out is

"sp meter curve: a squeezed-out bar arrives at the deact node", `tests/test_sp_meter_curve.cpp` lines 445 to 449:

```cpp
    // Straight through the squeezed-out phrase (6750 ms): no step there.
    CHECK(sp_meter_bars_at(c, 6750.0 - 1e-6) == doctest::Approx(0.125));
    CHECK(sp_meter_bars_at(c, 6750.0) == doctest::Approx(0.125));
    CHECK(sp_meter_bars_at(c, 7000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(1.0));   // its bar, as SP ends
```

It fails Q3 and Q6. The claim is that the gauge shows nothing at the squeezed-out chord, drains to the end of the window, and gains the bar there. The engine names the squeeze-out chord: the activation's `sqout_tick`, and the `squeezes` entry in the schedule output. On Blight (Hail The Sun, from the user's library) the record's squeeze-out is chord 228 at tick 64800, and the SP end is tick 65280 with no chord between. The gauge draws Star Power ending at 65280 and marks nothing on chord 228. The test requires exactly that: "no step there".

The rewrite takes the squeeze-out from the record: where the activation has a `sqout_tick`, the gauge marks the squeeze-out on that chord. No millisecond is typed, and "no step there" does not survive. What the gauge reads after that chord is for the Preview fix to decide, not for this page.

### B5. The engine-side twin, typed as raw ticks

"Bank: a squeezed-out bar arrives at the deact node", `tests/test_sp_end.cpp` lines 817 to 819:

```cpp
    // The phrase at 12960 was hit late, just after SP ended at 13440: its bar
    // arrives there. Then the phrase at 14400.
    CHECK((path.activations[1].bank_rise_ticks == std::vector<int64_t>{13440, 14400}));
```

It fails Q4: the check holds two raw ticks and the comment holds the rule. Q2 is not settled. The search stamps `bank_rise_ticks` (`src/search/engine.cpp` line 2015), so the field is the engine's. But the readers found in `src/` are the Preview and `Activation::sp_ready_tick`, and nothing found uses it to choose or score a path. Whether "the later of the previous deact node and that phrase" is a game fact or a drawing rule stamped by the search is open.

This is a different rule from B1's. An earlier version of this page called the two "one rule stated twice" and asked the user what Clone Hero does. They have different owners (`refill_tick` and `bank_rise_ticks`), and B1's is not a Clone Hero question at all.

The verdict is investigate, on Q2 only. If the rule holds, the check is rewritten as the rule on named inputs: the first arrival is the first window's end, and the second is the fixture's fourth phrase.

## Bad examples: the check is the fault

### B6. A snapshot total over the corpus

"corpus: squeeze-ins at exactly 0 and what they change", `tests/test_engine_sweeps.cpp` line 656, the first of a run of pinned totals:

```cpp
    CHECK(charts == 98);
```

It fails Q4 and Q5. The number is how many charts sat in `testdata/input` that day, and the totals after it are what the code printed over them. Deleting any of them hides "any change at all" (Q8), the mark of a snapshot.

The rewrite keeps the test's one property (every saved row shows the timing the engine worked out) and drops the totals. For the case the totals watched, it builds one small chart with a squeeze-in at exactly 0 and checks that chart's saved row shows 0.

### B7. A GUI count of corpus charts

`tests/ui/uitest_library.cpp` line 493:

```cpp
    IM_CHECK_EQ(shown(), (size_t)98);
```

It fails Q5 and Q6. The promise is that the library shows every chart the scan found. 98 is a snapshot. The rewrite takes the count from the scan the test just ran, and checks each place that shows a count against it. The number is never typed.

### B8. A loop floor

"SP end history: every corpus activation is consistent", `tests/test_sp_end.cpp` line 799. The same line ends "Bank: every corpus path banks in order" at line 836.

```cpp
    CHECK(acts > 1000);  // a floor the user approved (D43)
```

It fails Q5 and Q9. The comment cites an approval. It does not say what the check is for, where 1000 comes from, or why the test needs it. The real worry is that the loop checked nothing, or skipped charts. The rewrite checks that directly: no chart skipped (B9), and the loop ran at least once.

### B9. A silent skip

"Bank: every corpus path banks in order", `tests/test_sp_end.cpp` lines 831 to 832:

```cpp
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        if (song.is_empty()) continue;
```

It fails Q9. A chart that loads with no notes is passed over without a word. If a parser bug emptied every chart, the loop would check nothing. The rewrite counts the skips and requires zero, or names each chart that may be skipped and fails on any other.

### B10. A stored value checked against the function that made it

"stored transfer scales match the display-layer recomputation", `tests/test_engine_sweeps.cpp` line 428:

```cpp
            auto scales = frontend_transfer_scales(act, song.timing());
```

It fails Q9. The test calls `frontend_transfer_scales` and compares the result with the scale stored on the activation. If the engine stamps the stored scale through the same function, as an earlier version of this page reported from the test's header comment, the test proves only that the value was stored and read back. That comment was not re-read for this page. The test's name also says a display recomputes the scale, which is a Q2 question in its own right: find who calls the function before judging the test.

The record calls it a wiring check and names the test that checks `frontend_transfer_scales` against a hand-built tempo change. With none, the verdict is rewrite.

### B11. An expected value made by production code

"SP past the last note: backends measured from the tracked SP end", `tests/test_sp_end.cpp` line 112, the second half of a check that starts on line 111:

```cpp
          doctest::Approx(offset_from_sp_end(tick_ms(song, 5280), end_ms)).epsilon(1e-9));
```

It fails Q5. The expected value comes out of Hydra: `tick_ms` (`tests/record_fixtures.h` line 616) and `offset_from_sp_end`. A bug in either moves both sides together. The rewrite states the distance from the chart's own grid: the note sits a named fraction of a measure before the end, so the offset is that fraction of the fixture's measure length, negative.

### B12. A snapshot on one chart, chosen by file order

"graph: a deactivation edge's rows are a range of the graph's rows", `tests/test_sp_end.cpp` lines 844 to 847:

```cpp
    const Song& song = corpus::song(corpus::first_chart_with_notes(), ChartReading{});
    const ScoreGraph graph(song, 4);
    const size_t all = graph.all_backends().size();
    CHECK(all == 622);
```

It fails Q4, Q5 and Q9. The comment above the test says "The three numbers are pinned from one run". And 622 belongs to whichever chart sorts first. The same comment states the rule: "one row per chart timestamp". The rewrite checks that against the song the test loaded.

### B13. A raw result whose meaning lives in a comment

"Skipped fills: under the 1.1 rule a fill exactly at the reach edge is stored", `tests/test_fills_and_skips.cpp` lines 108 to 112:

```cpp
    // The deadline (7200 ms, checked above) plus the 85 ms hit window, pinned
    // as a number so a changed window fails here instead of moving the edge.
    const int64_t edge = 7285;
    CHECK((passed_over(edge) == std::vector<int64_t>{9600}));
    CHECK(passed_over(edge + 1).empty());
```

The test builds a chart at 125 BPM with 480 ticks a beat, so one tick is one millisecond. Fill A starts at tick 9120 and ends at 9600. Clone Hero's 1.1 deadline is four beats before a fill's start (FL-19 in `docs/clone-hero/04-fills.md`): 9120 minus 1920 is 7200. Hydra keeps a fill offered until Star Power comes ready up to 85 ms after that deadline (FL-23, Hydra's own reach, which the game does not have). So 7285 is "the fill's start, minus four beats, plus Hydra's 85 ms reach". None of that is in the check. It fails question 4: cover the comments and nobody can tell what 7285 is. The comment's reason for typing it ("so a changed window fails here") is real, but a named rule number does the same job.

The rewrite names the inputs and the two rules: `fill_a_start`, `deadline = fill_a_start - 4 * beat`, and `reach = deadline + 85` with the 85 cited to FL-23. Then `passed_over(reach)` holds fill A and `passed_over(reach + 1)` is empty.

## What a good test looks like

No test in this repo passes every question yet. Earlier versions of this page listed "good examples" that each broke a question: a raw result such as 7285, a silent skip, a check in the same test as snapshot totals, or a display check that tested only one direction. They are gone, because a good list that holds bad tests teaches the wrong thing.

A good test of a display, the case this page was rewritten for, would take every chart in `testdata/input` and skip none. At each chord it asks the engine's record what happens there: did Star Power pay this chord, is it the squeezed chord, does Star Power end or refill here. Then it requires the display to draw exactly that at that chord, and nothing of the kind at any other chord. It types no number but zero. A failure names the chart and the chord. Today it would fail on Pitch Dark, where the gauge refills one note before the squeezed snare, and on Blight, where Star Power is drawn running past the squeezed-out chord. The first test written to this shape becomes this page's first good example.

## What was removed from this page, and why

An earlier version held three more examples. One judged `refill_tick(1) == 13440` as only a naming fault; it is now B2. One called the squeeze-in and squeeze-out rules a single rule; they are now B1 and B5. A third, a 4000 BPM fixture offered as a rule "no page states", was dropped because its claim was not checked against the engine's record. A hand-worked scoring example was dropped as a good example for the same reason: its checks hold raw results and its rule sits in a comment. The six good examples of the version after that were dropped too: each broke a question (7285 is now B13).
