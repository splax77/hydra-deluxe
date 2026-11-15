# Plan: the early-fill window becomes one adjustable setting

Task id: early-fill-setting-plan. Written on branch `claude/early-fill-setting-plan`, cut from main at `c10c7f54`. This plan changes no code.

## What the user asked

These lines are filed under `## Confirmed` in `docs/agents/rulings/code-and-engine.md` (lines 55 to 57).

- "make the early fill threshold an adjustable setting pinned to one number in both directions"
- "analysis settings panel, default 60ms, supports 0ms to 85ms"
- Earlier the same day: "so we should use the hit window. and stop inventing our own numbers."

In a question box the user also chose "Early fill only": the new setting moves the early-fill window and nothing else. Squeeze budgets, Rating words and timing tiers keep the fixed 85 ms hit window. That choice is quoted in the main session's brief and is not a filed line, so the Premise map marks it as the brief's quote.

## The short version

Today one fixed number, the hit window (85 ms), sets both sides of the early-fill window. After this work the user sets that number themselves, in a new `Early fill window` row of the Analysis settings panel. Call the number W. It starts at 60 ms and can go from 0 to 85 ms.

W does two jobs, as the hit window does today. A fill is dropped when Star Power came ready more than W after the fill's deadline. An activation gets the `E` mark when Star Power came ready less than W before the deadline. Both jobs read the same W, so the two sides can never drift apart.

Results are saved per W. A result analyzed at 60 and one analyzed at 85 sit side by side in the library file, and switching the box back brings the old result back, the way the SP cap and `1.0 fills` already work.

Nothing else moves. `kHitWindowMs` stays 85 and keeps its three other readers: the squeeze budget, the Rating bands and the report's hit-window text.

## Owner map

The scout's table, copied in unchanged. The scout's own file, with its Stops and search notes, is `docs/superpowers/plans/2026-11-15-early-fill-setting-owner-map.md` (removed; `git show 89950507:docs/superpowers/plans/2026-11-15-early-fill-setting-owner-map.md`). Tasks below name rows by id (O1 to O19).

