# G4b report: the engine's views of the graph

Files read in full: src/search/graph.h (304 lines), graph.cpp (450), engine.cpp (2,091), pather.cpp (504). I also read engine.h and pather.h, and the owner headers core/backend_value.h, core/sqout_chord.h and the parts of core/model.h and model.cpp that the engine calls. Agent id: the progress hook never named one, so I used `G4b` for the status file.

Short answer to the specific question. Both views are filled from the graph type through one converter, `enumerate()` in engine.cpp (lines 97-187). Nothing else in `src/` builds an `EdgeView` or a `ChoiceView`. I checked with a grep over all of `src/`: `EdgeView v;` appears once (line 163) and the `ChoiceView{...}` brace-fill appears once (line 180). Details are under "Parallel on purpose".

## Candidates

### G4b-1. Is this squeeze choice's chord after the SP end?
Looks like: K1 (same rule, one call written twice for the same chord and end; they agree).
Homes: `ScoreGraph::add_deact_edge`, search/graph.cpp 430 (`choice.late = core::after_sp_end(chord ticks, end ticks)`, stored as `SqueezeChoice::late`, copied into `ChoiceView::late` at engine.cpp 182);
       `Engine::create_deactivated_path`, search/engine.cpp 1033 (calls `core::after_sp_end(sq->chord, sp_end)` again, on the very same choice, one function away from the places that read `sq->late` at 1115 and 1128).
       A third call sits inside `core::offered_phrase`, core/sqout_chord.h 92, and a fourth family member is `paid_by_sp_walk` (backend_value.h 30), already G4a-4.
Table, home 1 (the stored flag): chord tick < end tick -> false; chord tick == end tick -> false; chord tick > end tick -> true. Inputs are the chord's timecode ticks and the deact edge's dest ticks.
Table, home 2 (engine 1033): the same three rows. Inputs are `sq->chord` (the same chord ticks, narrowed to the view) and `node(e.dest).tick` (the same dest ticks).
Differ on: nothing today; they are the same two numbers. The engine already holds the answer in `sq->late`. The risk is that a later change to how `late` is set (for example from the sign of `timing`, which is what `paid_by_sp_walk` reads) would leave line 1033 on the old rule.
Note: this is the engine side of G4a-4. backend_value.h 32-35 says in a comment that the graph and the engine both ask `after_sp_end`; it does not say the engine could read the stored flag. Verifier: check that `sq->late` and `after_sp_end(sq->chord, sp_end)` can never differ (both inputs come from `squeeze_window_phrases` and the deact node, so by reading they cannot).

