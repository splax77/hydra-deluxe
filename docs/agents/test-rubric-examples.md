# Test rubric: examples

These are real tests from this repo, judged by the questions in `docs/agents/test-rubric.md`. Each quote was copied from the file at the commit that added this page; line numbers are from that commit and will drift. "Q5" means question 5 on the rubric page.

Ten bad examples come first, then six good ones. Several good examples have a weak spot, and each says so. No test found so far passes every question.

## Bad examples

### B1. A snapshot total over the corpus

"corpus: squeeze-ins at exactly 0 and what they change", `tests/test_engine_sweeps.cpp` lines 656 to 660:

```cpp
    CHECK(charts == 98);
    CHECK(sqin_at_zero_acts == 56);
    CHECK(sqin_at_zero_timed == 56);
    CHECK(best_timed_by_sqin == 5);
    CHECK(saved_rows == 374);
```

It fails Q1 and Q2. Each number is what the code printed once over `testdata/input`. Nobody can say why 374 is right, and adding a chart changes it. It fails Q4 too: deleting any of these lines hides "any change at all", which is the mark of a snapshot.

The rewrite keeps the test's one property (G2 below) and drops the totals. For the case the totals were watching, it builds one small chart with a squeeze-in at exactly 0.0 and checks that chart's saved row shows 0.0.

### B2. A GUI count of corpus charts

`test_library_search`, `tests/ui/uitest_library.cpp` lines 493 to 496:

```cpp
    IM_CHECK_EQ(shown(), (size_t)98);
    IM_CHECK(visible_text(h).find("98 charts") != std::string::npos);
    IM_CHECK(ctx->ItemExists("**/All (98)##chipall"));
    IM_CHECK(ctx->ItemExists("**/Not analyzed (98)##chipnew"));
```

It fails Q2 and Q5. The promise is "the library shows every chart the scan found, and the footer and chips say the same count". 98 is how many charts sat in the folder that day. Any chart added to the folder turns all four lines red.

The rewrite takes the count from the scan the test just ran, and checks the three places agree with it: `shown()` equals the number of entries the scan stored, the footer text holds that number, and the "All" chip is labelled with it. The number is never typed.

### B3. A loop floor

"SP end history: every corpus activation is consistent", `tests/test_sp_end.cpp` line 799. The same line ends "Bank: every corpus path banks in order" at line 836.

```cpp
    CHECK(acts > 1000);  // a floor the user approved (D43)
```

It fails Q2 and Q5. The comment cites an approval. D43's addenda in `docs/audit/2026-10-03-fix-decisions.md` say "Approved as a test limit" and give no reason for 1000 over 10 or 5000. An agent wrote the number in commit ad3aab5c. The real worry is that the loop checked nothing, or skipped charts.

The rewrite checks the worry directly: no chart was skipped (see B4), and `acts > 0`.

### B4. A silent skip

"Bank: every corpus path banks in order", `tests/test_sp_end.cpp` lines 831 to 832:

```cpp
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        if (song.is_empty()) continue;
```

It fails Q5. A chart that loads with no notes is passed over without a word. If a parser bug emptied every chart, this loop would check nothing. Only the floor in B3 would notice, and only by accident. The same skip opens the corpus loops at lines 183, 449 and 766 of this file.

The rewrite counts the skips and requires the count to be zero. If some corpus chart really has no notes at this difficulty, the test names that chart and fails on any other.

### B5. Raw ticks as expected results on a made-up chart

"SP end history: a late squeeze-in is a SqIn step on its phrase", `tests/test_sp_end.cpp` lines 482 to 487:

```cpp
    CHECK((act.sp_end_steps == std::vector<SpEndStep>{
               {5760, 13440, SpEndKind::Activation}, {13920, 17280, SpEndKind::SqIn}}));
    // The bar arrives at the old end: the player hits the phrase early.
    CHECK(act.refill_tick(1) == 13440);
    CHECK(act.deact_tick() == std::optional<int64_t>(17280));
    CHECK(act.nominal_end() == std::optional<int64_t>(13440));
```

It fails Q6. The arithmetic is right: the fixture's comment in `tests/record_fixtures.h` (lines 81 to 87) works out that two bars from the fill at 5760 run four measures of 1920 ticks, to 13440. But the rule lives in that comment, in another file, and the check holds four raw numbers. Cover the comments and nobody can tell what 17280 is.

The rewrite names the inputs and writes the rule in the check: the activation is at `fill`, the plain end is `fill + bars_banked * measures_per_bar * measure`, the late phrase is a quarter measure after that end, and the extended end is the plain end plus one bar. Each `CHECK` compares against one of those names.

### B6. One rule stated twice, on no page

The same test's line 485 (quoted in B5) rests on "the bar arrives at the old end". "Bank: a squeezed-out bar arrives at the deact node", `tests/test_sp_end.cpp` lines 817 to 819, states it again in other words:

