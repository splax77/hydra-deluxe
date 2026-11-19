# The work list: what the test audit found, ready to fix

Written by the main session, session 8e6a2908, on 2026-11-18, from the handoff `docs/handoffs/2026-11-18-test-audit-fix-plan-handoff.md`. No test was re-read for it. Its sources are the audit's own files under `docs/audit/2026-11-14-test-audit/`: the 114 notes marked `prod` in `exits/notes/marks.tsv`, the `prod` rows in `exits/cross-rows.md`, the scouts' `exits/notread/findings1-4.md`, the 53 keys in `exits/keys.md` and `exits/groups.md`, and `exits/open.md`. A few lines were opened in the code to confirm a source line still says what the note says; each one is named where it is used.

The user's reason for this file: "well i still want to fix all the garbage tests, i just want to stop wasting time with more reads and get to the point where we can actually act on our findings" (session 98fa2925).

It has three parts. Part 1 is the real Hydra and tool bugs. Part 2 is one fix recipe per fault key, applied to every test under that key. Part 3 is the questions no ruling decides. The fix plan turns parts 1 and 2 into executor tasks.

## Part 1: real bugs in Hydra and its tools

Each item merges every note that names the same fault. Each says whether a player would see a different number or word if it were fixed. Only those need a question first (agent rule 11); they are marked **visible** and repeated in Part 3.

### 1A. A display works out a game answer itself

The engine should give each of these answers once, and the screen should show it (agent rule 15). Every item here is a derive-once fault.

1. **The score box decides doubled or plain on its own.** `build_score_box` (`src/app/preview_view.cpp:634-635`, opened) asks the Preview's own SP window, through `running_activation`, whether to show the doubled multiplier. The replay row already says it in `in_sp` and `multiplier_shown`. On the leeway chord they disagree: the replay says doubled, the box shows plain. `src/render/track_state.cpp` answers "is SP running" a third time. **Visible.** Sources: `records/T3-run2.md:259`, `:311`.
2. **The drain box estimates bar and full-meter times from one tempo.** `build_drain_box` (`src/app/preview_view.cpp:663-685`) takes the measure length at the playhead and multiplies. The search integrates across tempo changes (`src/search/graph.cpp:303-309`). They agree on charts with no tempo change near the playhead and differ otherwise. **Visible** on charts with tempo changes. Sources: `records/T3.md:15`, `:23`, `records/T3-run2.md:436`, `records/B7.md:195`, `:372`.
3. **The solo box sums the solo bonus itself.** `build_preview_base` adds `solo_bonus(...)` per chord (`src/app/preview_view.cpp:341`, opened). The path view and the Stars tab show the engine's `path.score_solo`. They agree on the one test song. **Visible** only where they differ, which nobody has shown. `solo_percent` (`preview_view.cpp:690`) is the same kind. Sources: `exits/notread/findings4.md:32`, `records/T3.md:607`.
4. **"Enough SP saved" is read from the drawn meter curve.** `saved_enough` (`src/ui/preview_timeline.cpp:493-515`) uses the curve's y positions, not `Activation::sp_ready_tick` (`model.h:631`), which the note window already uses. **Visible** if the two land on different chords. Sources: `records/A22.md:17`, `verify/A22.md:114`.
5. **The Path Builder's fill reason counts phrases from the gauge.** `fill_reason` (`src/pathbuilder/pb_text.cpp:70-74`) reads `bars_saved_at` from the Preview gauge, not the engine's own per-fill state. **Visible** if they differ. Source: `verify/A19.md:83`.
6. **The squeeze window on screen and the engine disagree.** `squeeze_window_ends` and `early_fill_window_end` are called only from `src/app/path_view.cpp:225`, `:236`, and `tools/replay_schedule.cpp` works out `feasible` its own way. The verifier saw the screen say "47.1 ms safe" where the engine says not feasible, on `late_sqin_twice` at 30.0 ms. **Visible**, and seen. Source: `verify/A3.md:37`.
7. **The squeeze rating figures live only in display code.** `rate_note`, `effective_backend_ms`, `squeeze_budget_ms` and `nominal_budget_ms` (`src/core/squeeze_rating.cpp`, called from `path_view.cpp:281`, `:387`) and `core::sqout_cost` (called from `path_view.cpp:126`, `:421`, while the engine records `sqout_reduction`). The user asked for the figure (D269 call 2); the fault is only where it is worked out. Moving it to the engine should change no number. If one changes, that is a question. Sources: `records/A30.md:550`, `verify/A30.md:29`, `:57`, `verify/A2.md:35`, `verify/C12.md:43`.
8. **The hardest-figure fill skips some paths.** `record_store.cpp:1519-1520` picks paths by `sqin_count > 0`, but `engine_hardest_ms` (`model.h:919-933`) also counts squeeze-outs and early fills. A path with only those never gets its hardest figure filled in. **Visible.** Source: `verify/A32.md:77`.
9. **The fill page counts its copies twice.** `charts` (`src/app/fill_report.cpp:93`) and `tally_fill_rows` (`:290`) are two sums of the same copies. **Visible** only if they ever differ. Source: `records/A14.md:771`.
10. **Bank arrivals override the bank rise.** `Path::bank_arrivals` (`src/core/model.cpp:574-579`) is called only by the Preview and overrides `bank_rise_ticks`. Source: `records/A29.md:312`.
11. **Two paths can share a key.** `path_identity` (`src/core/model.cpp:633-643`) uses only score, activation ticks and deactivation ticks. Two paths that differ only in squeeze steps would share the Preview overlay key and the path-list key. Reasoned, not seen. Source: `records/A20.md:453`.
12. **The all-0 answer has no single reader.** It is stored in `record.paths` best or `record.allzero_paths` (`pather.cpp:53-56`), and the display and tests each work it out again. Sources: `records/delta-DL5.md:95`, `:395`.