| Question | Owner | Its own words | Status |
|---|---|---|---|
| O1: How late may Star Power come ready after a fill's deadline and the fill still spawns (the late side)? | `fill_refuses` in `src/core/model.h:318`: `e_offset < -kHitWindowMs`. Called by `Engine::branch_activate` (`src/search/engine.cpp:1000-1001`) and `Engine::ready_class` (`engine.cpp:856`). | model.h:127-131: "After the deadline it is the reach: the latest Star Power may come ready and still summon the fill with an early hit. fill_refuses reads it, so it decides which activations the search can find." And model.h:317: "The fill refuses to spawn: SP became ready later than the window allows." | found |
| O2: How much time to spare before the deadline still marks an activation as an early fill (the `E` mark, the early fill badge; the early side)? | `is_e0` in `src/core/model.h:322`: `e_offset < kHitWindowMs && skips == 0`. Readers: `Activation::is_e_critical` and `Activation::is_E0` (`src/core/model.cpp:473,475`), which `hardest`, `badge_timings`, `within_ms_limit` and the notation string read (`model.cpp:478-552`); `Engine::ready_class` (`engine.cpp:854,857`); `fill_within_limit` and `allzero_activation` (below). | model.h:132-136: "Before the deadline it is how little time to spare still marks the activation E-critical. is_e0 reads it." And model.h:320-321: "E0: SP came ready less than the window before the deadline, and nothing was skipped. is_e0(e_offset, 0) alone is 'E-critical'." | found |
| O3: Is an activation's early fill inside the user's Path limit? | `fill_within_limit` (`model.h:356`) reads `is_e0` only; no window of its own. Called from `Engine::act_within_limit` (`engine.cpp:1218`), `ready_class` (`engine.cpp:863`) and `Activation::within_ms_limit` (`model.cpp:535`). | model.h:353-355: "Is an activation's early fill inside a ms limit? An optional fill is never a limit's business". | found (reads the E window through `is_e0`, not `kHitWindowMs`) |
| O4: Does a path fit the all-0 rule, where a player activates at every first opportunity and may skip one early fill they never see (E1)? | `allzero_activation` (`model.h:367`): `skips == 0 \|\| (skips == 1 && is_e0(e_offset, 0))`. Readers: `Path::is_allzero` (`model.cpp:855`), `ready_class` (`engine.cpp:857`), the engine's all-0 rule (`engine.cpp:1930`). | model.h:360-366: "Does an activation fit the all-0 rule (D102)? ... (is_e0 holds for it: an E1)." | found (reads the E window through `is_e0`) |
| O5: Which group of paths does a waiting path belong to, as far as upcoming fills that refuse or would be over the limit? | `Engine::ready_class` (`src/search/engine.cpp:846`). It calls `is_e0` and `fill_refuses` and relies on the two being one cut-off. | engine.cpp:851-853: "When even this one leaves too much time to spare for an E0, none is an E0, and none refuses (is_e0 holds for every fill that refuses)." This claim holds only while the refusing reach is at least the E width (see O13). | found |
| O6: What is the width of the badge's longest figure (layout)? | `longest_activation_badge` (`src/app/path_view.cpp:78`). It uses `kSqueezeWindowMs` only. The comment (lines 83-85) bounds an early fill figure by `kHitWindowMs` (`is_e0`, `fill_refuses`) and says it lies inside the squeeze window. | path_view.cpp:79-84: "An early fill is named only on an E-critical activation, so its figure is above minus kHitWindowMs (is_e0) and at most kHitWindowMs (fill_refuses). The hit window is narrower than the squeeze window". | found. The comment names `kHitWindowMs` for an early-fill reader; the code does not read it. |
| O7: How far is the note that made SP ready from the game's own hit window (the Dynamic Hit Window row for an early fill)? | `early_fill_window_end` (`src/core/note_window.cpp:132`) with `early_fill_safe_ms` (`note_window.cpp:71`: ready side minus the early fill figure). It takes the figure from `Activation::e_difficulty`. It reads neither `kHitWindowMs` nor the early-fill window. Its own 85.0 is `kWindowBackMs` (`note_window.h:33`), the game's clamp, a different number. | note_window.h:9-11: "The row is display only. Hydra's ratings, tiers, badges and search keep kHitWindowMs (core/model.h), a separate number (D270 call 8)." | found (not an early-fill window reader; stays as is) |
| O8: Which readers of `kHitWindowMs` are NOT early-fill readers? | Squeeze budget: `src/core/squeeze_rating.cpp:133` (`budget_for_window(transfer_r, kHitWindowMs)`) and `:242` (`const double w = kHitWindowMs`; Rating bands, comment `squeeze_rating.h:107,232`). The report's text: `hit_window_text()` (`src/app/report.cpp:92`, comment `report.h:115`). The timing tiers go through `squeeze_rating.cpp`. A retired settings key: `config.h:90`. | model.h:101-104: "the squeeze budgets, the Rating words, the report's timing tiers and the early-fill reach all read it". `kHitWindowMs` has exactly these four code readers in `src/` and `tools/`: `model.h:318`, `model.h:323`, `squeeze_rating.cpp:133,242`, `report.cpp:92`. A grep of `src/` and `tools/` for `kHitWindowMs` found no other. | found |
| O9: Which engine readers of the early-fill window exist outside `src/core/model.h`? | Only the callers in O1 to O5. `tools/` has no C++ reader of `fill_refuses`, `is_e0`, `allzero_activation`, `e_offset` or `fill_within_limit` (grep of `tools/` `.cpp` and `.h`; the only hits are the Python probes under `tools/ch_probe` and `tools/ch_mcp/tests`). `src/pathbuilder` and `src/cli` read it only through `Activation` methods. | model.h:309-315: "Clone Hero's early-fill rule and the E0, stated once around one window ... The search's branch_activate and its group key (ready_class) ask these; so do Activation's E tests." | found |
| O10: Where does an analysis setting enter the search today? (traced for `legacy_fills` and `mslimit`) | `Settings` field (`src/app/config.h:187` `legacy_fills`; `:163-164` `mslimit_*`) -> `Settings::to_analysis_settings()` (`src/app/config.cpp:402`) -> `SearchSettings` (`src/search/pather.h:32-43`; `AnalysisSettings` in `src/app/analysis.h` is the same value). Then: the fill rule goes `search_graph` (`src/search/pather.cpp:261`, `fill_rule_for(settings.legacy_fills)`) into `ScoreGraph`, and the Path limit goes `main_search_options` (`pather.cpp:15`) into `EngineOptions.ms_filter` (`src/search/engine.h:25-30`). `record.legacy_fills` and `record.ms_limit` are set at `pather.cpp:26,546`. | engine.h:22-24: "Everything one run_search call can be asked to do, in one value". pather.h:37-40: "The same flag, under the same name, as Settings::legacy_fills and store::Lens::legacy_fills". Today no early-fill value is on `EngineOptions`, `ScoreGraph` or `SearchSettings`: `fill_refuses` and `is_e0` take only `e_offset` and read a constant (see O1 and O2). | found for the route; the window has no value on that route today (see "Carry out" notes) |
| O11: Which file line holds a numeric analysis setting, and how is an out-of-range value handled? | The key table `keys()` in `src/app/config.cpp:181` (`number("mslimit_value", &Settings::mslimit_value, -kWindowMs, kWindowMs)` at :193; `number("sp_cap", ..., 1, kNoCeiling)` at :199). `pull_into_range` (:211) clamps to the nearest edge, run by `read_value` (:235-238). `Settings::clamp` (`config.h:238`, `config.cpp:305`) is the one range owner for the panel box and the tools. Unknown keys are refused; retired keys are skipped (`kRetiredSettingsKeys`, `config.h:85-91`). | config.h:233-237: "A number setting pulled into its allowed range: the one the key table in config.cpp keeps, which its box reads too. A value outside the range lands on the nearest edge (D51 Q14, D270 calls 1 and 2)." Note: `hit_window_ms` is on the retired list (`config.h:90`, retired in 2.14.0), so a new key cannot reuse that name without removing it from the list. | found |
| O12: Where is a panel setting listed, reset, summarized and worded? | `kAnalysisFields` (`config.h:348-359`): one row per `Settings` field with its `AnalysisSetting` and its `KeyPart` (ChartMode, Cap or Lens). Read by `Settings::with_analysis_defaults` (`config.cpp:359`) and `Settings::analysis_changes` (`config.cpp:367`). Words: `change_phrase` (`src/ui/library_dialogs.cpp:88-105`, a switch over `AnalysisSetting`, throws `logic_error` when a setting has no phrase); button label `settings_button_label` (`library_dialogs.cpp:129`: "Analysis settings: ..."); `settings_at_defaults` (:133); `reset_settings_tooltip` (:137); `batch_settings_summary` rows (:70-83, drawn at :358-362 as Difficulty, SP cap, Fills, Score range, Path limit); `describe_settings` (`config.cpp:459`) used by the batch confirm and `hydra_batch`'s header (`src/cli/batch.cpp:145-150`). Panel rows: `src/ui/settings_bar.cpp` (`render_chart_group` :81, `render_rules_group` :123 holds "SP cap" and "1.0 fills", `render_paths_group` :148 holds "Score range" and "Path limit", `render_reset_button` :200). | config.h:343-347: "The analysis set, stated once: every field the Analysis settings panel sets ... a new analysis setting is a new row here (concept audit M13). Nothing can notice a field left out of it." library_dialogs.cpp:102-103: "A panel setting added to app::AnalysisSetting without words here; the Reset test in tests/test_config.cpp walks every one". | found |
| O13: When both sides of the window are pinned to one number, does anything assume the refusing reach is wider than the E width? | `ready_class` (O5) and the model.h comment at lines 127-136. With one number on both sides, `fill_refuses(e)` is `e < -w` and `is_e0(e,0)` is `e < w`; every refusing offset is an E0 offset, so the engine.cpp:851-853 claim holds. | model.h:320: "E0: SP came ready less than the window before the deadline"; engine.cpp:852-853 (quoted in O5). | found. The claim holds only for one shared number. Two different numbers, with the reach narrower than the E width, would break it. The user's "one number in both directions" keeps it true. |
| O14: Which stored value says a saved result was made under a given analysis setting, so results are saved per setting? | `store::Profile` = chartmode + `CapQuery` + `Lens` (`src/store/record_store.h:103-156`). `Lens` holds `ms_enabled, ms_value, depth_mode, depth_value, legacy_fills`. The column list is `kLensColumnList` (`src/store/record_store.cpp:183`); the `profiles` table's UNIQUE key (:257), `lens_match` (:383), `bind_lens` (:391) and the profile read (:915) are all built from it. `Settings::lens()` (`config.cpp:424`) builds it from the settings; `Settings::profile()` (:432) wraps it. Which key part a panel field belongs to is `KeyPart` in `kAnalysisFields`. `prepare_record` (`record_store.h:294-301`) throws when a profile names settings the analysis did not run under (the `legacy_fills` check is at `record_store.cpp:654-660`). | record_store.h:103-109: "The rest of the settings a run happened under ... Two runs of the same chart at the same cap under different lenses are two results, neither overwriting the other." record_store.cpp:180-183: "The lens's columns, in Lens's field order ... The profile rows, lens_match and convert_results's read of an older file's results are all built from this list." `static_assert(count_list_names(kLensColumnList) == 5, ...)` at :207 ("grow it and Lens with the list"). | found. Nothing carries the early-fill window today: `Lens`, `HydraRecord` and `profiles` have no field for it. |
| O15: Does an older library file gain a new column on its `profiles` table? | Not for `profiles`. The only column adds are `charts.album` (`add_album_column`, `record_store.cpp:848-861`, kind `LacksAlbumColumn`) and `records.failure_kind/failure_text` (`add_failure_columns`, :863-876, kind `LacksFailureColumns`). The file's kind is decided by `kind_of` in `src/store/library_file.cpp:71-105`. The `profiles` table is made by `CREATE TABLE IF NOT EXISTS` (`record_store.cpp:247-257`), so an existing file's table is left as it is. | library_file.cpp:69-70: "The layouts, oldest first. Each test reads only what a release wrote". | none for `profiles`. Searched: `ALTER TABLE` and `ADD COLUMN` across `src/store`, `src/app`, `tools`; `LibraryFileKind` and `kind_of` in `library_file.cpp`; `kLensColumnList` users. This is a storage question, not a game question, but a row marked `none` stops the work, so the main session decides whether it matters (it matters only if the window joins `Lens`). |
| O16: Which stamp or rule decides whether an old saved result stays Ready when an analysis input changes? | `kResultsStamp` (`src/store/stored_versions.h:55`), compared by `StampRule::is_current`; used by `rank_row` (`record_store.cpp:445-454`: `current = kResultsStamp.is_current(hyversion)`, `same_rules = rules_fp == fixed_fp`, `ready() = current && same_rules`). The rules fingerprint is `Rules::fingerprint()` (`src/core/rules.h:56`), the hydra_rules.ini fields. The lens columns (O14) decide which profile a lookup reads. Today `kResultsStamp` is `3.1.2+efillhitwindow`, unreleased (3.1.1 is the latest release), set by D344. | stored_versions.h:42-54: "BUMP IT, to the version of the release that ships the change, whenever analysis output changes in a way the hydra_rules.ini fingerprint doesn't already catch ... Changes that ship in the same release share one bump (decision D23, 2026-10-03). A change to how a saved figure is shown is not a bump". ADR 0018 line 59-68 says the same. | found. By its own words one unreleased bump already covers every change that ships in 3.1.2, so a second bump is not required by the rule. Whether a new Lens column needs a file-kind step is O15. |
| O17: Which tools take analysis settings, and how? | `hydra_batch` (`src/cli/batch.cpp:104`): `load_tool_settings(settings, legacy_fills)` with only the `--legacy-fills` flag, so every other setting comes from `hydra_settings.ini`; it prints Score range, SP cap, Path limit, Fills (:143-151) and files results under `settings.batch_run()` (:119). `hydra_bench` (`tools/bench.cpp:273-277`, `ini_settings`): `load_tool_settings(st, g_legacy_fills)` for `--engine` and `--parse`; folder mode (:96) uses `app::Settings` defaults; its fixed yardstick (:247-252) uses bare `SearchSettings` defaults on purpose ("Fixed on purpose"); `--ini` (:1131) loads a file. `hydra_replay` (`tools/replay.cpp:189-234`, `settings_from`): starts from `Settings s` defaults, not the ini, and applies only `--cap`, `--ms`, `--depth-mode`, `--depth`, `--legacy-fills`, `--difficulty`, `--prodrums`, `--bass2x`, `--noteshuffle`, then clamps the number fields through `Settings::clamp` (:230-232). `hydra_fillcompare` (`src/cli/fillcompare.cpp:77`): `load_tool_settings(settings, nullopt)`. The Path Builder (`src/pathbuilder/pb_state.h:132-139`, `pb_state.cpp`): reads `Settings::load()` and keeps `Settings::batch_run()`. | config.h:325-331: "How a command-line tool loads its settings: Settings::load_for_command_line when the tool has a fill-rule flag of its own ... else Settings::load." replay.cpp:187-188: "The app's settings with the typed flags applied. A flag not given keeps the app's default, so this tool keys and prices a run exactly as the app does." | found. A new setting reaches `hydra_batch`, `hydra_bench --engine/--parse`, `hydra_fillcompare` and the Path Builder through the ini; it reaches `hydra_replay` only by its struct default unless a flag is added. |
| O18: Where do the docs, the single-owner scan and the tests state the window's number or read `kHitWindowMs`? | Docs: `CONTEXT.md:610-620` ("Early fill (E)", marker `default: kHitWindowMs` at :613) and `:683-690` ("Hit window", marker at :685); `docs/UserGuide.md` settings table (:127-136, markers `default: Settings::mslimit_*`) and the early fill paragraphs (:263 "under 85 ms, the hit window", :269, :273-277, :283); `docs/differences-from-public-hydra.md:30` and `:146`; `docs/clone-hero/04-fills.md` FL-24 (:242-249); `docs/clone-hero/03-timing.md:365`; `docs/releases/release-next-notes.md:3`. Tests: `tests/test_model.cpp:152-200` and `:575-584` (boundary checks at `kHitWindowMs`); `tests/test_fills_and_skips.cpp:74,108-110` (edge 7285 = deadline + 85, pinned) and `:141-160` (70 ms early fill); `tests/test_docs_match_code.cpp:320-330` (the default-value table has `kHitWindowMs` and `Settings::mslimit_*`; markers are checked at :477-497); `tests/test_replay_schedule.cpp:355` (asserts the source text `constexpr double kHitWindowMs = 85.0;` is in `model.h`); `tests/test_squeeze_rating.cpp:751,771` (squeeze budgets, not early fill); `tests/test_perf_digest.cpp` with `tests/song_digest.h` (engine digests pinned under `digest_settings`; repinned at 6fd37fd3 for the hit-window E mark); `tests/single_owner/rules/core.json` (rows at :344-353, :419-466, :1026-1039, :1855-1876 name `kHitWindowMs` as owner or flag `+/- kHitWindowMs` and `< kHitWindowMs` compares) and `tests/single_owner/rules/app.json:343-358` (`hit_window_text`). `tests/test_engine_sweeps.cpp` has no early-fill window number: its matches are about zero-slack early fills (D85) and the 0 ms limit. | core.json:466 note: "The window has one width, kHitWindowMs, on both sides of the deadline. ... Neither flags every read of the hit window, which the squeeze budgets and the report also read." core.json:1855: `"owner": "kHitWindowMs in src/core/model.h"`. | found |
| O19: What does the early-fill window do at 0 ms? | `fill_refuses(e)`: `e < -0.0` is `e < 0`, so any fill whose SP came ready even a hair after the deadline is refused; exactly on the deadline (e == 0) still spawns (strict compare, as the test at `test_model.cpp:192` pins for the window edge). `is_e0(e, skips)`: `e < 0.0 && skips == 0`, so no activation that spawns (`e >= 0`) is ever E, and no E1 exists (`allzero_activation` reduces to `skips == 0`). A path then has no early fill at all. `early_fill_difficulty` (`model.h:327`) is `-e + 0.0`, no division. `timing_needs_hitting` is `>= 0.0`. No reader divides by the window or by `kHitWindowMs`: the readers in O1 to O9 only compare or negate. The squeeze rating's `budget_for_window` and `w` (`squeeze_rating.cpp:133,242`) take the fixed 85, which the request leaves alone. | model.h:317-323 (O1 and O2 quotes). `tests/test_model.cpp:187-200` pins "exactly at the reach still spawns" for the late side and "exactly the hit window to spare: no E" for the early side. | found. No reader breaks on 0. At 0 the early fill feature is effectively off. |