```cpp
    // The phrase at 12960 was hit late, just after SP ended at 13440: its bar
    // arrives there. Then the phrase at 14400.
    CHECK((path.activations[1].bank_rise_ticks == std::vector<int64_t>{13440, 14400}));
```

It fails Q8, Q2 and Q7. Where a squeezed phrase's bar arrives is a Hydra convention. A search of `docs/clone-hero/` for "old end" found no page that states it; that is one search, so treat it as not found, not as absent. The first audit trial kept one of these tests and sent the other to investigate.

The rule gets one verdict, investigate: the user is asked what the game does, the answer goes on a Clone Hero page, and both tests then point at that page.

### B7. Hydra checked only against Hydra

"stored transfer scales match the display-layer recomputation", `tests/test_engine_sweeps.cpp` lines 428 and 433 to 434:

```cpp
            auto scales = frontend_transfer_scales(act, song.timing());
```

```cpp
            if (differs(scales->post, *act.transfer_post))
                return "stored scales diverge from recomputation";
```

It fails Q7. The test's own header comment says the engine stamps each activation "through frontend_transfer_scales". So the test calls the function that made the stored value and checks the two agree. It proves the value was stored and read back unchanged. It cannot show a wrong scale.

The record calls it a wiring check and names the test that checks `frontend_transfer_scales` against a hand-built tempo change. If there is none, the verdict is rewrite: add a chart whose tempo doubles at a named tick and require the scale that follows from "measures last twice as long".

### B8. An expected value made by production code

"SP past the last note: backends measured from the tracked SP end", `tests/test_sp_end.cpp` lines 111 to 112:

```cpp
    CHECK(*last_note->offset_ms ==
          doctest::Approx(offset_from_sp_end(tick_ms(song, 5280), end_ms)).epsilon(1e-9));
```

It fails Q2 and Q7. Both halves of the expected value come from Hydra: `tick_ms` reads the song's own tempo map (`tests/record_fixtures.h` line 616) and `offset_from_sp_end` is production's subtraction. This is the nearest thing to a copy found in the files read for this page. A line-for-line copy of production steps inside a test was not found; the precheck's check for recomputed values already hunts those.

The rewrite states the distance from the chart's own grid. The chart is 120 BPM with 768 ticks to a 2000 ms measure, and the last note is 96 ticks before the end, an eighth of a measure. So the check reads `offset == -(measure_ms / 8)`.

### B9. A snapshot on one chart, chosen by file order

"graph: a deactivation edge's rows are a range of the graph's rows", `tests/test_sp_end.cpp` lines 844 to 847:

```cpp
    const Song& song = corpus::song(corpus::first_chart_with_notes(), ChartReading{});
    const ScoreGraph graph(song, 4);
    const size_t all = graph.all_backends().size();
    CHECK(all == 622);
```

It fails Q1, Q2 and Q5. The comment above the test says "The three numbers are pinned from one run". And 622 belongs to whichever chart sorts first, so a new chart with an earlier name changes it.

The comment already states the rule: "one row per chart timestamp". The rewrite checks that: the row count equals the number of chords in the song the test loaded.

### B10. A rule no page states, on a chart no player meets

"SP end history: no activation squeezes a phrase it banked before it started", `tests/test_sp_end.cpp` lines 710 to 717:

```cpp
            CHECK(act.timecode.ticks() == 2304);
            CHECK_FALSE(act.sqout_tick.has_value());
            CHECK(act.sqinouts.empty());
            // The history is never empty, and SP ends where the banked bars
            // put it.
            CHECK((act.sp_end_steps ==
                   std::vector<SpEndStep>{{2304, 6912, SpEndKind::Activation}}));
            CHECK(act.deact_tick() == std::optional<int64_t>(6912));
```

It passes Q9, narrowly. The fixture is 4000 BPM, and its comment says why: only then does the 500 ms window span more than one bar and reach back past the activation (`tests/record_fixtures.h` lines 207 to 214). It fails Q7 and Q2. "A phrase banked before Star Power started cannot be squeezed" is Hydra's reasoning. A search of `docs/clone-hero/` for "banked before" found no page that states it. It also fails Q6 with 6912.

The verdict is investigate, not keep with a note. The question for the user is whether the rule is the game's. Once a page states it, the expected end is written as `activation + bars_banked * measures_per_bar * measure`.

## Good examples

### G1. A Clone Hero rule number, checked on both sides of the edge

"Skipped fills: under the 1.1 rule a fill exactly at the reach edge is stored", `tests/test_fills_and_skips.cpp` lines 108 to 112:

```cpp
    // The deadline (7200 ms, checked above) plus the 85 ms hit window, pinned
    // as a number so a changed window fails here instead of moving the edge.
    const int64_t edge = 7285;
    CHECK((passed_over(edge) == std::vector<int64_t>{9600}));
    CHECK(passed_over(edge + 1).empty());
```