### 1B. The same words typed in two places

13. **The all-0 group's name.** The Preview types "(best all-0)" (`src/app/preview_view.cpp:824`, opened). The owner `kBestAllZeroLabel` says "Best all-0 path" (`src/app/path_view.h:226`, opened). Reading the owner changes what the Preview shows. **Visible.** Sources: `records/A20.md:571`, `verify/A20.md:99`.
14. **The settings names.** `library_dialogs.cpp` types "Pro Drums", "2x Bass" and "Note Shuffle" in both `batch_settings_summary` (:73-75) and `change_phrase` (:93-96). The confirm says "Clone Hero 1.0/1.1" where the settings button says "1.0 fills" (`library_dialogs.cpp:99`, opened). One owner for the names changes no text; picking one spelling for the fills setting does. **Visible** for the fills spelling. Sources: `records/A8.md:101`, `verify/A8.md:27`, `:67`, `:70`.
15. **Other screen sentences with no owner.** The upgrade box sentences (`src/ui/library_view.cpp:151-152`), "Type any words, or narrow it down." (`library_table.cpp:235`), the version bar sentences in `app_state.cpp`, the report window sentences (`report_window.cpp:121-124`), "Nothing matches" (`html_page.h:47`) and "cannot open file: " typed in at least six places. Giving each one owner changes no text. Sources: `records/A42.md:194`, `records/A47.md:196`, `verify/A41.md:141`, cross row X5.e.
16. **One refusal stated twice.** "Beat shorter than a tick" is refused in both `apply_timesig` (`song.cpp:63-64`) and `MeasureIndex` (`timing.cpp:125`). Source: `records/B7.md:80`.
17. **An unnamed SQLite setting.** `wal_autocheckpoint` is a bare literal. Source: cross row X10.g.

### 1C. Test hooks inside production code

18. **Test-only switches in the store and the jobs.** `fail_chart_list_reader_open_for_test` (`record_store.h:718`, `:760`), `fail_next_group_commit_for_test` (`record_store.h:592`), `between_lookup_reads_for_test` (`record_store.cpp:1385-1390`, `:1464`), the path swap in `open_sqlite` (`record_store.cpp:1709-1719`), `set_lookup_hold_for_test`, and the two fake-analyzer seams `BatchJob::set_analyzer_for_test` and `set_app_batch_analyzer_for_test` (`src/ui/library_jobs.h:285`, `:419`). No ruling asked for any. Each goes when the tests that use it are rewritten under C-analyzer and the store keys in Part 2. Sources: `records/A31.md:617`, `records/A4.md:724`, `verify/A4.md:58`, `records/delta-DL1.md:125`, `verify/delta-DL1.md:32`, `verify/A31.md:88`, `verify/delta-DL4.md:23`, cross row KR.b.