Its two Stops are settled by the user's lines above. Stop A (60 against "use the hit window"): the later lines make the window the user's own setting with a default of 60. Stop B ("never a setting" in `CONTEXT.md` and `model.h`): task T5 rewrites those texts, and the `CONTEXT.md` entry changes because the user changed the term.

## The design, one decision at a time

### How W reaches the engine (O1, O2, O3, O4, O5, O10)

The four early-fill questions in `src/core/model.h` take the window as an argument instead of reading `kHitWindowMs`. Those are `fill_refuses`, `is_e0`, `fill_within_limit` and `allzero_activation`. No other rule changes: the same strict compares, the same owners, one more argument.

W travels the route the owner map traced for `legacy_fills` (O10), the one that reaches every search. It starts as `Settings::early_fill_window_ms`. `Settings::to_analysis_settings()` copies it onto `SearchSettings`. `search_graph` in `src/search/pather.cpp` passes it to the `ScoreGraph` constructor, as a new last argument defaulting to the default below, beside the fill rule and the rules it already passes. The graph keeps it and has a getter, `early_fill_window_ms()`. `run_search` reads it off the graph and hands it to the engine, the way it already hands over `graph.rules().backend_leeway_ms` (`src/search/engine.cpp:2164`). The engine passes it to the four questions.

It rides the graph and not `EngineOptions` because `EngineOptions` is built in places that have no `SearchSettings`: `allzero_options()` takes no arguments, and `run_target` (`pather.cpp:289`) builds its own options from a graph alone. Those two serve the all-0 pass, `search_target`, `paths_from_pathstring`, `best_finish`, the Path Builder's coach and the path costs. Every one of them runs on a graph that `search_graph` built from the settings, so the graph is the one place that reaches all of them (P26). `tools/bench.cpp:317` builds a graph by hand from the settings and passes W the same way.

A drawn activation must answer "is this an E?" with the W its search ran under, not with whatever the box says now. So the engine writes W onto each `Activation` beside `e_offset`, in a new field `e_window_ms`. `Activation::is_e_critical`, `is_E0`, `within_ms_limit` and `Path::is_allzero` pass that field to the same four questions. This is a carry-out of a value the engine already uses, not a new rule.

The record gets the same value once, as `HydraRecord::early_fill_window_ms`, set in `pather.cpp` beside `record.legacy_fills`, from the same `SearchSettings`. The store uses it to refuse a mismatched profile (see below).

### Where the default lives

