# Test rubric: examples

These are real tests from this repo, judged by the questions in `docs/agents/test-rubric.md`. "Q3" means question 3 on that page. Every quoted line was copied from the file at the commit that last changed this page; line numbers are from that commit and will drift. Where a neighbouring line was not read again for this page, the example says so.

Eighteen bad examples follow. The first five are the ones earlier audits missed: the test's own account of what should happen is the fault. Read those first. B15 to B18 show the four kinds of bad test the 2026-11-14 test audit named, and they quote an older commit, as their section says. Then comes G1, written for this page as a good example and no longer one: it retypes game-rule values from a Clone Hero page. No existing test passes every question, and this page holds no good example today.

## Bad examples: the framing is the bug

B1 to B4 quote their tests as they stood at commit 003d76a3. The Preview fix those four asked for has since landed. The Preview's own refill-tick function is gone, the gauge refills at the squeeze-in step's stored tick, and the quoted lines were rewritten from the engine's record.

### B1. A display test that pins the display's own rule

"sp meter curve: a late squeeze-in refills at the old SP end", `tests/test_sp_meter_curve.cpp` lines 420 to 422:

```cpp
    CHECK(sp_meter_bars_at(c, 7000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(1.0));   // the refill, at the old end
    CHECK(sp_meter_bars_at(c, 7250.0) == doctest::Approx(0.875)); // the late phrase
```

It fails Q2, Q3 and Q6. The claim (Q1) is that the gauge refills at the old SP end, a quarter measure before the phrase the player squeezes in. A refill-tick function on the activation, in `src/core/model.cpp`, worked that moment out, and the only caller in `src/` was the Preview (`src/app/preview_view.cpp` line 428). The search never asked for it. So it was a game rule that only a display used.

Q3 shows what that costs. On Pitch Dark (`testdata/input/test_pitch_dark_ratio`), the engine's record for the first activation has its squeeze-in step on chord 243, at tick 128520. That function handed the gauge tick 128400 instead, the end in force before that step. The gauge refilled at a moment the record never named. The test typed that moment in milliseconds, so it held the gauge to the wrong place.

The rewrite takes the expected side from the record. For each step in the activation's `sp_end_steps`, the gauge changes at the tick that step names, and it changes nowhere else. No millisecond and no bar reading is typed. The values 7000 ms and 0.875 do not survive. This rewrite failed on the code of its day, because the gauge still asked the refill-tick function, so it was written together with the Preview fix, not before it. The finding against that function was reported on its own line (derive-once, kind A), and the test rewrite did not close it; removing the function did.

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

An earlier version of this page judged line 485 and proposed naming the number: "old_end" in place of 13440. That keeps the wrong claim and makes it easier to read. Naming a value is never a fix for a claim nobody checked.

The rewrite removes both lines. Line 483 of the first test already checks what the engine recorded: the squeeze-in step sits on the phrase's own chord. That check has its own Q4 fault, raw ticks, and is rewritten with the step's chord named as the fixture's third phrase.

### B3. A check that cannot fail, under a comment that says it guards something

"SP end history: every corpus activation is consistent", `tests/test_sp_end.cpp` lines 788 to 790:

```cpp
                    // SP never runs dry inside a window: every step takes
                    // effect at or before the end in force.
                    CHECK(act.refill_tick(k) <= prev.end_tick);
```

It fails Q8. For a squeeze-in step, the refill-tick function returned the smaller of the step's tick and `prev.end_tick`. So for the one case the comment is about, the line compares a value with something it was already cut down to. It cannot go red there. An earlier version of this page listed this line among the test's sound properties.

The rewrite states the behaviour on the record's own fields. For every step that is not a squeeze-in, the step's tick is at or before the end in force. For a squeeze-in step, the squeeze the record lists for it is the one that carries the gap, and the check says so by comparing the step with that squeeze.

### B4. A gauge test that never asks the record where the squeeze-out is