### 1D. Code only tests call

Each of these has no caller in `src/` or `tools/`; a test keeps it alive. The fix is to delete the code and the tests that pin only it. No player sees a change. Where a test reaches real code only through the dead wrapper, the test moves to the real function.

19. `Activation::needs_timing` and `Path::needs_timing` (`model.cpp:530`, `:685`); `assign_score` (`replay.cpp:251`, whose header and `CONTEXT.md:518` call it half of "the only pairing", so those words change too); `report::tier_for` (`report.cpp:76`); the tick-list `search_target` overload (`pather.cpp:387-391`); `RecordStore::ready_song_ids` and `ready_records` (`record_store.cpp:1436`, `:1445`); `ReplayOptions::table` and its two refusals (`replay.h:239-244`, `replay.cpp:76-90`); `SongTiming::sp_end_ms`, `tick_at_measures_f` and `MeasureCount::tick_at` (`timing.cpp:234-238`, `:340-346`); `MidiFile::from_file` and `message_type_name` (`src/parse/midi.cpp`); `sng_decode_file` (`sng.cpp:141`; `sng_decode_file_into` is the real decoder, and it has no wrap case); `mix_stems` (`src/audio/mixer.h`); `Playhead(DecodedAudio)` with `BufferSource`; `Settings::is_rescan` (written, never read; the Scan button draws one fixed label); `AppState::any_job_running` (`app_state.cpp:609`); `answered_count` (`library_jobs.cpp:499`); the `chords = nullptr` default and its branch in `path_view.h:161`, `:324` (29 test calls use it); the `Any` branch of `term_applies_to` (`library_query.cpp:330`).
20. Kept by a tool, not dead: `file_size_bytes` (used by `tools/bench.cpp:490`, `:522`) and `load_songbytes_sng` / `load_songbytes_srb` (used by the fuzz harness). These stay.
21. In the Clone Hero tools: `EngineModel.capture_object` and five readers (`tools/ch_probe/engine.py:169-201`), the `sp_compare._find_activations` alias (`sp_compare.py:241`), and the `waiver.py:49-50` entry point that no hook call reaches.

### 1E. Wrong comments and stale lists

22. `app_state.cpp:293-295` cites D87 item 1 for a claim D87 item 1 does not make. `preview_load_job.cpp:33` says the slices add up to 1, but `Progress::fraction()` (`:248`) returns 0.945 for the Highway step. `tests/code_lexer.h:3-11` says only two checks use it, but `tools/precheck` uses it too. `config.h:345-347` warns that `kAnalysisFields` can miss a field and nothing checks it. `tools/compare_db.py:130-133` lists tables the store no longer writes. `menu_tools.py:45-51` calls the scroll key a guess while code and three tests carry it. `dump_facts.py:141-144` names a field '0D58' from the middle of its type text. `foreground.py:20-21` and `dump_build.py:22-25` cite answers nobody found. Sources: `records/A6.md:77`, `records/A22.md:18`, `verify/A12.md:133`, `records/A9.md:636`, `records/D14.md:7`, `records/D7.md:17`, `records/D2.md:1947`.

### 1F. Bugs in tools and hooks (agents see these, players do not)