The user's 60 gets one home: `kDefaultEarlyFillWindowMs = 60` in `src/core/model.h`, beside `kHitWindowMs`, with the ruling named in its comment. `Settings`, `SearchSettings`, the `ScoreGraph` constructor, `Activation`, `HydraRecord` and `Lens::from` all take their default from it. No file writes a second 60.

### The settings file (O11)

The new key is `early_fill_window_ms`. The name `hit_window_ms` is on the retired list (`src/app/config.h:90`) and stays there, because the hit window is still fixed.

The key is a `number(...)` row in the key table in `src/app/config.cpp`, with the range 0 to 85. The top edge is written as `static_cast<int>(kHitWindowMs)`, the way `kWindowMs` is already written from `kSqueezeWindowMs` on line 158, so no second 85 exists. A value outside the range loads at the nearest edge through the existing `pull_into_range`, and `Settings::clamp` serves the panel box and the tools. Text that is not a whole number is refused by the existing `read_value`. Nothing new is needed for any of that.

### The panel row (O12)

The row goes in the "Clone Hero rules" group in `src/ui/settings_bar.cpp`, under `1.0 fills`. It follows the panel's one row rule through `begin_row`: the name and its (?) on the left, only the control on the right.

- Name: `Early fill window`
- Control: a whole-number box (`ImGui::InputInt("##earlyfillwindow", ...)`), then the word `ms`. It is the same kind of box as Path limit's, with the same default step of 1 and the same `px(100)` width, so no new number is added. Each step goes through `Settings::clamp` and `app.edit_settings()`, as the SP cap box does.
- (?) tooltip: "How far from a fill's deadline your Star Power may come ready and still count as an early fill. Ready up to this many ms after the deadline, you can still summon the fill by hitting early; later than that, the fill never spawns. Ready less than this many ms before the deadline, the activation is marked E. At 0 no activation is an early fill. 85 ms, the hit window, is the most it allows."

Each tooltip sentence is checked against the code. "Later than that, the fill never spawns" is `fill_refuses` (`e_offset < -W`). "Less than this many ms before" is `is_e0` (`e_offset < W`). "At 0 no activation is an early fill" is O19. The 85 is printed through `hit_window_text()` (`src/app/report.cpp:92`), not typed.

`kAnalysisFields` gets one new row after `legacy_fills`: `{&Settings::early_fill_window_ms, AnalysisSetting::EarlyFillWindow, KeyPart::Lens}`. `AnalysisSetting` gets `EarlyFillWindow` after `Fills`. Reset to defaults and the summary then pick it up with no further code, because both walk the table.

### The toolbar button's words (O12)

`change_phrase` words a number setting as its name, a space, then the value from `describe_settings`: "SP cap 6 bars", "Path limit 10 ms". The new case follows it: `"Early fill window " + values.early_fill`. `SettingsText` gains `early_fill`, filled by `describe_settings` with the existing `format_ms_whole`.

So with the box at 85 and everything else default, the button reads `Analysis settings: Early fill window 85 ms`. At the default it reads `Analysis settings: defaults`. The Reset tooltip reads `Resets: Early fill window 85 ms`.

### How results are saved per W (O14, O15, O16)

W joins the lens, the existing owner of "which settings was this result made under" (O14). `Lens` gets a sixth field, `efill_window_ms`, and `kLensColumnList` gets a sixth name. `lens_match`, `bind_lens`, the `profiles` table's columns and its UNIQUE key are all built from that list, so they follow. `prepare_record` gains one check beside the fill-rule check: it throws when the profile's window differs from the record's.

I looked for a way to carry W without a new column and found none that fits. The chart mode and the SP cap are other key parts with other meanings. The rules fingerprint belongs to `hydra_rules.ini`, which the user turned down for this setting.

**The O15 gap.** No code adds a column to an existing file's `profiles` table. The scout's search, repeated here: `ALTER TABLE` and `ADD COLUMN` across `src/store`, `src/app` and `tools` find only `add_album_column` and `add_failure_columns`, and `record_tables_sql` makes `profiles` with `CREATE TABLE IF NOT EXISTS`, which leaves an old table alone. So T3 adds the step. This is a storage step, not a game rule.

A plain `ADD COLUMN` is not enough, because the table's UNIQUE key would still leave the new column out, and two profiles that differ only in W would collide. So the step rebuilds the table inside one transaction: rename the old table, create the new one from the same SQL a fresh file uses, copy every row across with its `id` kept, then drop the old table. `records` and `record_paths` point at profiles by `id`, so they need no change. The table is tiny (one row per settings combination), so the rebuild is quick. A failure rolls the whole step back and raises `DatabaseUpgrade`, as the two existing column steps do.

`kind_of` in `src/store/library_file.cpp` gets one more layout, `LacksEarlyFillColumn`: a `profiles` table without the new column. It is the last test, after the album test, because `kind_of` returns the first layout that matches. So a file that lacks the album too (the v2.12.1 fixture) still reads `LacksAlbumColumn`, as today. The step does not depend on the kind: `set_up_schema` runs it for every kind older than `Current`, after `add_failure_columns` and `add_album_column`, and the step itself asks the file whether `profiles` lacks the column, on the store's own handle. That is how `add_album_column` asks through `charts_table_lacks_album`, so the new probe sits beside it in `library_file.cpp`. Opening such a file shows the user nothing beyond the open itself: the callers of `classify_library_file` act only on `TooOld` and report progress only for `Convertible` (P16). A Convertible file (2.2.0 to 2.7.0) gets a fresh `profiles` table anyway; its `convert_results` read of the old `results` table keeps its own list of the five old lens columns, because that table never had a sixth.

**What an old row is filed under.** No saved row records a window. Every row saved before this change is filed under the default, 60.

**The stamp moves, once.** `kResultsStamp` changes from `3.1.2+efillhitwindow` to `3.1.2+efillwindow`, unless the user answers question 1 the other way. Rows stamped `3.1.2+efillhitwindow` were analyzed at 85 on both sides (that stamp first appeared in merge `0bfa2dce`, so only dev builds wrote it: P28), and filed under 60 with a current stamp they would read Ready at 60, which is false. The bump is what stops that. The stamp is unreleased, so this is still the one bump 3.1.2 ships (D23), not a second one: a 3.1.1 user sees one change of stamp either way.

**How old results read once this ships.**

- Results from 3.1.1 and earlier (late side 85, E mark 60) already read Stale under today's main. They still read Stale, at the default setting, where the user starts. Nothing new for them. `docs/releases/release-next-notes.md` already says so.
- Results saved on a dev build since merge `0bfa2dce` (2026-11-15, both sides 85) read Ready today. After this change they read Stale. That is a visible change nobody told the user about. It is question 1 below.
- No result vanishes at the default setting. At any other W a chart reads as not yet analyzed there until it is analyzed at that W, the same as changing the SP cap.

### The tools (O17)

`hydra_batch`, `hydra_bench --engine/--parse`, `hydra_fillcompare` and the Path Builder load `hydra_settings.ini`, so they get W with no code of their own. `hydra_bench`'s fixed yardstick uses bare `SearchSettings` defaults and so runs at 60, which changes the work it times: bench figures from before and after this change are not comparable on charts with fills 60 to 85 ms from a deadline (P32).

`hydra_replay` starts from the app's defaults, not the file. It gets one flag, `--efill <ms>`, handled like `--ms`: read through `setting_flag_number`, then clamped by `Settings::clamp` in the loop at the end of `settings_from`. With no flag it runs at the default, 60.

### The 0 ms edge (O19)

At 0, `fill_refuses` is `e_offset < 0` and `is_e0` is `e_offset < 0 && skips == 0`. So a fill whose Star Power came ready any time after the deadline is refused, one ready exactly on the deadline still spawns, and no activation that spawns is ever an E. `allzero_activation` reduces to `skips == 0`. Nothing divides by W. T1 pins each of these with a test.

### What stays as it is