"sp meter curve: a squeezed-out bar arrives at the deact node" (since renamed "...on the squeezed chord"), `tests/test_sp_meter_curve.cpp` lines 445 to 449 as this record was written:

```cpp
    // Straight through the squeezed-out phrase (6750 ms): no step there.
    CHECK(sp_meter_bars_at(c, 6750.0 - 1e-6) == doctest::Approx(0.125));
    CHECK(sp_meter_bars_at(c, 6750.0) == doctest::Approx(0.125));
    CHECK(sp_meter_bars_at(c, 7000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(1.0));   // its bar, as SP ends
```

It fails Q3 and Q6. The claim is that the gauge shows nothing at the squeezed-out chord, drains to the end of the window, and gains the bar there. The engine names the squeeze-out chord: the activation's `sqout_tick`, and the `squeezes` entry in the schedule output. On Blight (Hail The Sun, from the user's library) the record's squeeze-out is chord 228 at tick 64800, and the SP end is tick 65280 with no chord between. The gauge draws Star Power ending at 65280 and marks nothing on chord 228. The test requires exactly that: "no step there".

The rewrite takes the squeeze-out from the record: where the activation has a `sqout_tick`, the gauge marks the squeeze-out on that chord. No millisecond is typed, and "no step there" does not survive. Like B1's, this rewrite fails on today's code and is written together with the Preview fix. What the gauge reads after that chord is for the Preview fix to decide, not for this page.

### B5. The engine-side twin, typed as raw ticks

"Bank: a squeezed-out bar arrives at the deact node" (since renamed "...on the squeezed chord"; the test reads the chord from the record), `tests/test_sp_end.cpp` lines 817 to 819 as this record was written:

```cpp
    // The phrase at 12960 was hit late, just after SP ended at 13440: its bar
    // arrives there. Then the phrase at 14400.
    CHECK((path.activations[1].bank_rise_ticks == std::vector<int64_t>{13440, 14400}));
```

It fails Q4: the check holds two raw ticks and the comment holds the rule. Q2 is not settled. The search stamps `bank_rise_ticks` (`src/search/engine.cpp` line 2015), so the field is the engine's. But the readers found in `src/` are the Preview and `Activation::sp_ready_tick`, and nothing found uses it to choose or score a path. Whether "the later of the previous deact node and that phrase" is a game fact or a drawing rule stamped by the search was open here. It is settled now: the search stamps the squeezed chord for early and late squeeze-outs alike (SP-35 on `docs/clone-hero/05-star-power.md`), and `Path::bank_arrivals` returns the stored list unchanged.

This is a different rule from B1's. An earlier version of this page called the two "one rule stated twice" and asked the user what Clone Hero does. They had different owners (the refill-tick function, now removed, and `bank_rise_ticks`), and B1's was not a Clone Hero question at all.

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

It fails Q5 and Q6. The promise is that the library shows every chart the scan found. 98 is a snapshot. The rewrite takes the charts from the scan the test just ran. It checks the rows by name, one row for each scanned chart and no other row, because a count alone passes with the wrong rows (Q9). Each place that shows a count is then checked against the size of that same scan. The number is never typed.

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

It fails Q9. The test calls `frontend_transfer_scales` and compares the result with the scale stored on the activation. The engine stamps the stored scale through that same function: `src/search/engine.cpp` line 2099 is its only caller in `src/` and `tools/`. So the test proves only that the value was stored and read back. The test's name says a display recomputes the scale, and no display calls the function, so the name is wrong too (Q1).

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

It fails Q4, Q5 and Q9. The comment above the test says "The three numbers are pinned from one run". And 622 belongs to whichever chart sorts first. The same comment states the rule: "one row per chart timestamp". The rewrite checks that against the song the test loaded, item by item: every row sits on one of the song's timestamps, and every timestamp has exactly one row. Comparing the two counts would be a stand-in (Q9).

### B13. A raw result whose meaning lives in a comment

"Skipped fills: under the 1.1 rule a fill exactly at the reach edge is stored", `tests/test_fills_and_skips.cpp` lines 108 to 112:

```cpp
    // The deadline (7200 ms, checked above) plus the 85 ms hit window, pinned
    // as a number so a changed window fails here instead of moving the edge.
    const int64_t edge = 7285;
    CHECK((passed_over(edge) == std::vector<int64_t>{9600}));
    CHECK(passed_over(edge + 1).empty());
```

The test builds a chart at 125 BPM with 480 ticks a beat, so one tick is one millisecond. Fill A starts at tick 9120 and ends at 9600. Clone Hero's 1.1 deadline is four beats before a fill's start (FL-19 in `docs/clone-hero/04-fills.md`): 9120 minus 1920 is 7200. Hydra keeps a fill offered until Star Power comes ready up to 85 ms after that deadline (FL-23, Hydra's own reach, which the game does not have). So 7285 is "the fill's start, minus four beats, plus Hydra's 85 ms reach". None of that is in the check. It fails question 4: cover the comments and nobody can tell what 7285 is. The comment's reason for typing it ("so a changed window fails here") is real, but reading Hydra's own value by name does the same job without a typed copy.

The rewrite names the inputs and reads the two rule values from Hydra's code by name: a name "fill_a_start", `deadline = fill_a_start - 4 * beat` with the four beats read from where Hydra holds them, and `reach = deadline + <Hydra's reach, read by name>`. Where Hydra holds either value only as an unnamed literal, it is named in production first, and the test never types 4 or 85. The pages FL-19 and FL-23 are not a reason to type them. Then `passed_over(reach)` holds fill A, written as fill A's named end and not as 9600, and `passed_over(reach + 1)` is empty. This page has not done Q2 and Q3 for the reach rule (who uses it, and what the engine recorded on a real chart). So this is a sketch of the checks only, and the test stays off the good list until both are answered.

### B14. A near miss: typed rule values, plus one number nobody can explain and one line that guards nothing

"timing: to_multiplier thresholds", `tests/test_timing.cpp` lines 72 to 82. This is the closest any existing test came, so it is quoted whole:

```cpp
TEST_CASE("timing: to_multiplier thresholds") {
    using hydra::to_multiplier;
    CHECK(to_multiplier(0) == 1);
    CHECK(to_multiplier(9) == 1);
    CHECK(to_multiplier(10) == 2);
    CHECK(to_multiplier(19) == 2);
    CHECK(to_multiplier(20) == 3);
    CHECK(to_multiplier(29) == 3);
    CHECK(to_multiplier(30) == 4);
    CHECK(to_multiplier(1000) == 4);
}
```

Its claim is right (Q1 to Q3, as in G1's record below). It still fails four questions, and G1 does not repair the typed values.

It fails Q5 on line 81. Nothing says why 1000. The line is there to show the multiplier stops at x4, and 1000 is just a big number somebody picked. It fails Q8 on line 74: every small change to `to_multiplier` in `src/core/timing.cpp` that turns `to_multiplier(0) == 1` red also turns line 75 red, so line 74 guards nothing of its own. It fails Q4 on lines 75, 77 and 79: 9, 19 and 29 are each "one below a step", but the line does not say so. Typing 10, 20 and 30 is also a constant pin: `to_multiplier` in `src/core/timing.cpp` holds them as unnamed literals, so the fix starts in production (name them), and the test reads the names. It fails Q7: the commit that added it, 3b699581, is a bulk checkpoint that names no failure.

The verdict is rewrite. G1 was the first rewrite, and the filed rulings now fail it too.

## Bad examples: the four kinds the audit named

The 2026-11-14 test audit judged the tests as they stood at commit c10c7f54, and its records give their line numbers from that commit. B15 to B18 quote their tests from that commit, opened with `git show c10c7f54:<file>`. The audit's records were removed from the tree after it closed; each example names the record it comes from, which `git show b2287e56:docs/audit/2026-11-14-test-audit/<record>` still opens. Each example also says what the test looks like on today's tree.

### B15. An echo

"user_messages: the no-notes message is already plain and passes through", `tests/test_user_messages.cpp` lines 120 to 124 at c10c7f54:

```cpp
TEST_CASE("user_messages: the no-notes message is already plain and passes through") {
    const std::string msg = "No Expert Pro Drums notes in this chart.";
    CHECK(plain_error(hydra::ChartFileError(hydra::ErrorKind::AlreadyPlain, msg)) == msg);
    // The typed error the loaders throw passes through the same way.
    CHECK(plain_error(hydra::NoNotesError(hydra::Difficulty::Expert, true)) == msg);
```

It fails Q8 as an echo (the audit's `cross-findings.md`, section X11, line 147). Line 122 types a sentence into the error and requires `plain_error` to hand the same sentence back. For that kind of error, `plain_error` in `src/app/user_messages.cpp` (line 181 at c10c7f54) returns the error's own text. So the line states nothing the code works out; it shows only that a typed string came back. Line 124 already does the real job: it runs the error the chart loaders really throw through `plain_error`. The rewrite drops line 122 and keeps line 124.

The test is unchanged on today's tree, at lines 129 to 133.

### B16. A constant pin

"precheck pick (d): a test file the range changes is picked", `tests/test_precheck_pick.cpp` lines 315 to 322 at c10c7f54:

```cpp
TEST_CASE("precheck pick (d): a test file the range changes is picked") {
    PickRepo fx("precheck_pick_d", {{"tests/test_d.cpp", "int d1() { return 1; }\n"},
                                    {"tests/test_e.cpp", "int e1() { return 1; }\n"}});
    const TestPick p = fx.pick({{"tests/test_d.cpp", "int d1() { return 2; }\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_d.cpp"});
    CHECK(reasons_of(p, "tests/test_d.cpp") == std::vector<std::string>{"changed in range"});
}
```

It fails Q5 as a constant pin (`counts/T5-IR10.md` line 27). Line 320 checks the behaviour: the one test file the range changed is picked, and the other is not. Line 321 types the reason text again, "changed in range", which `tools/precheck/test_pick.cpp` writes at line 363. If someone rewords that reason, the test goes red with no bug. If the pick stopped giving any reason, line 321 would catch it, but so would any check that the reason is there. For a message, the behaviour is when it is shown and what it names. Here that is: the picked file carries one reason, and it names no function, because no function changed.

On today's tree the test no longer types the text. It requires one reason that is not empty and names no changed function, and it reads that same reason back where a helper header passes it on.

### B17. Cited non-code

"test_flush_fill_last_note_is_six_seconds_after_the_case_fill", `tools/ch_probe/tests/test_early_fill_walk.py` at c10c7f54. The file's header, lines 1 to 4:

```python
"""Tests for the Early Fill Walk probe song's layout. Pure text and data: no
game, no ffmpeg and no hydra_replay. The layout's plan is "Which chart" and
"T1" in docs/superpowers/plans/2026-11-12-early-fill-live-check.md; the
numbers pinned here are that plan's."""
```

The test, lines 121 to 125:

```python
    def test_flush_fill_last_note_is_six_seconds_after_the_case_fill(self):
        case_lasts = [start + length for start, length in self.fills[0::2]]
        flush_lasts = [start + length for start, length in self.fills[1::2]]
        for case_last, flush_last in zip(case_lasts, flush_lasts):
            self.assertEqual(flush_last - case_last, 6000)
```

It fails Q5 as cited non-code, and as a constant pin too (`counts/T5-IR5.md` line 106). The header says every number pinned in the file is the plan's. So 6000 names a plan as its source, not the code that writes the chart. The code that makes the chart is the generator in `tools/ch_probe/probe_songs.py`, and it already held the gap by name at that commit: `EFW_FLUSH_AFTER_MS = 6000` (line 105). The test types the number again instead of reading it, and says it came from the plan. An intended change to the generator's gap makes this line red with no bug in the layout. The rewrite reads the gap by that name and checks that the written chart keeps it.

On today's tree the test is renamed "test_flush_fill_last_note_comes_the_flush_gap_after_the_case_fill" and reads the gap as `P.EFW_FLUSH_AFTER_MS`. The header now says every expected number is read from the generator's own layout table. This page has not judged today's version.

### B18. A fake stand-in

"test_a_process_first_seen_gains_nothing", `tools/ch_mcp/tests/test_machine_recorder.py` at c10c7f54. The made-up processes, lines 123 to 129:

```python
TWO_PROCESSES = [
    {"pid": 7, "parent": 3, "name": "game.exe", "user": 100_000, "kernel": 50_000, "read": 11,
     "write": 22, "faults": 33,
     "threads": [(71, 10_000, 20_000, 5, 2, 0), (72, 30_000, 40_000, 6, 5, 6)]},
    {"pid": 8, "parent": 7, "name": "other.exe", "user": 1, "kernel": 2, "read": 3, "write": 4,
     "faults": 5, "threads": [(81, 1, 2, 3, 1, 0)]},
]
```

The helper that turns them into a table, lines 53 to 57:

```python
def fake_table(processes, base=0):
    """Table bytes built from the literal layout above, for the tests of the gains,
    the files and the loop. Each process is a dict of pid, parent, name, user,
    kernel, read, write, faults and threads; each thread is (tid, kernel, user,
    switches, state, wait_reason). No test of the parser's offsets reads it."""
```

The test, lines 323 to 327:

```python
    def test_a_process_first_seen_gains_nothing(self):
        current = M.parse_table(fake_table(TWO_PROCESSES)).processes
        gains = M.deltas({}, current)
        self.assertEqual(gains[7][:4], (0, 0, 0, 0))
        self.assertEqual(gains[7].threads[71], M.ThreadGain(0, 0.0))
```

It fails Q9 as a fake stand-in for outside data (`counts/T5-IR6.md` line 34). In real use the process table comes from Windows: `tools/ch_mcp/machine_recorder.py` asks `NtQuerySystemInformation` for it (lines 62 and 454 at c10c7f54). Here the test types two processes nobody ran, with made-up times and counts, and writes them into bytes in the layout the test file itself declares. So the recorder's sums are checked on a table Windows never wrote. The audit's counter re-read this mark after the user chose that input a test builds is fine, and kept it, as a stand-in for outside data. The same file already held a table Windows really wrote, which its `CapturedTable` tests read. The rewrite runs the same check on that table.

On today's tree the test reads `self.captured()` instead, which loads that saved table, and checks every process in it. This page has not judged today's version.

## A former good example

### G1. The combo multiplier steps up every ten notes and stops at x4

This test was written for this page. It is in `tests/test_timing.cpp` on main, where it replaced "timing: to_multiplier thresholds" (B14). It passes on today's code. Its one review added the check at combo 0, which the first draft had dropped. With the bug "combo < 10" typed as "combo <= 10" planted in `to_multiplier`, it fails on `CHECK( to_multiplier(x2_from) == 2 )`, values `1 == 2`, which names the rule that broke. Its name has no commas, because doctest's `-tc` filter splits on them.

**G1 is no longer a good example.** It types 10, 20 and 30 and cites SC-05 in `docs/clone-hero/06-scoring.md` as the reason. The user's rulings say "tests should not be pinned to typed constants. they should be pinned to the behavior of the code." and "tests exist to verify the behavior of the code. a test citing something that isn't code is always wrong". A Clone Hero page is not code, and Hydra already holds these values, so G1 fails the rulings. Its claim and its checks at each step are sound. Its typed values are the fault. The test is rewritten in this plan's scans phase; the rewrite reads the three values from Hydra by name, which needs them named in `src/core/timing.cpp` first. Until it lands, the test and the record below stay here as the shape of a claim checked on a real chart, not as a model for typed values. This page does not edit the test.

```cpp
TEST_CASE("timing: the combo multiplier steps up every ten notes and stops at x4") {
    using hydra::to_multiplier;

    // The combo at which each multiplier starts: SC-05 in
    // docs/clone-hero/06-scoring.md.
    const int x2_from = 10;
    const int x3_from = 20;
    const int x4_from = 30;

    // SC-05: 1x runs from combo 0 up to the combo before x2_from.
    CHECK(to_multiplier(0) == 1);
    CHECK(to_multiplier(x2_from - 1) == 1);
    CHECK(to_multiplier(x2_from) == 2);
    CHECK(to_multiplier(x3_from - 1) == 2);
    CHECK(to_multiplier(x3_from) == 3);
    CHECK(to_multiplier(x4_from - 1) == 3);
    CHECK(to_multiplier(x4_from) == 4);

    // x4 is the top, however long the combo runs.
    CHECK(to_multiplier(std::numeric_limits<int>::max()) == 4);
}
```

Every number in it is either typed from SC-05 (10, 20, 30 and the multipliers 1 to 4, the fault above) or written as its working (`x2_from - 1`). The last line's input is the longest combo an `int` can hold, so no big number is picked.

The record, in the rubric's own shape:

```
timing: the combo multiplier steps up every ten notes and stops at x4
            (tests/test_timing.cpp)
Read:       the test above, all of it; to_multiplier (src/core/timing.cpp
            :12-17); multiplier_step and crosses_multiplier_step (:19-27);
            category_scores (src/core/scoring.cpp:41-57);
            MultSqueeze::applies' call (src/core/model.cpp:369); grep
            to_multiplier in src/ and tools/ (core files only); SC-05
            (docs/clone-hero/06-scoring.md:53-61); the hydra_replay
            schedule output for Pitch Dark (testdata/input/
            test_pitch_dark_ratio), from the main session; where that
            output's per-note multiplier is written
            (tools/replay_schedule.cpp:391) and filled from
            category_scores' per-note answer (src/core/replay.cpp:160,
            ReplayNote in src/core/replay.h:147-154); the old test
            (tests/test_timing.cpp:72-82) and its commit, 3b699581; grep
            to_multiplier and multiplier_step in tests/.
1 Claim:    The 10th note in a row is the first that pays double, the
            20th the first that pays triple, the 30th the first that pays
            four times, and no combo pays more than four times.
2 Owner:    to_multiplier works it out. The engine owns it: category_scores
            calls it for every note it scores (scoring.cpp:57, after
            combo += 1 on :56), and multiplier_step calls it for
            category_scores (:46) and for the multiplier squeeze
            (model.cpp:369). No file under src/app, src/ui or tools calls
            it.
3 Real chart: Engine, Pitch Dark, per-note multiplier in the schedule
            output, every note hit from the start: note 9 (chord 4, tick
            72240) x1; note 10 (chord 5, tick 72480) x2; note 19 (chord
            12, tick 74160) x2; note 20 (chord 13, tick 74400) x3; note 29
            (chord 18, tick 75840) x3; note 30 (chord 19, tick 76000) x4;
            note 31 (chord 20) x4. No Star Power pays these chords.
            Claim: x2 from the 10th, x3 from the 20th, x4 from the 30th.
            They agree.
4 Direct:   Eight checks. The first reads "a combo of 0 pays x1", SC-05's
            first combo. Six read "at this combo, this multiplier", each
            combo written as a step SC-05 names or as one below it. The
            last reads "the longest combo still pays x4". With the three
            comments covered, the constants' names still say what each
            number is.
5 Reason:   Fails. 10, 20, 30 and the multipliers 1 to 4 are typed from
            SC-05 ("So 1x at 0 to 9, 2x at 10 to 19, 3x at 20 to 29 and
            4x from 30"). A Clone Hero page is not code, and
            to_multiplier already holds these values as unnamed literals
            (timing.cpp:13-15). They are constant pins, and the page
            citation is cited non-code. Fix: name them in production,
            read the names in the test. The "- 1" is the test's working,
            on the line. numeric_limits<int>::max() is the test's own
            input, and its name says what it is. No other number.
6 Display:  Not a display test. Question 2's callers are all in src/core;
            nothing a person reads calls to_multiplier directly.
7 Failure:  A step that sits one combo early or late, or a missing top.
            Every chord near a step would be scored at the wrong
            multiplier, and a multiplier squeeze would be offered on the
            wrong chord. Still possible: to_multiplier is three
            hand-typed comparisons. The old test's commit named no
            failure, so this one is named here (reasoned).
8 Deleted:  Each a change to to_multiplier; the (x2_from) == 2 line is confirmed
            (planted, built, red), the rest reasoned.
            (0) == 1:           a combo of 0 sent to its own branch that
                                returns 0, as if no note had been hit.
            (x2_from - 1) == 1: "combo < 10" typed as "combo < 9".
            (x2_from) == 2:     "combo < 10" typed as "combo <= 10" (confirmed).
            (x3_from - 1) == 2: "combo < 20" typed as "combo < 19".
            (x3_from) == 3:     "combo < 20" typed as "combo <= 20".
            (x4_from - 1) == 3: "combo < 30" typed as "combo < 29".
            (x4_from) == 4:     "combo < 30" typed as "combo <= 30".
            (max) == 4:         the body replaced by SC-05's formula,
                                1 + combo / 10, without its cap. The six
                                lines above all still pass on that.
            Each line can fail, and no other line catches its bug.
9 Stand-in: None. The risk is a wrong multiplier at a step, and each line
            asks for the multiplier at a step. It leans on no chart, file
            order, timing or other test. A failure prints the line, and
            the line names the step.
10 Same rule: tests/test_timing.cpp:72 states it and is replaced by this
            test (B14). tests/test_model.cpp:249 tests multiplier_step, a
            different question: what a chord's first and last notes pay.
            tests/test_stars.cpp (lines 47, 51, 60-63, 134 and 229-236) and
            tests/test_replay.cpp (lines 1671, 1717 and 1880) call
            to_multiplier for a value or search for a step, and do not
            state where the steps sit.
11 Real:    Yes. Pitch Dark reaches all three steps in its first 20
            chords. A combo as long as the last line's input cannot
            happen; the line says why it is used (no combo, however long).
Verdict:    Rewrite. Keep the claim and the eight checks. Read the three
            step values from Hydra by name, after naming them in
            production. Type none of them.
```

## The display test that comes with the Preview fix

This page was rewritten because of a display, the Preview's gauge, so it should hold a good display test. It cannot hold one today. A good display test checks the gauge against the engine's record, and the gauge disagrees with the record today (B1 and B4). A test that is right would fail, and a test that passes would be wrong.

Here is the test to write in the same change as the Preview fix. It takes every chart in `testdata/input` and skips none. At each chord it asks the engine's record what happens there: did Star Power pay this chord, is it the squeezed chord, does Star Power end or refill here. Then it requires the display to draw exactly that at that chord, and nothing of the kind at any other chord. It types no number but zero. A failure names the chart and the chord. Today it would fail on Pitch Dark, where the gauge refills one note before the squeezed snare, and on Blight, where Star Power is drawn running past the squeezed-out chord. When it is written and passes, it gets its own record here and becomes G2.

## What was removed from this page, and why

An earlier version held three more examples. One judged `refill_tick(1) == 13440` as only a naming fault; it is now B2. One called the squeeze-in and squeeze-out rules a single rule; they are now B1 and B5. A third, a 4000 BPM fixture offered as a rule "no page states", was dropped because its claim was not checked against the engine's record. A hand-worked scoring example was dropped as a good example for the same reason: its checks hold raw results and its rule sits in a comment. The six good examples of the version after that were dropped too: each broke a question (7285 is now B13). The version after that had no good example at all; G1 was written to fill that gap, and the user's rulings on typed constants and cited non-code have since taken it off the good list.
