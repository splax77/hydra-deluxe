# Derivation audit ledger, 2026-10-03

This is the coverage ledger for the derivation audit in `2026-10-03-derivation-audit.md`. It has one section per source file and one row for every function in scope, with a row for each agent that read it.

The function list was built mechanically, not from memory. A script read every C++ file under `src/`, `tests/` and `tools/` (skipping `third_party/`) and listed each function body and each doctest `TEST_CASE`. Python's own parser listed every `def` in the Python tools. That gave 2688 functions. A plain grep for `TEST_CASE` found the same 592 test cases the script did.

Every one of the 2688 functions has at least one row. There are 3443 rows in all, because some functions were read by more than one agent.

How to read a row. **Read by** names the agent that read the function. The nine question-family finders are `spwin`, `backend`, `squeeze`, `fills`, `score`, `tempo`, `parse`, `store` and `display`. `screenA` and `screenB` traced the screens back to code. Agents named `sweep-*` read the functions no finder had read. **Questions** lists the question ids the function answers; the catalog at the end spells each one out. **Role** says whether the function owns the answer (`owns`), re-decides something owned elsewhere or decides it twice (`copies`), only calls the owner (`wrapper`), is a test that checks behaviour without recomputing it (`test`), or answers no audited question (`none`). **Note** names the owner when the role is `copies`.

A row records what that agent saw. It is not a proof that nothing else is there. The strongest claim this ledger supports is "none found in the ledgered functions by methods 1 to 4".

Role counts across all rows: none 1216, test 817, owns 580, copies 486, wrapper 344.

## Functions, by file

### `src/app/analysis.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1 | `list_dir` | 42–59 | parse | none | none | dir listing |
| 2 | `is_dir` | 61–64 | parse | none | none | dir test |
| 3 | `join_path` | 66–70 | parse | none | none | path join |
| 4 | `parent_of` | 72–77 | parse | none | none | path parent |
| 5 | `relpath` | 82–91 | parse | none | none | relpath |
| 6 | `Md5Provider::Md5Provider` | 103–107 | parse | none | none | md5 provider |
| 7 | `Md5Provider::~Md5Provider` | 108–110 | parse | none | none | md5 provider |
| 8 | `Md5Provider::handle` | 114–114 | parse | none | none | md5 provider |
| 9 | `stream_md5` | 126–160 | parse | parse-8 | owns | chart hash (lowercase hex) |
| 10 | `read_metadata_ini` | 168–182 | parse | parse-6;parse-5 | copies | "<unknown artist>"/"<unknown charter>" literal; empty value kept as empty; no owner |
| 11 | `parse_sng_metadata` | 198–212 | parse | parse-6;parse-24 | copies | second literal copy of artist/charter fallback; empty kept |
| 12 | `parse_srb_metadata` | 223–243 | parse | parse-6;parse-3;parse-25 | copies | third copy; empty treated as unknown (drifts from ini/sng); re-walks srb stream 1 |
| 13 | `sig_of` | 267–271 | parse | none | none | rescan sig |
| 14 | `read_song_ini_keys` | 278–318 | parse | parse-5 | owns | song.ini reader |
| 15 | `hash_chart_file` | 323–331 | parse | parse-8 | wrapper | calls stream_md5 |
| 16 | `discover_charts` | 335–483 | parse | parse-1;parse-2;parse-4;parse-7;parse-22 | owns | folder notes pick (mid over chart), ini pick (last match), title fallback via owner, no md5 dedupe |
| 17 | `discover_charts` | 487–491 | parse | none | wrapper | overload |
| 18 | `analyze_chart_file` | 497–507 | parse | parse-19 | wrapper | load + no_notes_message |
| 18 | `analyze_chart_file` | 497–507 | store | store-26 | none |  |
| 19 | `batch_worker_count` | 516–520 | parse | none | none | workers |
| 20 | `run_batch` | 539–616 | parse | parse-22;parse-13 | copies | analyzes every copy of a duplicated md5; names songmeta by whichever copy saves last (rebuild_chart_library names by first copy) |
| 20 | `run_batch` | 539–616 | screenB | screenB-32 | none |  |
| 20 | `run_batch` | 539–616 | store | store-4;store-8 | copies | cap from AnalysisSettings.sp_cap; owner Settings::cap_query |

### `src/app/config.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 21 | `exe_dir` | 16–23 | store | none | none |  |
| 22 | `set_path_overrides` | 29–29 | store | none | none |  |
| 23 | `path_overrides` | 30–30 | store | none | none |  |
| 24 | `db_path` | 32–35 | store | none | none |  |
| 25 | `ini_path` | 37–40 | store | store-16 | owns |  |
| 26 | `open_store` | 43–45 | store | none | wrapper |  |
| 27 | `asset_dir` | 47–50 | store | none | none |  |
| 28 | `Settings::load` | 52–52 | store | store-16 | wrapper |  |
| 28 | `Settings::load` | 52–52 | tempo | tempo-24 | owns | hit_window_ms parsed with atoi (int) |
| 29 | `Settings::load_file` | 54–101 | backend | backend-11 | none | reads backendlimit_value without clamp |
| 29 | `Settings::load_file` | 54–101 | fills | fills-25;fills-26 | none | reads depth_mode/depth_value with atoi, no range check |
| 29 | `Settings::load_file` | 54–101 | spwin | spwin-8 | copies | v >= 1 (partial read) |
| 29 | `Settings::load_file` | 54–101 | store | store-16 | owns | reader; validation ranges |
| 30 | `Settings::save` | 103–103 | store | store-16 | wrapper |  |
| 31 | `Settings::save_file` | 105–127 | store | store-16 | copies | key list typed again; owner load_file |
| 32 | `Settings::difficulty` | 129–131 | parse | parse-19 | wrapper | difficulty from name |
| 32 | `Settings::difficulty` | 129–131 | screenA | screenA-34 | owns |  |
| 32 | `Settings::difficulty` | 129–131 | store | store-11 | owns |  |
| 33 | `Settings::effective_bass2x` | 133–135 | parse | parse-13 | owns | 2x bass is Expert-only |
| 33 | `Settings::effective_bass2x` | 133–135 | screenA | screenA-34 | owns |  |
| 33 | `Settings::effective_bass2x` | 133–135 | store | store-11 | owns |  |
| 34 | `Settings::chartmode_key` | 137–143 | parse | parse-13;parse-19 | none | chartmode key uses effective_bass2x |
| 34 | `Settings::chartmode_key` | 137–143 | store | store-11 | owns |  |
| 35 | `Settings::to_analysis_settings` | 145–157 | fills | fills-25;fills-9 | copies | depth_mode==1 -> Points; one of four readers of the int |
| 35 | `Settings::to_analysis_settings` | 145–157 | parse | parse-13 | wrapper | bass2x = effective_bass2x |
| 35 | `Settings::to_analysis_settings` | 145–157 | screenB | screenB-32 | owns |  |
| 35 | `Settings::to_analysis_settings` | 145–157 | store | store-9;store-26 | owns | depth_mode 1->Points else Scores |
| 36 | `Settings::backend_limit` | 159–163 | backend | backend-11 | owns | abs(backendlimit_value) |
| 36 | `Settings::backend_limit` | 159–163 | screenA | screenA-29 | copies | abs, no clamp; UI clamps 0..500 |
| 36 | `Settings::backend_limit` | 159–163 | store | none | none | backend_limit abs |
| 37 | `Settings::cap_query` | 165–167 | screenB | none | none |  |
| 37 | `Settings::cap_query` | 165–167 | store | store-8 | owns |  |
| 38 | `Settings::lens` | 169–173 | fills | fills-10 | wrapper | Lens::from with legacy_fills |
| 38 | `Settings::lens` | 169–173 | screenB | screenB-21 | owns |  |
| 38 | `Settings::lens` | 169–173 | store | store-9 | owns | depth_mode raw into key |
| 39 | `Settings::record_key` | 175–177 | store | store-7 | wrapper |  |
| 40 | `Settings::batch_run` | 179–181 | screenB | screenB-32 | wrapper |  |
| 40 | `Settings::batch_run` | 179–181 | store | store-7;store-9 | wrapper | BatchRun has no cap |

### `src/app/display_format.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 41 | `py_round3` | 8–15 | display | display-3 | owns | python-style round to 3 places |
| 41 | `py_round3` | 8–15 | score | score-13 | owns | py_round3 |
| 41 | `py_round3` | 8–15 | screenA | screenA-28 | owns |  |
| 41 | `py_round3` | 8–15 | screenB | screenB-28 | owns |  |
| 42 | `format_avg_mult` | 17–21 | display | display-3 | owns | avg multiplier text |
| 42 | `format_avg_mult` | 17–21 | score | score-13 | wrapper | format_avg_mult |
| 42 | `format_avg_mult` | 17–21 | screenA | screenA-28 | owns |  |
| 42 | `format_avg_mult` | 17–21 | screenB | screenB-28 | wrapper |  |
| 43 | `format_ms` | 23–27 | display | display-1 | owns | "%.1fms" form |
| 43 | `format_ms` | 23–27 | screenA | screenA-14 | owns |  |
| 43 | `format_ms` | 23–27 | screenB | none | none |  |
| 43 | `format_ms` | 23–27 | tempo | none | none | ms text |
| 44 | `format_ms_spaced` | 29–33 | display | display-1 | owns | "%.1f ms" form; second owner of the same question as format_ms (two forms) |
| 44 | `format_ms_spaced` | 29–33 | screenA | screenA-4;screenA-15 | owns |  |
| 44 | `format_ms_spaced` | 29–33 | screenB | none | none |  |

### `src/app/dm_report.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 45 | `page_template` | 141–144 | display | none | none | page shell |
| 45 | `page_template` | 141–144 | screenB | none | none |  |
| 46 | `collect_dm_rows` | 152–211 | display | display-14;display-21 | owns | status and pct; pct then re-rounded twice (%.4f in build_dm_html, toFixed(2) on page) |
| 46 | `collect_dm_rows` | 152–211 | fills | fills-32 | copies | forces cap 4 but not the 1.1 fill rule |
| 46 | `collect_dm_rows` | 152–211 | parse | parse-8 | copies | lowercases library md5 for matching (stream_md5 already lowercase) |
| 46 | `collect_dm_rows` | 152–211 | score | score-22 | owns | collect_dm_rows delta/status/pct |
| 46 | `collect_dm_rows` | 152–211 | screenB | screenB-20;screenB-21 | owns | forces cap 4, keeps settings lens |
| 46 | `collect_dm_rows` | 152–211 | store | store-8 | copies | always kCloneHeroSpCap |
| 47 | `build_dm_html` | 214–252 | display | display-14 | copies | %.4f pre-rounding of pct before the page's toFixed(2) (double rounding) |
| 47 | `build_dm_html` | 214–252 | score | score-22 | copies | build_dm_html page JS: delta<0 -> over re-decides above-optimal; stats recount statuses and points left |
| 47 | `build_dm_html` | 214–252 | screenB | screenB-20;screenB-21 | copies | page JS 'over' from r.delta < 0; prose 'SP cap 4' |
| 48 | `tally_dm_rows` | 254–264 | display | display-21 | wrapper | tallies status strings |
| 48 | `tally_dm_rows` | 254–264 | score | score-22 | copies | tally_dm_rows counts statuses (JS stats counts again) |
| 48 | `tally_dm_rows` | 254–264 | screenB | screenB-20 | wrapper |  |
| 49 | `generate_dm_report` | 270–290 | display | display-19 | copies | footer literal "SP cap 4" while collect_dm_rows uses kCloneHeroSpCap |
| 49 | `generate_dm_report` | 270–290 | score | score-22 | wrapper | generate_dm_report |
| 49 | `generate_dm_report` | 270–290 | screenB | screenB-21 | copies | footer prose 'at SP cap 4' |

### `src/app/dynamics_breakdown.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 50 | `DynamicsBreakdown::row` | 13–15 | display | none | none | accessor |
| 50 | `DynamicsBreakdown::row` | 13–15 | parse | none | none | row accessor |
| 50 | `DynamicsBreakdown::row` | 13–15 | screenA | none | none |  |
| 51 | `DynamicsBreakdown::pads_total` | 17–25 | display | display-23 | owns | pad total |
| 51 | `DynamicsBreakdown::pads_total` | 17–25 | parse | none | none | pad total |
| 51 | `DynamicsBreakdown::pads_total` | 17–25 | screenA | screenA-36 | owns |  |
| 52 | `DynamicsBreakdown::kicks_total` | 27–35 | display | display-23 | owns | all kicks incl. 2x always |
| 52 | `DynamicsBreakdown::kicks_total` | 27–35 | parse | parse-13 | none | kicks_total always includes Kick2x row |
| 52 | `DynamicsBreakdown::kicks_total` | 27–35 | screenA | screenA-36 | owns |  |
| 53 | `DynamicsBreakdown::played_total` | 37–50 | display | display-23 | owns | played total (2x only when bass2x) |
| 53 | `DynamicsBreakdown::played_total` | 37–50 | parse | parse-13 | copies | re-decides whether 2x kicks are played (parser already drops them when bass2x off) |
| 53 | `DynamicsBreakdown::played_total` | 37–50 | screenA | screenA-34;screenA-36 | copies | re-decides whether 2x kicks are played (parser decides with mode_bass2x_) |
| 54 | `dynamics_row_label` | 54–77 | display | none | owns | row labels |
| 54 | `dynamics_row_label` | 54–77 | parse | parse-12 | none | labels (pro wording) |
| 54 | `dynamics_row_label` | 54–77 | screenA | none | none |  |
| 55 | `row_for` | 83–100 | display | none | none | binning |
| 55 | `row_for` | 83–100 | parse | parse-11;parse-12;parse-13 | none | reads is2x/is_cymbal from the parsed note |
| 55 | `row_for` | 83–100 | screenA | screenA-35 | owns |  |
| 56 | `count_dynamics` | 104–119 | display | none | none | counting |
| 56 | `count_dynamics` | 104–119 | parse | parse-16 | none | counts parsed dynamics |
| 56 | `count_dynamics` | 104–119 | screenA | screenA-35 | owns |  |
| 57 | `write_u32_le` | 129–134 | display | none | none | codec |
| 57 | `write_u32_le` | 129–134 | parse | none | none | byte helper |
| 57 | `write_u32_le` | 129–134 | store | store-6 | copies | LE write; owner BinaryWriter |
| 58 | `read_u32_le` | 136–141 | display | none | none | codec |
| 58 | `read_u32_le` | 136–141 | parse | none | none | byte helper |
| 58 | `read_u32_le` | 136–141 | store | store-6 | copies | LE read; owner BinaryReader |
| 59 | `encode_dynamics` | 145–156 | display | none | none | codec |
| 59 | `encode_dynamics` | 145–156 | parse | none | none | blob |
| 59 | `encode_dynamics` | 145–156 | screenA | none | none |  |
| 59 | `encode_dynamics` | 145–156 | store | store-20 | owns | dynamics blob writer |
| 60 | `decode_dynamics` | 158–171 | display | none | none | codec |
| 60 | `decode_dynamics` | 158–171 | parse | none | none | blob |
| 60 | `decode_dynamics` | 158–171 | screenA | none | none |  |
| 60 | `decode_dynamics` | 158–171 | store | store-20 | owns | blob stamp check |
| 61 | `dynamics_cache_key` | 175–177 | display | none | none | key |
| 61 | `dynamics_cache_key` | 175–177 | parse | parse-23 | copies | path-keyed identity for the in-memory dynamics copy; store uses md5 |
| 61 | `dynamics_cache_key` | 175–177 | screenA | none | none |  |
| 61 | `dynamics_cache_key` | 175–177 | store | none | none | in-memory job key |
| 62 | `dynamics_store_key` | 179–181 | display | none | none | key |
| 62 | `dynamics_store_key` | 179–181 | parse | parse-23 | owns | md5-keyed dynamics identity |
| 62 | `dynamics_store_key` | 179–181 | screenA | none | none |  |
| 62 | `dynamics_store_key` | 179–181 | store | store-20 | owns |  |
| 63 | `load_stored_dynamics` | 184–188 | display | none | none | store |
| 63 | `load_stored_dynamics` | 184–188 | parse | none | wrapper | store read |
| 63 | `load_stored_dynamics` | 184–188 | screenA | none | none |  |
| 63 | `load_stored_dynamics` | 184–188 | store | store-20 | wrapper |  |
| 64 | `save_dynamics` | 191–193 | display | none | none | store |
| 64 | `save_dynamics` | 191–193 | parse | none | wrapper | store write |
| 64 | `save_dynamics` | 191–193 | screenA | none | none |  |
| 64 | `save_dynamics` | 191–193 | store | store-20 | wrapper |  |
| 65 | `dynamics_entry_from_analysis` | 196–206 | display | none | none | store |
| 65 | `dynamics_entry_from_analysis` | 196–206 | parse | parse-13 | owns | refuses to store unless the parse kept 2x kicks |
| 65 | `dynamics_entry_from_analysis` | 196–206 | screenA | screenA-34 | owns | gates on bass2x (Expert only) |
| 65 | `dynamics_entry_from_analysis` | 196–206 | store | store-20 | none |  |

### `src/app/dynamics_breakdown.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 66 | `DynamicsCounts::all` | 26–26 | sweep-core1 | screenA-36 | owns | row total ghost+accent+normal |
| 67 | `DynamicsCounts::has_dynamics` | 27–27 | sweep-core1 | sweep-core1-1;screenA-36 | owns | ghost+accent>0; render_dynamics_panel re-sums played.ghost + played.accent for 'Dynamic notes' |

### `src/app/fill_report.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 68 | `page_template` | 135–138 | display | none | none | page shell |
| 68 | `page_template` | 135–138 | screenB | none | none |  |
| 69 | `collect_fill_rows` | 146–207 | display | display-21 | owns | fill status; both-sides-no-score falls to "only 1.1" |
| 69 | `collect_fill_rows` | 146–207 | fills | fills-10 | none | forces legacy_fills 1/0 per side |
| 69 | `collect_fill_rows` | 146–207 | score | score-23 | owns | collect_fill_rows delta/status |
| 69 | `collect_fill_rows` | 146–207 | screenB | screenB-19 | owns |  |
| 69 | `collect_fill_rows` | 146–207 | store | store-10 | copies | old=1.0 new=1.1 hard-coded |
| 70 | `tally_fill_rows` | 209–220 | display | display-21 | wrapper | tallies status strings (else-branch counts any unknown status as only_new) |
| 70 | `tally_fill_rows` | 209–220 | fills | none | none | status tally |
| 70 | `tally_fill_rows` | 209–220 | score | score-23 | none | tally_fill_rows |
| 70 | `tally_fill_rows` | 209–220 | screenB | screenB-19 | wrapper |  |
| 71 | `build_fill_html` | 224–268 | display | display-21 | copies | page JS colours delta by sign rather than reading status |
| 71 | `build_fill_html` | 224–268 | fills | none | none | page payload |
| 71 | `build_fill_html` | 224–268 | screenB | screenB-19 | copies | page JS colours delta by sign, same question as status |
| 72 | `generate_fill_report` | 274–297 | display | display-18 | copies | footer restates fill rule in prose (owner graph.h FillDeadlineRule) |
| 72 | `generate_fill_report` | 274–297 | fills | fills-34 | none | footer prose restates the rule |
| 72 | `generate_fill_report` | 274–297 | screenB | screenB-19 | wrapper |  |

### `src/app/html_page.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 73 | `replace_all` | 9–16 | display | none | none | string helper |
| 74 | `html_escape` | 18–32 | display | none | none | escape |
| 75 | `json_escape_into` | 34–93 | display | none | none | escape |
| 76 | `render_page` | 96–104 | display | none | none | template fill |
| 77 | `page_template` | 492–505 | display | display-1 | copies | shared page JS fmtMs = toFixed(1) (JS rounding ties up, C printf ties to even) |

### `src/app/library_query.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 78 | `byte_at` | 27–29 | sweep-core1 | none | none | byte helper |
| 79 | `is_ascii_space` | 31–33 | sweep-core1 | sweep-core1-3 | copies | core::trim kSpace (same six chars) |
| 80 | `is_ascii_alpha` | 35–37 | sweep-core1 | none | none | tag-name letter test |
| 81 | `is_continuation` | 39–41 | sweep-core1 | none | none | UTF-8 continuation test |
| 82 | `ascii_lower` | 43–45 | sweep-core1 | sweep-core1-3 | copies | core strutil lower_ascii/to_lower_ascii |
| 83 | `iequals_ascii` | 47–54 | sweep-core1 | sweep-core1-3 | copies | strutil (to_lower_ascii / ends_with_ci loop); plan 2026-09-26 Task 16 says every copy goes |
| 84 | `starts_with_ci` | 56–58 | sweep-core1 | sweep-core1-3 | copies | strutil ends_with_ci (prefix twin) |
| 85 | `fold_into` | 103–178 | sweep-core1 | sweep-core1-10 | owns | library search folding (accents, full-width, whitespace runs, ASCII lowercase) |
| 86 | `rich_tag_length` | 194–216 | display | display-9 | owns | rich tag recogniser (8 tags) |
| 87 | `field_named` | 220–226 | sweep-core1 | none | none | query field keywords |
| 88 | `read_quoted` | 230–236 | sweep-core1 | none | none | quoted run reader |
| 89 | `add_term` | 238–244 | sweep-core1 | none | none | trims folded term edges |
| 90 | `add_error` | 246–249 | sweep-core1 | none | none | dedupes error strings |
| 91 | `parse_stars` | 251–261 | score | score-16 | copies | parse_stars bounds by kMaxStars but kStarsError text hard-codes 0 to 7 |
| 92 | `parse_squeeze` | 263–273 | display | display-31 | none | parse squeeze value |
| 92 | `parse_squeeze` | 263–273 | squeeze | squeeze-28 | none | parse limit |
| 93 | `contains` | 277–279 | sweep-core1 | none | none | substring test |
| 94 | `term_matches` | 281–292 | sweep-core1 | sweep-core1-5 | owns | which row fields a term is tested against |
| 95 | `fold_for_search` | 296–300 | sweep-core1 | sweep-core1-10 | wrapper | fold_into |
| 96 | `strip_rich_tags` | 302–317 | display | display-9 | owns | strip_rich_tags |
| 97 | `LibraryQuery::empty` | 319–321 | sweep-core1 | none | none | query has any filter |
| 98 | `parse_library_query` | 323–371 | display | display-31 | copies | squeeze< and squeeze<= parse to the same bound |
| 99 | `make_searchable` | 374–381 | display | display-9 | wrapper | strip_rich_tags |
| 100 | `query_matches` | 383–394 | display | display-31 | owns | hardest > max excludes (so <= semantics) |
| 100 | `query_matches` | 383–394 | squeeze | squeeze-28 | copies | hardest > limit on stored column (owner Engine::passes_ms_filter form) |
| 101 | `match_spans` | 397–425 | sweep-core1 | sweep-core1-5 | copies | re-decides term/field applicability (owner term_matches) |

### `src/app/path_view.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 102 | `bars_text` | 16–18 | display | display-10 | copies | singular/plural bar text; parallel to report::counted and charts_text |
| 102 | `bars_text` | 16–18 | screenA | screenA-9;screenA-10 | owns |  |
| 103 | `format_measure` | 25–31 | display | none | owns | measure text m<measure>.<beat>.<tick> |
| 103 | `format_measure` | 25–31 | screenA | screenA-10 | owns |  |
| 103 | `format_measure` | 25–31 | screenB | none | owns | format_measure |
| 103 | `format_measure` | 25–31 | tempo | tempo-6 | wrapper | formats Timecode mbt |
| 104 | `format_measure` | 33–35 | display | none | wrapper | calls format_measure(Timecode) |
| 104 | `format_measure` | 33–35 | screenA | screenA-10 | wrapper |  |
| 104 | `format_measure` | 33–35 | screenB | none | wrapper |  |
| 104 | `format_measure` | 33–35 | tempo | tempo-6 | wrapper | format via SongTiming::timecode |
| 105 | `activation_badge` | 37–55 | display | display-1;display-6 | copies | re-picks which squeeze is hardest by float equality (Activation::difficulty owns the value); %.0f whole-ms form differs from format_ms_spaced and notationstr_verbose truncation |
| 105 | `activation_badge` | 37–55 | fills | fills-38;fills-13 | wrapper | uses is_e_critical / e_difficulty |
| 105 | `activation_badge` | 37–55 | screenA | screenA-11;screenA-14 | copies | re-finds which item produced Activation::difficulty; adds optional early fill; rounds %.0f while notationstr_verbose truncates |
| 105 | `activation_badge` | 37–55 | screenB | screenB-16 | wrapper |  |
| 105 | `activation_badge` | 37–55 | squeeze | squeeze-29;squeeze-5 | copies | adds optional-fill fallback to activation hardest (owner Activation::difficulty) |
| 106 | `squeeze_sentences` | 60–111 | backend | backend-2;backend-4;backend-5;backend-12;backend-13 | copies | lost = row.points - value re-derives sqout_reduction (owner CategoryScores::sqout_reduction); counted gate copy |
| 106 | `squeeze_sentences` | 60–111 | display | display-1;display-13 | copies | squeeze-out cost via lost>0 while build_activations row uses counted; owner core::backend_row_value; ms via format_ms_spaced (wrapper part) |
| 106 | `squeeze_sentences` | 60–111 | screenA | screenA-15;screenA-16;screenA-17 | copies | cost computed again in build_activations; direction test parallels rate_activation |
| 106 | `squeeze_sentences` | 60–111 | spwin | spwin-15;spwin-16 | copies | hard-codes SqOutPosition::Exact for the squeezed-out row; SqOut sentence by kind |
| 106 | `squeeze_sentences` | 60–111 | squeeze | squeeze-10;squeeze-24;squeeze-32 | copies | earned/free by timing sign (owner none; rate_activation also); squeeze-out cost gate separate from table |
| 107 | `build_record_status` | 119–143 | display | display-10;display-11;display-30 | copies | "No paths found." duplicates render_record_state/render_headline; "SP cap: N bars" never singular; Paths kept = all_paths().size() vs summarize_record pathcount |
| 107 | `build_record_status` | 119–143 | score | score-10 | wrapper | build_record_status prints best totalscore |
| 107 | `build_record_status` | 119–143 | screenA | screenA-38 | copies | No paths found also in render_record_state/render_headline |
| 107 | `build_record_status` | 119–143 | store | store-12 | copies | reads ms_limit/sp_cap blob copies; owner results columns/key |
| 108 | `build_multsqueezes` | 145–157 | display | none | wrapper | reads MultSqueeze points/howto/rowstr |
| 108 | `build_multsqueezes` | 145–157 | score | score-18 | wrapper | build_multsqueezes shows MultSqueeze::points |
| 108 | `build_multsqueezes` | 145–157 | screenA | screenA-27 | wrapper |  |
| 109 | `multsqueeze_summary` | 159–165 | display | display-10 | none | group_thousands total |
| 109 | `multsqueeze_summary` | 159–165 | score | score-18 | none | multsqueeze_summary sums points for display |
| 109 | `multsqueeze_summary` | 159–165 | screenA | screenA-27 | owns |  |
| 110 | `build_activations` | 188–367 | backend | backend-2;backend-3;backend-4;backend-5;backend-11;backend-12;backend-13 | copies | maps squeezed_out to Exact/NoSqOut by hand (owner sqout_position); (-N)=points-value re-derives sqout_reduction; backend limit filter; value_or(0.0) |
| 110 | `build_activations` | 188–367 | display | display-1;display-4;display-12;display-13;display-17 | copies | 0.005 print threshold beside rate_activation; song_fraction uses last-note length (Preview uses transport length); backend row timing %.1f; squeezed-out (-N)+warn whenever counted (squeeze_sentences t |
| 110 | `build_activations` | 188–367 | fills | fills-38;fills-13 | wrapper | uses is_e_critical / is_E0 |
| 110 | `build_activations` | 188–367 | score | score-5;score-7 | copies | build_activations: picks SqOutPosition from br.squeezed_out (owner sqout_position) and prints bsq.points - value as the squeeze-out cost (owner sqout_reduction) |
| 110 | `build_activations` | 188–367 | screenA | screenA-9;screenA-13;screenA-14;screenA-16;screenA-17;screenA-18;screenA-20;screenA-22;screenA-23;screenA-24;screenA-25;screenA-26 | copies | owns 0.005 scale gate (vs transfer_is_material); picks SqOutPosition itself (vs sqout_position); recomputes squeeze-out cost (vs squeeze_sentences) |
| 110 | `build_activations` | 188–367 | spwin | spwin-15;spwin-6 | copies | SqOutPosition from br.squeezed_out instead of sqout_position; reads clamp_tick |
| 110 | `build_activations` | 188–367 | squeeze | squeeze-16;squeeze-18;squeeze-19;squeeze-21;squeeze-24;squeeze-30;squeeze-15 | copies | shows() 0.005 regate of material warns; (-N) gating differs from sentence; picks SqOutPosition from flag (owner sqout_position) |
| 110 | `build_activations` | 188–367 | tempo | tempo-2;tempo-12;tempo-13;tempo-14 | copies | inline llround(tick_at_ms(song_length)) for last measure; song_fraction = ms / last-note ms; rounding duplicates preview tick_at |
| 111 | `build_score_breakdown` | 369–389 | display | display-3 | wrapper | calls format_avg_mult |
| 111 | `build_score_breakdown` | 369–389 | score | score-10;score-13 | wrapper | build_score_breakdown prints stored fields |
| 111 | `build_score_breakdown` | 369–389 | screenA | screenA-28 | wrapper |  |
| 112 | `build_path_list` | 391–434 | display | display-32 | copies | all-0 delta vs flat.front(); build_path_buttons uses best_path() |
| 112 | `build_path_list` | 391–434 | score | score-19;score-20;score-27 | copies | build_path_list: all-0 delta vs optimal (also in build_path_buttons); assumes traversal score order |
| 112 | `build_path_list` | 391–434 | screenA | screenA-3;screenA-6;screenA-7 | copies | allzero_label delta duplicates build_path_buttons detail |
| 112 | `build_path_list` | 391–434 | screenB | screenB-18 | copies | first score group = optimal; owner best_path |
| 113 | `within_label` | 436–442 | display | display-10 | copies | inline pluralization |
| 113 | `within_label` | 436–442 | fills | fills-25 | copies | depth_mode==1 -> points label |
| 113 | `within_label` | 436–442 | score | score-21 | none | within_label text |
| 113 | `within_label` | 436–442 | screenA | screenA-8 | owns |  |
| 113 | `within_label` | 436–442 | screenB | none | none |  |
| 113 | `within_label` | 436–442 | store | store-9 | copies | depth_mode 1->points else scores; owner to_analysis_settings |
| 114 | `build_path_buttons` | 444–474 | display | display-1;display-4;display-32 | copies | timing via format_ms_spaced and Path::is_difficult (wrappers); all-0 delta re-derived vs best_path() |
| 114 | `build_path_buttons` | 444–474 | fills | fills-25 | wrapper | calls within_label |
| 114 | `build_path_buttons` | 444–474 | score | score-19;score-20 | copies | build_path_buttons: all-0 delta vs best (also in build_path_list) |
| 114 | `build_path_buttons` | 444–474 | screenA | screenA-3;screenA-4;screenA-5;screenA-6 | owns | optimal = group 0; best = best_path() |
| 114 | `build_path_buttons` | 444–474 | screenB | screenB-18;screenB-15;screenB-16 | copies | every path tied with the first is '(optimal)'; report keeps one rank-1 |
| 114 | `build_path_buttons` | 444–474 | squeeze | squeeze-6;squeeze-7 | wrapper | Path::difficulty / is_difficult |
| 115 | `PathsTabUi::reset` | 476–480 | display | none | none | fold state |
| 115 | `PathsTabUi::reset` | 476–480 | screenA | none | none |  |
| 116 | `PathsTabUi::all_open` | 482–487 | display | none | none | fold state |
| 116 | `PathsTabUi::all_open` | 482–487 | screenA | none | none |  |
| 117 | `PathsTabUi::set_all` | 489–491 | display | none | none | fold state |
| 117 | `PathsTabUi::set_all` | 489–491 | screenA | none | none |  |
| 118 | `PathsTabUi::click_row` | 493–501 | display | none | none | fold state |
| 118 | `PathsTabUi::click_row` | 493–501 | screenA | none | none |  |
| 119 | `PathsTabCache::status` | 504–511 | display | none | wrapper | cache over build_record_status |
| 119 | `PathsTabCache::status` | 504–511 | screenA | screenA-38 | wrapper |  |
| 120 | `PathsTabCache::details` | 516–535 | display | none | wrapper | cache over build_activations |
| 120 | `PathsTabCache::details` | 516–535 | screenA | none | wrapper |  |
| 121 | `PathsTabCache::buttons` | 538–548 | display | none | wrapper | cache over build_path_buttons |
| 121 | `PathsTabCache::buttons` | 538–548 | screenA | none | wrapper |  |

### `src/app/path_view.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 122 | `PathsTabCache::ui` | 267–267 | sweep-core1 | none | none | accessor |
| 123 | `PathsTabCache::ui` | 268–268 | sweep-core1 | none | none | accessor |
| 124 | `PathsTabCache::status_builds` | 271–271 | sweep-core1 | none | none | test counter |
| 125 | `PathsTabCache::details_builds` | 272–272 | sweep-core1 | none | none | test counter |
| 126 | `PathsTabCache::buttons_builds` | 273–273 | sweep-core1 | none | none | test counter |

### `src/app/preview_clock.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 127 | `PreviewClock::steady_now` | 24–27 | sweep-core1 | none | none | monotonic seconds |
| 128 | `PreviewClock::PreviewClock` | 29–29 | sweep-core1 | none | none | ctor |
| 129 | `PreviewClock::play` | 31–35 | sweep-core1 | none | none | clock start |
| 130 | `PreviewClock::pause` | 36–40 | sweep-core1 | tempo-28 | none | freezes now_ms |
| 131 | `PreviewClock::toggle` | 41–41 | sweep-core1 | none | none | toggle |
| 132 | `PreviewClock::seek_ms` | 44–47 | tempo | none | none | clock seek |
| 133 | `PreviewClock::now_ms` | 49–51 | tempo | tempo-28 | none | seconds to ms for clock |
| 134 | `PreviewClock::playing` | 52–52 | sweep-core1 | none | none | accessor |

### `src/app/preview_source.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 135 | `base_name` | 30–33 | parse | none | none | path helper |
| 136 | `dir_name` | 36–39 | parse | none | none | path helper |
| 137 | `stem_of` | 42–46 | parse | none | none | path helper |
| 138 | `read_u64` | 48–53 | parse | none | none | byte helper |
| 139 | `srb_decrypt_blob` | 60–118 | parse | none | none | srb audio decrypt |
| 140 | `find_song_ini` | 122–138 | parse | parse-4 | copies | first matching song.ini; discover_charts takes last match |
| 141 | `parse_delay_ms` | 142–151 | parse | none | none | delay text |
| 141 | `parse_delay_ms` | 142–151 | tempo | tempo-11 | owns | delay text to ms |
| 142 | `sng_audio_from` | 154–166 | parse | parse-24 | wrapper | uses sng owner |
| 143 | `is_audio_filename` | 175–179 | parse | none | none | audio names |
| 144 | `looks_like_audio` | 181–192 | parse | none | none | audio magic |
| 145 | `find_loose_audio` | 194–218 | parse | none | none | loose audio |
| 146 | `extract_sng_audio` | 220–222 | parse | none | none | wrapper |
| 147 | `sng_delay_ms` | 224–229 | parse | parse-27 | owns | sng delay |
| 147 | `sng_delay_ms` | 224–229 | screenB | none | none |  |
| 147 | `sng_delay_ms` | 224–229 | tempo | tempo-11 | wrapper | sng delay key |
| 148 | `extract_srb_audio` | 231–312 | parse | parse-3;parse-25 | copies | re-walks srb stream chain (stream 1 at header, stream 2 after) like load_songpath_srb |
| 149 | `read_ini_delay_ms` | 314–324 | parse | parse-27;parse-5 | wrapper | uses read_song_ini_keys |
| 149 | `read_ini_delay_ms` | 314–324 | tempo | tempo-11 | wrapper | song.ini delay key |
| 150 | `preview_audio_offset_ms` | 331–335 | parse | parse-27 | owns | delay vs Offset |
| 150 | `preview_audio_offset_ms` | 331–335 | screenB | screenB-31 | owns |  |
| 150 | `preview_audio_offset_ms` | 331–335 | tempo | tempo-11 | owns | delay vs Offset precedence and sign |
| 151 | `resolve_preview_source` | 339–363 | parse | parse-1 | copies | dispatches on ends_with_ci(".sng"/".srb") instead of chart_format_of |
| 151 | `resolve_preview_source` | 339–363 | tempo | tempo-11 | wrapper | picks delay source per format |

### `src/app/preview_source.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 152 | `PreviewAudioStem::from_file` | 42–42 | sweep-core1 | none | none | loose vs container stem |

### `src/app/preview_view.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 153 | `lane_of` | 18–27 | display | none | none | lane map |
| 153 | `lane_of` | 18–27 | parse | parse-10 | none | lane map NoteColor->PreviewLane |
| 153 | `lane_of` | 18–27 | screenB | none | none |  |
| 154 | `note_from` | 31–43 | display | none | none | note map |
| 154 | `note_from` | 31–43 | parse | parse-11;parse-16 | none | copies parsed flags into PreviewNote |
| 154 | `note_from` | 31–43 | screenB | none | none |  |
| 154 | `note_from` | 31–43 | tempo | tempo-1;tempo-6 | wrapper | reads Timecode ms/measure |
| 155 | `span_from_ticks` | 45–52 | display | display-35 | none | tick to ms via song.timecode |
| 155 | `span_from_ticks` | 45–52 | screenB | none | none |  |
| 155 | `span_from_ticks` | 45–52 | spwin | none | none | span helper |
| 155 | `span_from_ticks` | 45–52 | tempo | tempo-1 | wrapper | span ms from Timecode |
| 156 | `push_segment` | 64–67 | screenB | screenB-1 | none | segment helper |
| 156 | `push_segment` | 64–67 | spwin | spwin-20 | none | segment push |
| 157 | `build_sp_meter_curve` | 73–200 | display | display-24;display-26 | copies | re-derives SP banking between activations (+1 per phrase, cap) and the drain; engine owns SP meter |
| 157 | `build_sp_meter_curve` | 73–200 | screenB | screenB-1;screenB-2;screenB-3;screenB-12;screenB-30 | copies | re-walks SP banking/drain owned by search/graph.cpp (max_sp_bars, extend_deacts, store_new_backend); snaps to stored sp_meter and deact |
| 157 | `build_sp_meter_curve` | 73–200 | spwin | spwin-1;spwin-3;spwin-5;spwin-6;spwin-8;spwin-11;spwin-17;spwin-18;spwin-20;spwin-23 | copies | own bank count (owner Engine::advance), own drain + cap clamp (owner extend_deacts), squeezed-out count as complement (owner create_deactivated_path / sqout_tick), cap<1?1 |
| 157 | `build_sp_meter_curve` | 73–200 | squeeze | squeeze-23 | copies | squeezed-out phrases by set difference (owner engine sqout_tick) |
| 157 | `build_sp_meter_curve` | 73–200 | tempo | tempo-6;tempo-7;tempo-22;tempo-23 | copies | re-derives SP drain over measures with measures_at_tick_f (engine uses plusmeasure); banks phrase at end_ms |
| 158 | `build_score` | 204–220 | backend | backend-6 | wrapper | reads replay multiplier_shown |
| 158 | `build_score` | 204–220 | display | none | wrapper | replay_stored_path |
| 158 | `build_score` | 204–220 | score | score-24 | wrapper | build_score reads replay steps |
| 158 | `build_score` | 204–220 | screenB | screenB-9 | wrapper | calls core/replay replay_stored_path |
| 158 | `build_score` | 204–220 | spwin | spwin-11 | wrapper | replay_stored_path |
| 159 | `build_preview_scene` | 225–360 | display | display-25;display-35 | copies | taken fill by end_tick == a.tick (also track_state); song_length_ms = last note ms (also store::song_length_ms); activation ms via song.timecode but sp_end_ms via ms_index |
| 159 | `build_preview_scene` | 225–360 | fills | fills-7;fills-20 | copies | derives offered/taken/hidden from skip count by nearest-before rule |
| 159 | `build_preview_scene` | 225–360 | parse | parse-21 | copies | scene.song_length_ms = last note ms (store::song_length_ms answers the same) |
| 159 | `build_preview_scene` | 225–360 | score | score-9 | copies | build_preview_scene coalesces flag_solo runs (same question as replay_path last_of_run; no owner) |
| 159 | `build_preview_scene` | 225–360 | screenB | screenB-6;screenB-10 | copies | fill offered/hidden re-decided from skips; owner engine.cpp Engine::branch_activate |
| 159 | `build_preview_scene` | 225–360 | spwin | spwin-10;spwin-9 | wrapper | reads deact_tick via activation_deact_tick; default sp_cap param |
| 159 | `build_preview_scene` | 225–360 | squeeze | none | wrapper | activation_deact_tick |
| 159 | `build_preview_scene` | 225–360 | store | store-17 | copies | song length = last scene note; owner store::song_length_ms |
| 159 | `build_preview_scene` | 225–360 | tempo | tempo-1;tempo-4;tempo-12 | copies | song_length_ms = notes.back().ms (owner store::song_length_ms); tempos/meters/sections ms via MsIndex::at |
| 160 | `sp_meter_bars_at` | 362–375 | display | none | none | curve read |
| 160 | `sp_meter_bars_at` | 362–375 | screenB | screenB-1 | wrapper | reads the curve |
| 160 | `sp_meter_bars_at` | 362–375 | spwin | spwin-20 | none | curve read-back |
| 160 | `sp_meter_bars_at` | 362–375 | tempo | none | none | curve lookup |
| 161 | `build_beat_events` | 377–411 | screenB | none | none |  |
| 161 | `build_beat_events` | 377–411 | tempo | tempo-9;tempo-4 | copies | beat = tick_r, half = tick_r/2, own per-section bar walk |
| 162 | `clock_str` | 415–422 | display | display-15 | owns | "%d:%06.3f" can print 0:60.000 |
| 162 | `clock_str` | 415–422 | screenB | none | none |  |
| 162 | `clock_str` | 415–422 | tempo | tempo-28 | none | m:ss.mmm text |
| 163 | `tick_at` | 427–430 | display | none | none | tick lookup |
| 163 | `tick_at` | 427–430 | screenB | screenB-8 | owns |  |
| 163 | `tick_at` | 427–430 | tempo | tempo-2 | copies | llround + clamp >= 0; owner none (path_view inline copy) |
| 164 | `shown_length` | 433–433 | display | none | none | clamp |
| 164 | `shown_length` | 433–433 | screenB | screenB-8 | owns |  |
| 164 | `shown_length` | 433–433 | tempo | tempo-12 | none | clamp length |
| 165 | `shown_ms` | 438–441 | display | none | none | clamp |
| 165 | `shown_ms` | 438–441 | screenB | screenB-8 | owns |  |
| 165 | `shown_ms` | 438–441 | tempo | tempo-12 | none | clamp playhead |
| 166 | `build_time_box` | 446–487 | display | display-12 | copies | length measure from transport length (max(last note, audio)), Paths uses last note |
| 166 | `build_time_box` | 446–487 | screenB | screenB-7;screenB-8 | copies | BPM by ms vs time sig/section by tick; two rules for 'in force' |
| 166 | `build_time_box` | 446–487 | tempo | tempo-3;tempo-5;tempo-2;tempo-13 | copies | tempo in force by ms loop (owner MsIndex::tps_at) while signature/section by rounded tick |
| 167 | `build_score_box` | 489–512 | display | none | wrapper | score box reads replay steps |
| 167 | `build_score_box` | 489–512 | score | score-24 | wrapper | build_score_box reads step at playhead |
| 167 | `build_score_box` | 489–512 | screenB | screenB-9 | wrapper | reads replay steps |
| 168 | `build_drain_box` | 514–555 | backend | none | none | SP drain box (SP-window family) |
| 168 | `build_drain_box` | 514–555 | display | display-24 | copies | SP running = a.ms <= now < sp_end_ms |
| 168 | `build_drain_box` | 514–555 | screenB | screenB-3;screenB-4;screenB-5;screenB-6;screenB-7;screenB-8 | copies | own playhead clamp; running test duplicates track_state/curve; full meter differs from SongTiming::sp_end_ms |
| 168 | `build_drain_box` | 514–555 | spwin | spwin-21;spwin-11 | copies | running = a.ms <= now < sp_end_ms (half-open, ms) vs engine inclusive-at-D |
| 168 | `build_drain_box` | 514–555 | tempo | tempo-8;tempo-22 | copies | SP running = a.ms <= now < sp_end_ms; rate via ms_per_measure_at(now_tick) |
| 169 | `step_tick_ms` | 558–563 | screenB | screenB-8 | wrapper | uses shown_ms |
| 169 | `step_tick_ms` | 558–563 | tempo | tempo-2;tempo-1 | wrapper | tick_at then MsIndex::at |
| 170 | `build_scrub_marks` | 565–572 | display | display-12 | copies | fraction over transport length; Paths song_fraction over last-note length |
| 170 | `build_scrub_marks` | 565–572 | screenB | none | none |  |
| 170 | `build_scrub_marks` | 565–572 | tempo | tempo-14 | copies | activation fraction over transport length; build_activations uses last-note length |
| 171 | `activation_jump_ms` | 580–589 | display | none | none | kOnActivationMs 0.5 |
| 171 | `activation_jump_ms` | 580–589 | screenB | screenB-11 | owns |  |
| 171 | `activation_jump_ms` | 580–589 | tempo | tempo-21 | owns | kOnActivationMs tolerance |
| 172 | `build_next_act_box` | 591–604 | display | none | none | next box text |
| 172 | `build_next_act_box` | 591–604 | screenB | screenB-11 | owns |  |
| 172 | `build_next_act_box` | 591–604 | tempo | tempo-21 | copies | same tolerance re-applied; owner activation_jump_ms constant |
| 173 | `sp_meter_readout` | 606–611 | display | none | none | "%.1f/%d" |
| 173 | `sp_meter_readout` | 606–611 | screenB | screenB-1 | wrapper |  |
| 173 | `sp_meter_readout` | 606–611 | spwin | spwin-20 | none | readout text |
| 174 | `preview_path_label` | 613–620 | display | none | none | "(optimal)"/"(best all-0)" labels |
| 174 | `preview_path_label` | 613–620 | screenB | screenB-18 | wrapper | reads PathButtonView group |
| 175 | `path_overlay_key` | 622–628 | display | none | none | key |
| 175 | `path_overlay_key` | 622–628 | screenB | none | none |  |

### `src/app/report.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 176 | `page_template` | 159–162 | display | none | none | page shell |
| 176 | `page_template` | 159–162 | screenB | none | none |  |
| 177 | `py_repr` | 165–177 | display | none | none | float repr for JSON |
| 177 | `py_repr` | 165–177 | screenB | none | none |  |
| 178 | `plain` | 181–214 | display | display-9 | copies | strips only <color> tags and trims; app::strip_rich_tags strips 8 tag kinds and does not trim |
| 178 | `plain` | 181–214 | screenB | none | none |  |
| 179 | `counted` | 216–218 | display | display-10 | copies | count+noun; library_table charts_text is the same rule |
| 179 | `counted` | 216–218 | screenB | none | none |  |
| 180 | `tier_for` | 221–225 | display | display-5 | wrapper | calls tier_for(table) |
| 180 | `tier_for` | 221–225 | screenB | screenB-13 | wrapper |  |
| 180 | `tier_for` | 221–225 | squeeze | squeeze-8 | wrapper | timing_tiers |
| 180 | `tier_for` | 221–225 | tempo | tempo-25 | wrapper | tier lookup |
| 181 | `tier_for` | 228–237 | display | display-4;display-5 | owns | tier label; Normal floor < kDifficultMs disagrees with is_difficult at exactly 2.0 |
| 181 | `tier_for` | 228–237 | screenB | screenB-13 | owns | label lookup |
| 181 | `tier_for` | 228–237 | squeeze | squeeze-8 | owns | band lookup |
| 181 | `tier_for` | 228–237 | tempo | tempo-25 | wrapper | tier lookup |
| 182 | `records_by_hash` | 241–248 | display | none | none | join helper |
| 182 | `records_by_hash` | 241–248 | parse | parse-8 | copies | lowercases hyhash for matching |
| 182 | `records_by_hash` | 241–248 | screenB | none | none |  |
| 182 | `records_by_hash` | 241–248 | store | store-3 | wrapper | list_records |
| 183 | `collect_rows` | 253–304 | display | display-3;display-8 | copies | efill: own max over e_difficulty (no owner elsewhere); mult via py_round3 (wrapper); hardest via summarize_path |
| 183 | `collect_rows` | 253–304 | fills | fills-14 | wrapper | efill from e_difficulty |
| 183 | `collect_rows` | 253–304 | score | score-13;score-19;score-27 | copies | collect_rows re-sorts paths by totalscore (engine order owner); rank 1 = best |
| 183 | `collect_rows` | 253–304 | screenB | screenB-13;screenB-16;screenB-17;screenB-18 | copies | rank-1 by own stable sort; owner HydraRecord::best_path; efill loop wraps e_difficulty |
| 183 | `collect_rows` | 253–304 | squeeze | squeeze-8;squeeze-27 | wrapper | summarize_path + tier_for |
| 183 | `collect_rows` | 253–304 | store | store-22 | wrapper | summarize_path |
| 184 | `build_html` | 307–371 | display | display-3;display-5 | copies | page JS stats re-decides Beyond via r.ms >= BEYOND; fmtMs/toFixed(1) and toFixed(3) are JS copies of format_ms/format_avg_mult with different tie rounding |
| 184 | `build_html` | 307–371 | score | score-19 | copies | build_html page JS: rank===1 is the only best row; path_view marks every top-score tie Optimal |
| 184 | `build_html` | 307–371 | screenB | screenB-13;screenB-14;screenB-29 | copies | page JS recomputes BEYOND (Math.max) and counts Beyond by r.ms >= BEYOND; owner beyond_edge_ms/tier_for |
| 184 | `build_html` | 307–371 | squeeze | squeeze-8 | wrapper | serializes tiers |
| 185 | `generate_report` | 374–413 | display | display-5;display-10 | copies | footer truncates beyond_edge_ms; cap_label "N bars" never singular |
| 185 | `generate_report` | 374–413 | screenB | screenB-14 | wrapper | footer prints beyond_edge_ms |
| 185 | `generate_report` | 374–413 | squeeze | squeeze-8;squeeze-9 | wrapper | footer from beyond_edge_ms; claims tiers match ratings |
| 185 | `generate_report` | 374–413 | store | none | none |  |
| 185 | `generate_report` | 374–413 | tempo | tempo-24 | wrapper | int hit window to double |

### `src/app/report_files.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 186 | `db_folder` | 20–22 | sweep-core1 | sweep-core1-6 | owns | fallback report folder = db folder |
| 187 | `known_documents_dir` | 25–33 | sweep-core1 | sweep-core1-6 | owns | Documents lookup |
| 188 | `html_artifact_path` | 36–36 | sweep-core1 | sweep-core1-6 | owns | report page path = reports_dir()/name |
| 189 | `reports_dir` | 43–56 | sweep-core1 | sweep-core1-6 | owns | Documents\Hydra, db folder fallback |
| 190 | `set_documents_dir_lookup` | 58–58 | sweep-core1 | none | none | test seam |
| 191 | `set_open_in_browser` | 60–60 | sweep-core1 | none | none | test seam |
| 192 | `open_in_browser` | 62–67 | sweep-core1 | none | none | ShellExecute >32 success rule |
| 193 | `report_html_path` | 69–69 | sweep-core1 | sweep-core1-6 | owns | "hydra_paths.html"; src/cli/report.cpp repeats the literal as its default out |
| 194 | `dm_report_html_path` | 71–71 | sweep-core1 | sweep-core1-6 | owns | "hydra_dmcompare.html" |
| 195 | `open_report_in_browser` | 73–73 | sweep-core1 | sweep-core1-6 | wrapper | report_html_path |
| 196 | `open_dm_report_in_browser` | 75–75 | sweep-core1 | sweep-core1-6 | wrapper | dm_report_html_path |
| 197 | `report_file_exists` | 77–79 | sweep-core1 | sweep-core1-7 | copies | winstr file_exists_utf8 (same GetFileAttributesW test on a wide path) |
| 198 | `write_report_file` | 81–106 | sweep-core1 | none | none | atomic temp+rename write |

### `src/app/rules_file.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 199 | `bad` | 14–16 | store | none | none |  |
| 200 | `to_double` | 19–26 | store | store-15 | owns |  |
| 201 | `to_int` | 29–36 | store | store-15 | owns |  |
| 202 | `load_rules_file` | 40–77 | backend | backend-16 | owns | reads backend_leeway_ms, min 0.0 |
| 202 | `load_rules_file` | 40–77 | fills | fills-36;fills-31 | owns | fill keys and ranges; ignores auto keys |
| 202 | `load_rules_file` | 40–77 | store | store-14;store-15;store-16 | copies | field names and sqout spellings typed again; owner fixed_cap_text for the list |
| 203 | `default_rules_path` | 79–83 | store | store-15 | owns |  |

### `src/app/user_messages.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 204 | `starts_with` | 61–63 | display | none | none | helper |
| 205 | `ends_with` | 64–66 | display | none | none | helper |
| 206 | `starts_with_any` | 67–71 | display | none | none | helper |
| 207 | `is_no_notes_message` | 75–77 | display | display-28 | copies | copies parse/song.cpp no_notes_message shape |
| 208 | `plain_error_text` | 81–146 | display | display-28 | copies | prefix copies of throw-site literals in dmbot_client, record_store, serialize, decode etc. |
| 209 | `plain_error` | 148–158 | display | display-28 | wrapper | calls plain_error_text, then type fallbacks |
| 210 | `plain_error_detail` | 160–160 | display | none | none | what() |

### `src/app/work_pool.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 211 | `run_work_pool` | 42–93 | sweep-core1 | none | none | thread pool |

### `src/audio/decode.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 212 | `has_tag` | 29–33 | sweep-media1 | sweep-media1-1 | owns | part of sniff_format magic test |
| 213 | `contains_tag` | 38–45 | sweep-media1 | sweep-media1-1 | owns | part of sniff_format (Ogg codec tag within 64 bytes) |
| 214 | `sniff_format` | 49–68 | sweep-media1 | sweep-media1-1 | owns | audio container by magic; copied by app::looks_like_audio (preview_source.cpp) with a looser truth table |
| 215 | `decode_with_miniaudio` | 74–103 | sweep-media1 | display-28 | owns | throw-site literal decode_audio: prefix |
| 216 | `decode_ogg_vorbis` | 106–122 | sweep-media1 | display-28 | owns | throw-site literal decode_audio: prefix |
| 217 | `decode_ogg_opus` | 133–222 | sweep-media1 | sweep-media1-2;display-28 | owns | Opus 48 kHz written twice (out.sample_rate, opus_decoder_create) + kMaxFrame 5760 = 120 ms x 48 |
| 218 | `decode_audio` | 226–240 | sweep-media1 | sweep-media1-1;display-28 | wrapper | dispatch on sniff_format |
| 219 | `decode_stem` | 242–248 | sweep-media1 | none | none | file vs bytes plumbing |

### `src/audio/decode.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 220 | `DecodedAudio::frames` | 36–38 | sweep-media1 | sweep-media1-3 | owns | frames = samples / channels |
| 221 | `DecodedAudio::empty` | 39–39 | sweep-media1 | none | none | plumbing |
| 222 | `sniff_format` | 49–51 | sweep-media1 | sweep-media1-1 | wrapper | calls sniff_format |
| 223 | `decode_audio` | 56–58 | sweep-media1 | none | wrapper | calls decode_audio |

### `src/audio/device.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 224 | `data_callback` | 26–29 | sweep-media1 | none | none | device callback plumbing |
| 225 | `set_headless` | 37–37 | sweep-media1 | none | none | headless flag |
| 226 | `PreviewAudioDevice::PreviewAudioDevice` | 47–47 | sweep-media1 | none | none | device init |
| 227 | `PreviewAudioDevice::~PreviewAudioDevice` | 63–68 | sweep-media1 | none | none | device teardown |
| 228 | `PreviewAudioDevice::start` | 70–77 | sweep-media1 | none | none | device start |

### `src/audio/mixer.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 229 | `convert_stem` | 24–56 | sweep-media1 | display-28 | owns | throw-site literal mix_stems: prefix; resample/channel map |
| 230 | `add_into` | 62–65 | sweep-media1 | sweep-media1-4 | owns | stems summed in order, zero-extend shorter |
| 231 | `mix_stems` | 70–77 | sweep-media1 | sweep-media1-4 | wrapper | calls convert_stem/add_into; production-unused (tests only) |
| 232 | `decode_and_mix` | 81–103 | sweep-media1 | sweep-media1-4;sweep-media1-5 | owns | skips an undecodable stem (CONTEXT Mixer) |
| 233 | `pad_front_ms` | 105–109 | tempo | tempo-20 | copies | ms->frames llround; same formula as Playhead::seek_ms |

### `src/audio/player.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 234 | `Playhead::Playhead` | 13–13 | sweep-media1 | sweep-media1-3 | wrapper | length_ = audio.frames() |
| 235 | `Playhead::play` | 15–15 | sweep-media1 | none | none | state flip |
| 236 | `Playhead::pause` | 16–16 | sweep-media1 | none | none | state flip |
| 237 | `Playhead::toggle` | 17–17 | sweep-media1 | none | none | state flip |
| 238 | `Playhead::seek_frames` | 19–21 | sweep-media1 | sweep-media1-6 | owns | playhead clamp [0, length frames] |
| 239 | `Playhead::seek_ms` | 23–26 | tempo | tempo-20 | copies | ms->frames llround; same formula as pad_front_ms |
| 240 | `Playhead::position_ms` | 28–30 | tempo | tempo-20 | copies | frames->ms |
| 241 | `Playhead::length_ms` | 32–34 | tempo | tempo-20 | copies | frames->ms |
| 242 | `Playhead::read_frames` | 36–69 | sweep-media1 | sweep-media1-7 | owns | auto-pause at end of audio, silence when paused |

### `src/audio/player.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 243 | `Playhead::playing` | 30–30 | sweep-media1 | none | none | getter |
| 244 | `Playhead::position_frames` | 36–36 | sweep-media1 | none | none | getter |
| 245 | `Playhead::length_frames` | 38–38 | sweep-media1 | none | none | getter |
| 246 | `Playhead::channels` | 40–40 | sweep-media1 | none | none | getter |
| 247 | `Playhead::sample_rate` | 41–41 | sweep-media1 | none | none | getter |
| 248 | `Playhead::set_gain` | 45–45 | sweep-media1 | sweep-media1-8 | owns | gain clamp < 0 -> 0; re-typed in PreviewTransport::set_gain |
| 249 | `Playhead::gain` | 46–46 | sweep-media1 | none | none | getter |

### `src/cli/batch.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 250 | `clip_utf8` | 56–65 | display | none | none | console clip |
| 250 | `clip_utf8` | 56–65 | screenB | none | none |  |
| 251 | `same_file` | 73–81 | display | none | none | path compare |
| 251 | `same_file` | 73–81 | screenB | none | none |  |
| 252 | `main` | 85–276 | display | display-9;display-10;display-20 | copies | prints title/artist raw (no tag strip); "SP cap : N bars"; error text says the rule is not stored on each result (ADR 0010 superseded) |
| 252 | `main` | 85–276 | fills | fills-10;fills-11;fills-33 | copies | reads engine_mode stamp to refuse; stale message says rule not stored per result |
| 252 | `main` | 85–276 | screenB | screenB-22;screenB-23;screenB-24;screenB-32 | copies | decides DB fill rule (unstamped = ch11); stderr says results are not tagged (contradicts CONTEXT) |
| 252 | `main` | 85–276 | store | store-10;store-26;store-28 | copies | engine_mode interpretation and stale guard text; no single owner |

### `src/cli/fillcompare.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 253 | `main` | 36–140 | display | display-21 | wrapper | prints stats |
| 253 | `main` | 36–140 | fills | fills-10;fills-11 | copies | reads file engine_mode stamp beside the row key |
| 253 | `main` | 36–140 | screenB | screenB-22 | copies | decides DB fill rule (unstamped never warns) |
| 253 | `main` | 36–140 | store | store-10 | copies | engine_mode interpretation; no single owner |

### `src/cli/report.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 254 | `main` | 36–123 | display | none | none | CLI wrapper over generate_report |
| 254 | `main` | 36–123 | fills | fills-10 | copies | engine_mode stamp overrides app setting |
| 254 | `main` | 36–123 | screenB | screenB-22 | copies | decides DB fill rule (stamp ch10 forces legacy lens) |
| 254 | `main` | 36–123 | store | store-9;store-10 | copies | ch10 stamp forces legacy lens; no single owner |

### `src/core/backend_value.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 255 | `sqout_position` | 21–26 | backend | backend-3 | owns | sqout_position |
| 255 | `sqout_position` | 21–26 | score | score-7 | owns | sqout_position |
| 255 | `sqout_position` | 21–26 | screenA | screenA-18 | owns |  |
| 255 | `sqout_position` | 21–26 | spwin | spwin-15 | owns | sqout_position Before/Exact/After |
| 255 | `sqout_position` | 21–26 | squeeze | squeeze-21;squeeze-22 | owns | sqout position by tick |
| 256 | `paid_by_sp_walk` | 30–30 | backend | backend-1 | owns | paid_by_sp_walk |
| 256 | `paid_by_sp_walk` | 30–30 | score | score-6;score-7 | owns | paid_by_sp_walk |
| 256 | `paid_by_sp_walk` | 30–30 | screenA | screenA-17 | owns |  |
| 256 | `paid_by_sp_walk` | 30–30 | spwin | spwin-11 | owns | offset <= 0 paid by SP walk (ms form of graph structure) |
| 256 | `paid_by_sp_walk` | 30–30 | squeeze | squeeze-20 | owns | paid by SP walk |
| 257 | `counted_without_squeeze` | 34–36 | backend | backend-2;backend-16 | owns | counted_without_squeeze; strict < at leeway |
| 257 | `counted_without_squeeze` | 34–36 | score | score-6;score-7 | owns | counted_without_squeeze |
| 257 | `counted_without_squeeze` | 34–36 | screenA | screenA-17 | owns |  |
| 257 | `counted_without_squeeze` | 34–36 | spwin | spwin-11 | owns | paid or under leeway |
| 257 | `counted_without_squeeze` | 34–36 | squeeze | squeeze-20 | owns | leeway rule |
| 258 | `backend_row_value` | 41–45 | backend | backend-4 | owns | backend_row_value |
| 258 | `backend_row_value` | 41–45 | score | score-7 | owns | backend_row_value |
| 258 | `backend_row_value` | 41–45 | screenA | screenA-16;screenA-18 | owns |  |
| 258 | `backend_row_value` | 41–45 | spwin | none | none | row value (scoring family) |
| 258 | `backend_row_value` | 41–45 | squeeze | squeeze-24 | owns | row value |

### `src/core/model.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 259 | `allows_cymbals` | 14–17 | parse | parse-12 | owns | allows_cymbals |
| 260 | `color_str` | 19–28 | parse | none | none | color names |
| 261 | `dynamic_str` | 30–37 | parse | none | none | dynamic names |
| 262 | `color_notationstr` | 39–48 | parse | none | none | lane letters |
| 263 | `operator==` | 52–55 | parse | none | none | equality |
| 264 | `ChordNote::str` | 57–79 | parse | parse-12 | none | display string uses allows_cymbals |
| 265 | `ChordNote::basescore` | 81–85 | parse | none | none | scoring family |
| 265 | `ChordNote::basescore` | 81–85 | score | score-1;score-17;score-18 | owns | ChordNote::basescore: one note's 1x value; also read by MultSqueeze and sqout |
| 266 | `lane_flag` | 93–95 | parse | parse-12 | copies | kick->2x, pad->cymbal flag rule spelled again; owner allows_cymbals |
| 267 | `lane_allows_flag` | 97–99 | parse | parse-12 | copies | Kick or allows_cymbals; third spelling of the lane-flag rule |
| 268 | `Chord::code` | 103–124 | parse | parse-12;parse-18 | owns | chord code; stray_flag re-spells lane rule inline (copies allows_cymbals) |
| 269 | `Chord::from_code` | 126–153 | parse | parse-18 | owns | code read back |
| 270 | `operator==` | 157–164 | parse | none | none | equality |
| 271 | `Chord::at` | 166–168 | parse | none | none | accessor |
| 272 | `Chord::at` | 170–172 | parse | none | none | accessor |
| 273 | `Chord::notes` | 174–185 | parse | none | none | notes |
| 274 | `Chord::count` | 187–192 | parse | none | none | count |
| 275 | `Chord::hands_count` | 194–196 | parse | none | none | hands |
| 276 | `Chord::rowstr` | 198–206 | display | none | owns | chord row text |
| 276 | `Chord::rowstr` | 198–206 | parse | none | none | display |
| 276 | `Chord::rowstr` | 198–206 | screenA | screenA-10 | owns |  |
| 277 | `Chord::notationstr` | 208–216 | display | none | owns | chord notation |
| 277 | `Chord::notationstr` | 208–216 | parse | none | none | display |
| 277 | `Chord::notationstr` | 208–216 | screenA | screenA-18 | owns |  |
| 278 | `Chord::apply_disco_flip` | 218–233 | parse | parse-15 | owns | disco flip transform |
| 279 | `Chord::apply_flam_conversion` | 235–246 | parse | parse-20 | owns | flam transform |
| 280 | `Chord::add_note` | 248–253 | parse | parse-10 | owns | duplicate note rejection |
| 281 | `Chord::insert_note` | 255–255 | parse | none | none | insert |
| 282 | `Chord::add_2x` | 257–260 | parse | parse-13 | owns | 2x kick note |
| 283 | `Chord::apply_cymbal` | 262–267 | parse | parse-11 | owns | cymbal |
| 284 | `Chord::apply_ghost` | 269–272 | parse | parse-16 | owns | ghost |
| 285 | `Chord::apply_accent` | 274–277 | parse | parse-16 | owns | accent |
| 286 | `Chord::activation_note` | 279–288 | parse | none | none | activation note |
| 287 | `SPSqueeze::description` | 292–301 | display | display-1 | copies | "%.1fms" written inline instead of format_ms |
| 287 | `SPSqueeze::description` | 292–301 | screenA | screenA-15 | owns | SqIn/SqOut edge wording (report/verbose) |
| 287 | `SPSqueeze::description` | 292–301 | squeeze | squeeze-32 | wrapper | prints timing() |
| 288 | `operator==` | 305–309 | backend | none | none | BackendSqueeze equality |
| 289 | `BackendSqueeze::summarystr` | 311–327 | backend | backend-9;backend-2;backend-12;backend-15 | owns | label ladder owner; Standard edge wraps counted_without_squeeze; value_or(0.0) fallback; -10/+10 edges literal |
| 289 | `BackendSqueeze::summarystr` | 311–327 | display | display-16 | owns | rating label; inner -10/10 edges fixed |
| 289 | `BackendSqueeze::summarystr` | 311–327 | screenA | screenA-21 | owns | -10/10 absolute edges |
| 289 | `BackendSqueeze::summarystr` | 311–327 | screenB | none | none | Paths tab backend label ladder (+-10, +-w) |
| 289 | `BackendSqueeze::summarystr` | 311–327 | spwin | spwin-11 | wrapper | rating labels; calls counted_without_squeeze |
| 289 | `BackendSqueeze::summarystr` | 311–327 | squeeze | squeeze-9;squeeze-10;squeeze-1;squeeze-20 | owns | rating ladder (+-10, +-W); is_sp labels every SP row as SqOut; Free SqOut only at >=W (copies earned/free question) |
| 289 | `BackendSqueeze::summarystr` | 311–327 | tempo | tempo-26;tempo-24 | owns | +-10 literal and +-W edges |
| 290 | `MultSqueeze::MultSqueeze` | 332–334 | score | score-17 | wrapper | MultSqueeze ctor calls validate |
| 291 | `MultSqueeze::applies` | 345–358 | score | score-2;score-17 | copies | MultSqueeze::applies hard-codes combos 7,8,17,18,27,28 instead of asking to_multiplier; owner to_multiplier |
| 292 | `MultSqueeze::validate` | 360–363 | score | score-17 | wrapper | MultSqueeze::validate calls applies |
| 293 | `MultSqueeze::multiplier` | 365–365 | score | score-2 | copies | MultSqueeze::multiplier = to_multiplier(combo)+1; owner to_multiplier |
| 294 | `MultSqueeze::direction` | 367–369 | score | score-2;score-17 | copies | MultSqueeze::direction from combo%10; owner to_multiplier straddle geometry |
| 295 | `MultSqueeze::points` | 371–374 | display | none | none | points |
| 295 | `MultSqueeze::points` | 371–374 | score | score-18 | copies | MultSqueeze::points = hi-lo basescore; category_scores pays the real gain (differs on 4-note chords) |
| 295 | `MultSqueeze::points` | 371–374 | screenA | screenA-27 | owns |  |
| 296 | `MultSqueeze::notationstr` | 376–378 | display | none | none | "Nx" |
| 296 | `MultSqueeze::notationstr` | 376–378 | score | score-2 | wrapper | MultSqueeze::notationstr prints multiplier() |
| 296 | `MultSqueeze::notationstr` | 376–378 | screenA | screenA-27 | owns |  |
| 297 | `MultSqueeze::howto` | 380–409 | display | none | none | howto text |
| 297 | `MultSqueeze::howto` | 380–409 | score | score-18 | none | MultSqueeze::howto advice text from basescore edges |
| 297 | `MultSqueeze::howto` | 380–409 | screenA | screenA-27 | owns |  |
| 298 | `Activation::is_e_critical` | 413–415 | display | none | none | E-critical |
| 298 | `Activation::is_e_critical` | 413–415 | fills | fills-13 | owns | e_offset < kEarlyFillWindowMs |
| 298 | `Activation::is_e_critical` | 413–415 | screenA | screenA-14 | owns |  |
| 298 | `Activation::is_e_critical` | 413–415 | screenB | none | none |  |
| 298 | `Activation::is_e_critical` | 413–415 | spwin | none | none | E window |
| 298 | `Activation::is_e_critical` | 413–415 | tempo | tempo-18 | copies | e_offset < kEarlyFillWindowMs re-stated; is_e0 has the same edge |
| 299 | `Activation::is_E0` | 417–417 | display | none | none | E0 |
| 299 | `Activation::is_E0` | 417–417 | fills | fills-14 | wrapper | calls is_e0 |
| 299 | `Activation::is_E0` | 417–417 | screenA | screenA-14 | owns |  |
| 299 | `Activation::is_E0` | 417–417 | screenB | screenB-17 | wrapper | is_e0 |
| 300 | `Activation::e_difficulty` | 419–422 | display | display-2 | owns | sign: -e_offset, positive = early |
| 300 | `Activation::e_difficulty` | 419–422 | fills | fills-37 | wrapper | calls early_fill_difficulty |
| 300 | `Activation::e_difficulty` | 419–422 | screenA | screenA-14;screenA-40 | owns |  |
| 300 | `Activation::e_difficulty` | 419–422 | screenB | screenB-17 | owns |  |
| 300 | `Activation::e_difficulty` | 419–422 | tempo | tempo-19 | owns | sign via early_fill_difficulty |
| 301 | `Activation::difficulty` | 424–430 | display | display-6;display-7 | owns | activation hardest value |
| 301 | `Activation::difficulty` | 424–430 | fills | fills-15 | copies | same max as Engine::act_difficulty |
| 301 | `Activation::difficulty` | 424–430 | screenA | screenA-4;screenA-11 | owns |  |
| 301 | `Activation::difficulty` | 424–430 | screenB | screenB-16 | owns |  |
| 301 | `Activation::difficulty` | 424–430 | squeeze | squeeze-5 | owns | activation hardest ms |
| 302 | `Activation::is_difficult` | 432–437 | display | display-4 | owns | activation warn rule |
| 302 | `Activation::is_difficult` | 432–437 | fills | fills-15 | none | warning colour rule (squeeze family) |
| 302 | `Activation::is_difficult` | 432–437 | screenA | screenA-5;screenA-12 | copies | same truth table as difficulty()>kDifficultMs, written separately (owner Path::is_difficult form) |
| 302 | `Activation::is_difficult` | 432–437 | screenB | screenB-15 | copies | re-loops is_difficult instead of difficulty() > kDifficultMs; edge differs from timing_tiers Normal |
| 302 | `Activation::is_difficult` | 432–437 | squeeze | squeeze-7 | copies | own loop instead of difficulty()>kDifficultMs (owner SPSqueeze::is_difficult / Path::is_difficult form) |
| 303 | `Activation::notationstr` | 439–444 | display | none | owns | notation |
| 303 | `Activation::notationstr` | 439–444 | fills | fills-13 | wrapper | E prefix from is_e_critical |
| 303 | `Activation::notationstr` | 439–444 | screenA | screenA-10 | owns |  |
| 303 | `Activation::notationstr` | 439–444 | screenB | none | none |  |
| 303 | `Activation::notationstr` | 439–444 | squeeze | none | none | notation symbols |
| 304 | `Activation::notationstr_verbose` | 446–463 | display | display-1 | copies | whole-ms by truncation toward zero (badge rounds) |
| 304 | `Activation::notationstr_verbose` | 446–463 | fills | fills-13 | wrapper | uses is_e_critical |
| 304 | `Activation::notationstr_verbose` | 446–463 | screenA | screenA-41;screenA-11 | copies | truncates difficulty to whole ms; badge rounds |
| 304 | `Activation::notationstr_verbose` | 446–463 | screenB | none | none |  |
| 304 | `Activation::notationstr_verbose` | 446–463 | squeeze | squeeze-4 | wrapper | prints difficulty |
| 305 | `Path::all_activations` | 467–473 | sweep-core1 | sweep-core1-4 | copies | Path::walk_activations / ActivationWalk (own then variant tail) |
| 306 | `Path::has_activations` | 475–477 | sweep-core1 | sweep-core1-4 | copies | ActivationWalk::empty |
| 307 | `Path::totalscore` | 479–482 | display | none | none | total |
| 307 | `Path::totalscore` | 479–482 | score | score-10 | owns | Path::totalscore |
| 307 | `Path::totalscore` | 479–482 | screenA | screenA-1 | owns |  |
| 307 | `Path::totalscore` | 479–482 | screenB | screenB-9 | owns |  |
| 308 | `Path::pathstring` | 484–493 | display | none | owns | pathstring |
| 308 | `Path::pathstring` | 484–493 | screenA | screenA-2 | owns |  |
| 308 | `Path::pathstring` | 484–493 | screenB | none | none |  |
| 309 | `Path::pathstring_verbose` | 495–529 | display | display-1 | wrapper | notationstr_verbose |
| 309 | `Path::pathstring_verbose` | 495–529 | score | score-10 | wrapper | Path::pathstring_verbose prints totalscore |
| 309 | `Path::pathstring_verbose` | 495–529 | screenA | screenA-41 | owns |  |
| 309 | `Path::pathstring_verbose` | 495–529 | screenB | none | none |  |
| 310 | `Path::recount_tied_paths` | 531–538 | fills | fills-23 | owns | tied count from variant tree |
| 311 | `Path::prepare_variants` | 540–556 | score | score-10;score-11 | none | Path::prepare_variants copies parent totals onto variants |
| 312 | `Path::difficulty` | 558–566 | display | display-7 | owns | path hardest |
| 312 | `Path::difficulty` | 558–566 | fills | fills-16 | owns | max over activations |
| 312 | `Path::difficulty` | 558–566 | screenA | screenA-4 | owns |  |
| 312 | `Path::difficulty` | 558–566 | screenB | screenB-16 | owns |  |
| 312 | `Path::difficulty` | 558–566 | squeeze | squeeze-6 | owns | path hardest ms |
| 313 | `Path::is_difficult` | 568–571 | display | display-4 | owns | path warn rule |
| 313 | `Path::is_difficult` | 568–571 | fills | fills-16 | wrapper | calls difficulty |
| 313 | `Path::is_difficult` | 568–571 | screenA | screenA-5 | owns |  |
| 313 | `Path::is_difficult` | 568–571 | screenB | screenB-15 | owns | > kDifficultMs |
| 313 | `Path::is_difficult` | 568–571 | squeeze | squeeze-7 | owns | difficulty()>kDifficultMs |
| 314 | `Activation::is_sqout_backend` | 575–577 | backend | backend-3 | copies | tick == sqout_tick re-decides SqOutPosition::Exact; owner sqout_position |
| 314 | `Activation::is_sqout_backend` | 575–577 | score | score-5;score-7 | owns | Activation::is_sqout_backend (squeeze family owner) |
| 314 | `Activation::is_sqout_backend` | 575–577 | screenA | screenA-19 | owns |  |
| 314 | `Activation::is_sqout_backend` | 575–577 | spwin | spwin-15;spwin-16 | copies | tick == sqout_tick duplicates sqout_position Exact |
| 314 | `Activation::is_sqout_backend` | 575–577 | squeeze | squeeze-21 | owns | tick == sqout_tick |
| 315 | `Activation::display_backends` | 579–596 | backend | backend-3;backend-8;backend-12 | copies | is_beyond_sqout re-decides SqOutPosition::After (owner sqout_position); fabs<kSqueezeWindowMs; value_or(0.0) |
| 315 | `Activation::display_backends` | 579–596 | display | none | none | display rows |
| 315 | `Activation::display_backends` | 579–596 | score | score-7 | none | Activation::display_backends drops rows past sqout tick |
| 315 | `Activation::display_backends` | 579–596 | screenA | screenA-20 | owns |  |
| 315 | `Activation::display_backends` | 579–596 | spwin | spwin-14;spwin-15 | copies | is_beyond_sqout copies sqout_position After; fabs<kSqueezeWindowMs re-decides graph window (owner is_recent_to_head) |
| 315 | `Activation::display_backends` | 579–596 | squeeze | squeeze-3;squeeze-22;squeeze-30 | copies | is_beyond_sqout repeats engine copy-out trim (owner sqout_position After / rebuild) |
| 315 | `Activation::display_backends` | 579–596 | tempo | tempo-16 | copies | fabs(offset) < kSqueezeWindowMs |
| 316 | `Path::is_allzero` | 598–604 | display | none | none | all-0 |
| 316 | `Path::is_allzero` | 598–604 | fills | fills-29 | owns | all-0 check |
| 316 | `Path::is_allzero` | 598–604 | screenA | none | none |  |
| 317 | `Path::chart_base_score` | 606–608 | display | none | none | base score |
| 317 | `Path::chart_base_score` | 606–608 | score | score-11 | owns | Path::chart_base_score |
| 317 | `Path::chart_base_score` | 606–608 | screenA | screenA-30 | owns |  |
| 317 | `Path::chart_base_score` | 606–608 | screenB | screenB-28 | owns |  |
| 318 | `Path::avg_mult` | 610–615 | display | display-3 | owns | avg mult value |
| 318 | `Path::avg_mult` | 610–615 | score | score-12;score-13 | owns | Path::avg_mult; also subtracts solo itself (score-12 has no owner) |
| 318 | `Path::avg_mult` | 610–615 | screenA | screenA-28 | owns |  |
| 318 | `Path::avg_mult` | 610–615 | screenB | screenB-28 | owns |  |
| 319 | `flatten_paths` | 624–638 | screenA | screenA-3 | owns |  |
| 319 | `flatten_paths` | 624–638 | sweep-core1 | score-27 | owns | depth-first path-tree order |
| 320 | `HydraRecord::all_paths` | 642–644 | screenA | screenA-3 | wrapper |  |
| 320 | `HydraRecord::all_paths` | 642–644 | screenB | screenB-18 | none |  |
| 320 | `HydraRecord::all_paths` | 642–644 | sweep-core1 | score-27 | wrapper | flatten_paths |
| 321 | `HydraRecord::all_allzero_paths` | 646–648 | screenA | screenA-3 | wrapper |  |
| 321 | `HydraRecord::all_allzero_paths` | 646–648 | sweep-core1 | score-27 | wrapper | flatten_paths |
| 322 | `group_thousands` | 652–668 | display | display-10 | owns | thousands grouping |
| 322 | `group_thousands` | 652–668 | screenA | none | owns |  |

### `src/core/model.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 323 | `ChartFileError::ChartFileError` | 39–39 | sweep-core1 | none | none | exception type |
| 324 | `ChordNote::operator!=` | 103–103 | sweep-core1 | none | none | negation |
| 325 | `ChordNote::is_dynamic` | 106–106 | sweep-core1 | score-1;parse-16 | owns | ghost-or-accent predicate (category_scores doubling reads it) |
| 326 | `ChordNote::is_accent` | 107–107 | sweep-core1 | parse-16 | owns | accent predicate |
| 327 | `ChordNote::is_ghost` | 108–108 | sweep-core1 | parse-16 | owns | ghost predicate |
| 328 | `ChordNote::is_cymbal` | 109–109 | sweep-core1 | score-1;parse-11 | owns | cymbal predicate (+15) |
| 329 | `Chord::operator!=` | 127–127 | sweep-core1 | none | none | negation |
| 330 | `squeeze_difficulty` | 171–173 | fills | none | none | squeeze difficulty (squeeze family) |
| 330 | `squeeze_difficulty` | 171–173 | screenA | screenA-11 | owns |  |
| 330 | `squeeze_difficulty` | 171–173 | spwin | none | none | difficulty sign |
| 330 | `squeeze_difficulty` | 171–173 | squeeze | squeeze-4 | owns | difficulty sign rule |
| 331 | `is_e0` | 176–178 | fills | fills-14;fills-13 | copies | restates e_offset < kEarlyFillWindowMs instead of calling is_e_critical |
| 331 | `is_e0` | 176–178 | screenA | screenA-14 | owns |  |
| 332 | `early_fill_difficulty` | 181–181 | fills | fills-37 | owns | -e_offset |
| 332 | `early_fill_difficulty` | 181–181 | screenA | screenA-14 | owns |  |
| 333 | `SPSqueeze::offset` | 189–189 | sweep-core1 | squeeze-2 | wrapper | returns stored offset_ms |
| 334 | `SPSqueeze::timing` | 190–190 | screenA | screenA-15 | owns |  |
| 334 | `SPSqueeze::timing` | 190–190 | squeeze | squeeze-32 | owns | timing edge |
| 335 | `SPSqueeze::difficulty` | 191–191 | screenA | screenA-11 | wrapper |  |
| 335 | `SPSqueeze::difficulty` | 191–191 | squeeze | squeeze-4 | wrapper | calls squeeze_difficulty |
| 336 | `SPSqueeze::symbol` | 192–194 | sweep-core1 | sweep-core1-9 | owns | "+"/"-" symbol from kind |
| 337 | `SPSqueeze::is_difficult` | 195–195 | screenA | screenA-12 | owns |  |
| 337 | `SPSqueeze::is_difficult` | 195–195 | squeeze | squeeze-7 | owns | d > kDifficultMs |
| 338 | `SPSqueeze::type_name` | 196–198 | sweep-core1 | sweep-core1-9 | owns | "SqIn"/"SqOut"; tools/replay_json.cpp reads back with a literal "SqOut" |
| 339 | `MultSqueeze::chord` | 238–238 | sweep-core1 | none | none | accessor |
| 340 | `MultSqueeze::combo` | 239–239 | sweep-core1 | none | none | accessor |
| 341 | `MultSqueeze::operator==` | 241–243 | sweep-core1 | none | none | equality |
| 342 | `ActivationWalk::iterator::iterator` | 358–358 | sweep-core1 | none | none | iterator ctor |
| 343 | `ActivationWalk::iterator::operator*` | 359–361 | sweep-core1 | sweep-core1-4 | owns | index -> own or tail element |
| 344 | `ActivationWalk::iterator::operator->` | 362–362 | sweep-core1 | none | none | arrow |
| 345 | `ActivationWalk::iterator::operator++` | 367–371 | sweep-core1 | none | none | increment |
| 346 | `ActivationWalk::iterator::operator==` | 372–372 | sweep-core1 | none | none | compare |
| 347 | `ActivationWalk::iterator::operator!=` | 373–373 | sweep-core1 | none | none | compare |
| 348 | `ActivationWalk::ActivationWalk` | 382–382 | sweep-core1 | none | none | ctor |
| 349 | `ActivationWalk::size` | 384–384 | sweep-core1 | sweep-core1-4 | owns | own+tail count |
| 350 | `ActivationWalk::empty` | 385–385 | sweep-core1 | sweep-core1-4 | wrapper | size()==0 |
| 351 | `ActivationWalk::front` | 389–389 | sweep-core1 | none | none | accessor |
| 352 | `ActivationWalk::back` | 390–390 | sweep-core1 | none | none | accessor |
| 353 | `ActivationWalk::begin` | 391–391 | sweep-core1 | none | none | iterator |
| 354 | `ActivationWalk::end` | 392–392 | sweep-core1 | none | none | iterator |
| 355 | `Path::walk_activations` | 422–422 | sweep-core1 | sweep-core1-4 | owns | activations then variant_tail |
| 356 | `Path::tied_pathcount` | 431–431 | sweep-core1 | fills-23 | wrapper | returns tied_count |
| 357 | `HydraRecord::best_path` | 496–496 | sweep-core1 | score-19;screenB-18 | owns | paths.at(0) |
| 358 | `HydraRecord::best_path` | 497–497 | sweep-core1 | score-19;screenB-18 | owns | paths.at(0) (mutable twin) |

### `src/core/replay.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 359 | `sqout_candidates` | 30–37 | backend | backend-8;backend-14 | copies | fabs(ts.ms - D) < kSqueezeWindowMs re-decides graph candidate window; owner graph is_recent_to_head |
| 359 | `sqout_candidates` | 30–37 | score | none | none | sqout_candidates (squeeze family) |
| 359 | `sqout_candidates` | 30–37 | screenB | screenB-27 | copies | re-derives squeeze-out candidates; owner search/graph.cpp is_recent_to_head/add_deact_edge |
| 359 | `sqout_candidates` | 30–37 | spwin | spwin-13;spwin-14 | copies | first SP chord strictly within kSqueezeWindowMs of D: copy of add_deact_edge/store_new_backend rule |
| 359 | `sqout_candidates` | 30–37 | squeeze | squeeze-1;squeeze-3 | copies | re-derives graph's sqout candidate rule (owner add_deact_edge/store_new_backend) |
| 359 | `sqout_candidates` | 30–37 | tempo | tempo-16 | copies | fabs(ts.ms - D.ms) < kSqueezeWindowMs |
| 360 | `replay_path` | 42–164 | backend | backend-1;backend-2;backend-3;backend-4;backend-6;backend-7 | copies | own offset recompute (owner graph add_deact_edge); separate After/counted gate duplicates backend_row_value checks and drives in_sp |
| 360 | `replay_path` | 42–164 | fills | fills-7 | wrapper | is_fill = has_activation |
| 360 | `replay_path` | 42–164 | parse | parse-18 | none | chord_code via owner |
| 360 | `replay_path` | 42–164 | score | score-3;score-6;score-8;score-9;score-10;score-12;score-24 | copies | replay_path: own combo walk, own SP-window membership (owner engine), own solo formula (owner graph), own solo-run end, cum_onscreen_total subtracts pending solo |
| 360 | `replay_path` | 42–164 | screenA | screenA-18 | wrapper | replay uses sqout_position |
| 360 | `replay_path` | 42–164 | screenB | screenB-9 | copies | per-chord SP window membership re-decided; owner engine/graph; gated by totals |
| 360 | `replay_path` | 42–164 | spwin | spwin-11;spwin-12;spwin-15 | copies | in-window by tick (inclusive both ends) copies graph SP walk; offset formula copy; uses sqout_position |
| 360 | `replay_path` | 42–164 | squeeze | squeeze-2;squeeze-20;squeeze-22 | copies | offset formula with min(...,0) clamp differs from graph's (owner add_deact_edge) |
| 360 | `replay_path` | 42–164 | tempo | tempo-15 | copies | offset = row.ms - D.ms, clamped <= 0 for tick <= D; owner add_deact_edge |
| 361 | `score_of` | 167–176 | backend | none | none | score_of |
| 361 | `score_of` | 167–176 | score | score-10 | wrapper | score_of copies Path fields |
| 361 | `score_of` | 167–176 | screenB | screenB-9 | wrapper |  |
| 362 | `windows_for_path` | 179–196 | backend | backend-3 | none | copies sqout_tick across |
| 362 | `windows_for_path` | 179–196 | score | score-6 | none | windows_for_path reads stored deact/sqout ticks |
| 362 | `windows_for_path` | 179–196 | screenB | screenB-6 | wrapper |  |
| 362 | `windows_for_path` | 179–196 | spwin | spwin-10;spwin-16 | wrapper | reads deact_tick, sqout_tick and sqinouts (double read of squeeze-out) |
| 362 | `windows_for_path` | 179–196 | squeeze | squeeze-21 | none | reads both sqinouts SqOut offset and sqout_tick |
| 362 | `windows_for_path` | 179–196 | tempo | none | none | reads stored ticks |
| 363 | `replay_stored_path` | 199–206 | backend | none | wrapper | replay_stored_path |
| 363 | `replay_stored_path` | 199–206 | score | score-10 | wrapper | replay_stored_path |
| 363 | `replay_stored_path` | 199–206 | screenB | screenB-9 | wrapper |  |
| 363 | `replay_stored_path` | 199–206 | spwin | none | wrapper | replay_stored_path |
| 364 | `resolve_sqout_note` | 208–251 | backend | backend-14 | copies | cands.front() = engine's first SP chord; owner graph add_deact_edge |
| 364 | `resolve_sqout_note` | 208–251 | score | none | none | resolve_sqout_note (squeeze family) |
| 364 | `resolve_sqout_note` | 208–251 | screenB | screenB-27 | copies | uses sqout_candidates |
| 364 | `resolve_sqout_note` | 208–251 | spwin | spwin-13 | copies | nearest candidate, refuses non-first |
| 364 | `resolve_sqout_note` | 208–251 | squeeze | squeeze-1 | copies | nearest candidate + first-candidate rule (owner graph) |
| 364 | `resolve_sqout_note` | 208–251 | tempo | tempo-16 | wrapper | uses sqout_candidates |
| 365 | `ambiguous_window_warnings` | 255–296 | backend | backend-14;backend-6 | copies | cands.front(); reads in_sp |
| 365 | `ambiguous_window_warnings` | 255–296 | score | score-6 | none | ambiguous_window_warnings |
| 365 | `ambiguous_window_warnings` | 255–296 | screenB | screenB-27 | copies | uses sqout_candidates |
| 365 | `ambiguous_window_warnings` | 255–296 | spwin | spwin-13;spwin-11 | copies | uses sqout_candidates + in_sp |
| 365 | `ambiguous_window_warnings` | 255–296 | squeeze | squeeze-1 | copies | first candidate = engine's pick (owner graph) |
| 365 | `ambiguous_window_warnings` | 255–296 | tempo | tempo-1 | wrapper | Timecode ms gaps for text |

### `src/core/replay.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 366 | `ReplayScore::total` | 80–80 | score | score-10 | copies | ReplayScore::total re-sums six categories; owner Path::totalscore |
| 367 | `ReplayScore::add` | 81–88 | score | score-10 | none | ReplayScore::add |
| 368 | `ReplayScore::operator==` | 89–92 | sweep-core1 | none | none | category equality |
| 369 | `PathReplay::all_windows` | 202–202 | score | none | none | PathReplay::all_windows |
| 369 | `PathReplay::all_windows` | 202–202 | spwin | none | none | all_windows |
| 370 | `PathReplay::totals_match` | 204–204 | score | score-10 | none | PathReplay::totals_match compares replay to stored |
| 371 | `PathReplay::faithful` | 206–206 | score | score-10 | none | PathReplay::faithful |

### `src/core/rules.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 372 | `add_line` | 10–14 | store | store-14 | owns |  |
| 373 | `fnv1a64` | 16–23 | store | store-14 | owns |  |
| 374 | `fixed_cap_text` | 31–42 | backend | backend-16 | none | fingerprint text includes leeway |
| 374 | `fixed_cap_text` | 31–42 | fills | fills-35 | owns | fingerprint text |
| 374 | `fixed_cap_text` | 31–42 | store | store-14 | owns | field list |
| 375 | `hash_rules_text` | 45–48 | store | store-14 | owns |  |
| 376 | `Rules::fingerprint` | 52–52 | store | store-14 | owns |  |
| 377 | `Rules::retired_auto_fingerprint` | 54–60 | fills | fills-31 | owns | frozen Auto ladder literal |
| 377 | `Rules::retired_auto_fingerprint` | 54–60 | store | store-25 | owns | ladder literal |
| 378 | `default_rules` | 62–65 | store | none | none |  |
| 379 | `default_stamp` | 67–70 | store | store-14 | wrapper |  |

### `src/core/rules.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 380 | `RulesStamp::of` | 66–68 | store | store-14 | wrapper |  |
| 381 | `RulesStamp::none` | 69–69 | store | store-14 | wrapper |  |

### `src/core/scoring.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 382 | `category_scores` | 11–116 | backend | backend-5 | owns | sqout_reduction per sqout_rule |
| 382 | `category_scores` | 11–116 | score | score-1;score-3;score-4;score-5 | copies | category_scores re-derives note value inline (base+cymb+dyn_cymb, pad 50) beside basescore(); per-note sp computed twice in-function (accumulators and basescore*mult); owns combo-per-note and SP extra |

### `src/core/scoring.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 383 | `CategoryScores::sqout_sp` | 28–28 | backend | backend-5 | owns | sqout_sp = sp - sqout_reduction |
| 383 | `CategoryScores::sqout_sp` | 28–28 | score | score-5 | owns | CategoryScores::sqout_sp: sp - sqout_reduction |

### `src/core/squeeze_rating.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 384 | `transfer_scale_between` | 12–20 | display | none | none | scale math (squeeze family) |
| 384 | `transfer_scale_between` | 12–20 | screenA | screenA-23 | owns |  |
| 384 | `transfer_scale_between` | 12–20 | spwin | spwin-4 | none | measure-rate ratio |
| 384 | `transfer_scale_between` | 12–20 | squeeze | squeeze-13 | owns | transfer scale between two ticks |
| 384 | `transfer_scale_between` | 12–20 | tempo | tempo-27;tempo-8 | owns | ratio of ms_per_measure_at |
| 385 | `activation_deact_tick` | 22–24 | display | none | none | reads stored deact |
| 385 | `activation_deact_tick` | 22–24 | screenA | none | wrapper |  |
| 385 | `activation_deact_tick` | 22–24 | spwin | spwin-10 | wrapper | returns stored deact_tick |
| 385 | `activation_deact_tick` | 22–24 | squeeze | none | wrapper | hands back stored deact_tick |
| 386 | `frontend_transfer_scales` | 27–61 | display | none | none | scales |
| 386 | `frontend_transfer_scales` | 27–61 | screenA | screenA-23 | owns |  |
| 386 | `frontend_transfer_scales` | 27–61 | spwin | spwin-6;spwin-19;spwin-16;spwin-5 | copies | pre end = D - 1 bar re-derives the pre-SqIn end; anchors on act_tick even when clamp_tick set (owner of anchor: engine clamp_tick) |
| 386 | `frontend_transfer_scales` | 27–61 | squeeze | squeeze-12;squeeze-14;squeeze-26 | owns | re-derives pre tick as D-2 measures (engine knows it); anchors on act_tick even when clamp_tick set |
| 386 | `frontend_transfer_scales` | 27–61 | tempo | tempo-7;tempo-27 | copies | pre end = plusmeasure(D,-2) re-derives the pre-SqIn end the graph had; can drop a tick |
| 387 | `effective_backend_ms` | 63–65 | backend | backend-10 | owns | effective_backend_ms |
| 387 | `effective_backend_ms` | 63–65 | display | display-17 | owns | effective ms |
| 387 | `effective_backend_ms` | 63–65 | screenA | screenA-22 | owns |  |
| 387 | `effective_backend_ms` | 63–65 | squeeze | squeeze-15 | owns | effective ms |
| 387 | `effective_backend_ms` | 63–65 | tempo | none | none | effective ms |
| 388 | `squeeze_budget_ms` | 67–69 | backend | backend-10 | owns | squeeze_budget_ms |
| 388 | `squeeze_budget_ms` | 67–69 | display | display-17 | owns | budget |
| 388 | `squeeze_budget_ms` | 67–69 | screenA | screenA-26 | owns |  |
| 388 | `squeeze_budget_ms` | 67–69 | squeeze | squeeze-15 | owns | combined budget |
| 388 | `squeeze_budget_ms` | 67–69 | tempo | tempo-24 | none | budget from W |
| 389 | `transfer_is_material` | 72–77 | backend | backend-10 | owns | transfer_is_material |
| 389 | `transfer_is_material` | 72–77 | display | display-17 | owns | material rule (kTransferImpactMs) |
| 389 | `transfer_is_material` | 72–77 | screenA | screenA-22;screenA-23 | owns |  |
| 389 | `transfer_is_material` | 72–77 | squeeze | squeeze-16;squeeze-17 | owns | impact clause duplicated in rate_activation |
| 390 | `rate_activation` | 81–195 | backend | backend-1;backend-2;backend-10;backend-12 | owns | scale by offset sign: >0.0 / <0.0 re-encode paid_by_sp_walk; rows in [0,leeway) get no scale; skips unset offset |
| 390 | `rate_activation` | 81–195 | display | display-17 | copies | effective_ms > kTransferImpactMs gate written twice (backend rows and phrase notes) beside transfer_is_material's own > kTransferImpactMs |
| 390 | `rate_activation` | 81–195 | screenA | screenA-19;screenA-22;screenA-24;screenA-25 | copies | re-types transfer_is_material's 1 ms clause twice for effective_ms |
| 390 | `rate_activation` | 81–195 | spwin | spwin-6;spwin-15;spwin-16 | wrapper | reads stored clamp_tick/sqout via is_sqout_backend |
| 390 | `rate_activation` | 81–195 | squeeze | squeeze-10;squeeze-11;squeeze-12;squeeze-16;squeeze-17;squeeze-20;squeeze-25 | copies | impact check repeated twice (owner transfer_is_material); frontend-decided row decided twice in one function; direction ladder written twice |
| 390 | `rate_activation` | 81–195 | tempo | tempo-27 | none | reads stored scales |
| 391 | `timing_tiers` | 197–205 | display | display-5 | owns | tier ladder |
| 391 | `timing_tiers` | 197–205 | screenA | none | none |  |
| 391 | `timing_tiers` | 197–205 | screenB | screenB-13;screenB-15 | owns | Normal cutoff kDifficultMs (< 2.0) |
| 391 | `timing_tiers` | 197–205 | squeeze | squeeze-8;squeeze-7 | owns | tier ladder; Normal floor uses d<kDifficultMs (opposite edge to is_difficult) |
| 391 | `timing_tiers` | 197–205 | tempo | tempo-25;tempo-24 | owns | tier cutoffs from W |
| 392 | `beyond_edge_ms` | 207–212 | display | display-5 | owns | beyond edge |
| 392 | `beyond_edge_ms` | 207–212 | screenA | none | none |  |
| 392 | `beyond_edge_ms` | 207–212 | screenB | screenB-14 | owns |  |
| 392 | `beyond_edge_ms` | 207–212 | squeeze | squeeze-8 | wrapper | reads timing_tiers |
| 392 | `beyond_edge_ms` | 207–212 | tempo | tempo-25 | owns | beyond edge |

### `src/core/stars.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 393 | `star_cutoff` | 7–12 | display | none | none | star cutoff (engine fact) |
| 393 | `star_cutoff` | 7–12 | score | score-14 | owns | star_cutoff |
| 393 | `star_cutoff` | 7–12 | screenA | screenA-31 | owns |  |
| 394 | `star_cutoffs` | 14–21 | display | none | none | star cutoffs |
| 394 | `star_cutoffs` | 14–21 | score | score-14;score-11 | owns | star_cutoffs |
| 394 | `star_cutoffs` | 14–21 | screenA | screenA-30;screenA-31 | owns |  |
| 395 | `stars_for_score` | 23–29 | display | none | none | star count |
| 395 | `stars_for_score` | 23–29 | score | score-15 | owns | stars_for_score |
| 395 | `stars_for_score` | 23–29 | screenA | screenA-32 | owns |  |
| 396 | `path_stars` | 31–33 | display | none | none | path stars |
| 396 | `path_stars` | 31–33 | score | score-12;score-15 | owns | path_stars; subtracts solo itself (also in avg_mult) |
| 396 | `path_stars` | 31–33 | screenA | screenA-32 | owns |  |

### `src/core/strutil.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 397 | `lower_ascii` | 9–11 | sweep-core1 | sweep-core1-3 | owns | A-Z to a-z |
| 398 | `to_lower_ascii` | 15–19 | sweep-core1 | sweep-core1-3 | owns | ASCII lowercase |
| 399 | `trim` | 21–26 | sweep-core1 | sweep-core1-3 | owns | six-char whitespace trim |
| 400 | `ends_with` | 28–30 | sweep-core1 | sweep-core1-3 | owns | suffix test |
| 401 | `ends_with_ci` | 32–38 | sweep-core1 | sweep-core1-3 | owns | case-blind suffix test |

### `src/core/timing.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 402 | `to_multiplier` | 9–14 | score | score-2 | owns | to_multiplier |
| 402 | `to_multiplier` | 9–14 | tempo | none | none | combo multiplier, not time |
| 403 | `shown_multiplier` | 16–18 | backend | backend-6 | owns | shown_multiplier |
| 403 | `shown_multiplier` | 16–18 | score | score-4 | copies | shown_multiplier uses kStarPowerMultiplier=2, a parallel constant for the SP doubling category_scores pays implicitly |
| 403 | `shown_multiplier` | 16–18 | tempo | none | none | not time |
| 404 | `MsIndex::MsIndex` | 22–45 | tempo | tempo-1 | owns | builds tps and elapsed per tempo section |
| 405 | `MsIndex::at` | 47–54 | screenB | none | owns |  |
| 405 | `MsIndex::at` | 47–54 | spwin | none | none | MsIndex::at |
| 405 | `MsIndex::at` | 47–54 | tempo | tempo-1 | owns | tick to ms |
| 406 | `MsIndex::tps_at` | 56–61 | tempo | tempo-3 | owns | tempo in force at tick (upper_bound) |
| 407 | `MsIndex::ms_at_tick_f` | 63–73 | screenB | none | owns |  |
| 407 | `MsIndex::ms_at_tick_f` | 63–73 | tempo | tempo-1 | copies | sub-tick tick->ms, display-only twin of MsIndex::at; used by Ch10 deadline and span ends |
| 408 | `MsIndex::tick_at_ms` | 75–82 | screenB | screenB-8 | owns |  |
| 408 | `MsIndex::tick_at_ms` | 75–82 | tempo | tempo-2 | owns | continuous ms->tick |
| 409 | `MeasureIndex::MeasureIndex` | 86–107 | tempo | tempo-4 | owns | builds meter sections |
| 410 | `MeasureIndex::section_at` | 109–114 | tempo | tempo-4 | owns | meter section at tick (lower_bound, boundary tick reads the earlier section) |
| 411 | `Timecode::Timecode` | 120–141 | screenB | none | owns |  |
| 411 | `Timecode::Timecode` | 120–141 | tempo | tempo-6;tempo-9 | owns | measure.beat.tick and decimal measure; beat = tick_r |
| 412 | `SongTiming::SongTiming` | 148–148 | tempo | none | wrapper | constructs indexes |
| 413 | `bisect_left_lo` | 152–162 | tempo | tempo-7 | owns | helper of plusmeasure |
| 414 | `SongTiming::plusmeasure` | 165–195 | screenB | screenB-26 | owns |  |
| 414 | `SongTiming::plusmeasure` | 165–195 | spwin | spwin-4 | owns | plusmeasure |
| 414 | `SongTiming::plusmeasure` | 165–195 | tempo | tempo-7 | owns | N measures after a tick |
| 415 | `SongTiming::ms_per_measure_at` | 197–204 | screenB | screenB-4 | owns |  |
| 415 | `SongTiming::ms_per_measure_at` | 197–204 | spwin | spwin-4 | none | local measure ms |
| 415 | `SongTiming::ms_per_measure_at` | 197–204 | tempo | tempo-8;tempo-4 | owns | local ms per measure; re-picks meter with section_at(t+1) (right-continuous) |
| 416 | `SongTiming::measures_at_tick_f` | 206–215 | screenB | screenB-1 | owns |  |
| 416 | `SongTiming::measures_at_tick_f` | 206–215 | spwin | spwin-4 | copies | continuous measures position (display copy of plusmeasure's measure math) |
| 416 | `SongTiming::measures_at_tick_f` | 206–215 | tempo | tempo-6;tempo-4 | copies | continuous measure position; own right-continuous section scan; owner Timecode::Timecode |
| 417 | `SongTiming::tick_at_measures_f` | 217–232 | spwin | spwin-4 | copies | continuous inverse |
| 417 | `SongTiming::tick_at_measures_f` | 217–232 | tempo | tempo-7;tempo-4 | copies | inverse measure position; owner SongTiming::plusmeasure |
| 418 | `SongTiming::sp_end_ms` | 234–241 | screenB | screenB-5 | owns |  |
| 418 | `SongTiming::sp_end_ms` | 234–241 | spwin | spwin-4;spwin-3 | copies | exact sub-tick SP end (display); only tests call it |
| 418 | `SongTiming::sp_end_ms` | 234–241 | tempo | tempo-7 | copies | continuous SP end (act+N measures); owner SongTiming::plusmeasure; test-only caller |

### `src/core/timing.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 419 | `MeasureIndex::keys_at` | 73–73 | sweep-core1 | tempo-4 | none | accessor |
| 420 | `MeasureIndex::tpm_at` | 74–74 | sweep-core1 | tempo-4 | none | accessor |
| 421 | `MeasureIndex::starts_at` | 75–75 | sweep-core1 | tempo-4 | none | accessor |
| 422 | `MeasureIndex::measures_at` | 76–76 | sweep-core1 | tempo-4 | none | accessor |
| 423 | `MeasureIndex::count` | 77–77 | sweep-core1 | none | none | accessor |
| 424 | `Timecode::raw` | 100–104 | sweep-core1 | none | none | ticks-only Timecode |
| 425 | `Timecode::ticks` | 106–106 | sweep-core1 | none | none | accessor |
| 426 | `Timecode::measure_beats_ticks` | 108–108 | sweep-core1 | tempo-6 | wrapper | stored by Timecode::Timecode |
| 427 | `Timecode::measures_decimal` | 109–109 | sweep-core1 | tempo-6 | wrapper | stored by Timecode::Timecode |
| 428 | `Timecode::ms` | 110–110 | sweep-core1 | tempo-1 | wrapper | stored MsIndex::at value |
| 429 | `Timecode::operator<` | 112–112 | sweep-core1 | none | none | compare on ticks |
| 430 | `Timecode::operator<=` | 113–113 | sweep-core1 | none | none | compare on ticks |
| 431 | `Timecode::operator>` | 114–114 | sweep-core1 | none | none | compare on ticks |
| 432 | `Timecode::operator>=` | 115–115 | sweep-core1 | none | none | compare on ticks |
| 433 | `Timecode::operator==` | 116–116 | sweep-core1 | none | none | compare on ticks |
| 434 | `Timecode::operator!=` | 117–117 | sweep-core1 | none | none | compare on ticks |
| 435 | `sp_bars_to_measures` | 129–129 | spwin | spwin-4 | owns | 2 measures per bar |
| 436 | `SongTiming::tick_resolution` | 141–141 | sweep-core1 | tempo-9 | wrapper | ticks per beat |
| 437 | `SongTiming::measure_index` | 142–142 | sweep-core1 | none | none | accessor |
| 438 | `SongTiming::ms_index` | 143–143 | sweep-core1 | none | none | accessor |
| 439 | `SongTiming::timecode` | 145–147 | sweep-core1 | tempo-1;tempo-6 | wrapper | Timecode::Timecode |

### `src/core/winstr.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 440 | `utf8_to_wide` | 15–22 | sweep-core1 | sweep-core1-7 | owns | UTF-8 to UTF-16 |
| 441 | `wide_to_utf8` | 24–32 | sweep-core1 | sweep-core1-7 | owns | UTF-16 to UTF-8; tests/test_srb.cpp has its own WideCharToMultiByte copy |
| 442 | `fopen_utf8` | 34–36 | sweep-core1 | sweep-core1-7 | owns | _wfopen |
| 443 | `file_exists_utf8` | 38–41 | sweep-core1 | sweep-core1-7 | owns | GetFileAttributesW |
| 444 | `read_file_bytes` | 43–53 | sweep-core1 | none | none | file read |
| 445 | `read_file_text` | 55–58 | sweep-core1 | none | none | file read |
| 446 | `split_command_line_utf8` | 60–71 | sweep-core1 | none | none | argv split |
| 447 | `utf8_argv` | 73–75 | sweep-core1 | none | none | argv |

### `src/image/decode.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 448 | `decode_image` | 13–27 | sweep-media1 | none | none | stb_image wrapper, empty on failure |

### `src/image/decode.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 449 | `DecodedImage::empty` | 26–26 | sweep-media1 | none | none | plumbing |
| 450 | `decode_image` | 32–34 | sweep-media1 | none | wrapper | calls decode_image |

### `src/net/dmbot_client.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 451 | `Handle::Handle` | 31–31 | sweep-media1 | none | none | RAII |
| 452 | `Handle::~Handle` | 32–32 | sweep-media1 | none | none | RAII |
| 453 | `Handle::operator bool` | 35–35 | sweep-media1 | none | none | RAII |
| 454 | `fail` | 38–40 | sweep-media1 | display-28 | owns | formats " (error N)" that plain_error_text parses ("(error 12002)") |
| 455 | `cancelled` | 42–42 | sweep-media1 | none | none | cancel flag read |
| 456 | `AsyncState::~AsyncState` | 62–65 | sweep-media1 | none | none | RAII |
| 457 | `on_status` | 71–100 | sweep-media1 | none | none | WinHTTP callback plumbing |
| 458 | `AsyncRequest::~AsyncRequest` | 110–114 | sweep-media1 | none | none | RAII |
| 459 | `await_step` | 122–129 | sweep-media1 | display-28 | owns | throws literal "cancelled" (also JobCancelled in ui/job_base.h) |
| 460 | `http_get` | 134–228 | sweep-media1 | display-28;sweep-media1-9 | owns | throw-site literals + timeouts 15000/20000/30000/120000, status 200 |
| 461 | `jstr` | 232–236 | sweep-media1 | none | none | JSON reader |
| 462 | `jint` | 238–242 | sweep-media1 | sweep-media1-9 | owns | default argument used for speed=100 in parse_score |
| 463 | `joptint` | 244–248 | sweep-media1 | none | none | JSON reader |
| 464 | `join_charters` | 250–260 | sweep-media1 | sweep-media1-10 | owns | charter_refs joined with ", " |
| 465 | `parse_score` | 262–276 | parse | parse-8 | copies | lowercases leaderboard identifier |
| 466 | `parse_json` | 278–284 | sweep-media1 | display-28 | owns | throw-site literal |
| 467 | `parse_users_json` | 288–307 | sweep-media1 | display-28;sweep-media1-11 | owns | drops users with empty id; throw literal |
| 468 | `parse_scores_json` | 309–325 | sweep-media1 | sweep-media1-11 | owns | keeps scores and unknown_scores with non-empty identifier |
| 469 | `get` | 330–332 | sweep-media1 | none | none | fetcher seam |
| 470 | `set_fetcher` | 335–335 | sweep-media1 | none | none | fetcher seam |
| 471 | `fetch_users` | 337–339 | sweep-media1 | none | none | endpoint path |
| 472 | `fetch_scores` | 342–344 | sweep-media1 | none | none | endpoint path |

### `src/parse/chart_files.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 473 | `chart_format_of` | 9–15 | parse | parse-1 | owns | chart_format_of |
| 474 | `notes_file_format` | 17–22 | parse | parse-2 | owns | notes_file_format |
| 475 | `is_song_ini` | 24–24 | parse | parse-4 | owns | is_song_ini |

### `src/parse/midi.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 476 | `text_meta` | 14–23 | parse | none | none | midi text meta names |
| 477 | `name_meta` | 27–34 | parse | none | none | midi name meta names |
| 478 | `channel_data_len` | 37–43 | parse | none | none | channel data length |
| 479 | `be32` | 45–48 | parse | none | none | byte helper |
| 480 | `be16` | 50–52 | parse | none | none | byte helper |
| 481 | `decode_latin1` | 57–71 | parse | none | none | latin1 decode |
| 482 | `read_varlen` | 76–84 | parse | none | none | varlen |
| 483 | `read_message_length` | 93–99 | parse | none | none | message length cap (user decision 27 cited in comment) |
| 484 | `meta_message` | 103–137 | parse | none | none | meta message builder |
| 485 | `MidiFile::MidiFile` | 141–143 | parse | none | none | ctor |
| 486 | `MidiFile::MidiFile` | 145–147 | parse | none | none | ctor |
| 487 | `MidiFile::from_file` | 149–165 | parse | none | none | file open |
| 488 | `MidiFile::parse` | 167–202 | parse | none | none | MThd chunk walk |
| 489 | `MidiFile::parse_track` | 204–291 | parse | parse-9 | owns | first track_name wins |

### `src/parse/midi.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 490 | `MidiError::MidiError` | 30–30 | sweep-core1 | none | none | exception type |

### `src/parse/sng.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 491 | `fits` | 8–10 | parse | parse-24 | owns | bounds helper |
| 492 | `u64_at` | 12–16 | parse | parse-24 | owns | u64 |
| 493 | `u32_at` | 18–22 | parse | parse-24 | owns | u32 |
| 494 | `string_at` | 24–26 | parse | parse-24 | owns | string |
| 495 | `sng_read_metadata` | 30–52 | parse | parse-24 | owns | sng metadata pairs |
| 496 | `sng_read_file_table` | 54–79 | parse | parse-24 | owns | sng file table |
| 497 | `sng_decode_file` | 82–91 | parse | parse-24 | owns | sng XOR decode |

### `src/parse/song.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 498 | `split_ws` | 25–35 | parse | none | none | split helper |
| 499 | `split_char` | 37–50 | parse | none | none | split helper |
| 500 | `section_name_of` | 55–65 | parse | parse-28 | owns | section names |
| 501 | `try_parse_int` | 67–79 | parse | none | none | int helper |
| 502 | `re_dynamics` | 81–84 | parse | parse-17 | owns | ENABLE_CHART_DYNAMICS regex |
| 503 | `re_disco_on` | 85–88 | parse | parse-14 | owns | disco on regex hard-codes mix 3 (Expert) for every difficulty |
| 504 | `re_disco_off` | 89–92 | parse | parse-14 | owns | disco off regex hard-codes mix 3 |
| 505 | `full_match` | 94–96 | parse | none | none | regex helper |
| 506 | `fill_lands_on_chord` | 104–111 | fills | fills-4 | owns | landing rule; shared by both parsers |
| 506 | `fill_lands_on_chord` | 104–111 | parse | none | none | fill placement (fill family) |
| 506 | `fill_lands_on_chord` | 104–111 | tempo | tempo-9 | copies | beat length = res * slop; no beat owner |
| 507 | `apply_timesig` | 118–122 | parse | none | none | timesig (timing family) |
| 507 | `apply_timesig` | 118–122 | tempo | tempo-10;tempo-4 | owns | ticks per measure from a time signature |
| 508 | `apply_fill_end` | 126–131 | fills | fills-6 | owns | fill length and inside-fill check |
| 508 | `apply_fill_end` | 126–131 | parse | none | none | fill end |
| 509 | `mark_sp_phrase_end` | 135–142 | parse | none | none | SP phrase end |
| 509 | `mark_sp_phrase_end` | 135–142 | spwin | none | none | marks phrase end |
| 510 | `emit_chord_timestamp` | 149–157 | parse | parse-15;parse-20 | owns | disco flip only when pro; flam only on .mid |
| 511 | `difficulty_name` | 161–168 | parse | parse-19 | owns | difficulty names |
| 512 | `difficulty_from_name` | 170–181 | parse | parse-19 | owns | name lookup |
| 513 | `no_notes_message` | 183–186 | parse | parse-19 | owns | no-notes wording |
| 514 | `title_or_unknown` | 188–193 | parse | parse-7 | owns | title fallback |
| 515 | `Song::sp_phrase_count` | 197–202 | parse | none | none | SP phrase count |
| 515 | `Song::sp_phrase_count` | 197–202 | spwin | spwin-7 | owns | phrase count |
| 516 | `Song::check_activations` | 206–286 | fills | fills-1;fills-2;fills-3 | copies | owns fills-1/2; its SongIter tpm walk copies the meter-at-tick question owned by SongTiming/MeasureIndex and lags one change per chord |
| 516 | `Song::check_activations` | 206–286 | parse | none | none | fill synthesis (fill family) |
| 516 | `Song::check_activations` | 206–286 | tempo | tempo-4;tempo-3;tempo-9;tempo-7 | copies | own tpm/bpm walk for meter before chord (owner MeasureIndex::section_at); bpm walk unused; beat = res |
| 517 | `difficulty_base_pitch` | 302–309 | parse | parse-10;parse-19 | owns | pitch base per difficulty |
| 517 | `difficulty_base_pitch` | 302–309 | screenA | screenA-34 | owns |  |
| 518 | `is_handled_note` | 315–327 | parse | parse-10;parse-13 | owns | accepts 95 at every difficulty |
| 518 | `is_handled_note` | 315–327 | screenA | screenA-34 | owns | 95 accepted on every difficulty |
| 519 | `MidiParser::MidiParser` | 331–331 | parse | none | none | ctor |
| 520 | `MidiParser::op_enable_dynamics` | 342–342 | parse | parse-17 | owns | dynamics flag set from the event tick on |
| 521 | `MidiParser::op_disco` | 343–343 | parse | parse-14 | owns | disco state |
| 522 | `MidiParser::op_tempo` | 344–346 | parse | none | none | tempo |
| 522 | `MidiParser::op_tempo` | 344–346 | tempo | tempo-10 | owns | MIDI tempo to BPM |
| 523 | `MidiParser::op_timesig` | 347–349 | parse | none | none | timesig |
| 524 | `MidiParser::op_fillstart` | 350–353 | parse | none | none | fill |
| 525 | `MidiParser::op_store_fillend` | 354–354 | parse | none | none | fill |
| 526 | `MidiParser::op_apply_fill` | 355–355 | parse | none | none | fill |
| 527 | `MidiParser::op_sp_start` | 356–356 | parse | none | none | SP |
| 528 | `MidiParser::op_sp_end` | 357–361 | parse | none | none | SP |
| 529 | `MidiParser::op_tom` | 362–364 | parse | parse-11 | owns | tom marker span state |
| 530 | `MidiParser::op_flam` | 365–365 | parse | parse-20 | owns | flam state |
| 531 | `MidiParser::op_solo` | 366–366 | parse | none | none | solo |
| 532 | `MidiParser::op_note` | 367–373 | parse | parse-11;parse-16;parse-17 | owns | cymbal only if pro and allows_cymbals; per-note dynamics gate |
| 532 | `MidiParser::op_note` | 367–373 | screenA | screenA-34;screenA-37 | owns |  |
| 533 | `MidiParser::optype` | 392–519 | fills | fills-5 | none | maps MIDI 120 on/off to fill start/end ops |
| 533 | `MidiParser::optype` | 392–519 | parse | parse-10;parse-11;parse-13;parse-16 | owns | pitch->lane, velocity->dynamic, 95->2x kick (bass2x only) |
| 533 | `MidiParser::optype` | 392–519 | score | score-26 | owns | MidiParser::optype velocity 127/1 rule for pads and kicks |
| 533 | `MidiParser::optype` | 392–519 | screenA | screenA-34 | owns | case 95 gated only by mode_bass2x_ |
| 533 | `MidiParser::optype` | 392–519 | tempo | tempo-10 | wrapper | routes tempo/timesig metas |
| 534 | `MidiParser::run_ops` | 521–528 | parse | none | none | run ops |
| 534 | `MidiParser::run_ops` | 521–528 | screenA | screenA-34 | owns | duplicate notes silently dropped, first wins |
| 535 | `MidiParser::push_timestamp` | 530–577 | fills | fills-5;fills-4 | copies | attach gate duplicated with ChartParser::push_timestamp; calls fill_lands_on_chord |
| 535 | `MidiParser::push_timestamp` | 530–577 | parse | none | none | per-tick ops |
| 535 | `MidiParser::push_timestamp` | 530–577 | tempo | none | none | fill placement order |
| 536 | `MidiParser::parse` | 580–641 | parse | parse-9;parse-17 | owns | PART DRUMS exact; dynamics_enabled from end state |
| 536 | `MidiParser::parse` | 580–641 | tempo | tempo-10 | owns | tempo map from track 0 then build_timing |
| 537 | `ChartDataEntry::is_tick_data` | 676–676 | parse | none | none | helper |
| 538 | `ChartDataEntry::ChartDataEntry` | 680–733 | parse | parse-14 | owns | .chart disco only when E token unquoted single word |
| 538 | `ChartDataEntry::ChartDataEntry` | 680–733 | tempo | tempo-10 | owns | .chart B and TS parse |
| 539 | `ChartSection::add` | 741–750 | parse | none | none | section add |
| 540 | `ChartParser::ChartParser` | 762–762 | parse | none | none | ctor |
| 541 | `ChartParser::op_disco` | 773–773 | parse | parse-14 | owns | disco state |
| 542 | `ChartParser::op_tempo` | 774–774 | parse | none | none | tempo |
| 542 | `ChartParser::op_tempo` | 774–774 | tempo | tempo-10 | owns | .chart tempo store |
| 543 | `ChartParser::op_timesig` | 775–777 | parse | none | none | timesig |
| 544 | `ChartParser::op_fillstart` | 778–781 | parse | none | none | fill |
| 545 | `ChartParser::op_fillend` | 782–782 | parse | none | none | fill |
| 546 | `ChartParser::op_sp_start` | 783–786 | parse | none | none | SP |
| 547 | `ChartParser::op_sp_end` | 787–790 | parse | none | none | SP |
| 548 | `ChartParser::op_solo` | 791–791 | parse | none | none | solo |
| 549 | `ChartParser::op_note` | 792–792 | parse | parse-10 | owns | note add |
| 550 | `ChartParser::op_2x` | 793–793 | parse | parse-13 | owns | 2x add |
| 551 | `ChartParser::op_accent` | 794–794 | parse | parse-16 | owns | accent |
| 552 | `ChartParser::op_ghost` | 795–795 | parse | parse-16 | owns | ghost |
| 553 | `ChartParser::op_cymbal` | 796–796 | parse | parse-11 | owns | cymbal |
| 554 | `ChartParser::load_sections` | 813–854 | parse | none | none | section reader |
| 555 | `ChartParser::optype` | 856–921 | fills | fills-5 | none | maps .chart S 64 phrase to fill start/end |
| 555 | `ChartParser::optype` | 856–921 | parse | parse-10;parse-11;parse-13;parse-16 | owns | .chart note numbers -> lane; N 5 dropped; 66-68 pro only |
| 555 | `ChartParser::optype` | 856–921 | screenA | screenA-34 | owns |  |
| 555 | `ChartParser::optype` | 856–921 | tempo | tempo-10 | wrapper | routes B/TS to op_* |
| 556 | `ChartParser::push_timestamp` | 924–974 | fills | fills-5;fills-4 | copies | attach gate duplicated with MidiParser::push_timestamp; calls fill_lands_on_chord |
| 556 | `ChartParser::push_timestamp` | 924–974 | parse | none | none | per-tick ops |
| 556 | `ChartParser::push_timestamp` | 924–974 | tempo | none | none | phrase/fill ordering |
| 557 | `ChartParser::parse` | 977–1062 | parse | parse-17;parse-19;parse-26;parse-27 | owns | section by difficulty name; dynamics always on; Resolution required; Offset |
| 557 | `ChartParser::parse` | 977–1062 | tempo | tempo-10;tempo-11 | owns | Resolution and Offset read |
| 558 | `load_songbytes_mid` | 1069–1072 | parse | none | wrapper | loader |
| 559 | `load_songbytes_chart` | 1075–1077 | parse | none | wrapper | loader |
| 560 | `load_songpath_mid` | 1080–1083 | parse | none | wrapper | loader |
| 561 | `load_songpath_chart` | 1086–1089 | parse | none | wrapper | loader |
| 562 | `load_songpath_sng` | 1092–1119 | parse | parse-2;parse-3 | copies | re-decides mid-over-chart in its own loop; owner of folder rule is discover_charts |
| 563 | `load_songpath_srb` | 1122–1148 | parse | parse-3 | copies | srb notes entry by extension + MThd sniff; differs from sng exact-name rule |
| 564 | `load_songpath` | 1151–1160 | parse | parse-1 | wrapper | dispatch via chart_format_of |

### `src/parse/song.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 565 | `SongTimestamp::has_activation` | 76–76 | parse | none | none | accessor |
| 566 | `Song::Song` | 91–94 | parse | none | none | ctor |
| 566 | `Song::Song` | 91–94 | tempo | tempo-4 | owns | default 4/4 meter at tick 0 |
| 567 | `Song::tick_resolution` | 96–96 | parse | none | none | accessor |
| 568 | `Song::build_timing` | 121–123 | parse | none | none | timing |
| 569 | `Song::timing` | 124–124 | parse | none | none | timing |
| 570 | `Song::timecode` | 125–125 | parse | none | none | timing |
| 570 | `Song::timecode` | 125–125 | tempo | tempo-1 | wrapper | Song::timecode |
| 571 | `Song::start_time` | 126–126 | parse | none | none | timing |
| 571 | `Song::start_time` | 126–126 | tempo | tempo-1 | wrapper | Song::start_time |
| 572 | `Song::is_empty` | 128–128 | parse | none | none | accessor |

### `src/parse/srb.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 573 | `srb_inflate_stream` | 11–49 | parse | parse-25 | owns | srb inflate |
| 574 | `srb_parse_metadata` | 51–70 | parse | parse-25 | owns | srb metadata |

### `src/render/highway_draw.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 575 | `texture_file` | 11–45 | sweep-media1 | none | none | texture file names (asset manifest) |
| 576 | `make_camera` | 47–61 | sweep-media1 | sweep-media1-13 | owns | camera view/projection |
| 577 | `track_height` | 63–68 | sweep-media1 | sweep-media1-14 | owns | track rect height min(h, w*ratio) >= 1 |
| 578 | `time_to_z` | 70–74 | sweep-media1 | sweep-media1-15 | owns | time->depth; far time = now + speed*secs_future |
| 579 | `z_to_time` | 76–80 | sweep-media1 | sweep-media1-15 | copies | re-types far_time = now + speed*secs_future (owner time_to_z) |
| 580 | `pad_x` | 82–86 | sweep-media1 | sweep-media1-16 | owns | lane x extent = note area / 4 |
| 581 | `stretch_matrix` | 88–97 | sweep-media1 | none | none | matrix plumbing |
| 582 | `light_for` | 99–107 | sweep-media1 | sweep-media1-17 | owns | gem light at box top centre (preview_config.h comment says bottom centre) |
| 583 | `flat` | 112–122 | sweep-media1 | none | none | draw command builder |
| 584 | `color_mat` | 124–129 | sweep-media1 | none | none | material builder |
| 585 | `tex_mat` | 131–136 | sweep-media1 | none | none | material builder |
| 586 | `overlay_mat` | 138–144 | sweep-media1 | none | none | material builder |
| 587 | `toggle_on` | 146–146 | sweep-media1 | sweep-media1-18 | copies | on = not Empty and not End; same rule as make_toggle_bounds after-instant state (owner TrackState::make_toggle_bounds) |
| 588 | `lane_tex` | 155–163 | sweep-media1 | none | none | texture table |
| 589 | `target_tex` | 165–173 | sweep-media1 | none | none | texture table |
| 590 | `gem_tex` | 176–195 | sweep-media1 | sweep-media1-19 | owns | energy texture for notes in an SP phrase (od flag) |
| 591 | `blend` | 197–199 | sweep-media1 | none | none | colour average |
| 592 | `build_highway_draws` | 204–381 | sweep-media1 | screenB-3;screenB-10;display-24;sweep-media1-15;sweep-media1-19;sweep-media1-20;sweep-media1-21;sweep-media1-22 | copies | far_t re-typed (time_to_z); hit-at-playhead t<now twice vs build_score_box s.ms<=now; fill lane pad re-picked from window instants (owner TrackState::synthesize); SP floor reads sp_active (wrapper) |

### `src/render/obj_loader.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 593 | `tokens_of` | 18–29 | sweep-media1 | none | none | tokenizer |
| 594 | `to_float` | 31–33 | sweep-media1 | none | none | number parse |
| 595 | `to_long` | 35–38 | sweep-media1 | none | none | number parse |
| 596 | `parse_corner` | 41–57 | sweep-media1 | none | none | obj corner parse |
| 597 | `resolve` | 60–65 | sweep-media1 | none | none | obj index resolve |
| 598 | `load_obj` | 69–125 | sweep-media1 | sweep-media1-23 | owns | fan triangulation (Onyx triangulate) |
| 599 | `sort_far_first` | 127–142 | sweep-media1 | none | none | triangle order by z sum |
| 600 | `vtx` | 146–157 | sweep-media1 | none | none | vertex builder |
| 601 | `push_quad` | 161–168 | sweep-media1 | sweep-media1-23 | copies | fan order re-typed ("matching load_obj's order"), owner load_obj |
| 602 | `make_flat_quad` | 172–178 | sweep-media1 | none | none | unit quad |
| 603 | `make_box` | 180–196 | sweep-media1 | none | none | unit box |
| 604 | `mesh_bounds` | 198–207 | sweep-media1 | none | none | bounds, tests only |

### `src/render/obj_loader.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 605 | `ObjMesh::triangle_count` | 28–28 | sweep-media1 | none | none | count |

### `src/render/overlay_layout.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 606 | `project_to_image` | 17–29 | sweep-media1 | sweep-media1-14 | owns | aspect w/th and bottom anchor (h - th); PreviewRenderer::render re-codes both (fc.rect_max, viewport, make_camera aspect) |
| 607 | `highway_span_at` | 31–47 | sweep-media1 | sweep-media1-24 | owns | railing outer edge on screen; 1e-3f epsilon |
| 608 | `overlay_scale` | 50–65 | sweep-media1 | sweep-media1-25 | owns | box scale clamp [kOverlayMinScale 0.6, 1] |
| 609 | `bottom_left_room` | 67–69 | sweep-media1 | sweep-media1-24 | wrapper | calls highway_span_at at bottom row |
| 610 | `tail_start` | 74–84 | sweep-media1 | sweep-media1-26 | owns | kept tail start |
| 611 | `wrap_words` | 89–116 | sweep-media1 | sweep-media1-26 | owns | word wrap with kept tail |
| 612 | `widest_word` | 119–133 | sweep-media1 | sweep-media1-26 | copies | re-walks words and trims tail itself to state the narrowest wrap_words result (owner wrap_words/tail_start) |
| 613 | `ellipsize` | 136–159 | sweep-media1 | sweep-media1-27 | owns | UTF-8 safe ellipsis cut |

### `src/render/preview_config.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 614 | `parse_hex_color` | 12–28 | sweep-media1 | sweep-media1-28 | owns | hex colour, magenta fallback |
| 615 | `get_f` | 33–35 | sweep-media1 | none | none | json reader |
| 616 | `get_i` | 36–38 | sweep-media1 | none | none | json reader |
| 617 | `get_color` | 39–42 | sweep-media1 | none | none | json reader |
| 618 | `get_vec3` | 43–49 | sweep-media1 | none | none | json reader |
| 619 | `get_light` | 50–57 | sweep-media1 | none | none | json reader |
| 620 | `sub` | 58–62 | sweep-media1 | none | none | json reader |
| 621 | `load_preview_config` | 66–127 | sweep-media1 | sweep-media1-29;display-28 | owns | reads 3d-config.json; header defaults are a second copy of every value |

### `src/render/preview_renderer.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 622 | `check` | 33–35 | sweep-media1 | display-28 | owns | throw prefix "PreviewRenderer: " |
| 623 | `asset_bytes` | 40–46 | sweep-media1 | none | none | file read |
| 624 | `asset_text` | 48–51 | sweep-media1 | none | none | file read |
| 625 | `f4` | 89–89 | sweep-media1 | none | none | colour convert |
| 626 | `compile` | 92–103 | sweep-media1 | display-28 | owns | throw "PreviewRenderer: shader" (maps to generic text) |
| 627 | `PreviewRenderer::upload` | 142–153 | sweep-media1 | none | none | GPU upload |
| 628 | `PreviewRenderer::load_texture` | 157–184 | sweep-media1 | display-28;sweep-media1-30 | owns | missing/undecodable texture literals; rows flipped so v=0 is bottom |
| 629 | `PreviewRenderer::load_model` | 186–190 | sweep-media1 | display-28 | owns | missing model literal |
| 630 | `PreviewRenderer::create_targets` | 192–235 | sweep-media1 | sweep-media1-14 | wrapper | scene target height = track_h; msaa fallback max(1) |
| 631 | `PreviewRenderer::update_cb` | 238–243 | sweep-media1 | none | none | cbuffer upload |
| 632 | `PreviewRenderer::draw_command` | 245–278 | sweep-media1 | sweep-media1-31 | owns | Onyx specular 0.5 / shininess 32 literals, diffuse_type 1/2/3 mirrors shader |
| 633 | `PreviewRenderer::PreviewRenderer` | 283–379 | sweep-media1 | display-28 | owns | missing 3d-config / shader file literals |
| 634 | `PreviewRenderer::~PreviewRenderer` | 381–381 | sweep-media1 | none | none | teardown |
| 635 | `PreviewRenderer::resize` | 383–389 | sweep-media1 | sweep-media1-14 | wrapper | calls track_height; max(1) clamp also inside track_height |
| 636 | `PreviewRenderer::set_scene` | 391–393 | sweep-media1 | none | wrapper | calls build_track_state |
| 637 | `PreviewRenderer::render` | 395–482 | tempo | tempo-28 | copies | now_ms / 1000 |
| 638 | `PreviewRenderer::texture_srv` | 484–484 | sweep-media1 | none | none | getter |
| 639 | `PreviewRenderer::width` | 485–485 | sweep-media1 | none | none | getter |
| 640 | `PreviewRenderer::height` | 486–486 | sweep-media1 | none | none | getter |
| 641 | `PreviewRenderer::config` | 487–487 | sweep-media1 | none | none | getter |

### `src/render/track_state.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 642 | `s_of` | 25–25 | display | none | none | unit |
| 642 | `s_of` | 25–25 | parse | none | none | seconds |
| 642 | `s_of` | 25–25 | screenB | none | none |  |
| 643 | `pad_of` | 27–36 | display | none | none | pad map |
| 643 | `pad_of` | 27–36 | parse | none | none | lane->pad |
| 644 | `gem_of` | 38–48 | display | none | none | gem map |
| 644 | `gem_of` | 38–48 | parse | parse-11;parse-12 | copies | cymbal = pro && n.cymbal && lane != Red re-decides pro gate and red-no-cymbal; owner parser + allows_cymbals |
| 644 | `gem_of` | 38–48 | screenB | none | none |  |
| 645 | `toggle_at` | 52–66 | display | display-24 | owns | toggle rule |
| 645 | `toggle_at` | 52–66 | screenB | screenB-3 | owns | interval toggle |
| 646 | `build_track_state` | 68–141 | display | display-24;display-25 | copies | SP-active intervals [a.ms, sp_end]; taken fill by f.span.end_tick == a.tick (also in build_preview_scene) |
| 646 | `build_track_state` | 68–141 | fills | fills-20 | none | draws the states it is given |
| 646 | `build_track_state` | 68–141 | parse | none | none | scene to track state (calls gem_of) |
| 646 | `build_track_state` | 68–141 | screenB | screenB-3;screenB-10 | copies | SP-active interval and taken-fill lookup (end_tick == act tick) repeated from build_preview_scene |
| 646 | `build_track_state` | 68–141 | spwin | spwin-21;spwin-11 | copies | sp_active_ interval [act ms, sp_end_ms) |
| 646 | `build_track_state` | 68–141 | tempo | tempo-1;tempo-22;tempo-23 | copies | span end = ms_at_tick_f(end_tick + 0.5); sp_active_ interval from a.ms..sp_end_ms |
| 647 | `TrackState::synthesize` | 143–171 | display | display-24 | wrapper | toggle_at |
| 647 | `TrackState::synthesize` | 143–171 | screenB | screenB-3 | wrapper |  |
| 648 | `TrackState::window` | 173–185 | sweep-media1 | sweep-media1-32 | owns | visible instants near < t < far, midpoint synth |
| 649 | `TrackState::make_toggle_bounds` | 189–212 | sweep-media1 | sweep-media1-18 | owns | span state entering (End/Restart/On) and after each instant (Start/Restart/On) |

### `src/render/track_state.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 650 | `TrackWindow::TrackWindow` | 71–71 | sweep-media1 | none | none | view ctor |
| 651 | `TrackWindow::TrackWindow` | 72–72 | sweep-media1 | none | none | view ctor |
| 652 | `TrackWindow::begin` | 74–74 | sweep-media1 | none | none | iterator |
| 653 | `TrackWindow::end` | 75–75 | sweep-media1 | none | none | iterator |
| 654 | `TrackWindow::rbegin` | 76–78 | sweep-media1 | none | none | iterator |
| 655 | `TrackWindow::rend` | 79–81 | sweep-media1 | none | none | iterator |
| 656 | `TrackWindow::size` | 82–82 | sweep-media1 | none | none | size |
| 657 | `TrackWindow::empty` | 83–83 | sweep-media1 | none | none | empty |
| 658 | `TrackWindow::front` | 84–84 | sweep-media1 | none | none | front |
| 659 | `TrackState::instants` | 97–97 | sweep-media1 | none | none | getter |

### `src/search/engine.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 660 | `has_value` | 28–28 | spwin | none | none | NaN helper |
| 661 | `enumerate` | 71–155 | spwin | none | none | graph enumeration |
| 662 | `StampMap::reset` | 244–258 | sweep-core1 | none | none | hash map reset |
| 663 | `StampMap::get_or_insert` | 259–275 | sweep-core1 | none | none | hash map probe |
| 664 | `StampMap::hash` | 278–283 | sweep-core1 | none | none | splitmix64 mixer |
| 665 | `Engine::Engine` | 311–311 | backend | backend-16 | wrapper | stores leeway |
| 666 | `Engine::set_progress_cb` | 317–317 | sweep-core1 | none | none | setter |
| 667 | `Engine::out_paths` | 319–319 | sweep-core1 | none | none | accessor |
| 668 | `Engine::out_acts` | 320–320 | sweep-core1 | none | none | accessor |
| 669 | `Engine::out_sqs` | 321–321 | sweep-core1 | none | none | accessor |
| 670 | `Engine::out_cols` | 322–322 | sweep-core1 | none | none | accessor |
| 671 | `Engine::node` | 325–325 | sweep-core1 | none | none | accessor |
| 672 | `Engine::edge` | 326–326 | sweep-core1 | none | none | accessor |
| 673 | `Engine::eobj` | 327–327 | spwin | none | none | edge accessor |
| 674 | `Engine::new_act` | 330–344 | spwin | spwin-6 | none | new Act starts clamp_tick NO_TIME |
| 675 | `Engine::clone_tail` | 345–349 | sweep-core1 | none | none | copy tail act |
| 676 | `Engine::push_sq` | 350–357 | sweep-core1 | none | none | push SqIn/SqOut record |
| 677 | `Engine::push_col` | 358–361 | sweep-core1 | spwin-17 | wrapper | stores a collected phrase tick |
| 678 | `Engine::trim_cols` | 364–368 | spwin | spwin-15;spwin-17 | copies | tick >= sqinout_time is a copy of sqout_position Exact\|After (owner core::sqout_position) |
| 679 | `Engine::act_count` | 369–371 | sweep-core1 | none | none | act depth |
| 680 | `Engine::emit_cols` | 391–398 | spwin | spwin-17 | none | emits collected chain |
| 681 | `Engine::advance` | 460–528 | fills | fills-17;fills-18 | copies | old_sp<2 && sp>=2 restates the 2-bar threshold; owns sp_ready_ms |
| 681 | `Engine::advance` | 460–528 | score | score-10;score-6 | copies | Engine::advance keeps p.score as a second running sum beside p.sc[]; owner Path::totalscore |
| 681 | `Engine::advance` | 460–528 | spwin | spwin-1;spwin-2;spwin-5;spwin-6;spwin-7;spwin-17 | owns | SP-track extension + clamp_tick + collected phrases; base-track bar count with cap clamp; old_sp<2 literal |
| 681 | `Engine::advance` | 460–528 | squeeze | none | none | SP extension bookkeeping, late SqIn buffered |
| 681 | `Engine::advance` | 460–528 | tempo | tempo-1 | wrapper | sp_ready_ms from Timecode |
| 682 | `Engine::branch_activate` | 531–585 | fills | fills-12;fills-17;fills-19 | copies | owns summonable check and skip charge; p.sp<2 restates the 2-bar threshold |
| 682 | `Engine::branch_activate` | 531–585 | score | score-10;score-6 | copies | Engine::branch_activate adds frontend to sc[2] and p.score separately |
| 682 | `Engine::branch_activate` | 531–585 | screenB | screenB-10 | owns | skip only on real opportunity |
| 682 | `Engine::branch_activate` | 531–585 | spwin | spwin-2;spwin-3 | copies | p.sp < 2 literal (no owner); reads activation_initial_end_times |
| 682 | `Engine::branch_activate` | 531–585 | tempo | tempo-18 | copies | e_offset < -kEarlyFillWindowMs legality; is_e0 owns the inside edge |
| 683 | `Engine::deactivation_type` | 587–592 | backend | backend-14 | none | deact type from sqout_time |
| 683 | `Engine::deactivation_type` | 587–592 | spwin | spwin-10 | owns | sp_end_time == dest tick decides deactivation |
| 683 | `Engine::deactivation_type` | 587–592 | squeeze | squeeze-1 | none | sp_end vs sqout_time decides SQINOUT |
| 684 | `Engine::create_deactivated_path` | 595–645 | backend | backend-1;backend-3;backend-4;backend-12 | wrapper | calls sqout_position, paid_by_sp_walk, backend_row_value; value_or(0.0) fallback |
| 684 | `Engine::create_deactivated_path` | 595–645 | score | score-6;score-7;score-10 | copies | Engine::create_deactivated_path backend pricing via backend_row_value; adds to sc[2] and p.score separately |
| 684 | `Engine::create_deactivated_path` | 595–645 | screenA | screenA-18 | owns | engine prices rows via sqout_position |
| 684 | `Engine::create_deactivated_path` | 595–645 | spwin | spwin-18;spwin-15;spwin-17 | owns | c.sp = is_sq_out ? 1 : 0; trims collected; prices rows through backend_row_value |
| 684 | `Engine::create_deactivated_path` | 595–645 | squeeze | squeeze-22;squeeze-24 | wrapper | calls sqout_position/backend_row_value |
| 684 | `Engine::create_deactivated_path` | 595–645 | tempo | none | none | reads stored offsets |
| 685 | `Engine::branch_deactivate` | 648–678 | backend | backend-14 | none | SqOut child always made with SqIn parent |
| 685 | `Engine::branch_deactivate` | 648–678 | spwin | spwin-13;spwin-19 | owns | SqIn/SqOut branch: sqin_time becomes sp end |
| 685 | `Engine::branch_deactivate` | 648–678 | squeeze | squeeze-1 | none | takes SqIn branch, sets sp_end to sqin_time |
| 686 | `Engine::act_difficulty` | 681–695 | fills | fills-15;fills-14;fills-37 | copies | activation difficulty loop duplicates Activation::difficulty |
| 686 | `Engine::act_difficulty` | 681–695 | squeeze | squeeze-5 | copies | own max over squeezes+E0 (owner Activation::difficulty) |
| 687 | `Engine::close_last_activation` | 697–702 | fills | fills-16 | copies | running max of activation difficulty; parallel to Path::difficulty |
| 687 | `Engine::close_last_activation` | 697–702 | squeeze | squeeze-6 | copies | running max (owner Path::difficulty) |
| 688 | `Engine::search_difficulty` | 704–709 | fills | fills-16 | copies | parallel to Path::difficulty |
| 688 | `Engine::search_difficulty` | 704–709 | squeeze | squeeze-6 | copies | path max (owner Path::difficulty) |
| 689 | `Engine::passes_ms_filter` | 711–715 | fills | fills-28 | owns | d <= ms_filter |
| 689 | `Engine::passes_ms_filter` | 711–715 | squeeze | squeeze-28 | owns | d <= ms_filter |
| 690 | `Engine::reduce_group` | 718–843 | fills | fills-21;fills-22;fills-23;fills-24 | owns | tie fold and depth band; engine tied_count parallel to Path::recount_tied_paths |
| 690 | `Engine::reduce_group` | 718–843 | score | score-21 | owns | Engine::reduce_group score band; Scores rule decided twice (fast path and outscored_by loop) |
| 691 | `Engine::reduce_iteration_paths` | 846–938 | fills | fills-21 | owns | grouping key omits sp_ready_ms (assumption) |
| 691 | `Engine::reduce_iteration_paths` | 846–938 | score | score-19;score-21 | owns | Engine::reduce_iteration_paths optimal_score_ |
| 692 | `Engine::emit_acts` | 943–991 | spwin | spwin-22;spwin-6 | owns | final_sp_end for still-running activation |
| 693 | `Engine::emit_variant` | 993–1012 | score | score-10 | none | Engine::emit_variant leaves totals to prepare_variants |
| 694 | `Engine::emit_path` | 1014–1031 | score | score-10 | none | Engine::emit_path copies sc[] to OutPath |
| 695 | `Engine::run` | 1034–1146 | fills | fills-19;fills-29;fills-30 | owns | no_skips and target branches |
| 695 | `Engine::run` | 1034–1146 | spwin | none | none | BFS driver |
| 696 | `rebuild` | 1164–1314 | backend | backend-3;backend-7 | copies | trim lambda ticks > sqout_tick copies sqout_position After; tail offset b.ms - end_ms copies add_deact_edge offset rule |
| 696 | `rebuild` | 1164–1314 | spwin | spwin-10;spwin-12;spwin-15;spwin-16;spwin-22 | owns | stamps deact_tick/sqout_tick/clamp_tick/collected; tail-backend offset is a copy of add_deact_edge formula; remove_if ticks > sqout_tick copies sqout_position After |
| 696 | `rebuild` | 1164–1314 | squeeze | squeeze-2;squeeze-22 | copies | tail offset formula (owner add_deact_edge); trims rows past sqout (owner sqout_position); stamps deact/clamp/sqout/transfer |
| 696 | `rebuild` | 1164–1314 | tempo | tempo-15 | copies | tail offset = b.ms - at(final_sp_end); owner add_deact_edge |
| 697 | `run_search` | 1319–1335 | backend | backend-16 | wrapper | passes graph.rules().backend_leeway_ms |
| 697 | `run_search` | 1319–1335 | spwin | none | none | run_search entry |

### `src/search/graph.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 698 | `TickGreater::operator` | 18–20 | spwin | none | none | heap comparator |
| 699 | `ScoreGraph::new_node` | 24–30 | sweep-core1 | none | none | pool alloc |
| 700 | `ScoreGraph::new_edge` | 32–35 | sweep-core1 | none | none | pool alloc |
| 701 | `ScoreGraph::plusmeasure` | 37–44 | spwin | spwin-4 | wrapper | caches SongTiming::plusmeasure |
| 701 | `ScoreGraph::plusmeasure` | 37–44 | tempo | tempo-7 | wrapper | plusmeasure cache |
| 702 | `activation_fill_deadline_ms` | 49–74 | fills | fills-8 | owns | CH 1.1 4-beat and CH 1.0 clamp/pad deadline |
| 702 | `activation_fill_deadline_ms` | 49–74 | screenB | screenB-10 | owns |  |
| 702 | `activation_fill_deadline_ms` | 49–74 | spwin | none | none | fill deadline, not SP window |
| 702 | `activation_fill_deadline_ms` | 49–74 | tempo | tempo-17;tempo-9;tempo-1 | owns | fill deadline; Ch10 uses ms_at_tick_f off the scoring surface |
| 703 | `ScoreGraph::ScoreGraph` | 78–87 | spwin | spwin-7 | wrapper | reads song.sp_phrase_count for max_sp_bars |
| 704 | `ScoreGraph::build` | 89–190 | backend | backend-5;backend-8 | wrapper | feeds sg.sp / sg.sqout_sp() to store_new_backend; sqout_deacts window |
| 704 | `ScoreGraph::build` | 89–190 | fills | fills-7 | owns | reads has_activation to make activation edges |
| 704 | `ScoreGraph::build` | 89–190 | score | score-3;score-8;score-6 | copies | ScoreGraph::build: combo_ += count (also in replay_path), solo bonus kSoloBonusPerNote*count (also in replay_path) |
| 704 | `ScoreGraph::build` | 89–190 | spwin | spwin-5;spwin-13;spwin-14;spwin-11 | owns | build loop: pending deacts, phrase extension dispatch, sqout_deacts filter (line 131) duplicates is_recent_to_head window test |
| 704 | `ScoreGraph::build` | 89–190 | squeeze | squeeze-3 | copies | inline D - phrase < kSqueezeWindowMs (owner none; is_recent_to_head form) |
| 704 | `ScoreGraph::build` | 89–190 | tempo | tempo-16;tempo-15 | copies | sqout_deacts: D.ms - phrase.ms < kSqueezeWindowMs |
| 705 | `ScoreGraph::store_notecount` | 192–195 | spwin | none | none | store_notecount |
| 706 | `ScoreGraph::store_soloscore` | 196–199 | score | score-8 | none | store_soloscore accumulates |
| 707 | `ScoreGraph::store_basescore` | 200–203 | score | score-10 | none | store_basescore accumulates |
| 708 | `ScoreGraph::store_comboscore` | 204–207 | score | score-10 | none | store_comboscore accumulates |
| 709 | `ScoreGraph::store_spscore` | 208–210 | score | score-6 | none | store_spscore accumulates SP track only |
| 709 | `ScoreGraph::store_spscore` | 208–210 | spwin | spwin-11 | owns | SP points only on SP track: structural SP-walk payment |
| 710 | `ScoreGraph::store_accentscore` | 211–214 | score | score-10 | none | store_accentscore |
| 711 | `ScoreGraph::store_ghostscore` | 215–218 | score | score-10 | none | store_ghostscore |
| 712 | `ScoreGraph::store_multsqueeze` | 219–221 | score | score-17 | none | store_multsqueeze |
| 713 | `ScoreGraph::store_new_backend` | 224–249 | backend | backend-7;backend-14 | owns | offset = ts.ms - dest.ms (post-D rows); first SP row after D becomes sqinout |
| 713 | `ScoreGraph::store_new_backend` | 224–249 | score | score-6;score-7 | owns | store_new_backend: points=sg.sp, sqout_points=sg.sqout_sp |
| 713 | `ScoreGraph::store_new_backend` | 224–249 | screenB | screenB-2 | owns | late SqIn: sqin_time + 1 bar |
| 713 | `ScoreGraph::store_new_backend` | 224–249 | spwin | spwin-12;spwin-13;spwin-5 | copies | late-SqIn: offset formula copy of add_deact_edge; first-SP-chord rule half; sqin_time +1 bar without cap ceiling (owner extend_deacts) |
| 713 | `ScoreGraph::store_new_backend` | 224–249 | squeeze | squeeze-1;squeeze-2 | copies | second copy of sqinout detection and offset (paired with add_deact_edge) |
| 713 | `ScoreGraph::store_new_backend` | 224–249 | tempo | tempo-15;tempo-16 | copies | offset = ts.ms - D.ms; owner add_deact_edge |
| 714 | `ScoreGraph::max_sp_bars` | 251–254 | fills | none | none | cap vs phrase count clamp (SP family) |
| 714 | `ScoreGraph::max_sp_bars` | 251–254 | screenB | screenB-1 | owns |  |
| 714 | `ScoreGraph::max_sp_bars` | 251–254 | spwin | spwin-7 | owns | min(cap, phrase count) |
| 715 | `ScoreGraph::extend_deacts` | 257–278 | screenB | screenB-2 | owns |  |
| 715 | `ScoreGraph::extend_deacts` | 257–278 | spwin | spwin-5;spwin-6;spwin-23 | owns | +2 measures or cap ceiling, clamped flag, tie -> no clamp |
| 715 | `ScoreGraph::extend_deacts` | 257–278 | squeeze | none | none | cap clamp for normal extension |
| 715 | `ScoreGraph::extend_deacts` | 257–278 | tempo | tempo-7 | wrapper | extension via plusmeasure |
| 716 | `ScoreGraph::is_recent_to_head` | 280–282 | backend | backend-8 | owns | head - tc < kSqueezeWindowMs |
| 716 | `ScoreGraph::is_recent_to_head` | 280–282 | screenB | screenB-27 | owns |  |
| 716 | `ScoreGraph::is_recent_to_head` | 280–282 | spwin | spwin-14 | owns | head - tc < kSqueezeWindowMs |
| 716 | `ScoreGraph::is_recent_to_head` | 280–282 | squeeze | squeeze-3 | owns | window membership |
| 716 | `ScoreGraph::is_recent_to_head` | 280–282 | tempo | tempo-16 | copies | head.ms - tc.ms < kSqueezeWindowMs |
| 717 | `ScoreGraph::set_head_time` | 284–297 | backend | backend-8 | wrapper | prunes via is_recent_to_head |
| 717 | `ScoreGraph::set_head_time` | 284–297 | spwin | spwin-14 | wrapper | prunes recent edges/backends via is_recent_to_head |
| 717 | `ScoreGraph::set_head_time` | 284–297 | squeeze | squeeze-3 | wrapper | calls is_recent_to_head |
| 717 | `ScoreGraph::set_head_time` | 284–297 | tempo | tempo-16 | wrapper | filters by is_recent_to_head |
| 718 | `ScoreGraph::handle_deact` | 300–306 | spwin | spwin-10 | owns | deact handled only if still pending |
| 719 | `ScoreGraph::advance_tracks` | 309–325 | score | none | none | advance_tracks |
| 719 | `ScoreGraph::advance_tracks` | 309–325 | spwin | none | none | advance tracks |
| 720 | `ScoreGraph::add_act_edge` | 327–344 | fills | fills-8;fills-17 | copies | calls deadline owner; meter map from 2 bars restates the 2-bar threshold also in Engine::branch_activate/advance |
| 720 | `ScoreGraph::add_act_edge` | 327–344 | score | score-6 | owns | add_act_edge: frontend_points = activation chord sp |
| 720 | `ScoreGraph::add_act_edge` | 327–344 | screenB | screenB-26 | owns |  |
| 720 | `ScoreGraph::add_act_edge` | 327–344 | spwin | spwin-2;spwin-3 | owns | initial ends for sp=2..max_sp_bars; literal 2 = minimum bars (also in Engine) |
| 720 | `ScoreGraph::add_act_edge` | 327–344 | tempo | tempo-7 | wrapper | act + 2B measures via plusmeasure |
| 721 | `ScoreGraph::add_deact_edge` | 346–370 | backend | backend-7;backend-14 | copies | offset = row.ms - dest.ms written again (pre-D rows); owner store_new_backend/add_deact_edge pair |
| 721 | `ScoreGraph::add_deact_edge` | 346–370 | score | score-6;score-7 | owns | add_deact_edge: backend rows offsets |
| 721 | `ScoreGraph::add_deact_edge` | 346–370 | screenB | screenB-27 | owns |  |
| 721 | `ScoreGraph::add_deact_edge` | 346–370 | spwin | spwin-12;spwin-13;spwin-5 | copies | offset owner; first SP chord rule owner (pre-D half); sqout_time/sqin_time = dest +1 bar without cap ceiling (owner extend_deacts) |
| 721 | `ScoreGraph::add_deact_edge` | 346–370 | squeeze | squeeze-1;squeeze-2 | owns | first SP note before D, offset |
| 721 | `ScoreGraph::add_deact_edge` | 346–370 | tempo | tempo-15;tempo-7 | owns | backend offset_ms = ts.ms - dest.ms; sqin/sqout +2 measures |

### `src/search/graph.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 722 | `engine_mode_stamp` | 41–43 | fills | fills-11 | owns | engine_mode_stamp |
| 722 | `engine_mode_stamp` | 41–43 | store | store-10 | owns | stamp spelling |
| 723 | `ScoreGraph::start` | 113–113 | sweep-core1 | none | none | accessor |
| 724 | `ScoreGraph::sp_meter_cap` | 114–114 | sweep-core1 | screenB-12 | none | accessor |
| 725 | `ScoreGraph::rules` | 115–115 | sweep-core1 | none | none | accessor |
| 726 | `ScoreGraph::timing` | 116–116 | sweep-core1 | none | none | accessor |
| 727 | `ScoreGraph::multsqueezes` | 119–119 | sweep-core1 | none | none | accessor |
| 728 | `ScoreGraph::tail_backends` | 125–127 | backend | backend-8 | none | tail_backends accessor |
| 728 | `ScoreGraph::tail_backends` | 125–127 | squeeze | squeeze-3 | none | tail backends |
| 729 | `ScoreGraph::head_time_offset` | 156–158 | backend | backend-8 | owns | head_time_offset |
| 729 | `ScoreGraph::head_time_offset` | 156–158 | squeeze | squeeze-3 | none | head offset |
| 729 | `ScoreGraph::head_time_offset` | 156–158 | tempo | tempo-16 | owns | head offset ms |

### `src/search/pather.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 730 | `read` | 15–25 | fills | fills-24 | wrapper | passes depth to engine |
| 730 | `read` | 15–25 | score | score-21 | wrapper | pather read passes depth options |
| 731 | `std::function<void` | 34–37 | sweep-core1 | none | none | progress rescale |
| 732 | `attach_allzero` | 42–58 | display | none | none | all-0 gate (search family) |
| 732 | `attach_allzero` | 42–58 | fills | fills-28 | copies | difficulty <= 0.0 restates search_allzero's 0 ms limit |
| 732 | `attach_allzero` | 42–58 | score | score-20 | none | attach_allzero skip when optimal is all-0 with no timing |
| 733 | `search_allzero` | 63–93 | display | none | none | all-0 search |
| 733 | `search_allzero` | 63–93 | fills | fills-28;fills-24 | owns | 0 ms hard filter, no_skips |
| 733 | `search_allzero` | 63–93 | score | score-21 | none | search_allzero depth 0 |
| 734 | `search_target` | 96–137 | fills | fills-9;fills-30 | copies | legacy flag -> FillDeadlineRule mapping duplicated with analyze_at_cap |
| 734 | `search_target` | 96–137 | score | score-21 | none | search_target widest Points band |
| 734 | `search_target` | 96–137 | store | none | none |  |
| 735 | `graph_build_cap` | 139–141 | fills | none | none | graph height (SP family) |
| 735 | `graph_build_cap` | 139–141 | spwin | spwin-7 | copies | min(cap, max(phrases,1)) duplicates max_sp_bars |
| 736 | `analyze_chart` | 178–195 | fills | fills-10 | wrapper | stamps record.legacy_fills from settings |
| 736 | `analyze_chart` | 178–195 | spwin | spwin-7 | wrapper | calls graph_build_cap |
| 736 | `analyze_chart` | 178–195 | store | store-12;store-13;store-14 | owns | stamps record fields |

### `src/store/path_codec.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 737 | `rotl64` | 17–19 | store | none | none | hash helper |
| 738 | `fmix64` | 21–28 | store | none | none | hash helper |
| 739 | `getblock64` | 30–34 | store | none | none | hash block load; LE read for hashing only |
| 740 | `murmur3_x64_128` | 37–98 | store | store-24 | owns | MurmurHash3 |
| 741 | `write_activation` | 104–140 | backend | backend-8 | wrapper | stores display_backends only |
| 741 | `write_activation` | 104–140 | spwin | spwin-14 | wrapper | stores display_backends + SP ticks |
| 741 | `write_activation` | 104–140 | store | store-19 | owns | activation writer |
| 742 | `read_activation` | 142–183 | backend | none | none | reads backend rows |
| 742 | `read_activation` | 142–183 | spwin | none | none | decodes SP ticks |
| 742 | `read_activation` | 142–183 | store | store-19 | owns | activation reader |
| 743 | `write_root_totals` | 188–197 | store | store-19 | owns | root totals writer |
| 744 | `read_root_totals` | 199–208 | store | store-19 | owns | root totals reader |
| 745 | `write_tree_entry` | 215–231 | store | store-19;store-24 | owns | tree entry writer |
| 746 | `read_tree_entry` | 233–251 | store | store-19 | owns | tree entry reader |
| 747 | `encode_path_node` | 257–263 | store | store-19 | owns | node writer stamps kPathFormatStamp.written |
| 748 | `decode_path_node` | 265–278 | store | store-1;store-19 | wrapper | format check via StampRule::is_current |
| 749 | `path_hash_bytes` | 282–286 | store | store-24 | owns |  |
| 750 | `hash_to_hex` | 288–297 | store | none | none | hex render |
| 751 | `path_hash` | 299–301 | store | store-24 | wrapper | path_hash_bytes |
| 752 | `flatten_record` | 305–338 | store | store-5;store-12;store-19 | owns | writes 12-byte head and blob copies of ms_limit/sp_cap |
| 753 | `rebuild_record` | 341–385 | store | store-1;store-5;store-19 | owns | reads head; format check via StampRule |
| 754 | `rebuild_record` | 387–399 | store | store-19 | wrapper | rebuild_record |

### `src/store/record_store.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 755 | `Stmt::Stmt` | 25–25 | store | none | none |  |
| 756 | `Stmt::~Stmt` | 36–38 | store | none | none |  |
| 757 | `ResetOnExit::~ResetOnExit` | 50–52 | store | none | none |  |
| 758 | `prepare` | 55–61 | store | none | none |  |
| 759 | `bind_text` | 63–65 | store | none | none |  |
| 760 | `bind_blob` | 66–68 | store | none | none |  |
| 761 | `column_text` | 69–74 | store | none | none |  |
| 762 | `column_blob` | 75–81 | store | none | none |  |
| 763 | `column_opt_i64` | 82–85 | store | none | none |  |
| 764 | `column_opt_f64` | 86–89 | store | none | none |  |
| 765 | `encode_tempomap` | 93–107 | store | store-18 | owns |  |
| 766 | `decode_tempomap` | 109–128 | store | store-18 | owns |  |
| 766 | `decode_tempomap` | 109–128 | tempo | tempo-1;tempo-4 | wrapper | rebuilds SongTiming from stored maps |
| 767 | `bind_summary` | 132–153 | store | store-22 | none | binds summary columns |
| 768 | `read_summary` | 155–168 | score | score-15 | none | read_summary reads stored columns |
| 768 | `read_summary` | 155–168 | store | store-22 | none | reads summary columns |
| 769 | `sort_column_name` | 170–186 | store | none | none |  |
| 770 | `sort_column_is_songmeta` | 187–190 | store | none | none |  |
| 771 | `lens_match` | 247–251 | store | store-7 | owns | SQL lens identity |
| 772 | `bind_lens` | 252–259 | store | store-7 | owns |  |
| 773 | `read_le` | 265–269 | store | store-6 | copies | LE read; owner BinaryReader |
| 774 | `write_le` | 270–274 | store | store-6 | copies | LE write; owner BinaryWriter |
| 775 | `layout_is_current` | 277–280 | store | store-1;store-5 | copies | head offset 0..4 hard-coded; owner flatten_record layout |
| 776 | `structure_is_current` | 286–290 | store | store-1;store-5 | copies | head offset 4..12 hard-coded; owner flatten_record layout |
| 777 | `Candidate::ready` | 304–304 | store | store-1 | owns |  |
| 778 | `rank_row` | 308–311 | store | store-1 | owns |  |
| 779 | `stale_reasons` | 324–330 | store | store-1;store-2 | copies | re-composes readiness pieces; owner rank_row |
| 780 | `placeholders` | 343–347 | store | none | none |  |
| 781 | `row_ready_sql` | 349–355 | store | store-1;store-5 | copies | SQL twin of rank_row; head offsets as substr; owner rank_row/flatten_record |
| 782 | `bind_ready_params` | 357–362 | store | store-1 | copies | binds the SQL twin; owner rank_row |
| 783 | `outranks` | 370–374 | store | store-3 | owns |  |
| 784 | `WinnerPicker::offer` | 387–392 | store | store-3 | owns |  |
| 785 | `WinnerPicker::winners` | 394–398 | store | store-3 | owns |  |
| 786 | `WinnerPicker::only_winner` | 401–404 | store | store-3 | owns |  |
| 787 | `WinnerPicker::rank` | 405–405 | store | store-3 | owns |  |
| 788 | `append_candidate_filter` | 417–421 | store | store-7;store-8 | owns | candidate filter |
| 789 | `bind_candidate_filter` | 423–427 | store | store-7;store-8 | owns |  |
| 790 | `rollback_if_open` | 432–434 | store | none | none |  |
| 791 | `analyzed_filter` | 442–447 | store | store-4 | owns |  |
| 792 | `bind_analyzed_filter` | 450–456 | store | store-4 | owns |  |
| 793 | `song_length_ms` | 462–465 | display | display-35 | owns | song length |
| 793 | `song_length_ms` | 462–465 | parse | parse-21 | owns | last note onset |
| 793 | `song_length_ms` | 462–465 | store | store-17 | owns |  |
| 793 | `song_length_ms` | 462–465 | tempo | tempo-12 | owns | last note onset ms |
| 794 | `summarize_path` | 467–495 | display | display-7 | copies | own max loop over Activation::difficulty instead of Path::difficulty |
| 794 | `summarize_path` | 467–495 | fills | fills-16 | copies | own max loop over a.difficulty() instead of Path::difficulty |
| 794 | `summarize_path` | 467–495 | score | score-10;score-13;score-15 | wrapper | summarize_path stores score/avgmult/stars from owners |
| 794 | `summarize_path` | 467–495 | screenA | screenA-4;screenA-32 | copies | hardest_ms re-loops activations instead of Path::difficulty |
| 794 | `summarize_path` | 467–495 | screenB | screenB-16 | copies | own max loop over Activation::difficulty; owner Path::difficulty |
| 794 | `summarize_path` | 467–495 | squeeze | squeeze-6;squeeze-27 | copies | own hardest loop (owner Path::difficulty) |
| 794 | `summarize_path` | 467–495 | store | store-22 | owns |  |
| 795 | `summarize_record` | 497–505 | display | display-30 | copies | pathcount = sum tied_pathcount; Paths kept uses all_paths().size() |
| 795 | `summarize_record` | 497–505 | score | score-15 | wrapper | summarize_record |
| 795 | `summarize_record` | 497–505 | screenA | none | wrapper |  |
| 795 | `summarize_record` | 497–505 | screenB | screenB-18 | wrapper | best_path |
| 795 | `summarize_record` | 497–505 | squeeze | none | wrapper | summarize_path(best) |
| 795 | `summarize_record` | 497–505 | store | store-22 | owns |  |
| 796 | `current_record_version` | 507–507 | store | store-1 | wrapper | kResultsStamp.written |
| 797 | `prepare_row` | 509–544 | store | store-7;store-8;store-12;store-13 | owns | cross-checks key vs record; ms only checked when lens ms on |
| 798 | `RecordStore::RecordStore` | 549–628 | store | store-27 | owns | schema setup and migrations |
| 799 | `RecordStore::add_fill_rule_column` | 630–650 | fills | fills-10;fills-11 | copies | literal "ch10" instead of engine_mode_stamp |
| 799 | `RecordStore::add_fill_rule_column` | 630–650 | store | store-10;store-27 | copies | interprets engine_mode stamp; no single owner |
| 800 | `RecordStore::~RecordStore` | 652–652 | store | none | none |  |
| 801 | `RecordStore::close` | 654–659 | store | none | none |  |
| 802 | `RecordStore::exec` | 661–668 | store | none | none |  |
| 803 | `RecordStore::has_column` | 670–676 | store | none | none |  |
| 804 | `RecordStore::meta_get` | 678–683 | store | none | none |  |
| 805 | `RecordStore::meta_set` | 685–691 | store | none | none |  |
| 806 | `RecordStore::engine_mode` | 693–696 | fills | fills-10 | none | meta getter |
| 806 | `RecordStore::engine_mode` | 693–696 | store | store-10 | wrapper | meta read |
| 807 | `RecordStore::set_engine_mode` | 698–701 | fills | fills-10 | none | meta setter |
| 807 | `RecordStore::set_engine_mode` | 698–701 | store | store-10 | wrapper | meta write |
| 808 | `RecordStore::put_dynamics` | 704–707 | store | store-20 | wrapper |  |
| 809 | `RecordStore::insert_dynamics` | 710–721 | store | store-20 | none | row write |
| 810 | `RecordStore::save_analysis` | 726–751 | parse | parse-21;parse-22 | none | writes songmeta names/length via upsert_song |
| 810 | `RecordStore::save_analysis` | 726–751 | store | store-17 | wrapper | song_length_ms |
| 811 | `RecordStore::get_dynamics` | 753–765 | store | store-20 | owns | count stamp check |
| 812 | `RecordStore::create_result_tables` | 767–791 | store | store-7 | owns | UNIQUE key columns |
| 813 | `RecordStore::load_nodes` | 805–814 | store | none | none |  |
| 814 | `RecordStore::load_nodes` | 817–820 | store | none | none |  |
| 815 | `RecordStore::reload_row` | 823–839 | store | store-7 | copies | row identity check omits lens; owner RecordKey::operator== |
| 816 | `RecordStore::add_song` | 843–849 | store | store-17 | wrapper | song_length_ms |
| 817 | `RecordStore::set_song_length` | 851–859 | store | store-17 | none | fills NULL length |
| 818 | `RecordStore::upsert_song` | 864–886 | parse | parse-21;parse-22 | copies | length per difficulty stored under hyhash; names from the analyzed copy |
| 818 | `RecordStore::upsert_song` | 864–886 | store | store-17 | owns | latest-non-null length wins |
| 819 | `RecordStore::add_record` | 888–890 | store | none | wrapper |  |
| 820 | `RecordStore::add_row` | 892–902 | store | none | wrapper |  |
| 821 | `RecordStore::write_row` | 904–1006 | store | store-21 | owns | purge policy |
| 822 | `RecordStore::collect_orphan_paths` | 1009–1021 | store | none | none |  |
| 823 | `RecordStore::delete_auto_results` | 1028–1071 | fills | fills-31 | none | deletes Auto rows by retired fingerprint |
| 823 | `RecordStore::delete_auto_results` | 1028–1071 | store | store-5;store-25 | copies | substr(structure,5,8) head offset; owner flatten_record layout |
| 824 | `RecordStore::get_summary` | 1073–1075 | store | store-1 | wrapper |  |
| 825 | `RecordStore::get_summaries` | 1079–1142 | store | store-1;store-3 | wrapper |  |
| 826 | `RecordStore::get_record` | 1144–1220 | store | store-1;store-2;store-13 | owns | sets record.legacy_fills from key |
| 827 | `RecordStore::read_tempomap` | 1222–1228 | store | store-18 | none |  |
| 828 | `RecordStore::get_timing` | 1230–1235 | store | store-18 | wrapper |  |
| 829 | `RecordStore::has_record` | 1237–1245 | store | store-4 | wrapper |  |
| 830 | `RecordStore::analyzed_hashes` | 1249–1257 | store | store-4 | wrapper |  |
| 831 | `RecordStore::for_each_blob` | 1262–1414 | store | store-1;store-3;store-13 | copies | decodes record without restoring legacy_fills; owner get_record |
| 832 | `RecordStore::reindex` | 1416–1469 | score | score-15 | wrapper | RecordStore::reindex recomputes summaries |
| 832 | `RecordStore::reindex` | 1416–1469 | store | store-22 | wrapper | summarize_record |
| 833 | `RecordStore::fill_missing_stars` | 1471–1529 | score | score-15 | wrapper | RecordStore::fill_missing_stars backfills stars column |
| 833 | `RecordStore::fill_missing_stars` | 1471–1529 | store | store-22 | wrapper | summarize_record |
| 834 | `RecordStore::list_records` | 1533–1591 | store | store-3 | wrapper |  |
| 835 | `RecordStore::counts` | 1593–1604 | store | none | none |  |
| 836 | `RecordStore::rebuild_chart_library` | 1608–1647 | parse | parse-22 | owns | names a duplicated md5 by first listed copy |
| 836 | `RecordStore::rebuild_chart_library` | 1608–1647 | store | none | none |  |
| 837 | `RecordStore::chart_library_cache` | 1649–1668 | parse | none | none | rescan cache |
| 837 | `RecordStore::chart_library_cache` | 1649–1668 | store | none | none |  |
| 838 | `RecordStore::chart_library_count` | 1670–1684 | store | none | none |  |
| 839 | `RecordStore::list_chart_library` | 1687–1718 | store | none | none |  |

### `src/store/record_store.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 840 | `CapQuery::at` | 78–78 | store | store-8 | owns |  |
| 841 | `CapQuery::operator==` | 80–80 | store | store-8 | wrapper |  |
| 842 | `CapQuery::operator!=` | 81–81 | store | store-8 | wrapper |  |
| 843 | `Lens::from` | 105–113 | store | store-9 | owns | ms off canonical 0; depth_mode copied raw |
| 844 | `Lens::operator==` | 116–120 | store | store-7 | owns |  |
| 845 | `Lens::operator!=` | 121–121 | store | store-7 | wrapper |  |
| 846 | `RecordKey::operator==` | 131–134 | store | store-7 | owns |  |
| 847 | `RecordKey::operator!=` | 135–135 | store | store-7 | wrapper |  |

### `src/store/serialize.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 848 | `BinaryWriter::u32` | 9–11 | store | store-6 | owns |  |
| 849 | `BinaryWriter::u64` | 12–14 | store | store-6 | owns |  |
| 850 | `BinaryWriter::f64` | 15–19 | store | store-6 | owns |  |
| 851 | `BinaryWriter::str` | 20–23 | store | store-6 | owns |  |
| 852 | `BinaryWriter::opt_i32` | 24–27 | store | store-6 | owns |  |
| 853 | `BinaryWriter::opt_i64` | 28–31 | store | store-6 | owns |  |
| 854 | `BinaryWriter::opt_f64` | 32–35 | store | store-6 | owns |  |
| 855 | `BinaryWriter::opt_str` | 36–39 | store | store-6 | owns |  |
| 856 | `BinaryReader::need` | 41–43 | store | store-6 | owns |  |
| 857 | `BinaryReader::u8` | 44–47 | store | store-6 | owns |  |
| 858 | `BinaryReader::u32` | 48–53 | store | store-6 | owns |  |
| 859 | `BinaryReader::u64` | 54–59 | store | store-6 | owns |  |
| 860 | `BinaryReader::f64` | 60–65 | store | store-6 | owns |  |
| 861 | `BinaryReader::str` | 66–72 | store | store-6 | owns |  |
| 862 | `BinaryReader::opt_i32` | 73–76 | store | store-6 | owns |  |
| 863 | `BinaryReader::opt_i64` | 77–80 | store | store-6 | owns |  |
| 864 | `BinaryReader::opt_f64` | 81–84 | store | store-6 | owns |  |
| 865 | `BinaryReader::opt_str` | 85–88 | store | store-6 | owns |  |
| 866 | `restore_activation` | 92–95 | store | store-18 | none |  |
| 867 | `restore_path` | 97–100 | store | store-18 | none |  |
| 868 | `restore_timecodes` | 104–113 | store | store-18 | none |  |

### `src/store/serialize.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 869 | `SerializeError::SerializeError` | 24–24 | store | none | none |  |
| 870 | `BinaryWriter::u8` | 34–34 | store | store-6 | owns |  |
| 871 | `BinaryWriter::boolean` | 35–35 | store | store-6 | owns |  |
| 872 | `BinaryWriter::i32` | 37–37 | store | store-6 | owns |  |
| 873 | `BinaryWriter::i64` | 39–39 | store | store-6 | owns |  |
| 874 | `BinaryReader::BinaryReader` | 51–51 | store | none | none |  |
| 875 | `BinaryReader::boolean` | 54–54 | store | store-6 | owns |  |
| 876 | `BinaryReader::i32` | 56–56 | store | store-6 | owns |  |
| 877 | `BinaryReader::i64` | 58–58 | store | store-6 | owns |  |

### `src/store/stored_versions.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 878 | `StampRule::is_current` | 29–33 | store | store-1;store-20 | owns | StampRule::is_current |

### `src/ui/activation_row_layout.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 879 | `activation_row_layout` | 33–42 | sweep-ui1 | sweep-ui1-15 | owns |  |

### `src/ui/app_shell.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 880 | `read_int_pair` | 37–47 | sweep-ui1 | none | none |  |
| 881 | `placement_read_open` | 52–56 | sweep-ui1 | none | none |  |
| 882 | `placement_read_line` | 58–63 | sweep-ui1 | none | none |  |
| 883 | `placement_write_all` | 65–74 | sweep-ui1 | none | none |  |
| 884 | `placement_on_screen` | 78–87 | sweep-ui1 | sweep-ui1-11 | owns |  |
| 885 | `format_window_placement` | 89–93 | sweep-ui1 | sweep-ui1-21 | owns | writer half of the [Hydra][Window] format |
| 886 | `parse_window_placement_line` | 95–109 | sweep-ui1 | sweep-ui1-21 | owns | reader half of the [Hydra][Window] format |
| 887 | `window_placement` | 111–111 | sweep-ui1 | none | none |  |
| 888 | `remember_window_placement` | 113–119 | sweep-ui1 | none | none |  |
| 889 | `format_layout` | 121–126 | sweep-ui1 | sweep-ui1-22 | owns | writer half of the [Hydra][Layout] format |
| 890 | `parse_layout_line` | 128–144 | sweep-ui1 | sweep-ui1-22;sweep-ui1-8 | copies | owns sweep-ui1-22; the (0,1) share test is also typed in remember_library_share (no owner) |
| 891 | `library_share` | 146–146 | sweep-ui1 | none | none |  |
| 892 | `remember_library_share` | 148–152 | sweep-ui1 | sweep-ui1-8 | copies | same (0,1) share test as parse_layout_line; no owner |
| 893 | `library_hidden` | 154–154 | sweep-ui1 | none | none |  |
| 894 | `remember_library_hidden` | 156–160 | sweep-ui1 | none | none |  |
| 895 | `ui_scale_for_dpi` | 162–164 | sweep-ui1 | sweep-ui1-7 | owns | main.cpp startup uses ImGui_ImplWin32_GetDpiScaleForMonitor/ForHwnd instead |
| 896 | `scaled_style` | 166–171 | sweep-ui1 | sweep-ui1-23 | owns |  |
| 897 | `set_ui_scale` | 173–181 | sweep-ui1 | sweep-ui1-23 | wrapper | scaled_style + g_ui_scale |
| 898 | `setup_imgui` | 183–264 | sweep-ui1 | sweep-ui1-9;sweep-ui1-10 | copies | resource dir exe_dir()+resource also typed in load_icons; 18.0f font size typed three times in this function |
| 899 | `shutdown_imgui` | 266–266 | sweep-ui1 | none | none |  |
| 900 | `run_frame` | 268–294 | sweep-ui1 | none | none |  |

### `src/ui/app_shell.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 901 | `ScreenRect::width` | 43–43 | sweep-ui1 | none | none |  |
| 902 | `ScreenRect::height` | 44–44 | sweep-ui1 | none | none |  |
| 903 | `ScreenRect::operator==` | 46–48 | sweep-ui1 | none | none |  |
| 904 | `ScreenRect::operator!=` | 49–49 | sweep-ui1 | none | none |  |
| 905 | `WindowPlacement::operator==` | 57–59 | sweep-ui1 | none | none |  |
| 906 | `WindowPlacement::operator!=` | 60–60 | sweep-ui1 | none | none |  |

### `src/ui/app_state.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 907 | `load_startup_settings` | 19–27 | store | store-15 | wrapper |  |
| 908 | `AppState::AppState` | 31–31 | sweep-ui1 | none | none |  |
| 909 | `AppState::AppState` | 40–42 | store | store-14 | wrapper | RulesStamp::of / none |
| 910 | `AppState::AppState` | 50–52 | store | store-7 | wrapper |  |
| 911 | `AppState::~AppState` | 56–56 | sweep-ui1 | none | none |  |
| 912 | `AppState::set_render_device` | 58–61 | sweep-ui1 | none | none |  |
| 913 | `AppState::preview_controller` | 63–67 | sweep-ui1 | none | none |  |
| 914 | `AppState::reload_library` | 69–76 | sweep-ui1 | none | none |  |
| 915 | `AppState::refresh_library_summaries` | 78–84 | store | store-7 | wrapper |  |
| 916 | `AppState::refresh_library_row` | 86–88 | store | store-7 | wrapper |  |
| 917 | `AppState::set_search` | 90–93 | sweep-ui1 | none | none |  |
| 918 | `AppState::library_matches` | 95–101 | sweep-ui1 | display-31 | wrapper | library.matches() (query_matches) |
| 919 | `AppState::tick_library` | 103–121 | sweep-ui1 | sweep-ui1-19 | owns | 1.0 s summary re-read throttle |
| 920 | `AppState::view_row_count` | 124–124 | sweep-ui1 | none | none |  |
| 921 | `AppState::view_row` | 126–128 | sweep-ui1 | none | none |  |
| 922 | `AppState::view_row_status` | 130–132 | sweep-ui1 | store-1 | wrapper | reads the stored row status |
| 923 | `AppState::relative_row` | 134–145 | sweep-ui1 | sweep-ui1-5 | copies | open song identified by notespath; update_song_length uses md5 |
| 924 | `AppState::can_select_relative` | 147–147 | sweep-ui1 | none | none |  |
| 925 | `AppState::select_relative` | 149–151 | sweep-ui1 | none | none |  |
| 926 | `AppState::select` | 153–161 | sweep-ui1 | none | none |  |
| 927 | `AppState::close_details` | 163–188 | sweep-ui1 | none | none |  |
| 928 | `AppState::selected_file_ok` | 190–198 | sweep-ui1 | sweep-ui1-20 | copies | kFileCheckSeconds 2.0, parallel to kReportCheckSeconds 2.0; no owner |
| 929 | `AppState::refresh_viewed_record` | 200–212 | store | store-7 | wrapper |  |
| 930 | `AppState::show_record_for_settings` | 214–245 | store | store-7 | wrapper |  |
| 931 | `AppState::refresh_viewed_summary` | 247–254 | screenA | screenA-32 | none | reads stored summary |
| 931 | `AppState::refresh_viewed_summary` | 247–254 | store | store-1 | wrapper |  |
| 932 | `AppState::analyze_running` | 256–256 | sweep-ui1 | none | none |  |
| 933 | `AppState::batch_running` | 258–258 | sweep-ui1 | none | none |  |
| 934 | `AppState::analyze_job_shown` | 260–263 | sweep-ui1 | sweep-ui1-5 | copies | job song vs open song by notespath; update_song_length matches by md5 |
| 935 | `AppState::tick` | 265–271 | sweep-ui1 | none | none |  |
| 936 | `AppState::update_song_length` | 273–303 | store | store-17 | none | length job + set_song_length |
| 937 | `AppState::update_analyze_job` | 305–348 | sweep-ui1 | sweep-ui1-24;display-28 | owns | owns the 0.5 s Done! flash; wraps job->message() (plain_error) |
| 938 | `AppState::report_file_shown` | 350–357 | sweep-ui1 | sweep-ui1-20 | copies | kReportCheckSeconds 2.0, parallel to kFileCheckSeconds; no owner |
| 939 | `AppState::update_dynamics` | 359–398 | parse | parse-23 | none | compares path key, reads md5 store key |
| 939 | `AppState::update_dynamics` | 359–398 | screenA | screenA-34 | wrapper |  |
| 940 | `AppState::reap_dynamics` | 400–418 | parse | parse-23 | none | saves under md5 key |
| 940 | `AppState::reap_dynamics` | 400–418 | screenA | none | none |  |
| 941 | `AppState::start_scan` | 420–425 | sweep-ui1 | none | none |  |
| 942 | `AppState::open_batch_confirm` | 427–433 | parse | none | none | batch scope |
| 943 | `AppState::start_batch` | 435–453 | parse | parse-22 | none | starts batch over every listed copy |
| 944 | `AppState::update_background_jobs` | 455–496 | sweep-ui1 | store-8;store-9;tempo-24;sweep-ui1-6 | wrapper | calls Settings::cap_query/lens/hit_window_ms, but reads current settings at batch end, not the batch's BatchRun snapshot |
| 945 | `AppState::cancel_dm_fetch` | 498–503 | sweep-ui1 | none | none |  |
| 946 | `AppState::cancel_dm_report` | 505–510 | sweep-ui1 | none | none |  |
| 947 | `AppState::start_analyze` | 512–520 | parse | none | none | analyze |
| 947 | `AppState::start_analyze` | 512–520 | store | store-7;store-26 | wrapper |  |
| 948 | `AppState::store_finished_analysis` | 522–546 | parse | parse-13;parse-22 | none | stores under the snapshotted song copy's names |
| 949 | `AppState::start_dm_fetch` | 548–553 | sweep-ui1 | none | none |  |
| 950 | `AppState::start_dm_report` | 555–566 | fills | fills-32 | none | passes settings.lens() |
| 950 | `AppState::start_dm_report` | 555–566 | store | store-9 | wrapper |  |
| 951 | `AppState::set_status` | 568–572 | sweep-ui1 | none | none |  |
| 952 | `AppState::set_problem` | 574–578 | sweep-ui1 | none | none |  |
| 953 | `AppState::dismiss_status` | 580–584 | sweep-ui1 | none | none |  |
| 954 | `AppState::commit_settings` | 586–589 | sweep-ui1 | none | none |  |
| 955 | `AppState::edit_settings` | 591–594 | sweep-ui1 | none | none |  |
| 956 | `AppState::flush_settings` | 596–598 | sweep-ui1 | none | none |  |
| 957 | `AppState::save_settings` | 600–605 | sweep-ui1 | store-16 | wrapper | Settings::save |
| 958 | `AppState::apply_settings` | 607–627 | store | store-7 | wrapper |  |

### `src/ui/app_state.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 959 | `AppState::analysis_blocked` | 161–161 | sweep-ui1 | sweep-ui1-25 | owns |  |
| 960 | `AppState::library_view_order` | 183–183 | sweep-ui1 | none | none |  |
| 961 | `AppState::library_shown_count` | 184–184 | sweep-ui1 | none | none |  |
| 962 | `AppState::library_row_at` | 186–188 | sweep-ui1 | none | none |  |
| 963 | `AppState::library_match_count` | 191–191 | sweep-ui1 | display-31 | wrapper | library.counts().all |
| 964 | `AppState::details_open` | 202–202 | sweep-ui1 | none | none |  |
| 965 | `AppState::settings_locked` | 305–305 | sweep-ui1 | sweep-ui1-26 | owns |  |

### `src/ui/details_panel.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 966 | `render_panel_header` | 32–76 | display | display-9 | wrapper | uses strip_rich_tags |
| 966 | `render_panel_header` | 32–76 | screenA | none | none |  |
| 967 | `render_headline` | 82–145 | display | display-11 | copies | verbatim copy of render_record_state's Stale sentence and "No paths found."; stars read from stored summary (wrapper) |
| 967 | `render_headline` | 82–145 | score | score-10;score-15 | copies | render_headline: score from blob best.totalscore(), stars from stored summary column (double read) |
| 967 | `render_headline` | 82–145 | screenA | screenA-1;screenA-2;screenA-3;screenA-32;screenA-38 | copies | stars from stored PathSummary (double read vs live record); stale/no-paths literals duplicate render_record_state |
| 967 | `render_headline` | 82–145 | store | store-2 | copies | fixed stale text, both reasons; owner stale_reasons |
| 968 | `render_panel_notices` | 148–162 | display | none | none | notices |
| 968 | `render_panel_notices` | 148–162 | screenA | none | none |  |
| 969 | `render_analyze_progress` | 165–209 | display | display-14 | copies | progress "%.0f%%" rounding (dynamics truncates) |
| 969 | `render_analyze_progress` | 165–209 | screenA | none | none |  |
| 970 | `render_record_state` | 219–239 | display | display-11 | owns | shared record-state lines |
| 970 | `render_record_state` | 219–239 | screenA | screenA-38 | owns |  |
| 970 | `render_record_state` | 219–239 | store | store-2 | copies | fixed stale text, both reasons; owner stale_reasons |
| 971 | `render_song_panel` | 243–318 | display | none | none | panel layout |
| 971 | `render_song_panel` | 243–318 | screenA | screenA-3 | wrapper | selected path = best_path() |

### `src/ui/dm_jobs.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 972 | `DmFetchUsersJob::start` | 14–14 | sweep-ui1 | none | none |  |
| 973 | `DmFetchUsersJob::run` | 16–23 | sweep-ui1 | none | none |  |
| 974 | `DmReportJob::DmReportJob` | 34–34 | sweep-ui1 | none | none |  |
| 975 | `DmReportJob::start` | 36–36 | sweep-ui1 | none | none |  |
| 976 | `DmReportJob::run` | 38–60 | sweep-ui1 | display-28;score-22 | owns | throw-site literal 'this user has no scores to compare' (display-28 owner = throw sites); generate_dm_report wrapper for score-22 |

### `src/ui/dm_jobs.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 977 | `DmFetchUsersJob::~DmFetchUsersJob` | 30–30 | sweep-ui1 | none | none |  |
| 978 | `DmFetchUsersJob::users` | 35–35 | sweep-ui1 | none | none |  |
| 979 | `DmReportJob::~DmReportJob` | 54–54 | sweep-ui1 | none | none |  |
| 980 | `DmReportJob::stats` | 60–60 | sweep-ui1 | score-22 | wrapper | DmReportStats from the tally |
| 981 | `DmReportJob::saved_path` | 64–64 | sweep-ui1 | none | none |  |
| 982 | `DmReportJob::opened` | 65–65 | sweep-ui1 | none | none |  |
| 983 | `DmReportJob::open_problem` | 66–66 | sweep-ui1 | none | none |  |

### `src/ui/dynamics_load_job.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 984 | `DynamicsLoadJob::DynamicsLoadJob` | 10–12 | parse | parse-23 | none | path key |
| 984 | `DynamicsLoadJob::DynamicsLoadJob` | 10–12 | screenA | screenA-34 | none |  |
| 985 | `DynamicsLoadJob::start` | 14–14 | screenA | none | none |  |
| 985 | `DynamicsLoadJob::start` | 14–14 | sweep-ui1 | none | none |  |
| 986 | `DynamicsLoadJob::run` | 16–28 | parse | parse-13 | copies | parses with bass2x=true at every difficulty (bypasses Expert-only rule in Settings::effective_bass2x) |
| 986 | `DynamicsLoadJob::run` | 16–28 | screenA | screenA-34 | copies | parses with kDynamicsParseBass2x=true on every difficulty; parser owner is effective_bass2x |
| 987 | `DynamicsLoadJob::take_result` | 30–32 | screenA | none | none |  |
| 987 | `DynamicsLoadJob::take_result` | 30–32 | sweep-ui1 | none | none |  |

### `src/ui/dynamics_load_job.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 988 | `DynamicsLoadJob::~DynamicsLoadJob` | 23–23 | sweep-ui1 | none | none |  |
| 989 | `DynamicsLoadJob::key` | 32–32 | sweep-ui1 | parse-23 | wrapper | dynamics_cache_key computed in ctor |
| 990 | `DynamicsLoadJob::entry` | 36–36 | sweep-ui1 | none | none |  |
| 991 | `DynamicsLoadJob::pro` | 37–37 | sweep-ui1 | none | none |  |
| 992 | `DynamicsLoadJob::difficulty` | 38–38 | sweep-ui1 | none | none |  |

### `src/ui/dynamics_tab.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 993 | `pad_color` | 18–31 | display | none | none | colours |
| 993 | `pad_color` | 18–31 | parse | none | none | lane colours (single place) |
| 993 | `pad_color` | 18–31 | screenA | none | none |  |
| 994 | `pad_dot` | 34–43 | display | none | none | draw |
| 994 | `pad_dot` | 34–43 | screenA | none | none |  |
| 995 | `dynamics_table_row` | 47–67 | display | none | none | row draw |
| 995 | `dynamics_table_row` | 47–67 | parse | none | none | table row |
| 995 | `dynamics_table_row` | 47–67 | screenA | screenA-36 | wrapper |  |
| 996 | `render_dynamics_panel` | 73–224 | display | display-14;display-23 | copies | percent by truncation; All kicks row (kicks_total) vs Totals (played_total) |
| 996 | `render_dynamics_panel` | 73–224 | parse | parse-13;parse-17 | copies | 2x row/messages say "2x Bass off" on Hard; All kicks includes 2x row parsed at Hard |
| 996 | `render_dynamics_panel` | 73–224 | screenA | screenA-34;screenA-36;screenA-37 | copies | inline pct truncation; bass2x/pro gating of rows |

### `src/ui/fonts.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 997 | `px` | 24–24 | sweep-ui1 | sweep-ui1-23 | owns | explicit pixel scaling |

### `src/ui/generation.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 998 | `Generation::bump` | 16–16 | sweep-ui1 | none | none |  |
| 999 | `GenerationWatcher::changed` | 24–28 | sweep-ui1 | none | none |  |

### `src/ui/icons.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1000 | `load_png_texture` | 21–63 | sweep-ui1 | none | none |  |
| 1001 | `load_icons` | 67–75 | sweep-ui1 | sweep-ui1-9 | copies | exe_dir()+resource typed again (setup_imgui honours options.resource_dir, this does not); no owner |

### `src/ui/job_base.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1002 | `JobCancelled::what` | 23–23 | sweep-ui1 | none | none |  |
| 1003 | `JobBase::cancel` | 33–33 | sweep-ui1 | none | none |  |
| 1004 | `JobBase::is_cancelled` | 34–34 | sweep-ui1 | none | none |  |
| 1005 | `JobBase::finished` | 35–35 | sweep-ui1 | none | none |  |
| 1006 | `JobBase::spawn` | 44–44 | sweep-ui1 | none | none |  |
| 1007 | `JobBase::shutdown` | 45–48 | sweep-ui1 | none | none |  |
| 1008 | `JobBase::throw_if_cancelled` | 52–54 | sweep-ui1 | none | none |  |
| 1009 | `ResultJobBase::ok` | 65–65 | sweep-ui1 | none | none |  |
| 1010 | `ResultJobBase::error` | 68–68 | sweep-ui1 | none | none |  |
| 1011 | `ResultJobBase::message` | 72–72 | sweep-ui1 | none | none |  |
| 1012 | `ResultJobBase::run_guarded` | 79–88 | sweep-ui1 | sweep-ui1-14;display-28 | owns | owns the guarded-run tail; display-28 via app::plain_error (wrapper) |

### `src/ui/label_layout.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1013 | `spaced_labels` | 22–47 | sweep-ui1 | sweep-ui1-16 | owns |  |

### `src/ui/library_dialogs.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1014 | `main_hwnd` | 24–26 | sweep-ui1 | none | none |  |
| 1015 | `enter_pressed` | 48–51 | sweep-ui1 | none | none |  |
| 1016 | `batch_counts` | 54–58 | sweep-ui1 | sweep-ui1-2;display-10 | copies | analyzed = completed - failed; CLI counts on_result instead; also in render_batch_strip tooltip |
| 1017 | `count_label` | 62–64 | sweep-ui1 | display-10 | copies | display-10 has no owner; report::counted, charts_text, bars_text etc. are parallel |
| 1018 | `format_duration` | 66–75 | sweep-ui1 | sweep-ui1-4 | owns |  |
| 1019 | `batch_settings_summary` | 77–89 | fills | fills-25;fills-33 | copies | depth_mode==0 -> scores else points; own fill-rule label |
| 1019 | `batch_settings_summary` | 77–89 | parse | parse-13 | none | settings summary uses effective_bass2x |
| 1019 | `batch_settings_summary` | 77–89 | store | store-9;store-11 | copies | depth_mode 0->scores else points; chartmode parts spelled again |
| 1020 | `empty_library_message` | 91–96 | sweep-ui1 | none | none |  |
| 1021 | `render_folder_manager` | 102–206 | sweep-ui1 | sweep-ui1-27 | owns | folder dedupe by exact string; list rows clamp 1..12 |
| 1022 | `render_scan_modal` | 208–263 | sweep-ui1 | display-10 | copies | count_label for found/problems but raw ungrouped %d for folders_seen and charts_cached |
| 1023 | `render_batch_confirm` | 268–353 | sweep-ui1 | store-4;display-10 | copies | to_run/with from library chip counts (C++ Ready via get_summaries); batch skip uses analyzed_hashes (SQL analyzed_filter); own has/have plural |
| 1024 | `render_batch_strip` | 358–427 | sweep-ui1 | sweep-ui1-2;sweep-ui1-3;display-10 | copies | kept = completed - failed (copy of batch_counts); frac falls back to 0.0 at total 0, progress_bar_counted to 1.0 |
| 1025 | `render_batch_done` | 431–528 | sweep-ui1 | sweep-ui1-4;display-10 | wrapper | format_duration, batch_counts, count_label |
| 1026 | `render_dm_picker_modal` | 534–665 | sweep-ui1 | display-10;score-22 | copies | '%d scores' with no singular and no grouping; stats read from DmReportJob |

### `src/ui/library_jobs.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1027 | `ScanJob::ScanJob` | 18–18 | sweep-ui1 | none | none |  |
| 1028 | `ScanJob::start` | 20–20 | sweep-ui1 | none | none |  |
| 1029 | `ScanJob::snapshot` | 22–25 | sweep-ui1 | none | none |  |
| 1030 | `ScanJob::run` | 27–90 | sweep-ui1 | parse-2 | wrapper | app::discover_charts |
| 1031 | `steady_seconds` | 96–99 | sweep-ui1 | none | none |  |
| 1032 | `BatchClock::start` | 103–108 | sweep-ui1 | sweep-ui1-13 | owns | BatchClock |
| 1033 | `BatchClock::pause` | 110–112 | sweep-ui1 | sweep-ui1-13 | owns | BatchClock |
| 1034 | `BatchClock::resume` | 114–118 | sweep-ui1 | sweep-ui1-13 | owns | BatchClock |
| 1035 | `BatchClock::finish` | 120–124 | sweep-ui1 | sweep-ui1-13 | owns | BatchClock |
| 1036 | `BatchClock::elapsed_s` | 126–130 | sweep-ui1 | sweep-ui1-13 | owns | BatchClock elapsed with pauses left out |
| 1037 | `batch_eta_s` | 132–135 | sweep-ui1 | sweep-ui1-12 | owns | kEtaMinFinished 3 |
| 1038 | `BatchJob::BatchJob` | 143–143 | sweep-ui1 | none | none |  |
| 1039 | `BatchJob::BatchJob` | 151–151 | sweep-ui1 | none | none |  |
| 1040 | `BatchJob::set_analyzer_for_test` | 153–156 | sweep-ui1 | none | none |  |
| 1041 | `BatchJob::start` | 158–166 | sweep-ui1 | none | none |  |
| 1042 | `BatchJob::pause` | 168–178 | sweep-ui1 | none | none |  |
| 1043 | `BatchJob::resume` | 180–190 | sweep-ui1 | none | none |  |
| 1044 | `BatchJob::stop` | 192–200 | sweep-ui1 | none | none |  |
| 1045 | `BatchJob::wait_while_paused` | 202–206 | sweep-ui1 | none | none |  |
| 1046 | `BatchJob::note_started` | 208–215 | sweep-ui1 | none | none |  |
| 1047 | `BatchJob::snapshot` | 217–223 | sweep-ui1 | none | none |  |
| 1048 | `BatchJob::run` | 225–296 | sweep-ui1 | sweep-ui1-1;sweep-ui1-14;display-28 | copies | skipped = items_.size() - p.total (cli/batch.cpp main does the same subtraction); load failure builds its own failure lines; plain_error_text wrapper |
| 1049 | `AnalyzeJob::AnalyzeJob` | 304–304 | sweep-ui1 | none | none |  |
| 1050 | `AnalyzeJob::start` | 306–332 | sweep-ui1 | sweep-ui1-14 | copies | catch tail re-types run_guarded's error_/message_/ok_/finished_ stores |
| 1051 | `AnalyzeJob::take_result` | 334–334 | sweep-ui1 | none | none |  |
| 1052 | `ReportJob::ReportJob` | 344–344 | sweep-ui1 | tempo-24 | copies | header default int(kDefaultHitWindowMs), third parallel default beside Settings::hit_window_ms and ReportOptions::hit_window_ms |
| 1053 | `ReportJob::start` | 346–346 | sweep-ui1 | none | none |  |
| 1054 | `ReportJob::run` | 348–375 | sweep-ui1 | display-28;store-8 | owns | throw-site literal 'no records stored yet'; ReportOptions cap/lens wrapper |

### `src/ui/library_jobs.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1055 | `ScanJob::~ScanJob` | 53–53 | sweep-ui1 | none | none |  |
| 1056 | `BatchClock::paused` | 80–80 | sweep-ui1 | none | none |  |
| 1057 | `BatchJob::~BatchJob` | 107–110 | sweep-ui1 | none | none |  |
| 1058 | `BatchJob::cancel` | 129–129 | sweep-ui1 | none | none |  |
| 1059 | `AnalyzeJob::~AnalyzeJob` | 195–195 | sweep-ui1 | none | none |  |
| 1060 | `AnalyzeJob::song` | 199–199 | sweep-ui1 | none | none |  |
| 1061 | `AnalyzeJob::key` | 200–200 | sweep-ui1 | none | none |  |
| 1062 | `AnalyzeJob::settings` | 201–201 | sweep-ui1 | none | none |  |
| 1063 | `AnalyzeJob::progress` | 205–205 | sweep-ui1 | none | none |  |
| 1064 | `ReportJob::~ReportJob` | 235–235 | sweep-ui1 | none | none |  |
| 1065 | `ReportJob::saved_path` | 241–241 | sweep-ui1 | none | none |  |
| 1066 | `ReportJob::opened` | 242–242 | sweep-ui1 | none | none |  |
| 1067 | `ReportJob::open_problem` | 243–243 | sweep-ui1 | none | none |  |

### `src/ui/library_model.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1068 | `chip_of` | 13–20 | display | none | none | chip mapping |
| 1068 | `chip_of` | 13–20 | store | store-1 | wrapper | maps RecordStatus |
| 1069 | `unscored_rank` | 24–31 | display | none | none | sort rank |
| 1070 | `facts_of` | 35–38 | display | none | none | facts gate |
| 1071 | `matches_query` | 40–42 | display | display-31 | wrapper | calls query_matches |
| 1072 | `apply_summary` | 47–57 | display | none | none | summary apply |
| 1073 | `best_path_label` | 62–72 | display | display-11 | copies | "Stale"/"Not analyzed" labels, own wording |
| 1073 | `best_path_label` | 62–72 | store | store-1 | wrapper | maps RecordStatus |
| 1074 | `ChipCounts::of` | 74–82 | display | none | none | counts |
| 1075 | `LibraryModel::set_charts` | 84–100 | display | display-9 | wrapper | strip_rich_tags |
| 1076 | `LibraryModel::hashes` | 102–107 | display | none | none | hashes |
| 1077 | `LibraryModel::set_summaries` | 109–116 | display | none | none | summaries |
| 1078 | `LibraryModel::set_summary_for` | 118–124 | display | none | none | summaries |
| 1079 | `LibraryModel::summaries_changed` | 126–131 | display | none | none | resort |
| 1080 | `LibraryModel::set_query` | 133–138 | display | none | none | query |
| 1081 | `LibraryModel::set_chip` | 140–144 | display | none | none | chip |
| 1082 | `LibraryModel::set_sort` | 146–152 | display | none | none | sort |
| 1083 | `LibraryModel::matches` | 154–160 | display | none | none | matches |
| 1084 | `LibraryModel::resort` | 162–213 | display | none | none | sort |
| 1085 | `LibraryModel::refilter` | 215–235 | display | none | none | filter |

### `src/ui/library_model.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1086 | `LibraryModel::rows` | 74–74 | sweep-ui2 | none | none | accessor |
| 1087 | `LibraryModel::order` | 77–77 | sweep-ui2 | none | none | accessor |
| 1088 | `LibraryModel::query` | 81–81 | sweep-ui2 | none | none | accessor |
| 1089 | `LibraryModel::counts` | 82–82 | sweep-ui2 | none | none | accessor |
| 1090 | `LibraryModel::chip` | 83–83 | sweep-ui2 | none | none | accessor |
| 1091 | `LibraryModel::sort_column` | 84–84 | sweep-ui2 | none | none | accessor |
| 1092 | `LibraryModel::ascending` | 85–85 | sweep-ui2 | none | none | accessor |

### `src/ui/library_table.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1093 | `charts_text` | 57–59 | display | display-10 | copies | count+noun (same rule as report::counted) |
| 1094 | `clear_search` | 61–66 | sweep-ui2 | none | none | plumbing |
| 1095 | `render_heading` | 69–78 | display | display-10 | wrapper | charts_text |
| 1096 | `render_search_box` | 80–129 | display | display-31 | none | help text "squeeze<=20" |
| 1097 | `chip_button` | 131–144 | sweep-ui2 | none | none | chip look; disabled via begin_disabled_button |
| 1098 | `render_chips` | 148–183 | display | display-11 | copies | Stale hint, own wording |
| 1098 | `render_chips` | 148–183 | store | store-2 | copies | fixed stale hint; owner stale_reasons |
| 1099 | `overlay_matches` | 188–202 | sweep-ui2 | none | none | draws match spans from app::match_spans |
| 1100 | `cell_text` | 205–210 | sweep-ui2 | sweep-ui2-3 | wrapper | calls text_ellipsized |
| 1101 | `draw_title_ellipsized` | 216–230 | sweep-ui2 | sweep-ui2-3 | copies | widgets.h text_ellipsized (own cut at max_w - ellipsis width) |
| 1102 | `second_line` | 247–265 | sweep-ui2 | none | none | calls app::match_spans; "charted by " prefix |
| 1103 | `render_table` | 267–448 | display | display-11 | copies | Stale/Not analyzed tooltip, own wording; colours by status |
| 1103 | `render_table` | 267–448 | store | store-2 | copies | fixed stale tooltip; owner stale_reasons |
| 1104 | `render_footer` | 452–463 | display | none | none | footer |
| 1105 | `render_library` | 467–492 | display | none | none | layout |

### `src/ui/library_toolbar.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1106 | `render_status_line` | 20–41 | sweep-ui2 | sweep-ui2-13 | owns | 6.0 s fade literal (21ddf53) |
| 1107 | `render_actions_row` | 44–154 | fills | fills-32 | copies | gates on expert, cap 4 and 1.1 fills |

### `src/ui/library_view.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1108 | `render_library_pane` | 23–27 | sweep-ui2 | sweep-ui2-8 | copies | LibraryModel::rows().size() (reads cached AppState::library_total) |
| 1109 | `render_library_and_panel` | 40–94 | sweep-ui2 | none | none | layout; px(320) min library (99cbda2), 0.5 px drag threshold |
| 1110 | `render_main_window` | 98–148 | sweep-ui2 | none | none | plumbing |

### `src/ui/main.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1111 | `add_work_area` | 54–61 | sweep-ui2 | none | none | win32 plumbing |
| 1112 | `monitor_work_areas` | 65–69 | sweep-ui2 | none | none | win32 plumbing |
| 1113 | `note_window_placement` | 76–88 | sweep-ui2 | none | none | win32 plumbing |
| 1114 | `main` | 91–324 | sweep-ui2 | sweep-ui2-11 | copies | owner none; clear colour and 1280x720 also in tests/ui/uitest_harness |
| 1115 | `CreateDeviceD3D` | 327–370 | sweep-ui2 | none | none | d3d plumbing |
| 1116 | `CleanupDeviceD3D` | 373–378 | sweep-ui2 | none | none | d3d plumbing |
| 1117 | `CreateRenderTarget` | 381–386 | sweep-ui2 | none | none | d3d plumbing |
| 1118 | `CleanupRenderTarget` | 389–391 | sweep-ui2 | none | none | d3d plumbing |
| 1119 | `WndProc` | 398–436 | sweep-ui2 | none | wrapper | calls ui_scale_for_dpi |

### `src/ui/paths_tab.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1120 | `text_color` | 37–37 | screenA | none | none |  |
| 1120 | `text_color` | 37–37 | sweep-ui2 | none | none | style read |
| 1121 | `dim_color` | 38–38 | screenA | none | none |  |
| 1121 | `dim_color` | 38–38 | sweep-ui2 | none | none | style read |
| 1122 | `text_at` | 43–50 | screenA | none | none |  |
| 1122 | `text_at` | 43–50 | sweep-ui2 | none | none | draw helper |
| 1123 | `wrapped_height` | 53–58 | screenA | none | none |  |
| 1123 | `wrapped_height` | 53–58 | sweep-ui2 | none | none | measure helper |
| 1124 | `end_overlay` | 63–66 | screenA | none | none |  |
| 1124 | `end_overlay` | 63–66 | sweep-ui2 | none | none | layout helper |
| 1125 | `fits_on_line` | 69–71 | screenA | none | none |  |
| 1125 | `fits_on_line` | 69–71 | sweep-ui2 | sweep-ui2-2 | owns | right edge = WorkRect.Max.x |
| 1126 | `flow_next` | 76–78 | screenA | none | none |  |
| 1126 | `flow_next` | 76–78 | sweep-ui2 | sweep-ui2-2 | wrapper | calls fits_on_line |
| 1127 | `align_right` | 82–86 | screenA | none | none |  |
| 1127 | `align_right` | 82–86 | sweep-ui2 | sweep-ui2-2 | wrapper | calls fits_on_line |
| 1128 | `button_width` | 88–90 | screenA | none | none |  |
| 1128 | `button_width` | 88–90 | sweep-ui2 | sweep-ui2-1 | copies | widgets.h button_slot_width (same formula) |
| 1129 | `mono_width` | 98–103 | screenA | none | none |  |
| 1129 | `mono_width` | 98–103 | sweep-ui2 | none | none | measure helper |
| 1130 | `path_button` | 110–136 | display | display-4 | wrapper | timing_warn from Path::is_difficult |
| 1130 | `path_button` | 110–136 | screenA | screenA-1;screenA-3;screenA-4;screenA-5 | wrapper | lays out PathButtonView; optimal colour from group |
| 1131 | `render_path_list` | 140–156 | display | none | none | headings ("Best all-0 path" hint) |
| 1131 | `render_path_list` | 140–156 | screenA | screenA-3;screenA-8 | wrapper | headings from group |
| 1132 | `render_timeline` | 166–231 | display | display-4 | copies | orange outline when badge non-empty, not when is_difficult; number colour by badge.empty() |
| 1132 | `render_timeline` | 166–231 | screenA | screenA-13 | copies | left label literal m1; marks from song_fraction (owner build_activations) |
| 1133 | `render_activation_row` | 240–276 | display | display-4 | wrapper | badge colour from a.difficult |
| 1133 | `render_activation_row` | 240–276 | screenA | screenA-10;screenA-11;screenA-12 | wrapper |  |
| 1134 | `squeeze_box` | 280–296 | display | display-4 | wrapper | box border from TextLine.warn |
| 1134 | `squeeze_box` | 280–296 | screenA | screenA-12 | wrapper |  |
| 1135 | `render_backend_table` | 299–356 | backend | none | none | renders strings only |
| 1135 | `render_backend_table` | 299–356 | display | none | none | table layout |
| 1135 | `render_backend_table` | 299–356 | screenA | screenA-18;screenA-21;screenA-22 | wrapper |  |
| 1136 | `render_activation_body` | 361–412 | display | display-17 | wrapper | scale line colour from scale_warn |
| 1136 | `render_activation_body` | 361–412 | screenA | screenA-14;screenA-15;screenA-23;screenA-24;screenA-25 | wrapper |  |
| 1137 | `render_activations` | 416–443 | display | none | none | layout |
| 1137 | `render_activations` | 416–443 | screenA | screenA-9 | wrapper |  |
| 1138 | `fold_button` | 448–453 | display | none | none | button |
| 1138 | `fold_button` | 448–453 | screenA | none | none |  |
| 1139 | `render_path_footer` | 456–511 | backend | backend-11;backend-8 | copies | clamp(0,500) literal 500 = kSqueezeWindowMs |
| 1139 | `render_path_footer` | 456–511 | display | display-29 | copies | backend limit clamp 0..500 literal (kSqueezeWindowMs is 500); Settings load does not clamp |
| 1139 | `render_path_footer` | 456–511 | screenA | screenA-27;screenA-28;screenA-29 | copies | clamp 0..500 owns nothing; Settings::backend_limit uses abs; 500 duplicates kSqueezeWindowMs |
| 1139 | `render_path_footer` | 456–511 | spwin | spwin-14 | copies | backend limit clamp 0..500 literal parallels kSqueezeWindowMs (partial read) |
| 1140 | `copy_selected_path` | 515–520 | display | display-1 | wrapper | pathstring_verbose |
| 1140 | `copy_selected_path` | 515–520 | screenA | screenA-41 | wrapper | Path::pathstring_verbose |
| 1141 | `render_path_panel` | 522–562 | display | none | none | layout |
| 1141 | `render_path_panel` | 522–562 | screenA | none | wrapper |  |

### `src/ui/preview_controller.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1142 | `overlay_key` | 22–24 | screenB | none | none |  |
| 1142 | `overlay_key` | 22–24 | sweep-ui2 | sweep-ui2-7 | owns | composite key path\|capN |
| 1143 | `PreviewController::PreviewController` | 29–29 | sweep-ui2 | none | none | ctor |
| 1144 | `PreviewController::~PreviewController` | 31–31 | sweep-ui2 | none | none | dtor |
| 1145 | `PreviewController::open` | 36–66 | screenB | screenB-12 | none |  |
| 1145 | `PreviewController::open` | 36–66 | sweep-ui2 | screenB-12;sweep-ui2-17 | wrapper | takes cap from render_preview_panel |
| 1146 | `PreviewController::start_scene_job` | 68–75 | sweep-ui2 | none | none | plumbing |
| 1147 | `PreviewController::load_progress` | 77–81 | sweep-ui2 | none | wrapper | calls Progress::fraction/label |
| 1148 | `PreviewController::close` | 83–106 | sweep-ui2 | sweep-ui2-17 | copies | render_preview_panel cap pick (close resets sp_cap_ to kCloneHeroSpCap) |
| 1149 | `PreviewController::poll` | 108–168 | screenB | none | none |  |
| 1149 | `PreviewController::poll` | 108–168 | sweep-ui2 | tempo-12;sweep-ui2-5 | copies | Settings::preview_volume (percent/100 typed again, also in set_volume); passes scene length to transport |
| 1150 | `PreviewController::render` | 170–199 | screenB | none | none |  |
| 1150 | `PreviewController::render` | 170–199 | sweep-ui2 | none | none | renderer plumbing |
| 1151 | `PreviewController::play` | 203–206 | sweep-ui2 | none | none | forward |
| 1152 | `PreviewController::pause` | 208–208 | sweep-ui2 | none | none | forward |
| 1153 | `PreviewController::toggle` | 210–215 | sweep-ui2 | none | none | forward |
| 1154 | `PreviewController::playing` | 217–217 | sweep-ui2 | none | none | forward |
| 1155 | `PreviewController::position_ms` | 219–219 | sweep-ui2 | none | none | forward |
| 1156 | `PreviewController::length_ms` | 221–221 | sweep-ui2 | tempo-12 | wrapper | transport length |
| 1157 | `PreviewController::seek_ms` | 223–223 | sweep-ui2 | none | none | forward (transport clamps) |
| 1158 | `PreviewController::jump_ms` | 225–228 | screenB | none | none |  |
| 1158 | `PreviewController::jump_ms` | 225–228 | sweep-ui2 | none | none | forward |
| 1159 | `PreviewController::step_ticks` | 230–235 | screenB | screenB-8 | wrapper |  |
| 1159 | `PreviewController::step_ticks` | 230–235 | sweep-ui2 | screenB-8 | wrapper | calls step_tick_ms |
| 1160 | `PreviewController::set_scrubbing` | 237–247 | sweep-ui2 | none | none | scrub pause rule |
| 1161 | `PreviewController::has_audio` | 249–249 | sweep-ui2 | none | none | forward |
| 1162 | `PreviewController::set_volume` | 253–256 | sweep-ui2 | sweep-ui2-5 | copies | Settings::preview_volume + load_file range (own clamp 0..100 and /100) |
| 1163 | `PreviewController::time_box` | 258–261 | screenB | screenB-7 | wrapper |  |
| 1163 | `PreviewController::time_box` | 258–261 | sweep-ui2 | screenB-7;screenB-8 | wrapper | build_time_box with transport length |
| 1164 | `PreviewController::score_box` | 263–265 | screenB | screenB-9 | wrapper |  |
| 1164 | `PreviewController::score_box` | 263–265 | sweep-ui2 | screenB-9;screenB-8 | wrapper | build_score_box, no length passed |
| 1165 | `PreviewController::drain_box` | 267–269 | screenB | screenB-3 | wrapper |  |
| 1165 | `PreviewController::drain_box` | 267–269 | sweep-ui2 | screenB-3;screenB-4;screenB-5;screenB-6;screenB-8 | wrapper | build_drain_box, no length passed |
| 1166 | `PreviewController::scrub_marks` | 271–273 | screenB | none | none |  |
| 1166 | `PreviewController::scrub_marks` | 271–273 | sweep-ui2 | display-12 | wrapper | build_scrub_marks over transport length |
| 1167 | `PreviewController::next_act_box` | 275–277 | screenB | screenB-11 | wrapper |  |
| 1167 | `PreviewController::next_act_box` | 275–277 | sweep-ui2 | screenB-11;tempo-21 | wrapper | build_next_act_box |
| 1168 | `PreviewController::next_act_boxes` | 279–285 | screenB | screenB-11 | wrapper |  |
| 1168 | `PreviewController::next_act_boxes` | 279–285 | sweep-ui2 | screenB-11;tempo-21 | wrapper | build_next_act_box at each a.ms |
| 1169 | `PreviewController::sp_meter_readout` | 287–289 | screenB | screenB-1 | wrapper |  |
| 1169 | `PreviewController::sp_meter_readout` | 287–289 | sweep-ui2 | screenB-1 | wrapper | sp_meter_readout |
| 1170 | `PreviewController::jump_activation` | 291–298 | screenB | screenB-11 | wrapper |  |
| 1170 | `PreviewController::jump_activation` | 291–298 | sweep-ui2 | screenB-11;tempo-21 | wrapper | activation_jump_ms |
| 1171 | `PreviewController::seek_activation` | 300–304 | screenB | none | none |  |
| 1171 | `PreviewController::seek_activation` | 300–304 | sweep-ui2 | tempo-21 | none | seeks exactly a.ms |
| 1172 | `PreviewController::preview_config` | 306–309 | sweep-ui2 | none | none | config read |
| 1173 | `PreviewController::sp_meter_bars` | 311–313 | screenB | screenB-1 | wrapper |  |
| 1173 | `PreviewController::sp_meter_bars` | 311–313 | sweep-ui2 | screenB-1 | wrapper | sp_meter_bars_at |
| 1174 | `PreviewController::sp_meter_cap` | 315–315 | screenB | screenB-12 | wrapper |  |
| 1174 | `PreviewController::sp_meter_cap` | 315–315 | sweep-ui2 | screenB-12;sweep-ui2-17 | wrapper | reads scene_.sp_meter.cap, not sp_cap_ |
| 1175 | `PreviewController::sp_meter_has_curve` | 317–319 | screenB | none | none |  |
| 1175 | `PreviewController::sp_meter_has_curve` | 317–319 | sweep-ui2 | sweep-ui2-16 | copies | owner none; build_drain_box repeats segments.empty() |

### `src/ui/preview_controller.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1176 | `PreviewController::active` | 70–70 | sweep-ui2 | none | none | accessor |
| 1177 | `PreviewController::overlay_path_key` | 75–75 | sweep-ui2 | sweep-ui2-7 | wrapper | returns composite key |
| 1178 | `PreviewController::loading` | 87–87 | sweep-ui2 | none | none | accessor |
| 1179 | `PreviewController::has_error` | 94–94 | sweep-ui2 | none | none | accessor |
| 1180 | `PreviewController::error` | 95–95 | sweep-ui2 | none | none | accessor |
| 1181 | `PreviewController::has_audio_warning` | 100–100 | sweep-ui2 | none | none | accessor |
| 1182 | `PreviewController::audio_warning` | 101–101 | sweep-ui2 | none | none | accessor |
| 1183 | `PreviewController::set_audio_device_factory` | 109–111 | sweep-ui2 | none | none | test seam |
| 1184 | `PreviewController::set_overlay_scale` | 184–184 | sweep-ui2 | none | none | accessor |
| 1185 | `PreviewController::overlay_scale` | 185–185 | sweep-ui2 | none | none | accessor |

### `src/ui/preview_load_job.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1186 | `PreviewLoadJob::PreviewLoadJob` | 22–22 | sweep-ui2 | none | none | ctor |
| 1187 | `PreviewLoadJob::start` | 24–24 | sweep-ui2 | none | none | spawn |
| 1188 | `PreviewLoadJob::run` | 26–65 | tempo | tempo-11 | copies | negative offset becomes front pad; transport applies the rest; owner preview_audio_offset_ms |
| 1189 | `PreviewLoadJob::take_result` | 67–67 | sweep-ui2 | none | none | accessor |
| 1190 | `PreviewLoadJob::progress` | 69–75 | sweep-ui2 | none | none | accessor |
| 1191 | `PreviewLoadJob::Progress::fraction` | 77–91 | sweep-ui2 | sweep-ui2-4 | copies | owner none; total 0 gives 0.10 (progress_bar_counted 1.0, batch strip 0.0) |
| 1192 | `PreviewLoadJob::Progress::label` | 93–103 | sweep-ui2 | none | none | label text |
| 1193 | `PreviewSceneJob::PreviewSceneJob` | 111–111 | sweep-ui2 | none | none | ctor |
| 1194 | `PreviewSceneJob::start` | 113–113 | sweep-ui2 | none | none | spawn |
| 1195 | `PreviewSceneJob::run` | 115–121 | sweep-ui2 | none | wrapper | calls build_preview_scene |
| 1196 | `PreviewSceneJob::take_scene` | 123–123 | sweep-ui2 | none | none | accessor |

### `src/ui/preview_load_job.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1197 | `PreviewLoadJob::~PreviewLoadJob` | 33–33 | sweep-ui2 | none | none | dtor |
| 1198 | `PreviewSceneJob::~PreviewSceneJob` | 93–93 | sweep-ui2 | none | none | dtor |
| 1199 | `PreviewSceneJob::key` | 98–98 | sweep-ui2 | none | none | accessor |

### `src/ui/preview_tab.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1200 | `key_hints` | 48–104 | screenB | none | none |  |
| 1200 | `key_hints` | 48–104 | sweep-ui2 | sweep-ui2-6;sweep-ui2-2 | copies | owner none for 5 s; "5 seconds"/"5 ticks" text, no constant (f4ade35); own wrap rule |
| 1201 | `render_path_picker` | 110–152 | screenB | screenB-18 | wrapper |  |
| 1201 | `render_path_picker` | 110–152 | sweep-ui2 | screenB-18;sweep-ui2-1;sweep-ui2-3 | copies | widgets.h button_slot_width (own chrome width); render::ellipsize; labels via preview_path_label |
| 1202 | `draw_scrub_marks` | 157–171 | screenB | none | none |  |
| 1202 | `draw_scrub_marks` | 157–171 | sweep-ui2 | display-34;display-12;sweep-ui2-14 | owns | kBestPathColor wrapper; pad 2.0 literal (c5e9758) |
| 1203 | `render_preview_panel` | 178–585 | screenB | screenB-12 | copies | re-clamps cap (std::max(1,..)) also clamped in build_sp_meter_curve |
| 1203 | `render_preview_panel` | 178–585 | spwin | spwin-8;spwin-9 | copies | value_or(kCloneHeroSpCap) and max(1, cap) (partial read) |
| 1203 | `render_preview_panel` | 178–585 | store | store-8;store-12 | copies | blob sp_cap else kCloneHeroSpCap |

### `src/ui/preview_transport.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1204 | `PreviewTransport::PreviewTransport` | 9–9 | sweep-ui2 | none | none | ctor |
| 1205 | `PreviewTransport::load` | 12–25 | tempo | tempo-11;tempo-12 | copies | song length = max(last note, audio end - offset); owner store::song_length_ms for last note |
| 1206 | `PreviewTransport::unload` | 27–34 | sweep-ui2 | none | none | unload |
| 1207 | `PreviewTransport::play` | 36–45 | tempo | tempo-11 | wrapper | applies offset on play |
| 1208 | `PreviewTransport::pause` | 47–51 | sweep-ui2 | none | none | pause |
| 1209 | `PreviewTransport::toggle` | 53–58 | sweep-ui2 | none | none | toggle |
| 1210 | `PreviewTransport::seek_ms` | 60–66 | tempo | tempo-11 | wrapper | applies offset on seek, clamps to length |
| 1211 | `PreviewTransport::playing` | 68–68 | sweep-ui2 | none | none | accessor |
| 1212 | `PreviewTransport::length_ms` | 70–70 | tempo | tempo-12 | wrapper | length getter |
| 1213 | `PreviewTransport::has_audio` | 72–75 | sweep-ui2 | none | none | audio present |
| 1214 | `PreviewTransport::tick` | 77–84 | tempo | tempo-12 | none | stop at end |
| 1215 | `PreviewTransport::now_ms` | 86–86 | tempo | none | wrapper | clock read |
| 1216 | `PreviewTransport::set_gain` | 88–92 | sweep-ui2 | sweep-ui2-5 | copies | Settings::preview_volume range (own clamp, < 0 only) |
| 1217 | `PreviewTransport::read_frames` | 94–98 | sweep-ui2 | none | none | audio plumbing |
| 1218 | `PreviewTransport::channels` | 100–103 | sweep-ui2 | none | none | accessor |
| 1219 | `PreviewTransport::sample_rate` | 105–108 | sweep-ui2 | none | none | accessor |

### `src/ui/preview_transport.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1220 | `PreviewTransport::gain` | 59–59 | sweep-ui2 | none | none | accessor |

### `src/ui/report_outcome.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1221 | `publish_report` | 30–39 | sweep-ui2 | none | none | write + open; kReportOpenProblem |

### `src/ui/settings_bar.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1222 | `separator_gap` | 23–23 | sweep-ui2 | none | none | layout px(14) |
| 1223 | `draw_divider` | 29–32 | sweep-ui2 | none | none | draw helper |
| 1224 | `render_difficulty` | 34–72 | display | none | none | settings |
| 1225 | `render_sp_cap` | 74–103 | display | display-18;display-19 | copies | fill rule prose (owner graph.h); SP cap via kCloneHeroSpCap (wrapper) |
| 1225 | `render_sp_cap` | 74–103 | spwin | spwin-8 | copies | max(1, cap) (partial read) |
| 1226 | `render_score_range` | 105–130 | display | none | none | settings |
| 1226 | `render_score_range` | 105–130 | fills | fills-25;fills-26 | copies | combo index; only place that clamps depth_value >= 0 |
| 1227 | `render_path_limit` | 132–150 | display | display-29 | copies | ms limit clamp -500..500 literal |
| 1228 | `render_settings_bar` | 154–243 | display | none | none | layout |
| 1228 | `render_settings_bar` | 154–243 | fills | fills-34 | none | help text restates the deadline rule |

### `src/ui/song_length_job.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1229 | `SongLengthJob::SongLengthJob` | 8–8 | sweep-ui2 | none | none | ctor |
| 1230 | `SongLengthJob::start` | 10–10 | sweep-ui2 | none | none | spawn |
| 1231 | `SongLengthJob::run` | 12–23 | parse | parse-21 | wrapper | calls store::song_length_ms |
| 1231 | `SongLengthJob::run` | 12–23 | store | store-17 | wrapper | store::song_length_ms |
| 1231 | `SongLengthJob::run` | 12–23 | tempo | tempo-12 | wrapper | calls store::song_length_ms |

### `src/ui/song_length_job.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1232 | `SongLengthJob::~SongLengthJob` | 25–25 | sweep-ui2 | none | none | dtor |
| 1233 | `SongLengthJob::entry` | 29–29 | sweep-ui2 | none | none | accessor |
| 1234 | `SongLengthJob::length_ms` | 31–31 | tempo | tempo-12 | wrapper | getter |

### `src/ui/stars_tab.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1235 | `render_stars_panel` | 17–59 | display | display-22 | copies | computes cutoff + solo_bonus itself; "%.1f" multiplier from kStarMultipliers |
| 1235 | `render_stars_panel` | 17–59 | score | score-12;score-14 | copies | render_stars_panel computes cutoff + solo_bonus in the UI; cutoffs from star_cutoffs |
| 1235 | `render_stars_panel` | 17–59 | screenA | screenA-30;screenA-31;screenA-33 | wrapper | + cutoff+solo addition |

### `src/ui/theme.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1236 | `apply_theme` | 5–56 | sweep-ui2 | sweep-ui2-12 | owns | (0,100,100) literal typed twice |
| 1237 | `begin_disabled_button` | 58–66 | sweep-ui2 | none | none | disabled colours |
| 1238 | `end_disabled_button` | 68–71 | sweep-ui2 | none | none | pop |
| 1239 | `begin_disabled_input` | 73–86 | sweep-ui2 | sweep-ui2-12 | none | uses kDisabledInputTextColor (same value as kDimTextColor) |
| 1240 | `end_disabled_input` | 88–91 | sweep-ui2 | none | none | pop |
| 1241 | `begin_disabled_checkbox` | 93–102 | sweep-ui2 | sweep-ui2-12 | none | uses kDimTextColor |
| 1242 | `end_disabled_checkbox` | 104–107 | sweep-ui2 | none | none | pop |

### `src/ui/widgets.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1243 | `hint` | 28–31 | sweep-ui2 | none | none | tooltip |
| 1244 | `help_marker` | 35–40 | sweep-ui2 | none | none | tooltip |
| 1245 | `WarnColor::WarnColor` | 45–45 | sweep-ui2 | none | none | raii |
| 1246 | `WarnColor::~WarnColor` | 46–46 | sweep-ui2 | none | none | raii |
| 1247 | `progress_bar_counted` | 53–58 | sweep-ui2 | sweep-ui2-4 | copies | owner none; total 0 gives 1.0 (full) |
| 1248 | `widest_digits` | 70–79 | sweep-ui2 | sweep-ui2-15 | owns | widest digit sample |
| 1249 | `digit_count` | 82–86 | sweep-ui2 | none | none | digit count |
| 1250 | `text_slot_width` | 88–88 | sweep-ui2 | none | none | measure |
| 1251 | `text_in_slot` | 93–97 | sweep-ui2 | none | none | slot layout |
| 1252 | `button_slot_width` | 101–104 | sweep-ui2 | sweep-ui2-1 | owns | label + 2x FramePadding.x |
| 1253 | `button_in_slot` | 105–107 | sweep-ui2 | none | none | button |
| 1254 | `pin_next_modal_width` | 112–114 | sweep-ui2 | none | none | modal width |
| 1255 | `overflow_tooltip` | 119–129 | sweep-ui2 | none | none | tooltip; wrap at 30 font sizes (21ddf53) |
| 1256 | `text_ellipsized` | 136–156 | sweep-ui2 | sweep-ui2-3 | owns | RenderTextEllipsis; fits when <= avail |
| 1257 | `row_selectable` | 162–170 | sweep-ui2 | sweep-ui2-3 | copies | widgets.h text_ellipsized (fit test re-typed as > avail) |

### `src/ui/win32_dialogs.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1258 | `browse_for_folder` | 17–55 | sweep-ui2 | none | none | folder dialog |
| 1259 | `show_in_folder_seam` | 58–61 | sweep-ui2 | none | none | seam |
| 1260 | `set_show_in_folder` | 64–64 | sweep-ui2 | none | none | seam |
| 1261 | `show_in_folder` | 66–73 | sweep-ui2 | sweep-ui2-9 | copies | owner none; twin of app/report_files.cpp open_in_browser (> 32) |

### `tests/corpus_util.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1262 | `root` | 38–38 | sweep-tests1 | none | none | corpus root path |
| 1263 | `chart_paths` | 43–55 | sweep-tests1 | parse-1;parse-2 | wrapper | calls app::discover_charts |
| 1264 | `first_chart_with_suffix` | 58–63 | sweep-tests1 | parse-1 | copies | chart_format_of (picks by case-sensitive ends_with suffix, test-only selection) |
| 1265 | `read_bytes` | 65–71 | sweep-tests1 | none | none | file read |
| 1266 | `load_json` | 73–75 | sweep-tests1 | none | none | json parse |
| 1267 | `add_settings` | 95–105 | sweep-tests1 | store-7;store-9 | copies | store::Lens/RecordKey + Settings::record_key (own list of record-changing SearchSettings fields for the cache key; all 6 today) |
| 1268 | `song` | 113–130 | sweep-tests1 | store-14 | wrapper | calls Rules::fingerprint for the parse cache key |
| 1269 | `analyzed` | 136–156 | sweep-tests1 | store-7 | wrapper | calls add_settings + analyze_chart |
| 1270 | `analyzed` | 161–165 | sweep-tests1 | none | wrapper | converts SearchSettings to AnalysisSettings |

### `tests/env_util.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1271 | `read_env` | 14–21 | sweep-tests1 | none | none | env read |

### `tests/midi_util.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1272 | `smf` | 18–28 | sweep-tests1 | none | none | SMF builder (div 480) |
| 1273 | `track_name` | 31–36 | sweep-tests1 | none | none | track-name meta builder |
| 1274 | `text_event` | 39–44 | sweep-tests1 | none | none | text meta builder |
| 1275 | `set_tempo` | 48–53 | sweep-tests1 | none | none | tempo meta builder |
| 1276 | `note_on` | 56–58 | sweep-tests1 | none | none | note_on builder |
| 1277 | `end_of_track` | 61–63 | sweep-tests1 | none | none | end-of-track builder |
| 1278 | `concat` | 67–71 | sweep-tests1 | none | none | byte concat |

### `tests/multidiff_chart.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1279 | `chart_text` | 19–54 | sweep-tests1 | parse-10;parse-19 | test | multi-difficulty .chart fixture |
| 1280 | `chart_bytes` | 62–65 | sweep-tests1 | none | none | bytes of fixture |

### `tests/record_bytes.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1281 | `record_bytes` | 15–21 | store | store-19 | wrapper | flatten_record |

### `tests/test_activation_row_layout.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1282 | `mono` | 24–24 | sweep-tests1 | sweep-tests1-1 | test | font width assumption 9.6 px |
| 1283 | `text` | 25–25 | sweep-tests1 | sweep-tests1-1 | test | font width assumption 7 px |
| 1284 | `activation row: a short measure keeps the bars at +200 px` | 29–34 | sweep-tests1 | sweep-tests1-1 | test | pins 104/200 literals instead of kRowMeasureX/kRowMinBarsX |
| 1285 | `activation row: an 11-character measure pushes the bars past it` | 36–43 | sweep-tests1 | sweep-tests1-1 | copies | activation_row_layout (re-types 104 + w > 200 overlap rule) |
| 1286 | `activation row: the layout scales with the DPI` | 45–51 | sweep-tests1 | sweep-tests1-1 | copies | activation_row_layout (recomputes 2*(104+mono(11)+gap)) |
| 1287 | `activation row: the badge sits flush right, its pill clear of long bars` | 53–62 | sweep-tests1 | sweep-tests1-1 | copies | activation_row_layout (recomputes row_w - kRowBadgeRight, badge_x - pad) |
| 1288 | `activation row: every row of a path puts its bars at the same x` | 64–69 | sweep-tests1 | sweep-tests1-1 | test |  |

### `tests/test_analysis.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1289 | `discover_charts walks the corpus without throwing` | 43–55 | sweep-tests1 | parse-8 | test | md5 is 32 hex |
| 1290 | `discover_charts returns nothing for an empty root list` | 57–61 | sweep-tests1 | none | test |  |
| 1291 | `rel_of` | 67–74 | sweep-tests1 | sweep-tests1-11 | copies | tools/bench.cpp scan_mode --dump-rel (snapshot key relativization) |
| 1292 | `discover_charts output matches the checked-in scan snapshot` | 81–108 | sweep-tests1 | parse-8;parse-6;parse-7 | test | scan snapshot pin |
| 1293 | `rescan cache reproduces the scan without reading chart files` | 110–150 | sweep-tests1 | sweep-tests1-12 | test |  |
| 1294 | `run_batch files results under the lens it is given` | 152–176 | sweep-tests1 | store-8;store-9;fills-25 | copies | Settings::batch_run (builds BatchRun lens Lens::from(10,0,10) and settings separately; key rebuilt as CapQuery::at(settings.sp_cap)) |
| 1295 | `run_work_pool hands every item to the consumer once` | 178–183 | sweep-tests1 | sweep-tests1-13 | test |  |
| 1296 | `run_work_pool: a cancel mid-run never strands the consumer` | 185–215 | sweep-tests1 | sweep-tests1-13 | test |  |
| 1297 | `fake_items` | 220–230 | sweep-tests1 | none | none | fake scan items |
| 1298 | `run_batch: cancel stops running searches within seconds` | 234–282 | sweep-tests1 | sweep-tests1-14 | test | 5 s pass line |
| 1299 | `run_batch: a cancelled real search is neither a result nor a failure` | 284–323 | sweep-tests1 | sweep-tests1-14 | test |  |
| 1300 | `write_sng_with_metadata` | 329–344 | parse | parse-24 | copies | test helper re-encodes sng metadata layout |
| 1301 | `discover_charts: a song with no usable name reads unknown` | 348–384 | sweep-tests1 | parse-7;parse-6 | test |  |
| 1302 | `rescan cache: an old placeholder or blank title reads unknown` | 386–406 | sweep-tests1 | parse-7 | test | old placeholder <unknown title> |
| 1303 | `scan_fixture_dir` | 411–418 | sweep-tests1 | none | none | temp dir |
| 1304 | `write_fixture` | 420–425 | sweep-tests1 | none | none | file write |
| 1305 | `discover_charts finds a folder whose notes and ini names are capitalized` | 429–445 | sweep-tests1 | parse-2;parse-4 | test | capitalized Notes.mid/Song.ini |

### `tests/test_app_shell.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1306 | `test_options` | 28–34 | sweep-tests1 | none | none | ImGui options |
| 1307 | `app_shell: a saved window is reopened only where its title bar can be reached` | 38–48 | sweep-tests1 | sweep-tests1-2 | test |  |
| 1308 | `app_shell: the [Hydra][Window] text round-trips` | 50–77 | sweep-tests1 | sweep-tests1-4 | test |  |
| 1309 | `app_shell: the UI scale for a monitor DPI` | 79–85 | sweep-tests1 | sweep-tests1-3 | test |  |
| 1310 | `app_shell: scaling always starts from the unscaled style` | 87–99 | sweep-tests1 | sweep-tests1-15 | test |  |
| 1311 | `app_shell: hydra_ui.ini remembers the window placement` | 101–131 | sweep-tests1 | sweep-tests1-4 | test |  |
| 1312 | `app_shell: the [Hydra][Layout] text round-trips and refuses junk` | 133–152 | sweep-tests1 | sweep-tests1-4 | test |  |
| 1313 | `app_shell: hydra_ui.ini remembers the library split, not the child's own width` | 154–183 | sweep-tests1 | sweep-tests1-4 | test |  |
| 1314 | `app_shell: set_ui_scale rescales sizes, fonts and px together` | 185–201 | sweep-tests1 | sweep-tests1-15 | copies | ui::scaled_style (recomputes truncation static_cast<int>(before*2)) |

### `tests/test_app_state.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1315 | `temp_path` | 51–56 | sweep-tests1 | none | none | temp path |
| 1316 | `ScratchPaths::ScratchPaths` | 72–81 | sweep-tests1 | none | none | path overrides setup |
| 1317 | `ScratchPaths::~ScratchPaths` | 83–88 | sweep-tests1 | none | none | path overrides restore |
| 1318 | `library_entry` | 91–103 | sweep-tests1 | none | none | library fixture row |
| 1319 | `seeded_store` | 109–123 | sweep-tests1 | store-7;store-8;store-11 | copies | Settings::chartmode_key / Settings::cap_query (kChartMode literal "Expert Pro Drums, 2x Bass" and kSeededCap 4) |
| 1320 | `app_on` | 127–136 | sweep-tests1 | none | none | AppState setup |
| 1321 | `commit_settings refreshes the viewed record when the chart mode changes` | 140–163 | sweep-tests1 | store-7;store-11 | test |  |
| 1322 | `commit_settings refreshes the record and the library row on an SP cap change` | 165–183 | sweep-tests1 | store-7;store-8 | test |  |
| 1323 | `commit_settings refreshes when the ms limit or the score range changes` | 185–218 | sweep-tests1 | store-7;store-9 | test |  |
| 1324 | `commit_settings with a non-identity change does not bump the record generation` | 220–234 | sweep-tests1 | store-7;screenA-39 | test | hit window is display-only |
| 1325 | `a bad hydra_rules.ini names the key and keeps analysis off` | 236–257 | sweep-tests1 | store-15 | test |  |
| 1326 | `no hydra_rules.ini leaves analysis on` | 259–265 | sweep-tests1 | store-15 | test |  |
| 1327 | `under a bad hydra_rules.ini no stored record reads Ready` | 267–290 | sweep-tests1 | store-1;store-14 | test |  |
| 1328 | `update_dynamics recounts a stored row with an older count stamp` | 292–308 | sweep-tests1 | store-20 | test |  |
| 1329 | `update_dynamics uses a stored row with the current count stamp` | 310–321 | sweep-tests1 | store-20 | test |  |
| 1330 | `close_details keeps a Dynamics count that finished on another tab` | 326–347 | sweep-tests1 | store-20;parse-23 | test |  |
| 1331 | `the chart-file check runs on open and then every two seconds` | 351–367 | sweep-tests1 | sweep-tests1-16 | test |  |
| 1332 | `number boxes apply at once but write the INI only on flush` | 372–384 | sweep-tests1 | sweep-tests1-17 | test |  |
| 1333 | `stepping a number box back reuses the lookup it already made` | 388–413 | sweep-tests1 | sweep-tests1-18 | test |  |
| 1334 | `the report-file check is cached for two seconds` | 417–429 | sweep-tests1 | sweep-tests1-16 | test |  |
| 1335 | `set_search narrows the library and the match count` | 432–442 | sweep-tests1 | sweep-tests1-19 | test |  |
| 1336 | `refresh_library_row picks up one chart's new result` | 446–464 | sweep-tests1 | store-1 | test |  |

### `tests/test_audio_decode.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1337 | `read_fixture` | 27–30 | sweep-tests1 | none | none | fixture read (duplicate of test_audio_mixer read_fixture) |
| 1338 | `bytes` | 32–37 | sweep-tests1 | none | none | byte list |
| 1339 | `put_u16` | 39–42 | sweep-tests1 | none | none | WAV LE writer |
| 1340 | `put_u32` | 43–45 | sweep-tests1 | none | none | WAV LE writer |
| 1341 | `make_wav_mono16` | 49–68 | sweep-tests1 | none | none | WAV fixture builder |
| 1342 | `estimate_freq_hz` | 73–86 | sweep-tests1 | none | none | test measurement (duplicate of test_audio_mixer estimate_freq_hz) |
| 1343 | `peak_abs` | 88–95 | sweep-tests1 | none | none | test measurement |
| 1344 | `sniff_format classifies audio containers by their magic bytes` | 99–117 | sweep-tests1 | sweep-tests1-6 | test |  |
| 1345 | `decode_audio: PCM16 WAV decodes to matching float samples` | 119–131 | sweep-tests1 | sweep-tests1-20 | test |  |
| 1346 | `decode_audio: MP3 fixture decodes to the 220 Hz sine` | 133–141 | sweep-tests1 | sweep-tests1-20 | test |  |
| 1347 | `decode_audio: OGG Vorbis fixture decodes to the 220 Hz sine` | 143–151 | sweep-tests1 | sweep-tests1-20 | test |  |
| 1348 | `decode_stem: a file-path stem and a bytes stem decode identically` | 153–173 | sweep-tests1 | sweep-tests1-20 | test |  |
| 1349 | `decode_audio: Ogg-Opus fixture decodes to the 220 Hz sine at 48 kHz` | 175–183 | sweep-tests1 | sweep-tests1-20 | test |  |
| 1350 | `decode_audio: Ogg-Opus output is pinned bit for bit` | 188–202 | sweep-tests1 | sweep-tests1-20 | test | FNV pin of Opus output |

### `tests/test_audio_device.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1351 | `PreviewAudioDevice opens and starts the default output device` | 16–26 | sweep-tests1 | none | test | device opens |

### `tests/test_audio_mixer.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1352 | `read_fixture` | 27–30 | sweep-tests1 | none | none | fixture read (duplicate of test_audio_decode read_fixture) |
| 1353 | `make_pcm` | 32–38 | sweep-tests1 | none | none | PCM fixture |
| 1354 | `synth_tone` | 41–51 | sweep-tests1 | none | none | tone fixture |
| 1355 | `estimate_freq_hz` | 54–66 | sweep-tests1 | none | none | test measurement (duplicate of test_audio_decode estimate_freq_hz) |
| 1356 | `reference_mix` | 73–87 | sweep-tests1 | sweep-tests1-7 | copies | audio::mix_stems (test oracle re-implements the summation) |
| 1357 | `same_bits` | 90–96 | sweep-tests1 | none | none | bit compare |
| 1358 | `mix_stems sums same-format stems and zero-extends the shorter` | 100–112 | sweep-tests1 | sweep-tests1-7 | test |  |
| 1359 | `mix_stems of no stems is empty at the requested format` | 114–119 | sweep-tests1 | sweep-tests1-7 | test |  |
| 1360 | `mix_stems resamples to the output rate and unifies channels` | 121–137 | sweep-tests1 | sweep-tests1-7 | test |  |
| 1361 | `decode_and_mix decodes each stem, skips undecodable ones` | 139–167 | sweep-tests1 | sweep-tests1-7 | test |  |
| 1362 | `mix_stems matches the convert-all-then-sum mix bit for bit` | 169–179 | sweep-tests1 | sweep-tests1-7 | test |  |
| 1363 | `decode_and_mix matches decoding every stem then mixing` | 181–206 | sweep-tests1 | sweep-tests1-7 | test |  |

### `tests/test_audio_player.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1364 | `make_ramp` | 18–28 | sweep-tests1 | none | none | ramp fixture |
| 1365 | `Playhead starts paused at the start with the mix's format` | 32–41 | sweep-tests1 | tempo-20;sweep-tests1-8 | test |  |
| 1366 | `play, pause, and toggle drive the playhead state` | 43–53 | sweep-tests1 | sweep-tests1-8 | test |  |
| 1367 | `read_frames while paused writes silence and does not advance` | 55–62 | sweep-tests1 | sweep-tests1-8 | test |  |
| 1368 | `read_frames while playing copies frames and advances the clock` | 64–75 | sweep-tests1 | tempo-20;sweep-tests1-8 | test |  |
| 1369 | `read_frames past the end zero-fills, auto-pauses, clamps position` | 77–90 | sweep-tests1 | sweep-tests1-8 | test |  |
| 1370 | `seek clamps to the valid range in both frames and ms` | 92–102 | sweep-tests1 | tempo-20;sweep-tests1-8 | test |  |
| 1371 | `Playhead applies the output gain to served frames` | 104–120 | sweep-tests1 | sweep-tests1-8 | test |  |

### `tests/test_batch_text.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1372 | `batch text: counts group thousands and pick the right noun` | 12–17 | sweep-tests1 | display-10 | test | count_label |
| 1373 | `batch text: durations read m:ss under an hour and h:mm:ss over it` | 19–25 | sweep-tests1 | sweep-tests1-5 | test |  |
| 1374 | `batch text: the confirm lists the settings a batch runs with` | 27–46 | sweep-tests1 | display-10;display-19;fills-25;store-11;parse-13 | test | does not check the fills line |
| 1375 | `batch text: the empty library says what to do next` | 48–55 | sweep-tests1 | sweep-tests1-21 | test |  |

### `tests/test_chord_code.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1376 | `lane_shapes` | 25–39 | sweep-tests1 | parse-12 | copies | lane_allows_flag (2x hard-coded to Kick; cymbal via allows_cymbals) |
| 1377 | `every chord a chart can express round-trips through its code` | 43–71 | sweep-tests1 | parse-18;parse-12 | test |  |
| 1378 | `a chord's code spells its lanes` | 73–97 | sweep-tests1 | parse-18 | test |  |
| 1379 | `a malformed code is rejected` | 99–102 | sweep-tests1 | parse-18 | test |  |

### `tests/test_cli.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1380 | `run_exe` | 49–83 | sweep-tests1 | none | none | process runner |
| 1381 | `contains` | 85–87 | sweep-tests1 | none | none | substring test |
| 1382 | `small_chart` | 91–107 | sweep-tests1 | parse-1 | copies | chart_format_of (inline case-sensitive ".chart" suffix compare) |
| 1383 | `CliSandbox::CliSandbox` | 116–132 | sweep-tests1 | none | none | sandbox setup |
| 1384 | `CliSandbox::~CliSandbox` | 133–136 | sweep-tests1 | none | none | sandbox teardown |
| 1385 | `CliSandbox::copy_tool` | 137–142 | sweep-tests1 | none | none | file copy |
| 1386 | `CliSandbox::db` | 143–143 | sweep-tests1 | none | none | path |
| 1387 | `CliSandbox::folder` | 144–144 | sweep-tests1 | none | none | path |
| 1388 | `hydra_batch stamps a new database with the rule it ran under` | 149–168 | sweep-tests1 | fills-10;fills-11 | test |  |
| 1389 | `hydra_batch --legacy-fills refuses the tool's own hydra.db` | 170–178 | sweep-tests1 | fills-10 | test |  |
| 1390 | `hydra_batch --reindex keeps a legacy database's stamp` | 180–192 | sweep-tests1 | fills-10 | test |  |
| 1391 | `hydra_batch refuses a run whose fill rule disagrees with the database` | 194–220 | sweep-tests1 | fills-10;screenB-22 | test |  |
| 1392 | `hydra_batch treats an unstamped database with records as Clone Hero 1.1` | 222–243 | sweep-tests1 | screenB-22;store-10 | test | unstamped + records = 1.1 (hydra_batch only) |
| 1393 | `hydra_batch reuses the GUI's scan cache` | 245–265 | sweep-tests1 | sweep-tests1-12 | test |  |
| 1394 | `hydra_report writes a page for a filled database and says so for an empty one` | 267–292 | sweep-tests1 | none | test | hydra_report output |
| 1395 | `hydra_fillcompare compares a 1.0 and a 1.1 database` | 294–322 | sweep-tests1 | score-23;screenB-22;fills-10 | test |  |
| 1396 | `hydra_fillcompare compares both rules out of one database` | 324–349 | sweep-tests1 | score-23;store-10 | test |  |
| 1397 | `hydra_report reports a --legacy-fills database under the 1.0 rule` | 351–368 | sweep-tests1 | screenB-22;store-10;fills-33 | test |  |

### `tests/test_config.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1398 | `temp_ini` | 25–30 | sweep-tests1 | none | none | temp ini path |
| 1399 | `settings round-trip through an INI file` | 34–75 | store | store-16 | test |  |
| 1400 | `the 1.0 fills setting reaches the search and the result's key together` | 77–90 | sweep-tests1 | store-9;store-26;fills-9 | test |  |
| 1401 | `a missing INI yields defaults` | 92–107 | sweep-tests1 | store-16;display-33;tempo-24;screenB-25;sweep-tests1-22 | test | pins defaults 4/10/50/85/40 |
| 1402 | `malformed INI lines are tolerated` | 109–129 | sweep-tests1 | store-16;fills-26 | test | junk depth -> 0 accepted |
| 1403 | `chartmode_key names the view flags` | 131–142 | sweep-tests1 | store-11 | test |  |
| 1404 | `chartmode_key: a non-Expert difficulty is always 1x Bass` | 144–167 | sweep-tests1 | store-11;parse-13;parse-19 | test |  |
| 1405 | `view_difficulty round-trips, and a junk value normalizes to Expert` | 169–190 | sweep-tests1 | parse-19;store-16 | test |  |
| 1406 | `view_difficulty matches any case and loads as the real name` | 192–212 | sweep-tests1 | parse-19 | test |  |
| 1407 | `record_key carries the chartmode, the SP cap and the lens` | 214–248 | store | store-7;store-9 | test |  |
| 1408 | `batch_run bundles one Settings' chartmode, lens and search settings` | 250–268 | store | store-7;store-9 | test |  |
| 1409 | `sp_cap round-trips as a number auto, zero, junk and pre-1.6 keys read as 4` | 270–312 | store | store-8;store-16 | test |  |
| 1410 | `to_analysis_settings maps the cap` | 314–333 | store | store-26 | test |  |
| 1411 | `to_analysis_settings maps depth_mode onto the search's enum` | 338–349 | store | store-9;store-26 | test | pins out-of-range depth_mode -> Scores |

### `tests/test_corpus_cache.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1412 | `corpus cache: one parse and one analysis per chart and settings` | 17–48 | sweep-tests1 | none | test | corpus cache identity |
| 1413 | `corpus cache: a failure is thrown again on every call` | 50–62 | sweep-tests1 | none | test |  |

### `tests/test_dm_report.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1414 | `fill_store` | 45–70 | sweep-tests1 | store-8;store-11;fills-32 | copies | Settings::chartmode_key (kMode literal; CapQuery::at(kCloneHeroSpCap)) |
| 1415 | `make_score` | 72–85 | sweep-tests1 | none | none | score fixture |
| 1416 | `collect_dm_rows joins scores to records and labels them` | 89–122 | display | display-14 | copies | recomputes pct formula |
| 1417 | `collect_dm_rows: no pct off 100% speed store identity fallback` | 124–139 | sweep-tests1 | score-22 | test | no pct off 100% speed |
| 1418 | `dmbot JSON parsers handle canned payloads` | 141–190 | sweep-tests1 | parse-8;sweep-tests1-24 | test |  |
| 1419 | `build_dm_html substitutes every placeholder` | 192–210 | sweep-tests1 | none | test | placeholder substitution |
| 1420 | `generate_dm_report: tally and framing behind one seam` | 212–244 | sweep-tests1 | score-22;screenB-20;display-21 | test |  |
| 1421 | `collect_dm_rows: a blank stored song name reads unknown` | 246–259 | sweep-tests1 | parse-7 | test |  |
| 1422 | `LoopbackServer::LoopbackServer` | 271–286 | sweep-tests1 | none | none | loopback server |
| 1423 | `LoopbackServer::~LoopbackServer` | 287–291 | sweep-tests1 | none | none | loopback server |
| 1424 | `LoopbackServer::url` | 295–295 | sweep-tests1 | none | none | loopback url |
| 1425 | `LoopbackServer::serve_one` | 298–320 | sweep-tests1 | none | none | loopback serve |
| 1426 | `http_response` | 328–332 | sweep-tests1 | none | none | http reply text |
| 1427 | `http transport: cancel aborts a request the server never answers` | 336–370 | sweep-tests1 | sweep-tests1-25 | test | 500 ms latency pass line |
| 1428 | `http transport: a 200 reply is read in full, a 404 keeps its message` | 372–396 | sweep-tests1 | sweep-tests1-25 | test |  |
| 1429 | `collect_dm_rows tells not analyzed from not in library` | 398–433 | sweep-tests1 | score-22;parse-8 | test | case-insensitive md5 join |

### `tests/test_dynamics_breakdown.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1430 | `add_note` | 26–34 | sweep-tests1 | none | none | song builder |
| 1431 | `make_test_song` | 36–76 | sweep-tests1 | none | none | song fixture |
| 1432 | `dynamics_breakdown: hand-built song row counts` | 84–124 | sweep-tests1 | screenA-35;parse-16 | test |  |
| 1433 | `dynamics_breakdown: played_total includes/excludes 2x kick` | 130–150 | sweep-tests1 | display-23;screenA-36 | test |  |
| 1434 | `dynamics_breakdown: Alpha Wolf - Acid Romance real chart` | 158–204 | sweep-tests1 | screenA-35;parse-16;parse-17 | test |  |
| 1435 | `dynamics_breakdown: no ENABLE_CHART_DYNAMICS tag` | 210–220 | sweep-tests1 | parse-17;screenA-37 | test |  |
| 1436 | `dynamics_breakdown: row labels pro vs non-pro` | 226–238 | sweep-tests1 | sweep-tests1-26 | test |  |

### `tests/test_dynamics_store.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1437 | `make_full_breakdown` | 31–41 | sweep-tests1 | none | none | breakdown fixture |
| 1438 | `TempFile::TempFile` | 46–52 | sweep-tests1 | none | none | temp file |
| 1439 | `TempFile::~TempFile` | 53–53 | sweep-tests1 | none | none | temp file |
| 1440 | `dynamics encode/decode round-trip` | 62–77 | sweep-tests1 | store-20 | test |  |
| 1441 | `dynamics decode rejects bad blobs` | 83–100 | sweep-tests1 | store-20 | test | assumes version byte at blob[0] |
| 1442 | `RecordStore dynamics put/get` | 107–164 | sweep-tests1 | parse-23;store-20 | test | hand-built DynamicsKey with literal "Expert" |
| 1443 | `dynamics keys come from one place` | 166–178 | sweep-tests1 | parse-23;parse-13 | test |  |
| 1444 | `dynamics_entry_from_analysis counts only when the parse kept 2x kicks` | 180–200 | sweep-tests1 | parse-13;parse-23 | test |  |
| 1445 | `RecordStore dynamics rows from before the stamp read as missing` | 202–225 | sweep-tests1 | store-20 | test | re-types the pre-stamp dynamics schema SQL |
| 1446 | `RecordStore dynamics rows with another count stamp read as missing` | 227–244 | sweep-tests1 | store-20 | test |  |

### `tests/test_fill_deadline.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1447 | `flat_timing` | 24–28 | fills | none | test | timing helper |
| 1447 | `flat_timing` | 24–28 | tempo | none | test | flat timing fixture |
| 1448 | `ch10` | 30–33 | fills | fills-8 | wrapper | calls owner |
| 1449 | `ch11` | 35–38 | fills | fills-8 | wrapper | calls owner |
| 1450 | `fill deadline: CH 1.0 anchor value` | 42–50 | fills | fills-8 | test | worked value |
| 1451 | `fill deadline: CH 1.0 lead over two fill lengths is 3750/bpm` | 52–65 | fills | fills-8 | test | worked value |
| 1452 | `fill deadline: CH 1.0 pad stays fractional at odd resolutions` | 67–80 | fills | fills-8 | test | worked value |
| 1453 | `fill deadline: CH 1.0 preroll clamps at 250 ms` | 82–93 | fills | fills-8 | copies | recomputes fend - fill_len_ms - 250 (owner activation_fill_deadline_ms) |
| 1454 | `fill deadline: CH 1.0 preroll clamps at 10000 ms` | 95–106 | fills | fills-8 | copies | recomputes fend - fill_len_ms - 10000 (owner activation_fill_deadline_ms) |
| 1455 | `fill deadline: CH 1.1 is unchanged 4-beats-before math` | 108–123 | fills | fills-8 | copies | recomputes fill_end - len - 4*res (owner activation_fill_deadline_ms) |
| 1456 | `fill deadline: the two rules genuinely differ` | 125–136 | fills | fills-8 | test | ordering only |

### `tests/test_fill_report.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1457 | `key_for` | 34–38 | sweep-tests1 | store-9;store-11 | copies | Settings::lens / Lens::from (Lens built by hand legacy?1:0; kMode literal) |
| 1458 | `sample_chart` | 44–63 | sweep-tests1 | none | none | corpus sample (same loop as test_dm_report fill_store) |
| 1459 | `put` | 70–84 | sweep-tests1 | store-13 | test | sets record.legacy_fills and key lens together |
| 1460 | `put_ch10` | 87–89 | sweep-tests1 | store-13 | wrapper | calls put |
| 1461 | `put_ch11` | 91–93 | sweep-tests1 | store-13 | wrapper | calls put |
| 1462 | `compare` | 96–100 | sweep-tests1 | none | wrapper | calls collect_fill_rows |
| 1463 | `find` | 103–107 | sweep-tests1 | none | none | row lookup |
| 1464 | `collect_fill_rows: delta sign and status literals` | 111–135 | sweep-tests1 | score-23;screenB-19;display-21 | test |  |
| 1465 | `collect_fill_rows: 1.0 higher and same` | 137–160 | sweep-tests1 | score-23 | test |  |
| 1466 | `collect_fill_rows: a chart in one database only still gets a row` | 162–187 | sweep-tests1 | score-23 | test |  |
| 1467 | `collect_fill_rows: each side reads only its own rule, even from one store` | 189–209 | sweep-tests1 | fills-10;store-10 | test |  |
| 1468 | `tally_fill_rows counts every status` | 211–228 | sweep-tests1 | display-21 | test |  |
| 1469 | `build_fill_html substitutes every placeholder` | 230–252 | sweep-tests1 | none | test |  |
| 1470 | `generate_fill_report: tally and framing behind one seam` | 254–284 | sweep-tests1 | score-23 | test |  |
| 1471 | `collect_fill_rows: a blank stored song name reads unknown` | 286–300 | sweep-tests1 | parse-7 | test |  |

### `tests/test_highway_draw.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1472 | `note` | 21–30 | sweep-tests1 | none | none | note fixture (one tick per ms) |
| 1473 | `span` | 32–39 | sweep-tests1 | none | none | span fixture |
| 1474 | `fill` | 41–46 | sweep-tests1 | none | none | fill fixture |
| 1475 | `timed_scene` | 51–56 | sweep-tests1 | none | none | scene fixture |
| 1476 | `transform` | 58–63 | sweep-tests1 | none | none | matrix helper |
| 1477 | `of_mesh` | 65–70 | sweep-tests1 | none | none | filter helper |
| 1478 | `time_to_z: Onyx's linear time->depth map` | 74–83 | sweep-tests1 | sweep-tests1-27;tempo-28 | test |  |
| 1479 | `pad_x: the note area split into four lanes, left to right` | 85–100 | sweep-tests1 | sweep-tests1-27 | test |  |
| 1480 | `make_camera: Onyx's tilted view and right-handed projection` | 102–123 | sweep-tests1 | sweep-tests1-27 | test |  |
| 1481 | `stretch_matrix and light_for` | 125–151 | sweep-tests1 | sweep-tests1-27 | test |  |
| 1482 | `build_highway_draws: order and geometry for a frame` | 153–264 | sweep-tests1 | sweep-tests1-27;tempo-23 | test | half-tick edge typed as 1.5005 |
| 1483 | `build_highway_draws: a Red gem just ahead fills Onyx's box` | 266–283 | sweep-tests1 | sweep-tests1-27 | test |  |
| 1484 | `build_highway_draws: ghost shrinks 70% and overlays accent overlays` | 285–305 | sweep-tests1 | sweep-tests1-27 | test |  |
| 1485 | `build_highway_draws: a ghost kick keeps full width and overlays` | 307–324 | sweep-tests1 | sweep-tests1-27 | test |  |
| 1486 | `build_highway_draws: hit flash and target glow after a note passes` | 326–368 | sweep-tests1 | sweep-tests1-27 | copies | PreviewConfig targets_secs_light (re-types 0.1666666 in the glow alpha) |
| 1487 | `build_highway_draws: energy gems inside an SP phrase, tinted floor in an active window` | 370–403 | sweep-tests1 | sweep-tests1-10;screenB-3;display-24 | test |  |
| 1488 | `build_highway_draws: the taken fill lights its lane with the lit target` | 405–435 | sweep-tests1 | screenB-10;display-25;tempo-23 | test |  |
| 1489 | `build_highway_draws: an offered fill's strips are dimmed` | 437–454 | sweep-tests1 | screenB-10 | test |  |
| 1490 | `build_highway_draws: a hidden fill draws no lane strips` | 456–468 | sweep-tests1 | screenB-10 | test |  |
| 1491 | `build_highway_draws: an empty window still draws floor, railings and targets` | 470–480 | sweep-tests1 | sweep-tests1-27 | test |  |
| 1492 | `texture_file names every texture` | 482–486 | sweep-tests1 | sweep-tests1-27 | test |  |
| 1493 | `build_highway_draws: a chord one tick after a phrase ends draws plain` | 488–518 | sweep-tests1 | sweep-tests1-10;tempo-23 | test |  |

### `tests/test_image_decode.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1494 | `decode_image decodes a 2x2 PNG to RGBA with the right pixels` | 31–47 | sweep-tests2 | none | test | PNG decode plumbing |
| 1495 | `decode_image returns an empty image on garbage bytes` | 49–55 | sweep-tests2 | none | test | decode error path |
| 1496 | `decode_image handles empty input` | 57–60 | sweep-tests2 | none | test | decode empty input |

### `tests/test_label_layout.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1497 | `check_spaced` | 17–26 | sweep-tests2 | none | test | invariant checker for ui::spaced_labels (no overlap, on strip); checks properties, does not recompute positions |
| 1498 | `label layout: labels with room all show, centred on their marks` | 30–38 | sweep-tests2 | none | test | spaced_labels centring; literal expected values |
| 1499 | `label layout: 39 crowded numbers never overlap, first and last show` | 40–58 | sweep-tests2 | none | test | spaced_labels crowding |
| 1500 | `label layout: labels at the ends stay on the strip` | 60–66 | sweep-tests2 | none | test | spaced_labels clamp to strip |
| 1501 | `label layout: the last label drops the ones it would cover, never the first` | 68–82 | sweep-tests2 | none | test | spaced_labels drop rule |

### `tests/test_library_jobs.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1502 | `wait_until` | 38–45 | sweep-tests2 | none | none | poll helper |
| 1503 | `fake_charts` | 47–58 | sweep-tests2 | none | none | fixture builder |
| 1504 | `test_run` | 60–64 | sweep-tests2 | none | none | fixture builder |
| 1505 | `gated_failure` | 68–75 | sweep-tests2 | none | none | fake analyzer |
| 1506 | `jobs: the batch clock leaves paused time out` | 79–96 | sweep-tests2 | none | test | BatchClock pause accounting via production |
| 1507 | `jobs: time left needs three finished charts` | 98–104 | sweep-tests2 | sweep-tests2-1 | test | calls ui::batch_eta_s; literal 3 restates kEtaMinFinished |
| 1508 | `jobs: pause lets the chart in flight finish and starts no new one` | 106–132 | sweep-tests2 | sweep-tests2-1 | test | BatchJob pause; eta hidden while paused (library_jobs.cpp:221) |
| 1509 | `jobs: stop while paused ends the run and fails nothing` | 134–155 | sweep-tests2 | none | test | BatchJob stop |
| 1510 | `jobs: the snapshot names the chart being analyzed and freezes its clock at the end` | 157–180 | sweep-tests2 | none | test | snapshot current chart |
| 1511 | `jobs: a failed chart reads in plain words and keeps the raw text` | 182–198 | sweep-tests2 | display-28 | test | plain_error_text via BatchJob; expected sentence literal |
| 1512 | `jobs: a report the browser refuses is saved, not failed` | 200–233 | sweep-tests2 | none | test | publish_report outcome |
| 1513 | `jobs: a cancelled leaderboard fetch is not an error` | 235–270 | sweep-tests2 | display-28 | test | cancelled -> empty message (DmReportJob) |
| 1514 | `jobs: a failed leaderboard fetch says what to do` | 272–284 | sweep-tests2 | display-28 | test | network error -> kNetUnreachable via DmFetchUsersJob |

### `tests/test_library_model.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1515 | `chart` | 25–34 | sweep-tests2 | none | none | fixture builder |
| 1516 | `ready` | 36–44 | sweep-tests2 | none | none | fixture: stored summary values typed in |
| 1517 | `stale` | 46–50 | sweep-tests2 | none | none | fixture |
| 1518 | `sample` | 54–67 | sweep-tests2 | none | none | fixture library |
| 1519 | `titles` | 69–73 | sweep-tests2 | none | none | accessor |
| 1520 | `library model: every chart shows, sorted by title, with its Best path text` | 77–90 | sweep-tests2 | display-11;display-9;screenA-1;screenA-2;display-10 | test | best_path_label (library_model.cpp:61) composes stored score + bestpath; strip_rich_tags via set_charts; chip counts |
| 1521 | `library model: the search narrows the rows and the chip counts follow it` | 92–118 | sweep-tests2 | display-31;score-15;squeeze-28 | test | query via LibraryModel; stars:/squeeze<= read stored PathSummary (facts_of) |
| 1522 | `library model: a status chip narrows the rows, and an emptied chip falls back to All` | 120–136 | sweep-tests2 | none | test | chip fallback |
| 1523 | `library model: Best path sorts by score, with unscored rows last both ways` | 138–151 | sweep-tests2 | sweep-tests2-2 | test | unscored_rank order Ready<Stale<NotAnalyzed |
| 1524 | `library model: one chart in two folders gets both rows updated` | 153–161 | sweep-tests2 | parse-22 | test | same md5 in two folders: set_summary_for updates both rows |
| 1525 | `library model: filtering 20,000 charts takes under 20 ms` | 163–202 | sweep-tests2 | none | test | perf timing; 20 ms budget literal |

### `tests/test_library_query.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1526 | `analyzed` | 26–31 | sweep-tests2 | none | none | fixture: RowFacts |
| 1527 | `burnout` | 33–36 | sweep-tests2 | none | none | fixture row (make_searchable) |
| 1528 | `deadbolt` | 38–40 | sweep-tests2 | none | none | fixture row |
| 1529 | `acid_romance` | 42–45 | sweep-tests2 | none | none | fixture row |
| 1530 | `spans_equal` | 47–52 | sweep-tests2 | none | none | span compare |
| 1531 | `library query: folding lowercases and removes accents` | 56–63 | sweep-tests2 | sweep-tests2-3 | test | fold_for_search |
| 1532 | `library query: folding turns full-width ASCII into ASCII` | 65–69 | sweep-tests2 | sweep-tests2-3 | test | fold_for_search full-width |
| 1533 | `library query: folding collapses whitespace and keeps other scripts` | 71–76 | sweep-tests2 | sweep-tests2-3 | test | fold_for_search whitespace |
| 1534 | `library query: invalid UTF-8 passes through byte for byte` | 78–83 | sweep-tests2 | sweep-tests2-3 | test | fold_for_search invalid UTF-8 |
| 1535 | `library query: rich-text tags are stripped and other angle brackets kept` | 85–94 | sweep-tests2 | display-9 | test | strip_rich_tags pins b/i/u/s/size/sub/sup removal; report::plain removes only color tags |
| 1536 | `library query: a tagged charter is found by its plain name` | 96–103 | sweep-tests2 | display-9 | test | make_searchable strips tags |
| 1537 | `library query: an accented name is found without its accents` | 105–110 | sweep-tests2 | sweep-tests2-3 | test | accent folding in match |
| 1538 | `library query: words match in any order and across fields` | 112–120 | sweep-tests2 | sweep-tests2-4 | test | query_matches word AND |
| 1539 | `library query: a quoted phrase must appear whole in one field` | 122–142 | sweep-tests2 | sweep-tests2-4 | test | phrase rule |
| 1540 | `library query: artist: charter: folder: and title: limit a term to one field` | 144–165 | sweep-tests2 | sweep-tests2-4 | test | field prefixes |
| 1541 | `library query: unknown field names are words and empty field values are dropped` | 167–179 | sweep-tests2 | sweep-tests2-4 | test | unknown field / empty value |
| 1542 | `library query: stars:N keeps analyzed rows with exactly N stars` | 181–192 | sweep-tests2 | score-15;score-16 | test | stars:N exact on stored stars; range 0..kMaxStars |
| 1543 | `library query: squeeze<=N keeps analyzed rows whose hardest squeeze is at most N ms` | 194–209 | sweep-tests2 | display-31;squeeze-28 | test | squeeze<=N inclusive; '<' reads as '<='; nullopt hardest passes |
| 1544 | `library query: bad filter values give a plain error and filter nothing` | 211–234 | sweep-tests2 | score-16 | test | error texts; kStarsError literal 'from 0 to 7' vs kMaxStars |
| 1545 | `library query: an empty query matches every row` | 236–247 | sweep-tests2 | sweep-tests2-4 | test | empty query |
| 1546 | `library query: highlight spans cover the displayed bytes` | 249–281 | sweep-tests2 | sweep-tests2-5 | test | match_spans byte spans |
| 1547 | `library query: matching 20000 rows takes well under a frame` | 283–309 | sweep-tests2 | none | test | perf; 20 ms literal |

### `tests/test_midi.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1548 | `event_view` | 24–49 | sweep-tests2 | sweep-tests2-6 | test | absolute-tick view of parsed MidiFile (sums deltas; viewer, not a production value) |
| 1549 | `midi: every corpus .mid reads with a sane structure` | 56–81 | sweep-tests2 | sweep-tests2-6;parse-9 | test | MidiFile::from_file corpus smoke; test types its own PART DRUMS literal (expects every corpus mid has it) |
| 1550 | `midi: running status and zero-velocity note_on` | 83–99 | sweep-tests2 | sweep-tests2-6 | test | running status, vel-0 note_on |
| 1551 | `midi: running status survives a meta event` | 101–121 | sweep-tests2 | sweep-tests2-6 | test | running status across meta (mido semantics) |
| 1552 | `midi: first track_name wins over later ones` | 123–136 | sweep-tests2 | sweep-tests2-6;parse-9 | test | first track_name wins |
| 1553 | `midi: sysex and skipped metas do not lose time` | 138–153 | sweep-tests2 | sweep-tests2-6 | test | sysex skipped without losing time |
| 1554 | `midi: track_name is not exposed as text` | 155–177 | sweep-tests2 | sweep-tests2-6 | test | track_name exposed as name not text |
| 1555 | `midi: non-MIDI input is rejected` | 179–182 | sweep-tests2 | sweep-tests2-6 | test | rejects non-MIDI |
| 1556 | `midi: SMPTE division is rejected` | 184–190 | sweep-tests2 | sweep-tests2-6 | test | rejects SMPTE |
| 1557 | `midi: a five-byte delta accumulates past 32 bits, as mido does` | 192–203 | sweep-tests2 | sweep-tests2-6 | test | five-byte delta |
| 1558 | `midi: a message longer than mido's 1,000,000-byte cap refuses the file, as mido does` | 205–228 | sweep-tests2 | sweep-tests2-6 | test | 1,000,000-byte message cap (mido) |

### `tests/test_model.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1559 | `basescore matches ChordNote.basescore` | 23–32 | score | score-1 | test | basescore literals |
| 1560 | `note value: one owner for base, cymbal and dynamic points` | 34–42 | score | score-1 | test | constants pinned |
| 1561 | `category_scores: the squeeze-out cut is basescore at each note's multiplier` | 44–78 | score | score-5 | copies | test recomputes sqout cut as basescore*to_multiplier(combo+1+i) |
| 1562 | `group_thousands matches Python :,` | 80–87 | sweep-tests2 | display-10 | test | group_thousands |
| 1563 | `squeeze symbols, timing, difficulty` | 89–99 | sweep-tests2 | squeeze-4;squeeze-32 | test | SPSqueeze symbol/difficulty/timing via production |
| 1564 | `squeeze_difficulty and is_e0: one owner for the engine and the model` | 101–120 | fills | fills-14;fills-37 | test | calls owners |
| 1565 | `Path::is_difficult: past the difficult floor, not at it` | 122–137 | sweep-tests2 | squeeze-7;display-4;screenA-5 | test | Path::is_difficult strict > kDifficultMs |
| 1566 | `Activation notationstr: E prefix, skips, symbols` | 139–157 | fills | fills-13 | test | boundary cases |
| 1567 | `Path pathstring and pathstring_verbose` | 159–180 | sweep-tests2 | screenA-2;screenA-41;display-10 | test | pathstring / pathstring_verbose |
| 1568 | `Path::walk_activations: own activations then the variant tail, in place` | 184–219 | sweep-tests2 | none | test | walk_activations structure |
| 1569 | `display_backends drops rows beyond a squeeze out` | 225–251 | sweep-tests2 | squeeze-30;backend-8;squeeze-22;backend-3 | test | display_backends: window + beyond-sqout trim; comment restates +/-500 |
| 1570 | `Chord rowstr / notationstr / disco flip` | 253–269 | sweep-tests2 | parse-15;parse-12;sweep-tests2-7 | test | Chord rowstr GreenTom on non-pro green; disco flip |
| 1571 | `a ghost or accent kick scores double, like a pad` | 271–279 | sweep-tests2 | score-1;score-26 | test | ChordNote::basescore kick dynamics doubled |
| 1572 | `ChordNote::str shows the kick's dynamic and its 2x flag` | 281–299 | sweep-tests2 | sweep-tests2-7 | test | ChordNote::str spelling |
| 1573 | `Chord::code spells a ghost/accent kick in the kick lane` | 301–320 | sweep-tests2 | parse-18 | test | Chord::code / from_code |
| 1574 | `backend_row_value: every engine case` | 325–367 | backend | backend-1;backend-2;backend-3;backend-4 | test | calls owners |
| 1575 | `difficulty names: one list, one spelling` | 369–376 | sweep-tests2 | parse-19 | test | difficulty_name list |
| 1576 | `difficulty_from_name: any case, nothing else` | 378–386 | sweep-tests2 | parse-19 | test | difficulty_from_name |
| 1577 | `no_notes_message names the difficulty and the drum mode` | 388–391 | sweep-tests2 | display-28 | test | no_notes_message; user_messages is_no_notes_message re-matches its shape |
| 1578 | `title_or_unknown: one fallback for a song with no usable name` | 393–399 | sweep-tests2 | parse-7;parse-6 | test | title_or_unknown; legacy '<unknown title>' folded |
| 1579 | `MultSqueeze accepts exactly the 2- and 3-note chords that straddle a multiplier step` | 405–431 | score | score-17 | copies | test derives straddle from to_multiplier (the rule applies should use) |
| 1580 | `MultSqueeze::howto names the lone note of a 3-note chord` | 436–468 | sweep-tests2 | screenA-27 | test | MultSqueeze::howto |
| 1581 | `MultSqueeze::applies answers exactly when the constructor accepts` | 472–492 | sweep-tests2 | score-17;score-2 | test | applies vs constructor |

### `tests/test_obj_loader.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1582 | `zsum` | 34–37 | sweep-tests2 | none | none | z-sum helper for sort-order property |
| 1583 | `load_obj fan-triangulates quads and keeps per-corner attributes` | 41–63 | sweep-tests2 | none | test | load_obj |
| 1584 | `load_obj sorts triangles far-first ascending z sum` | 65–78 | sweep-tests2 | none | test | load_obj sort property |
| 1585 | `load_obj handles v//vn, bare v, negative indices, and CRLF` | 80–90 | sweep-tests2 | none | test | load_obj formats |
| 1586 | `load_obj rejects text with no faces and bad indices` | 92–95 | sweep-tests2 | none | test | load_obj errors |
| 1587 | `make_flat_quad: unit square at y=0, +Y normal, Onyx UV layout` | 97–121 | sweep-tests2 | none | test | make_flat_quad |
| 1588 | `make_box: four outward faces, no bottom or back` | 123–148 | sweep-tests2 | none | test | make_box |
| 1589 | `the copied Onyx drum models load with their known extents` | 150–174 | sweep-tests2 | none | test | Onyx model extents |

### `tests/test_overlay_layout.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1590 | `track_height: the width ratio, capped by the image height` | 16–21 | sweep-tests2 | sweep-tests2-9 | test | track_height |
| 1591 | `highway_span_at: passes through the projected railing corners` | 23–40 | sweep-tests2 | sweep-tests2-9 | test | highway_span_at vs project_to_image |
| 1592 | `highway_span_at: symmetric, and wider nearer the camera` | 42–50 | sweep-tests2 | sweep-tests2-9 | test | highway_span_at symmetry |
| 1593 | `highway_span_at: a narrower image leaves the highway's size alone` | 52–60 | sweep-tests2 | sweep-tests2-9 | test | highway_span_at narrow image |
| 1594 | `overlay_scale: full size with room, floored when narrow, clear in between` | 62–98 | sweep-tests2 | sweep-tests2-9 | test | overlay_scale; property check re-states the room inequality (highway_span_at - gap) rather than a value |
| 1595 | `ten_per_char` | 103–108 | sweep-tests2 | none | none | fixed-pitch width stand-in |
| 1596 | `wrap_words: one line when it fits, otherwise broken at spaces` | 111–136 | sweep-tests2 | sweep-tests2-8 | test | wrap_words |
| 1597 | `wrap_words: the last words kept together` | 138–149 | sweep-tests2 | sweep-tests2-8 | test | wrap_words keep_last |
| 1598 | `widest_word: the widest run between spaces, the last words as one` | 151–158 | sweep-tests2 | sweep-tests2-8 | test | widest_word |
| 1599 | `next-activation box: off the lane at the bottom, one scale for the whole path` | 160–206 | sweep-tests2 | sweep-tests2-9;sweep-tests2-8 | copies | line 174 recomputes bottom_left_room as highway_span_at(h).left - gap; owner render::bottom_left_room |
| 1600 | `ellipsize: whole when it fits, cut and ended in an ellipsis when not` | 208–221 | sweep-tests2 | sweep-tests2-8 | test | ellipsize |
| 1601 | `ellipsize: a 40-activation path fits its line, and a cut never splits a character` | 223–239 | sweep-tests2 | sweep-tests2-8 | test | ellipsize UTF-8 |

### `tests/test_path_codec.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1602 | `fixture` | 39–64 | sweep-tests2 | screenB-25 | none | fixture: types its own analysis settings (cap 4, Scores, depth 4, ms 10) instead of reading app::Settings defaults |
| 1603 | `diff_summary` | 67–78 | sweep-tests2 | store-22 | none | field-by-field PathSummary compare (no computation) |
| 1604 | `collect_payloads` | 82–86 | sweep-tests2 | store-24 | none | collects encode_path_node payloads (production) |
| 1605 | `distinct_payloads` | 88–93 | sweep-tests2 | store-24 | none | distinct payload set |
| 1606 | `pathstrings` | 95–100 | sweep-tests2 | screenA-2 | none | pathstring list |
| 1607 | `path codec: a rebuilt record flattens to the same bytes` | 104–144 | sweep-tests2 | store-19;store-22;store-12 | test | flatten/rebuild round trip |
| 1608 | `path codec: a node carries activations only, never totals` | 148–160 | sweep-tests2 | store-19 | test | node excludes totals |
| 1609 | `path codec: root totals ride in the structure, once per root` | 162–179 | sweep-tests2 | store-19 | test | root totals in structure |
| 1610 | `path codec: the multiplier squeezes are stored once per record` | 181–200 | sweep-tests2 | store-19 | copies | expects structure growth 2*13 bytes (4-byte length + 5-char code + 4-byte combo): restates the multsqueeze wire layout owned by flatten_record/write_* in path_codec.cpp |
| 1611 | `path codec: a record with no paths round-trips` | 202–217 | sweep-tests2 | store-19 | test | empty record round trip |
| 1612 | `path codec: node payloads are flat and content-addressed` | 219–258 | store | store-24 | test |  |
| 1613 | `path codec: a node in an older layout is rejected` | 262–266 | store | store-1 | test |  |
| 1614 | `path codec: flattening dedups and is stable` | 268–289 | sweep-tests2 | store-24;store-19 | test | dedup + stable hashes via path_hash |
| 1615 | `path codec: a missing node or a bad structure blob throws` | 291–352 | store | store-1;store-5 | test |  |

### `tests/test_path_view.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1616 | `analyzed` | 25–42 | sweep-tests2 | none | none | fixture: first analyzable corpus chart; types its own settings (Scores, depth 10, ms 10) |
| 1617 | `burnout` | 46–58 | sweep-tests2 | none | none | fixture: Burnout; types settings (depth 2, ms 10) to mirror the GUI tests |
| 1618 | `build_record_status: the three states and their lines` | 65–112 | sweep-tests2 | display-11;display-30;display-10;store-12;screenA-1;screenA-38 | test | build_record_status lines; expected 'Paths kept' recomputed as all_paths().size() (same expression as production) |
| 1619 | `build_score_breakdown: exact lines, rounded like the report` | 114–126 | sweep-tests2 | screenA-28;display-3 | test | build_score_breakdown |
| 1620 | `display format: the average multiplier rounds the exact double` | 128–134 | sweep-tests2 | display-3 | test | format_avg_mult / py_round3 |
| 1621 | `display format: ms text is one decimal and a unit` | 136–140 | sweep-tests2 | display-1 | test | format_ms |
| 1622 | `build_activations: the early fill reads positive = early on both lines` | 142–177 | display | display-2 | test | pins positive = early (contradicts UserGuide line 118) |
| 1623 | `path buttons: each path's own hardest timing, warn past the difficult floor` | 179–207 | display | display-4 | test | warn floor |
| 1624 | `build_path_list: score groups and the all-0 dedupe rule` | 209–238 | display | display-32 | copies | expected label rebuilt with group_thousands |
| 1625 | `build_activations: rows and backend rows line up` | 240–263 | sweep-tests2 | screenA-10;screenA-20;squeeze-30;screenA-5 | test | build_activations rows vs notationstr/is_difficult/display_backends (calls owners) |
| 1626 | `build_activations: the scale line shows every multiplier, early first` | 265–334 | sweep-tests2 | squeeze-18;squeeze-19;screenA-23;screenA-24;display-17;squeeze-12;squeeze-16 | test | scale line text and warn |
| 1627 | `build_activations: overfill warning text` | 336–381 | sweep-tests2 | screenA-25;tempo-6 | test | overfill warning measure text |
| 1628 | `build_activations: the backend limit hides far rows but never  "squeezed-out ones"` | 384–428 | backend | backend-11;backend-13 | test |  |
| 1629 | `build_activations: a squeezed-out row past the leeway is worth 0` | 430–472 | backend | backend-4;backend-13 | test |  |
| 1630 | `build_activations: plain rows past the leeway show 0` | 476–519 | backend | backend-2;backend-9;backend-15 | test |  |
| 1631 | `find a chart with an uncounted squeezed-out row * doctest::skip()` | 523–549 | sweep-tests2 | backend-15;screenA-21 | test | skipped finder; matches rating text 'squeezed out (uncounted)' |
| 1632 | `build_multsqueezes: one labeled entry per squeeze` | 551–559 | sweep-tests2 | screenA-27 | test | build_multsqueezes |
| 1633 | `PathsTabCache: views are built once and rebuilt only when their inputs move` | 561–608 | sweep-tests2 | screenA-2 | test | PathsTabCache rebuild keys |
| 1634 | `PathsTabCache: 600 cached frames cost far less than 600 rebuilds` | 610–638 | sweep-tests2 | none | test | perf |
| 1635 | `format_measure: one form for both tabs` | 640–650 | sweep-tests2 | screenA-10;tempo-6;display-1 | test | format_measure / format_ms_spaced |
| 1636 | `activation rows: Burnout's three activations` | 652–696 | sweep-tests2 | screenA-9;screenA-10;screenA-11;screenA-15;screenA-16;display-10;score-5;backend-5 | test | Burnout rows: badge integer ms vs button 1-decimal; '260 fewer points' |
| 1637 | `activation timeline: onset over the song's length, and the end measure` | 698–725 | sweep-tests2 | display-12;tempo-13;tempo-14;screenA-13;tempo-2 | test | song_fraction = onset/length; timeline_end from length via tick round trip |
| 1638 | `activation badge: shown for a squeeze or an early fill` | 727–757 | sweep-tests2 | screenA-11;squeeze-29;display-6;fills-38;display-1;squeeze-5 | test | activation_badge picks squeeze vs fill (E0 fill outranks, optional fill loses to a needed squeeze), %.0f ms |
| 1639 | `squeeze sentences: SqIn, SqOut, and what a squeeze-out costs` | 759–837 | sweep-tests2 | squeeze-10;screenA-15;screenA-16;backend-13;squeeze-24;display-13;squeeze-32;backend-2;squeeze-15 | test | squeeze_sentences wording, cost via backend_row_value |
| 1640 | `backend table: a counted row inside SP shows its early-scale eff. figure` | 839–864 | sweep-tests2 | squeeze-15;screenA-22;backend-10;screenA-26 | test | eff. figure in backend row |
| 1641 | `path buttons: Burnout's list, in the mockup's groups` | 866–894 | sweep-tests2 | screenA-3;screenA-6;screenA-8;display-32;score-20;screenA-4;screenA-5;display-1 | test | build_path_buttons groups, '2,360 below optimal', within_label |
| 1642 | `multiplier squeeze: Burnout's one squeeze and the fold's summary` | 896–906 | sweep-tests2 | screenA-27 | test | multsqueeze label/summary |
| 1643 | `PathsTabUi: one row open at a time, expand and collapse all` | 908–932 | sweep-tests2 | none | test | PathsTabUi folds |
| 1644 | `PathsTabCache: folds reset for a new path, not for a display setting` | 934–967 | sweep-tests2 | none | test | cache fold reset |

### `tests/test_preview_clock.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1645 | `PreviewClock: frozen while paused, wall-clock while playing` | 10–33 | sweep-tests2 | sweep-tests2-10 | test | PreviewClock play/pause |
| 1646 | `PreviewClock: seek sets the song time, playing or not` | 35–56 | sweep-tests2 | sweep-tests2-10 | test | PreviewClock seek |

### `tests/test_preview_config.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1647 | `check_color` | 20–25 | sweep-tests2 | none | none | colour compare helper |
| 1648 | `check_light` | 27–34 | sweep-tests2 | none | none | light compare helper |
| 1649 | `check_is_onyx` | 37–75 | sweep-tests2 | sweep-tests2-11 | test | types Onyx 3d-config.yml values a third time to check struct defaults and JSON |
| 1650 | `PreviewConfig defaults are Onyx's 3d-config.yml values` | 79–84 | sweep-tests2 | sweep-tests2-11 | test | struct defaults == Onyx values |
| 1651 | `the shipped 3d-config.json loads to the Onyx values` | 86–94 | sweep-tests2 | sweep-tests2-11 | test | shipped JSON == Onyx values |
| 1652 | `load_preview_config keeps defaults for missing keys and overrides present ones` | 96–102 | sweep-tests2 | sweep-tests2-11 | test | missing keys keep defaults |
| 1653 | `load_preview_config rejects malformed JSON` | 104–106 | sweep-tests2 | none | test | malformed JSON |
| 1654 | `parse_hex_color follows Onyx stackColor` | 108–113 | sweep-tests2 | sweep-tests2-12 | test | parse_hex_color |
| 1655 | `load_preview_config reads the time box size and margin` | 115–120 | sweep-tests2 | sweep-tests2-11 | test | time box size/margin |

### `tests/test_preview_controller.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1656 | `wait_finished` | 45–48 | sweep-tests2 | none | none | poll helper |
| 1657 | `entry_for` | 50–56 | sweep-tests2 | none | none | fixture |
| 1658 | `copy_file_utf8` | 58–64 | sweep-tests2 | none | none | file copy helper |
| 1659 | `chart_with_audio` | 69–79 | sweep-tests2 | none | none | temp chart folder with audio |
| 1660 | `a cancelled Preview load stops before decoding` | 85–94 | sweep-tests2 | none | test | PreviewLoadJob cancel; literal cap 4 |
| 1661 | `a cancelled Dynamics load stops before counting` | 96–104 | sweep-tests2 | none | test | DynamicsLoadJob cancel |
| 1662 | `with no audio device the Preview still loads, muted, with a warning` | 108–131 | sweep-tests2 | tempo-12 | test | no audio device: loads muted; length_ms > 0 |
| 1663 | `switching paths builds the new overlay off the UI thread` | 135–182 | sweep-tests2 | none | test | overlay built off UI thread; fixture analysis settings typed (Scores, 2, 10 ms) |

### `tests/test_preview_golden.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1664 | `write_bmp` | 55–76 | sweep-tests2 | none | none | BMP writer (dev aid) |
| 1665 | `render_chart` | 80–108 | sweep-tests2 | none | none | render helper; fixture settings typed (Scores, depth 10, ms 10) |
| 1666 | `half_res` | 111–125 | sweep-tests2 | none | none | box downsample for golden compare |
| 1667 | `preview golden: a Hydra frame matches the Onyx screenshot` | 129–204 | sweep-tests2 | none | test | golden MAE vs Onyx screenshot; tolerance fallback 20.0 |
| 1668 | `preview dump dev aid, HYDRA_PREVIEW_DUMP` | 206–259 | sweep-tests2 | sweep-tests2-13;tempo-6;tempo-3 | copies | dev-aid dump decides 'note in SP phrase' itself (s.start_ms <= n.ms <= s.end_ms over scene.sp_phrases); owner: the parser's SP-phrase membership carried on the chart (Song timestamps is_sp) / build_pr |

### `tests/test_preview_renderer.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1669 | `is_background` | 33–35 | sweep-tests2 | sweep-tests2-11 | none | Onyx background #1c1d2b typed again with +/-2 tolerance |
| 1670 | `count_non_background` | 37–42 | sweep-tests2 | none | none | pixel counter |
| 1671 | `note_at` | 44–51 | sweep-tests2 | none | none | fixture: tick == ms |
| 1672 | `PreviewRenderer: bare highway — background outside, lit floor, horizon fade WARP` | 55–92 | sweep-tests2 | sweep-tests2-11 | test | bare highway; top 16% restates track_fade_top 0.8316 |
| 1673 | `PreviewRenderer: a gem at the strike line adds drawn pixels WARP` | 94–123 | sweep-tests2 | none | test | gem draws pixels |
| 1674 | `PreviewRenderer: SP phrase energy gems and active SP floor change pixels WARP` | 125–169 | sweep-tests2 | screenB-3;tempo-23 | test | SP floor tint while act.ms <= now < sp_end_ms; phrase span end half tick |
| 1675 | `PreviewRenderer: resize and a tall target keep the track at the bottom WARP` | 171–188 | sweep-tests2 | sweep-tests2-9 | copies | computes the track height by hand (100 * 1.1666 = 117) instead of calling render::track_height |
| 1676 | `PreviewRenderer: a missing asset dir is a clear error` | 190–195 | sweep-tests2 | none | test | missing asset dir error |

### `tests/test_preview_source.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1677 | `write_bytes` | 36–41 | sweep-tests2 | none | none | file write helper |
| 1678 | `fixture_dir` | 44–54 | sweep-tests2 | none | none | temp dir |
| 1679 | `make_subdir` | 56–60 | sweep-tests2 | none | none | temp subdir |
| 1680 | `push_u32` | 62–64 | sweep-tests2 | store-6 | none | little-endian u32 writer for fixtures (own copy of BinaryWriter's LE rule) |
| 1681 | `push_u64` | 65–67 | sweep-tests2 | store-6 | none | little-endian u64 writer for fixtures |
| 1682 | `deflate_raw` | 69–78 | sweep-tests2 | none | none | miniz raw deflate wrapper |
| 1683 | `bytes_of` | 80–82 | sweep-tests2 | none | none | string to bytes |
| 1684 | `push_str` | 150–153 | sweep-tests2 | parse-25 | copies | u32-length string writer re-encoding the .srb metadata string layout; owner srb_parse_metadata (src/parse/srb.h) |
| 1685 | `make_metadata` | 155–163 | sweep-tests2 | parse-25 | copies | re-encodes the .srb metadata block ('4b4' magic, notes filename, 7 strings, 16 bytes); owner srb_parse_metadata |
| 1686 | `make_srb` | 169–181 | parse | parse-25 | copies | second test srb encoder |
| 1687 | `chart_with_offset` | 184–191 | sweep-tests2 | parse-27;tempo-11 | none | fixture: chart with Offset line |
| 1688 | `is_audio_filename / looks_like_audio recognize the formats` | 195–211 | sweep-tests2 | sweep-tests2-14 | test | is_audio_filename / looks_like_audio |
| 1689 | `find_loose_audio: stems beside the notes, preview and art excluded` | 213–229 | sweep-tests2 | sweep-tests2-14 | test | find_loose_audio excludes preview and art |
| 1690 | `extract_sng_audio: audio entries come back XOR-demasked` | 231–255 | sweep-tests2 | parse-24;parse-3;sweep-tests2-14 | test | extract_sng_audio XOR demask (fixture make_sng is the parse-24 copy) |
| 1691 | `extract_srb_audio: trailing audio streams inflate art is skipped` | 257–279 | sweep-tests2 | parse-25;parse-3;sweep-tests2-14 | test | extract_srb_audio trailing streams |
| 1692 | `resolve_preview_source: a .srb with no extractable audio falls  "back to a loose file be` | 282–299 | sweep-tests2 | parse-1;parse-3 | test | .srb with no audio falls back to loose file |
| 1693 | `resolve_preview_source: a loose chart parses and finds its audio` | 301–313 | sweep-tests2 | parse-1;sweep-tests2-14 | test | loose chart audio |
| 1694 | `containers pass the difficulty through to the chart inside` | 315–345 | sweep-tests2 | parse-19;parse-3;parse-1 | test | difficulty through containers |
| 1695 | `read_ini_delay_ms reads song.ini delay in milliseconds` | 347–366 | sweep-tests2 | parse-5;parse-27;tempo-11;screenB-31 | test | read_ini_delay_ms |
| 1696 | `preview_audio_offset_ms combines delay and Offset` | 368–378 | sweep-tests2 | parse-27;tempo-11;screenB-31 | test | preview_audio_offset_ms: nonzero delay replaces Offset; delay 0 = unset |
| 1697 | `resolve_preview_source reads delay from a song.ini in any case` | 380–387 | sweep-tests2 | parse-4;parse-27 | test | song.ini found in any case (find_song_ini) |
| 1698 | `sng_delay_ms reads the delay key in any case` | 389–398 | sweep-tests2 | parse-27;parse-24 | test | sng_delay_ms any key case |
| 1699 | `resolve_preview_source: a .sng's metadata delay replaces the chart Offset` | 400–419 | sweep-tests2 | parse-27;screenB-31 | test | .sng delay replaces chart Offset |
| 1700 | `resolve_preview_source: a .srb uses its chart's Offset` | 421–426 | sweep-tests2 | parse-27;screenB-31 | test | .srb uses chart Offset only |

### `tests/test_preview_transport.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1701 | `make_ramp` | 24–34 | sweep-tests2 | none | none | 48 kHz stereo ramp fixture |
| 1702 | `make_playhead` | 37–39 | sweep-tests2 | tempo-20 | copies | converts ms to frames itself (ms * 48.0); production has no single owner (Playhead::seek_ms/position_ms/length_ms, pad_front_ms) |
| 1703 | `an unloaded transport plays and scrubs with no audio` | 43–62 | sweep-tests2 | sweep-tests2-10;screenB-8 | test | unloaded transport: no end, seek unclamped |
| 1704 | `load takes the later of last note and audio end as the length` | 64–80 | sweep-tests2 | tempo-12;display-12;display-35;parse-21 | test | length = max(last note, audio end - offset) |
| 1705 | `play seeks the playhead to the clock and both run` | 82–108 | sweep-tests2 | sweep-tests2-10 | test | play seeks playhead |
| 1706 | `tick pauses at the end and pins the time` | 110–128 | sweep-tests2 | screenB-8;sweep-tests2-10 | test | tick pins at end |
| 1707 | `seek clamps to [0, length]` | 130–140 | sweep-tests2 | screenB-8 | test | seek clamps [0,length] |
| 1708 | `read_frames serves audio while playing and silence while paused` | 142–163 | sweep-tests2 | none | test | read_frames |
| 1709 | `gain set before load applies to the next playhead` | 165–185 | sweep-tests2 | none | test | gain |
| 1710 | `an audio offset seeks the playhead ahead of the clock` | 187–202 | sweep-tests2 | tempo-11;tempo-20 | test | audio offset shifts playhead |
| 1711 | `load with no audio offset behaves exactly as before` | 204–210 | sweep-tests2 | tempo-11 | test | no offset |
| 1712 | `pad_front_ms adds silence before the first sample` | 212–221 | sweep-tests2 | tempo-20 | test | pad_front_ms 1 ms = 48 frames |

### `tests/test_preview_view.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1713 | `make_hand_song` | 34–80 | sweep-tests3 | none | none | fixture: hand-built Song (sets flag_sp, sp_phrase_start, activation_length directly) |
| 1714 | `make_fill_song` | 84–98 | sweep-tests3 | none | none | fixture: four candidate fills |
| 1715 | `act_at` | 100–105 | sweep-tests3 | none | none | fixture: bare Activation |
| 1716 | `sp_act_at` | 146–154 | screenB | screenB-26 | copies | test fixture recomputes deact node (plusmeasure act+2*sp_meter); owner engine add_act_edge |
| 1716 | `sp_act_at` | 146–154 | spwin | spwin-3 | copies | fixture recomputes D as plusmeasure(act, 2*sp_meter) |
| 1716 | `sp_act_at` | 146–154 | tempo | tempo-7 | copies | fixture deact = act + 2B measures (engine rule restated via plusmeasure) |
| 1717 | `check_curve_well_formed` | 158–168 | screenB | none | test |  |
| 1717 | `check_curve_well_formed` | 158–168 | spwin | spwin-20 | test | curve bounds |
| 1718 | `make_overfill_song` | 174–195 | sweep-tests3 | none | none | fixture: overfill song mirrored from test_search.cpp fixture (two copies of the same note list) |
| 1719 | `analyzed` | 198–215 | sweep-tests3 | none | none | fixture: runs analyze_chart_file on first corpus chart |
| 1720 | `priced_path` | 220–231 | screenB | screenB-9 | test |  |
| 1720 | `priced_path` | 220–231 | sweep-tests3 | sweep-tests3-1;score-10 | copies | writes ReplayScore categories back into Path score_* fields by hand; reverse of core/replay.cpp score_of; engine.cpp writes the same field map (op.score_* from p.sc[]); owner score_of (field correspon |
| 1721 | `build_preview_scene: notes carry lane and drum attributes` | 238–269 | sweep-tests3 | display-35;parse-21;tempo-12;parse-10;parse-11;parse-16 | test | asserts song_length_ms == last note ms (750), the build_preview_scene copy of store::song_length_ms |
| 1722 | `build_preview_scene: SP phrase, solo, and fill spans` | 271–288 | sweep-tests3 | score-9;fills-7 | test | solo span coalescing + fill span = end - activation_length |
| 1723 | `build_preview_scene: an unanalyzed chart offers every candidate fill` | 290–295 | fills | fills-20 | test | no path -> offered |
| 1724 | `build_preview_scene: skips say which fills the path was offered` | 297–310 | fills | fills-20 | test | pins nearest-before mapping |
| 1724 | `build_preview_scene: skips say which fills the path was offered` | 297–310 | screenB | screenB-10 | test |  |
| 1725 | `build_preview_scene: a second activation with skips 0 hides what lies between` | 312–332 | fills | fills-20 | test | pins nearest-before mapping |
| 1725 | `build_preview_scene: a second activation with skips 0 hides what lies between` | 312–332 | screenB | screenB-10 | test |  |
| 1726 | `build_beat_events: bars, beats, and half-beats from the timing alone` | 334–356 | sweep-tests3 | tempo-9;tempo-4 | test | beat grid from timing |
| 1727 | `build_beat_events: a 3/4 section changes the beat count per bar` | 358–377 | sweep-tests3 | tempo-4 | test | 3/4 bar walk |
| 1728 | `build_preview_scene fills beats, tempos and resolution` | 379–390 | sweep-tests3 | tempo-3;sweep-tests3-2 | test | beat grid extends two measures past last note (literal 2 in build_preview_scene) |
| 1729 | `build_time_box: timestamp, measure, tempo` | 392–412 | screenB | screenB-7 | test |  |
| 1729 | `build_time_box: timestamp, measure, tempo` | 392–412 | sweep-tests3 | tempo-6;tempo-3;tempo-5;screenB-8;display-15 | test | time box position/clock/clamp |
| 1730 | `build_time_box: the end measure runs past the last beat line` | 414–424 | sweep-tests3 | tempo-6;tempo-13 | test | end measure from transport length past last beat line |
| 1731 | `build_time_box: the practice section in force` | 426–446 | sweep-tests3 | screenB-7;tempo-1 | test | section in force by tick; section ms past last note |
| 1732 | `build_time_box: a mid-measure meter change follows the engine` | 448–487 | sweep-tests3 | tempo-4;tempo-6 | test | compares box to engine Timecode::measure_beats_ticks (calls owner) |
| 1733 | `build_time_box: the time signature in force, as the chart wrote it` | 489–517 | screenB | screenB-7 | test |  |
| 1733 | `build_time_box: the time signature in force, as the chart wrote it` | 489–517 | sweep-tests3 | tempo-5;sweep-tests3-3 | test | time signature in force; empty scene reads 4/4 fallback |
| 1734 | `build_preview_scene: no path means no overlay` | 519–523 | sweep-tests3 | none | test | no path no overlay |
| 1735 | `parser keeps the SP-phrase start on the flagged note` | 525–544 | sweep-tests3 | sweep-tests3-4 | test | parser stores sp_phrase_start <= flagged tick |
| 1736 | `build_preview_scene: an analyzed chart's overlay matches its path` | 546–596 | sweep-tests3 | display-35;tempo-12;parse-21;spwin-10;tempo-1;parse-10 | test | asserts song_length_ms == notes.back().ms (pins the copy of store::song_length_ms); sp_end_ms via MsIndex::at owner |
| 1737 | `sp meter curve: each phrase's last note banks one bar` | 598–617 | spwin | spwin-1 | test | flat bank |
| 1738 | `sp meter curve: an activation snaps to the recorded bars, then drains` | 619–643 | spwin | spwin-1;spwin-20 | test | pins gauge phrase count 1 vs record 2 before activation |
| 1739 | `sp meter curve: a phrase collected mid-activation jumps the meter a bar` | 645–679 | screenB | screenB-2 | test | pins on-time collection only; no late-SqIn case |
| 1739 | `sp meter curve: a phrase collected mid-activation jumps the meter a bar` | 645–679 | spwin | spwin-17;spwin-20 | test | collected step |
| 1740 | `sp meter curve: a squeezed-out phrase does not bank mid-drain` | 681–709 | spwin | spwin-18 | test | squeezed-out banks at window close |
| 1741 | `sp meter curve: a full bank that collects a phrase and stores no row` | 711–762 | spwin | spwin-20 | test | full bank collection |
| 1742 | `sp meter curve: two clamped collections refill twice and empty at the deact node` | 764–800 | spwin | spwin-6;spwin-20 | test | clamped collections |
| 1743 | `sp meter curve: a phrase ending on the activation note is not counted twice` | 802–824 | sweep-tests3 | spwin-1;screenB-30;spwin-3 | test | phrase on activation tick not counted twice |
| 1744 | `sp meter curve: the drain is linear in measures across a tempo change` | 826–850 | screenB | screenB-1 | test |  |
| 1744 | `sp meter curve: the drain is linear in measures across a tempo change` | 826–850 | sweep-tests3 | spwin-20;tempo-7;spwin-4 | test | drain linear in measures across tempo change |
| 1745 | `sp meter curve: an activation with no recorded bars empties the meter at once` | 852–870 | sweep-tests3 | spwin-1;screenB-1 | test | no sp_meter: bank empties at activation |
| 1746 | `sp meter curve: the bank stops at the cap` | 872–891 | sweep-tests3 | spwin-7;screenB-12;spwin-1 | test | bank capped at cap |
| 1747 | `sp meter curve: without a path the meter fills and never drains` | 893–908 | sweep-tests3 | spwin-1 | test | meter never drains without a path |
| 1748 | `sp meter curve: a chart with no SP and no path has no curve at all` | 910–915 | sweep-tests3 | screenB-1 | test | no SP no curve |
| 1749 | `sp_meter_bars_at: before the curve, after it, and on a shared boundary` | 917–939 | sweep-tests3 | screenB-1;spwin-20 | test | curve read-back boundary rule |
| 1750 | `path_overlay_key: no overlay, the same path, and a changed path` | 941–966 | sweep-tests3 | none | test | path_overlay_key identity |
| 1751 | `step_tick_ms: one tick from the tick the time box shows` | 968–993 | sweep-tests3 | screenB-8;tempo-2 | test | step from rounded tick; expected via MsIndex::at owner |
| 1752 | `step_tick_ms: a tempo change moves the tick length with it` | 995–1007 | sweep-tests3 | tempo-2 | test | tempo change tick length |
| 1753 | `step_tick_ms: a scene with no song leaves the time alone` | 1009–1012 | sweep-tests3 | screenB-8 | test | empty scene step |
| 1754 | `score box: no path hides the box` | 1014–1020 | sweep-tests3 | screenB-9 | test | no path hides box |
| 1755 | `score box: a solo's bonus lands on its last note` | 1022–1048 | sweep-tests3 | score-12;score-24;score-8;score-9 | copies | expected step total recomputed as cum.total() - points.solo; owner core/replay.cpp replay_path cum_onscreen_total (solo_pending) |
| 1756 | `score box: before the first note nothing is hit yet` | 1050–1061 | sweep-tests3 | score-24 | test | before first note |
| 1757 | `score box: the multiplier is the replay's, doubled on chords Star Power pays` | 1063–1090 | screenB | screenB-9 | test |  |
| 1757 | `score box: the multiplier is the replay's, doubled on chords Star Power pays` | 1063–1090 | sweep-tests3 | score-24;backend-6;spwin-11;spwin-3 | test | multiplier_shown compared per step to replay (calls owner); expected x2/x4 literals |
| 1758 | `score box: a path the replay can't reproduce says so` | 1092–1113 | sweep-tests3 | screenB-9;score-10 | test | stored score vs replay mismatch -> Score unavailable |
| 1759 | `score box: the analyzed chart ends on the path's total` | 1115–1128 | screenB | screenB-9 | test |  |
| 1759 | `score box: the analyzed chart ends on the path's total` | 1115–1128 | sweep-tests3 | score-10;screenB-9 | test | last step == Path::totalscore |
| 1760 | `drain box: hidden without an SP gauge` | 1132–1139 | sweep-tests3 | screenB-1 | test | drain box hidden without gauge |
| 1761 | `drain box: idle at a steady tempo reads the rate and a full meter` | 1141–1152 | screenB | screenB-5 | test |  |
| 1761 | `drain box: idle at a steady tempo reads the rate and a full meter` | 1141–1152 | sweep-tests3 | screenB-4;screenB-5 | test | rate and full meter literals |
| 1762 | `drain box: the rate switches exactly at a tempo change` | 1154–1167 | screenB | screenB-4;screenB-5 | test |  |
| 1762 | `drain box: the rate switches exactly at a tempo change` | 1154–1167 | sweep-tests3 | screenB-4;screenB-5;tempo-8 | test | rate switches at tempo change |
| 1763 | `drain box: a 7/8 section drains faster at the same BPM` | 1169–1178 | screenB | screenB-4 | test |  |
| 1763 | `drain box: a 7/8 section drains faster at the same BPM` | 1169–1178 | sweep-tests3 | screenB-4;tempo-8;tempo-4 | test | 7/8 section rate |
| 1764 | `drain box: active inside the stored SP window, idle outside it` | 1180–1202 | screenB | screenB-3 | test |  |
| 1764 | `drain box: active inside the stored SP window, idle outside it` | 1180–1202 | sweep-tests3 | screenB-3;spwin-21;tempo-22;display-24;spwin-11 | test | active iff a.ms <= now < sp_end_ms (half-open) |
| 1765 | `drain box: empties in reads the stored end, not a recount` | 1204–1218 | screenB | screenB-6 | test |  |
| 1765 | `drain box: empties in reads the stored end, not a recount` | 1204–1218 | sweep-tests3 | screenB-6 | test | empties in reads stored end |
| 1766 | `drain box: an activation with no stored end stays idle` | 1220–1232 | screenB | screenB-3 | test |  |
| 1766 | `drain box: an activation with no stored end stays idle` | 1220–1232 | sweep-tests3 | screenB-3 | test | no stored end idle |
| 1767 | `TwoActs::TwoActs` | 1244–1251 | sweep-tests3 | none | none | fixture: two one-bar activations |
| 1768 | `scrub marks: each activation's onset over the scrubber's length` | 1256–1266 | sweep-tests3 | display-12;tempo-14 | test | scrub fraction over transport length, clamped to 1 |
| 1769 | `activation jumps: nearest activation before or after the playhead` | 1268–1279 | sweep-tests3 | screenB-11;tempo-21 | test | kOnActivationMs tolerance |
| 1770 | `next activation box: the activation at or after the playhead` | 1281–1292 | sweep-tests3 | screenB-11;tempo-21 | test | next activation at or after playhead |
| 1771 | `sp meter readout: bars banked over the cap` | 1294–1301 | sweep-tests3 | screenB-1 | test | readout %.1f/cap |
| 1772 | `preview path label: the notation and which list it came from` | 1303–1314 | sweep-tests3 | screenB-18;screenA-3 | test | preview label (optimal)/(best all-0) |

### `tests/test_replay.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1773 | `replay reproduces the engine's score for every corpus path` | 37–71 | sweep-tests3 | score-10;score-6;screenB-9 | test | replay totals vs stored engine totals over corpus (settings from app::Settings, not literals) |
| 1774 | `replay: combo_after is the combo once the chord is hit` | 75–100 | sweep-tests3 | score-3;score-24 | test | combo_after literals |
| 1775 | `targeted search reproduces every corpus path` | 108–177 | sweep-tests3 | fills-30;spwin-10 | test | search_target reproduces each path's deact_tick and sqinouts |
| 1776 | `targeted search rejects a tick that is not a fill` | 181–199 | sweep-tests3 | fills-30;fills-7 | test | non-fill tick refused |
| 1777 | `replay without Star Power scores no doubling at all` | 201–221 | sweep-tests3 | score-4;score-10;score-12;score-24 | test | prefix sum via ReplayScore::add/total; onscreen <= real |
| 1778 | `a squeezed-out chord past the leeway earns nothing` | 227–258 | backend | backend-3;backend-6 | test |  |
| 1779 | `shown_multiplier doubles the combo multiplier only under Star Power` | 260–265 | backend | backend-6 | test |  |
| 1780 | `a path JSON becomes windows with the squeeze-out offset intact` | 273–303 | sweep-tests3 | spwin-16;squeeze-21;sweep-tests3-5 | test | windows_from_json keeps SqOut offset and sqout_tick; -1 deact sentinel throws |
| 1781 | `windows read from a path JSON match the ones read from the record` | 308–340 | spwin | spwin-16;spwin-10 | test | JSON windows vs record windows on full records only |
| 1782 | `paths_json writes every field the dump readers use` | 344–375 | sweep-tests3 | display-27;spwin-3;squeeze-31;sweep-tests3-1 | test | dump field list; score key list typed again ("base".."ghost") beside kReplayScoreFields |
| 1783 | `a window ending on a phrase note with no offset is flagged` | 381–453 | sweep-tests3 | backend-8;spwin-14;squeeze-3;tempo-16 | copies | picks just_after/long_after with its own c.ms - phrase.ms < / > kSqueezeWindowMs test; owner ScoreGraph::is_recent_to_head (and replay sqout_candidates) |
| 1784 | `per-note sp points sum to the chord's sp points` | 455–473 | sweep-tests3 | score-4;score-5 | test | per-note sp sums to chord sp |
| 1785 | `song_with` | 479–493 | sweep-tests3 | none | none | fixture: 192-tick song with R+Y chords |
| 1786 | `make_chord_song` | 496–507 | sweep-tests3 | none | none | fixture: chord per beat |
| 1787 | `add_dynamic_cymbal` | 510–514 | sweep-tests3 | none | none | fixture: dynamic yellow cymbal |
| 1788 | `a typed squeeze-out offset resolves to the phrase chord` | 520–550 | sweep-tests3 | squeeze-1;spwin-13;backend-14 | test | resolve_sqout_note picks the phrase chord |
| 1789 | `a typed squeeze-out on a chord the engine never squeezes out is refused` | 556–580 | sweep-tests3 | squeeze-1;spwin-13 | test | non-first phrase chord refused (user decision 23 per comment); message literal "500 ms" |
| 1790 | `the squeeze-out warning names the chord the graph would squeeze` | 586–612 | sweep-tests3 | squeeze-1;squeeze-3;backend-2;backend-16 | test | warning names graph chord; exactly 500 ms outside; 1 tick after D inside leeway |
| 1791 | `category_scores reports the multiplier each note was paid at` | 614–637 | sweep-tests3 | score-2;score-3 | test | multiplier literals at combo 8/9/19/29/40 |
| 1792 | `category_scores prices a dynamic cymbal's bonus at its own note` | 639–662 | sweep-tests3 | score-1 | test | dynamic cymbal bonus literal 130 = (50+15)x2 |
| 1793 | `replay reports the multipliers category_scores applied` | 664–695 | sweep-tests3 | score-2;score-3 | test | replay multipliers vs category_scores |
| 1794 | `replay copies each note's dynamics bonus` | 697–707 | sweep-tests3 | score-1 | test | dynamics bonus 65 literal |
| 1795 | `replay names each note's dynamic` | 709–730 | sweep-tests3 | sweep-tests3-6 | test | dynamic_str spellings |
| 1796 | `replay multipliers agree with the combo on every corpus chord` | 732–755 | score | score-3;score-2 | copies | test recomputes multiplier via to_multiplier(combo_before+1) |
| 1797 | `replay score fields: one list in schema order` | 757–766 | sweep-tests3 | sweep-tests3-1 | test | kReplayScoreFields order |

### `tests/test_report.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1798 | `fill_store` | 45–69 | sweep-tests3 | none | none | fixture: analyzes corpus charts into a store (depth 10, given cap) |
| 1799 | `check_cap` | 71–101 | sweep-tests3 | score-19;store-8 | test | one rank-1 row per record; placeholders substituted |
| 1800 | `report page embeds every stored record 4 bars` | 105–105 | sweep-tests3 | score-19 | test | wrapper of check_cap(4) |
| 1801 | `report lists only the wanted cap and names it` | 107–140 | sweep-tests3 | store-8;display-10;sweep-tests3-7 | test | report lists only the wanted cap; subtitle "1 record across 1 chart" (generate_report counts records by rank==1, charts by distinct hyhash) |
| 1802 | `collect_rows: a blank or old-placeholder song name reads unknown` | 142–173 | sweep-tests3 | parse-7;display-9 | test | blank or "<unknown title>" song name reads kUnknownTitle (owner title_or_unknown via kOldPlaceholder in parse/song.cpp) |
| 1803 | `tier_for: raw-ms bands derived from the two-hit budget` | 175–198 | display | display-4;display-5 | test | pins tier_for(2.0) == Hard |
| 1804 | `tier_for walks the timing_tiers table edge by edge` | 200–212 | screenB | screenB-13 | test |  |
| 1804 | `tier_for walks the timing_tiers table edge by edge` | 200–212 | sweep-tests3 | display-5;squeeze-8;screenB-13 | test | tier_for vs timing_tiers cutoffs (calls owner) |
| 1805 | `report payload carries the hit window and the tier table` | 214–232 | sweep-tests3 | display-5;screenB-14;tempo-25;tempo-24 | test | payload carries hit window and tier table literals 2.0 / 170.0 |
| 1806 | `report page reads the Beyond edge from the tier table` | 234–239 | sweep-tests3 | screenB-14;tempo-25 | test | page JS BEYOND = Math.max( (pins the JS re-derivation of beyond_edge_ms) |
| 1807 | `generate_report: one seam frames the page for every entry point` | 242–276 | sweep-tests3 | display-10;screenB-13;sweep-tests3-7 | test | subtitle via report::counted; expected literals "top 5 paths per chart", "past the 170 ms window" |
| 1808 | `generate_report hands back nothing when its cancel flag is set` | 278–293 | sweep-tests3 | none | test | cancel flag |
| 1809 | `write_report_file swaps the page in and leaves no .tmp behind` | 295–316 | sweep-tests3 | none | test | atomic file write |
| 1810 | `path report shows each row's chart mode in its own column` | 318–337 | sweep-tests3 | store-11 | test | mode column |
| 1811 | `the three report pages share one stylesheet and one script` | 339–354 | sweep-tests3 | none | test | shared css/js |
| 1812 | `tier_for over a built table matches the window form` | 356–362 | sweep-tests3 | display-5;squeeze-8 | test | tier_for(table) == tier_for(window) |
| 1813 | `records_by_hash keys every listed record by its lower-case hash` | 364–393 | sweep-tests3 | parse-8 | test | records_by_hash lower-cases the hash |
| 1814 | `report pages: write samples for the browser check * doctest::skip()` | 399–494 | display | display-14;display-21 | copies | add_dm fixture recomputes delta and pct instead of calling collect_dm_rows |
| 1815 | `DocumentsSandbox::DocumentsSandbox` | 510–521 | sweep-tests3 | none | none | fixture: Documents sandbox |
| 1816 | `DocumentsSandbox::~DocumentsSandbox` | 523–528 | sweep-tests3 | none | none | fixture teardown |
| 1817 | `reports_dir is Documents\\Hydra, made on first use` | 533–543 | sweep-tests3 | sweep-tests3-8 | test | reports_dir = Documents\Hydra; report file names |
| 1818 | `reports_dir falls back to the database folder without Documents` | 545–557 | sweep-tests3 | sweep-tests3-8 | test | fallback to db folder |
| 1819 | `reports_dir keeps a harness's reports next to its database` | 559–568 | sweep-tests3 | sweep-tests3-8 | test | harness override keeps reports next to db |
| 1820 | `col_line` | 575–579 | sweep-tests3 | none | none | helper: COLS line |
| 1821 | `occurrences` | 581–585 | sweep-tests3 | none | none | helper: substring count |
| 1822 | `luminance` | 588–594 | sweep-tests3 | sweep-tests3-9 | owns | WCAG relative luminance formula (0.03928, 12.92, 2.4, 0.2126/0.7152/0.0722); only copy found in tests (not found in src/ or tools/) |
| 1823 | `contrast` | 596–599 | sweep-tests3 | sweep-tests3-9 | owns | WCAG contrast ratio (L+0.05)/(L+0.05) |
| 1824 | `css_tokens` | 602–618 | sweep-tests3 | none | none | helper: css token parse |
| 1825 | `report pages are standards-mode documents` | 622–633 | sweep-tests3 | none | test | doctype/shell |
| 1826 | `report colours meet WCAG contrast in both themes` | 635–653 | sweep-tests3 | sweep-tests3-9 | test | contrast thresholds 4.5 and 3.0 (WCAG AA literals) |
| 1827 | `report table header sticks and the page prints` | 655–668 | sweep-tests3 | none | test | sticky header/print css |
| 1828 | `report pages number their rows in a # column` | 670–678 | sweep-tests3 | none | test | # column |
| 1829 | `path report explains and renames its columns` | 680–701 | sweep-tests3 | screenB-17;screenB-28;display-8 | test | column labels 'Early fill (ms)' / 'Avg multiplier' and definitions present |
| 1830 | `path report counts charts by hash in the tile and the subtitle` | 703–723 | sweep-tests3 | sweep-tests3-7;parse-8 | test | JS tile counts charts by Set over c (c assigned by hyhash in build_html); C++ generate_report counts the same charts by its own hyhash set |
| 1831 | `comparison page explains its columns and splits the missing scores` | 725–739 | sweep-tests3 | score-22;display-21 | test | dm page column definitions and missing-score statuses |

### `tests/test_rules.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1832 | `write_rules` | 31–37 | sweep-tests3 | none | none | helper: write temp ini |
| 1833 | `fill_song` | 41–53 | sweep-tests3 | none | none | fixture: one note per measure, no authored fills |
| 1834 | `fill_count` | 55–60 | sweep-tests3 | fills-7 | wrapper | counts has_activation |
| 1835 | `rules: defaults are the values Hydra always used` | 64–73 | sweep-tests3 | backend-16;fills-22;fills-2 | test | pins core::default_rules values (3.0, FirstNote, 4, 4, 0.5, 0.5, 1/32) as literals |
| 1836 | `rules: a missing file and an empty file both load the defaults` | 75–81 | sweep-tests3 | store-15 | test | missing/empty file loads defaults |
| 1837 | `rules: every key in the file is read` | 83–100 | sweep-tests3 | store-15;fills-36 | test | every key read |
| 1838 | `rules: a # starts a comment anywhere on a line` | 102–109 | sweep-tests3 | store-15;store-16 | test | # comment anywhere (differs from settings ini comment rule per store-16) |
| 1839 | `rules: a bad value or an unknown key is an error that names the key` | 111–126 | sweep-tests3 | store-15;fills-36 | test | bad value/unknown key names key |
| 1840 | `rules: no rules value has the no-rules fingerprint` | 128–138 | sweep-tests3 | store-14;store-25 | test | no real rules hash to kNoRulesFingerprint |
| 1841 | `rules: whole_chord takes every note's SP doubling on a squeeze-out` | 140–153 | sweep-tests3 | backend-5;score-5 | test | sqout_reduction FirstNote 50 / WholeChord 100 |
| 1842 | `rules: the leeway moves the Standard edge of a backend rating` | 155–161 | backend | backend-9;backend-16 | test |  |
| 1843 | `rules: the leeway changes what the engine counts` | 163–190 | sweep-tests3 | backend-16;backend-2 | test | leeway monotone in engine score |
| 1844 | `rules: max_tied_paths caps the tied paths the engine keeps` | 192–206 | fills | fills-22 | test | engine behaviour |
| 1845 | `rules: the generated-fill values come from the rules` | 208–224 | fills | fills-2 | test | expected lengths |
| 1846 | `rules: the retired Auto keys are read and ignored` | 228–240 | sweep-tests3 | fills-31;store-15 | test | retired Auto keys ignored |
| 1847 | `rules: the retired Auto fingerprint is what Hydra 1.8.4 stamped` | 242–254 | sweep-tests3 | fills-31;store-25;store-14 | test | pinned fingerprints 0x70d2e96669604cf2 / 0x5b610b430a43a4be |
| 1848 | `rules: the default stamp is built once and matches a fresh record` | 256–266 | sweep-tests3 | store-14 | test | default_stamp built once |
| 1849 | `rules: a run is stamped with the rules fingerprint at every cap` | 268–279 | sweep-tests3 | store-14;store-1 | test | analysis stamps rules fingerprint at caps 8 and 4 |

### `tests/test_search.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1850 | `search invariants hold across the corpus and config knobs` | 27–94 | sweep-tests3 | score-27;score-19;fills-23;squeeze-28 | test | depth does not move optimum; paths best-first; tied_pathcount >= 1; ms filter best <= unconstrained |
| 1851 | `the graph finds the chart's multiplier squeezes once, in chart order` | 107–140 | sweep-tests3 | score-3;score-17 | copies | test spells its own combo walk (combo += chord.count() before each chord) to build the expected MultSqueeze list; owner category_scores / ScoreGraph::build combo counter |
| 1852 | `legacy fill deadline analyzes a chart end to end` | 142–171 | fills | fills-9 | test | smoke |
| 1853 | `stored transfer scales match the display-layer recomputation` | 174–257 | spwin | spwin-12;spwin-10 | copies | recomputes deact tick from row offsets via tick_at_ms (invariant check) |
| 1854 | `no activation keeps backends past its squeezed-out note` | 264–321 | backend | backend-3 | copies | restates tick > sqout_tick / == instead of calling sqout_position |
| 1854 | `no activation keeps backends past its squeezed-out note` | 264–321 | spwin | spwin-15 | copies | ticks > sqout_tick re-decided in test |
| 1855 | `search_allzero returns only all-0 paths inside the 0 ms limit` | 327–370 | fills | fills-28;fills-29 | copies | recomputes difficulty > 0.0 limit (owner engine ms filter) |
| 1856 | `build_tail_song` | 391–406 | sweep-tests3 | none | none | fixture: 192-tick tail song (activation_length 384) |
| 1857 | `tick_ms` | 408–410 | tempo | tempo-1 | wrapper | MsIndex::at |
| 1858 | `last_act` | 414–418 | sweep-tests3 | none | none | helper: best path's last activation |
| 1859 | `SP past the last note: backends measured from the tracked SP end` | 422–467 | backend | backend-7 | test | compares offset to ms_index.at - end_ms (restates offset rule) |
| 1859 | `SP past the last note: backends measured from the tracked SP end` | 422–467 | spwin | spwin-22;spwin-12 | test | tail backends |
| 1860 | `SP past the last note: a mid-activation phrase extends the end` | 469–510 | spwin | spwin-5;spwin-3 | test | extension; checks plain end via plusmeasure |
| 1861 | `SP past the last note: synthesized rows survive a store round-trip` | 512–554 | spwin | spwin-10 | test | round trip |
| 1862 | `run_search: EngineOptions carries each knob to the engine` | 558–600 | sweep-tests3 | fills-28;fills-30 | copies | re-spells the all-0 EngineOptions recipe (ms_filter 0.0, no_skips, hard_ms_filter) and checks it equals search_allzero; owner search_allzero (fills-28 already lists the separate 0.0 literal) |
| 1863 | `a 4-bar graph built at the song's phrase count stores the same paths` | 608–654 | spwin | spwin-7 | test | build cap equivalence |
| 1864 | `SP cap overfill: a mid-SP phrase that clamps records the  "collecting note"` | 666–698 | spwin | spwin-6 | test | clamp |
| 1865 | `SP cap overfill: a mid-SP phrase that only ties the cap does  "not clamp"` | 701–728 | spwin | spwin-23 | test | tie |
| 1866 | `SP cap overfill: a later unclamped extension keeps the earlier  "clamp_tick"` | 731–769 | spwin | spwin-6 | test | clamp kept |
| 1867 | `SP cap overfill: a second clamp in the same window replaces  "clamp_tick"` | 772–802 | spwin | spwin-6 | test | clamp replaced |
| 1868 | `collected phrases: none when no phrase lands during the activation` | 804–815 | spwin | spwin-17 | test | none collected |
| 1869 | `collected phrases: one phrase mid-activation is recorded` | 817–827 | spwin | spwin-17 | test | one collected |
| 1870 | `collected phrases: two phrases under a full meter are both recorded, in order` | 829–844 | spwin | spwin-17;spwin-6 | test | clamped counts as collected |
| 1871 | `collected phrases: the corpus agrees with the squeezes and the SP end` | 846–874 | spwin | spwin-16;spwin-17 | test | sqout_tick present iff SqOut in sqinouts |
| 1872 | `path codec: encode/decode a path node keeps clamp_tick` | 876–891 | spwin | spwin-6 | test | codec |
| 1873 | `graph_build_cap: never taller than the song's phrases, never below one` | 893–897 | spwin | spwin-7 | test | graph_build_cap |

### `tests/test_sng.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1874 | `push_u32` | 27–29 | sweep-tests3 | store-6;parse-24 | copies | little-endian u32 writer for the .sng fixture; owner store BinaryWriter (store-6) / parse/sng reader layout (parse-24); test_srb push_str and test_store write_le are siblings |
| 1875 | `push_u64` | 30–32 | sweep-tests3 | store-6;parse-24 | copies | little-endian u64 writer for the .sng fixture; owner as push_u32 |
| 1876 | `make_sng` | 40–73 | parse | parse-24 | copies | test helper re-encodes sng layout incl. XOR mask |
| 1877 | `tiny_mid` | 75–81 | parse | none | none | fixture bytes |
| 1878 | `sng_fixture_path` | 83–87 | sweep-tests3 | none | none | helper: temp path |
| 1879 | `write_fixture` | 89–94 | sweep-tests3 | none | none | helper: write bytes |
| 1880 | `sng: metadata pairs read back in order` | 98–104 | sweep-tests3 | parse-24 | test | sng metadata pairs |
| 1881 | `sng: the file table and each file decode` | 106–120 | sweep-tests3 | parse-24 | test | sng file table decode |
| 1882 | `sng: truncated input stops early instead of reading past the end` | 122–137 | sweep-tests3 | parse-24 | test | truncated input / wrapping offset refused |
| 1883 | `sng: the note loader reads the chart through the shared reader` | 139–152 | sweep-tests3 | parse-3;parse-2;parse-24 | test | loader picks NOTES.MID via notes_file_format (case-insensitive) |

### `tests/test_song.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1884 | `check_invariants` | 30–49 | sweep-tests3 | parse-18;fills-6 | test | invariants: strictly increasing ticks, chord code round trip via Chord::code/from_code, activation_length > 0 |
| 1885 | `song parse holds its invariants over the corpus` | 53–65 | sweep-tests3 | parse-18 | test | corpus invariants at Expert |
| 1886 | `song parse holds its invariants at Hard too` | 67–91 | sweep-tests3 | parse-19;parse-1 | copies | classifies .mid with case-sensitive ends_with(path, ".mid") instead of chart_format_of (owner, case-insensitive) |
| 1887 | `.chart: each difficulty reads its own section` | 93–132 | parse | parse-19 | test | section per difficulty |
| 1888 | `.mid: each difficulty reads its own pitch base` | 134–158 | parse | parse-19 | test | pitch base per difficulty (no disco or 95 case) |
| 1889 | `.chart: [Events] section markers become practice sections` | 160–199 | parse | parse-28 | test | sections |
| 1890 | `.chart: the note on the solo end tick is in the solo` | 203–218 | parse | none | test | solo |
| 1891 | `.chart: time signatures are kept as written` | 222–245 | parse | none | test | timesig |
| 1892 | `.mid: the note on the solo marker's note-off tick is outside the solo` | 249–272 | parse | none | test | solo |
| 1893 | `put_varlen` | 276–287 | sweep-tests3 | sweep-tests3-11 | copies | MIDI varlen writer; tests/midi_util.h (testmidi) is the test-side MIDI writer; parse/midi.cpp reads varlen |
| 1894 | `put_meta` | 290–296 | sweep-tests3 | sweep-tests3-11 | copies | meta event writer with delta; testmidi::track_name/text_event write the same bytes at delta 0 |
| 1895 | `put_bytes` | 298–300 | sweep-tests3 | none | none | helper: push bytes |
| 1896 | `put_track` | 302–309 | sweep-tests3 | sweep-tests3-11 | copies | MTrk header + big-endian length; testmidi::smf writes the same header for one track |
| 1897 | `.mid: EVENTS text metas become practice sections` | 313–346 | sweep-tests3 | parse-28 | test | EVENTS [section X] and [prc_X] become practice sections; [crowd_*] ignored |
| 1898 | `mid: kick velocity is read as ghost/accent, like a pad's` | 348–399 | sweep-tests3 | parse-16;score-26;parse-13;parse-17 | test | kick velocity 1/127 ghost/accent only with [ENABLE_CHART_DYNAMICS]; 95 is 2x kick |
| 1899 | `mid: Won't Get Fooled Again O has 36 ghost kicks` | 401–435 | sweep-tests3 | score-26;parse-18 | copies | re-inlines the check_invariants tick and code checks (same file, id 1884) minus the fill check; expected 36 ghost kicks |
| 1900 | `chart_files: loose-folder notes names match in any case` | 437–446 | sweep-tests3 | parse-2;parse-4 | test | notes_file_format / is_song_ini case-insensitive |
| 1901 | `chart_files: a path's format comes from its extension in any case` | 448–455 | sweep-tests3 | parse-1 | test | chart_format_of case-insensitive |
| 1902 | `mid: a stray SP note-off flags nothing` | 457–472 | sweep-tests3 | sweep-tests3-4 | test | stray SP note-off flags nothing |
| 1903 | `.chart: [Song] Offset is read in seconds` | 474–486 | sweep-tests3 | parse-27;tempo-11 | test | .chart [Song] Offset in seconds; absent stays unset |

### `tests/test_song_panel_state.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1904 | `temp_path` | 29–34 | sweep-tests3 | none | none | helper: temp path (same recipe also in test_report, test_sng sng_fixture_path, test_store temp_db; plumbing) |
| 1905 | `ScratchPaths::ScratchPaths` | 43–50 | sweep-tests3 | none | none | fixture: scratch ini/db overrides |
| 1906 | `ScratchPaths::~ScratchPaths` | 51–55 | sweep-tests3 | none | none | fixture teardown |
| 1907 | `entry` | 58–70 | sweep-tests3 | none | none | fixture: library entry |
| 1908 | `app_with_library` | 72–80 | sweep-tests3 | none | none | fixture: AppState with library |
| 1909 | `song panel: next and previous walk the view and never wrap` | 84–104 | sweep-tests3 | sweep-tests3-12 | test | next/previous walk the view, never wrap |
| 1910 | `song panel: a song outside the view has no neighbours` | 106–112 | sweep-tests3 | sweep-tests3-12 | test | song outside view has no neighbours |
| 1911 | `song panel: tick runs the teardown once, on the closing edge` | 114–127 | sweep-tests3 | none | test | teardown on closing edge from tick |
| 1912 | `song panel: nothing is locked while idle` | 129–135 | sweep-tests3 | none | test | nothing locked while idle |

### `tests/test_squeeze_rating.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1913 | `frontend_transfer_scales: measure-rate ratio, both directions` | 18–79 | squeeze | squeeze-13;squeeze-31 | copies | builds deact_tick from plusmeasure (owner engine) |
| 1914 | `frontend_transfer_scales: a SqIn splits the two ends` | 81–119 | squeeze | squeeze-12;squeeze-31 | copies | builds deact_tick from plusmeasure |
| 1915 | `frontend_transfer_scales: direction-dependent at boundaries` | 121–179 | squeeze | squeeze-13;squeeze-31 | copies | deact_tick from plusmeasure; bpm ratios |
| 1916 | `field fixture: What's My Age Again? Sync Chart SqOut` | 181–257 | squeeze | squeeze-15;squeeze-31 | copies | recomputes gap/(1+r) and r*frontend+note>gap (owner squeeze_budget_ms) |
| 1917 | `activation_deact_tick: the stored node, read back` | 259–278 | squeeze | none | test |  |
| 1918 | `field fixture: Dumpweed SqOut end anchored on the deact node` | 280–357 | squeeze | squeeze-15 | copies | 2*gap/(1+r) recomputed (owner effective_backend_ms) |
| 1919 | `difficulty is the raw gap, untouched by stored transfer scales` | 359–373 | squeeze | squeeze-5 | test |  |
| 1920 | `rate_activation: SqIns warn late, SqOuts early, with no backend rows` | 375–396 | squeeze | squeeze-11 | test |  |
| 1921 | `transfer_is_material: impact or budget, not \|r - 1\|` | 398–410 | squeeze | squeeze-16 | test |  |
| 1922 | `rate_activation: stored scales, materiality-gated warns and rows` | 412–479 | squeeze | squeeze-16;squeeze-17 | test |  |
| 1923 | `rate_activation: free squeezes read the opposite scale direction` | 481–556 | squeeze | squeeze-10;squeeze-11 | test |  |
| 1924 | `rate_activation: counted rows before the SP end read the early scale` | 558–589 | squeeze | squeeze-11 | test |  |
| 1925 | `rate_activation: SqIn/SqOut eff. figures, one per squeeze` | 591–612 | squeeze | squeeze-17 | test |  |
| 1926 | `rate_activation: the stored scales are the only scales` | 614–633 | squeeze | squeeze-12 | test |  |
| 1927 | `timing_tiers: the ladder at W=85 and at W=70` | 635–687 | squeeze | squeeze-8 | test |  |
| 1928 | `display_backends: 500 ms window keeps everything the 500 ms search graph collects` | 689–718 | squeeze | squeeze-3;squeeze-30 | test |  |
| 1929 | `rate_activation: cap_clamped flag` | 720–753 | squeeze | squeeze-25 | test |  |
| 1930 | `rate_activation: a plain row inside the leeway is not a frontend squeeze` | 758–791 | backend | backend-2;backend-9 | test |  |
| 1930 | `rate_activation: a plain row inside the leeway is not a frontend squeeze` | 758–791 | squeeze | squeeze-20;squeeze-25;squeeze-9 | test |  |
| 1931 | `squeeze_budget_ms: identity scale is twice the hit window` | 793–797 | squeeze | squeeze-15 | test |  |
| 1932 | `beyond_edge_ms: the last finite timing-tier cutoff` | 799–807 | screenB | screenB-14 | test |  |
| 1932 | `beyond_edge_ms: the last finite timing-tier cutoff` | 799–807 | squeeze | squeeze-8 | copies | recomputes last cutoff loop (owner beyond_edge_ms) |

### `tests/test_srb.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1933 | `read_bytes` | 36–38 | sweep-tests3 | none | wrapper | calls hydra::read_file_bytes |
| 1934 | `write_bytes` | 40–45 | sweep-tests3 | none | none | helper: write bytes |
| 1935 | `fixture_dir` | 48–64 | sweep-tests3 | none | none | helper: temp fixture dir; converts wide->UTF-8 with its own WideCharToMultiByte instead of core/winstr wide_to_utf8 (plumbing copy) |
| 1936 | `deflate_raw` | 66–76 | sweep-tests3 | parse-25 | test | raw deflate via miniz (what .srb streams use) |
| 1937 | `push_str` | 78–82 | sweep-tests3 | parse-25;store-6 | copies | u32 little-endian length + bytes string writer for the .srb fixture; owner parse/srb.h reader layout; test_preview_source make_srb types the same layout again |
| 1938 | `make_metadata` | 87–101 | sweep-tests3 | parse-25 | copies | srb metadata layout ('4','b','4',1 magic + string table + junk); owner srb_parse_metadata; same literal magic typed again in tests/test_preview_source.cpp:156 |
| 1939 | `make_srb` | 105–121 | parse | parse-25 | copies | test helper re-encodes srb layout |
| 1940 | `songs_equal` | 123–138 | sweep-tests3 | sweep-tests3-13 | test | own Song equality (resolution, tpm, bpm, features, ticks, chord code, solo/sp flags, activation_length) |
| 1941 | `corpus_chart_path` | 141–143 | sweep-tests3 | none | wrapper | corpus::first_chart_with_suffix |
| 1942 | `srb: a wrapped chart parses identically to the loose file` | 147–166 | sweep-tests3 | parse-25;parse-3 | test | wrapped chart parses like loose file |
| 1943 | `srb: an unexpected notes filename falls back to payload sniffing` | 168–179 | sweep-tests3 | parse-3 | test | unexpected notes filename falls back to MThd sniff |
| 1944 | `srb: malformed containers throw instead of crashing` | 181–204 | sweep-tests3 | parse-25 | test | malformed containers throw |
| 1945 | `srb: metadata parser reads the string table` | 206–229 | sweep-tests3 | parse-25 | test | metadata string table |
| 1946 | `srb: discovery surfaces the embedded metadata` | 231–251 | sweep-tests3 | parse-25;parse-6 | test | discovery surfaces embedded metadata |
| 1947 | `srb: an empty embedded name reads unknown` | 253–268 | sweep-tests3 | parse-7;parse-6 | test | empty embedded name reads kUnknownTitle |
| 1948 | `srb: one named cap bounds every inflated stream` | 270–275 | sweep-tests3 | sweep-tests3-14 | test | kSrbMaxStream == 1<<30 pinned, > kSrbMaxMetadata |

### `tests/test_stars.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1949 | `cutoffs_for` | 19–23 | score | score-14 | copies | cutoffs_for re-runs star_cutoffs loop over star_cutoff |
| 1949 | `cutoffs_for` | 19–23 | screenA | screenA-31 | test |  |
| 1950 | `stars: the table is the game's first seven multipliers` | 27–31 | score | score-14 | test | table pinned |
| 1950 | `stars: the table is the game's first seven multipliers` | 27–31 | screenA | screenA-31 | test |  |
| 1951 | `stars: whole-number products come out exact` | 33–36 | score | score-14 | test | exact products |
| 1951 | `stars: whole-number products come out exact` | 33–36 | screenA | screenA-31 | test |  |
| 1952 | `stars: fractional products round up` | 38–42 | score | score-14 | test | round up |
| 1952 | `stars: fractional products round up` | 38–42 | screenA | screenA-31 | test |  |
| 1953 | `stars: the multiply is 32-bit float, as in the game` | 44–50 | score | score-14 | test | float32 |
| 1953 | `stars: the multiply is 32-bit float, as in the game` | 44–50 | screenA | screenA-31 | test |  |
| 1954 | `stars: a zero base gives zero cutoffs` | 52–55 | score | score-14 | test | zero base |
| 1954 | `stars: a zero base gives zero cutoffs` | 52–55 | screenA | screenA-31 | test |  |
| 1955 | `stars: star_cutoffs reads the path's base score and solo bonus` | 57–73 | score | score-11;score-14 | test | star_cutoffs inputs |
| 1955 | `stars: star_cutoffs reads the path's base score and solo bonus` | 57–73 | screenA | screenA-30 | test |  |
| 1956 | `stars: avg_mult divides by the same base score` | 75–84 | score | score-12;score-13 | test | avg_mult |
| 1956 | `stars: avg_mult divides by the same base score` | 75–84 | screenA | screenA-28 | test |  |
| 1957 | `stars: the base score is the sum of every note's basescore, on every path` | 86–136 | score | score-1;score-11 | copies | recomputes base score as sum of basescore (cross-check) |
| 1957 | `stars: the base score is the sum of every note's basescore, on every path` | 86–136 | screenA | screenA-30 | copies | recomputes base score as sum of ChordNote::basescore to cross-check Path::chart_base_score |
| 1958 | `stars: path_stars counts cutoffs reached without the solo bonus` | 138–163 | score | score-12;score-15 | test | path_stars |
| 1958 | `stars: path_stars counts cutoffs reached without the solo bonus` | 138–163 | screenA | screenA-32 | test |  |
| 1959 | `stars: Burnout's optimal path earns 7 stars` | 165–180 | score | score-15 | test | Burnout 7 stars |
| 1959 | `stars: Burnout's optimal path earns 7 stars` | 165–180 | screenA | screenA-32 | test |  |

### `tests/test_store.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 1960 | `diff_summary` | 70–82 | sweep-tests3 | store-22 | none | helper: field-by-field PathSummary comparison (test-only list of summary fields; the summary column list also lives in summarize_path and the results schema) |
| 1961 | `records round-trip through RecordStore across the corpus and config matrix` | 86–192 | store | store-19 | test |  |
| 1962 | `stored transfer scales equal a live recompute after a store round trip` | 200–255 | sweep-tests3 | tempo-27;squeeze-13;squeeze-12 | test | stored transfer scales == live frontend_transfer_scales after round trip (calls owner) |
| 1963 | `RecordStore maintenance: has_record, list_records, reindex` | 257–312 | sweep-tests3 | store-4;store-1;store-22 | test | has_record, list_records, reindex; 0.0.0 stamp reads Stale |
| 1964 | `RecordStore results stamp: every accepted stamp reads Ready, others Stale` | 314–359 | store | store-1 | test |  |
| 1965 | `fixture` | 368–389 | sweep-tests3 | none | none | fixture: first analyzable corpus chart at cap 4 |
| 1966 | `at_cap` | 394–398 | store | none | test |  |
| 1967 | `temp_db` | 400–405 | sweep-tests3 | none | none | helper: temp db path (same recipe as test_report/test_song_panel_state/test_sng) |
| 1968 | `scalar` | 409–419 | sweep-tests3 | none | none | helper: one SQL scalar from a closed db |
| 1969 | `at_cap_ms10` | 428–432 | sweep-tests3 | store-12 | none | fixture: record relabeled with ms_limit 10 |
| 1970 | `records at different caps coexist each lookup sees only its own cap` | 436–496 | store | store-7;store-8 | test |  |
| 1971 | `a row an old migration marked with unknown settings reads Not analyzed` | 498–525 | store | store-7 | test |  |
| 1972 | `a row in an older path format is Stale even when this build stamped it` | 527–586 | store | store-1;store-5 | copies | pokes structure[0..3]; owner flatten_record layout |
| 1973 | `a row in the 1.8.1 path layout structure format 5 reads Stale` | 588–600 | store | store-1;store-5 | copies | pokes structure[0] |
| 1974 | `a row analyzed under other rules reads Stale until the rules match again` | 602–647 | store | store-1;store-21 | test |  |
| 1975 | `a Stale lookup says why: another build, other rules, or both` | 649–692 | store | store-2 | test |  |
| 1976 | `for_each_blob does not hold the store lock across its callback` | 694–732 | sweep-tests3 | none | test | for_each_blob lock not held across callback |
| 1977 | `a write during for_each_blob skips the row it replaced` | 734–766 | sweep-tests3 | store-7 | test | rewritten row skipped in the walk |
| 1978 | `a rewritten row that lands on its own id is read fresh, not mixed up` | 768–792 | sweep-tests3 | store-7 | test | row rewritten onto its own id read fresh |
| 1979 | `for_each_blob stops between rows when its cancel flag is set` | 794–810 | sweep-tests3 | none | test | cancel between rows |
| 1980 | `the listing and a lookup agree on which row is a chart's answer` | 812–882 | sweep-tests3 | store-3;store-1;store-8 | test | listing and get_record pick the same row; ms_enabled = -1 migrated row is no candidate |
| 1981 | `prepare_row refuses a key whose exact cap isn't the record's` | 884–894 | sweep-tests3 | store-12;store-8 | test | prepare_row cap guard |
| 1982 | `prepare_row refuses a key whose ms limit isn't the record's` | 896–914 | sweep-tests3 | store-12;store-9 | test | prepare_row ms-limit guard (only when lens has it on) |
| 1983 | `RecordKey compares on every part of the identity` | 916–937 | sweep-tests3 | store-7;store-9;store-8 | test | RecordKey/Lens identity; CapQuery{} == at(kCloneHeroSpCap); off-limit ms value normalised to 0 |
| 1984 | `a current-version record with no paths is Ready, not Stale` | 939–956 | sweep-tests3 | store-1;display-11 | test | current-version record with no paths is Ready |
| 1985 | `the same chart at the same cap keeps one result per lens` | 965–1000 | sweep-tests3 | store-7;store-4 | test | one result per lens |
| 1986 | `a path stored under two lenses is stored once` | 1002–1025 | sweep-tests3 | store-24 | test | path node stored once across lenses |
| 1987 | `replacing one lens's result leaves the other's bytes untouched` | 1027–1067 | sweep-tests3 | store-21;store-24 | test | replacing one lens keeps the other's bytes; GC of unreferenced nodes |
| 1988 | `a current-version write purges the chart's old-version rows and their paths` | 1069–1108 | sweep-tests3 | store-21 | test | current-version write purges the chart's old-version rows only |
| 1989 | `chart_entry` | 1114–1122 | sweep-tests3 | none | none | fixture: library entry |
| 1990 | `exec_on_file` | 1125–1135 | sweep-tests3 | none | none | helper: run SQL on a closed db file |
| 1991 | `a database from Hydra 1.6 or older opens with nothing to show` | 1139–1177 | sweep-tests3 | store-1 | test | pre-1.7 records table left unread; user_version 3 (user decision 2026-09-26 quoted in comment) |
| 1992 | `legacy_at_cap` | 1191–1195 | sweep-tests3 | none | none | fixture: record with legacy_fills |
| 1993 | `downgrade_to_schema2` | 1199–1220 | store | store-27 | copies | schema 2 column list typed again; owner kSchema2ResultsColumns |
| 1994 | `1.0 and 1.1 results for one chart sit side by side` | 1224–1249 | sweep-tests3 | store-7;fills-10 | test | 1.0 and 1.1 rows coexist |
| 1995 | `prepare_row refuses a key that names the other fill rule` | 1251–1257 | sweep-tests3 | store-13;fills-10 | test | prepare_row refuses mismatched fill rule |
| 1996 | `a schema 2 database keeps its results, filed under Clone Hero 1.1` | 1259–1285 | sweep-tests3 | store-10;fills-10;store-27 | test | schema 2 rows filed under 1.1 |
| 1997 | `a schema 2 database hydra_batch --legacy-fills filled is filed under 1.0` | 1287–1305 | sweep-tests3 | store-10;fills-10;fills-11 | test | schema 2 + ch10 engine_mode stamp filed under 1.0 (engine_mode_stamp owner) |
| 1998 | `a failed library rebuild keeps the previous scan` | 1307–1333 | sweep-tests3 | none | test | failed library rebuild keeps previous scan |
| 1999 | `a charts table from before the sig column still rebuilds` | 1335–1354 | sweep-tests3 | none | test | charts table gains sig column |
| 2000 | `a song's stored names follow the latest analysis and the latest scan` | 1356–1386 | sweep-tests3 | parse-22;store-17 | test | names follow latest analysis and first-listed scan copy |
| 2001 | `get_record reads a whole row while another thread rewrites it` | 1388–1427 | sweep-tests3 | none | test | get_record whole-row read under concurrent rewrite |
| 2002 | `has_record and a lookup agree on which rows are readable` | 1429–1465 | sweep-tests3 | store-1;store-5 | test | has_record (SQL kRowReadySql) == get_summary Ready (C++ rank_row) for build/format/rules rows; pokes structure[0..3] for the format stamp |
| 2003 | `the vendored SQLite is built without FTS5` | 1470–1472 | sweep-tests3 | none | test | SQLite built without FTS5 |
| 2004 | `a file store runs in WAL mode with an index on chart names` | 1476–1495 | sweep-tests3 | none | test | WAL mode and charts_by_name index |
| 2005 | `save_analysis writes the song, the result and the count together` | 1497–1514 | sweep-tests3 | store-20;parse-23 | test | save_analysis writes song, result, dynamics count together |
| 2006 | `a save_analysis that fails leaves nothing behind` | 1516–1538 | sweep-tests3 | none | test | failed save_analysis rolls back |
| 2007 | `analyzed_hashes names exactly the charts has_record would skip` | 1540–1562 | sweep-tests3 | store-4;store-3 | test | analyzed_hashes == has_record |
| 2008 | `get_summaries answers a page the same as get_summary row by row` | 1564–1593 | sweep-tests3 | store-3;store-1 | test | get_summaries == get_record row by row |
| 2009 | `auto_run_at` | 1601–1605 | store | store-25 | test |  |
| 2010 | `the first open deletes the results Auto saved, and their paths, once` | 1609–1666 | sweep-tests3 | store-25;fills-31 | test | first open deletes Auto results once (meta key auto_results_deleted) |
| 2011 | `a start with a bad rules file leaves the Auto results for the next good start` | 1668–1691 | sweep-tests3 | store-25;store-14 | test | bad rules file leaves Auto results |
| 2012 | `a saved result stores its best path's star count` | 1695–1734 | sweep-tests3 | score-15;store-22 | test | stored stars == path_stars(best) (calls owner) |
| 2013 | `get_summaries answers a whole library in chunks` | 1736–1772 | sweep-tests3 | store-3 | test | get_summaries chunking past SQLite bind limit |
| 2014 | `a database from before the stars column gets its stars filled on open` | 1774–1806 | sweep-tests3 | score-15;store-22 | test | stars column filled on open via path_stars |
| 2015 | `under rules that make every row Stale, the stars fill changes nothing` | 1808–1831 | sweep-tests3 | score-15;store-1 | test | under RulesStamp::none the stars fill changes nothing |
| 2016 | `a stored song keeps its length, and an old songmeta row reads none` | 1833–1876 | sweep-tests3 | store-17;parse-21;tempo-12 | copies | expected length recomputed as song.sequence.back().timecode.ms() (then checked against store::song_length_ms owner); also pins set_song_length keeping an existing length while upsert_song lets the lat |

### `tests/test_strutil.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2017 | `strutil: to_lower_ascii lowers A-Z and leaves every other byte` | 12–18 | sweep-tests4 | sweep-tests4-6 | test | pins core/strutil to_lower_ascii |
| 2018 | `strutil: trim strips ASCII whitespace from both ends only` | 20–26 | sweep-tests4 | sweep-tests4-6 | test | pins core/strutil trim |
| 2019 | `strutil: ends_with is exact and ends_with_ci ignores ASCII case` | 28–38 | sweep-tests4 | sweep-tests4-6 | test | pins core/strutil ends_with/ends_with_ci; user_messages.cpp keeps its own ends_with |

### `tests/test_theme.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2020 | `channel` | 17–19 | sweep-tests4 | sweep-tests4-2 | copies | WCAG sRGB channel linearisation (0.04045); second copy in tests/test_report.cpp luminance uses 0.03928; no production owner |
| 2021 | `luminance` | 21–23 | sweep-tests4 | sweep-tests4-2 | copies | WCAG relative luminance; same formula re-typed in tests/test_report.cpp luminance |
| 2022 | `contrast` | 25–29 | sweep-tests4 | sweep-tests4-2 | copies | WCAG contrast ratio; same formula re-typed in tests/test_report.cpp contrast |
| 2023 | `theme: button text reads at 4.5:1 in every button state` | 33–37 | sweep-tests4 | sweep-tests4-2 | test | theme constants against 4.5:1 via the local contrast() |
| 2024 | `theme: dimmed and disabled text stays readable` | 39–45 | sweep-tests4 | sweep-tests4-2 | test | theme constants against 4.5:1 via the local contrast() |
| 2025 | `theme: apply_theme uses the readable shades` | 47–56 | sweep-tests4 | none | test | apply_theme copies theme constants into ImGui style |

### `tests/test_timing.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2026 | `timing: corpus conversions are monotone and consistent` | 17–52 | sweep-tests4 | tempo-1;tempo-6 | test | calls Timecode/ms over the corpus |
| 2027 | `timing: to_multiplier thresholds` | 54–64 | score | score-2 | test | to_multiplier thresholds |
| 2028 | `timing: a map without a tick-0 entry throws` | 66–72 | sweep-tests4 | tempo-1;tempo-4 | test | MsIndex/MeasureIndex need a tick-0 entry |
| 2029 | `timing: a tick before the first tempo mark reads at the opening tempo` | 74–81 | sweep-tests4 | tempo-1 | test | MsIndex::at extrapolates before the first tempo |
| 2030 | `timing: a meter change off a barline carries a partial measure` | 83–96 | sweep-tests4 | tempo-4 | test | MeasureIndex::section_at boundary belongs to the prior section |
| 2031 | `timing: ms_per_measure_at reads local measure durations` | 98–119 | sweep-tests4 | tempo-8 | test | SongTiming::ms_per_measure_at; expected values typed as literals |
| 2032 | `timing: continuous helpers are exact, inverse, and monotone` | 121–171 | sweep-tests4 | tempo-1;tempo-2;tempo-7;spwin-4 | test | ms_at_tick_f vs timecode, tick_at_ms inverse, sp_end_ms continuous copy agrees with plusmeasure only at flat tempo on-tick |
| 2033 | `timing: one SP bar is two measures` | 173–185 | sweep-tests4 | spwin-3;spwin-4;tempo-7 | test | kMeasuresPerSpBar / sp_bars_to_measures |

### `tests/test_track_state.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2034 | `note` | 19–28 | sweep-tests4 | none | none | fixture builder (1 tick per ms) |
| 2035 | `span` | 30–37 | sweep-tests4 | none | none | fixture builder |
| 2036 | `fill` | 39–44 | sweep-tests4 | none | none | fixture builder |
| 2037 | `timed_scene` | 49–54 | sweep-tests4 | none | none | fixture scene 60 BPM 1000 res |
| 2038 | `find` | 56–60 | sweep-tests4 | none | none | lookup helper |
| 2039 | `toggle_at: Onyx makeToggle truth table` | 64–72 | sweep-tests4 | display-24;tempo-22 | test | toggle_at truth table |
| 2040 | `build_track_state: gems, pro-off, and the phrase end note reads inside` | 74–107 | sweep-tests4 | tempo-23;parse-11;parse-16 | test | half-tick phrase end (1.0005 = kSpanEndTicks at 1 tick/ms typed as literal), pro-off strips cymbals, ghost/accent velocity |
| 2041 | `build_track_state: active SP window ends exactly at the deact node` | 109–131 | sweep-tests4 | display-24;tempo-22;screenB-3;spwin-21 | test | sp_active ends exactly at sp_end_ms (no half tick), unlike phrase spans |
| 2042 | `build_track_state: taken, offered and hidden fills toggle different spans` | 133–181 | sweep-tests4 | fills-20;display-25;screenB-10 | test | taken/offered/hidden fill toggles |
| 2043 | `build_track_state: beats land on instants solo toggles` | 183–197 | sweep-tests4 | tempo-23;tempo-9 | test | beats on instants; one-note solo ends half a tick later |
| 2044 | `window: strict bounds, and a synthesized instant when empty` | 199–224 | sweep-tests4 | tempo-23 | test | window bounds and synthesized instant |
| 2045 | `make_toggle_bounds: covers [near, far], merges equal neighbours` | 226–258 | sweep-tests4 | tempo-23 | test | touching solos merge because of the half-tick end |
| 2046 | `build_track_state: a chord one tick after a phrase ends is not SP 480 res, 300 BPM` | 260–289 | sweep-tests4 | tempo-23;screenB-30 | test | a chord one tick after a phrase end is not SP |
| 2047 | `build_track_state: spans need the song timing` | 291–301 | sweep-tests4 | tempo-23 | test | spans need timing (tick-based ends) |
| 2048 | `window: a view into the state, not a copy` | 305–321 | sweep-tests4 | none | test | window is a view |

### `tests/test_user_messages.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2049 | `user_messages: database errors say to check the disk` | 37–52 | sweep-tests4 | display-28 | test | database error mapping; user sentences typed again as expected literals |
| 2050 | `user_messages: a missing or unreadable song file` | 54–70 | sweep-tests4 | display-28 | test | song file error mapping |
| 2051 | `user_messages: the no-notes message is already plain and passes through` | 72–76 | sweep-tests4 | display-28;sweep-tests4-12 | test | no-notes message passes through |
| 2052 | `user_messages: a broken search is reported as Hydra's bug` | 78–82 | sweep-tests4 | display-28 | test | broken search mapping |
| 2053 | `user_messages: leaderboard errors` | 84–98 | sweep-tests4 | display-28 | test | leaderboard error mapping |
| 2054 | `user_messages: report, rules, stored results, memory` | 100–118 | sweep-tests4 | display-28 | test | report/rules/blob/memory mapping |
| 2055 | `user_messages: anything else falls back, and the detail keeps the raw text` | 120–128 | sweep-tests4 | display-28 | test | fallback + kSomethingWentWrong + cancelled |

### `tests/test_utf8_paths.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2056 | `non_ascii_dir` | 32–42 | sweep-tests4 | none | none | temp folder helper |
| 2057 | `remove_dir` | 44–47 | sweep-tests4 | none | none | temp folder helper |
| 2058 | `read_file_text reads a file under a non-ASCII folder` | 51–62 | sweep-tests4 | none | test | read_file_text UTF-8 path plumbing |
| 2059 | `the settings INI saves and loads under a non-ASCII folder` | 64–77 | sweep-tests4 | store-16 | test | Settings save_file/load_file round trip (depth_value, dm_last_user) |
| 2060 | `the Preview renderer loads its assets from a non-ASCII folder WARP` | 79–91 | sweep-tests4 | none | test | renderer asset load under non-ASCII path (msaa 4 from 3d-config.json) |

### `tests/test_winstr.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2061 | `split_command_line_utf8 keeps a fullwidth slash in a chart path` | 13–25 | sweep-tests4 | none | test | split_command_line_utf8 |
| 2062 | `split_command_line_utf8 hands back UTF-8, not the ANSI code page` | 27–36 | sweep-tests4 | none | test | split_command_line_utf8 |
| 2063 | `split_command_line_utf8 of an empty command line is empty` | 38–41 | sweep-tests4 | none | test | split_command_line_utf8 |
| 2064 | `utf8_argv reads this process's command line` | 43–47 | sweep-tests4 | none | test | utf8_argv |

### `tests/ui/uitest_batch_reports.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2065 | `child_window` | 23–25 | sweep-tests4 | none | none | window lookup |
| 2066 | `batch_search` | 29–44 | sweep-tests4 | sweep-tests4-14;display-10 | copies | rebuilds the "Analyze search (N)..." label (owner render_actions_row in src/ui/library_toolbar.cpp) |
| 2067 | `dismiss_done` | 47–55 | sweep-tests4 | none | none | UI plumbing |
| 2068 | `test_batch_strip_drift` | 59–93 | sweep-tests4 | none | test | strip layout stability |
| 2069 | `test_settings_and_reports` | 95–134 | sweep-tests4 | store-16;score-22 | test | INI persists view_bass2x; dm report matched count |
| 2070 | `test_dm_compare_flow` | 139–218 | sweep-tests4 | fills-32 | test | compare gate at cap 8/4 and Hard/Expert with literal 4 instead of kCloneHeroSpCap; 1.0-fills leg not exercised here |
| 2071 | `test_report_buttons` | 223–272 | sweep-tests4 | sweep-tests4-14;display-10;store-4 | copies | second copy of the "Analyze search (N)..." label in this file (owner render_actions_row); redo skips nothing |
| 2072 | `test_batch_confirm` | 276–311 | sweep-tests4 | display-19;display-33;store-11;fills-27 | test | confirm text: "4 bars (Clone Hero's rule)", "10 ms", "2 scores", "Expert · Pro Drums · 2x Bass", corpus count 97 literal |
| 2073 | `test_batch_pause_stop` | 314–342 | sweep-tests4 | none | test | pause/resume/stop |
| 2074 | `test_batch_done_strip` | 345–377 | sweep-tests4 | none | test | done strip buttons |
| 2075 | `test_batch_open_failure` | 381–395 | sweep-tests4 | none | test | refused browser message |
| 2076 | `test_status_line` | 398–414 | sweep-tests4 | sweep-tests4-3 | test | news fades after 6 s (Yield(400) at 1/60 s mirrors render_status_line literal 6.0) |
| 2077 | `test_compare_disabled` | 419–443 | sweep-tests4 | fills-32 | test | compare gate at cap 6 and Hard, literal 4 |
| 2078 | `test_dialog_keys` | 446–468 | sweep-tests4 | none | test | dialog keys |
| 2079 | `batch_report_tests` | 472–487 | sweep-tests4 | none | none | registration |

### `tests/ui/uitest_details.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2080 | `test_analyze` | 23–58 | sweep-tests4 | store-7;store-8;screenA-2;store-1 | test | records per SP cap; Ready/NotAnalyzed |
| 2081 | `test_cap_switch` | 64–93 | sweep-tests4 | store-7;store-8 | test | cap switch swaps records |
| 2082 | `test_legacy_fills` | 99–140 | sweep-tests4 | fills-10;store-13;fills-32;store-7 | test | 1.0 fills keys a result; compare greys out |
| 2083 | `test_dynamics` | 142–183 | sweep-tests4 | screenA-34;screenA-36;screenA-37;parse-13 | test | Dynamics tab counts and 2x Bass off |
| 2084 | `test_dynamics_stored` | 189–268 | sweep-tests4 | store-20;parse-23 | test | stored dynamics reused; analysis stores them |
| 2085 | `test_stars` | 273–323 | display | display-22 | copies | recomputes cutoff + solo_bonus |
| 2086 | `test_details_close_teardown` | 328–349 | sweep-tests4 | none | test | teardown on hide |
| 2087 | `test_panel_open_close` | 353–378 | sweep-tests4 | sweep-tests4-1 | test | library > 0.9 / < 0.7 of the main window |
| 2088 | `test_panel_split` | 384–444 | sweep-tests4 | sweep-tests4-1 | copies | re-types the split clamp (owner render_library_and_panel in src/ui/library_view.cpp): min(share*room, room-px(kMinSongPanelW)) drops the px(320) floor; max(px(320.0f), 0.3*room) copies the unnamed 320 |
| 2089 | `test_panel_hide_library` | 450–502 | sweep-tests4 | sweep-tests4-1 | test | Hide library gives the panel the room |
| 2090 | `overflowing_windows` | 507–521 | sweep-tests4 | none | test | overflow detector |
| 2091 | `test_layout_sweep` | 526–585 | sweep-tests4 | screenA-37 | test | layout sweep; Themata "Dynamics enabled: no (markings ignored by Clone Hero)" |
| 2092 | `test_long_error_wraps` | 591–629 | sweep-tests4 | none | test | long error wraps |
| 2093 | `test_panel_prev_next` | 632–647 | sweep-tests4 | none | test | prev/next |
| 2094 | `test_panel_headline` | 652–676 | sweep-tests4 | screenA-1;screenA-2;screenA-4;score-15;display-1 | test | Burnout headline 378,315 / 3- 1 2 / 7 stars / 163.0 ms literals |
| 2095 | `test_panel_keeps_analysis` | 680–696 | sweep-tests4 | none | test | close keeps analysis |
| 2096 | `test_settings_lock` | 699–719 | sweep-tests4 | none | test | settings lock |
| 2097 | `details_tests` | 723–743 | sweep-tests4 | none | none | registration |

### `tests/ui/uitest_harness.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2098 | `capture_pixels` | 35–70 | sweep-tests4 | sweep-tests4-13 | copies | own staging-texture read-back (same steps as tests/warp_util.h read_pixels) |
| 2099 | `temp_root` | 72–78 | sweep-tests4 | none | none | temp path |
| 2100 | `init_scratch` | 81–97 | sweep-tests4 | none | none | path overrides |
| 2101 | `init_engine` | 99–115 | sweep-tests4 | none | none | test engine config |
| 2102 | `Harness::init_attached` | 119–127 | sweep-tests4 | none | none | attach mode |
| 2103 | `Harness::queue` | 129–148 | sweep-tests4 | sweep-tests4-8 | owns | test selection by name |
| 2104 | `Harness::print_results` | 150–165 | sweep-tests4 | none | none | result printing |
| 2105 | `Harness::init` | 167–203 | sweep-tests4 | sweep-tests4-13 | copies | own D3D11CreateDevice WARP call (tests/warp_util.h make_device does the same) |
| 2106 | `Harness::frame` | 205–218 | sweep-tests4 | none | none | frame loop |
| 2107 | `Harness::stop` | 220–225 | sweep-tests4 | none | none | stop |
| 2108 | `Harness::shutdown` | 227–239 | sweep-tests4 | none | none | shutdown |
| 2109 | `reset_app` | 241–290 | sweep-tests4 | store-16;sweep-tests4-1 | test | writes INI keys chartfolder/auto_open_report/depth_value=2; restores kDefaultLibraryShare; canned dm fetcher |
| 2110 | `wait_until` | 292–308 | sweep-tests4 | none | none | wait loop |
| 2111 | `jobs_busy` | 310–320 | sweep-tests4 | none | none | job polling |
| 2112 | `visible_text` | 322–329 | sweep-tests4 | none | none | frame text |
| 2113 | `yes_no` | 333–333 | sweep-tests4 | none | none | bool to text |
| 2114 | `dump_window` | 335–354 | sweep-tests4 | none | none | widget dump |
| 2115 | `dump_widgets` | 358–369 | sweep-tests4 | none | none | widget dump |
| 2116 | `dump_state` | 371–408 | sweep-tests4 | sweep-tests4-4;store-11;screenA-3 | copies | own status words current/stale/new (app says Analyzed/Stale/Not analyzed); calls chartmode_key and best_path |
| 2117 | `screenshot` | 410–423 | sweep-tests4 | none | none | screenshot |
| 2118 | `escape_ref` | 425–432 | sweep-tests4 | none | none | ref escaping |
| 2119 | `scan_library` | 435–450 | sweep-tests4 | none | none | scan helper |
| 2120 | `set_panel_ref` | 454–458 | sweep-tests4 | none | none | panel ref |
| 2121 | `open_details` | 461–475 | sweep-tests4 | none | none | open helper |
| 2122 | `open_titled` | 479–491 | sweep-tests4 | none | none | open helper |
| 2123 | `analyze_button_ref` | 495–498 | sweep-tests4 | sweep-tests4-5 | copies | re-types details_panel.cpp's NotAnalyzed -> "Analyze this song" / "Re-analyze" rule |
| 2124 | `analyze_open_song` | 501–509 | sweep-tests4 | store-1 | test | waits for Ready |
| 2125 | `open_preview` | 512–524 | sweep-tests4 | none | none | preview open helper |

### `tests/ui/uitest_harness.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2126 | `harness` | 100–102 | sweep-tests4 | none | none | accessor |

### `tests/ui/uitest_library.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2127 | `test_scan` | 19–31 | sweep-tests4 | none | test | scan flips is_rescan |
| 2128 | `test_difficulty` | 36–98 | sweep-tests4 | store-11;parse-13;parse-19 | test | chartmode_key strings "Hard Pro Drums, 1x Bass"/"Expert Pro Drums, 2x Bass"; 2x Bass Expert-only |
| 2129 | `test_rules_error` | 102–122 | sweep-tests4 | store-15;fills-22 | test | max_tied_paths = 0 is refused and blocks analysis |
| 2130 | `test_library_state_per_app` | 128–162 | sweep-tests4 | none | test | per-app library state |
| 2131 | `test_view_settings` | 166–215 | sweep-tests4 | store-11;store-16 | test | Pro Drums changes chart mode; folder removal persists |
| 2132 | `test_library_search` | 219–299 | sweep-tests4 | display-9;display-10;score-16 | test | search, rich-tag stripping, "5 of 97 charts", stars:9 error |
| 2133 | `test_library_sort_scroll` | 303–346 | sweep-tests4 | none | test | sort and scroll |
| 2134 | `library_table` | 349–356 | sweep-tests4 | none | none | table lookup |
| 2135 | `test_library_layout` | 361–457 | sweep-tests4 | sweep-tests4-1 | copies | px(321.0f) mirrors the unnamed px(320.0f) library minimum; caption centring formula re-typed from settings_bar.cpp |
| 2136 | `test_library_column_order` | 462–491 | sweep-tests4 | sweep-tests4-7 | copies | literal column index 4 for Best path (library_table.cpp setup order) |
| 2137 | `library_tests` | 495–508 | sweep-tests4 | none | none | registration |

### `tests/ui/uitest_main.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2138 | `usage` | 36–41 | sweep-tests4 | none | none | usage text |
| 2139 | `launch_child` | 59–79 | sweep-tests4 | none | none | process launch |
| 2140 | `run_parallel` | 83–182 | sweep-tests4 | sweep-tests4-8 | copies | same "script" skip and name match as Harness::queue |
| 2141 | `main` | 186–257 | sweep-tests4 | none | none | argument parsing |

### `tests/ui/uitest_paths.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2142 | `FakeClipboard::text` | 32–35 | sweep-tests4 | none | none | fake clipboard |
| 2143 | `FakeClipboard::FakeClipboard` | 39–47 | sweep-tests4 | none | none | fake clipboard |
| 2144 | `FakeClipboard::~FakeClipboard` | 48–51 | sweep-tests4 | none | none | fake clipboard |
| 2145 | `on_screen` | 54–56 | sweep-tests4 | none | none | text helper |
| 2146 | `open_burnout` | 59–71 | sweep-tests4 | screenA-2 | test | Burnout best path "3- 1 2" |
| 2147 | `test_paths_list` | 75–104 | sweep-tests4 | screenA-1;screenA-3;screenA-4;screenA-6;screenA-7;screenA-8;score-20;display-32 | test | path list headings, titles, 163.0 ms, "2,360 below optimal" |
| 2148 | `test_paths_rows` | 108–156 | sweep-tests4 | screenA-9;screenA-10;screenA-11;screenA-15;screenA-20;display-10 | test | activation rows, badge "squeeze out 163 ms", "3 notes near the SP end" |
| 2149 | `test_paths_backend_timings` | 159–197 | sweep-tests4 | backend-11;screenA-29;display-29;screenA-21;screenA-16;backend-5 | test | backend limit default 50, clamp 0..500 (600 -> 500 literal = kSqueezeWindowMs), "(-260)" |
| 2150 | `test_paths_folds_copy` | 200–236 | sweep-tests4 | screenA-27;screenA-28;screenA-41 | test | mult squeeze fold, breakdown, Copy path = pathstring_verbose |
| 2151 | `test_paths_uncounted` | 241–263 | sweep-tests4 | backend-13;backend-15;screenA-16 | test | "squeezed out (uncounted)" and its sentence |
| 2152 | `window_named` | 266–270 | sweep-tests4 | none | none | window lookup |
| 2153 | `test_paths_fit_narrow` | 276–298 | sweep-tests4 | none | test | narrow panel fit |
| 2154 | `test_paths_long_path` | 303–328 | sweep-tests4 | none | test | long path fit |
| 2155 | `text_w` | 331–336 | sweep-tests4 | sweep-tests4-9 | copies | font pixel size FontSizeBase*FontScaleMain*FontScaleDpi (ImGui's own rule) |
| 2156 | `narrowest_panel` | 339–349 | sweep-tests4 | none | test | narrowest panel |
| 2157 | `backend_table` | 353–371 | sweep-tests4 | sweep-tests4-10 | copies | rebuilds paths_tab.cpp's "##backends%d_%d_%d_%d" table id |
| 2158 | `test_paths_backend_fit` | 376–407 | sweep-tests4 | screenA-21 | test | rating column wraps |
| 2159 | `test_paths_row_layout` | 413–452 | sweep-tests4 | screenA-11 | copies | calls activation_row_layout; badge wording "early fill 999 ms"/"squeeze out 999 ms" typed again (owner activation_badge in src/app/path_view.cpp) |
| 2160 | `test_paths_length_backfill` | 457–477 | sweep-tests4 | store-17;parse-21;tempo-13;display-35 | test | song length backfill; timeline end "m96" |
| 2161 | `register_paths_tests` | 482–504 | sweep-tests4 | none | none | registration |

### `tests/ui/uitest_preview.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2162 | `test_preview` | 23–73 | sweep-tests4 | sweep-tests4-11 | test | pins PreviewLoadJob::Progress fraction/label |
| 2163 | `test_analyze_on_preview` | 79–90 | sweep-tests4 | none | test | analyze from Preview stores record |
| 2164 | `preview_path_combo` | 96–99 | sweep-tests4 | none | none | combo id |
| 2165 | `pick_preview_path` | 103–115 | sweep-tests4 | screenA-3 | wrapper | calls build_path_buttons + preview_path_label |
| 2166 | `test_preview_path_overlay` | 122–209 | sweep-tests4 | score-27;screenA-3 | test | assumes Preview list index == all_paths() index (true today: build_path_buttons walks build_path_list groups in all_paths order) |
| 2167 | `test_preview_controls` | 214–272 | sweep-tests4 | screenB-9;screenB-8;tempo-2 | test | score box at end = all_paths().front()->totalscore(); 5 s jumps clamp; one tick step < 20 ms |
| 2168 | `test_preview_drain_box` | 277–319 | sweep-tests4 | screenB-3;screenB-4;screenB-5;screenB-6 | test | drain box idle/active wording |
| 2169 | `DisplayWidth::DisplayWidth` | 326–328 | sweep-tests4 | none | none | guard |
| 2170 | `DisplayWidth::~DisplayWidth` | 329–329 | sweep-tests4 | none | none | guard |
| 2171 | `test_preview_overlay_fit` | 337–360 | sweep-tests4 | none | test | overlay scale bounds (kOverlayMinScale) |
| 2172 | `test_preview_buttons_keys` | 365–421 | sweep-tests4 | tempo-2 | test | transport buttons and keys, 5-tick steps |
| 2173 | `test_scrub_hold` | 427–450 | sweep-tests4 | none | test | scrub hold pauses |
| 2174 | `test_layout_drift` | 455–476 | sweep-tests4 | none | test | layout drift |
| 2175 | `open_burnout_preview` | 480–498 | sweep-tests4 | screenA-2 | test | Burnout preview, 3 scrub marks |
| 2176 | `test_preview_path_picker` | 504–596 | sweep-tests4 | screenB-18;screenA-3;screenB-11;sweep-tests4-9 | copies | "(optimal)"/"(best all-0)" labels; combo width and font size re-typed twice; ellipsize called |
| 2177 | `test_preview_error_wraps` | 600–638 | sweep-tests4 | none | test | preview error wraps |
| 2178 | `test_preview_overlay_steady` | 644–678 | sweep-tests4 | tempo-6 | test | measure format m32.1.0 / m58.1.0 |
| 2179 | `test_preview_activation_jumps` | 682–733 | sweep-tests4 | screenB-11;screenB-12;tempo-6;tempo-13 | test | next-act box, readout ends "/4" (cap literal), time box length "m96.3.240" (vs Paths timeline "m96") |
| 2180 | `preview_tests` | 737–754 | sweep-tests4 | none | none | registration |

### `tests/ui/uitest_script.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2181 | `split_verb` | 20–29 | sweep-tests4 | none | none | script parsing |
| 2182 | `run_script` | 31–122 | sweep-tests4 | none | none | script interpreter |
| 2183 | `register_script_test` | 126–130 | sweep-tests4 | none | none | registration |

### `tests/ui/uitest_tests.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2184 | `register_tests` | 18–59 | sweep-tests4 | none | none | registration order |

### `tests/warp_util.h`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2185 | `make_device` | 17–23 | sweep-tests4 | sweep-tests4-13 | owns | WARP device helper |
| 2186 | `read_pixels` | 27–51 | sweep-tests4 | sweep-tests4-13 | owns | texture read-back helper |
| 2187 | `pixel` | 53–55 | sweep-tests4 | none | none | pixel index |

### `tools/bench.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2188 | `secs_since` | 42–44 | sweep-core1 | none | none | timer |
| 2189 | `folder_breakdown` | 48–91 | sweep-core1 | screenB-25;store-26 | wrapper | app::Settings defaults + to_analysis_settings; prints mslimit_value without checking mslimit_enabled (true by default) |
| 2190 | `scan_mode` | 100–176 | sweep-core1 | none | none | scan timing |
| 2191 | `corpus_bench` | 178–208 | sweep-core1 | screenB-25;store-26 | copies | app::Settings defaults (hard-codes cap 4, Scores depth 4, no ms filter, pro+2x Expert) |
| 2192 | `dump_db` | 212–231 | sweep-core1 | none | none | db dump |
| 2193 | `main` | 233–275 | sweep-core1 | store-15 | wrapper | load_rules_file |

### `tools/ch_probe/debugger.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2194 | `decode_xmm0_double` | 49–58 | sweep-probe1 | sweep-probe1-16 | copies | process.decode_double (own struct.unpack '<d') |
| 2195 | `adjust_rip_after_int3` | 61–68 | sweep-probe1 | sweep-probe1-34 | owns |  |
| 2196 | `mask_breakpoints` | 71–87 | sweep-probe1 | none | none |  |
| 2197 | `_BP.__init__` | 96–99 | sweep-probe1 | none | none |  |
| 2198 | `BreakpointTable.__init__` | 116–119 | sweep-probe1 | none | none |  |
| 2199 | `BreakpointTable.add` | 121–126 | sweep-probe1 | none | none |  |
| 2200 | `BreakpointTable.arm` | 128–136 | sweep-probe1 | none | none |  |
| 2201 | `BreakpointTable.disarm` | 138–146 | sweep-probe1 | none | none |  |
| 2202 | `BreakpointTable.remove` | 148–155 | sweep-probe1 | none | none |  |
| 2203 | `BreakpointTable.addresses` | 157–159 | sweep-probe1 | none | none |  |
| 2204 | `BreakpointTable.has` | 163–164 | sweep-probe1 | none | none |  |
| 2205 | `BreakpointTable.is_armed` | 166–168 | sweep-probe1 | none | none |  |
| 2206 | `BreakpointTable.original` | 170–172 | sweep-probe1 | none | none |  |
| 2207 | `BreakpointTable.callback` | 174–176 | sweep-probe1 | none | none |  |
| 2208 | `BreakpointTable.armed_originals` | 178–185 | sweep-probe1 | none | none |  |
| 2209 | `ThreadContext.__init__` | 204–207 | sweep-probe1 | none | none |  |
| 2210 | `ThreadContext.xmm0_double` | 209–211 | sweep-probe1 | sweep-probe1-16 | wrapper | calls decode_xmm0_double |
| 2211 | `_Win32.__init__` | 383–431 | sweep-probe1 | sweep-probe1-25 | copies | process._kernel32/_make_reader/_make_writer (second kernel32 binding set) |
| 2212 | `Debugger.__init__` | 446–457 | sweep-probe1 | none | none |  |
| 2213 | `Debugger.attach` | 461–484 | sweep-probe1 | sweep-probe1-24 | owns | kill-on-exit off after DebugActiveProcess |
| 2214 | `Debugger.stop` | 486–509 | sweep-probe1 | sweep-probe1-24 | owns |  |
| 2215 | `Debugger._drain` | 511–525 | sweep-probe1 | sweep-probe1-24 | owns |  |
| 2216 | `Debugger._rewind_rip` | 527–542 | sweep-probe1 | sweep-probe1-34 | wrapper | adjust_rip_after_int3 |
| 2217 | `Debugger.set_breakpoint` | 546–550 | sweep-probe1 | none | none |  |
| 2218 | `Debugger.clear_breakpoint` | 552–554 | sweep-probe1 | none | none |  |
| 2219 | `Debugger.read` | 558–561 | sweep-probe1 | none | none |  |
| 2220 | `Debugger.write` | 563–565 | sweep-probe1 | none | none |  |
| 2221 | `Debugger._raw_read` | 567–576 | sweep-probe1 | sweep-probe1-25 | copies | process._make_reader |
| 2222 | `Debugger._raw_write` | 578–589 | sweep-probe1 | sweep-probe1-25 | copies | process._make_writer |
| 2223 | `Debugger.run` | 593–617 | sweep-probe1 | sweep-probe1-24 | owns |  |
| 2224 | `Debugger._handle_exception` | 619–657 | sweep-probe1 | sweep-probe1-24 | owns |  |
| 2225 | `Debugger._on_our_breakpoint` | 659–689 | sweep-probe1 | sweep-probe1-34;sweep-probe1-24 | wrapper | adjust_rip_after_int3 |
| 2226 | `Debugger._context_from_ctx` | 692–707 | sweep-probe1 | none | none |  |

### `tools/ch_probe/engine.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2227 | `EngineModel.__init__` | 41–48 | sweep-probe1 | none | none |  |
| 2228 | `EngineModel.capture_object` | 52–84 | sweep-probe1 | sweep-probe1-35 | owns |  |
| 2229 | `EngineModel.capture_object._on_ctor` | 71–72 | sweep-probe1 | sweep-probe1-35 | owns | object ptr = rcx at ctor |
| 2230 | `EngineModel.use_object` | 86–90 | sweep-probe1 | none | none |  |
| 2231 | `EngineModel._addr` | 94–100 | sweep-probe1 | sweep-probe1-1 | owns |  |
| 2232 | `EngineModel.total_window` | 102–106 | sweep-probe1 | sweep-probe1-1 | owns | OFF_TOTAL_WINDOW |
| 2233 | `EngineModel.back_window` | 108–110 | sweep-probe1 | sweep-probe1-1 | owns | OFF_BACK_WINDOW |
| 2234 | `EngineModel.front_window` | 112–114 | sweep-probe1 | sweep-probe1-1 | owns | OFF_FRONT_WINDOW |
| 2235 | `EngineModel.hit_time` | 116–119 | sweep-probe1 | sweep-probe1-1 | owns | OFF_HIT_TIME |
| 2236 | `EngineModel.score` | 121–124 | score | score-25 | wrapper | EngineModel.score reads C.OFF_SCORE |
| 2237 | `EngineModel.note_count` | 126–128 | sweep-probe1 | sweep-probe1-1 | owns | OFF_NOTE_COUNT |
| 2238 | `EngineModel.precision_mode` | 130–138 | sweep-probe1 | sweep-probe1-2 | owns | flags & PRECISION_MODE_BIT |
| 2239 | `EngineModel.song_clock` | 140–144 | sweep-probe1 | sweep-probe1-1 | owns | OFF_SONG_CLOCK |
| 2240 | `EngineModel.constants` | 148–174 | sweep-probe1 | sweep-probe1-36 | owns | keys normal_back/normal_front/precision_*/hitcheck_threshold are literals, only divisor/exponent/prefixes come from constants |

### `tools/ch_probe/engine_finder.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2241 | `is_rw` | 51–55 | sweep-probe1 | none | none |  |
| 2242 | `hits_in_region` | 58–69 | sweep-probe1 | sweep-probe1-5 | owns | object = match - OFF_BACK_WINDOW |
| 2243 | `scan_for_engine` | 72–112 | sweep-probe1 | sweep-probe1-5 | owns |  |
| 2244 | `_raw` | 117–118 | sweep-probe1 | none | none |  |
| 2245 | `normal_pattern` | 121–123 | sweep-probe1 | sweep-probe1-5 | owns |  |
| 2246 | `all_patterns` | 126–131 | sweep-probe1 | sweep-probe1-5 | owns |  |
| 2247 | `find_live_engine` | 136–183 | sweep-probe1 | sweep-probe1-3;sweep-probe1-4 | owns | clock moves >1e-6 in 0.12 s; skip total window <0.001; MODULE_SPAN |

### `tools/ch_probe/experiments/active_probe.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2248 | `plan_inputs` | 81–90 | sweep-probe1 | tempo-1;tempo-2 | wrapper | probe_chart.probe_note_ticks; then treats ticks as ms (tick==ms by probe_songs RESOLUTION/BPM) |
| 2249 | `write_probe_song` | 93–107 | sweep-probe1 | sweep-probe1-20;sweep-probe1-29;sweep-probe1-37 | copies | probe_songs.write_song (own folder writer, own length = last+SILENCE_MS) |
| 2250 | `ActiveCollector.__init__` | 113–116 | sweep-probe1 | none | none |  |
| 2251 | `ActiveCollector.rows` | 119–120 | sweep-probe1 | none | none |  |
| 2252 | `ActiveCollector.add_row` | 122–123 | sweep-probe1 | none | none |  |
| 2253 | `ActiveCollector.on_hit_check` | 125–129 | sweep-probe1 | none | none |  |
| 2254 | `drive_inputs` | 132–182 | sweep-probe1 | sweep-probe1-6;sweep-probe1-7;sweep-probe1-11;sweep-probe1-27 | copies | walk_edges.Row.measured_ms / walk_edges.main (hit=score rose after SETTLE_MS; skip notes <raw+150 ms) |
| 2255 | `drive_inputs.wait_until` | 141–159 | sweep-probe1 | sweep-probe1-9;sweep-probe1-10 | copies | walk_edges.main.wait_until (same loop, same 1.0/5.0/0.04/0.03/0.5) |
| 2256 | `find_any_mode_engine` | 185–187 | sweep-probe1 | sweep-probe1-3 | wrapper | engine_finder.find_live_engine + all_patterns |
| 2257 | `find_game_window` | 190–191 | sweep-probe1 | sweep-probe1-22 | copies | no owner; FindWindowW 'Clone Hero' also inline in walk_edges.main, play_chart.main, pad_flash_test.main |
| 2258 | `run_active_probe` | 194–267 | sweep-probe1 | sweep-probe1-15 | wrapper | verify_targets |
| 2259 | `run_active_probe.focus` | 227–229 | sweep-probe1 | sweep-probe1-22 | copies | no owner (SetForegroundWindow) |
| 2260 | `run_active_probe.worker` | 235–241 | sweep-probe1 | none | none |  |
| 2261 | `_write_rows` | 270–291 | sweep-probe1 | none | none |  |
| 2262 | `_print_summary` | 294–303 | sweep-probe1 | none | none |  |
| 2263 | `_ms_list` | 306–307 | sweep-probe1 | none | none |  |
| 2264 | `main` | 310–319 | sweep-probe1 | none | none |  |

### `tools/ch_probe/experiments/analysis.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2265 | `find_window_edge` | 66–123 | sweep-probe1 | sweep-probe1-18 | owns |  |
| 2266 | `clamp_verdict` | 153–206 | sweep-probe1 | sweep-probe1-12 | owns | tolerance 1.0 ms, decisive 0.8 |
| 2267 | `predicted_window_normal` | 211–235 | sweep-probe1 | sweep-probe1-17 | owns |  |
| 2268 | `predicted_window_precision` | 238–259 | sweep-probe1 | sweep-probe1-17 | copies | predicted_window_normal (inner term (t*c1 - t**e*c2)*c3 and t=spacing*divisor written twice) |
| 2269 | `group_by_spacing` | 264–276 | sweep-probe1 | none | none |  |
| 2270 | `normal_formula_constants` | 289–303 | sweep-probe1 | sweep-probe1-36 | wrapper | constants keys |
| 2271 | `summarize_active` | 306–341 | sweep-probe1 | sweep-probe1-18;sweep-probe1-17 | wrapper | find_window_edge + predicted_window_normal |

### `tools/ch_probe/experiments/find_clock3.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2272 | `find_all_engines` | 30–35 | sweep-probe1 | sweep-probe1-4;sweep-probe1-5 | copies | engine_finder.MODULE_SPAN (literal 0x4000000) + normal_pattern |
| 2273 | `safe_read` | 38–42 | sweep-probe1 | none | none |  |
| 2274 | `main` | 45–186 | sweep-probe1 | sweep-probe1-1;sweep-probe1-3;sweep-probe1-16;sweep-probe1-31 | copies | EngineModel.total_window (literal +0x20); engine_finder.find_live_engine (own rule: window changed >0.0001 in 0.2 s and w1>0.001) |

### `tools/ch_probe/experiments/find_engine.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2275 | `main` | 33–123 | sweep-probe1 | sweep-probe1-1;sweep-probe1-2;sweep-probe1-3;sweep-probe1-4;sweep-probe1-5;sweep-probe1-16;sweep-probe1-31 | copies | EngineModel readers (literal 0x20/0x30/0x38/0x8C/0x198), precision_mode, find_live_engine (own rule tw>0.001), MODULE_SPAN literal |

### `tools/ch_probe/experiments/hit_detect.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2276 | `first_engine_with_window` | 40–55 | sweep-probe1 | sweep-probe1-3;sweep-probe1-4;sweep-probe1-5 | copies | engine_finder.find_live_engine (first heap hit with tw>0.001), MODULE_SPAN literal, normal_pattern |
| 2277 | `main` | 58–190 | sweep-probe1 | sweep-probe1-1;sweep-probe1-6;sweep-probe1-7;sweep-probe1-23;sweep-probe1-31;sweep-probe1-33 | copies | constants OFF_* (own OFF_TOTAL_WINDOW/OFF_SCORE), hit = notes-hit counter rose, own chord press 5 ms hold, 2X kick vk 0x4F not in DEFAULT_BINDINGS, lanes as literal 0..4 |
| 2278 | `main.read_window` | 83–84 | sweep-probe1 | sweep-probe1-1 | copies | EngineModel.total_window (own OFF_TOTAL_WINDOW=0x20) |
| 2279 | `main.read_hits` | 86–87 | sweep-probe1 | sweep-probe1-1;sweep-probe1-6 | owns | OFF_HITS=0xb0 lives here, not in constants.py |
| 2280 | `main.read_time_a` | 89–90 | sweep-probe1 | sweep-probe1-1 | owns | OFF_TIME_A=0x28 lives here, not in constants.py |
| 2281 | `main.read_hit_time` | 92–95 | sweep-probe1 | sweep-probe1-1 | copies | EngineModel.song_clock (reads constants.OFF_SONG_CLOCK directly) |
| 2282 | `main.read_score` | 97–98 | score | score-25 | copies | hit_detect main.read_score uses its own OFF_SCORE = 0x94; owner constants.py |

### `tools/ch_probe/experiments/key_delivery_test.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2283 | `main` | 46–68 | sweep-probe1 | sweep-probe1-23 | wrapper | DEFAULT_BINDINGS/LANE_NAMES |

### `tools/ch_probe/experiments/live.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2284 | `Snapshot.precision` | 42–43 | sweep-probe1 | sweep-probe1-2 | copies | EngineModel.precision_mode |
| 2285 | `decode_snapshot` | 46–60 | sweep-probe1 | sweep-probe1-1;sweep-probe1-16 | copies | EngineModel readers (second decoder of OFF_TOTAL_WINDOW/SONG_CLOCK/SCORE/HIT_TIME/FLAGS, window x1000) and process.decode_* |
| 2286 | `decode_snapshot.dbl` | 48–49 | sweep-probe1 | sweep-probe1-16 | copies | process.decode_double |
| 2287 | `decode_snapshot.u32` | 51–52 | sweep-probe1 | sweep-probe1-16 | copies | process.decode_u32 |
| 2288 | `read_snapshot` | 63–64 | sweep-probe1 | sweep-probe1-1 | wrapper | decode_snapshot |
| 2289 | `load_manifest` | 67–69 | sweep-probe1 | none | none |  |

### `tools/ch_probe/experiments/milestone1.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2290 | `main` | 29–71 | sweep-probe1 | sweep-probe1-15;sweep-probe1-36 | copies | EngineModel.constants (re-lists the .rdata constant set); verify_targets wrapper |

### `tools/ch_probe/experiments/milestone2.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2291 | `main` | 32–91 | sweep-probe1 | sweep-probe1-35;sweep-probe1-31 | wrapper | EngineModel.capture_object; distinct window by round(ms,2) |

### `tools/ch_probe/experiments/pad_flash_test.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2292 | `main` | 50–73 | sweep-probe1 | sweep-probe1-22;sweep-probe1-23 | copies | sp1-22 no owner (FindWindowW inline); bindings via DEFAULT_BINDINGS |

### `tools/ch_probe/experiments/passive_probe.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2293 | `find_any_mode_engine` | 64–66 | sweep-probe1 | sweep-probe1-3 | wrapper | engine_finder (identical twin of active_probe.find_any_mode_engine) |
| 2294 | `PassiveCollector.__init__` | 76–82 | sweep-probe1 | none | none |  |
| 2295 | `PassiveCollector.rows` | 85–86 | sweep-probe1 | none | none |  |
| 2296 | `PassiveCollector.on_formula_entry` | 88–104 | sweep-probe1 | sweep-probe1-16 | copies | process.decode_u64 (own struct.unpack '<Q') |
| 2297 | `PassiveCollector.on_formula_return` | 106–109 | sweep-probe1 | none | none |  |
| 2298 | `PassiveCollector._finish_pending` | 111–116 | sweep-probe1 | sweep-probe1-1 | wrapper | EngineModel.total_window x1000 |
| 2299 | `run_passive_probe` | 119–171 | sweep-probe1 | sweep-probe1-12;sweep-probe1-13;sweep-probe1-14;tempo-24 | copies | whole-window edge = 2*EXPECT_*_BACK_MS (170) vs measured cap 171.43 in watch_window.CAP_EXPECT_MS; calls clamp_verdict |
| 2300 | `_write_rows` | 174–195 | sweep-probe1 | none | none |  |
| 2301 | `_print_first_rows` | 198–203 | sweep-probe1 | none | none |  |
| 2302 | `_print_verdict` | 206–218 | sweep-probe1 | none | none |  |
| 2303 | `main` | 221–225 | sweep-probe1 | none | none |  |

### `tools/ch_probe/experiments/play_chart.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2304 | `chart_notes_to_lanes` | 50–67 | parse | parse-10;parse-11;parse-13 | copies | .chart lanes: N 5 green, 32 always, cymbal marker ignores pro; owner ChartParser::optype |
| 2305 | `parse_chart` | 89–130 | parse | parse-10;parse-19;parse-26 | copies | regex .chart reader: [ExpertDrums] fixed, Resolution fallback 480, tempo fallback 120, no disco, no TS |
| 2305 | `parse_chart` | 89–130 | tempo | tempo-10;tempo-1 | copies | own .chart parse; 480/120 fallbacks |
| 2306 | `midi_notes_to_lanes` | 153–170 | parse | parse-10;parse-11;parse-13 | copies | MIDI lanes: 95 always kick, tom marker only same tick; owner MidiParser::optype/op_tom |
| 2307 | `_read_vlq` | 173–179 | parse | none | copies | second VLQ reader (owner read_varlen in midi.cpp) |
| 2308 | `parse_midi` | 182–287 | parse | parse-9;parse-14;parse-20;parse-26 | copies | own MIDI reader: track by "DRUMS" substring, no disco, no flam, tempo fallback 120 |
| 2308 | `parse_midi` | 182–287 | tempo | tempo-10;tempo-1 | copies | own MIDI tempo parse |
| 2309 | `ticks_to_seconds` | 290–303 | parse | none | none | timing family (tick->seconds copy) |
| 2309 | `ticks_to_seconds` | 290–303 | tempo | tempo-1;tempo-3 | copies | own tick->seconds walk; owner MsIndex::at |
| 2310 | `main` | 306–475 | parse | parse-2 | copies | notes.chart wins over notes.mid (Hydra: mid wins) |
| 2311 | `main.ensure_focus` | 348–350 | sweep-probe1 | sweep-probe1-22 | copies | no owner (SetForegroundWindow) |
| 2312 | `main.read_clock` | 352–353 | sweep-probe1 | sweep-probe1-1 | wrapper | EngineModel.song_clock |
| 2313 | `main.read_score` | 355–356 | sweep-probe1 | score-25 | wrapper | EngineModel.score |

### `tools/ch_probe/experiments/poll_windows.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2314 | `main` | 43–144 | sweep-probe1 | sweep-probe1-1;sweep-probe1-3;sweep-probe1-4;sweep-probe1-5;sweep-probe1-12;sweep-probe1-31 | copies | EngineModel (literal 0x20/0x30), find_live_engine (heap_hits[0]), MODULE_SPAN literal, analysis.clamp_verdict (inline w_max > 2*back+0.5) |

### `tools/ch_probe/experiments/test_attach.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2315 | `main` | 28–86 | sweep-probe1 | sweep-probe1-24 | copies | Debugger.attach/run (own pump, kill-on-exit left on) |

### `tools/ch_probe/experiments/walk_edges.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2316 | `parse_range` | 60–67 | sweep-probe1 | none | none |  |
| 2317 | `build_schedule` | 70–80 | sweep-probe1 | none | none |  |
| 2318 | `SongClock.__init__` | 93–102 | sweep-probe1 | sweep-probe1-8 | owns |  |
| 2319 | `SongClock.read` | 104–115 | sweep-probe1 | sweep-probe1-8 | owns | fresh 2 ms, fill cap 50 ms |
| 2320 | `Row.measured_ms` | 129–131 | sweep-probe1 | sweep-probe1-7 | owns |  |
| 2321 | `summarize` | 134–176 | sweep-probe1 | sweep-probe1-18 | copies | analysis.find_window_edge (edge = between widest hit and narrowest miss) |
| 2322 | `main` | 181–286 | sweep-probe1 | sweep-probe1-6;sweep-probe1-11;sweep-probe1-22;sweep-probe1-27;sweep-probe1-1 | copies | hit=score rose after SETTLE_MS (no owner); skip <raw+150 ms; FindWindowW inline; raw OFF_SONG_CLOCK read instead of EngineModel.song_clock |
| 2323 | `main.wait_until` | 224–240 | sweep-probe1 | sweep-probe1-9;sweep-probe1-10 | copies | active_probe.drive_inputs.wait_until twin (no owner) |

### `tools/ch_probe/experiments/watch_window.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2324 | `changes` | 64–71 | sweep-probe1 | sweep-probe1-31 | copies | no owner (change epsilon 1e-6) |
| 2325 | `value_at` | 74–81 | sweep-probe1 | none | none |  |
| 2326 | `block_spans` | 84–104 | sweep-probe1 | none | none |  |
| 2327 | `note_around` | 107–116 | sweep-probe1 | none | none |  |
| 2328 | `steady_value` | 119–133 | sweep-probe1 | none | none |  |
| 2329 | `_gap` | 136–137 | sweep-probe1 | none | none |  |
| 2330 | `_note_label` | 140–143 | sweep-probe1 | none | none |  |
| 2331 | `window_report` | 146–197 | sweep-probe1 | sweep-probe1-12;sweep-probe1-13 | copies | no owner for cap value (CAP_EXPECT_MS 171.43, CAP_CHECK_FROM_GAP_MS 170, match 0.01); analysis.clamp_verdict is the clamp owner |
| 2332 | `hit_time_report` | 200–225 | sweep-probe1 | sweep-probe1-6 | copies | hit = score rose (no owner); hit-time match 10 ms |
| 2333 | `clock_step_line` | 228–232 | sweep-probe1 | none | none |  |
| 2334 | `resolve_song_dir` | 237–242 | sweep-probe1 | sweep-probe1-21 | wrapper | live.PROBE_ROOT |
| 2335 | `main` | 245–325 | sweep-probe1 | sweep-probe1-1;sweep-probe1-9;sweep-probe1-31 | copies | raw back/front reads via constants instead of EngineModel; stall STALL_S 8.0 vs 5.0 elsewhere; change epsilon 1e-6 inline |

### `tools/ch_probe/input_driver.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2336 | `InputDriver.__init__` | 118–135 | sweep-probe1 | none | none |  |
| 2337 | `InputDriver.set_binding` | 139–145 | sweep-probe1 | sweep-probe1-23 | owns |  |
| 2338 | `InputDriver.get_binding` | 147–153 | sweep-probe1 | sweep-probe1-23 | owns |  |
| 2339 | `InputDriver.tap` | 157–161 | sweep-probe1 | none | none |  |
| 2340 | `InputDriver.press_chord` | 163–182 | sweep-probe1 | sweep-probe1-33 | owns | hold 3 ms |
| 2341 | `InputDriver.send_key` | 184–206 | sweep-probe1 | none | none |  |
| 2342 | `InputDriver.schedule_hit` | 210–229 | sweep-probe1 | sweep-probe1-10 | owns | poll until clock() >= target |

### `tools/ch_probe/interfaces.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2343 | `ProcessHandle.resolve` | 36–38 | sweep-probe1 | none | none |  |
| 2344 | `ProcessHandle.read` | 40–41 | sweep-probe1 | none | none |  |
| 2345 | `ProcessHandle.write` | 43–44 | sweep-probe1 | none | none |  |
| 2346 | `ProcessHandle.read_double` | 46–48 | sweep-probe1 | none | none |  |
| 2347 | `ProcessHandle.read_u32` | 50–51 | sweep-probe1 | none | none |  |
| 2348 | `ProcessHandle.read_u64` | 53–54 | sweep-probe1 | none | none |  |
| 2349 | `ProcessHandle.read_const_double` | 56–58 | sweep-probe1 | none | none |  |
| 2350 | `ProcessHandle.verify_targets` | 60–65 | sweep-probe1 | none | none |  |
| 2351 | `open_process` | 68–72 | sweep-probe1 | none | none |  |
| 2352 | `ThreadContext.xmm0_double` | 90–91 | sweep-probe1 | none | none |  |
| 2353 | `Debugger.attach` | 103–104 | sweep-probe1 | none | none |  |
| 2354 | `Debugger.set_breakpoint` | 106–108 | sweep-probe1 | none | none |  |
| 2355 | `Debugger.clear_breakpoint` | 110–111 | sweep-probe1 | none | none |  |
| 2356 | `Debugger.read` | 113–114 | sweep-probe1 | none | none |  |
| 2357 | `Debugger.write` | 116–117 | sweep-probe1 | none | none |  |
| 2358 | `Debugger.run` | 119–122 | sweep-probe1 | none | none |  |
| 2359 | `Debugger.stop` | 124–125 | sweep-probe1 | none | none |  |
| 2360 | `EngineModel.capture_object` | 140–142 | sweep-probe1 | none | none |  |
| 2361 | `EngineModel.use_object` | 144–146 | sweep-probe1 | none | none |  |
| 2362 | `EngineModel.score` | 148–150 | sweep-probe1 | none | none |  |
| 2363 | `EngineModel.total_window` | 152–154 | sweep-probe1 | none | none |  |
| 2364 | `EngineModel.back_window` | 156–157 | sweep-probe1 | none | none |  |
| 2365 | `EngineModel.front_window` | 159–160 | sweep-probe1 | none | none |  |
| 2366 | `EngineModel.hit_time` | 162–163 | sweep-probe1 | none | none |  |
| 2367 | `EngineModel.note_count` | 165–166 | sweep-probe1 | none | none |  |
| 2368 | `EngineModel.precision_mode` | 168–170 | sweep-probe1 | none | none |  |
| 2369 | `EngineModel.song_clock` | 172–174 | sweep-probe1 | none | none |  |
| 2370 | `EngineModel.constants` | 176–179 | sweep-probe1 | none | none |  |
| 2371 | `generate_probe_chart` | 186–198 | sweep-probe1 | sweep-probe1-32 | copies | probe_chart.generate_probe_chart defaults (192/120/0) restated in Protocol stub |
| 2372 | `InputDriver.set_binding` | 211–214 | sweep-probe1 | none | none |  |
| 2373 | `InputDriver.schedule_hit` | 216–221 | sweep-probe1 | none | none |  |
| 2374 | `InputDriver.tap` | 223–225 | sweep-probe1 | none | none |  |
| 2375 | `InputDriver.send_key` | 227–229 | sweep-probe1 | none | none |  |
| 2376 | `InputDriver.press_chord` | 231–233 | sweep-probe1 | none | none |  |

### `tools/ch_probe/ocr.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2377 | `parse_accuracy_text` | 30–49 | sweep-probe1 | sweep-probe1-30 | owns |  |

### `tools/ch_probe/probe_chart.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2378 | `ms_to_ticks` | 48–61 | tempo | tempo-2 | copies | ms->ticks round (half-even); owner MsIndex::tick_at_ms |
| 2379 | `_song_section` | 64–78 | sweep-probe1 | sweep-probe1-19 | owns |  |
| 2380 | `_sync_track_section` | 81–94 | sweep-probe1 | sweep-probe1-19 | owns | B = int(round(bpm*1000)) |
| 2381 | `_expert_drums_section` | 97–107 | sweep-probe1 | sweep-probe1-19 | owns |  |
| 2382 | `probe_note_ticks` | 110–128 | tempo | tempo-9 | copies | whole note = 4*res |
| 2383 | `build_probe_chart_text` | 131–151 | sweep-probe1 | sweep-probe1-19;sweep-probe1-32 | owns | defaults 192/120 restated in probe_note_ticks, build_probe_chart_text, generate_probe_chart |
| 2384 | `generate_probe_chart` | 154–176 | sweep-probe1 | sweep-probe1-32 | wrapper | build_probe_chart_text |

### `tools/ch_probe/probe_songs.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2385 | `Timeline.__init__` | 74–76 | sweep-probe1 | none | none |  |
| 2386 | `Timeline.add` | 78–79 | sweep-probe1 | none | none |  |
| 2387 | `Timeline.run` | 81–86 | sweep-probe1 | none | none |  |
| 2388 | `Timeline.step` | 88–89 | sweep-probe1 | none | none |  |
| 2389 | `Timeline.silence` | 91–92 | sweep-probe1 | none | none |  |
| 2390 | `window_map` | 95–112 | sweep-probe1 | sweep-probe1-13 | none | lays out CAP_GAPS_MS incl. 170; no cap value asserted |
| 2391 | `edge_walk` | 115–118 | sweep-probe1 | none | none |  |
| 2392 | `chart_text` | 121–142 | sweep-probe1 | sweep-probe1-19 | copies | probe_chart._song_section/_sync_track_section/_expert_drums_section (own header, B=int(BPM*1000) truncates, own KICK=0 not constants.PROBE_CHART_NOTE_KICK) |
| 2393 | `song_ini` | 145–157 | sweep-probe1 | sweep-probe1-37 | owns |  |
| 2394 | `manifest` | 160–181 | sweep-probe1 | none | none |  |
| 2395 | `write_silent_ogg` | 184–193 | sweep-probe1 | none | none |  |
| 2396 | `write_song` | 196–208 | sweep-probe1 | sweep-probe1-20;sweep-probe1-29 | owns | length = last note + SILENCE_MS |
| 2397 | `main` | 211–219 | sweep-probe1 | sweep-probe1-29 | copies | probe_songs.write_song (recomputes last+SILENCE_MS for the print) |

### `tools/ch_probe/process.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2398 | `decode_double` | 38–43 | sweep-probe1 | sweep-probe1-16 | owns |  |
| 2399 | `decode_u32` | 46–50 | sweep-probe1 | sweep-probe1-16 | owns |  |
| 2400 | `decode_u64` | 53–57 | sweep-probe1 | sweep-probe1-16 | owns |  |
| 2401 | `check_normal_constants` | 60–90 | sweep-probe1 | sweep-probe1-15;sweep-probe1-14;tempo-24 | owns | EXPECT_NORMAL_BACK_S/FRONT_S, tolerance CONST_MATCH_TOLERANCE_MS/1000 |
| 2402 | `Process.__init__` | 122–135 | sweep-probe1 | none | none |  |
| 2403 | `Process.handle` | 138–141 | sweep-probe1 | none | none |  |
| 2404 | `Process.resolve` | 145–148 | sweep-probe1 | sweep-probe1-38 | owns | live = module_base + rva |
| 2405 | `Process.read` | 152–159 | sweep-probe1 | none | none |  |
| 2406 | `Process.write` | 161–163 | sweep-probe1 | none | none |  |
| 2407 | `Process.read_double` | 167–169 | sweep-probe1 | sweep-probe1-16 | wrapper | decode_double |
| 2408 | `Process.read_u32` | 171–173 | sweep-probe1 | sweep-probe1-16 | wrapper | decode_u32 |
| 2409 | `Process.read_u64` | 175–177 | sweep-probe1 | sweep-probe1-16 | wrapper | decode_u64 |
| 2410 | `Process.read_const_double` | 179–182 | sweep-probe1 | sweep-probe1-38 | wrapper | resolve |
| 2411 | `Process.verify_targets` | 186–196 | sweep-probe1 | sweep-probe1-15 | wrapper | check_normal_constants |
| 2412 | `Process.close` | 200–204 | sweep-probe1 | none | none |  |
| 2413 | `Process.__enter__` | 206–207 | sweep-probe1 | none | none |  |
| 2414 | `Process.__exit__` | 209–210 | sweep-probe1 | none | none |  |
| 2415 | `_kernel32` | 224–232 | sweep-probe1 | none | none |  |
| 2416 | `_wintypes` | 235–238 | sweep-probe1 | none | none |  |
| 2417 | `_find_pid_by_name` | 252–293 | sweep-probe1 | none | none |  |
| 2418 | `_find_module_base` | 296–338 | sweep-probe1 | none | none |  |
| 2419 | `_make_reader` | 341–365 | sweep-probe1 | sweep-probe1-25 | owns |  |
| 2420 | `_make_reader.read` | 348–363 | sweep-probe1 | sweep-probe1-25 | owns |  |
| 2421 | `_make_writer` | 368–390 | sweep-probe1 | sweep-probe1-25 | owns |  |
| 2422 | `_make_writer.write` | 374–388 | sweep-probe1 | sweep-probe1-25 | owns |  |
| 2423 | `open_process` | 393–431 | sweep-probe1 | none | none |  |

### `tools/ch_probe/tests/test_analysis.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2424 | `TestFindWindowEdge.test_clean_crossover_lands_on_the_midpoint` | 26–38 | sweep-probe1 | sweep-probe1-18 | test | calls analysis.find_window_edge |
| 2425 | `TestFindWindowEdge.test_works_on_signed_deltas_via_magnitude` | 40–49 | sweep-probe1 | sweep-probe1-18 | test | calls analysis.find_window_edge |
| 2426 | `TestFindWindowEdge.test_one_noisy_outlier_stays_within_tolerance` | 51–64 | sweep-probe1 | sweep-probe1-18 | test | calls analysis.find_window_edge |
| 2427 | `TestFindWindowEdge.test_empty_input_returns_no_edge` | 66–70 | sweep-probe1 | sweep-probe1-18 | test | calls analysis.find_window_edge |
| 2428 | `TestFindWindowEdge.test_all_hits_edge_sits_above_the_data` | 72–76 | sweep-probe1 | sweep-probe1-18 | test | calls analysis.find_window_edge |
| 2429 | `TestClampVerdict.test_stored_tracks_raw_means_no_clamp` | 80–91 | sweep-probe1 | sweep-probe1-12 | test | calls analysis.clamp_verdict |
| 2430 | `TestClampVerdict.test_stored_flat_at_cap_means_clamp` | 93–104 | sweep-probe1 | sweep-probe1-12 | test | calls analysis.clamp_verdict |
| 2431 | `TestClampVerdict.test_never_crossing_the_cap_is_inconclusive` | 106–115 | sweep-probe1 | sweep-probe1-12 | test | calls analysis.clamp_verdict |
| 2432 | `TestClampVerdict.test_split_evidence_is_inconclusive` | 117–126 | sweep-probe1 | sweep-probe1-12 | test | calls analysis.clamp_verdict |
| 2433 | `TestParabolaPredictor.test_simple_hand_computed_point` | 130–136 | sweep-probe1 | sweep-probe1-17 | test | calls predicted_window_normal |
| 2434 | `TestParabolaPredictor.test_quadratic_and_offset_hand_computed` | 138–144 | sweep-probe1 | sweep-probe1-17 | test | calls predicted_window_normal |
| 2435 | `TestParabolaPredictor.test_prescale_by_divisor_is_applied` | 146–152 | sweep-probe1 | sweep-probe1-17 | test | calls predicted_window_normal |
| 2436 | `TestParabolaPredictor.test_precision_shape_hand_computed` | 154–160 | sweep-probe1 | sweep-probe1-17 | test | calls predicted_window_precision |
| 2437 | `TestSummarizeActive.test_groups_and_pairs_measured_with_predicted` | 164–183 | sweep-probe1 | sweep-probe1-18;sweep-probe1-17 | test | calls summarize_active |
| 2438 | `TestSummarizeActive.test_without_constants_predicted_is_none` | 185–189 | sweep-probe1 | sweep-probe1-18 | test | calls summarize_active |
| 2439 | `_ConstProcess.read_const_double` | 201–202 | sweep-probe1 | none | test | fake ProcessHandle |
| 2440 | `TestNormalFormulaConstants.test_maps_the_real_engine_shape` | 206–213 | sweep-probe1 | sweep-probe1-36 | test | calls EngineModel.constants + normal_formula_constants |
| 2441 | `TestNormalFormulaConstants.test_missing_exponent_gives_no_prediction` | 215–218 | sweep-probe1 | sweep-probe1-36 | test | calls normal_formula_constants |
| 2442 | `TestProbeSettingsHaveOneHome.test_clamp_verdict_needs_an_explicit_edge` | 222–224 | sweep-probe1 | sweep-probe1-12 | test | calls clamp_verdict |
| 2443 | `TestProbeSettingsHaveOneHome.test_probe_chart_spacings_and_note_come_from_constants` | 226–229 | sweep-probe1 | sweep-probe1-19 | test | pins probe_chart defaults to constants |

### `tools/ch_probe/tests/test_debugger.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2444 | `FakeMemory.__init__` | 38–39 | sweep-probe2 | none | none | test fake memory |
| 2445 | `FakeMemory.read` | 41–42 | sweep-probe2 | none | none | test fake memory |
| 2446 | `FakeMemory.write` | 44–46 | sweep-probe2 | none | none | test fake memory |
| 2447 | `BreakpointTableTests.test_arm_writes_int3_and_saves_original` | 50–58 | sweep-probe2 | sweep-probe2-6 | test |  |
| 2448 | `BreakpointTableTests.test_disarm_restores_original_byte` | 60–68 | sweep-probe2 | sweep-probe2-6 | test |  |
| 2449 | `BreakpointTableTests.test_rearm_after_disarm_saves_no_stale_byte` | 70–81 | sweep-probe2 | sweep-probe2-6 | test |  |
| 2450 | `BreakpointTableTests.test_arm_is_idempotent_and_preserves_original` | 83–91 | sweep-probe2 | sweep-probe2-6 | test |  |
| 2451 | `BreakpointTableTests.test_remove_restores_and_forgets` | 93–101 | sweep-probe2 | sweep-probe2-6 | test |  |
| 2452 | `BreakpointTableTests.test_armed_originals_lists_only_armed` | 103–110 | sweep-probe2 | sweep-probe2-6 | test |  |
| 2453 | `BreakpointTableTests.test_disarm_of_unarmed_is_a_safe_no_op` | 112–120 | sweep-probe2 | sweep-probe2-6 | test |  |
| 2454 | `MaskBreakpointsTests.test_masks_installed_cc_back_to_original` | 124–130 | sweep-probe2 | sweep-probe2-6 | test |  |
| 2455 | `MaskBreakpointsTests.test_ignores_breakpoints_outside_the_range` | 132–137 | sweep-probe2 | sweep-probe2-6 | test |  |
| 2456 | `MaskBreakpointsTests.test_no_breakpoints_returns_input` | 139–141 | sweep-probe2 | sweep-probe2-6 | test |  |
| 2457 | `Xmm0DecodeTests.test_decodes_known_double_from_low_eight_bytes` | 145–150 | sweep-probe2 | sweep-probe2-4 | test | decode_xmm0_double |
| 2458 | `Xmm0DecodeTests.test_decodes_the_normal_back_window` | 152–154 | sweep-probe2 | sweep-probe2-4 | test | decode_xmm0_double |
| 2459 | `Xmm0DecodeTests.test_short_buffer_raises` | 156–158 | sweep-probe2 | sweep-probe2-4 | test | decode_xmm0_double |
| 2460 | `AdjustRipTests.test_rip_minus_one_points_at_the_breakpoint` | 162–164 | sweep-probe2 | sweep-probe2-5 | test |  |
| 2461 | `ThreadContextTests.test_registers_are_plain_int_attributes` | 168–172 | sweep-probe2 | none | test | ThreadContext attribute default |
| 2462 | `ThreadContextTests.test_xmm0_double_reads_the_formula_return` | 174–177 | sweep-probe2 | sweep-probe2-4 | test |  |
| 2463 | `DebuggerSurfaceTests.test_debugger_constructs_without_attaching` | 185–190 | sweep-probe2 | none | test | surface/export check |
| 2464 | `DebuggerSurfaceTests.test_module_exports_the_expected_names` | 192–196 | sweep-probe2 | none | test | surface/export check |
| 2465 | `_exception_event` | 199–207 | sweep-probe2 | none | none | event builder |
| 2466 | `KillOnExitTests.setUp` | 214–241 | sweep-probe2 | none | none | fake kernel32 plumbing |
| 2467 | `KillOnExitTests.setUp.FakeK32.DebugActiveProcess` | 220–222 | sweep-probe2 | none | none | fake kernel32 plumbing |
| 2468 | `KillOnExitTests.setUp.FakeK32.DebugSetProcessKillOnExit` | 224–226 | sweep-probe2 | none | none | fake kernel32 plumbing |
| 2469 | `KillOnExitTests.setUp.FakeK32.DebugActiveProcessStop` | 228–230 | sweep-probe2 | none | none | fake kernel32 plumbing |
| 2470 | `KillOnExitTests.setUp.FakeK32.OpenProcess` | 232–234 | sweep-probe2 | none | none | fake kernel32 plumbing |
| 2471 | `KillOnExitTests.setUp.FakeWin32.__init__` | 237–238 | sweep-probe2 | none | none | fake kernel32 plumbing |
| 2472 | `KillOnExitTests.tearDown` | 243–244 | sweep-probe2 | none | none | fake kernel32 plumbing |
| 2473 | `KillOnExitTests.test_attach_turns_kill_on_exit_off_right_after_attaching` | 246–251 | sweep-probe2 | sweep-probe2-28 | test |  |
| 2474 | `KillOnExitTests.test_attach_detaches_if_kill_on_exit_cannot_be_turned_off` | 253–258 | sweep-probe2 | sweep-probe2-28 | test |  |
| 2475 | `SafeDetachTests.test_single_step_after_stop_does_not_rearm` | 265–271 | sweep-probe2 | sweep-probe2-28 | test |  |
| 2476 | `SafeDetachTests.test_queued_hit_on_a_removed_breakpoint_rewinds_and_continues` | 273–281 | sweep-probe2 | sweep-probe2-28 | test |  |
| 2477 | `SafeDetachTests.test_table_lists_every_registered_address` | 283–289 | sweep-probe2 | sweep-probe2-6 | test |  |

### `tools/ch_probe/tests/test_engine.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2478 | `FakeThreadContext.__init__` | 36–38 | sweep-probe2 | none | none | fakes |
| 2479 | `FakeThreadContext.xmm0_double` | 40–41 | sweep-probe2 | none | none | fakes |
| 2480 | `FakeProcess.__init__` | 54–60 | sweep-probe2 | none | none | fakes |
| 2481 | `FakeProcess.resolve` | 62–63 | sweep-probe2 | sweep-probe2-30 | copies | fake resolve re-implements process.Process.resolve (module_base + rva) |
| 2482 | `FakeProcess.read` | 65–66 | sweep-probe2 | none | none | fakes |
| 2483 | `FakeProcess.write` | 68–69 | sweep-probe2 | none | none | fakes |
| 2484 | `FakeProcess.read_double` | 71–72 | sweep-probe2 | none | none | fakes |
| 2485 | `FakeProcess.read_u32` | 74–75 | sweep-probe2 | none | none | fakes |
| 2486 | `FakeProcess.read_u64` | 77–78 | sweep-probe2 | none | none | fakes |
| 2487 | `FakeProcess.read_const_double` | 80–82 | sweep-probe2 | none | none | fakes |
| 2488 | `FakeProcess.verify_targets` | 84–85 | sweep-probe2 | none | none | fakes |
| 2489 | `FakeDebugger.__init__` | 96–99 | sweep-probe2 | none | none | fakes |
| 2490 | `FakeDebugger.attach` | 101–102 | sweep-probe2 | none | none | fakes |
| 2491 | `FakeDebugger.set_breakpoint` | 104–106 | sweep-probe2 | none | none | fakes |
| 2492 | `FakeDebugger.clear_breakpoint` | 108–109 | sweep-probe2 | none | none | fakes |
| 2493 | `FakeDebugger.read` | 111–112 | sweep-probe2 | none | none | fakes |
| 2494 | `FakeDebugger.write` | 114–115 | sweep-probe2 | none | none | fakes |
| 2495 | `FakeDebugger.run` | 117–120 | sweep-probe2 | none | none | fakes |
| 2496 | `FakeDebugger.stop` | 122–123 | sweep-probe2 | none | none | fakes |
| 2497 | `make_engine` | 131–136 | sweep-probe2 | none | none | fixture |
| 2498 | `TestCaptureObject.test_captures_rcx_as_object_ptr` | 143–148 | sweep-probe2 | sweep-probe2-31 | test |  |
| 2499 | `TestCaptureObject.test_breakpoint_set_at_resolved_ctor_address` | 150–154 | sweep-probe2 | sweep-probe2-31;sweep-probe2-30 | copies | expected addr = module_base + RVA recomputed; owner process.Process.resolve |
| 2500 | `TestCaptureObject.test_run_not_needed_when_callback_fires_immediately` | 156–160 | sweep-probe2 | sweep-probe2-31 | test |  |
| 2501 | `TestFieldReads._engine_with_fields` | 167–181 | sweep-probe2 | sweep-probe2-1 | test | fixture from constants OFF_* |
| 2502 | `TestFieldReads.test_total_window_reads_off_0x20` | 183–185 | sweep-probe2 | sweep-probe2-1 | test |  |
| 2503 | `TestFieldReads.test_back_window_reads_off_0x30` | 187–189 | sweep-probe2 | sweep-probe2-1 | test |  |
| 2504 | `TestFieldReads.test_front_window_reads_off_0x38` | 191–193 | sweep-probe2 | sweep-probe2-1 | test |  |
| 2505 | `TestFieldReads.test_hit_time_reads_off_0x2e0` | 195–200 | sweep-probe2 | sweep-probe2-1 | test | pins literal 0x2E0 alongside C.OFF_HIT_TIME |
| 2506 | `TestFieldReads.test_total_is_twice_back_at_construction` | 202–206 | sweep-probe2 | sweep-probe2-1;sweep-probe2-18 | test | asserts total == 2*back (constructor relation) |
| 2507 | `TestFieldReads.test_reads_before_capture_raise` | 208–211 | sweep-probe2 | sweep-probe2-1 | test |  |
| 2508 | `TestNoteCount.test_note_count_reads_u32_off_0x8c` | 215–219 | sweep-probe2 | sweep-probe2-1 | test |  |
| 2509 | `TestPrecisionMode._mode` | 226–230 | sweep-probe2 | sweep-probe2-2 | test |  |
| 2510 | `TestPrecisionMode.test_bit_clear_is_normal_mode` | 232–233 | sweep-probe2 | sweep-probe2-2 | test |  |
| 2511 | `TestPrecisionMode.test_bit_set_is_precision_mode` | 235–236 | sweep-probe2 | sweep-probe2-2 | test |  |
| 2512 | `TestPrecisionMode.test_only_the_precision_bit_matters` | 238–241 | sweep-probe2 | sweep-probe2-2 | test |  |
| 2513 | `TestPrecisionMode.test_precision_bit_among_other_bits` | 243–245 | sweep-probe2 | sweep-probe2-2 | test |  |
| 2514 | `TestPrecisionMode.test_precision_bit_is_0x1000` | 247–249 | sweep-probe2 | sweep-probe2-2 | test | pins literal 0x1000 |
| 2515 | `TestConstants._all_expected_rvas` | 256–268 | sweep-probe2 | sweep-probe2-32 | test |  |
| 2516 | `TestConstants.test_reads_every_expected_rva` | 270–275 | sweep-probe2 | sweep-probe2-32 | test |  |
| 2517 | `TestConstants.test_returns_a_value_for_every_name` | 277–306 | sweep-probe2 | sweep-probe2-32 | test | spells "normal_"/"precision_" prefixes literally, not C.CONST_KEY_PREFIX_* |
| 2518 | `TestConstants.test_constants_need_no_captured_object` | 308–312 | sweep-probe2 | sweep-probe2-32 | test |  |
| 2519 | `TestSongClock.test_reads_the_proven_clock_at_0x100` | 319–326 | sweep-probe2 | sweep-probe2-1 | test | pins literal 0x100 |
| 2520 | `TestSongClock.test_no_placeholder_offset_is_left` | 328–329 | sweep-probe2 | sweep-probe2-1 | test |  |
| 2521 | `TestScoreAndFoundObject.test_score_reads_u32_off_0x94` | 333–337 | sweep-probe2 | sweep-probe2-1;score-25 | test |  |
| 2522 | `TestScoreAndFoundObject.test_use_object_takes_a_scanned_pointer_without_a_debugger` | 339–344 | sweep-probe2 | sweep-probe2-31 | test |  |
| 2523 | `TestScoreAndFoundObject.test_capture_without_a_debugger_raises` | 346–349 | sweep-probe2 | sweep-probe2-31 | test |  |

### `tools/ch_probe/tests/test_engine_finder.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2524 | `FakeProc.__init__` | 32–36 | sweep-probe2 | none | none | fake |
| 2525 | `FakeProc.resolve` | 38–39 | sweep-probe2 | sweep-probe2-30 | copies | fake resolve = BASE + rva; owner process.Process.resolve |
| 2526 | `FakeProc.read` | 41–42 | sweep-probe2 | none | none | fake |
| 2527 | `FakeProc.read_double` | 44–50 | sweep-probe2 | none | none | fake |
| 2528 | `HitsInRegionTest.test_each_match_backs_up_to_the_object_start` | 54–58 | sweep-probe2 | sweep-probe2-7 | copies | expected object start uses literal 0x30; owner C.OFF_BACK_WINDOW via engine_finder.hits_in_region |
| 2529 | `FindLiveEngineTest.test_picks_the_engine_whose_clock_moves` | 62–77 | sweep-probe2 | sweep-probe2-7 | test | pins sleeps 0.12/0.4 of find_live_engine |
| 2530 | `FindLiveEngineTest.test_picks_the_engine_whose_clock_moves.scan` | 71–72 | sweep-probe2 | none | none | fake scan |
| 2531 | `FindLiveEngineTest.test_keeps_waiting_until_a_clock_moves` | 79–89 | sweep-probe2 | sweep-probe2-7 | test | pins sleeps 0.12/0.4 of find_live_engine |
| 2532 | `FindLiveEngineTest.test_a_candidate_two_patterns_find_is_checked_once_in_scan_order` | 91–99 | sweep-probe2 | sweep-probe2-7 | test | pins sleeps 0.12/0.4 of find_live_engine |
| 2533 | `PatternsTest.test_all_patterns_tries_the_precision_pair_both_ways` | 103–113 | sweep-probe2 | sweep-probe2-8 | test |  |

### `tools/ch_probe/tests/test_hit_window_scripts.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2534 | `window_map_notes` | 26–27 | sweep-probe2 | sweep-probe2-16 | wrapper | calls probe_songs.manifest/window_map |
| 2535 | `samples_following` | 30–35 | sweep-probe2 | none | none | fixture builder |
| 2536 | `LiveSnapshotTest.test_decode_reads_each_field_at_its_offset` | 39–51 | sweep-probe2 | sweep-probe2-1;sweep-probe2-2;sweep-probe2-29 | test | packs flags literal 0x1000 not C.PRECISION_MODE_BIT; 0.17143 s -> 171.43 ms |
| 2537 | `LiveSnapshotTest.test_live_keeps_no_offsets_or_finder_of_its_own` | 53–56 | sweep-probe2 | sweep-probe2-1 | test | guard: live has no offsets of its own |
| 2538 | `WatchWindowTest.test_value_at_takes_the_last_change_at_or_before` | 60–64 | sweep-probe2 | sweep-probe2-19 | test |  |
| 2539 | `WatchWindowTest.test_block_spans_split_in_the_silence_and_cover_every_note` | 66–72 | sweep-probe2 | sweep-probe2-20 | test |  |
| 2540 | `WatchWindowTest.test_run_values_come_from_the_run_not_the_markers` | 74–84 | sweep-probe2 | sweep-probe2-35;sweep-probe2-18 | test | expected text embeds 171.43 |
| 2541 | `WatchWindowTest.test_cap_verdict_passes_when_every_wide_run_reads_the_cap` | 86–92 | sweep-probe2 | sweep-probe2-18;sweep-probe2-35 | copies | fixture uses literals 171.43 and 170 instead of watch_window.CAP_EXPECT_MS / CAP_CHECK_FROM_GAP_MS |
| 2542 | `WatchWindowTest.test_frozen_window_says_so` | 94–97 | sweep-probe2 | sweep-probe2-35 | test |  |
| 2543 | `WatchWindowTest.test_hit_time_report_matches_changes_to_notes` | 99–107 | sweep-probe2 | sweep-probe2-34 | test |  |
| 2544 | `WalkEdgesTest.test_default_plan_fits_edge_walk` | 111–119 | sweep-probe2 | sweep-probe2-21 | test |  |
| 2545 | `WalkEdgesTest.test_plan_that_does_not_fit_is_refused` | 121–123 | sweep-probe2 | sweep-probe2-21 | test |  |
| 2546 | `WalkEdgesTest.test_parse_range` | 125–129 | sweep-probe2 | sweep-probe2-21 | test |  |
| 2547 | `WalkEdgesTest.test_song_clock_fills_in_after_a_fresh_change` | 131–142 | sweep-probe2 | sweep-probe2-22 | test | pins fill-in cap 0.05 s |
| 2548 | `WalkEdgesTest.test_song_clock_ignores_a_change_seen_late` | 144–152 | sweep-probe2 | sweep-probe2-22 | test | pins fill-in cap 0.05 s |
| 2549 | `WalkEdgesTest.test_summary_brackets_the_edge` | 154–162 | sweep-probe2 | sweep-probe2-23 | test |  |
| 2550 | `WalkEdgesTest.test_summary_brackets_the_edge.row` | 155–156 | sweep-probe2 | none | none | row builder |
| 2551 | `WalkEdgesTest.test_summary_flags_overlap_and_all_miss` | 164–170 | sweep-probe2 | sweep-probe2-23 | test |  |
| 2552 | `WalkEdgesTest.test_summary_flags_overlap_and_all_miss.row` | 165–166 | sweep-probe2 | none | none | row builder |

### `tools/ch_probe/tests/test_input_driver.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2553 | `_FakeClock.__init__` | 42–44 | sweep-probe2 | none | none | fake clock / driver factory |
| 2554 | `_FakeClock.__call__` | 46–49 | sweep-probe2 | none | none | fake clock / driver factory |
| 2555 | `_make_driver` | 52–63 | sweep-probe2 | none | none | fake clock / driver factory |
| 2556 | `TestBindings.test_defaults_are_present` | 67–71 | sweep-probe2 | sweep-probe2-9 | test |  |
| 2557 | `TestBindings.test_set_binding_round_trips` | 73–76 | sweep-probe2 | sweep-probe2-9 | test |  |
| 2558 | `TestBindings.test_set_binding_adds_new_lane` | 78–81 | sweep-probe2 | sweep-probe2-9 | test |  |
| 2559 | `TestBindings.test_missing_lane_raises` | 83–86 | sweep-probe2 | sweep-probe2-9 | test |  |
| 2560 | `TestBindings.test_constructor_copies_defaults` | 88–93 | sweep-probe2 | sweep-probe2-9 | test |  |
| 2561 | `TestTap.test_tap_sends_down_then_up` | 97–101 | sweep-probe2 | sweep-probe2-11 | test |  |
| 2562 | `TestScheduleHit.test_fires_once_clock_reaches_target` | 105–111 | sweep-probe2 | sweep-probe2-10 | test |  |
| 2563 | `TestScheduleHit.test_does_not_fire_before_target` | 113–131 | sweep-probe2 | sweep-probe2-10 | test |  |
| 2564 | `TestScheduleHit.test_does_not_fire_before_target.spy` | 122–124 | sweep-probe2 | none | none | spy |
| 2565 | `TestScheduleHit.test_fires_immediately_if_already_past` | 133–140 | sweep-probe2 | sweep-probe2-10 | test |  |
| 2566 | `TestScheduleHit.test_timeout_when_clock_stalls` | 142–149 | sweep-probe2 | sweep-probe2-10 | test |  |
| 2567 | `TestKeyTable.test_lanes_follow_the_bind_screen` | 155–160 | sweep-probe2 | sweep-probe2-9 | test | pins key letters A S J K L U Y T |
| 2568 | `TestKeyTable.test_lane_numbers_are_unchanged` | 162–164 | sweep-probe2 | sweep-probe2-9 | test |  |
| 2569 | `TestKeyTable.test_every_lane_has_a_short_name` | 166–168 | sweep-probe2 | sweep-probe2-9 | test |  |
| 2570 | `TestPressChord.test_all_down_then_all_up` | 174–181 | sweep-probe2 | sweep-probe2-11 | test | pins hold 0.003 s |
| 2571 | `TestPressChord.test_unbound_lane_is_skipped` | 183–188 | sweep-probe2 | sweep-probe2-11 | test |  |

### `tools/ch_probe/tests/test_ocr.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2572 | `ParseAccuracyTextTests.test_plain_decimal` | 28–29 | sweep-probe2 | sweep-probe2-12 | test |  |
| 2573 | `ParseAccuracyTextTests.test_negative_integer` | 31–32 | sweep-probe2 | sweep-probe2-12 | test |  |
| 2574 | `ParseAccuracyTextTests.test_leading_plus_is_dropped` | 34–35 | sweep-probe2 | sweep-probe2-12 | test |  |
| 2575 | `ParseAccuracyTextTests.test_junk_is_none` | 37–38 | sweep-probe2 | sweep-probe2-12 | test |  |
| 2576 | `ParseAccuracyTextTests.test_empty_is_none` | 40–41 | sweep-probe2 | sweep-probe2-12 | test |  |
| 2577 | `ParseAccuracyTextTests.test_number_without_unit_is_none` | 43–45 | sweep-probe2 | sweep-probe2-12 | test |  |
| 2578 | `ParseAccuracyTextTests.test_zero` | 47–48 | sweep-probe2 | sweep-probe2-12 | test |  |
| 2579 | `ParseAccuracyTextTests.test_unit_spacing_and_case` | 50–53 | sweep-probe2 | sweep-probe2-12 | test |  |
| 2580 | `ParseAccuracyTextTests.test_extra_surrounding_text` | 55–57 | sweep-probe2 | sweep-probe2-12 | test |  |
| 2581 | `ParseAccuracyTextTests.test_none_input_is_none` | 59–61 | sweep-probe2 | sweep-probe2-12 | test |  |

### `tools/ch_probe/tests/test_play_chart.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2582 | `_parse` | 51–56 | parse | none | test | test helper |
| 2583 | `ChartPathTest.test_each_tick_gets_one_lane_per_gem` | 60–71 | parse | parse-11 | test | tests play_chart's own copy |
| 2584 | `ChartPathTest.test_times_follow_the_tempo` | 73–76 | sweep-probe2 | tempo-1;tempo-10 | test | tests play_chart.parse_chart, itself a copy (owner MsIndex::at / ChartParser) |
| 2585 | `ChartPathTest.test_lane_helper` | 78–82 | parse | parse-11 | test | tests play_chart's own copy |
| 2586 | `SharedPiecesTest.test_no_private_copies_are_left` | 88–91 | sweep-probe2 | sweep-probe2-1;sweep-probe2-9 | test | guard: no private offsets/lane table in play_chart |
| 2587 | `SharedPiecesTest.test_lane_names_come_from_the_key_table` | 93–95 | sweep-probe2 | sweep-probe2-9 | test |  |

### `tools/ch_probe/tests/test_probe_chart.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2588 | `parse_drum_ticks` | 35–56 | sweep-probe2 | parse-10 | copies | third .chart [ExpertDrums] note-line reader (regex); owner ChartParser::parse in src/parse |
| 2589 | `TestMsToTicks.test_known_conversion_default` | 60–63 | sweep-probe2 | tempo-2 | test | tests probe_chart.ms_to_ticks (itself a copy of MsIndex::tick_at_ms); comments recompute res*bpm/60 |
| 2590 | `TestMsToTicks.test_quarter_note_worth_of_ms` | 65–69 | sweep-probe2 | tempo-2 | test | tests probe_chart.ms_to_ticks (itself a copy of MsIndex::tick_at_ms); comments recompute res*bpm/60 |
| 2591 | `TestMsToTicks.test_one_second_at_various_bpm` | 71–74 | sweep-probe2 | tempo-2 | test | tests probe_chart.ms_to_ticks (itself a copy of MsIndex::tick_at_ms); comments recompute res*bpm/60 |
| 2592 | `TestMsToTicks.test_rounds_to_nearest` | 76–78 | sweep-probe2 | tempo-2 | test | tests probe_chart.ms_to_ticks (itself a copy of MsIndex::tick_at_ms); comments recompute res*bpm/60 |
| 2593 | `TestSectionsPresentAndOrdered.test_required_sections_in_order` | 82–91 | sweep-probe2 | sweep-probe2-13 | test |  |
| 2594 | `TestSectionsPresentAndOrdered.test_resolution_and_tempo_written` | 93–97 | sweep-probe2 | sweep-probe2-13 | test | tempo written as BPM*1000 |
| 2595 | `TestSectionsPresentAndOrdered.test_braces_balanced` | 99–101 | sweep-probe2 | sweep-probe2-13 | test |  |
| 2596 | `TestPairsAndSpacing.test_two_notes_per_spacing` | 105–108 | sweep-probe2 | sweep-probe2-15 | test |  |
| 2597 | `TestPairsAndSpacing.test_each_pair_is_the_right_delta_apart` | 110–123 | sweep-probe2 | sweep-probe2-15 | test |  |
| 2598 | `TestPairsAndSpacing.test_pairs_do_not_overlap` | 125–145 | sweep-probe2 | sweep-probe2-15 | test |  |
| 2599 | `TestPairsAndSpacing.test_ticks_are_strictly_increasing` | 147–150 | sweep-probe2 | sweep-probe2-15 | test |  |
| 2600 | `TestPairsAndSpacing.test_probe_note_ticks_are_the_written_ticks` | 152–158 | sweep-probe2 | sweep-probe2-15 | copies | expected ticks 3840/11731 hard-code 2-bar lead-in and 4-bar pad; owner probe_chart.probe_note_ticks (_LEAD_IN_WHOLE_NOTES/_PAD_WHOLE_NOTES) |
| 2601 | `TestFileWrite.test_generate_writes_a_readable_chart` | 162–173 | sweep-probe2 | sweep-probe2-13;sweep-probe2-15 | test |  |

### `tools/ch_probe/tests/test_probe_songs.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2602 | `chart_ticks` | 18–19 | sweep-probe2 | parse-10 | copies | fourth .chart note-line reader (kick-only regex); owner ChartParser::parse |
| 2603 | `ProbeSongsTest.test_one_tick_is_one_ms` | 23–24 | sweep-probe2 | tempo-2;tempo-1 | copies | recomputes RESOLUTION*BPM/60000 inline; owner probe_chart.ms_to_ticks |
| 2604 | `ProbeSongsTest.test_chart_ticks_match_manifest_times` | 26–31 | sweep-probe2 | sweep-probe2-13;sweep-probe2-16 | test |  |
| 2605 | `ProbeSongsTest.test_run_notes_have_the_same_gap_both_sides` | 33–41 | sweep-probe2 | sweep-probe2-16 | test |  |
| 2606 | `ProbeSongsTest.test_uneven_middle_notes` | 43–50 | sweep-probe2 | sweep-probe2-16 | test |  |
| 2607 | `ProbeSongsTest.test_blocks_are_separated_by_silence` | 52–56 | sweep-probe2 | sweep-probe2-16 | test |  |
| 2608 | `ProbeSongsTest.test_edge_walk_is_isolated` | 58–61 | sweep-probe2 | sweep-probe2-16 | test |  |
| 2609 | `ProbeSongsTest.test_song_ini_has_no_delay` | 63–64 | sweep-probe2 | tempo-11;parse-27 | test | probe song.ini writes delay = 0 |

### `tools/ch_probe/tests/test_process.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2610 | `_make_fake_process` | 29–45 | sweep-probe2 | none | none | fake process builder |
| 2611 | `_make_fake_process.reader` | 35–39 | sweep-probe2 | none | none | fake process builder |
| 2612 | `_make_fake_process.writer` | 41–42 | sweep-probe2 | none | none | fake process builder |
| 2613 | `DecodeHelpersTest.test_decode_double_known_pattern` | 51–54 | sweep-probe2 | sweep-probe2-4 | test |  |
| 2614 | `DecodeHelpersTest.test_decode_double_endianness` | 56–62 | sweep-probe2 | sweep-probe2-4 | test |  |
| 2615 | `DecodeHelpersTest.test_decode_double_wrong_length` | 64–66 | sweep-probe2 | sweep-probe2-4 | test |  |
| 2616 | `DecodeHelpersTest.test_decode_u32_known_pattern` | 68–70 | sweep-probe2 | sweep-probe2-4 | test |  |
| 2617 | `DecodeHelpersTest.test_decode_u32_wrong_length` | 72–74 | sweep-probe2 | sweep-probe2-4 | test |  |
| 2618 | `DecodeHelpersTest.test_decode_u64_known_pattern` | 76–78 | sweep-probe2 | sweep-probe2-4 | test |  |
| 2619 | `DecodeHelpersTest.test_decode_u64_wrong_length` | 80–82 | sweep-probe2 | sweep-probe2-4 | test |  |
| 2620 | `ResolveMathTest.test_resolve_adds_base` | 88–90 | sweep-probe2 | sweep-probe2-30 | test |  |
| 2621 | `ResolveMathTest.test_resolve_zero_rva` | 92–94 | sweep-probe2 | sweep-probe2-30 | test |  |
| 2622 | `TypedReadTest.test_read_double_through_fake_memory` | 100–104 | sweep-probe2 | sweep-probe2-4 | test |  |
| 2623 | `TypedReadTest.test_read_u32_through_fake_memory` | 106–110 | sweep-probe2 | sweep-probe2-4 | test |  |
| 2624 | `TypedReadTest.test_read_const_double_resolves_then_reads` | 112–117 | sweep-probe2 | sweep-probe2-4 | test |  |
| 2625 | `TypedReadTest.test_read_rejects_short_read` | 119–126 | sweep-probe2 | sweep-probe2-4 | test |  |
| 2626 | `TypedReadTest.test_write_goes_through_writer` | 128–131 | sweep-probe2 | sweep-probe2-4 | test |  |
| 2627 | `CheckNormalConstantsTest.test_exact_values_pass` | 139–141 | sweep-probe2 | sweep-probe2-3;tempo-24 | copies | literals 0.085/0.0375 instead of constants.EXPECT_NORMAL_BACK_S/FRONT_S |
| 2628 | `CheckNormalConstantsTest.test_within_tolerance_passes` | 143–146 | sweep-probe2 | sweep-probe2-3;tempo-24 | copies | literals 0.085/0.0375 instead of constants.EXPECT_NORMAL_BACK_S/FRONT_S |
| 2629 | `CheckNormalConstantsTest.test_back_off_by_one_fails` | 148–151 | sweep-probe2 | sweep-probe2-3;tempo-24 | copies | literals 0.085/0.0375 instead of constants.EXPECT_NORMAL_BACK_S/FRONT_S |
| 2630 | `CheckNormalConstantsTest.test_front_off_fails` | 153–155 | sweep-probe2 | sweep-probe2-3;tempo-24 | copies | literals 0.085/0.0375 instead of constants.EXPECT_NORMAL_BACK_S/FRONT_S |
| 2631 | `CheckNormalConstantsTest.test_just_past_tolerance_fails` | 157–160 | sweep-probe2 | sweep-probe2-3;tempo-24 | copies | literals 0.085/0.0375 instead of constants.EXPECT_NORMAL_BACK_S/FRONT_S |
| 2632 | `CheckNormalConstantsTest.test_values_in_ms_are_refused` | 162–165 | sweep-probe2 | sweep-probe2-3 | test |  |
| 2633 | `VerifyTargetsTest._proc_with_constants` | 171–178 | sweep-probe2 | none | none | fixture |
| 2634 | `VerifyTargetsTest.test_good_build_passes` | 180–182 | sweep-probe2 | sweep-probe2-3 | copies | literals 0.085/0.0375/0.084/0.025 instead of constants.EXPECT_NORMAL_*_S |
| 2635 | `VerifyTargetsTest.test_drifted_back_constant_raises` | 184–186 | sweep-probe2 | sweep-probe2-3 | copies | literals 0.085/0.0375/0.084/0.025 instead of constants.EXPECT_NORMAL_*_S |
| 2636 | `VerifyTargetsTest.test_drifted_front_constant_raises` | 188–190 | sweep-probe2 | sweep-probe2-3 | copies | literals 0.085/0.0375/0.084/0.025 instead of constants.EXPECT_NORMAL_*_S |
| 2637 | `ProtocolShapeTest.test_process_is_a_process_handle` | 197–201 | sweep-probe2 | none | test | protocol shape |

### `tools/ch_probe/tests/test_runners.py`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2638 | `FakeProcess.__init__` | 36–37 | sweep-probe2 | none | none | fake |
| 2639 | `FakeProcess.verify_targets` | 39–40 | sweep-probe2 | none | none | fake |
| 2640 | `FakeProcess.resolve` | 42–43 | sweep-probe2 | sweep-probe2-30 | copies | fake resolve = BASE + rva; owner process.Process.resolve |
| 2641 | `FakeProcess.read_double` | 45–46 | sweep-probe2 | none | none | fake |
| 2642 | `FakeProcess.read_u32` | 48–49 | sweep-probe2 | none | none | fake |
| 2643 | `FakeProcess.read_const_double` | 51–52 | sweep-probe2 | none | none | fake |
| 2644 | `FakeDebugger.__init__` | 56–59 | sweep-probe2 | none | none | fake debugger |
| 2645 | `FakeDebugger.attach` | 61–63 | sweep-probe2 | none | none | fake debugger |
| 2646 | `FakeDebugger.set_breakpoint` | 65–68 | sweep-probe2 | none | none | fake debugger |
| 2647 | `FakeDebugger.run` | 70–76 | sweep-probe2 | none | none | fake debugger |
| 2648 | `FakeDebugger.stop` | 78–79 | sweep-probe2 | none | none | fake debugger |
| 2649 | `_names` | 82–83 | sweep-probe2 | none | none | helper |
| 2650 | `PassiveRunnerTest._run` | 87–95 | sweep-probe2 | none | none | helper |
| 2651 | `PassiveRunnerTest.test_attach_comes_before_any_breakpoint` | 97–102 | sweep-probe2 | sweep-probe2-27 | test |  |
| 2652 | `PassiveRunnerTest.test_detaches_when_the_loop_is_interrupted` | 104–108 | sweep-probe2 | sweep-probe2-27 | test |  |
| 2653 | `FakeEngine.__init__` | 112–114 | sweep-probe2 | none | none | fakes |
| 2654 | `FakeEngine.song_clock` | 116–117 | sweep-probe2 | none | none | fakes |
| 2655 | `FakeEngine.total_window` | 119–120 | sweep-probe2 | none | none | fakes |
| 2656 | `FakeStackDebugger.__init__` | 128–129 | sweep-probe2 | none | none | fakes |
| 2657 | `FakeStackDebugger.read` | 131–132 | sweep-probe2 | none | none | fakes |
| 2658 | `FakeStackDebugger.set_breakpoint` | 134–135 | sweep-probe2 | none | none | fakes |
| 2659 | `_ctx` | 138–139 | sweep-probe2 | none | none | fakes |
| 2660 | `PassiveCollectorTest.test_raw_at_return_stored_at_next_call` | 143–157 | sweep-probe2 | sweep-probe2-24;sweep-probe2-29 | test |  |
| 2661 | `PassiveCollectorTest.test_a_result_with_no_next_call_is_not_logged` | 159–164 | sweep-probe2 | sweep-probe2-24 | test |  |
| 2662 | `ActiveRunnerTest._run` | 168–183 | sweep-probe2 | none | none | fixture |
| 2663 | `ActiveRunnerTest._run.drive` | 169–170 | sweep-probe2 | none | none | fixture |
| 2664 | `ActiveRunnerTest.test_attach_comes_before_the_breakpoint_and_the_inputs` | 185–192 | sweep-probe2 | sweep-probe2-27 | test |  |
| 2665 | `ActiveRunnerTest.test_detaches_when_the_loop_is_interrupted` | 194–202 | sweep-probe2 | sweep-probe2-27 | test |  |
| 2666 | `ActivePlanTest.test_one_pair_per_spacing_and_offset` | 206–212 | sweep-probe2 | sweep-probe2-25;sweep-probe2-15 | copies | asserts first_ms 3840 literal (2-bar lead-in at tick==ms); owner probe_chart.probe_note_ticks |
| 2667 | `ActivePlanTest.test_hit_checks_are_counted_only_during_an_input` | 214–220 | sweep-probe2 | sweep-probe2-26 | test |  |

### `tools/replay.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2668 | `flag_bool` | 90–90 | sweep-core1 | sweep-core1-8 | copies | Settings::load_file reads booleans as =="1" only; flag_bool also takes true/yes |
| 2669 | `usage` | 92–136 | screenB | screenB-23;screenB-24 | copies | usage text: '500 ms' literal, 'Every stored row is a 1.1 result' |
| 2669 | `usage` | 92–136 | store | store-28 | copies | stale "Every stored row is a 1.1 result" |
| 2670 | `settings_from` | 140–160 | fills | fills-25 | copies | "points" -> 1 else 0 |
| 2670 | `settings_from` | 140–160 | screenB | screenB-25 | wrapper | starts from app::Settings defaults |
| 2670 | `settings_from` | 140–160 | store | store-9 | copies | depth "points"->1 else 0; own cap/ms parsing |
| 2671 | `emit` | 162–172 | screenB | none | none |  |
| 2671 | `emit` | 162–172 | sweep-core1 | none | none | json out |
| 2672 | `strict_ll` | 181–193 | sweep-core1 | none | none | strict int parse |
| 2673 | `strict_d` | 196–208 | sweep-core1 | none | none | strict double parse |
| 2674 | `parse_acts` | 216–255 | screenB | none | none |  |
| 2674 | `parse_acts` | 216–255 | sweep-core1 | none | none | --acts parse |
| 2675 | `parse_index` | 259–269 | sweep-core1 | none | none | --index parse (n > 1000000 cap) |
| 2676 | `windows_from_file` | 276–305 | sweep-core1 | none | none | wraps windows_from_json |
| 2677 | `cmd_score` | 307–415 | score | score-10;score-24 | wrapper | cmd_score prints replay totals |
| 2677 | `cmd_score` | 307–415 | screenB | screenB-27 | wrapper | resolve_sqout_note, ambiguous_window_warnings |
| 2678 | `snapshot_db` | 425–467 | store | none | none |  |
| 2679 | `emit_dump` | 473–486 | screenB | screenB-18 | wrapper |  |
| 2679 | `emit_dump` | 473–486 | store | store-12 | copies | sp_cap from blob copy |
| 2680 | `cmd_dump` | 488–596 | fills | fills-9 | wrapper | legacy flag into SearchSettings |
| 2680 | `cmd_dump` | 488–596 | screenB | screenB-23 | none |  |
| 2680 | `cmd_dump` | 488–596 | store | store-2;store-28 | copies | stale wording uses hyversion as Hydra version |
| 2681 | `parse_ticks` | 604–620 | sweep-core1 | none | none | --ticks parse |
| 2682 | `cmd_target` | 622–675 | screenB | none | none |  |
| 2682 | `cmd_target` | 622–675 | sweep-core1 | score-19;fills-30 | wrapper | best_path, search_target |
| 2683 | `check_chart` | 689–786 | score | score-10 | test | check_chart compares replay to stored per category |
| 2683 | `check_chart` | 689–786 | screenB | screenB-26 | copies | prints nominal SP end via plusmeasure(act, 2*sp_meter); owner graph add_act_edge |
| 2683 | `check_chart` | 689–786 | spwin | spwin-3 | copies | nominal = plusmeasure(act, 2*sp_meter) printed |
| 2684 | `cmd_selfcheck` | 788–800 | screenB | none | none |  |
| 2684 | `cmd_selfcheck` | 788–800 | sweep-core1 | none | none | tally print |
| 2685 | `main` | 804–866 | screenB | screenB-25 | copies | Args literal defaults parallel to app::Settings |
| 2685 | `main` | 804–866 | sweep-core1 | screenB-25 | copies | Args literal defaults ("4","10","scores",4,true,true,"expert") parallel to app::Settings |

### `tools/replay_json.cpp`

| # | Function | Lines | Read by | Questions | Role | Note |
|---|---|---|---|---|---|---|
| 2686 | `windows_from_json` | 12–63 | display | none | none | JSON parse; last SqOut offset wins |
| 2686 | `windows_from_json` | 12–63 | score | none | none | windows_from_json |
| 2686 | `windows_from_json` | 12–63 | spwin | spwin-10;spwin-16 | copies | deact<0 throws (windows_for_path skips); SqOut without tick kept for resolve (windows_for_path skips) |
| 2686 | `windows_from_json` | 12–63 | squeeze | squeeze-21 | none | reads sqout_tick and SqOut offset |
| 2687 | `score_json` | 65–69 | display | none | none | score json |
| 2687 | `score_json` | 65–69 | score | score-10 | wrapper | score_json |
| 2688 | `paths_json` | 73–109 | display | display-27 | copies | computes nominal_deact_tick itself (plusmeasure of sp_meter bars) |
| 2688 | `paths_json` | 73–109 | score | score-10 | wrapper | paths_json prints totalscore |
| 2688 | `paths_json` | 73–109 | spwin | spwin-3 | copies | nominal_deact_tick = plusmeasure(act, 2*sp_meter): copy of add_act_edge initial end |
| 2688 | `paths_json` | 73–109 | squeeze | squeeze-31 | copies | nominal_deact_tick = act + 2B measures (owner engine) |

## Items that are not functions

Agents also ledgered constants, page scripts and doc paragraphs they checked. The mechanical list has no id for these, so they are listed here as the agents named them.

| Item | Read by | Questions | Role | Note |
|---|---|---|---|---|
| `src/ui/preview_tab.cpp:503:render_preview_panel_overlay` | display | display-34 | copies | IM_COL32(255,204,51) literal gold, five places |
| `docs/UserGuide.md:118:early-fill-paragraph` | display | display-2 | copies | says more negative = earlier; code shows positive = early |
| `docs/UserGuide.md:168:dynamics-paragraph` | display | display-23 | copies | says 2x kicks left out of the totals; All kicks row includes them |
| `tools/replay.cpp:73:Args` | display | display-33 | copies | default ms "10" parallel to config.h mslimit_value = 10 |
| `src/search/pather.cpp:149:analyze_at_cap` | fills | fills-9 | copies | legacy flag -> FillDeadlineRule mapping duplicated with search_target |
| `tests/test_preview_source.cpp:94:make_sng` | parse | parse-24 | copies | second test sng encoder |
| `tools/ch_probe/constants.py:80:OFF_SCORE` | score | score-25 | owns | score offset 0x94 |
| `src/core/timing.h:27:kStarPowerMultiplier` | score | score-4 | copies | parallel constant for SP doubling |
| `src/core/model.h:89:kNoteBasePoints` | score | score-1 | owns | note price constants |
| `src/app/report.cpp:70:kPageJs` | squeeze | squeeze-8 | copies | BEYOND = max(cutoffs) and r.ms >= BEYOND (owner beyond_edge_ms / tier_for) |
| `tests/test_fill_deadline.cpp:113:TEST_CASE fill deadline CH 1.1 4-beats` | tempo | tempo-17;tempo-9 | copies | recomputes tick_e = fill_end - len - 4*res; owner activation_fill_deadline_ms |
| `tests/test_fill_deadline.cpp:81:TEST_CASE fill deadline CH 1.0 clamps` | tempo | tempo-17 | copies | recomputes fend - fill_len_ms - 250/10000; owner activation_fill_deadline_ms |
| `tests/test_search.cpp:227:backend-implied deact check` | tempo | tempo-2;tempo-15 | copies | inline llround(tick_at_ms(ms - offset)) inverse of offset rule |
| `src/app/report.cpp:70:kPageJs BEYOND` | tempo | tempo-25 | copies | max cutoff in JS duplicates beyond_edge_ms |
| `tools/ch_probe/constants.py:146:EXPECT_NORMAL_BACK_S` | tempo | tempo-24 | copies | 0.085 s parallel to kDefaultHitWindowMs |
| `tools/ch_probe/probe_songs.py:36:BPM tick==ms` | tempo | tempo-1 | copies | 480 res at 125 BPM assumed 1 tick = 1 ms |

## Question catalog

Each agent wrote down the questions its functions answer, in the form `id: question | owner | answered also in`. They are reproduced here as written.

### backend

- backend-1: Has the SP walk already paid this row (is it at or before the SP end)? | owner: core::paid_by_sp_walk (src/core/backend_value.h) | answered also in: replay_path (row.tick <= deact_tick clamp + offset<=0 via counted_without_squeeze), rate_activation (offset > 0.0 / < 0.0 branches), Activation::display_backends comment only
- backend-2: Does a row after the SP end still score under SP with no squeeze (inside the backend leeway)? | owner: core::counted_without_squeeze | answered also in: replay_path (separate gate before backend_row_value), build_activations (counted flag for "(-N)"), squeeze_sentences (counted branch), BackendSqueeze::summarystr (wrapper), rate_activation (wrapper, twice)
- backend-3: Where does a row sit against the squeezed-out chord (no sqout / before / exact / after)? | owner: core::sqout_position | answered also in: Activation::is_sqout_backend (tick ==), Activation::display_backends is_beyond_sqout (tick >), rebuild trim lambda (tick >), build_activations (maps BackendRating::squeezed_out to Exact/NoSqOut by hand), test "no activation keeps backends past its squeezed-out note" (tick > / ==)
- backend-4: What SP score does one backend row add on this path? | owner: core::backend_row_value | answered also in: Engine::create_deactivated_path (wrapper + already_paid), replay_path (wrapper), build_activations (wrapper), squeeze_sentences (wrapper)
- backend-5: What does a squeezed-out chord keep, and what does squeezing it out cost? | owner: category_scores sqout_reduction / CategoryScores::sqout_sp | answered also in: squeeze_sentences (lost = row.points - backend_row_value(Exact)), build_activations ("(-N)" = bsq.points - value)
- backend-6: Does Star Power pay this chord (for the in_sp flag and the doubled multiplier disc)? | owner: none (replay_path sp_claims gate) | answered also in: backend_row_value (value > 0), Preview build_score reads multiplier_shown
- backend-7: How far (ms) is a row from the SP end? | owner: ScoreGraph::add_deact_edge / ScoreGraph::store_new_backend (two copies) | answered also in: rebuild (tail rows vs final_sp_end), replay_path (row.ms - deact_ms, with min(...,0) clamp)
- backend-8: Which rows are near enough to the SP end to be a backend row (the 500 ms squeeze horizon)? | owner: kSqueezeWindowMs + ScoreGraph::is_recent_to_head | answered also in: Activation::display_backends (fabs < kSqueezeWindowMs), sqout_candidates (fabs < kSqueezeWindowMs), render_path_footer (clamp literal 500), hydra_replay help text ("within 500 ms")
- backend-9: What rating label does a backend row get? | owner: BackendSqueeze::summarystr | answered also in: kBackendTimingsLead text, UserGuide "3ms to 85ms" sentence
- backend-10: Which frontend transfer scale governs a backend row, and does it change the row's figure? | owner: rate_activation | answered also in: none found
- backend-11: Which backend rows does the details table hide (Backend limit)? | owner: build_activations | answered also in: Settings::backend_limit (abs), render_path_footer (clamp 0..500), Settings::load_file (no clamp)
- backend-12: What is a row with no stored offset treated as? | owner: none | answered also in: Engine::create_deactivated_path (0.0), BackendSqueeze::summarystr (0.0), Activation::display_backends (0.0), build_activations (0.0, three times), squeeze_sentences (0.0), rate_activation (skips the row)
- backend-13: Is a squeezed-out row's cost shown with "(-N)" and warning colour, or as "(uncounted)"? | owner: build_activations | answered also in: squeeze_sentences ("costs no points" branch)
- backend-14: Which phrase chord can the engine squeeze out at a given SP end? | owner: ScoreGraph::add_deact_edge + ScoreGraph::store_new_backend (first is_sp row) | answered also in: sqout_candidates/resolve_sqout_note (front()), ambiguous_window_warnings (front())
- backend-15: Is a row the engine does not count marked "(uncounted)" in the table? | owner: BackendSqueeze::summarystr | answered also in: kBackendTimingsLead (claims every such row is marked), build_activations (Points column 0)
- backend-16: How many ms past the SP end is the leeway, and is the edge itself counted? | owner: core::Rules::backend_leeway_ms + counted_without_squeeze (strict <) | answered also in: load_rules_file, UserGuide ("3ms" literal), rules.h comment ("this close")

### display

- display-1: How is a timing in ms written (decimals, rounding, unit, space)? | owner: format_ms / format_ms_spaced (src/app/display_format.cpp) | answered also in: activation_badge (%.0f), Activation::notationstr_verbose (truncates to int), SPSqueeze::description (%.1fms), build_activations backend row (%.1f) + tooltip + eff., report page JS fmtMs/toFixed(1) and "Tightest squeeze" tile
- display-2: Which sign does a shown early-fill timing use (positive = hit early)? | owner: Activation::e_difficulty | answered also in: docs/UserGuide.md line 118 (says more negative = earlier), report column text, display_format.h comment
- display-3: How is the average multiplier rounded and shown? | owner: py_round3 / format_avg_mult | answered also in: report page JS r.mult.toFixed(3)
- display-4: When is a timing warn-coloured (orange) or "difficult"? | owner: SPSqueeze/Activation/Path::is_difficult (kDifficultMs, strict >) | answered also in: paths_tab render_timeline (orange outline when badge non-empty), report tier_for Normal floor (< 2 is Normal, so exactly 2.0 is Hard)
- display-5: Which timing tier does a hardest-ms fall in, and where does Beyond start? | owner: timing_tiers / tier_for / beyond_edge_ms | answered also in: report page JS stats (r.ms >= BEYOND), generate_report footer (truncated edge)
- display-6: Which part of an activation (squeeze in, squeeze out, early fill) is the hardest one the badge names? | owner: none (Activation::difficulty returns only the value) | answered also in: activation_badge (re-picks by float equality)
- display-7: What is a path's hardest timing? | owner: Path::difficulty | answered also in: store::summarize_path (own loop over Activation::difficulty)
- display-8: What is a path's hardest required early fill (report Early fill column)? | owner: none | answered also in: report::collect_rows
- display-9: How is Clone Hero rich-text markup removed from title/artist/charter for display? | owner: none (two functions) | answered also in: app::strip_rich_tags (library, details header), report::plain (all three report pages), hydra_batch main (prints raw)
- display-10: How is a count written with a grouped number and singular/plural noun? | owner: none | answered also in: report::counted, library_table charts_text, path_view bars_text, build_activations backends_label, within_label, render_headline stars, build_record_status "SP cap: N bars" (never singular), generate_report cap_label, hydra_batch "SP cap : %d bars"
- display-11: What does the screen say for a Stale / Not analyzed / no-paths record? | owner: render_record_state (stated as shared) | answered also in: render_headline (verbatim copy), build_record_status ("No paths found."), library_table render_table tooltip, render_chips hint, best_path_label
- display-12: Where does an activation sit along the song (0..1), and what is the song's last measure? | owner: none | answered also in: build_activations song_fraction/timeline_end (last-note length), build_scrub_marks and build_time_box (transport length = max(last note, audio end))
- display-13: What does squeezing out a backend note cost, as shown? | owner: core::backend_row_value | answered also in: squeeze_sentences (lost > 0 test) and build_activations row ("(-N)" + warn whenever counted)
- display-14: How is a percentage computed and rounded for display? | owner: none | answered also in: render_dynamics_panel (truncate), collect_dm_rows/build_dm_html %.4f then page toFixed(2), render_analyze_progress %.0f
- display-15: How is a song clock time written? | owner: clock_str | answered also in: none found where I looked
- display-16: What rating label does a backend row get, and which ms ranges does the guide quote? | owner: BackendSqueeze::summarystr | answered also in: docs/UserGuide.md line 122 (3 ms to 85 ms)
- display-17: When does the transfer-scale line print a scale, and when is it orange? | owner: rate_activation (material) | answered also in: build_activations (0.005 print threshold, shows()/same())
- display-18: How is the CH 1.0 vs 1.1 fill-spawn rule described to the user? | owner: search/graph.h FillDeadlineRule (+ CONTEXT.md) | answered also in: settings_bar render_sp_cap help, fill_report generate_fill_report footer
- display-19: Which SP cap is Clone Hero's rule, as named in text? | owner: kCloneHeroSpCap | answered also in: dm_report page column texts and generate_dm_report footer ("SP cap 4" literals)
- display-20: What does the CLI tell the user about whether a result carries its fill rule? | owner: docs/adr/0010 superseded note | answered also in: hydra_batch main error texts
- display-21: Which status bucket and colour does a comparison row get? | owner: collect_dm_rows / collect_fill_rows status | answered also in: dm and fill page JS cells (delta sign picks the class)
- display-22: What score including the solo bonus reaches each star? | owner: none (stars tab adds) | answered also in: render_stars_panel, uitest test_stars
- display-23: Which kick notes make up a kick total? | owner: DynamicsBreakdown::kicks_total / played_total | answered also in: render_dynamics_panel (All kicks row vs Totals), docs/UserGuide.md line 168
- display-24: Is Star Power running at time t in the Preview? | owner: none | answered also in: build_drain_box, build_track_state / toggle_at, build_sp_meter_curve
- display-25: Which fill did an activation take? | owner: none | answered also in: build_preview_scene, build_track_state
- display-26: How many SP bars are banked at a moment between activations (Preview meter)? | owner: engine | answered also in: build_sp_meter_curve (re-derives +1 bar per phrase, capped)
- display-27: Where does an activation's nominal SP end fall (CLI dump)? | owner: engine | answered also in: tools/replay_json paths_json
- display-28: Which user sentence does a raw exception message map to? | owner: throw-site literals | answered also in: plain_error_text (prefix copies of each literal)
- display-29: What range may the Path limit and backend limit inputs take? | owner: none (kSqueezeWindowMs is 500) | answered also in: render_path_limit (-500..500), render_path_footer (0..500), Settings load (no clamp)
- display-30: How many paths does a record keep? | owner: none | answered also in: build_record_status (all_paths().size()), summarize_record (sum of tied_pathcount)
- display-31: Does a library squeeze<N filter include N? | owner: query_matches | answered also in: parse_library_query (squeeze< and squeeze<= the same), docs/UserGuide.md line 84
- display-32: How far is the all-0 path from optimal, as shown? | owner: none | answered also in: build_path_list (vs flat.front(), "(+/-N)"), build_path_buttons (vs best_path(), "N below optimal")
- display-33: What is the default Path limit when none is given? | owner: Settings::mslimit_value (config.h) | answered also in: tools/replay.cpp Args ms = "10"
- display-34: Which colour is Star Power / activation gold? | owner: theme.h kBestPathColor | answered also in: preview_tab literal IM_COL32(255,204,51) five times
- display-35: What is a song's length (last note onset)? | owner: store::song_length_ms | answered also in: build_preview_scene (scene.notes.back().ms)

### fills

- fills-1: Does the chart have its own fills, or must Hydra generate them? | owner: Song::check_activations (src/parse/song.cpp) | answered also in: none found
- fills-2: Where do generated fills go (nearest chord to a downbeat, within max distance, after a cooldown) and how long are they? | owner: Song::check_activations | answered also in: none found (values from core::Rules)
- fills-3: What meter (ticks per measure) is in effect at a chord's tick, for a generated fill's length? | owner: SongTiming / MeasureIndex (core/timing.h) | answered also in: Song::check_activations (own SongIter walk over tpm_changes)
- fills-4: Does an authored fill land on the chord being emitted, or on the previous one? | owner: fill_lands_on_chord (src/parse/song.cpp) | answered also in: none (both parsers call it)
- fills-5: Has the fill end been reached at this chord, so the fill must be attached now? | owner: none | answered also in: MidiParser::push_timestamp, ChartParser::push_timestamp (same gate written twice)
- fills-6: How long is an authored fill (activation_length) and does the last chord lie inside it? | owner: apply_fill_end | answered also in: none
- fills-7: Which chart chords are activation points (fill nodes)? | owner: SongTimestamp::has_activation (song.h) read by ScoreGraph::build | answered also in: build_preview_scene (reads activation_length), replay_path is_fill (wrapper)
- fills-8: What is the fill spawn deadline under CH 1.1 and CH 1.0? | owner: activation_fill_deadline_ms (src/search/graph.cpp) | answered also in: tests/test_fill_deadline.cpp "CH 1.1 is unchanged 4-beats-before math" (recomputes tick_e), clamp tests (recompute fend - len - 250/10000)
- fills-9: Which deadline rule does a run use (legacy flag -> FillDeadlineRule)? | owner: none | answered also in: analyze_at_cap, search_target (src/search/pather.cpp)
- fills-10: Which fill rule produced the results in a database? | owner: store::Lens::legacy_fills (row key) | answered also in: meta engine_mode stamp read by hydra_batch main, hydra_report main, hydra_fillcompare main, RecordStore::add_fill_rule_column
- fills-11: What text marks a 1.0 / 1.1 database ("ch10"/"ch11")? | owner: engine_mode_stamp (search/graph.h) | answered also in: RecordStore::add_fill_rule_column (literal "ch10")
- fills-12: Can the fill be summoned at all (e_offset >= -kEarlyFillWindowMs)? | owner: Engine::branch_activate | answered also in: none
- fills-13: Is an activation E-critical (e_offset < kEarlyFillWindowMs)? | owner: Activation::is_e_critical | answered also in: is_e0 (model.h, restates the same comparison)
- fills-14: Is an activation E0 (E-critical and no skips)? | owner: is_e0 (model.h) | answered also in: none (Activation::is_E0 and Engine::act_difficulty call it)
- fills-15: How hard is one activation (max of its squeezes and its required early fill)? | owner: none | answered also in: Activation::difficulty, Engine::act_difficulty
- fills-16: How hard is a path (max over its activations)? | owner: Path::difficulty | answered also in: summarize_path (own loop), Engine::search_difficulty / close_last_activation (running max)
- fills-17: How many SP bars does an activation need (2)? | owner: none | answered also in: ScoreGraph::add_act_edge (map starts at 2), Engine::branch_activate (p.sp < 2), Engine::advance (old_sp < 2 && sp >= 2)
- fills-18: When did the SP meter become ready (sp_ready_ms)? | owner: Engine::advance | answered also in: none
- fills-19: Is a passed fill charged as a skip? | owner: Engine::branch_activate + Engine::run | answered also in: none in production (replay.h documents it refuses to simulate)
- fills-20: Which fills did the player see as offered, taken or hidden? | owner: none (engine stores only the skip count) | answered also in: build_preview_scene (maps count to the nearest-before fills)
- fills-21: Which live paths are grouped and folded as ties (grouping key)? | owner: Engine::reduce_iteration_paths / reduce_group | answered also in: none
- fills-22: How many tied paths may a leader hold? | owner: core::Rules::max_tied_paths (read by Engine::reduce_group) | answered also in: none
- fills-23: How many paths does a leader stand for (tied count)? | owner: Path::recount_tied_paths | answered also in: Engine::reduce_group (engine tied_count kept in parallel)
- fills-24: Which losing paths does the search keep (depth band, Scores vs Points)? | owner: Engine::reduce_group | answered also in: none
- fills-25: What does the stored depth_mode integer mean (0 scores, 1 points)? | owner: none | answered also in: Settings::to_analysis_settings (==1), within_label (==1), batch_settings_summary (==0), render_score_range (combo index), settings_from in tools/replay.cpp
- fills-26: What depth value is valid (>= 0)? | owner: none | answered also in: render_score_range only (Settings::load_file and settings_from accept any int)
- fills-27: What is the default search depth? | owner: none | answered also in: app::Settings::depth_value = 4, SearchSettings::depth_value = 4 (EngineOptions 0 by design)
- fills-28: Is the best path already the all-0 answer (so the all-0 pass is skipped)? | owner: attach_allzero | answered also in: search_allzero's EngineOptions ms_filter = 0.0 (same 0 ms limit, separate literal); test "search_allzero returns only all-0 paths" recomputes difficulty > 0.0
- fills-29: Is a path all-0 (every activation 0 skips)? | owner: Path::is_allzero | answered also in: Engine::run no_skips_ branch (enforces it during search)
- fills-30: Which fills does a targeted search activate on, and did it get them all? | owner: Engine::run (target_act_ticks) | answered also in: search_target (post-check of ticks)
- fills-31: What was Auto's cap ladder (for deleting Auto results)? | owner: Rules::retired_auto_fingerprint (frozen literal) | answered also in: load_rules_file (ignores auto keys)
- fills-32: Does the leaderboard comparison require the 1.1 fill rule and the 4-bar cap? | owner: none | answered also in: render_actions_row (all three gates), collect_dm_rows (forces cap 4, passes fill rule through)
- fills-33: What is the user-facing name of each fill rule? | owner: none | answered also in: hydra_batch main, batch_settings_summary, report.cpp cap label, fill_report page, settings_bar help
- fills-34: How is the deadline rule described in words (4 beats; one fill-length; 250..10000 ms)? | owner: CONTEXT.md / ADR 0010 | answered also in: graph.h, pather.h, UserGuide, settings_bar help_marker, fill_report footer, test_fill_deadline header
- fills-35: Which rule values go into the rules fingerprint? | owner: fixed_cap_text | answered also in: none
- fills-36: Which fill rule values are legal in hydra_rules.ini? | owner: load_rules_file | answered also in: none
- fills-37: How hard is an early fill, in ms (-e_offset)? | owner: early_fill_difficulty | answered also in: none (Activation::e_difficulty and Engine::act_difficulty call it)
- fills-38: Is a shown early fill required or optional? | owner: Activation::is_E0 | answered also in: build_activations, activation_badge (wrappers)

### parse

- parse-1: Which chart format is a file, judged by its extension (.mid/.chart/.sng/.srb)? | owner: chart_format_of (src/parse/chart_files.cpp) | answered also in: resolve_preview_source (src/app/preview_source.cpp, ends_with_ci ".sng"/".srb"), discover_charts (calls owner)
- parse-2: Which loose file in a folder is the notes file, and which wins when both notes.mid and notes.chart exist? | owner: notes_file_format + discover_charts (mid over chart) | answered also in: load_songpath_sng (mid over chart inside a .sng, own loop), play_chart.main (tools/ch_probe, .chart over .mid, opposite answer)
- parse-3: Which entry inside a container is its notes file and what format is it? | owner: none (split) | answered also in: load_songpath_sng (exact name notes.mid/notes.chart), load_songpath_srb (extension of the metadata filename, else MThd sniff), extract_srb_audio (re-walks stream 1 and 2 positions), parse_srb_metadata (stream 1 position)
- parse-4: Which file is this folder's song.ini? | owner: is_song_ini | answered also in: discover_charts (last match in listing), find_song_ini (first match in listing)
- parse-5: How is song.ini read (section, comments, key case, value trim)? | owner: read_song_ini_keys | answered also in: none found (read_metadata_ini and read_ini_delay_ms call owner)
- parse-6: What is a chart's artist/charter when the metadata lacks or blanks it? | owner: none | answered also in: read_metadata_ini, parse_sng_metadata, parse_srb_metadata (three literal "<unknown artist>"/"<unknown charter>" copies, srb treats empty as unknown, ini/sng keep empty)
- parse-7: What is a chart's title when the metadata lacks or blanks it? | owner: title_or_unknown | answered also in: parse_srb_metadata (skips empty name itself; harmless), callers wrap owner
- parse-8: What is a chart's identity hash and how is it normalised for matching? | owner: stream_md5 / hash_chart_file (lowercase hex MD5 of all bytes) | answered also in: records_by_hash (to_lower_ascii), collect_dm_rows (to_lower_ascii on library md5), parse_score (to_lower_ascii on leaderboard identifier)
- parse-9: Which drum track in a .mid holds the drums? | owner: MidiParser::parse ("PART DRUMS", exact, first track) | answered also in: play_chart.parse_midi ("DRUMS" substring, first match, name read from first 200 bytes)
- parse-10: Which MIDI pitches / .chart note numbers belong to the chosen difficulty and which lane each maps to? | owner: difficulty_base_pitch + is_handled_note + MidiParser::optype; ChartParser::optype + difficulty_name section | answered also in: play_chart.midi_notes_to_lanes / DRUM_NOTES (Expert 96-100 fixed), play_chart.chart_notes_to_lanes / CHART_DRUM_NOTES (treats N 5 as green; Hydra drops N 5)
- parse-11: Is a yellow/blue/green gem a cymbal or a tom? | owner: MidiParser::op_tom/op_note (tom marker 110-112 is a span from note-on to note-off) and ChartParser::optype (66-68 same-tick cymbal marker, pro only) | answered also in: play_chart.midi_notes_to_lanes (tom marker only on the same tick), play_chart.chart_notes_to_lanes (marker regardless of pro), gem_of (track_state.cpp re-gates cymbal on pro and lane != Red)
- parse-12: Which lanes may carry a cymbal flag / a 2x flag? | owner: allows_cymbals | answered also in: lane_flag, lane_allows_flag, Chord::code stray_flag (model.cpp), gem_of (track_state.cpp, lane != Red)
- parse-13: Is a note a 2x kick, and is it read at this difficulty/setting? | owner: MidiParser::optype case 95 + ChartParser::optype case 32 (gated only on bass2x) and Settings::effective_bass2x (Expert-only) | answered also in: DynamicsLoadJob::run (kDynamicsParseBass2x=true at every difficulty), DynamicsBreakdown::played_total, dynamics_entry_from_analysis gate, play_chart.midi_notes_to_lanes (95 always), chart_notes_to_lanes (32 always)
- parse-14: Is a disco-flip section active for the chosen difficulty? | owner: re_disco_on/re_disco_off + MidiParser/ChartParser optype | answered also in: none; but the regex hard-codes "mix.3" (Expert) while the difficulty is chosen elsewhere (difficulty_base_pitch / difficulty_name); play_chart ignores disco entirely
- parse-15: What does disco flip do to a chord? | owner: Chord::apply_disco_flip (via emit_chord_timestamp, pro only) | answered also in: none found
- parse-16: Is a note a ghost or accent? | owner: MidiParser::optype vel_dyn (127/1) gated by op_enable_dynamics; ChartParser 34-37/40-43 | answered also in: none found in C++; play_chart ignores dynamics (does not affect lanes)
- parse-17: Does this chart have dynamics enabled? | owner: MidiParser (dynamics_enabled_ at end of track), ChartParser::parse (always true) | answered also in: op_note (per-note, only notes after the event tick), docs/UserGuide.md line 170 (says counts still shown)
- parse-18: What is a chord's stored code and how is it read back? | owner: Chord::code / Chord::from_code | answered also in: none found (path_codec, replay call owner)
- parse-19: Which difficulty does a name/setting mean, and what is its section/pitch? | owner: difficulty_name / difficulty_from_name / difficulty_base_pitch | answered also in: Settings::difficulty (wrapper), play_chart.parse_chart ([ExpertDrums] fixed)
- parse-20: Is a flam marker applied? | owner: MidiParser (109) + Chord::apply_flam_conversion | answered also in: play_chart.parse_midi (ignores 109; no second note)
- parse-21: Which chart length (last note ms) belongs to a chart identity? | owner: store::song_length_ms | answered also in: build_preview_scene (scene.song_length_ms = notes.back().ms); songmeta.length_ms keyed by hyhash but written per difficulty (upsert_song)
- parse-22: When the same chart (same md5) is found twice, which copy names it and is it analyzed once? | owner: none | answered also in: rebuild_chart_library (first listed copy, MIN(rowid)), RecordStore::upsert_song via save_analysis (whichever copy was analyzed last), run_batch (no dedupe by md5 within one run)
- parse-23: What identity keys the dynamics counts? | owner: dynamics_store_key (md5, difficulty, pro) | answered also in: dynamics_cache_key (notespath, pro, difficulty) for the in-memory copy
- parse-24: How is the .sng container laid out (XOR mask, tables)? | owner: sng_read_metadata / sng_read_file_table / sng_decode_file | answered also in: tests make_sng (test_sng.cpp), make_sng (test_preview_source.cpp), write_sng_with_metadata (test_analysis.cpp) re-encode it
- parse-25: How is the .srb container laid out? | owner: srb.h (srb_inflate_stream, srb_parse_metadata) | answered also in: make_srb (test_srb.cpp), make_srb (test_preview_source.cpp)
- parse-26: What tempo/resolution does a chart have when the file omits it? | owner: ChartParser::parse (Resolution required, throws) / MidiFile::parse | answered also in: play_chart.parse_chart (Resolution fallback 480, tempo fallback 120), play_chart.parse_midi (tempo 120)
- parse-27: Where does a chart's audio offset come from? | owner: preview_audio_offset_ms + ChartParser Offset + read_ini_delay_ms/sng_delay_ms | answered also in: none found
- parse-28: Practice section names in the chart | owner: section_name_of | answered also in: none found

### score

- score-1: What is one note worth at 1x (50 a gem, +15 a cymbal, doubled for a ghost/accent)? | owner: ChordNote::basescore (+ kNoteBasePoints/kCymbalBonusPoints) | answered also in: category_scores (base+cymb+dyn_cymb plus the pad's 50 in accent/ghost; dynamics_bonus formula), test "stars: the base score is the sum of every note's basescore"
- score-2: What combo multiplier applies at a given combo count (x1/x2/x3/x4 at 10/20/30)? | owner: to_multiplier | answered also in: MultSqueeze::applies (hard-coded combo set 7,8,17,18,27,28 + mod-10 rule), MultSqueeze::multiplier (to_multiplier(combo)+1), MultSqueeze::direction (combo%10==7)
- score-3: At which combo does each note of a chord score, and what is the combo after the chord? | owner: category_scores (combo+1 per note) | answered also in: ScoreGraph::build (combo_ += count), replay_path (combo += count), model.cpp comment on MultSqueeze::applies (says combo_+i)
- score-4: How much does Star Power multiply a chord's value? | owner: category_scores (the sp_* accumulators pay one extra copy) | answered also in: kStarPowerMultiplier/shown_multiplier (timing.h)
- score-5: What does squeezing a chord out of SP cost (the first-hit share)? | owner: category_scores sqout_reduction / CategoryScores::sqout_sp | answered also in: build_activations ("-%d" = bsq.points - value), category_scores itself (per-note sp accumulators vs note.basescore()*combo_multiplier)
- score-6: Which chords on a path get their SP doubling paid? | owner: engine (ScoreGraph SP track + frontend_points + Engine::create_deactivated_path) | answered also in: replay_path window walk (tick >= act_tick, tick <= deact_tick or counted_without_squeeze)
- score-7: What is one backend row worth on one path? | owner: core::backend_row_value | answered also in: build_activations picks the SqOutPosition from BackendRating::squeezed_out instead of sqout_position()
- score-8: How many solo-bonus points does a chord earn? | owner: kSoloBonusPerNote (no function owns the formula) | answered also in: ScoreGraph::build, replay_path
- score-9: Which consecutive chords form one solo section (where a solo run ends)? | owner: none | answered also in: replay_path (last_of_run), build_preview_scene (solo span coalescing)
- score-10: What is a path's total score (sum of six categories)? | owner: Path::totalscore | answered also in: ReplayScore::total, Engine::advance / branch_activate / create_deactivated_path (running p.score kept beside p.sc[])
- score-11: What is the chart's base score (every note at 1x, no SP)? | owner: Path::chart_base_score | answered also in: test_stars note-sum test (recomputes from basescore)
- score-12: What is a path's score without the solo bonus? | owner: none | answered also in: path_stars (totalscore - score_solo), Path::avg_mult (totalscore - score_solo), render_stars_panel (cutoff + solo_bonus), replay_path cum_onscreen_total (cum.total - solo_pending)
- score-13: What is a path's average multiplier and how is it described? | owner: Path::avg_mult | answered also in: model.h comment ("points per scored note"), report.cpp column tooltip ("Points per note on average: ...")
- score-14: What score is needed for N stars? | owner: star_cutoff / star_cutoffs | answered also in: test helper cutoffs_for (re-runs star_cutoffs' loop)
- score-15: How many stars does the best path earn, as shown? | owner: path_stars | answered also in: stored summary column `stars` (summarize_path at write time, fill_missing_stars) read by render_headline and the library stars: filter, while render_stars_panel recomputes cutoffs from the blob
- score-16: What is the most stars a chart can show (7)? | owner: kMaxStars | answered also in: kStarsError text "0 to 7" (library_query.cpp)
- score-17: Is a chord a multiplier squeeze at this combo? | owner: MultSqueeze::applies | answered also in: to_multiplier straddle (what category_scores pays; the test's own derivation)
- score-18: How many points does a multiplier squeeze win? | owner: MultSqueeze::points | answered also in: category_scores (base-sorted order pays the real gain)
- score-19: Which paths are "optimal"? | owner: engine sort + HydraRecord::best_path (paths[0]) | answered also in: report.cpp JS rank===1 ('best' row, Best only filter), build_path_list / build_path_buttons (group 0 = every path at the top score)
- score-20: What does the all-0 path cost against the optimal path? | owner: none | answered also in: build_path_list (allzero_label delta), build_path_buttons (detail "below/above optimal")
- score-21: Is a path inside the user's score range (Scores / Points band)? | owner: Engine::reduce_group | answered also in: Engine::reduce_group fast path (n <= depth+1 all distinct) vs general loop; filtered vs unfiltered branches
- score-22: How does a posted leaderboard score compare with Hydra's optimal (delta, status, percent)? | owner: collect_dm_rows | answered also in: dm page JS cells (delta<0 -> "over"), dm page JS stats (counts, points left), tally_dm_rows (counts)
- score-23: How do the 1.0 and 1.1 fill-rule scores compare? | owner: collect_fill_rows | answered also in: fill page JS (gains/losses)
- score-24: What running score, multiplier and combo does the game show at a moment? | owner: replay_path (cum_onscreen_total, multiplier_shown, combo_after) | answered also in: build_score / build_score_box (read only)
- score-25: Where is the score held in Clone Hero's memory? | owner: tools/ch_probe/constants.py OFF_SCORE | answered also in: tools/ch_probe/experiments/hit_detect.py OFF_SCORE
- score-26: Is a note a ghost or an accent (velocity rule, kicks included)? | owner: MidiParser::optype | answered also in: none found where I looked (src/parse)
- score-27: In what order are a record's paths ranked by score? | owner: engine sort (rebuild/run order) + all_paths traversal | answered also in: report collect_rows (stable_sort by totalscore), build_path_list (assumes traversal is already score-descending)

### screenA

- screenA-1: path button score and the headline score (comma-grouped total) | owner: Path::totalscore | answered also in: none (render_headline, build_path_buttons, build_record_status all call it)
- screenA-2: path notation on a button, headline and library cell | owner: Path::pathstring | answered also in: none
- screenA-3: which paths are "Optimal" (gold, first heading) | owner: build_path_buttons via build_path_list group 0 | answered also in: HydraRecord::best_path (render_headline "Optimal path", build_path_buttons `best`), build_path_list allzero_label (flat.front())
- screenA-4: a path's hardest timing number beside its button | owner: Path::difficulty | answered also in: summarize_path (own max loop -> hardest_ms, library squeeze<= and report), activation_badge (adds optional early fills), Activation::notationstr_verbose (Ctrl+C, truncated)
- screenA-5: is that timing orange (difficult) | owner: Path::is_difficult (kDifficultMs) | answered also in: Activation::is_difficult (re-worded rule), SPSqueeze::is_difficult
- screenA-6: how far the all-0 path is from optimal | owner: build_path_buttons (detail line) | answered also in: build_path_list (allzero_label, test-only)
- screenA-7: is the all-0 path shown at all (dedupe) | owner: build_path_list | answered also in: none
- screenA-8: the "Within N scores/points" heading | owner: within_label | answered also in: none
- screenA-9: activation count and SP left over beside "Activations" | owner: build_activations reading Path::leftover_sp | answered also in: none
- screenA-10: an activation row's notation, measure and bars | owner: Activation::notationstr, format_measure, Activation::sp_meter | answered also in: none
- screenA-11: the activation badge: whether it shows, which squeeze/fill it names, its ms | owner: activation_badge over Activation::difficulty | answered also in: Activation::difficulty (max), notationstr_verbose (truncated ms), path button (%.1f)
- screenA-12: badge / squeeze-box border warning colour | owner: Activation::is_difficult and SPSqueeze::is_difficult | answered also in: Path::is_difficult
- screenA-13: timeline mark position and the end measure label | owner: build_activations (song_fraction, timeline_end) | answered also in: render_timeline hard-codes the left label "m1"
- screenA-14: early fill line value and (required)/(optional) | owner: Activation::e_difficulty(true), Activation::is_E0 | answered also in: activation_badge
- screenA-15: squeeze sentence wording: earn vs keep direction and ms | owner: squeeze_sentences (t >= 0 / t <= 0 tests on SPSqueeze::timing) | answered also in: SPSqueeze::description, rate_activation (difficulty >= 0 picks the achieving direction)
- screenA-16: what the squeeze-out costs ("N fewer points" and "(-N)") | owner: backend_row_value | answered also in: squeeze_sentences and build_activations each compute points - value and each test counted_without_squeeze
- screenA-17: is a backend row counted with no squeeze | owner: counted_without_squeeze | answered also in: none (called from squeeze_sentences, build_activations, summarystr, rate_activation x2)
- screenA-18: a backend row's Points column | owner: backend_row_value | answered also in: build_activations picks SqOutPosition itself (Exact/NoSqOut) instead of sqout_position(), which the engine (Engine::create_deactivated_path) and replay_path use
- screenA-19: which backend row is the squeezed-out note | owner: Activation::is_sqout_backend | answered also in: none (rate_activation wraps)
- screenA-20: which backend rows the table lists | owner: Activation::display_backends | answered also in: build_activations backend-limit filter (display window)
- screenA-21: backend row rating label | owner: BackendSqueeze::summarystr | answered also in: none
- screenA-22: is an "(eff. N ms)" figure printed on a row / SqIn sentence | owner: rate_activation | answered also in: transfer_is_material (first clause re-typed inline twice in rate_activation)
- screenA-23: is the scale line shown at all | owner: build_activations shows() (|r-1| >= 0.005) | answered also in: transfer_is_material / squeeze_rating.h ("shown when it is material"), CONTEXT.md "Squeeze rating"
- screenA-24: is the scale line orange | owner: build_activations scale_warn | answered also in: rate_activation warn flags
- screenA-25: overfill warning shown | owner: rate_activation cap_clamped (+ stored clamp_tick) | answered also in: none
- screenA-26: eff. tooltip budget numbers | owner: squeeze_budget_ms | answered also in: none
- screenA-27: multiplier squeeze lines and the fold summary | owner: MultSqueeze::points/howto, multsqueeze_summary | answered also in: none
- screenA-28: score breakdown lines and avg multiplier | owner: stored Path::score_*, Path::avg_mult, format_avg_mult | answered also in: none
- screenA-29: valid range of the backend limit | owner: none | answered also in: render_path_footer (clamp 0..500, literal 500 = kSqueezeWindowMs), Settings::backend_limit (abs, no clamp), config load (atoi, no clamp)
- screenA-30: Stars tab base score | owner: Path::chart_base_score | answered also in: test "the base score is the sum of every note's basescore" (sum of ChordNote::basescore)
- screenA-31: star cutoffs per star | owner: star_cutoff / star_cutoffs | answered also in: none
- screenA-32: star count in the panel headline | owner: path_stars | answered also in: stored PathSummary::stars read by render_headline (double read against the live record the Stars tab reads)
- screenA-33: "With full solo bonus" column | owner: render_stars_panel (cutoff + solo_bonus) | answered also in: none
- screenA-34: which notes the chart has at this difficulty / 2x setting (Dynamics counts) | owner: parser (MidiParser::optype case 95 gated by mode_bass2x_, fed Settings::effective_bass2x) | answered also in: DynamicsLoadJob::run (kDynamicsParseBass2x=true on every difficulty) + DynamicsBreakdown::played_total(bass2x) re-deciding by bin
- screenA-35: which Dynamics row a note goes in | owner: row_for | answered also in: none
- screenA-36: Dynamics totals and percentages | owner: played_total + inline int truncation in render_dynamics_panel | answered also in: kicks_total
- screenA-37: "Dynamics enabled" | owner: parser (Song::dynamics_enabled) | answered also in: none
- screenA-38: record state text (stale / no paths) | owner: render_record_state | answered also in: render_headline (same literals), build_record_status ("No paths found.")
- screenA-39: hit window feeding ratings and budgets | owner: Settings::hit_window_ms | answered also in: none
- screenA-40: sign of the early fill figure | owner: Activation::e_difficulty (positive = hit early) | answered also in: docs/UserGuide.md line 118 (says more negative = earlier)
- screenA-41: Copy path / Ctrl+C string | owner: Path::pathstring_verbose / Activation::notationstr_verbose | answered also in: none

### screenB

- screenB-1: Preview SP gauge fill and "x.x/4" readout: bars banked at the playhead between anchors | owner: none (engine stores only per-activation sp_meter, deact node, collected ticks; search/graph.cpp ScoreGraph::max_sp_bars counts phrases) | answered also in: app/preview_view.cpp build_sp_meter_curve (re-walks phrase banking +1 capped, squeezed-out banking), sp_meter_bars_at, sp_meter_readout
- screenB-2: SP left after a phrase is collected while SP runs (gauge step size and drain slope) | owner: search/graph.cpp ScoreGraph::extend_deacts (on-time, cap ceiling) and ScoreGraph::store_new_backend (late SqIn extends from the old deact node) | answered also in: app/preview_view.cpp build_sp_meter_curve (remaining = min(remaining + 2 measures, 2*cap) from the collection note)
- screenB-3: Is SP running at the playhead (drain box header/gold colour, highway SP tint, gauge drain) | owner: none single (all read the stored deact node) | answered also in: build_drain_box (a.ms <= now < sp_end_ms), render/track_state.cpp build_track_state (sp_active_ interval + toggle_at), build_sp_meter_curve (drain segments act.ms..sp_end_ms)
- screenB-4: Drain box rate "1 bar / X s" | owner: SongTiming::ms_per_measure_at with kMeasuresPerSpBar (build_drain_box wraps) | answered also in: none found where I looked
- screenB-5: Drain box "full meter X s" | owner: build_drain_box (bar time in force x cap; user said "yes do the snap", plan 2026-09-27-preview-sp-drain-box.md line 482) | answered also in: SongTiming::sp_end_ms (the real drain length from here), docs/UserGuide.md line 162 wording ("how long a full meter would last from here")
- screenB-6: Drain box "empties in X s" | owner: stored deact node via activation_deact_tick (build_preview_scene copies it) | answered also in: none
- screenB-7: Tempo / time signature / section in force at the playhead (time box lines) and the drain rate's section | owner: none (build_time_box decides BPM by ms, time sig and section by rounded tick; build_drain_box decides section by rounded tick via ms_per_measure_at) | answered also in: build_time_box (two rules), build_drain_box
- screenB-8: The playhead moment the boxes read (clamping) | owner: preview_view.cpp shown_ms/tick_at (time box, step_tick_ms) | answered also in: build_drain_box (own clamp: only < 0), build_score_box/build_next_act_box/sp_meter_readout (no clamp)
- screenB-9: Score box total, "xN" multiplier and combo | owner: engine stored totals (Path::totalscore) for the final value; per-chord values from core/replay.cpp replay_path | answered also in: search engine/graph (which chords SP pays); replay_path re-decides SP window membership per chord (in_sp), checked only by total
- screenB-10: Which candidate fills the highway draws as taken / offered / hidden | owner: search/engine.cpp Engine::branch_activate (charges a skip only on a real opportunity) | answered also in: build_preview_scene ("n nearest candidates before the activation"), build_track_state (re-finds the taken fill by end_tick == act tick for the lane)
- screenB-11: "Next: activation i of n" box and < Act / Act > targets | owner: build_next_act_box / activation_jump_ms with kOnActivationMs = 0.5 | answered also in: none
- screenB-12: SP cap the gauge uses | owner: record sp_cap (render_preview_panel picks it) | answered also in: build_sp_meter_curve (cap < 1 -> 1), render_preview_panel gauge (std::max(1, cap) and 0..1 clamp)
- screenB-13: Path report "Timing" tier chip and the "Past N ms" tile count | owner: core/squeeze_rating.cpp timing_tiers + app/report.cpp tier_for | answered also in: report.cpp kPageJs stats() (r.ms >= BEYOND), column description prose, footer prose
- screenB-14: Where "Beyond" starts on the path report | owner: beyond_edge_ms | answered also in: report.cpp kPageJs BEYOND = Math.max(cutoffs), column text "at least twice the hit window"
- screenB-15: Is a path's hardest timing "difficult" (Paths tab warn colour) vs its report tier "Normal" | owner: none (two edges on kDifficultMs) | answered also in: Path::is_difficult / Activation::is_difficult / SPSqueeze::is_difficult (> 2.0), timing_tiers "Normal" (< 2.0)
- screenB-16: A path's hardest ms (report "Hardest ms", Paths button timing) | owner: Path::difficulty | answered also in: store/record_store.cpp summarize_path (own max loop over Activation::difficulty)
- screenB-17: Report "Early fill (ms)" column | owner: Activation::e_difficulty | answered also in: report.cpp collect_rows max loop (wrapper over the owner)
- screenB-18: Which path is optimal / best | owner: HydraRecord::best_path (paths[0]) | answered also in: path_view.cpp build_path_buttons (every path in the first score group is "(optimal)" in the Preview "Showing" list), report.cpp collect_rows (stable sort by score, exactly one rank-1 row is "best")
- screenB-19: Fill comparison page: which rule scores higher (status chip, delta colour) | owner: fill_report.cpp collect_fill_rows (status from delta sign) | answered also in: fill_report.cpp kPageJs cells() (deltaCls from r.delta sign)
- screenB-20: dmleaderboards page: is a score above optimal ("over") | owner: dm_report.cpp collect_dm_rows (s.score > opt) | answered also in: dm_report.cpp kPageJs cells() (r.delta < 0 -> "over")
- screenB-21: dmleaderboards page: which Hydra record is "optimal" (cap and fill rule) | owner: dm_report.cpp collect_dm_rows (CapQuery::at(kCloneHeroSpCap), lens = app settings incl. 1.0 fills) | answered also in: kPageJs column text and footer ("at SP cap 4", "the Clone Hero rule")
- screenB-22: Which fill rule does a database file hold | owner: none | answered also in: cli/batch.cpp main (unstamped + results = ch11, mismatch exits 2), cli/report.cpp main (stamp ch10 forces legacy lens), cli/fillcompare.cpp main (unstamped never warns, mismatch only warns)
- screenB-23: Do stored results carry their fill rule (CLI text) | owner: CONTEXT.md "Fill spawn deadline (CH 1.0)" (part of a record's key) | answered also in: cli/batch.cpp stderr ("Legacy results are not tagged", "The rule is not stored on each result"), tools/replay.cpp usage ("Every stored row is a 1.1 result")
- screenB-24: Squeeze window value printed in CLI text | owner: kSqueezeWindowMs (core/model.h) | answered also in: tools/replay.cpp usage literal "within 500 ms"
- screenB-25: hydra_replay default analysis settings (cap, ms limit, depth, drum options) | owner: app::Settings member defaults (app/config.h) | answered also in: tools/replay.cpp struct Args literals ("4", "10", "scores", 4, true, true, "expert")
- screenB-26: Nominal SP end of an activation (printed on hydra_replay selfcheck FAIL lines; test fixtures) | owner: search/graph.cpp ScoreGraph::add_act_edge (activation_initial_end_times) | answered also in: tools/replay.cpp check_chart (plusmeasure(act, 2*sp_meter)), tests/test_preview_view.cpp sp_act_at
- screenB-27: Which phrase chord a window could squeeze out (hydra_replay note/warning text) | owner: search/graph.cpp is_recent_to_head + add_deact_edge | answered also in: core/replay.cpp sqout_candidates (|ms - D| < kSqueezeWindowMs)
- screenB-28: Average multiplier shown (report column, Paths breakdown) | owner: Path::avg_mult + display_format py_round3/format_avg_mult | answered also in: none (both screens call the owner)
- screenB-29: Path report "Tightest squeeze" tile | owner: report.cpp kPageJs stats() (max r.ms) | answered also in: column text says r.ms is "the hardest squeeze or early fill"
- screenB-30: Does a phrase end before the activation / inside the active window (gauge walk) | owner: none | answered also in: build_sp_meter_curve run_flat_to (end_ms < act.ms), window loop (end_tick > act.tick, end_ms >= sp_end_ms), split loop (t > act.tick, t < sp_end_tick)
- screenB-31: Preview audio offset from song.ini delay / chart Offset | owner: preview_source.cpp preview_audio_offset_ms | answered also in: none found where I looked
- screenB-32: hydra_batch header lines (SP cap, timing cap, depth) | owner: app::Settings via batch_run/to_analysis_settings | answered also in: cli/batch.cpp prints settings fields directly (same source, wrapper)

### spwin

- spwin-1: How many bars does the SP meter hold between activations (one per phrase completed outside SP, capped at the SP cap)? | owner: Engine::advance (base-track branch, src/search/engine.cpp) | answered also in: build_sp_meter_curve run_flat_to + trailing loop (src/app/preview_view.cpp), the "not offered" simulation note in src/core/replay.h
- spwin-2: How many bars must be banked before SP can be activated (2)? | owner: none (three literals) | answered also in: Engine::branch_activate (p.sp < 2), Engine::advance (old_sp < 2 && sp >= 2 for sp_ready_ms), ScoreGraph::add_act_edge (loop starts at sp = 2)
- spwin-3: Where does an activation's SP end before any phrase is collected (activation + 2 measures per banked bar)? | owner: ScoreGraph::add_act_edge (activation_initial_end_times) | answered also in: paths_json nominal_deact_tick (tools/replay_json.cpp), check_chart "nominal" (tools/replay.cpp), build_sp_meter_curve drain start (remaining = 2*sp_meter measures), test fixture sp_act_at (tests/test_preview_view.cpp), test_squeeze_rating fixtures
- spwin-4: How many ticks is N measures of SP from a given tick (measures-to-ticks for SP length)? | owner: SongTiming::plusmeasure | answered also in: SongTiming::measures_at_tick_f / tick_at_measures_f / sp_end_ms (continuous copy, display-only), build_sp_meter_curve drain via measures_at_tick_f
- spwin-5: Where does a pending SP end move when a phrase is collected while SP runs (+2 measures, or the cap ceiling)? | owner: ScoreGraph::extend_deacts | answered also in: ScoreGraph::add_deact_edge (sqout_time/sqin_time +1 bar, no cap ceiling), ScoreGraph::store_new_backend (late-SqIn sqin_time +1 bar), frontend_transfer_scales (pre end = D - 1 bar), build_sp_meter_curve (remaining + 2 measures, min 2*cap), ADR 0011 text and Activation::transfer_pre comment in core/model.h
- spwin-6: Did the SP cap clamp this window, and which collected note anchors its end? | owner: ScoreGraph::extend_deacts (clamped flag) + Engine::advance (clamp_tick) | answered also in: build_sp_meter_curve (min(remaining+2, 2*cap)), frontend_transfer_scales (always anchors on act_tick), rate_activation cap_clamped (reads stored clamp_tick: reader)
- spwin-7: How tall can the meter get (min of SP cap and the song's phrase count)? | owner: ScoreGraph::max_sp_bars | answered also in: graph_build_cap (src/search/pather.cpp), Engine::advance (sp > sp_cap_ clamp)
- spwin-8: What is the smallest allowed SP cap (1)? | owner: none | answered also in: Settings::load_file (v >= 1), render_sp_cap (max(1, cap)), build_sp_meter_curve (cap < 1 ? 1), render_preview_panel gauge (max(1, sp_meter_cap()))
- spwin-9: What SP cap does the Preview meter use when the record carries none? | owner: none | answered also in: render_preview_panel (value_or(kCloneHeroSpCap)), build_preview_scene default argument
- spwin-10: Where does this activation's SP end (the deact node D)? | owner: rebuild in src/search/engine.cpp (stamps Activation::deact_tick) | answered also in: readers only: activation_deact_tick (wrapper), windows_for_path, build_preview_scene, frontend_transfer_scales, windows_from_json, paths_json, check_chart
- spwin-11: Is a chord, or a moment, inside an activation's SP window? | owner: the ScoreGraph SP track (structural) + core::paid_by_sp_walk | answered also in: replay_path (tick >= act && tick <= deact, min(offset,0)), build_drain_box (a.ms <= now < sp_end_ms), build_track_state sp_active_ interval, build_sp_meter_curve window (end_ms < sp_end_ms && end_tick > act.tick) and collected filter (t > act.tick && t < sp_end_tick)
- spwin-12: How far (ms) is a backend row from the SP end? | owner: ScoreGraph::add_deact_edge | answered also in: ScoreGraph::store_new_backend, rebuild tail-backend loop (final_sp_end), replay_path (row.ms - deact_ms)
- spwin-13: Which phrase chord is the squeeze (SqIn/SqOut) note at a deact node (first SP chord strictly within kSqueezeWindowMs)? | owner: ScoreGraph::add_deact_edge (chords up to D) + ScoreGraph::store_new_backend (chords after D) | answered also in: sqout_candidates + resolve_sqout_note + ambiguous_window_warnings (src/core/replay.cpp)
- spwin-14: Is a note close enough to a deact node to be a squeeze/backend (strictly under kSqueezeWindowMs)? | owner: ScoreGraph::is_recent_to_head | answered also in: ScoreGraph::build sqout_deacts filter, Activation::display_backends, sqout_candidates, render_path_footer backend-limit clamp literal 500
- spwin-15: Where does a row sit against the squeezed-out chord (before / on / after)? | owner: core::sqout_position | answered also in: Activation::is_sqout_backend, Activation::display_backends is_beyond_sqout, rebuild remove_if (ticks > sqout_tick), Engine::trim_cols (tick >= sqinout_time), build_activations and squeeze_sentences (hard-coded Exact/NoSqOut)
- spwin-16: Did this activation end on a squeeze-out? | owner: rebuild (stores both sqinouts SqOut and sqout_tick) | answered also in: windows_for_path (reads both), windows_from_json (reads both), Activation::is_sqout_backend (tick), squeeze_sentences (kind), frontend_transfer_scales (kind for SqIn), test "collected phrases: the corpus agrees"
- spwin-17: Which phrases did SP collect while it ran? | owner: Engine::advance + Engine::create_deactivated_path (collected_phrase_ticks) | answered also in: build_sp_meter_curve (reader; derives the squeezed-out set as the complement)
- spwin-18: How many bars does the meter hold right after SP ends (0, or 1 after a squeeze-out)? | owner: Engine::create_deactivated_path (c.sp = is_sq_out ? 1 : 0) | answered also in: build_sp_meter_curve (bank = min(squeezed_out, cap))
- spwin-19: Where did SP end before a SqIn extended it (the "pre" end)? | owner: none stored (the engine's deact edge destination knows it) | answered also in: frontend_transfer_scales (D minus one bar)
- spwin-20: What does the SP meter read at each moment of an activation (the drain)? | owner: none in the engine | answered also in: build_sp_meter_curve (continuous measure drain, own cap clamp, forced to 0 at D), sp_meter_bars_at
- spwin-21: Is SP running at the playhead, and how long until it empties? | owner: none (display) | answered also in: build_drain_box, build_track_state
- spwin-22: Where does SP end when it outlasts the chart? | owner: Engine::emit_acts (final_sp_end) | answered also in: rebuild (stamps it as deact_tick)
- spwin-23: When the cap ceiling and the plain +2 measures land on the same tick, which wins (no clamp)? | owner: ScoreGraph::extend_deacts | answered also in: build_sp_meter_curve min() (no clamp flag, same value)

### squeeze

- squeeze-1: Which SP phrase note at a deact node becomes the edge's SqIn/SqOut note (the first SP phrase end inside the squeeze window)? | owner: none (written twice: ScoreGraph::add_deact_edge for notes up to D, ScoreGraph::store_new_backend for notes after D) | answered also in: sqout_candidates + resolve_sqout_note + ambiguous_window_warnings (core/replay.cpp), BackendSqueeze::summarystr (labels every is_sp row "... SqOut")
- squeeze-2: What is a row's / squeeze note's offset in ms from the SP end? | owner: none (ScoreGraph::add_deact_edge, ScoreGraph::store_new_backend) | answered also in: rebuild (engine.cpp tail backends, via ms_index().at), replay_path (clamps rows at/before D to <= 0)
- squeeze-3: Is a note inside the squeeze window (kSqueezeWindowMs) of the SP end? | owner: none (constant kSqueezeWindowMs only) | answered also in: ScoreGraph::is_recent_to_head, ScoreGraph::build (inline sqout_deacts test), Activation::display_backends, sqout_candidates
- squeeze-4: How hard is one squeeze in ms (SqIn = offset, SqOut = -offset)? | owner: squeeze_difficulty | answered also in: SPSqueeze::difficulty (wrapper), Engine::act_difficulty (wrapper call)
- squeeze-5: What is an activation's hardest ms (max over squeezes and an E0 fill)? | owner: Activation::difficulty | answered also in: Engine::act_difficulty (separate max loop), activation_badge (adds optional-fill fallback)
- squeeze-6: What is a path's hardest ms? | owner: Path::difficulty | answered also in: summarize_path (own loop), Engine::search_difficulty + close_last_activation
- squeeze-7: Is a squeeze / activation / path "difficult" (warning colour, kDifficultMs edge)? | owner: SPSqueeze::is_difficult (d > kDifficultMs) | answered also in: Activation::is_difficult (own loop), Path::is_difficult (difficulty() > k), timing_tiers "Normal" band (d < k, opposite side of the edge)
- squeeze-8: Which report timing tier does a hardest ms fall in, and where does "Beyond" start? | owner: timing_tiers + tier_for | answered also in: beyond_edge_ms, kPageJs BEYOND = max(cutoffs) and r.ms >= BEYOND
- squeeze-9: What rating label does a backend row get (Free/Easy/Standard/Hard/Insane, SqOut variants)? | owner: BackendSqueeze::summarystr | answered also in: timing_tiers (a second hardness ladder for the same SqOut note), report footer claim "tiers match Hydra's squeeze ratings"
- squeeze-10: Is a squeeze earned (difficulty >= 0) or already free? | owner: none | answered also in: rate_activation (difficulty() >= 0), squeeze_sentences (t >= 0 for SqOut, t <= 0 for SqIn), BackendSqueeze::summarystr ("Free SqOut" only at offset >= W)
- squeeze-11: Which frontend direction (early or late scale) governs a backend row or a phrase note? | owner: rate_activation | answered also in: rate_activation twice (offset-sign + squeezed_out ladder for rows, kind + difficulty-sign for notes)
- squeeze-12: Which SP end (pre or post) governs a squeeze? | owner: rate_activation | answered also in: frontend_transfer_scales comment ("pre governs SqIn feasibility"), Activation transfer_pre comment in model.h ("pre governs the SqIn/SqOut lines")
- squeeze-13: What is the transfer scale between two ticks? | owner: transfer_scale_between | answered also in: tests recompute bpm ratios
- squeeze-14: Where did the SP end sit before a SqIn phrase extended it (the pre tick)? | owner: none (frontend_transfer_scales re-derives it as D - 2 measures; the engine knows it exactly) | answered also in: none found
- squeeze-15: What is a row's effective ms and combined squeeze budget? | owner: effective_backend_ms + squeeze_budget_ms | answered also in: tests (2*gap/(1+r), gap/(1+r), frontend*r+note > gap)
- squeeze-16: Is a transfer scale material to a squeeze (worth warning)? | owner: transfer_is_material | answered also in: build_activations scale_warn adds shows() gate; squeeze_rating.h comment says "shown when material"
- squeeze-17: Does the scale move a figure enough to print an eff. value (> kTransferImpactMs)? | owner: none | answered also in: transfer_is_material (first clause), rate_activation backend loop, rate_activation note loop
- squeeze-18: Is a scale displayed at all, and do pre and post print the same (0.005)? | owner: build_activations (shows/same lambdas) | answered also in: none found
- squeeze-19: Is the scale line orange? | owner: build_activations scale_warn | answered also in: UserGuide.md line 124 (states a different rule)
- squeeze-20: Is a backend row counted with no squeeze (leeway)? | owner: counted_without_squeeze | answered also in: summarystr, rate_activation x2, squeeze_sentences, build_activations, replay_path, backend_row_value (all call the owner)
- squeeze-21: Is this backend row the squeezed-out note? | owner: Activation::is_sqout_backend / engine sqout_tick | answered also in: sqout_position (Exact), build_activations (Exact/NoSqOut chosen from the flag)
- squeeze-22: Is this row past the squeezed-out note? | owner: sqout_position (After) | answered also in: rebuild (remove_if b.ticks() > sqout_tick), Activation::display_backends (is_beyond_sqout)
- squeeze-23: Which window phrases were squeezed out (Preview meter)? | owner: engine sqout_tick (ADR 0014) | answered also in: build_sp_meter_curve (set difference against collected_phrase_ticks; CONTEXT.md SP meter gauge entry states this rule)
- squeeze-24: What does a squeeze-out cost in points, and is it shown as a loss? | owner: backend_row_value | answered also in: squeeze_sentences (lost > 0 gate), build_activations ("(-N)" + warn gated on counted only)
- squeeze-25: Does the activation list a squeeze the frontend decides (cap_clamped)? | owner: rate_activation | answered also in: rate_activation per-row loop (same decision in the row ladder)
- squeeze-26: Which note is the frontend lever of a cap-clamped window? | owner: none (scales anchor on act_tick; clamp_tick only drives the warning) | answered also in: docs/cap-clamped-squeeze-frontend-anchor.md, UserGuide.md line 126
- squeeze-27: How many SqIns and SqOuts does a path have? | owner: summarize_path | answered also in: none found
- squeeze-28: Does a path pass a squeeze ms limit (inclusive)? | owner: Engine::passes_ms_filter | answered also in: query_matches (stored hardest_ms), parse_squeeze ("<" treated as "<=")
- squeeze-29: Which kind of squeeze produced the hardest ms (badge name)? | owner: activation_badge | answered also in: none found
- squeeze-30: Which backend rows are shown in the details table? | owner: Activation::display_backends | answered also in: build_activations backend_limit filter, path_codec stores display_backends
- squeeze-31: Where would the plain (no-collection) SP end be (act + 2*B measures)? | owner: engine (deact_tick is stored) | answered also in: paths_json nominal_deact_tick, tests that build deact_tick from plusmeasure
- squeeze-32: What timing edge does a squeeze sentence print (timing() = -offset)? | owner: SPSqueeze::timing | answered also in: SPSqueeze::description, squeeze_sentences

### store

- store-1: Is a stored result row current (Ready) or Stale? | owner: rank_row / Candidate::ready (record_store.cpp), built on StampRule::is_current and structure_is_current | answered also in: row_ready_sql + bind_ready_params (SQL twin), stale_reasons (re-composes the pieces), decode_path_node and rebuild_record (format check again at decode), RecordStore::reindex / fill_missing_stars (call rank_row)
- store-2: Why is a Stale row stale (other build/layout vs other rules), and how is that told to the user? | owner: stale_reasons | answered also in: details_panel render_headline + render_record_state (fixed text, both reasons always), library_table render_chips + render_table (fixed text), docs/UserGuide.md line 61, tools/replay.cpp cmd_dump (uses stale_build/stale_rules but says "written by Hydra <stamp>")
- store-3: Which of several candidate rows answers a chart+mode lookup? | owner: WinnerPicker / outranks | answered also in: has_record and analyzed_hashes (SQL existence test, no picker; relies on UNIQUE key)
- store-4: Has this chart already been analyzed under this exact key (batch skip)? | owner: analyzed_filter (has_record, analyzed_hashes) | answered also in: run_batch (consumer), get_record (Ready status)
- store-5: Where in the structure blob do the path format and the rules fingerprint sit (the 12-byte head)? | owner: flatten_record / rebuild_record (path_codec.cpp) | answered also in: kStructureHeadBytes + read_le offsets in layout_is_current and structure_is_current, row_ready_sql substr(structure,1,4)/substr(structure,5,8), delete_auto_results substr(structure,5,8) three times, tests poking structure[0..3] (test_store.cpp)
- store-6: How is an integer written/read little-endian in a stored blob? | owner: BinaryWriter / BinaryReader (serialize.cpp) | answered also in: read_le / write_le (record_store.cpp), write_u32_le / read_u32_le (dynamics_breakdown.cpp)
- store-7: What is one result's identity (chart, chart mode, SP cap, lens)? | owner: RecordKey::operator== / Lens::operator== / results UNIQUE constraint (create_result_tables) | answered also in: lens_match (SQL), write_row replace purge, reload_row (checks hyhash, chartmode, hyversion, sp_cap but not the lens)
- store-8: Which SP cap does a lookup ask for / a result file under? | owner: Settings::cap_query | answered also in: run_batch (CapQuery::at(settings.sp_cap) from AnalysisSettings), collect_dm_rows (always kCloneHeroSpCap), ReportOptions default, render_preview_panel (record.sp_cap blob copy, else kCloneHeroSpCap)
- store-9: How do the settings become a Lens (ms limit, score range, fill rule) and the matching search settings? | owner: Settings::lens / Lens::from (key) and Settings::to_analysis_settings (search) | answered also in: tools/replay.cpp settings_from, library_dialogs batch_settings_summary (display), path_view within_label (display), cli/report main (fill rule from engine_mode)
- store-10: Which fill rule does a database's rows hold / a lookup ask for, given the engine_mode stamp? | owner: none | answered also in: RecordStore::add_fill_rule_column, cli/batch.cpp main (guard + unstamped-means-ch11), cli/report.cpp main (ch10 stamp forces 1.0 lens), cli/fillcompare.cpp main (warnings), fill_report collect_fill_rows (old=1.0, new=1.1 hard-coded)
- store-11: What chart-mode string keys a result? | owner: Settings::chartmode_key (+ difficulty, effective_bass2x) | answered also in: library_dialogs batch_settings_summary (display spelling of the same parts)
- store-12: Which SP cap / ms limit did a stored record run under (stored in both the results columns and the structure blob)? | owner: results columns (sp_cap, ms_enabled, ms_value) are the key; blob copy written by flatten_record | answered also in: prepare_row (cross-check, ms only when lens has it on), path_view build_record_status (reads blob copy), render_preview_panel (blob copy), replay emit_dump (blob copy), for_each_blob BlobRow.sp_cap (column)
- store-13: Which fill rule does a decoded HydraRecord say it ran under? | owner: get_record (sets record.legacy_fills from the key) | answered also in: for_each_blob (decodes without setting it, stays false), pather analyze_chart (sets it at analysis)
- store-14: What is the fingerprint of a set of rules, and which fields does it cover? | owner: Rules::fingerprint / fixed_cap_text | answered also in: load_rules_file (the same field-name list and the same sqout_rule spellings, typed again), Rules struct field list
- store-15: How is hydra_rules.ini read and which values are legal? | owner: load_rules_file (+ to_int, to_double) | answered also in: none found
- store-16: How is hydra_settings.ini read and written, and which values are legal? | owner: Settings::load_file | answered also in: Settings::save_file (key list spelled again), load_rules_file (a second key=value INI parser with a different comment rule)
- store-17: What is a song's stored length, and which analysis wins when two differ? | owner: store::song_length_ms + upsert_song (latest non-null wins) | answered also in: preview_view build_preview_scene (scene.song_length_ms = last scene note), SongLengthJob::run (fills a missing one with the current difficulty)
- store-18: How is the tempo map stored and turned back into a SongTiming? | owner: encode_tempomap / decode_tempomap | answered also in: none found (Song::build_timing uses the same constructor)
- store-19: What bytes make up a path node and the structure tree (encode/decode symmetry)? | owner: write_activation/read_activation, write_root_totals/read_root_totals, write_tree_entry/read_tree_entry | answered also in: none found
- store-20: Is a stored dynamics count current? | owner: kDynamicsCountStamp.is_current in RecordStore::get_dynamics; kDynamicsBlobStamp.is_current in decode_dynamics | answered also in: none found
- store-21: Which rows does a write delete, and does a row made under other rules survive a later write? | owner: RecordStore::write_row (purge 1 and purge 2) | answered also in: docs/adr/0014 ("It is not deleted"), docs/UserGuide.md line 61, plan 2026-09-24 user decision 15
- store-22: What summary columns does a record produce? | owner: summarize_record / summarize_path | answered also in: reindex, fill_missing_stars, report collect_rows (all call the owner)
- store-23: Which number names the current record/path format in comments and docs? | owner: kPathFormatStamp (6) | answered also in: path_codec.cpp comment "record format v7", core/model.h "blob v6, path structure v4", ADR 0017 "record format v7", ADR 0014 "kRowReadySql"
- store-24: What is a path node's content identity? | owner: path_hash_bytes (MurmurHash3) | answered also in: none found
- store-25: Which stored rows are Auto's (to delete once)? | owner: Rules::retired_auto_fingerprint + delete_auto_results | answered also in: none found
- store-26: Which search settings does a Settings run (engine side of the key)? | owner: Settings::to_analysis_settings | answered also in: tools/replay.cpp cmd_dump (legacy override), cli/batch.cpp main (legacy from flag)
- store-27: Which columns did the schema 2 results table have (migration)? | owner: kSchema2ResultsColumns | answered also in: tests/test_store.cpp downgrade_to_schema2 (column list typed again)
- store-28: Does a "why stale"/batch guard message describe the current storage rules (rule per result)? | owner: none | answered also in: cli/batch.cpp main (two messages: "not tagged as legacy", "rule is not stored on each result"), tools/replay.cpp usage ("Every stored row is a 1.1 result")

### sweep-core1

- sweep-core1-1: How many dynamic (ghost + accent) notes does a count hold, and does it have any? | owner: DynamicsCounts::has_dynamics (src/app/dynamics_breakdown.h, only the >0 test) | answered also in: render_dynamics_panel (src/ui/dynamics_tab.cpp, `played.ghost + played.accent` for "Dynamic notes")
- sweep-core1-2: Which kick rows add up to a kick total, and is Kick + 2x kick summed once? | owner: DynamicsBreakdown::kicks_total | answered also in: DynamicsBreakdown::played_total (re-adds the Kick and Kick2x rows field by field instead of calling kicks_total)
- sweep-core1-3: How is ASCII text lowercased, compared case-blind, trimmed, and which bytes count as whitespace? | owner: core/strutil (to_lower_ascii, trim/kSpace, ends_with_ci) per plan 2026-09-26 Task 16 | answered also in: library_query.cpp ascii_lower/iequals_ascii/starts_with_ci/is_ascii_space, song.cpp difficulty_from_name (std::tolower compare), report.cpp plain (std::tolower over "color"), analysis.cpp read_song_ini_keys (manual " \t" trims), model.cpp Chord::from_code (inline A-Z test)
- sweep-core1-4: Which activations does a path have, in what order (own list, then the shared variant tail), and does it have any? | owner: Path::walk_activations / ActivationWalk (src/core/model.h) | answered also in: Path::all_activations, Path::has_activations (src/core/model.cpp)
- sweep-core1-5: Does a library search term apply to a given field? | owner: term_matches (src/app/library_query.cpp) | answered also in: match_spans (own field gate)
- sweep-core1-6: Which folder and file name does each report page use? | owner: reports_dir / report_html_path / dm_report_html_path (src/app/report_files.cpp) | answered also in: src/cli/report.cpp default out "hydra_paths.html", tests/ui/uitest_harness.cpp literals
- sweep-core1-7: How is a UTF-8 path turned into a Windows wide path, opened, or tested for existence? | owner: core/winstr (utf8_to_wide, wide_to_utf8, fopen_utf8, file_exists_utf8) | answered also in: report_files.cpp report_file_exists (GetFileAttributesW), tests/test_srb.cpp (own WideCharToMultiByte)
- sweep-core1-8: Which text values mean "true" for a boolean option? | owner: none | answered also in: tools/replay.cpp flag_bool ("1"/"true"/"yes"), Settings::load_file (=="1" only)
- sweep-core1-9: How is a squeeze kind named or symbolised in text (+/-, SqIn/SqOut)? | owner: SPSqueeze::symbol / SPSqueeze::type_name | answered also in: tools/replay_json.cpp reads "SqOut" by literal; path_view.cpp "squeeze in"/"squeeze out" wording
- sweep-core1-10: How is text folded/lowercased for a search box match? | owner: fold_into (library search) | answered also in: report.cpp / dm_report.cpp / fill_report.cpp / html_page.cpp page JS (toLowerCase().includes over joined fields, no accent folding)

### sweep-media1

- sweep-media1-1: Which audio container are these bytes (magic-byte rule: RIFF+WAVE, fLaC, OggS+codec tag, ID3, MP3 sync)? | owner: audio::sniff_format (src/audio/decode.cpp) | answered also in: app::looks_like_audio (src/app/preview_source.cpp; RIFF without WAVE and OggS without a codec tag count as audio), is_audio_filename (by extension, loose folders and .sng entries)
- sweep-media1-2: At what rate does Opus decode, and how big is its largest packet? | owner: none | answered also in: decode_ogg_opus (out.sample_rate = 48000, opus_decoder_create(48000), kMaxFrame 5760 typed as a literal)
- sweep-media1-3: How many sample frames does a decoded buffer hold? | owner: DecodedAudio::frames | answered also in: none found where I looked (Playhead ctor and convert_stem call it)
- sweep-media1-4: How are stems combined into one mix (convert to one format, sum in order, zero-extend the shorter)? | owner: add_into + convert_stem (src/audio/mixer.cpp) | answered also in: none (mix_stems and decode_and_mix both call them; mix_stems is test-only)
- sweep-media1-5: Is an undecodable stem skipped or fatal? | owner: decode_and_mix (CONTEXT.md "Mixer") | answered also in: none found where I looked
- sweep-media1-6: What playhead positions are legal (clamp to [0, length])? | owner: Playhead::seek_frames | answered also in: PreviewTransport::seek_ms (own clamp to [0, length_ms_] on the song length, not the audio length)
- sweep-media1-7: Is the audio playing (and does it stop at the end)? | owner: PreviewTransport clock (CONTEXT.md "Transport": clock is the master) | answered also in: Playhead::read_frames (own playing_ flag, auto-pauses at the end of audio; PreviewTransport::seek_ms never re-plays it)
- sweep-media1-8: What is the lowest legal output gain? | owner: none | answered also in: Playhead::set_gain (gain < 0 -> 0), PreviewTransport::set_gain (same clamp typed again)
- sweep-media1-9: Which leaderboard scores count as played at base speed (speed 100, missing speed = 100)? | owner: none | answered also in: DmScore::speed default 100, parse_score jint(entry,"speed",100), collect_dm_rows (pct only when s.speed == 100, status "above optimal" ignores speed)
- sweep-media1-10: How is a leaderboard entry's charter list written? | owner: join_charters (", ") | answered also in: none found where I looked
- sweep-media1-11: Which leaderboard rows are kept (non-empty id / identifier; scores + unknown_scores)? | owner: parse_users_json / parse_scores_json | answered also in: none found where I looked
- sweep-media1-13: What camera and projection draw the highway? | owner: make_camera | answered also in: none (project_to_image and PreviewRenderer::render call it)
- sweep-media1-14: How big is the track rectangle, what aspect does the camera get, and where does it sit in the image? | owner: track_height (height only) | answered also in: project_to_image (aspect w/th, y offset h - th), PreviewRenderer::render (aspect width/track_h, viewport, fade rect -1 + 2*th/h), PreviewRenderer::resize (max(1) clamp also inside track_height)
- sweep-media1-15: What time sits at the far end of the highway (now + speed * secs_future)? | owner: none | answered also in: time_to_z, z_to_time, build_highway_draws far_t (three copies)
- sweep-media1-16: Where is a lane's x extent (note area / 4)? | owner: pad_x | answered also in: build_highway_draws kick spans x_left..x_right inline (different fact: full width)
- sweep-media1-17: Where is the gem light anchored? | owner: light_for (box top centre, max y) | answered also in: preview_config.h Gems::light comment ("relative to the gem's bottom centre")
- sweep-media1-18: Is a span on just after an instant, given its Toggle value? | owner: TrackState::make_toggle_bounds (Start/Restart/On) | answered also in: highway_draw.cpp toggle_on (not Empty and not End) used for gem overdrive
- sweep-media1-19: Which drawn notes are SP-phrase notes (energy texture)? | owner: parser phrase ranges -> build_preview_scene sp_phrases | answered also in: build_track_state (interval start_ms .. end_tick + 0.5 tick) read by build_highway_draws via toggle_on(overdrive)
- sweep-media1-20: Has the note at the playhead been hit yet? | owner: none | answered also in: build_highway_draws gem fade (t < now is hit) and target glow (t >= now not lit), build_score_box (step.ms <= now counts as hit)
- sweep-media1-21: Which lane does the taken fill light on the highway? | owner: build_track_state (activation lane at the fill whose end_tick == act tick) + TrackState::synthesize fill_lane_pad | answered also in: build_highway_draws (re-picks pad from window instants, falls back to the first pad anywhere in the window; merged touching spans share one pad)
- sweep-media1-22: Which colour shows Star Power running in the Preview? | owner: none | answered also in: 3d-config.json hydra.sp_active_color #6cf7c6 x 0.2 (highway floor), preview_tab IM_COL32(255,204,51) (drain box, gauge, "SP" label; display-34)
- sweep-media1-23: In what order is a polygon split into triangles (fan from the first corner)? | owner: load_obj | answered also in: push_quad (typed again, comment says it matches load_obj)
- sweep-media1-24: Where are the highway's outer edges on screen at a row? | owner: highway_span_at | answered also in: none (bottom_left_room wraps)
- sweep-media1-25: What scale range may the Preview text boxes take? | owner: overlay_scale with kOverlayMinScale 0.6 (plan 2026-09-27-preview-sp-drain-box.md line 477) | answered also in: none found where I looked
- sweep-media1-26: Where can a line break fall in a box's text (kept last N words)? | owner: wrap_words + tail_start | answered also in: widest_word (own word walk and tail trim to state the narrowest wrap)
- sweep-media1-27: How is a too-long label cut with an ellipsis? | owner: ellipsize | answered also in: none found where I looked
- sweep-media1-28: How is a config colour string read, and what is the fallback? | owner: parse_hex_color (magenta) | answered also in: none found where I looked
- sweep-media1-29: What are the Preview's look numbers (camera, highway, colours, timings)? | owner: assets/preview/3d-config.json (docs/adr/0008) | answered also in: PreviewConfig member defaults in src/render/preview_config.h (full second copy; unit tests that build PreviewConfig{} read it instead of the file)
- sweep-media1-30: Which texture row does v = 0 sample (bottom, as Onyx's GL upload)? | owner: PreviewRenderer::load_texture | answered also in: none found where I looked
- sweep-media1-31: What specular colour and shininess does every object use? | owner: draw_command literals (0.5, 32; Onyx hard-coded) | answered also in: assets/preview/shaders/object.hlsl not checked
- sweep-media1-32: Which instants does one frame draw (near < t < far)? | owner: TrackState::window | answered also in: build_highway_draws glow loop (re-tests t <= near_t, redundant)

### sweep-probe1

- sweep-probe1-1: Where does each Clone Hero DrumsEngine field live (byte offset) and how is it read/scaled (total window, back/front window, song clock, score, hit time, flags, note count)? | owner: tools/ch_probe/constants.py OFF_* + engine.py EngineModel readers | answered also in: experiments/live.py decode_snapshot (second decoder of 5 fields, window x1000), experiments/find_engine.py main (literals 0x20/0x30/0x38/0x8C/0x198), experiments/find_clock3.py main (literal 0x20), experiments/poll_windows.py main (literals 0x20/0x30), experiments/hit_detect.py (own OFF_TOTAL_WINDOW/OFF_SCORE, plus OFF_TIME_A 0x28 and OFF_HITS 0xb0 that are not in constants.py), experiments/watch_window.py main (raw back/front reads), experiments/walk_edges.py main (raw OFF_SONG_CLOCK read)
- sweep-probe1-2: Is the engine in precision mode (flags dword & 0x1000)? | owner: EngineModel.precision_mode | answered also in: live.Snapshot.precision, find_engine.main (inline test)
- sweep-probe1-3: Which engine-shaped heap object is the live one? | owner: engine_finder.find_live_engine (total window >= 0.001, clock moved > 1e-6 over 0.12 s) | answered also in: find_clock3.main (window changed > 0.0001 s in 0.2 s and w1 > 0.001), find_engine.main (tw > 0.001, all kept), hit_detect.first_engine_with_window (first heap hit with tw > 0.001), poll_windows.main (heap_hits[0], no filter), play_chart.main (find_live_engine but normal pattern only)
- sweep-probe1-4: Which scan matches are GameAssembly.dll's own .rdata copies (module_base .. +0x4000000)? | owner: engine_finder.MODULE_SPAN | answered also in: find_clock3.find_all_engines, find_engine.main, hit_detect.first_engine_with_window, poll_windows.main (all literal 0x4000000)
- sweep-probe1-5: Which byte pattern marks an engine object (back/front constants at +0x30/+0x38; precision pair in both orders)? | owner: engine_finder.normal_pattern / all_patterns / hits_in_region | answered also in: find_clock3.find_all_engines, find_engine.main, hit_detect.first_engine_with_window, poll_windows.main (each re-reads the normal pair inline; normal mode only)
- sweep-probe1-6: Did a pressed input count as a hit? | owner: none | answered also in: active_probe.drive_inputs (score rose, read SETTLE_MS 250 after the note), walk_edges.main (score rose, SETTLE_MS 250), play_chart.main (score rose, read 5 ms after the press), watch_window.hit_time_report (counts score rises), hit_detect.main (notes-hit counter +0xb0 rose, read 15 ms after release)
- sweep-probe1-7: What offset did a hit land at (engine +0x2e0 minus note if it changed, else estimated send time)? | owner: walk_edges.Row.measured_ms | answered also in: active_probe.drive_inputs (inline copy), hit_detect.main (+0x100 minus +0x28 instead)
- sweep-probe1-8: What is the song time between game frames (SongClock estimate)? | owner: walk_edges.SongClock | answered also in: play_chart.main (raw clock, fires 2 ms early instead)
- sweep-probe1-9: Has the song quit, paused or seeked (clock jumped back > 1 s, or frozen too long)? | owner: none | answered also in: active_probe.drive_inputs.wait_until (frozen 5.0 s, exact !=), walk_edges.main.wait_until (5.0 s, !=), watch_window.main (STALL_S 8.0), play_chart.main (5.0 s with 0.001 s epsilon; jump-back re-syncs instead of stopping)
- sweep-probe1-10: How does a runner wait until the song clock reaches a time? | owner: InputDriver.schedule_hit | answered also in: active_probe.drive_inputs.wait_until and walk_edges.main.wait_until (identical loops: sleep min(ahead-0.03,0.5) when ahead > 0.04), play_chart.main (0.05/0.04/0.01 variant)
- sweep-probe1-11: Which notes are already past when a runner starts mid-song? | owner: none | answered also in: walk_edges.main (skip time_ms < clock+150 ms), active_probe.drive_inputs (keep first_ms > clock+150 ms), play_chart.main (skip time_s < clock-0.05 s)
- sweep-probe1-12: Does the engine clamp the stored window (clamp verdict)? | owner: experiments/analysis.clamp_verdict (tolerance 1.0 ms, decisive 0.8) | answered also in: poll_windows.main (inline: w_max > 2*back + 0.5 ms means no clamp), watch_window.window_report (cap check against CAP_EXPECT_MS within 0.01)
- sweep-probe1-13: What is Clone Hero's capped total window, and from which gap does it apply (171.43 ms from 170 ms)? | owner: none in code (docs/superpowers/plans/2026-09-25-hit-window-testing.md text) | answered also in: watch_window CAP_EXPECT_MS/CAP_CHECK_FROM_GAP_MS, probe_songs CAP_GAPS_MS + docstring, passive_probe.run_passive_probe (whole-window edge 2*85 = 170), poll_windows.main (2*back + 0.5 = 170.5)
- sweep-probe1-14: What are CH's per-side window constants (85 / 37.5 normal, 40 / 25 precision)? | owner: constants.EXPECT_NORMAL_BACK_S/FRONT_S | answered also in: constants.EXPECT_NORMAL_BACK_MS (same 85 in ms, parallel constant), EXPECT_PRECISION_BACK_MS, src/core/model.h kDefaultHitWindowMs = 85.0 (see tempo-24)
- sweep-probe1-15: Do the live window constants match the build the addresses came from? | owner: process.check_normal_constants (via Process.verify_targets) | answered also in: none (milestone1/run_* call verify_targets)
- sweep-probe1-16: How are little-endian double/u32/u64 values decoded from raw bytes? | owner: process.decode_double/decode_u32/decode_u64 | answered also in: debugger.decode_xmm0_double, live.decode_snapshot dbl/u32, passive_probe.PassiveCollector.on_formula_entry ('<Q'), find_engine.main, find_clock3.main
- sweep-probe1-17: What window does CH's formula predict for a spacing (normal and precision shapes)? | owner: analysis.predicted_window_normal | answered also in: analysis.predicted_window_precision (same pre-scale and inner term written again)
- sweep-probe1-18: Where is the hit/miss edge for a set of (delta, hit) samples? | owner: analysis.find_window_edge | answered also in: walk_edges.summarize (edge = between widest hit and narrowest miss; reports no edge when they overlap)
- sweep-probe1-19: How is a probe .chart written (header, TS/B encoding, ExpertDrums line, kick note number)? | owner: probe_chart._song_section/_sync_track_section/_expert_drums_section | answered also in: probe_songs.chart_text (own header, B = int(BPM*1000) truncates instead of rounds, own KICK = 0)
- sweep-probe1-20: How is a playable probe song folder written (notes.chart, song.ini, silent song.ogg)? | owner: probe_songs.write_song | answered also in: active_probe.write_probe_song
- sweep-probe1-21: Where are the probe songs installed? | owner: probe_songs.DEFAULT_OUT | answered also in: experiments/live.py PROBE_ROOT (same literal path)
- sweep-probe1-22: How do runners find and focus the Clone Hero window (FindWindowW title "Clone Hero")? | owner: none | answered also in: active_probe.find_game_window, walk_edges.main, play_chart.main, pad_flash_test.main
- sweep-probe1-23: Which virtual key plays each drum lane? | owner: input_driver.DEFAULT_BINDINGS / Lane | answered also in: hit_detect.main (literal lanes 0..4, 2X kick vk 0x4F not in the table)
- sweep-probe1-24: How is the debugger attached safely (kill-on-exit off) and its event loop pumped? | owner: debugger.Debugger.attach/run/_handle_exception/stop | answered also in: experiments/test_attach.main (own DebugActiveProcess + pump; never turns kill-on-exit off)
- sweep-probe1-25: How is process memory read/written through kernel32? | owner: process._make_reader/_make_writer | answered also in: debugger._Win32 + Debugger._raw_read/_raw_write (second binding set; docstring says on purpose, no ADR)
- sweep-probe1-27: How long after a note does a runner wait before reading the result? | owner: none | answered also in: walk_edges.SETTLE_MS = 250, active_probe.SETTLE_MS = 250 (parallel constants), play_chart.main (5 ms)
- sweep-probe1-29: How long is a probe song (last note + SILENCE_MS)? | owner: probe_songs.write_song | answered also in: probe_songs.main (recomputes for print), active_probe.write_probe_song
- sweep-probe1-30: What signed ms value is in an OCR'd "Accuracy: X ms" string? | owner: ocr.parse_accuracy_text | answered also in: none
- sweep-probe1-31: Did the window field change (epsilon for a "new value")? | owner: none | answered also in: watch_window.changes and watch_window.main (1e-6 ms), poll_windows.main (0.001 ms), hit_detect.main and find_clock3.main (0.0001 s = 0.1 ms), find_engine.main (round to 4 dp), milestone2.main (round to 2 dp)
- sweep-probe1-32: What are the probe chart defaults (resolution 192, 120 BPM, note 0)? | owner: none (restated) | answered also in: probe_chart.probe_note_ticks, build_probe_chart_text, generate_probe_chart, interfaces.generate_probe_chart
- sweep-probe1-33: How is a chord pressed (all keys down, hold, all up)? | owner: InputDriver.press_chord (3 ms hold) | answered also in: hit_detect.main (own loop, 5 ms hold)
- sweep-probe1-34: Where is the breakpoint after an int3 trap (rip - 1)? | owner: debugger.adjust_rip_after_int3 | answered also in: none
- sweep-probe1-35: How is the live engine pointer captured at its constructor (rcx)? | owner: EngineModel.capture_object | answered also in: none
- sweep-probe1-36: Which .rdata constants make up the probe's constant set, under which keys? | owner: EngineModel.constants | answered also in: milestone1.main (re-lists the set by hand), analysis.normal_formula_constants (reader)
- sweep-probe1-37: What song.ini does a probe song carry (delay 0, pro_drums False)? | owner: probe_songs.song_ini | answered also in: none
- sweep-probe1-38: How does an RVA become a live address (module_base + rva)? | owner: process.Process.resolve | answered also in: none found

### sweep-probe2

- sweep-probe2-1: At what byte offset off the DrumsEngine object does each engine field sit (total window 0x20, back 0x30, front 0x38, song clock 0x100, score 0x94, hit time 0x2E0, flags 0x198, note count 0x8C)? | owner: tools/ch_probe/constants.py OFF_* (read through engine.EngineModel) | answered also in: experiments/live.py decode_snapshot (second reader of the same fields), experiments/watch_window.py main (reads OFF_BACK/FRONT_WINDOW directly), engine_finder.hits_in_region/find_live_engine, LITERAL copies in experiments/find_engine.py (0x20/0x30/0x38/0x8C/0x198), experiments/find_clock3.py (0x20), experiments/poll_windows.py (0x20/0x30), experiments/hit_detect.py (own OFF_TOTAL_WINDOW/OFF_SCORE); tests pin 0x2E0/0x100 and use 0x30
- sweep-probe2-2: Is the engine in precision mode (flags dword & 0x1000)? | owner: engine.EngineModel.precision_mode (bit from constants.PRECISION_MODE_BIT) | answered also in: experiments/live.py Snapshot.precision (separate flags & bit test)
- sweep-probe2-3: Do the live-read normal window constants match the build we expect (0.085 s / 0.0375 s within 0.5 ms)? | owner: process.check_normal_constants with constants.EXPECT_NORMAL_*_S and CONST_MATCH_TOLERANCE_MS | answered also in: experiments/milestone1.py (prints against EXPECT_*), tests/test_process.py literals
- sweep-probe2-4: How are raw little-endian bytes decoded into a double / u32 / u64? | owner: process.decode_double / decode_u32 / decode_u64 | answered also in: debugger.decode_xmm0_double (struct.unpack "<d" on first 8 bytes), experiments/live.py decode_snapshot (unpack_from "<d"/"<I"), experiments/passive_probe.py on_formula_entry (unpack "<Q" return address)
- sweep-probe2-5: Where is the breakpoint after an int3 trap (rip - 1)? | owner: debugger.adjust_rip_after_int3 | answered also in: none found (Debugger._rewind_rip and _on_our_breakpoint call it)
- sweep-probe2-6: Which byte does a memory read return at a planted breakpoint, and when is the original byte saved/restored? | owner: debugger.BreakpointTable + mask_breakpoints | answered also in: none found
- sweep-probe2-7: Which memory-scan hit is the live engine object (match minus 0x30, outside module_base..+0x4000000, total window > 0.001, clock moved > 1e-6 across 0.12 s)? | owner: engine_finder.hits_in_region + find_live_engine (MODULE_SPAN) | answered also in: experiments/find_engine.py (module_base + 0x4000000, tw > 0.001), experiments/find_clock3.py (0x4000000, w1 > 0.001), experiments/poll_windows.py (0x4000000)
- sweep-probe2-8: Which byte patterns identify an engine object (normal pair; precision pair both orders)? | owner: engine_finder.normal_pattern / all_patterns | answered also in: not checked in experiments
- sweep-probe2-9: Which keyboard key plays each input lane, and what are the lane numbers and short names? | owner: input_driver.Lane / DEFAULT_BINDINGS / LANE_NAMES | answered also in: experiments/play_chart.py (imports LANE_NAMES; guard test)
- sweep-probe2-10: When does a scheduled hit fire against the song clock (poll until clock >= target, else timeout)? | owner: input_driver.InputDriver.schedule_hit | answered also in: experiments/walk_edges.py main.wait_until (sleep(min(ahead - 0.03, 0.5))), experiments/active_probe.py (sleep(min(ahead - 0.03, 0.5))), experiments/play_chart.py (sleep(min(ahead - 0.04, 0.5)))
- sweep-probe2-11: How is a chord pressed (all keys down, hold 3 ms, all up; unbound lanes skipped)? | owner: input_driver.InputDriver.press_chord | answered also in: not checked
- sweep-probe2-12: How is on-screen "Accuracy: X ms" text parsed into signed ms? | owner: ocr.parse_accuracy_text | answered also in: not found where I looked (tools/, src/)
- sweep-probe2-13: How is a probe .chart file written ([Song] header, [SyncTrack] "0 = TS 4" + "0 = B bpm*1000", [ExpertDrums] "tick = N note 0")? | owner: none (two writers) | answered also in: probe_chart._song_section/_sync_track_section/_expert_drums_section (int(round(bpm*1000))) and probe_songs.chart_text (int(BPM*1000), adds MusicStream and [Events])
- sweep-probe2-14: Which .chart note number is the probe's kick (0)? | owner: constants.PROBE_CHART_NOTE_KICK | answered also in: probe_songs.KICK = 0 (parallel constant)
- sweep-probe2-15: Where do the probe chart's notes sit (2-bar lead-in, 4-bar pad, pair gap ms_to_ticks, 4 beats a bar)? | owner: probe_chart.probe_note_ticks | answered also in: tests/test_probe_chart.py and tests/test_runners.py literal 3840
- sweep-probe2-16: What are a probe song's note times, blocks and gaps before/after (manifest)? | owner: probe_songs.Timeline / window_map / edge_walk / manifest | answered also in: none found
- sweep-probe2-17: Which folder holds the probe songs (C:\Clone Hero\songs\Hydra Probe)? | owner: none | answered also in: probe_songs.DEFAULT_OUT and experiments/live.py PROBE_ROOT (same literal twice); "Hydra Probe - " name prefix in probe_songs.write_song and experiments/active_probe.write_probe_song
- sweep-probe2-18: What value is the total window's cap and what tolerance calls a reading "at the cap"? | owner: none | answered also in: experiments/watch_window.py CAP_EXPECT_MS = 171.43 (tol 0.01, gaps >= 170), experiments/passive_probe.py run_passive_probe "whole window" cap = 2 * EXPECT_NORMAL_BACK_MS = 170 judged by analysis.clamp_verdict (tol 1.0)
- sweep-probe2-19: What window value was in force at a moment (last change at or before t)? | owner: watch_window.value_at | answered also in: not checked
- sweep-probe2-20: Which test block does a note time belong to (spans split in the silence)? | owner: watch_window.block_spans | answered also in: not checked
- sweep-probe2-21: Which offsets does the edge walk try, in which order, and how is a range typed? | owner: walk_edges.build_schedule / parse_range | answered also in: none found
- sweep-probe2-22: What song time is it now, filling in between the game's clock writes (fresh 2 ms, max fill 50 ms)? | owner: walk_edges.SongClock | answered also in: engine.EngineModel.song_clock (raw only); play_chart not checked
- sweep-probe2-23: Where are the late and early hit edges bracketed from walk rows? | owner: walk_edges.summarize | answered also in: experiments/analysis.py edge detection (not compared)
- sweep-probe2-24: How is a formula call's (spacing, raw, stored) row assembled? | owner: passive_probe.PassiveCollector | answered also in: none found
- sweep-probe2-25: What inputs does the active probe plan (one pair per spacing x offset; note ticks used as ms)? | owner: active_probe.plan_inputs (via probe_chart.probe_note_ticks) | answered also in: none found
- sweep-probe2-26: Which hit-check calls are counted (only while an input is in flight)? | owner: active_probe.ActiveCollector.on_hit_check | answered also in: none found
- sweep-probe2-27: In what order does a probe runner verify, attach, set breakpoints, run and detach? | owner: passive_probe.run_passive_probe / active_probe.run_active_probe | answered also in: none found
- sweep-probe2-28: Does attach turn kill-on-exit off, and what happens to events queued after stop()? | owner: debugger.Debugger.attach / _handle_exception | answered also in: none found
- sweep-probe2-29: How are engine fields in seconds turned into ms? | owner: none (inline * 1000) | answered also in: live.decode_snapshot (window), passive_probe.PassiveCollector (3 places), watch_window.main (back/front, clock, hit time), walk_edges.main
- sweep-probe2-30: How does an RVA become a live address (module_base + rva)? | owner: process.Process.resolve | answered also in: test fakes (test_engine FakeProcess.resolve, test_engine_finder FakeProc.resolve, test_runners FakeProcess.resolve)
- sweep-probe2-31: How is the live engine object pointer obtained (ctor breakpoint rcx, or a scanned pointer)? | owner: engine.EngineModel.capture_object / use_object | answered also in: none found
- sweep-probe2-32: What keys does EngineModel.constants() return? | owner: engine.EngineModel.constants (+ constants.CONST_KEY_*) | answered also in: "normal_back"/"precision_back"/"hitcheck_threshold" spelled literally in engine.py while formula keys use CONST_KEY_PREFIX_*; tests/test_engine.py spells "normal_"/"precision_" literally
- sweep-probe2-34: Which +0x2e0 changes line up with which notes (hit-time report)? | owner: watch_window.hit_time_report | answered also in: none found
- sweep-probe2-35: What does the window report say per block and is a run at the cap? | owner: watch_window.window_report | answered also in: none found

### sweep-tests1

- sweep-tests1-1: Where do an activation row's measure, bars and badge sit (px)? | owner: ui::activation_row_layout (src/ui/activation_row_layout.h, kRowMeasureX/kRowMinBarsX/kRowBarsGap/kRowBadgeRight/kRowBadgePad) | answered also in: tests/test_activation_row_layout.cpp (re-types 104/200 and recomputes 2*(104+w+gap), row_w - right)
- sweep-tests1-2: Is a saved window rectangle safe to reopen (title bar reachable on a known monitor)? | owner: ui::placement_on_screen (src/ui/app_shell) | answered also in: none found where I looked (src/ui)
- sweep-tests1-3: What UI scale does a monitor DPI give (dpi/96, 0 -> 1)? | owner: ui::ui_scale_for_dpi (src/ui/app_shell.cpp:163) | answered also in: none found (grep 96.0f and /96 over src, tools, tests)
- sweep-tests1-4: How are the [Hydra][Window] and [Hydra][Layout] blocks of hydra_ui.ini written and read, and which LibraryShare/LibraryHidden values are legal? | owner: format_window_placement/parse_window_placement_line, format_layout/parse_layout_line (src/ui/app_shell) | answered also in: none found where I looked
- sweep-tests1-5: How is a batch elapsed/remaining duration written (m:ss under an hour, h:mm:ss over, rounded, negative -> 0:00)? | owner: ui::format_duration (src/ui/library_dialogs.cpp:66) | answered also in: none; clock_str (preview_view.cpp:415) writes a song clock m:ss.sss, a different fact
- sweep-tests1-6: Is a byte stream audio, and which container/codec is it (magic bytes)? | owner: audio::sniff_format (src/audio/decode.cpp:49) | answered also in: app::looks_like_audio (src/app/preview_source.cpp:181, own magic list, no WAVE/Opus/Vorbis check), app::is_audio_filename (by extension)
- sweep-tests1-7: How are decoded stems converted to one rate/channel count and summed into the Preview mix? | owner: audio::mix_stems / decode_and_mix | answered also in: tests/test_audio_mixer.cpp reference_mix (test oracle re-implements the summation)
- sweep-tests1-8: What does the Preview playhead do when paused, at the end, on a seek, and with a gain (silence, auto-pause, clamp, gain clamp)? | owner: audio::Playhead (src/audio/player) | answered also in: none found where I looked
- sweep-tests1-10: Is a chord part of an SP phrase (drawn as an energy gem / overdrive)? | owner: parser (SongTimestamp flag_sp + sp_phrase_start on the phrase's end note) | answered also in: render build_track_state overdrive_ interval (start_ms .. ms_at_tick_f(end_tick + kSpanEndTicks)) read by build_highway_draws
- sweep-tests1-11: How is a scanned chart path written relative to the corpus root for testdata/scan_snapshot.json? | owner: tools/bench.cpp scan_mode (--dump-rel, lines 148-150) | answered also in: tests/test_analysis.cpp rel_of
- sweep-tests1-12: Does a rescan reuse a cached library row instead of reading the chart file (signature match)? | owner: app::discover_charts with store::ChartLibraryCache | answered also in: hydra_batch (reuses the same cache; test_cli "reuses the GUI's scan cache")
- sweep-tests1-13: Does the work pool hand every item exactly once and return after a cancel? | owner: app::run_work_pool (src/app/work_pool.h) | answered also in: none found
- sweep-tests1-14: Is a cancelled chart a result, a failure, or neither? | owner: app::run_batch (cancel through a throwing progress callback) | answered also in: not checked (ui/library_jobs BatchJob)
- sweep-tests1-15: How are style sizes, fonts and px() scaled for a UI scale (always from the unscaled style, sizes truncated)? | owner: ui::scaled_style / set_ui_scale / px | answered also in: tests/test_app_shell.cpp "set_ui_scale..." recomputes static_cast<int>(before*2)
- sweep-tests1-16: How often are the chart file and the path report file re-checked on disk? | owner: AppState::kFileCheckSeconds and AppState::kReportCheckSeconds (two separate 2.0 constants, src/ui/app_state.h:231,242) | answered also in: none
- sweep-tests1-17: When does a settings edit reach hydra_settings.ini (commit at once vs number-box flush)? | owner: AppState::commit_settings / edit_settings / flush_settings | answered also in: none found
- sweep-tests1-18: Is a record lookup reused from the parked cache when a number box steps back? | owner: AppState::edit_settings (parked lookups, dropped on select) | answered also in: none found
- sweep-tests1-19: Which library rows match a search word? | owner: library_query query_matches via AppState::set_search | answered also in: not checked
- sweep-tests1-20: How is a stem decoded to float PCM (int16 scaling, Opus at 48 kHz, file vs bytes stem identical)? | owner: audio::decode_audio / decode_stem | answered also in: none found
- sweep-tests1-21: What does an empty library tell the user to do next? | owner: ui::empty_library_message | answered also in: none found
- sweep-tests1-22: What are the default backend limit (50) and Preview volume (40)? | owner: app::Settings members (src/app/config.h:76,85) | answered also in: tests/test_config.cpp pins (test)
- sweep-tests1-23: What range may the Preview volume take (0..100)? | owner: none | answered also in: Settings::load_file (config.cpp:82, v >= 0 && v <= 100), preview_tab volume SliderInt (preview_tab.cpp:280, 0..100)
- sweep-tests1-24: How is a dmbot payload read (identifier lowercased, missing speed = 100, charter_refs joined with ", ", users without id dropped)? | owner: net::parse_scores_json / parse_users_json | answered also in: none found
- sweep-tests1-25: How does the leaderboard HTTP transport answer a cancel, a 200 and a 404? | owner: net http transport (fetch_users over WinHTTP) | answered also in: none found
- sweep-tests1-26: What label does a Dynamics row get (pro vs non-pro)? | owner: app::dynamics_row_label | answered also in: not checked (ui render_dynamics_panel)
- sweep-tests1-27: Highway geometry for the Preview (time->depth, lane x, camera, railing/target/gem boxes, ghost 70%, flash and target-glow fades) | owner: render/highway_draw.cpp + render/preview_config.h (Onyx port) | answered also in: tests/test_highway_draw.cpp re-types 0.1666666 (targets_secs_light) and half-tick edges 1.0005/1.5005

### sweep-tests2

- sweep-tests2-1: How many finished charts before the batch shows "about X left", and how is X computed (elapsed/completed*(total-completed))? | owner: ui::batch_eta_s with kEtaMinFinished (src/ui/library_jobs.cpp) | answered also in: library_jobs.cpp:221 (hides it while paused; caller gate, not a copy); test 1507 expects literal 3
- sweep-tests2-2: Where does a row with no score sort under the library's Best path sort (Ready-no-paths, then Stale, then Not analyzed, both directions)? | owner: unscored_rank (src/ui/library_model.cpp) | answered also in: none found where I looked (src/ui, src/app)
- sweep-tests2-3: How is library text folded for search (lowercase, accents, full-width, whitespace)? | owner: app::fold_for_search (src/app/library_query.cpp) | answered also in: none found where I looked (src/, tools/)
- sweep-tests2-4: Does a library row match the query words/phrases/field prefixes? | owner: app::query_matches + parse_library_query | answered also in: none found where I looked
- sweep-tests2-5: Which displayed bytes does the library highlight for a query? | owner: app::match_spans | answered also in: none found where I looked
- sweep-tests2-6: How are a .mid file's bytes read into timed events (VLQ deltas, running status through metas, sysex skip, track name = first 0x03 meta, 1,000,000-byte message cap)? | owner: hydra::MidiFile (src/parse/midi.cpp) | answered also in: tools/ch_probe/experiments/play_chart.py parse_midi + _read_vlq (own byte walker: name from first 200 bytes, no sysex handling, no running status in the tempo track)
- sweep-tests2-7: What is a drum note called on screen (cymbal/tom, ghost/accent, 2x kick)? | owner: ChordNote::str + Chord::rowstr (src/core/model.cpp) | answered also in: app::dynamics_row_label (src/app/dynamics_breakdown.cpp: "Yellow cymbal", "Green tom"/"Green", "2x kick", own vocabulary keyed on pro)
- sweep-tests2-8: How is a line of UI text cut to fit its width (where the cut lands, trailing spaces, the ellipsis)? | owner: render::ellipsize (src/render/overlay_layout.cpp; Preview "Showing" picker) | answered also in: ui::text_ellipsized (src/ui/widgets.h, ImGui::RenderTextEllipsis), draw_title_ellipsized (src/ui/library_table.cpp, own CalcTextSizeA cut) -- three cutters, only the first drops spaces before the ellipsis
- sweep-tests2-9: Where does the Preview highway sit in the image, and how much room is left beside it for the text boxes? | owner: render::highway_span_at / track_height / bottom_left_room / overlay_scale (src/render/overlay_layout.cpp) | answered also in: not found elsewhere where I looked (src/ui/preview_tab.cpp calls them)
- sweep-tests2-10: What song time does the Preview show while playing, paused, or after a seek? | owner: app::PreviewClock (src/app/preview_clock.h) | answered also in: PreviewTransport (owns a clock and pins/clamps time; see tempo-20/display-24 for its ms math) -- not compared further
- sweep-tests2-11: What are the Preview's Onyx 3D layout/colour values (camera, track, fades, gem fade, Hydra-only SP colour)? | owner: none single: PreviewConfig struct defaults (src/render/preview_config.h) AND assets/preview/3d-config.json carry the same numbers | answered also in: tests/test_preview_config.cpp check_is_onyx (third copy, the oracle)
- sweep-tests2-12: How is a "#rrggbb[aa]" colour string turned into a colour? | owner: render::parse_hex_color | answered also in: not found elsewhere where I looked (src/render, src/ui)
- sweep-tests2-13: Is a note part of an SP phrase? | owner: the parser's SP phrase marking on the chart (Song/SongTimestamp is_sp, read by build_preview_scene into scene.sp_phrases spans) | answered also in: tests/test_preview_golden.cpp "preview dump" (start_ms <= n.ms <= end_ms over spans, twice), tests/test_preview_renderer.cpp (none), render track_state span ends (end_tick + 0.5 tick, see tempo-23)
- sweep-tests2-14: Which files/streams are the song's audio (extension list, magic bytes, preview/art excluded)? | owner: app::is_audio_filename / looks_like_audio / find_loose_audio (src/app/preview_source.cpp) | answered also in: not found elsewhere where I looked (src/, tools/ grep for .ogg/.opus)

### sweep-tests3

- sweep-tests3-1: Which Path score field is which ReplayScore category (the six-field map and its order)? | owner: core/replay.cpp score_of (Path -> ReplayScore) + core/replay.h kReplayScoreFields (names/order) | answered also in: tests/test_preview_view.cpp priced_path (reverse map typed by hand), search/engine.cpp (op.score_* = p.sc[0..5], then path.score_* = op.score_*), store/path_codec.cpp read order, tests/test_replay.cpp "paths_json writes every field" (key list typed again)
- sweep-tests3-2: How far past the last note does the Preview beat grid run (two measures)? | owner: app/preview_view.cpp build_preview_scene (last_tick + 2 * tpm) | answered also in: not found where I looked (src/app/preview_view.cpp, src/render grep for "2 * tpm" not done)
- sweep-tests3-3: Which time signature does the time box show before any change, or with no song (4/4)? | owner: app/preview_view.cpp build_time_box | answered also in: not checked beyond preview_view.cpp
- sweep-tests3-4: Which notes does the parser flag as an SP phrase end, with which start tick, and does a stray SP note-off flag anything? | owner: parse/midi + parse/chart parsers (SongTimestamp::flag_sp, sp_phrase_start) | answered also in: none found (tests only)
- sweep-tests3-5: What value marks "no deact node / no sqout tick" in a dump JSON (-1)? | owner: tools/replay_json.cpp paths_json (writes -1) | answered also in: tools/replay_json.cpp windows_from_json (reads -1 as missing); fcvideo readers are outside this repo (not checked)
- sweep-tests3-6: How is a note's dynamic named in text? | owner: core/model.cpp dynamic_str ("none"/"ghost"/"accent") | answered also in: core/model.cpp ChordNote::str mods ("Ghost"/"Accent", capitalised), ui/dynamics_tab.cpp column titles "Ghost"/"Accent"
- sweep-tests3-7: How many records and charts does the path report count? | owner: app/report.cpp generate_report (records = rank==1 rows, charts = distinct hyhash) | answered also in: app/report.cpp kPageJs stats() 'Charts' tile (new Set over r.c, c assigned per hyhash in build_html, over the rows passed to stats)
- sweep-tests3-8: Where are report pages written (Documents\Hydra, else the database folder; a harness db override wins)? | owner: app reports_dir (report_files) | answered also in: not checked
- sweep-tests3-9: Do report colours meet WCAG contrast (relative luminance + ratio)? | owner: tests/test_report.cpp luminance/contrast (test-only) | answered also in: not found in src/ or tools/ (grep 0.03928 / 0.2126)
- sweep-tests3-11: How is a MIDI byte stream built for a test fixture (varlen, meta events, MTrk header)? | owner: tests/midi_util.h (testmidi::smf, track_name, text_event) | answered also in: tests/test_song.cpp put_varlen/put_meta/put_track (own multi-track writer)
- sweep-tests3-12: Which song is next/previous in the song panel (view order, no wrap, none outside the view)? | owner: ui AppState::select_relative / can_select_relative | answered also in: not checked
- sweep-tests3-13: When are two parsed songs equal? | owner: none | answered also in: tests/test_srb.cpp songs_equal only (not found elsewhere in tests/ or src/)
- sweep-tests3-14: How large may an inflated .srb stream be? | owner: parse/srb.h kSrbMaxStream / kSrbMaxMetadata | answered also in: not checked
- sweep-tests3-15: When the "every path" report is asked for, what marks it (sentinel vs label threshold)? | owner: app/report.h kEveryPathSentinel (1e9, what --all-paths asks for) | answered also in: app/report.h kEveryPathLabelThreshold (1e8) read by generate_report's "every path" subtitle test (max_paths > threshold)
- sweep-tests3-16: When two writes give a song a length, which one wins? | owner: store upsert_song (COALESCE(excluded, existing): latest non-null analysis wins) | answered also in: RecordStore::set_song_length (UPDATE ... WHERE length_ms IS NULL: existing wins)

### sweep-tests4

- sweep-tests4-1: How wide is the library beside the open song panel (share of the room, clamped to a 320 px library minimum and the kMinSongPanelW panel minimum)? | owner: render_library_and_panel (src/ui/library_view.cpp; min_library = px(320.0f) is an unnamed literal, kDefaultLibraryShare, kMinSongPanelW) | answered also in: tests/ui/uitest_details.cpp test_panel_split (opened = min(share*room, room - px(kMinSongPanelW)) without the 320 floor; then max(px(320.0f), 0.3*room) re-typed), tests/ui/uitest_library.cpp test_library_layout (px(321.0f)), tests/ui/uitest_harness.cpp reset_app (restores kDefaultLibraryShare)
- sweep-tests4-2: What is the WCAG contrast ratio of two colours (relative luminance and its sRGB linearisation threshold)? | owner: none in production | answered also in: tests/test_theme.cpp channel/luminance/contrast (threshold 0.04045), tests/test_report.cpp luminance/contrast (threshold 0.03928)
- sweep-tests4-3: How long does a news line in the status bar stay before it fades (6 s)? | owner: render_status_line (src/ui/library_toolbar.cpp, literal 6.0) | answered also in: tests/ui/uitest_batch_reports.cpp test_status_line (Yield(400) "over 6 s of 1/60 s frames"); plan 2026-09-27-ui-redesign.md describes 6 s as the old behaviour
- sweep-tests4-4: Which word names a stored record's status (Ready / Stale / Not analyzed)? | owner: library_table.cpp chip and row labels ("Not analyzed", "Stale", "Analyzed") | answered also in: tests/ui/uitest_harness.cpp dump_state ("current" / "stale" / "new")
- sweep-tests4-5: Which label does the song panel's analyze button carry ("Analyze this song" vs "Re-analyze")? | owner: src/ui/details_panel.cpp (status == NotAnalyzed) | answered also in: tests/ui/uitest_harness.cpp analyze_button_ref (the same rule re-typed)
- sweep-tests4-6: How is a string lowercased, trimmed or suffix-checked (ASCII only)? | owner: core/strutil (to_lower_ascii, trim, ends_with, ends_with_ci) | answered also in: src/app/user_messages.cpp anonymous-namespace ends_with (byte-identical copy, added e6d8159 one day after 682e924 made strutil the owner), src/parse/song.cpp difficulty_from_name (inline std::tolower compare), src/app/report.cpp plain (inline std::tolower against "color")
- sweep-tests4-7: Which column index is the library table's Best path column? | owner: library_table.cpp column setup | answered also in: tests/ui/uitest_library.cpp test_library_column_order (literal `const int best = 4`)
- sweep-tests4-8: Which registered GUI tests does a --test / --all name select? | owner: uitest::Harness::queue | answered also in: tests/ui/uitest_main.cpp run_parallel (same "script" skip and "all"-or-exact-name match written again)
- sweep-tests4-9: At what pixel size does ImGui draw the default text (FontSizeBase * FontScaleMain * FontScaleDpi)? | owner: Dear ImGui (style) | answered also in: tests/ui/uitest_paths.cpp text_w, tests/ui/uitest_preview.cpp test_preview_path_picker (twice in one function)
- sweep-tests4-10: What ImGui ID does an activation's backend table get ("##backends<n>_<w>_<w>_<w>")? | owner: src/ui/paths_tab.cpp (snprintf of the id) | answered also in: tests/ui/uitest_paths.cpp backend_table (rebuilds the same format string)
- sweep-tests4-11: How far along is a Preview load (progress fraction and label per step)? | owner: PreviewLoadJob::Progress::fraction / label (src/ui/preview_load_job.cpp) | answered also in: tests/ui/uitest_preview.cpp test_preview (pins the values; test only)
- sweep-tests4-12: Is an exception message the parser's no-notes text, which passes through to the user unchanged? | owner: parse/song.cpp no_notes_message | answered also in: src/app/user_messages.cpp is_no_notes_message (re-derives the pattern "No " ... " notes in this chart.")
- sweep-tests4-13: How is a headless D3D11 WARP device made, and a texture read back to CPU memory? | owner: tests/warp_util.h make_device / read_pixels | answered also in: tests/ui/uitest_harness.cpp Harness::init (own D3D11CreateDevice WARP call), capture_pixels (own staging-texture copy)
- sweep-tests4-14: What does the batch button say while a search is typed ("Analyze search (N)...")? | owner: src/ui/library_toolbar.cpp render_actions_row | answered also in: tests/ui/uitest_batch_reports.cpp batch_search and test_report_buttons (the label built twice in one file from group_thousands(library_match_count()))

### sweep-ui1

- sweep-ui1-1: How many charts did a batch skip because they already had a result? | owner: none (run_batch filters with analyzed_hashes but reports only the dispatched total) | answered also in: BatchJob::run (items_.size() - p.total), cli/batch.cpp main (scanitems.size() - p.total)
- sweep-ui1-2: How many charts did a batch analyze and store? | owner: none (run_batch calls on_result per stored chart) | answered also in: batch_counts (completed - failed), render_batch_strip tooltip (completed - failed), cli/batch.cpp main (++analyzed in on_result)
- sweep-ui1-3: What fraction of a done/total progress bar is filled when total is 0? | owner: progress_bar_counted (ui/widgets.h, 1.0) | answered also in: render_batch_strip (0.0)
- sweep-ui1-4: How is an elapsed or remaining batch duration written (m:ss / h:mm:ss, rounded)? | owner: format_duration | answered also in: cli/batch.cpp main ("%.1fs")
- sweep-ui1-5: Is a job's chart the chart the song panel shows (identity by notespath or md5)? | owner: none | answered also in: AppState::analyze_job_shown (notespath), AppState::relative_row (notespath), AppState::update_song_length (md5), library_table selected_path (notespath)
- sweep-ui1-6: Which SP cap and lens does the post-batch path report list? | owner: Settings::cap_query / Settings::lens | answered also in: AppState::update_background_jobs (current settings at batch end, not BatchJob's BatchRun)
- sweep-ui1-7: What UI scale does a monitor DPI give (dpi / 96)? | owner: ui_scale_for_dpi | answered also in: main.cpp ImGui_ImplWin32_GetDpiScaleForMonitor and ImGui_ImplWin32_GetDpiScaleForHwnd (backend's own division)
- sweep-ui1-8: Is a library/song-panel split share legal (0 < share < 1)? | owner: none | answered also in: parse_layout_line, remember_library_share
- sweep-ui1-9: Where do the bundled resources (fonts, icons) live? | owner: none | answered also in: setup_imgui (options.resource_dir or exe_dir() + resource), load_icons (exe_dir() + resource only)
- sweep-ui1-10: What point size are the UI fonts loaded at (18)? | owner: none | answered also in: setup_imgui (three 18.0f literals: main, mono, CJK merge)
- sweep-ui1-11: Is a saved window placement still grabbable on some monitor? | owner: placement_on_screen (kMinVisiblePx) | answered also in: none found where I looked (src/ui)
- sweep-ui1-12: When does the batch show a time-left estimate, and what is it? | owner: batch_eta_s (kEtaMinFinished) | answered also in: none found where I looked
- sweep-ui1-13: How long has a batch been running, pauses left out? | owner: BatchClock::elapsed_s | answered also in: cli/batch.cpp main (own steady_clock difference, no pause feature)
- sweep-ui1-14: What error text and plain message does a failed or cancelled job carry? | owner: ResultJobBase::run_guarded | answered also in: AnalyzeJob::start catch (re-types the tail), BatchJob::run load-failure branch
- sweep-ui1-15: Where do the measure, bars and badge sit on an activation row? | owner: activation_row_layout | answered also in: none found where I looked
- sweep-ui1-16: Which timeline labels fit without overlapping? | owner: spaced_labels | answered also in: none found where I looked
- sweep-ui1-19: How often does the library re-read summaries while a batch runs (1 s)? | owner: AppState::tick_library | answered also in: none found where I looked
- sweep-ui1-20: How often does the UI re-check that a file exists (2 s)? | owner: none | answered also in: AppState::kFileCheckSeconds (selected_file_ok), AppState::kReportCheckSeconds (report_file_shown)
- sweep-ui1-21: What is the [Hydra][Window] section format in hydra_ui.ini? | owner: format_window_placement + parse_window_placement_line | answered also in: none found
- sweep-ui1-22: What is the [Hydra][Layout] section format in hydra_ui.ini? | owner: format_layout + parse_layout_line | answered also in: none found
- sweep-ui1-23: How are ImGui sizes, fonts and explicit pixels scaled for a UI scale? | owner: scaled_style + px | answered also in: set_ui_scale (wrapper)
- sweep-ui1-24: How long does the panel show "Done!" after a stored analysis (0.5 s)? | owner: AppState::update_analyze_job | answered also in: none
- sweep-ui1-25: Is analysis allowed (hydra_rules.ini loaded)? | owner: AppState::analysis_blocked | answered also in: none found where I looked
- sweep-ui1-26: Are the analysis settings locked? | owner: AppState::settings_locked | answered also in: none found where I looked
- sweep-ui1-27: Is a song folder already in the list? | owner: render_folder_manager (exact string compare) | answered also in: none found where I looked

### sweep-ui2

- sweep-ui2-1: How wide is a button with a given label (label width + 2x FramePadding.x)? | owner: widgets.h button_slot_width | answered also in: paths_tab.cpp button_width (same formula), library_table.cpp render_chips (inline, line 173), library_table.cpp render_search_box clear_w (inline, line 88), settings_bar.cpp render_score_range six_digits (inline, line 113), preview_tab.cpp render_path_picker chrome (FramePadding*2 + frame height for the combo)
- sweep-ui2-2: Does the next item still fit on the current line (wrap decision)? | owner: paths_tab.cpp fits_on_line (right edge = window WorkRect.Max.x) | answered also in: library_table.cpp render_chips (inline, right edge = cursor x + content avail taken at loop start), preview_tab.cpp key_hints (own running x + group_w > width)
- sweep-ui2-3: How is text cut with an ellipsis when it does not fit, and when does it count as not fitting? | owner: widgets.h text_ellipsized (ImGui::RenderTextEllipsis, fits when width <= avail) | answered also in: library_table.cpp draw_title_ellipsized (CalcTextSizeA at max_w - ellipsis width, own RenderChar), render::ellipsize (render/overlay_layout, used by preview_tab render_path_picker), widgets.h row_selectable (fit test re-typed as CalcTextSize > avail)
- sweep-ui2-4: What fraction does a progress bar show when the total is 0? | owner: none | answered also in: widgets.h progress_bar_counted (1.0, full), library_dialogs.cpp batch strip line 389 (0.0, empty), preview_load_job.cpp PreviewLoadJob::Progress::fraction Decoding (0.10, per-stem 0)
- sweep-ui2-5: What is the Preview volume's default and legal range, and how does a percent become audio gain? | owner: app::Settings::preview_volume = 40 (config.h) + Settings::load_file (accepts 0..100, else keeps default) | answered also in: PreviewController volume_pct_ = 40 (preview_controller.h:231), PreviewController::set_volume (clamps to 0..100), PreviewController::poll and set_volume (percent / 100.0f typed twice), preview_tab.cpp SliderInt 0..100, PreviewTransport::set_gain (clamps < 0 only)
- sweep-ui2-6: How far does a Preview time jump move (5 s) and how is it named? | owner: none (5000.0 literal; kTickStep owns the tick step) | answered also in: preview_tab.cpp render_preview_panel buttons -5s/+5s and Left/Right keys (jump_ms(+-5000.0) four times), key_hints kKeyHints text "5 seconds" and "5 ticks", button hint texts "Back 5 seconds"/"Back 5 ticks"
- sweep-ui2-7: Is the overlay the Preview has drawn the one for the selected path (composite overlay key)? | owner: preview_controller.cpp overlay_key (path_key + "|cap" + N) | answered also in: preview_tab.cpp render_preview_panel (overlay_path_key().rfind(ui.overlay_key, 0) == 0 prefix parse, although preview_controller.h:73 says "compare two of these, don't parse one")
- sweep-ui2-8: How many charts does the library hold? | owner: LibraryModel::rows().size() | answered also in: AppState::library_total (cached copy set in reload_library, read by render_library, render_library_pane, render_actions_row), library_table.cpp render_heading (reads rows().size() directly)
- sweep-ui2-9: Did a ShellExecuteW call succeed? | owner: none | answered also in: win32_dialogs.cpp show_in_folder (> 32), app/report_files.cpp open_in_browser (> 32)
- sweep-ui2-10: Does a chart have notes at the chosen difficulty, else which error? | owner: Song::is_empty + parse/song.cpp no_notes_message | answered also in: app/analysis.cpp analyze_chart_file and ui/preview_load_job.cpp PreviewLoadJob::run (each tests is_empty and throws ChartFileError(no_notes_message(...)); both call the owner), SongLengthJob::run (no test; song_length_ms returns nullopt)
- sweep-ui2-11: What is the frame clear colour and the default window/canvas size? | owner: none | answered also in: ui/main.cpp main (clear (0.10,0.11,0.13), window 1280x720 at 100,100), tests/ui/uitest_harness.cpp (clear {0.10,0.11,0.13}, width 1280 in uitest_harness.h)
- sweep-ui2-12: Which grey is dimmed / disabled text, and which teal is the hover face? | owner: theme.h kDimTextColor / apply_theme | answered also in: theme.h kDisabledInputTextColor (same 160 grey, parallel constant), apply_theme FrameBgHovered and HeaderHovered (literal (0,100,100) typed twice)
- sweep-ui2-13: How long does a neutral status message stay before it fades? | owner: library_toolbar.cpp render_status_line (6.0 s literal) | answered also in: none found where I looked (src/ui)
- sweep-ui2-14: Where along an ImGui slider does a 0..1 fraction sit (scrubber marks)? | owner: preview_tab.cpp draw_scrub_marks (copies ImGui's 2 px grab padding + half grab) | answered also in: none found where I looked
- sweep-ui2-15: How wide can an N-digit number get (fixed slot sample)? | owner: widgets.h widest_digits | answered also in: settings_bar.cpp render_score_range ("000000" sample, not the widest digit), library_toolbar.cpp and preview_tab.cpp (call widest_digits)
- sweep-ui2-16: Does the Preview have an SP gauge / drain box to draw (the meter has a curve)? | owner: none | answered also in: PreviewController::sp_meter_has_curve (!segments.empty()), app/preview_view.cpp build_drain_box (returns not shown when segments empty), render_preview_panel (drain_drawn = has_gauge && drain.shown)
- sweep-ui2-17: Which SP cap is the Preview scene built at (controller state vs scene)? | owner: render_preview_panel picks record sp_cap (screenB-12) | answered also in: PreviewController::sp_cap_ (member, reset to kCloneHeroSpCap in close), scene_.sp_meter.cap (read by sp_meter_cap; build_sp_meter_curve clamps cap < 1 to 1)

### tempo

- tempo-1: What ms does a given tick fall at (tick to ms)? | owner: MsIndex::at (through Timecode::Timecode / SongTiming::timecode) | answered also in: MsIndex::ms_at_tick_f (display sub-tick copy, also used by activation_fill_deadline_ms Ch10 pad and build_track_state span ends), play_chart.py ticks_to_seconds, probe_songs.py/active_probe.py (tick == ms by construction)
- tempo-2: Which tick sits at a given ms, and how is it rounded? | owner: MsIndex::tick_at_ms (continuous); rounding has no owner | answered also in: preview_view.cpp tick_at (llround, clamp >= 0), path_view.cpp build_activations timeline_end (inline llround, no clamp), test_search.cpp invariant check (inline llround), probe_chart.py ms_to_ticks (Python round, half-even)
- tempo-3: Which tempo is in force at a moment? | owner: MsIndex::tps_at (tick, a tick on a change reads the new tempo) | answered also in: build_time_box (loop over scene.tempos comparing ms), Song::check_activations bpm_pos walk (dead), play_chart.py ticks_to_seconds
- tempo-4: Which meter (ticks per measure) is in force at a tick? | owner: MeasureIndex::section_at (a tick on a change belongs to the section before) | answered also in: Song::check_activations tpm_pos walk (pre_tpm, advances at most one change per chord), SongTiming::ms_per_measure_at (section_at(t+1), right-continuous), SongTiming::measures_at_tick_f / tick_at_measures_f (own linear scans, right-continuous), build_preview_scene (tpm at last note), build_beat_events
- tempo-5: Which time signature (numerator/denominator) is in force at a tick? | owner: none (Song::timesig_changes is the only data) | answered also in: build_time_box (loop, right-continuous on now_tick)
- tempo-6: What measure position (decimal and measure.beat.tick) does a tick have? | owner: Timecode::Timecode | answered also in: SongTiming::measures_at_tick_f (right-continuous copy used by build_sp_meter_curve)
- tempo-7: Which tick is N measures after (or before) a tick? | owner: SongTiming::plusmeasure | answered also in: SongTiming::sp_end_ms + tick_at_measures_f (continuous copy, test-only today), frontend_transfer_scales (plusmeasure(D,-2) to recover the pre-SqIn end the graph already had), build_sp_meter_curve (drain measured with measures_at_tick_f), test_preview_view sp_act_at, test_squeeze_rating fixtures
- tempo-8: How long is a measure in ms right at a tick? | owner: SongTiming::ms_per_measure_at | answered also in: transfer_scale_between (wrapper), build_drain_box (wrapper)
- tempo-9: How many ticks is one beat? | owner: none (every caller multiplies tick_resolution) | answered also in: Timecode::Timecode (beat = remaining / tick_r), build_beat_events (beat lines every tick_r, half at tick_r/2), Song::check_activations (fill_max_distance_beats * res), fill_lands_on_chord (slop * res), activation_fill_deadline_ms (4 * res for Ch11, res/16 for Ch10)
- tempo-10: What tempo / meter does a chart file declare? | owner: MidiParser::op_tempo, ChartDataEntry::ChartDataEntry, apply_timesig | answered also in: play_chart.py parse_chart / parse_midi (own parsers, 480/120 BPM fallbacks Hydra does not have)
- tempo-11: How far apart are chart time and audio time (song.ini delay vs .chart Offset)? | owner: preview_audio_offset_ms | answered also in: PreviewTransport::load/play/seek_ms (apply it), PreviewLoadJob::run (negative offset becomes front padding); play_chart.py ignores it
- tempo-12: How long is the song in ms? | owner: store::song_length_ms (last note onset) | answered also in: build_preview_scene (scene.song_length_ms = notes.back().ms), PreviewTransport::load (max(last note, audio end - offset))
- tempo-13: What is the song's last measure as shown? | owner: none | answered also in: build_activations timeline_end (last note via ms round trip), build_time_box length (transport length, includes audio tail)
- tempo-14: Where on the song timeline (0..1) does an activation sit? | owner: none | answered also in: build_activations song_fraction (/ last-note ms), build_scrub_marks (/ transport length)
- tempo-15: How many ms is a note from the SP end (backend offset)? | owner: ScoreGraph::add_deact_edge | answered also in: ScoreGraph::store_new_backend, rebuild (tail rows), replay_path (with tick <= D clamp to <= 0)
- tempo-16: Is a note within the squeeze window (kSqueezeWindowMs) of an SP end? | owner: none (constant only) | answered also in: ScoreGraph::build sqout_deacts, ScoreGraph::is_recent_to_head, sqout_candidates (fabs), Activation::display_backends (fabs)
- tempo-17: What ms is a fill's spawn deadline? | owner: activation_fill_deadline_ms | answered also in: test_fill_deadline.cpp TEST_CASEs (recompute both rules' formulas)
- tempo-18: Is an early-fill offset legal / E-critical? | owner: kEarlyFillWindowMs with is_e0 | answered also in: Engine::branch_activate (e_offset < -window), Activation::is_e_critical
- tempo-19: Which sign does an early-fill timing print with? | owner: early_fill_difficulty | answered also in: docs/UserGuide.md line 118 (states the opposite sign)
- tempo-20: How many audio frames is a span of ms (and back)? | owner: none | answered also in: Playhead::seek_ms, Playhead::position_ms, Playhead::length_ms, pad_front_ms
- tempo-21: Does the playhead sit "on" an activation? | owner: activation_jump_ms / kOnActivationMs | answered also in: build_next_act_box (same constant)
- tempo-22: Is Star Power running at a moment in the Preview? | owner: none (stored deact_tick is the data) | answered also in: build_drain_box (a.ms <= now < sp_end_ms), build_track_state sp_active_ interval, build_sp_meter_curve (window loop)
- tempo-23: Where does a phrase/solo/fill span end in time? | owner: none | answered also in: build_track_state (end_tick + 0.5 tick), build_sp_meter_curve (end_ms = last note, banks there)
- tempo-24: What is the per-side hit window? | owner: Settings::hit_window_ms defaulting from kDefaultHitWindowMs | answered also in: ReportOptions::hit_window_ms (own int default), ch_probe constants.py EXPECT_NORMAL_BACK_S = 0.085
- tempo-25: Where do timing tiers end (Beyond edge)? | owner: timing_tiers / beyond_edge_ms | answered also in: report.cpp kPageJs BEYOND = max cutoff
- tempo-26: Which ms edges rate a backend row? | owner: BackendSqueeze::summarystr (inner +-10 literal, outer +-W) | answered also in: none found where I looked
- tempo-27: How does frontend timing scale to the SP end? | owner: transfer_scale_between / frontend_transfer_scales | answered also in: rate_activation reads stored scales (reader, not copy)
- tempo-28: ms to seconds for the renderer | owner: none | answered also in: track_state.cpp s_of, PreviewRenderer::render (now_ms / 1000), preview_tab.cpp scrubber