`kHitWindowMs`, `squeeze_rating.cpp`, `hit_window_text()` and the Dynamic Hit Window row (O7, O8) are not touched. `longest_activation_badge` (O6) keeps its code; only its comment changes, because an early-fill figure is now bounded by W, which is never wider than the hit window.

## What the user will see

Every screen below reads W through the owners in O2 to O4. The comparison is the new default, 60, against today's main, 85.

- **Paths tab, the `E` in a path's notation.** An activation whose Star Power came ready between 60 and 85 ms before the deadline loses its `E`. An activation whose Star Power came ready between 60 and 85 ms after the deadline is gone: the fill no longer spawns there, so the path activates on another fill or drops out, and a chart's best score can fall.
- **The early fill badge and the "Early fill" line.** They appear on the same activations as the `E`. The figure they show now lies inside 60 ms on either side, where today it lies inside 85.
- **The all-0 section.** A path may pass over one early fill and still be all-0. That allowance now covers fills with less than 60 ms to spare, not 85, so some charts' best all-0 path changes.
- **The path report's `Early fill (ms)` and `Hardest ms` columns.** No early-fill figure above 60 at the default.
- **The Analysis settings panel.** One new row, `Early fill window`, under `1.0 fills`.
- **The toolbar button.** `Analysis settings: Early fill window 85 ms` when the box is moved to 85, and so on for any value but 60.
- **Library status.** See "How old results read" above, and question 1.
- **The Path Builder, the path costs and a pinned path read from text.** They run the same engine on the same graph, so a pinned activation that needs a fill 60 to 85 ms late is no longer realizable at the default.
- **`hydra_bench`.** Its fixed yardstick runs at 60, so its timings are not comparable with runs from before this change on charts with such fills.

Against release 3.1.1 (late side 85, E mark 60) the `E` marks at the default match again. The one difference is on the late side, where fills between 60 and 85 ms late no longer spawn.

Real chart examples need a build. Live step LS1 gets them from `testdata/input`: it runs `hydra_replay` on each chart with `--efill 85` and with `--efill 60` and keeps the charts whose printed paths differ. The main session shows the user those lines, before and after, with question 1 (agent rule 11).

## Questions

1. **Results saved today.** Any result you saved on a dev build since this morning's merge (the 85 ms window) was analyzed at 85. Two answers. (a) Mark them out of date, the plan's way: after this change those charts show `Stale` at every box value until you analyze them again, and the settings stamp moves to `3.1.2+efillwindow`. (b) Keep them: the library file files those results under 85, so a chart shows `Ready` when the box is at 85 and `Not analyzed` at the default 60, and the stamp does not move. Results from 3.1.1 already read `Stale` and read `Stale` either way. Which?

   The user chose "Mark them Stale": the stamp moves to `3.1.2+efillwindow` (T3).
2. **The batch confirm box and `hydra_batch`'s header.** They list the settings a batch runs under (Difficulty, SP cap, Fills, Score range, Path limit). Your words did not ask for a new line there. Two answers. (a) Leave them: a batch at 85 and a batch at 60 print the same settings lines. (b) Add an `Early fill window` line to each, worded like `Path limit`. Until you say, the plan leaves both as they are.

   The user chose "Add a line": an `Early fill window` line goes in each, worded like `Path limit` (T2).

## Tasks

Each task is one Opus executor, in its own worktree, inside 100 tool calls and 20 minutes. Executors do not build; the main session builds each branch and takes its test list from `hydra_precheck --tests`. The tests each task names are the ones it writes or edits.

T1 and T5 start at once (`earlyStart`), in parallel. T3 starts from T1's tip. T2 starts from T3's tip. T4 and T6 start from T2's tip, in parallel. The main session joins the branches as they finish. One fresh reviewer reviews the joined set, once, because the tasks stack on each other and no branch builds alone past T1.

### T1: the engine reads W

Rows: O1, O2, O3, O4, O5, O6, O9, O10, O13, O19. `earlyStart`: yes. Depends on: nothing.

Owns `src/core/model.h`, `src/core/model.cpp`, `src/search/engine.h`, `src/search/engine.cpp`, `src/search/graph.h`, `src/search/graph.cpp`, `src/search/pather.h`, `src/search/pather.cpp`, `tools/bench.cpp` (the one `ScoreGraph` it builds at line 317 only), `src/app/path_view.cpp` (comment at lines 79 to 85 only), `tests/test_model.cpp`, `tests/test_fills_and_skips.cpp`, `tests/song_digest.h`, and any other test under `tests/` that builds an `Activation` or a `ScoreGraph` by hand and fails to compile or changes meaning (`record_fixtures.h`, `test_activation_rows.cpp`, `test_path_list.cpp`, `test_preview_boxes.cpp`, `test_report_rows.cpp`, `test_tied_variants.cpp`, `test_note_window.cpp`, `test_engine_sweeps.cpp`, `test_fast_tempo.cpp`). The store tests belong to T3, which owns `tests/test_store_results.cpp`.

The work is the "How W reaches the engine" and "Where the default lives" sections above. `tests/song_digest.h`'s `digest_settings` states its window outright as the hit window, so the pinned digests keep describing the search they were pinned on (P17). A test that analyzes a chart through a default `Settings` or a default `ScoreGraph` now runs at 60 where it ran at 85. The executor reads each such test for a fill 60 to 85 ms from a deadline and, where the expectation depends on it, states the window the expectation was written for. The main session's build finds any it missed, and a fresh executor fixes those.

Acceptance criteria:

- `fill_refuses`, `is_e0`, `fill_within_limit` and `allzero_activation` take the window, and nothing in `src/` or `tools/` reads `kHitWindowMs` for an early-fill question. Check: `tests/test_model.cpp` boundary cases run at W = 60 and W = 85 (ready exactly W late still spawns; a hair later refuses; exactly W to spare is no E; a hair less is an E). They fail if either side reads a constant.
- Both sides read one W. Check: a `test_model.cpp` case walks several W values and asserts `is_e0(e, 0, W)` holds for every `e` where `fill_refuses(e, W)` holds (O13). It fails if the two sides take different numbers.
- The 0 ms edge. Check: `test_model.cpp` cases at W = 0: ready a hair after the deadline refuses, exactly on it spawns, no `e_offset` at or above 0 is an E, and `allzero_activation(e, 1, 0)` is false for every such `e`.
- The search follows the setting, on every route. Check: in `tests/test_fills_and_skips.cpp`, the existing 70 ms early-fill chart is analyzed twice through `analyze` with `SearchSettings::early_fill_window_ms` at 85 and at 60. At 85 the activation is found and is an E; at 60 it is not found on that fill. The same chart is then run through `search_target` and `paths_from_pathstring` on a graph built by `search_graph` at 85 and at 60, and the pinned activation is realizable at 85 and not at 60. The all-0 pass is read the same way. Each fails if W is dropped on its way to that search (the graph is the one carrier, so a route that skips it falls back to 60).
- Every activation and record carries its W. Check: the same tests assert `Activation::e_window_ms` and `HydraRecord::early_fill_window_ms` equal the setting it ran under, for the main paths and the all-0 path.
- The default is 60 in one place. Check: a `test_model.cpp` case asserts a default `SearchSettings`, a default `ScoreGraph` (through its getter) and a default `Activation` all hold `kDefaultEarlyFillWindowMs`.
- The squeeze side is untouched. Check: the diff holds no change to `src/core/squeeze_rating.cpp` or `src/app/report.cpp`; `tests/test_squeeze_rating.cpp` and `tests/test_replay_schedule.cpp:355` pass unedited.

### T3: the library file saves results per W

Rows: O14, O15, O16. Depends on: T1 (it reads `HydraRecord::early_fill_window_ms`). Starts from T1's tip.