23. **The precheck names a flag that no longer exists.** `tools/precheck/check_3.cpp:421` prints "-WholeTree" (opened); the exe takes `--whole-tree`. In the same tool, `records.cpp:244` counts members that differ only in case once, and `code_mask.cpp:36-47` repeats `views.cpp:29-36`. Source: `records/C8.md:531`.
24. **The quiet check misses builds in this checkout's worktrees.** `tools/ch_mcp/quiet.py:239-245` scans only `<checkout>/.claude/worktrees`, so a build in any of the folders under `worktrees/` never stops a Clone Hero launch. Source: `verify/D8.md:18`.
25. **The status script says "appended" when the write failed.** `status_append.ps1:6` silences errors and line 26 prints "appended to" whatever happened; it also works out the status path that `common.status_path` owns. This is in the hooks repo. Sources: `verify/E4b.md:20`, `:256`.
26. **The Bash rule refuses real commit spellings.** `bash_windows_rule.py:25` knows only `-C` and `-c` as git options with values, so `git --no-pager commit -m ...` and `"git" commit -m ...` are refused as too long. Hooks repo. Source: `verify/E1b.md:58`.
27. **Plurals.** "1 messages seen" (`frida_attach.py:182-183`), "1 messages" and "(1 rows)" in the game tools' replies, and "1 context switches", "1 half-written rows", "1 whole samples" in `machine_report.py`. Tests pin the wrong text. Sources: `records/D2.md:839`, `records/D4.md:667`, `:800`, `records/D7.md:427`, `verify/D7.md:72`.
28. **A machine path as a default.** `tools/effort_test_guard.ps1:30` defaults `-VersionFile` to a path under the user's home folder. Source: `records/D15.md:226`.
29. **Copies in the Clone Hero tools.** The route words `ROUTE_KEYS` and `ROUTE_KIT` are defined in both `score_compare.py:55-56` and `game_tools.py`. `sp_compare.SP_MULTIPLIER = 2` (`sp_compare.py:107`) copies `kStarPowerMultiplier` (`timing.h:48`). `game_tools.py:89-91` reaches into `bot_run`'s private names. `game_files.py:348-350` (`song_ini_pro_drums`) and `song.cpp:2280-2283` (`ini_drum_key_is_true`) answer differently for a value such as 2. The stop text "clock at 200.00s" (`play_chart.py:777-778`) differs from the frozen-stop text "clock frozen at 20.00 s". Sources: `records/D12.md:9`, `records/D13.md:27`, `verify/D1.md:66`, `records/D3.md:9`, `records/D11.md:18`.
30. **A tool that checks a refuted rule.** `watch_checks.round_start_check` and `constants.py:299-301` test that +0x2E0 is a hit time at every chord, which `docs/clone-hero/03-timing.md` TM-14 refutes. Tests 90-96 and 109 in D13 pin it. Source: `records/D13.md:505`.

### 1G. Possible bugs nobody has proven

These need one check before they become a task. The fix plan gives each a short check step; a fault that does not show is dropped.

31. Running status after a sysex: `src/parse/midi.cpp:225` sets `running = b` for 0xF0 and 0xF7 too, unchecked against mido. Source: `capture/byreading/B3-verdicts.md:54`.
32. A search match inside a multi-letter fold (German sharp s as "ss") highlights the whole character (`library_query.cpp:491-492`), with no test. Source: `records/A17.md:31`.
33. Hydra counts a vocals track's low notes toward the song end and the game may not (`verify/B2.md:82`, CR-44, Q-74). This is a Clone Hero fact, so the check is against `docs/clone-hero/` first.
34. The key lock may not see route 'Keys' with a capital K (`test_percall_key.py:95`); `prepass.py` may require 'typed' where real transcripts say 'sdk' (`records/E3.md:554`, `:960`).

### 1H. Numbers in the tools with no named source

35. `play_chart.py:111`, `:115`; `CONST_MATCH_TOLERANCE_MS = 0.5` and `WINDOW_MATCH_TOLERANCE_MS = 0.01` (`constants.py:234-235`, `:261`); `prep_inputs.py:42-50`; `QUIET_SAMPLE_S`, `BUILD_RECENT_S`, `SCAN_CAP_S` (`quiet.py:29-46`), `RECORDER_EXIT_CAP_S` (`recorder_link.py:23`), `READER_CAP_S` (`mono_facts.py:45`). Each gets its source named in a comment where one exists. Where none exists, the number is an engineering choice, said so in one line. None goes to the user as a question (agent rule 15, last sentence). Sources: `records/D11.md:18`, `records/D12.md:9`, `:581`, `:686`, `records/D8.md:11`.

## Part 2: one fix recipe per fault key

A key names a ruling and the way a test breaks it (`exits/keys.md`). Each recipe below applies to every row under its key; `exits/labels/labels.tsv` lists the rows, and the row counts are from `exits/groups.md`. A test with two keys gets both recipes. Rows are tests, so the plan groups them by test file, not by key: one executor owns a file and applies every recipe its rows carry.

The rulings these recipes stand on are the user's words under `## Confirmed` in `docs/agents/rulings/tests-and-runs.md`, lines 9-13: R-fake, R-const, R-cite, R-string and R-db.

### Stand-ins for Hydra's own output (R-fake)

The recipe for all of them: run the real code on a real input and check what it gives. The input is a chart from `testdata/input`, or a hand-written chart, ini or file that the real reader reads, which the user's answer 1 calls input. The test then checks a property the claim names, read from that real result.