### G4b-2. Which chart rows fall inside an SP end's squeeze window?
Looks like: K1 (one predicate, `within_squeeze_window`, used by three different selection loops; they agree on every input I could think of).
Homes: graph sweep, search/graph.cpp 324-346 (`is_recent_to_head`, `set_head_time`) with 408-416 and 229-232 (an edge's rows are the range `backend_begin..backend_end` of `all_backends_`; inputs: row ms, SP end ms, head ms as the head moves forward), plus the same predicate at 155-156 (which pending SP ends hold this phrase in their window);
       `core::squeeze_window_phrases`, core/sqout_chord.h 28-43 (phrase chords only; inputs: the song and the end; walks outward from the end by binary search);
       the tail loop in `rebuild`, search/engine.cpp 1943-1952 (rows left over when SP outlasted the chart; filters `tail_backends` by `within_squeeze_window(offset_from_sp_end(row ms, end ms))` and copies each with its offset).
Table, home 1 (sweep): offset -499.9 ms -> in; -500 -> out; 0 -> in; +499.9 -> in; +500 -> out. A row exactly 500 ms from the end is out, because the sweep drops the edge when the head is 500 ms or more past it, and the row that arrives at that head is added after the edge closes.
Table, home 2 (`squeeze_window_phrases`): the same rows, but kept only if `flag_sp` is set.
Table, home 3 (rebuild tail): the same five rows, no `flag_sp` filter, measured against the tracked end tick instead of a graph node.
Differ on: nothing found; the strict-500 edge is the same in all three because all go through `within_squeeze_window` (core/model.h 119-124).
Note: read-only. The sweep's "row at the closing head is excluded" is my reading of the order `set_head_time` then `store_new_backend` (graph.cpp 133-146); the verifier should confirm. The comment at engine.cpp 1939-1942 says the tail loop uses "the same offset rule add_deact_edge uses", but add_deact_edge does not compute an offset: `edge_backends` does (graph.cpp 277-285, via `edge_row_offset`). That comment is stale by one name.

### G4b-3. How is the search graph built for a set of search settings?
Looks like: K1 (the same constructor call written three times; they agree).
Homes: `target_graph`, search/pather.cpp 257-261 (`ScoreGraph(song, graph_build_cap(sp_cap, phrase count), fill_rule_for(legacy_fill_deadline), rules)`);
       `analyze_chart` -> `analyze_at_cap`, pather.cpp 494 and 461-463 (the cap comes in as `build_cap` from `graph_build_cap`, then `ScoreGraph graph(song, cap, fill_rule_for(legacy_fills), rules)`);
       `tools/bench.cpp` 289-291 (outside my files; an exact copy of the `target_graph` expression, so the tool can time the build on its own).
Table, home 1: sp_cap 4, 12 phrases -> built at 4; sp_cap 8, 3 phrases -> built at 3; sp_cap 0 (or less) -> built at 1; legacy flag set -> Ch10; clear -> Ch11.
Table, home 2: the same four rows, through `graph_build_cap` and `fill_rule_for` (both single owners). Extra row: `build_cap` unset -> built at the raw `sp_cap` (pather.cpp 461-462). Its only caller always passes it, so that row is never taken.
Differ on: agree today; the unused `build_cap`-unset branch would build at the raw cap if some future caller left it empty.
Note: the three pieces (`graph_build_cap`, `fill_rule_for`, the `rules` pass-through) each have one owner, so the copy is only the way they are put together. Low weight. The bench copy is deliberate in purpose (timing), not in text.

### G4b-4. Which search settings change a stored analysis? (`settings_key`)
Looks like: K3-with-a-rule-inside. The rule inside is "which fields count", kept by hand.
Homes: `settings_key`, search/pather.cpp 65-75 (sp_cap, depth mode, depth value, ms limit or "none", legacy fill flag, rules fingerprint; used only by the test cache, tests/corpus_util.h 222, and two tests);
       `store::Lens` and `RecordKey`, store/record_store.h 115-160 (ms on/off and value, depth mode, depth value, legacy fills, plus the cap and chart mode; the rules fingerprint is stored on the row as a separate stamp, `rules_fp`, not in the key: record_store.cpp 583);
       `analyze_chart`, pather.cpp 495-500 (copies sp_cap, depth, ms limit, legacy flag, rules into the run, then stamps `rules_fingerprint` and `legacy_fills` on the record by hand).
Table, home 1 (`settings_key`): sp_cap -> in key; depth_mode -> in key (as `static_cast<int>` of the enum); depth_value -> in; ms_filter -> in (a number, or "none"); legacy_fill_deadline -> in; rules -> in (as the fingerprint).
Table, home 2 (`RecordKey`): sp_cap -> in key (as `CapQuery`); depth_mode -> in (raw int); depth_value -> in; ms_filter -> in (on/off plus value, with a canonical 0 when off); legacy fills -> in; rules -> not in the key, kept as a stamp that makes a row Stale.
Differ on: the rules. The test cache keys by them, the store does not. That is on purpose (a changed rules file makes rows Stale rather than new), but it means the two lists of "what counts" differ by one field by design.
Note: `pather.h` 46-48 says "A new SearchSettings field that changes the result must be added here"; the only guard is the tests in tests/test_engine_sweeps.cpp 551-563 that check each current field. See also wave A's G3-1 and G3-5 (settings_key is the `static_cast<int>(depth_mode)` line G3-5 names). I did not read record_store.cpp beyond the Lens and the `rules_fp` line; the "not in the key" claim comes from Lens/RecordKey in the header and should be checked against the lookup code.

### G4b-5. How many paths are tied under a root?
Looks like: K1 with a runtime guard (two counts of one number; the stored one is the recount).
Homes: the engine's running count, search/engine.cpp 245, 287, 323, 1241, 1247, 1263, 1730, 1747 (`tied_count` on `Path`, `Variant`, `OutPath`; it enforces the tie limit `max_tied_paths_`: a variant folds only if `leader.tied_count + p.tied_count <= max_tied_paths_`);
       `Path::recount_tied_paths`, core/model.cpp 649-656 (recounts the finished tree; this is the number stored and shown);
       two more calls of the recount in `keep_paths_where`, search/pather.cpp 187 and 201, after the tree has been pruned.
Table, home 1 (engine): root path -> 1; fold of a variant carrying k -> leader gains k; fold refused when leader + variant would exceed the limit.
Table, home 2 (recount): leaf -> 1; node with variants -> 1 + sum of each variant's recount.
Differ on: agree today. `rebuild` throws if the two numbers differ for any root (engine.cpp 2055-2063, finding 179). After `keep_target_paths` prunes variants the engine's count no longer applies at all and only the recount is used.
Note: this guard exists because the two counts are separate code. A K0 reading is also fair: the engine's count is a limit counter and the stored count is a display fact. The verifier should decide. Sibling case: `Path::score_*` is the stored form of the engine's `sc[6]` (see "Also seen in G4b: G4a-1"); `emit_variant` writes `OutPath{}` with zero score fields for variants and leaves them to `prepare_variants` (ADR 0017), so "zero" in an `OutPath` means "not set" with no flag saying so.

### G4b-6. Is every timing of this activation inside the ms limit?
Looks like: K3-with-a-rule-inside (the loop "every squeeze, then the early fill" is written twice over different data shapes; every comparison inside goes through shared helpers).
Homes: `Engine::act_within_limit`, search/engine.cpp 1137-1146 (walks the engine's `SqNode` chain, builds a temporary `SPSqueeze` for each, then asks `fill_within_limit(a.e_offset, a.skips, ms_filter_)`);
       `Activation::within_ms_limit`, core/model.cpp 543-547 (walks `sqinouts`, then `fill_within_limit(e_offset, skips(), limit)`).
Table, home 1: no limit set (`!has_ms_filter_`) -> true; no activation -> true; any squeeze over the limit -> false; early fill over the limit -> false.
Table, home 2: the same rows, except there is no "no limit" row because the caller passes a number; no activation is a `Path::within_ms_limit` row (model.cpp 694).
Differ on: agree today. The comment at engine.cpp 1134-1136 and model.h 640-642 name each other.
Note: the rule that an optional fill is never part of the limit lives in `fill_within_limit` (model.h 306-308), which both call. What is written twice is only the loop.

### G4b-7. The fill-deadline rule: computed in code, and restated in a sentence
Looks like: K1 on two numbers, one of them user-visible prose.
Homes: `activation_fill_deadline_ms`, search/graph.cpp 62-91 (1.1: fill start minus 4 beats; 1.0: clamp of the lead time to 250..10000 ms, fill-length plus a sixteenth of a beat);
       `fill_rule_description`, search/graph.h 76-81 ("sets the deadline 4 beats before the fill starts" / "about one fill-length before the fill starts"), which the "1.0 fills" tooltip and the fill report footer print;
       the header comment at graph.h 31-35 (4 beats; "clamped to 250..10000 ms").
Table, home 1: Ch11 -> 4 beats (`4 * tick_resolution`); Ch10 -> lead = max(250, min(fend - pad, 10000)) ms.
Table, home 2: Ch11 -> the words "4 beats"; Ch10 -> "about one fill-length", no clamp numbers.
Differ on: agree today. If the 4 changes in code, the sentence still says 4.
Note: wave A's G3-6/G1a-8 are about the words around this; this is the number behind them. The Ch10 sentence leaves out the clamp on purpose (it says "about"), so only the "4 beats" is a copy. Low weight.

## Also seen in G4b (candidates wave A already wrote)

### Also seen in G4b: G4a-1 (the six score categories)
New from my files, not in G4a-1:
- The index order of the engine's `sc[6]` is a convention with no names: 0 base, 1 combo, 2 sp, 3 solo, 4 accents, 5 ghosts. It is read at engine.cpp 820-825 and 1722-1727 (G4a-1 has these), and also written at two sites G4a-1 does not list: `branch_activate` line 962 (`c.sc[2] += e.frontend_points`) and `create_deactivated_path` line 1075 (`c.sc[2] += sp_delta`). The slot meaning is "frontend and backend SP points go into slot 2".
- The engine keeps a second running number, `p.score`, next to `sc[]`, and updates it by hand at those same three sites: the six-term sum at 826-827 in `advance`, `+= frontend_points` at 963, `+= sp_delta` at 1076. The two are compared once, at `emit_path` 1715-1720 against `score_total` (model.h 149), and the search throws if they differ. So the rule "score is the sum of six categories" is written as `score_total` (owner), as the hand sum at 826-827, and as the three paired updates. This is a K3 with a rule inside: the rule is the running total.
- The graph side: `store_*score` (graph.cpp 235-257) adds base, combo, solo, accent and ghost to both proto edges, and `spscore` to the SP proto edge only. That is the rule "the base track earns no SP points". It lives only there.
- `enumerate` narrows the graph's int64 totals to int32 (engine.cpp 165-170) with no range check. The comment at pather.h 51-55 says a real chart scores a few million at most, so this is safe in practice.

### Also seen in G4b: G5-5 (SQ_IN and SQ_OUT against SqueezeKind)
New from my files: the plain-int kinds are spelled in these engine places. Pushed: lines 1047 (`SQ_OUT`) and 1107 (`SQ_IN`). Compared against ints: 1141 (`kind == SQ_IN`), 1548 (`kind == SQ_OUT`), 1567 (`kind == SQ_IN`, a lambda handed to `core::sqin_rank`), 1962 (`os.kind == SQ_OUT`). Turned back into `SqueezeKind`: 1142 (`act_within_limit`, to ask `SPSqueeze::within_limit`) and 1967 (`rebuild`, SqIns only; the SqOut is built later by `set_sqout`). The model's own predicate for SPSqueeze is `core::is_sqin_squeeze` (model.h 504); the engine uses it at 2018 and its own int lambda at 1567. model.h 491-493 says on purpose that `sqin_rank` takes a predicate "so the engine's own squeeze records can be counted too", so the second predicate is a known choice.

### Also seen in G4b: G4a-4 (after the SP end)
See G4b-1 above: the engine's `create_deactivated_path` (engine.cpp 1033) and the graph's `late` flag (graph.cpp 430) are two calls of `after_sp_end` on the same inputs. `paid_by_sp_walk` is used by the engine at 1067 on the row offset, which is the other half of G4a-4. I found no third hand-written ordering test of this kind in my files. The other tick comparisons are different questions and are listed under Dropped leads.

### Also seen in G4b: G3-1 and G3-5 (which settings pick a result; depth mode as int)
`settings_key` (pather.cpp 65-75) is the test-cache form of the same list; see G4b-4. It writes `static_cast<int>(s.depth_mode)`, which is G3-5's pather.cpp 69 line. Also in my files: `analyze_chart` passes six settings fields positionally into `analyze_at_cap` (pather.cpp 496-498) and then stamps `legacy_fills` and `rules_fingerprint` on the record by hand (499-500).

## Parallel on purpose

### EdgeView against ScoreGraphEdge (K3: parallel on purpose)
The lead says they share five fields. The real count is higher: they share ten names (`dest`, the six score fields, `frontend_points`, `banked_phrase_ordinal`, `activation_fill_deadline_ms`) plus the choices range. The script missed the six because `EdgeView` declares them in one comma list at line 66.
What the graph type knows that the view does not: the pointers (`dest` as a node pointer), `sp_times`, `activation_initial_end_times`, `backend_begin` and `backend_end`, and the `squeeze_choices` vector. The engine reads these straight off the graph object through `eobj(i)` (engine.cpp 443, 818, 924, 1023): `sp_times` at 829-908, `activation_initial_end_times` at 934-941, and the backend range at 1061. So the engine has two ways to reach one edge, the view for the hot numbers and the raw object for the maps and lists. Each field is read through exactly one of them.
What the view knows that the graph type does not: `dest` as an index, scores narrowed to int32, the fill deadline with "no value" written as NaN, the choices as a start and end into one shared array (`Enum::choices`).
Why every entry is a per-module fact: index against pointer, int32 against int64, NaN against `optional` and begin/end against a vector are all about the engine's flat, index-packed memory layout (the comment at engine.cpp 53-57 says so). The one place a "no value" convention is chosen is `enumerate` 175-177; the conversion rules are `optional -> NaN` and `bool -> 0/1`, and neither is repeated elsewhere.
Converter: one, `enumerate()`, field by field, lines 161-185. Nothing else builds the views. Two cautions. `EdgeView v;` has no initialisers, so a new field the converter forgets to set would be garbage; today all twelve fields are set. And the compiler does not check that a field added to `ScoreGraphEdge` is copied.

### ChoiceView against SqueezeChoice (K3: parallel on purpose)
They share four fields: `chord`, `timing`, `sqin_time`, `late`. Only `late` and `sqin_time` are converted: `late` from bool to int32 (engine.cpp 182), `sqin_time` from `optional<Timecode>` to a tick or `NO_TIME` (181), `chord` from timecode to ticks (180). Converter: one, the brace-fill inside `enumerate` 179-182. It is a positional aggregate fill, so the compiler does check the field order and count (a missing field is a compile error for a brace list that is too long, and not for one that is too short, but the struct has exactly four fields).
Why per-module: the view keeps ticks only because the engine compares ticks; `timing` is carried through unchanged.
The one rule near this pair is the `late` question; see G4b-1.

### Engine `Path`, `Act`, `Variant` against core `Path`, `Activation`
Both structs named `Path` exist. The engine's is the running search state (node index, SP meter, bank and fill chain tails, `sc[6]`, `score`); core's is a stored result. The engine hands core a result only through `OutPath`/`OutAct`/`OutSq` and one converter, `rebuild` (engine.cpp 1892-2068). They share the six score slots and the tied count (see G4b-5 and the G4a-1 block above). The engine works around the name with `using MPath = ::hydra::Path` (line 26). Each engine field is a per-search fact (a chain index, a pending flag), so I consider the pair parallel on purpose apart from the two items above.

## Dropped leads

- `rebuild` in engine.cpp against the report windows' `rebuild`: K0. The engine's `rebuild` turns the search's flat decision log back into `Path` objects; the windows' `rebuild` redraws a table. They answer different questions and share only the word.
- `Path` in engine.cpp against `Path` in core/model.h: not a duplicate. It is the same word for two different things (search state against a stored result); see "Parallel on purpose" for the two fields they do share.
- "Clone Hero 1.0" and "Clone Hero 1.1" in search/graph.h and ui/library_parts.h: on my side graph.h has the single owner (`fill_rule_name`, `fill_rule_description`, `engine_mode_stamp`, `fill_rule_from_stamp`, `fill_rule_for`, lines 26-81), and I counted its callers across `src/`: they all call it; no second spelling of the strings in my files. The library_parts.h 71 line is a comment on a field that `library_dialogs.cpp` 94 fills with `fill_rule_name`, so it is G11b's to settle, and I think it is not a copy.
- Engine's `is_sqin_step` (engine.cpp 502-504) against core's `is_sqin_kind` and `is_sqin_step_on` (model.h 454-464): K0, with a confusing pair of names. Core's asks "is this step already a squeeze-in"; the engine's asks "is this the step on that tick that can be relabelled to SqIn" (it accepts Collected and Clamped, and refuses only the Activation step). They differ on purpose.
- Tick comparisons that look like `after_sp_end`: `given_back` (`step_tick >= sqout_phrase`, engine.cpp 491-493), `sqin_end_by_phrase` (`end_tick <= phrase_tick`, graph.cpp 35-37) and `sqout_position` (backend_value.h 20-26). K0: each answers a different question (is a step given back, is a moved end at or before its phrase, where a row sits against the squeezed-out chord) and each has one documented owner.
- `max_sp_bars` in graph.h and `graph_build_cap` in pather.cpp 445-447: already one owner (`graph_build_cap` calls `max_sp_bars`; graph.h 90-93 says so). Known, resolved by audit finding 260.
- `allzero_options` against the check in `attach_allzero` and the filter in `search_allzero`: they share `Path::is_allzero` and the same limit object, and the comment at pather.cpp 44-48 says so. K0.
- `pinned_end`/`target_act_ticks_` (engine.cpp 551-561, 1830) against `took_exactly`/`ended_as_pinned` (pather.cpp 209-234): the engine enforces the pins as it searches; the pather checks the result afterwards and also checks the squeeze-out, which the engine cannot. I treat it as a deliberate two-sided check, not a copy. One thing a verifier may want to look at: `pinned_end` relies on the activation ticks being sorted and the pinned ones coming first; `activation_pins` and `search_target` sort and validate (pather.cpp 307-330), so the ordering rule is stated in pather.h 143-147 and assumed in engine.h 50-55. The order is kept in one place that builds the options (`run_target`).
- `settings_key` outside the test cache: no production caller exists (grep over `src/` and `tools/`: only tests use it).

## Not read

None.