It passes Q1 and Q4. 85 is the rule, and the test types it on purpose so a changed constant fails. The two checks sit one millisecond apart, so an inclusive comparison turned strict goes red. The chart is built so one tick is one millisecond, and line 98 checks that. Its weak spot is Q6: `7285` would read better as the deadline plus 85. An auditor still opens the Clone Hero page that states the window (Q2); that page was not opened for this example.

### G2. A property over the corpus whose failure names the row

"corpus: squeeze-ins at exactly 0 and what they change", `tests/test_engine_sweeps.cpp` lines 648 to 649 and 655:

```cpp
            if (shown_timing(saved.hardest_ms) != path.difficulty())
                shown_differs.push_back(chart + " " + saved.path);
```

```cpp
    for (const std::string& d : shown_differs) CHECK_MESSAGE(false, "shown differs: " << d);
```

It passes Q1, Q2 and Q5. Every saved row must show the timing the engine worked out. No number is typed, a new chart cannot break it, and a failure names the chart and the path. These lines live in the same test as B1, and the loop above them has the silent skip of B4.

### G3. A hand-built chart whose check reads as the rule

"SP past the last note: backends measured from the tracked SP end", `tests/test_sp_end.cpp` lines 100 to 104:

```cpp
    // Every trailing note is before the SP end, so every offset is negative.
    for (const BackendSqueeze& b : act.backends) {
        REQUIRE(b.offset_ms.has_value());
        CHECK(*b.offset_ms < 0.0);
    }
```

It passes Q1 and Q6. The check is the rule: a note before the end has a negative offset. No tick is typed as a result. Line 98 requires the list to be non-empty first, so the loop cannot pass by checking nothing. The same test also holds B8.

### G4. Calling the owner, with the owner tested on its own

"graph: every deactivation edge lists the chords squeeze_window_phrases names", `tests/test_sp_end.cpp` lines 193 to 199:

```cpp
            const std::vector<const SongTimestamp*> window =
                core::squeeze_window_phrases(song, end);
            REQUIRE(e->squeeze_choices.size() == window.size());
            if (window.empty()) continue;
            ++claimed;
            for (size_t k = 0; k < window.size(); ++k)
                CHECK(e->squeeze_choices[k].chord.ticks() == window[k]->timecode.ticks());
```

It passes Q2 and Q7 as a wiring check. The graph is meant to use this owner, and the test checks it passes the owner's answer on, item by item. It ends with `CHECK(edges > 0);` and `CHECK(claimed > 0);`, the floor done right. The owner has its own test on hand-built songs: "squeeze_window_phrases: the phrase chords strictly inside the window, in chart order" in `tests/test_replay.cpp` line 1316. Two weak spots remain. Line 183 is the silent skip of B4. And the owner's test types raw ticks with the milliseconds in comments, so it is a Q6 rewrite itself.

### G5. A boundary case that says which side is out

The owner's test named in G4, `tests/test_replay.cpp` lines 1328 to 1330:

```cpp
    // On the window's edge before D is outside (within_squeeze_window).
    const Song edge = song_with({{0, false}, {2880, true}, {3072, false}});
    CHECK(core::squeeze_window_phrases(edge, edge.timecode(3072)).empty());
```

It passes Q4. The check needs no expected number: a phrase exactly on the window's edge is not in the list. An inclusive comparison in the window test turns it red. Its weak spot is Q6: 2880 and 3072 are inputs, which is allowed, but nothing in the line says they are exactly one window apart. `song_with` was not read for this example.

### G6. Rule numbers worked by hand from a two-note input

"scoring: category_scores is unchanged by sqout_pays_outside", `tests/test_scoring.cpp` lines 72 to 74 and 83 to 85:

```cpp
    // Worked by hand: Red and Kick at combo 8. The first note lands on combo
    // 9 (x1) and pays 50, the second on combo 10 (x2) and pays 100. Notes 100
```

```cpp
        CHECK(s->notes == 100);
        CHECK(s->combo == 50);
        CHECK(s->sp == 150);
```

It passes Q2 in kind: 50 points a note and a doubled multiplier at combo 10 are the game's scoring rules, not Hydra's output, and the input is two named notes at a stated combo. It is the best this repo has for a hand-worked score, and it still falls short on Q6, because the checks hold the results and the comment holds the rule. Written as `notes == 2 * points_per_note`, the check would state it. The Clone Hero page for these scoring numbers was not opened for this example.

## What a sound audit of tests/test_sp_end.cpp finds

This was the answer key for the first trial of the rubric. It cannot serve as a calibration file now that both pages discuss it; the rubric's "Running it as an audit" says how to pick one.

The file has two loop floors (B3), a silent skip at the top of each corpus loop (B4), raw ticks as results across the hand-built cases (B5), one bar-arrival rule stated in two tests (B6), an expected value made by production code (B8), a number pinned from one run (B9), and a rule no page states (B10). A run that returns keep for any of those tests has missed something.