| Key | Rows | Recipe |
|---|---|---|
| F-result | 174 | Run the engine on a corpus chart and read its result, in place of building activations, paths or best-path text by hand. |
| F-row | 214 | Build Library rows by scanning and analysing corpus charts through the real store, in place of `ready(...)`, `analyzed(...)`, `fake_charts`. |
| F-record | 127 | Write the record by analysing a chart, then read it back. |
| F-summary | 14 | Same as F-row, for summaries. |
| F-schedule | 61 | Make the schedule or replay JSON with `hydra_replay` on a corpus chart, check the file in under `testdata/`, and say in the test which chart and command made it. |
| F-sample, F-presslog, F-watchlog | 96, 37, 63 | Record the log from one real run, check it in under `testdata/` with the command that made it, and read that file. |
| F-verdict | 33 | Have the real review or waiver code write the record, then read it. |
| F-db | 27 | Use a store Hydra wrote from the test corpus (R-db), not SQL typed by hand. |
| F-window | 3 | Have Hydra write the saved-window text, then read it. |

### Stand-ins for outside data (R-fake)

The recipe: capture the real thing once, check it in under `testdata/` with where and when it came from, and read that file. The capture is real data, not a stand-in.

| Key | Rows | Recipe |
|---|---|---|
| O-reply | 31 | A saved real reply from dmleaderboards or GitHub. |
| O-transcript | 40 | A real transcript, cut to the lines the test needs. |
| O-gamemem | 339 | A saved dump of real game memory or method calls from one Clone Hero run. Making the dumps needs live runs; see Part 3, question 3. |
| O-process | 41 | A saved real process list. |
| O-payload | 6 | A saved real hook payload. |
| O-scorestats | 17 | A real `scorestats.json` from a run. |
| O-ocr | 8 | Real OCR output from a saved frame. |
| O-toolout | 8 | Run the outside program (`git diff` on a temp repo) in place of typing its output. |

### Fake code (R-fake)

The recipe: call the real code. Where the real code is slow or reaches outside the machine, the test runs it on the smallest real input that shows the claim.

| Key | Rows | Recipe |
|---|---|---|
| C-analyzer | 57 | Use the real analyzer on corpus charts. Then delete the seams in item 18. |
| C-reader, C-worker | 79, 14 | Use the real reader or worker on real files. |
| C-rig | 208 | Drive the real code through its own entry point. |
| C-clock | 149 | Use the real clock with a wait on an event, not a fake time. |
| C-session | 111 | Use a real session on a temp folder. |
| C-stub | 222 | Run the real program. |
| C-gate | 45 | Load the real gate module and the real rules file. |
| C-ownchecker | 51 | Test the real checker, not one the test file defines; delete the copy. |

### Typed constants (R-const)

The recipe: the test stops typing the answer. It either reads the value from the code's one owner, or checks the behaviour the value stands for. For a message, the behaviour is when it is shown and what it names (cross-findings X10).

| Key | Rows | Recipe |
|---|---|---|
| P-burnout, P-pbticks, P-allister, P-builtchart | 55, 20, 5, 146 | Check a property of the engine's answer (the path is reachable, the score equals the sum of its parts, the fill lands on a fill), not the number one run printed. |
| P-tiers | 19 | Read the tier names and edges from their owner. |
| P-message | 903 | Check that the message appears in the right case and names the right thing; read fixed wording from its owner. |
| P-placeholder, P-setting, P-name | 15, 99, 88 | Read the text, default or name from its owner. Where there is no owner, make one first (Part 1, items 13-17). |
| P-onefile, P-other | 13, 666 | Read the constant from its owner, or check what it does. |

### Citing, snapshots and the rubric's kinds