Owns `src/store/record_store.h`, `src/store/record_store.cpp`, `src/store/library_file.h`, `src/store/library_file.cpp`, `src/store/stored_versions.h`, `testdata/store/README.md`, `tests/test_store_open.cpp`, `tests/test_store_records.cpp`, `tests/test_store_results.cpp`, `tests/test_library_jobs.cpp`.

The work is the "How results are saved per W" section above. `convert_results` selects `kLensColumnList` from a 2.2.0 to 2.7.0 file's old `results` table (`record_store.cpp:903-905`), which has no sixth column, so it gets its own five-name list and the new field keeps its default there (P27). The P16 read is done: opening an older-kind file shows the user nothing beyond the open itself, and the new kind adds nothing to that.

Acceptance criteria:

- Two windows are two results. Check: `test_store_records.cpp` saves one chart under profiles that differ only in `efill_window_ms`, then looks each up and gets its own record back. It fails if the column is missing from the list or the key.
- A mismatched record is refused. Check: `prepare_record` with a profile at 85 and a record at 60 throws `std::invalid_argument`.
- An older file is rebuilt in place. Check: `test_store_open.cpp` opens a copy of `testdata/store/v2.12.1_library.db`, which classifies as `LacksAlbumColumn` before (it also lacks the album) and as `Current` after; after opening, every profile row keeps its `id` and its other columns; every record is still reachable under its old profile; the old row's window is `kDefaultEarlyFillWindowMs`; and two profiles differing only in W can then be saved. A second case builds a current-layout file whose `profiles` table lacks the column and checks it classifies as `LacksEarlyFillColumn`, then as `Current` after opening.
- The rebuild is all or nothing. Check: a test leaves a table named like the step's temporary one in the file, so the step's rename throws. It asserts the error kind is `DatabaseUpgrade` and the file reads exactly as before (the old `profiles` table, every row, `records` untouched). A failure at the later points is `in_transaction`'s rollback, which the existing `add_failure_columns` and `convert_results` cases already exercise.
- The older kinds still open. Check: the existing Convertible, `LacksFailureColumns` and `LacksAlbumColumn` cases in `test_store_open.cpp` pass and end with the new column present.
- The stamp is `3.1.2+efillwindow` and accepts only itself. Check: `test_store_results.cpp` saves a row stamped `3.1.2+efillhitwindow` and asserts it reads Stale.
- No old result vanishes at the default. Check: the rebuilt fixture's rows are found by a lookup at the default lens and read Stale, not absent.

### T2: the setting, its file line and its words

Rows: O10, O11, O12, O14. Depends on: T1 and T3. Starts from T3's tip.

Owns `src/app/config.h`, `src/app/config.cpp`, `src/ui/library_dialogs.cpp`, `tests/test_config.cpp`.

The work is the "settings file" and "toolbar button's words" sections, plus `Settings::lens()` passing W to `Lens::from` and `to_analysis_settings()` passing it on.

Acceptance criteria:

- The key round-trips. Check: `test_config.cpp` saves and loads `early_fill_window_ms` and gets the same value; a missing line loads 60.
- The range is 0 to 85 and clamps. Check: the existing case "a value outside its range loads at the nearest edge" gains 200 (loads 85) and -5 (loads 0); "Settings::clamp is the one range every caller asks" gains the field. Text that is not a number is refused.
- `hit_window_ms` stays retired. Check: the existing retired-keys case passes unedited.
- Reset and the summary cover it. Check: "settings reset: puts back each panel setting and keeps the rest" walks `kAnalysisFields`, so it fails if the row or the phrase is missing; a new check asserts the exact text `Analysis settings: Early fill window 85 ms` and `Resets: Early fill window 85 ms`.
- The profile follows the setting. Check: `Settings::profile()` at 60 and at 85 differ in the lens and nowhere else; `batch_run()`'s search settings and profile hold the same W.

### T4: the panel row

Rows: O12. Depends on: T2. Starts from T2's tip.

Owns `src/ui/settings_bar.cpp` (its "eight settings" comment on Reset becomes nine), `src/ui/library_parts.h` (if the tooltip text gets a function, as `legacy_fills_help_text` has), `tests/ui/uitest_library.cpp`, and `tests/ui/uitest_details.cpp`, where `settings-lock` lives (P31).

The work is "The panel row" section above.

Acceptance criteria:

- The row is on screen with its exact name and tooltip. Check: a new `hydra_uitest` test, `settings-early-fill`, opens the panel, finds `Early fill window`, and compares the tooltip with the help-text function's return, which the test also checks contains `hit_window_text()`.
- The box changes the setting and clamps. Check: the test types 85 and reads the button label `Analysis settings: Early fill window 85 ms`; types 200 and reads 85 back; types -5 and reads 0 back.
- The row follows the one row rule. Check: the row is made by `begin_row`; the reviewer reads the diff for it, and LS5's capture shows it.
- Reset puts it back. Check: the existing `settings-reset` test gains the box.
- The box locks during a batch. Check: the existing `settings-lock` test gains the box.

### T6: hydra_replay's flag

Rows: O17. Depends on: T2. Starts from T2's tip.

Owns `tools/replay.cpp` (its usage text too) and `tests/test_cli.cpp`, where the `--ms` and `--depth` flag cases run the built `hydra_replay` (P30).

Acceptance criteria:

- `--efill <ms>` is a whole-number flag like `--ms`. Check: a `test_cli.cpp` case runs `hydra_replay dump --chart <fixture> --efill abc` and asserts exit code 2 and the line `--efill takes a whole number, not "abc"` (the shape the `--ms` case asserts); it runs `--efill 85` and `--efill 900` and asserts exit 0 (a number past the range is pulled in, as for `--ms`). That the pull-in lands on 85 is `Settings::clamp`'s case in T2's `test_config.cpp`; `settings_from` is in the tool's own namespace and is not called by a test.
- The usage text names the flag. Check: the reviewer reads the diff of `usage()`; no doc page lists `hydra_replay`'s setting flags (a search of `docs/` for `--depth-mode` finds only the derivation audit).

### T5: the words in docs, comments and scans

Rows: O18, and Stops A and B. `earlyStart`: yes. Depends on: nothing for the text; its markers name `kDefaultEarlyFillWindowMs` and `Settings::early_fill_window_ms`, so it merges after T1 and T2.

Owns `CONTEXT.md` ("Early fill (E)" and "Hit window" entries), `docs/UserGuide.md`, `docs/differences-from-public-hydra.md`, `docs/clone-hero/04-fills.md` (FL-24), `docs/clone-hero/03-timing.md` (line 365), `docs/releases/release-next-notes.md`, `tests/test_docs_match_code.cpp` (the default-value table), `tests/single_owner/rules/core.json`, `tests/single_owner/rules/app.json`. The comments in `model.h` and `config.h` belong to T1 and T2.

The `CONTEXT.md` "Early fill (E)" entry becomes: "An activation timing (the `E` notation) near a fill's spawn deadline. The window is one number on both sides of the deadline: the `Early fill window` setting, 60 ms by default, from 0 to the hit window. On the orange side, Star Power comes ready after the deadline, so you must hit early to summon the fill. Later than the window (`fill_refuses`), the fill never spawns, and the search drops the activation. On the white side, Star Power comes ready before the deadline with less than the window to spare (`is_e0`). The activation still gets the `E` mark, but has nothing to time." The sentence "It is never a setting" goes. The "Hit window" entry keeps "fixed, not a setting" and drops the early fill from its list of readers.

The public pages follow `docs/agents/public-doc-style.md`. The User Guide's settings table gains an `Early fill window` row with a `default:` marker. Its early-fill paragraphs say "the `Early fill window` setting" where they say "85 ms, the hit window". The release-notes line becomes: "A new `Early fill window` setting in `Analysis settings` sets how far from a fill's deadline Star Power may come ready and still count as an early fill, on both sides. It starts at 60 ms and goes from 0 to 85 ms. Your saved results read Stale until you analyze them again."

Acceptance criteria:

- No doc says the early-fill window is fixed or is 85. Check: `hydra_tests.exe "-tc=docs match code*"` passes with the new `default:` markers in the table, so a doc number that drifts from `kDefaultEarlyFillWindowMs` fails.
- The single-owner scan agrees. Check: the scan's rows name the window argument as the owner of the early-fill compares, and still flag a bare `kHitWindowMs` compare on `e_offset`; the main session runs the scan on the joined branch.
- The public text follows the style page. Check: the reviewer reads the four changed public passages against `docs/agents/public-doc-style.md`.

## The live steps

The main session runs these, after the joined branch builds. None needs Clone Hero. Each step has at most one call never tried before, so a failed run names the one piece that failed.

### LS1: real charts, before and after

Run `hydra_replay dump --chart <file> --efill 85` and the same with `--efill 60` on each of the 98 charts in `testdata/input`, and read each run's `bestpath` and each path's `pathstring` (the notation with its `E` marks; `tools/replay_json.cpp:184` and `:199`). Keep the charts whose output differs, and show the user up to five, with the path lines before and after. Cap: one pass over the 98 charts, 10 minutes. If no chart differs, report that and name it as a gap in the corpus; do not reach for the whole library.

Calls: `hydra_replay dump --efill` (never tried)

### LS2: the panel's behaviour and text

Run the new and the two edited GUI tests. Cap: one run; a failure goes to a fresh executor.

Calls: `hydra_uitest --test settings-early-fill --test settings-reset --test settings-lock` (never tried)

### LS3: the real app starts

A panel row is a layout change, so case 2 of `docs/agents/ui-testing.md` applies. Start the real dev app on a snapshot copy of the user's library, the way "Seeing it in the real app" on that page says. No analysis is started on the copy.

Calls: `hydra_start('window')` (never tried)

### LS4: the panel opens in the real app

Click the Analysis settings button and read the panel's text for the `Early fill window` row.

Calls: `hydra_start('window')` (seen: LS3), `hydra_click` (never tried)

### LS5: the panel in the real app, captured

Capture the open panel and show the user the capture. Cap: one capture; if the panel cannot be captured, report what failed. This step settles P18.

Calls: `hydra_start('window')` (seen: LS3), `hydra_click` (seen: LS4), `hydra_screenshot` (never tried)

### LS6: the corpus app starts

Case 3 of the same page. Start the app headless on `testdata/input`.

Calls: `hydra_start('headless')` (never tried)

### LS7: a chart LS1 named opens

Open one chart LS1 named, with the box at 60.

Calls: `hydra_start('headless')` (seen: LS6), `hydra_open_song` (never tried)

### LS8: the early-fill screens on the corpus

Read the Paths tab text at 60, then set the box to 85 and read it again. Compare the `E` marks with LS1's lines for that chart.

Calls: `hydra_start('headless')` (seen: LS6), `hydra_open_song` (seen: LS7), `hydra_text` (never tried)

### LS9: the session closes

Calls: `hydra_stop` (never tried)

## Out of scope, for the user to approve

- Opening a rebuilt library file with an older Hydra build. Nothing in this plan tests it. Such a build looks results up by five lens columns, so it would not tell a result analyzed at 60 from one analyzed at 85. The column is added with no default, so an older build's save into the rebuilt file fails rather than filing a wrong row.
- Any change to the squeeze budgets, Rating words, timing tiers or `kHitWindowMs`.

## Promise map