| Key | Rows | Ruling | Recipe |
|---|---|---|---|
| Z-cite | 339 | R-cite | Move the expected value's source from the comment into code: compute it, or read it from the owner. |
| S-snapshot | 254 | R-const | Replace the printed total with a property of the result. |
| E-echo | 269 | rubric 8 | Check what the code does with the value, not that it comes back. |
| K-cantfail | 113 | rubric 8 | Make the check able to fail, or delete it. |
| K-floor | 118 | rubric 5, 9 | Give the floor its reason in code, or check the exact behaviour. |
| K-skip | 97 | rubric 9 | Fail when the input is missing, never skip quietly. |
| K-selfmade | 146 | rubric 5, 9 | Get the expected value some other way than the function under test. |
| K-comment | 247 | rubric 4 | Name the meaning in the check, not a comment. |
| K-unreachable | 42 | rubric 11 | Use a scenario a real chart reaches, or delete the test. |
| K-invented | 5 | rubric 2, 3, 6 | Test the engine's rule, not the display's. |
| K-duplicate | 179 | rubric | Delete the copy and keep the better test. |
| K-namebody | 69 | rubric 1 | Make the body check what the name says, or rename it. |
| K-partial | 33 | rubric 1 | Loop every case, or say in the name which ones. |
| K-nocode | 61 | rubric 2 | Delete it, or point it at Hydra code. |

The rubric keys (E-echo and the K keys) break no filed ruling. `keys.md` says a group under such a key is asked as its own question. Part 3, question 4 asks them as one.

### The 107 rows no key fits

`exits/groups.md`, "Rows no key fits", sorts them into kinds. Each folds into a recipe above or stands as a small task.

- **Never checked on real input** (the `records/A9.md` rows, the `records/T3-run2.md` rows, `records/A24.md:98`, `:120`): run the claim on a corpus chart or a real settings file, as the F recipes do.
- **Fixed sleeps and waits** (`records/D3.md:484` to `:561`, `records/C7.md:268`, `records/A4.md:145`): wait on the event with a cap, as C-clock does.
- **Flaky checks** (`records/E3.md:526`): make the check exact.
- **Bundled claims** (`records/D2.md:673` to `:1365`, `records/C6.md:1018`): split into one test per claim.
- **Hand-written input under a fake-input mark** (the `counts/T5-IR3.md`, `T5-IR17.md`, `T5-IR7.md`, `T5-IR19.md`, `T5-KR*` rows): these are input under answer 1, so the test stays as it is unless another key says otherwise.
- **Proxies** (`records/A7.md:420`, `records/A46.md:150`, `records/delta-DL3.md:599`, `records/A14.md:586` to `:671`, `records/A39.md:149`): check the drawn text or the real function, not a flag or a counting stand-in.
- **The D13 rows TM-14 refutes** (`records/D13.md:501` to `:590`): they go with Part 1, item 30.
- The rest (`records/A18.md:285`, `records/A25.md:535`, `records/A27.md:201`, `records/C6.md:774`, `records/D4.md:761`, `records/delta-DL2.md:361`, `counts/T5-IR9.md:92`): each a small task in the file that holds it.

## Part 3: the questions no ruling decides

These go to the user with the fix plan, five or fewer at a time. Each visible one shows what the screen says before and after, from a real chart (agent rule 11); the fix plan fills those in from a run, not from this file.

1. **Visible splits in Part 1.** Items 1, 2, 4, 5, 6, 8 and 13, and the fills spelling in item 14. For each, the engine's answer is the one the rules say to show, so the question is only whether the user accepts the change they will see. Items 3 and 9 join only if a run shows a difference.
2. **May the 20 open records be settled by a run?** `exits/open.md` puts them in three groups: 12 that need the test or one run on the corpus (A16 22 needs the user's library), 6 that need the real `dump.cs` and `facts.json`, and 2 scripts that have no assertion and need a person watching the game. The question for the last two is whether they are tests at all.
3. **Live Clone Hero runs to capture real game data.** O-gamemem's recipe (339 rows) and F-sample, F-presslog and F-watchlog need data recorded from real runs. The question is whether those runs may happen, and how many.
4. **The rubric keys.** E-echo and the K keys break no filed ruling, only the rubric. One question: fix them by the recipes above, as part of fixing "all the garbage tests"?
5. **Where hand-written input stops and a fake begins.** `exits/groups.md`, "Calls the verifiers made that E7 should weigh", lists the boundary calls: P-builtchart read broadly or narrowly, K-floor for fixed waits, whether a file's group note gives its rows a key, and the fake-input marks with no fake key. Each is a label call, and the recipe differs on each side.
6. **Words of the user's that are not filed.** "Yes, drop it" and "why are we relying on framerate at all? this is just bad design" (session 1ac0d6b4) sit under `## Unconfirmed` in `docs/agents/rulings/clone-hero.md:70`, while a tool cites them as Confirmed (`verify/D8.md:18`). Filing them is the user's click.