| Promise | Plan section | Task | The check that proves it |
|---|---|---|---|
| One number sets both sides of the window | The short version | T1 | `test_model.cpp`: for several W, every `e` that `fill_refuses` also satisfies `is_e0` |
| A fill is dropped when SP came ready more than W late; an E is marked with less than W to spare | How W reaches the engine | T1 | `test_model.cpp` boundary cases at W = 60 and W = 85 |
| The main search, the all-0 pass, pinned and best-finish searches and every drawn activation use the W the run was given | How W reaches the engine | T1 | `test_fills_and_skips.cpp`: the 70 ms chart at 85 and at 60 through `analyze`, `search_target` and `paths_from_pathstring`; `e_window_ms` on main and all-0 paths |
| The default is 60, written once | Where the default lives | T1, T2, T5 | `test_model.cpp` default case; `test_config.cpp` missing-line case; docs `default:` markers |
| The range is 0 to 85; outside it lands on the nearest edge | The settings file | T2, T4, T6 | `test_config.cpp` 200 and -5; `settings-early-fill` 200 and -5; replay `--efill 900` |
| The key is `early_fill_window_ms`; `hit_window_ms` stays retired | The settings file | T2 | round-trip case; retired-keys case unedited |
| The row is named `Early fill window`, sits under `1.0 fills`, follows the one row rule, and shows the written tooltip | The panel row | T4 | `settings-early-fill`; LS5 capture |
| No new number: the box steps and is sized like Path limit's | The panel row | T4 | reviewer reads the diff: default `InputInt` step, `px(100)` |
| The button reads `Analysis settings: Early fill window 85 ms`; Reset names it | The toolbar button's words | T2, T4 | `test_config.cpp` exact text; `settings-early-fill` label read |
| Reset to defaults puts W back; a batch locks the box | The panel row | T2, T4 | reset case walks `kAnalysisFields`; `settings-reset`, `settings-lock` |
| Results are saved per W; switching back brings the old result back | How results are saved per W | T3 | `test_store_records.cpp` two-window case |
| A record never files under another window | How results are saved per W | T3 | `prepare_record` throws |
| An older file is rebuilt in place, ids kept, all or nothing | The O15 gap | T3 | `test_store_open.cpp` rebuild case; failure forced at each of the three points |
| Old rows are filed under 60 and none vanishes at the default | What an old row is filed under | T3 | rebuilt fixture read at the default lens: Stale, not absent |
| The stamp moves once, to `3.1.2+efillwindow` | The stamp moves, once | T3 | `test_store_results.cpp`: a `3.1.2+efillhitwindow` row reads Stale |
| The user is asked before today's results go Stale | Questions | question 1 | the main session's question, with LS1's lines; the stamp text in T3 follows the answer |
| `hydra_replay` takes `--efill`, like `--ms` | The tools | T6 | `settings_from` flag cases |
| At 0 no activation is an early fill and nothing breaks | The 0 ms edge | T1 | `test_model.cpp` W = 0 cases |
| Squeeze budgets, Rating words, timing tiers and `kHitWindowMs` do not change | What stays as it is | T1 | no diff in `squeeze_rating.cpp` or `report.cpp`; their tests pass unedited |
| `CONTEXT.md`, the public pages and the scans say what the code does | T5 | T5 | docs-match-code test; single-owner scan; reviewer's style read |
| Real chart examples come from `testdata/input`, never the whole library | What the user will see | LS1 | LS1's command lines and cap |
| The panel change is seen in the real app before merge | LS3 to LS5 | LS5 | the capture shown to the user |
| A new line in the batch confirm waits for the user's yes | Questions | question 2 | no diff to `batch_settings_summary` or `src/cli/batch.cpp` unless answered |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The user asked for one adjustable number on both sides | ruling | `docs/agents/rulings/code-and-engine.md` line 56 | checked 2026-11-15 |
| P2: The user asked for the panel, default 60, range 0 to 85 | ruling | `docs/agents/rulings/code-and-engine.md` line 57 | checked 2026-11-15 |
| P3: The user chose "Early fill only", so squeezes, Rating words and tiers keep 85 | ruling | the user's choice in the main session's question box, quoted in this review's brief and confirmed there; the option reads "A new early-fill setting, default 85 ms, sets both sides of the fill window. Squeeze budgets, Rating words and timing tiers keep the fixed 85 ms hit window." and the typed `default 60ms` (`docs/agents/rulings/code-and-engine.md` line 57) overrides its 85 | checked 2026-11-15 |
| P4: Both early-fill compares read `kHitWindowMs` today and take only `e_offset` | code | `src/core/model.h:318` and `src/core/model.h:322-324` | checked 2026-11-15 |
| P5: The only early-fill readers of those functions are in `model.h`, `model.cpp` and `engine.cpp` | code | grep of `src` and `tools` for the function names: `src/core/model.cpp:473-855`, `src/search/engine.cpp:854-1930` | checked 2026-11-15 |
| P6: `kHitWindowMs` has three other readers, which this plan leaves alone | code | `src/core/squeeze_rating.cpp:133`, `src/core/squeeze_rating.cpp:242`, `src/app/report.cpp:92` | checked 2026-11-15 |
| P7: Only the engine fills `Activation::e_offset` in `src/` | code | grep of `src` for `e_offset`: hits only in `src/core/model.h`, `src/core/model.cpp`, `src/search/engine.cpp`, `src/app/path_view.cpp` | checked 2026-11-15 |
| P8: The library file saves per-path figures and the path text, not activations, so a saved figure already reflects the window it was analyzed under | code | `src/store/record_store.cpp:284-310` and `src/store/record_store.cpp:588-598` | checked 2026-11-15 |
| P9: A number key clamps to its range on load, and `Settings::clamp` serves boxes and tools | code | `src/app/config.cpp:211`, `src/app/config.cpp:235-238`, `src/app/config.cpp:305-311` | checked 2026-11-15 |
| P10: `hit_window_ms` is a retired key | code | `src/app/config.h:90` | checked 2026-11-15 |
| P11: Reset and the summary walk `kAnalysisFields`; a setting with no phrase throws | code | `src/app/config.cpp:359-377`, `src/ui/library_dialogs.cpp:88-105` | checked 2026-11-15 |
| P12: The button's phrase for a number setting is name, space, value | code | `src/ui/library_dialogs.cpp:97-100` and `src/ui/library_dialogs.cpp:129-131` | checked 2026-11-15 |
| P13: Every panel row is made by `begin_row`, and Path limit's box is a default-step `InputInt` at `px(100)` | code | `src/ui/settings_bar.cpp:63-70` and `src/ui/settings_bar.cpp:185-192` | checked 2026-11-15 |
| P14: The profile's columns, key, match and bind are built from `kLensColumnList`, and an existing `profiles` table is never altered | code | `src/store/record_store.cpp:183-208`, `src/store/record_store.cpp:247-257`, `src/store/record_store.cpp:383-393`, `src/store/record_store.cpp:848-876` | checked 2026-11-15 |
| P15: A lookup's candidates are the rows with the wanted lens; a row is Ready only with the current stamp and the same rules | code | `src/store/record_store.cpp:376-379` and `src/store/record_store.cpp:445-454` | checked 2026-11-15 |
| P16: Opening an older-kind file (`LacksAlbumColumn`) shows the user nothing beyond the open itself | code | `src/store/record_store.cpp:768-770` (only `TooOld` takes the caller's policy), `src/store/record_store.cpp:795-846` (the other kinds run silent steps), `src/ui/library_jobs.cpp:179-197` (progress words exist only for `Opening` and `ConvertingRecords`) | checked 2026-11-15 |
| P17: The pinned engine digests do not move when `digest_settings` states the window as 85 | unchecked | `tests/song_digest.h:324-330` builds a default `app::Settings`; `tests/test_perf_digest.cpp` was repinned for the 85 ms E mark in `6fd37fd3`, so W = 85 runs the compares the digests were pinned under. Only a run of the digest test can show them unmoved, and agents run no build | T1 |
| P18: The method under "Seeing it in the real app" can capture the open Analysis settings panel | unchecked | `docs/agents/ui-testing.md` "Seeing it in the real app" draws one OS window with PrintWindow; the panel is an ImGui popup (`src/ui/settings_bar.cpp:289-295`) that stays inside Hydra's window while it fits, but viewports are on (`src/ui/imgui_setup.cpp:100`), so a popup past the window's edge becomes its own window and would be missing. No capture has been tried | LS5; blocks nothing |
| P19: The stamp is `3.1.2+efillhitwindow`, compared as a whole string, and one bump covers a release | code | `src/store/stored_versions.h:42-55` | checked 2026-11-15 |
| P20: `hydra_replay` starts from default settings and clamps its number flags through `Settings::clamp` | code | `tools/replay.cpp:189-234` | checked 2026-11-15 |
| P21: `testdata/store/v2.12.1_library.db` holds one `profiles` row with the five lens columns, no `album` column, and classifies `LacksAlbumColumn` | code | `testdata/store/README.md` "Facts the test pins" (one profile, `ms_enabled` 1, `legacy_fills` 0, stamp `2.8.0+efill85`); `src/store/library_file.cpp:101-103` | checked 2026-11-15 |
| P22: Some chart in `testdata/input` has an activation between 60 and 85 ms from a fill deadline | unchecked | `testdata/input`: no run of `hydra_replay dump --efill` exists yet | LS1; blocks nothing |
| P23: `hydra_replay dump` analyzes a chart and prints each path's notation (with its `E` marks) and the best path | code | `tools/replay.cpp:496-519` (`cmd_dump` runs `analyze_chart_file` under `settings_from`), `tools/replay_json.cpp:184` (`pathstring`) and `:199` (`bestpath`) | checked 2026-11-15 |
| P24: The release notes already tell users their saved results read Stale in 3.1.2 | code | `docs/releases/release-next-notes.md:3` | checked 2026-11-15 |
| P25: `CONTEXT.md` says the early-fill window is never a setting | code | `CONTEXT.md:610-620` | checked 2026-11-15 |
| P26: Every engine run in the app and tools is on a graph `search_graph` built from the settings, and two option builders have no settings in hand | code | `src/search/pather.cpp:261-265` (`search_graph`), `:69` (`allzero_options()` takes no arguments), `:289` (`run_target` builds its own `EngineOptions`), `src/search/pathstring_read.cpp:176-178`, `src/pathbuilder/pb_state.cpp:27` | checked 2026-11-15 |
| P27: `convert_results` selects `kLensColumnList` from an old file's `results` table | code | `src/store/record_store.cpp:899-906` | checked 2026-11-15 |
| P28: The stamp `3.1.2+efillhitwindow` first appears in merge `0bfa2dce` (2026-11-15), so only dev builds wrote rows under it | code | `git show 0bfa2dce^1:src/store/stored_versions.h` holds `2.14.1+alltoms`; `src/store/stored_versions.h:55` holds the new text | checked 2026-11-15 |
| P29: `kind_of` returns the first matching layout, album test last, so a file lacking both columns reads `LacksAlbumColumn` | code | `src/store/library_file.cpp:71-105` | checked 2026-11-15 |
| P30: The `--ms` and `--depth` flag cases run the built `hydra_replay` from `tests/test_cli.cpp`; `settings_from` has no test of its own | code | `tests/test_cli.cpp:635-672`; a search of `tests/` for `settings_from` finds no hit | checked 2026-11-15 |
| P31: The `settings-lock` test lives in `tests/ui/uitest_details.cpp`, `settings-reset` in `tests/ui/uitest_library.cpp` | code | `tests/ui/uitest_details.cpp:1289,1374`, `tests/ui/uitest_library.cpp:2040` | checked 2026-11-15 |
| P32: `hydra_bench` builds a `ScoreGraph` by hand from the settings and its fixed yardstick uses bare `SearchSettings` defaults | code | `tools/bench.cpp:317-319`, `tools/bench.cpp:247` | checked 2026-11-15 |
| P33: The engine takes graph-carried values from `graph.rules()` when it is built | code | `src/search/engine.cpp:2164-2166` | checked 2026-11-15 |
| P34: `testdata/input` holds 98 chart files | code | a count of `notes.chart` and `notes.mid` under `testdata/input` | checked 2026-11-15 |
