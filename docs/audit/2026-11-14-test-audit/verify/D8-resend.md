Verifier agent ab19de5956138d8f9 (T5-V-RESEND-D8), slice D8, session 16892967-2cf8-4437-a7cd-e3dad250f621. Read-only. Nothing from the repo's tests was run. Every mark below is "by reading" unless it names a check I made on this machine (a few small Python and PowerShell reads of real files, listed where used).

# Verify D8, re-send: the 44 records the first verifier did not reach

Reached: all 14 of `test_mono_facts.py` and the 30 of `test_server.py` that `verify/D8.md` lists under "Not reached". 44 records checked. 32 agreed, 12 disputed. Not reached: none.

## Real input and independent owners found for question 3

These are the things the records said did not exist, or did not name.

1. A real Mono facts file exists: `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\4045\facts.json` (game_version v1.0.0.4045-master, five assemblies, 1,020 classes). I read it with a script. Every field offset and method rva is null. All 101 enums hold only `value__`, so constants are left out. 539 class names are non-ASCII and the `readable` flag equals `name.isascii()` for all 1,020. `[Out]` appears on 2 parameters, `[In]` on 17, `in` on 3, `out` on 45, `ref` on 70, and 3 parameters carry both `[In]` and `[Out]`. No enum, struct or interface has a base. 380 `typedef_index` values repeat.
2. The real 1.0 game folder exists: `C:\Clone Hero\1.0 old\Clone Hero\Clone Hero_Data\Managed` holds all five dlls `GAME_ASSEMBLIES` names.
3. The schema's owner is `tools/ch_mcp/README.md` lines 27 and 44-47: offsets and rvas are null for Mono, constants are left out, and `typedef_index` is the row in the type's own assembly's TypeDef table, so rows repeat across assemblies.
4. The .NET runtime is an independent owner for the two real-reader tests. I ran one PowerShell reflection read: `SortedDictionary<int,int>` has a private `_keys` field of type `KeyCollection`; `TryGetValue` returns bool with `int, int&`; `Interop.Globalization.GetLocales` has first parameter `char[]` with `IsOut` true; `EncodingExtensions.GetBytes` has `ReadOnlySequence&` parameters with `IsIn` true.
5. The real `dump.cs` (`hydra-data\ch-dumps\6142\dump.cs`) writes no base for `enum` and `struct` headers and writes `[Out] X`, `in X`, `ref X` and `[In] [Out] X` in parameters (counts: 24, 431, 3,231 lines; the combined form appears at its line with `float[]`).
6. `tools/ch_mcp/README.md:77` gives the reason `.mcp.json` has no `-I`: the frida module lives in the user's site-packages and `-I` skips them. I checked it: `py -c "import frida"` finds it at `...\Roaming\Python\Python314\site-packages\frida`, and `py -I -c "import frida"` raises ModuleNotFoundError.
7. The decision log answers two records the auditor left open. D326 item 2 (`docs/audit/2026-10-03-fix-decisions.md:1877`): "the MCP server ignores an argument it does not declare ... Sides: (a) leave the server as it is; (b) make the server refuse undeclared arguments. ... The user answered "go with recommended": keep (1), (a) for (2), keep (3)." D326 item 1 keeps the required `schedule_json`. D324 item 2 (line 1870): the user chose "Remove the flag (Recommended)" for `decline_mode`. The MCP page `docs/agents/clone-hero-mcp.md` lines 51-53 names `schedule_json`, `path_index`, `probes_json`, and the leftover `timing` behaviour, and cites `test_a_leftover_timing_argument_is_dropped_without_a_word` by name.

## Splits

- The text "nothing open" is typed in two places: `tools/ch_mcp/server.py:205` (a literal) and `tools/ch_mcp/front.py:60` (`NOTHING_OPEN`). Only `test_the_front_reads_the_real_nothing_open_as_nothing_open` ties them.
- `typedef_index` means two things under one key: the dump route writes the global TypeDefIndex of dump.cs, the Mono route writes the row inside the type's own assembly (README:27; 380 repeats in the real 4045 facts). The record's capture request for `test_row_number_is_the_typedef_index` ("the row and its dump.cs index for one build") cannot exist for the Mono build, since it has no dump.cs.
- D324 item 2 says a call passing `decline_mode` "fails as an unknown argument". The server ignores undeclared arguments. D326 item 2 is the user's answer for this ((a), leave it), so this is covered by that answer, not open.

## Gaps the records missed (not splits)

- `managed_folder` refuses on anything but one match (`mono_facts.py:51`); the test covers 0 and 1, never 2.
- `run_reader` has a second refusal, exit 0 with no output file (`mono_facts.py:76`); no test covers it.
- The three real parameters that carry both `[In]` and `[Out]` are not in any expectation.
- `test_each_public_check_answers_both_ways` covers four of the five public checks `Helper.open_items` calls; `frida.is_attached` (`server.py:101`) is covered only through the reply tests.

No record proposes a saved expected-output file. No record's Rewrite does.

## Records, test_mono_facts.py (14)

Shared answer S-MF: its question 2 ("callers in src/: none") holds; I searched `src/` for `mono_facts|mono_reader` and found none. Its remark that the bodies were not read is why I re-read `mono_facts.py` lines 33-151 and `dump_facts.py` 53, 85-87, 331-348.

- test_top_level_keys_and_version: agree Rewrite. `dump_file` is `", ".join(a["file"] ...)` (`mono_facts.py:138`), so line 95 types the join; the 9 at line 97 is the fixture's 8 + 1 types. Red if the version is dropped or a type is lost.
- test_fields_have_no_offset_and_constants_are_left_out: DISPUTE. Investigate -> Keep. Red at lines 102-106 if `to_class` stops filtering `literal` (line 114) or writes an offset. The schema owner is README:44-47 and real input is the 4045 facts (finding 1 above): all offsets null, all 101 enums hold only `value__`.
- test_methods_have_no_rva_and_keep_param_types: DISPUTE. Investigate -> Keep. Red if `rva` is not None or params change (`mono_facts.py:116-125`). Same real input and owner (README:27; 45 real `out` parameters).
- test_row_number_is_the_typedef_index: agree Investigate, claim not checked only. Correction to the capture: the independent owner is the assembly's TypeDef row (a loaded type's `MetadataToken` low 24 bits), not dump.cs; see the split above. Line 123 is red only if `raw["row"]` stops being copied (`mono_facts.py:107`).
- test_kinds_and_bases_follow_the_dump_cs_shape: DISPUTE. Investigate -> Keep. `class_kind` and the `base` rule are `mono_facts.py:84-102`; the real dump.cs header lines write no base for `enum` and `struct` (finding 5) and the real 4045 facts hold none. Red at lines 128-133 if a kind or base is misclassified. Weak spot: the "a class directly under System.Object has no base" rule (line 130) was checked against dump.cs headers only by the 793 plain class headers having no base text, not per class.
- test_readable_and_scrambled_names: DISPUTE. Investigate -> Keep. The owner is `dump_facts.is_readable` (`name.isascii()`, dump_facts.py:85-87), which the test does not recompute. The real obfuscated names are non-ASCII (539 of 1,020 in the real facts, the flag matching on all). Red if `to_class` stops calling it.
- test_signatures_name_the_three_readable_classes_only: agree Rewrite. The three names are `dump_facts.NAMED_SIGNATURES` (dump_facts.py:53); the test types them. `field_count 3` is the fixture's three non-literal fields; `method_count 2` is its two methods.
- test_a_missing_named_class_refuses: agree Keep. The text is `dump_facts.find_by_name` (dump_facts.py:335); red at line 158 if `build_facts` skips a missing class.
- test_a_missing_assembly_is_named: agree Rewrite. The message is `f"{managed_dir}: missing {', '.join(missing)}"` (`mono_facts.py:60`); line 165 types the last entry of the production list.
- test_the_managed_folder_must_be_one: DISPUTE. Investigate -> Keep. The production glob is `*_Data/Managed` (`mono_facts.py:50`) and the real game folder `Clone Hero_Data\Managed` matches it and holds the five dlls (finding 2); README:27 states the layout. Gap: more than one match is never tested (above).
- test_the_reader_cap_stops_with_a_line: agree Investigate. The comment at `mono_facts.py:44` says only "The time cap for one reader run, in seconds." I searched `docs/agents/rulings/*.md` for 300 and READER_CAP and found nothing, so 300 has no source. The fake raises at once, so no real wait is run.
- test_a_failed_reader_run_reports_its_error: DISPUTE. Keep -> Investigate, claim not checked only. The runner is faked and "BadImageFormatException" is typed; a real `run_reader` over a file that is not an assembly, under the real pwsh, gives the real stderr and takes seconds (the next two tests already run the real child). Line 189 is red if stderr is dropped from the message.
- test_the_real_reader_decodes_a_runtime_assembly: DISPUTE. Investigate -> Keep. The expected values have an independent owner: the runtime's own reflection (finding 4) confirms the `_keys` field and its type, `TryGetValue` with `out`, and the value-type enumerator is the reader's `struct` kind from `System.ValueType`. Real input and real child. Risk: the test reads PowerShell's own dll, so a PowerShell update that renames `_keys` turns it red without a Hydra fault.
- test_the_real_reader_writes_parameter_markers_the_way_dump_cs_does: DISPUTE. Investigate -> Keep. Reflection (finding 4) gives `GetLocales`' first parameter `IsOut` and `GetBytes`' `ReadOnlySequence&` with `IsIn`, and dump.cs (finding 5) writes `[Out] X` and `in X` the same way. Gap: the `[In] [Out]` pair is not asserted.

## Records, test_server.py (30)

Shared answer S-SV: question 3's wording "claims about reply text need a saved real reply" holds; where it applies I say so. The tests call the real MCP server object (`build_server(...).call_tool`) over a real `Helper`; only a tool class's method is replaced in the pass-through tests.

- test_ch_open_items_says_so_when_nothing_is_open: agree Keep. Production is `server.py:205` `... or "nothing open"`; red if the empty reply changes. The consumer's constant is `front.NOTHING_OPEN` (split above); line 74 types the text but the next two front tests tie it.
- test_ch_open_items_names_the_run_with_its_backup_folder: agree Investigate, claim not checked only. Text matches `game_tools.py:686-687`; the run is a `SimpleNamespace`.
- test_ch_open_items_names_an_adopted_backup: DISPUTE. Keep -> Investigate, claim not checked only. The record's reason is wrong: `front.open_items` (front.py:432-443) treats any reply other than the one line "nothing open" as open items and passes the lines on unread, so the wording "adopted backup: ..." is not a contract the front reads. Line 82 pins a printed line with no consumer. Capture: a real `ch_open_items` after a real adopted backup.
- test_the_front_reads_the_real_nothing_open_as_nothing_open: agree Keep. Real producer reply into the real consumer; red if `NOTHING_OPEN` and the server literal drift apart (line 93).
- test_the_front_reads_the_real_adopted_backup_line_as_open: agree Keep, for a different reason than the record's: it shows a reply that is not "nothing open" is returned as open (front.py:443). Line 97's typed string is a pass-through check.
- test_ch_open_items_lists_each_running_thing_on_its_own_line: agree Investigate, claim not checked only. Order and text match `server.py:99-111` and `game_tools.py:690-691`.
- test_ch_open_items_leaves_out_what_has_ended: agree Keep. Red if any ended thing keeps a line, since line 119 wants the whole reply to be "nothing open". The kit stays at its default None, so its ended case is the default.
- test_open_items_uses_public_checks: agree Rewrite. The only line is `assertNotIn("._", source)` (126); it would also fail on a legitimate `self._x` and pass on `getattr`.
- test_each_public_check_answers_both_ways: agree Keep. Three states per check (lines 144-148), red if a check ignores `ended` or `is_alive`. Gap: `frida.is_attached` is not in the list.
- test_a_long_reply_is_cut_at_a_line_with_the_clone_hero_closing_line: agree Rewrite. `REPLY_CAP_CHARS = 12_000` (`tools/hydra_ui_mcp.py:44`); line 159 types "12,000".
- test_the_trace_texts_say_the_hook_fires_on_hits: agree Investigate, claim not checked only. The game fact has a page entry, `docs/clone-hero/07-internals.md:235-237` (IN-19): "it would fire only for hit notes ... The trace half of that record is not on disk", and `docs/clone-hero/live-checks.md:135` lists IN-19 as an open check. The test checks wording only. Capture: the IN-19 run.
- test_the_kit_is_wired_into_the_menu_driver_and_the_auto_player: agree Investigate, claim not checked only. Replies match `game_tools.py:1105-1108`; both are typed copies of the owner's text built from `ROUTES` and `ROUTE_KIT`.
- test_the_play_tools_take_schedule_json_and_no_dump_json: DISPUTE. Investigate -> Keep. Every typed name now has a source: `path_index` D318 item 4; no `timing` D342; no `decline_mode` D324 item 2 (user: "Remove the flag (Recommended)"); required `schedule_json` D326 item 1 (user: "go with recommended"); `probes_json` the MCP page line 52 and a plan task only, no user words. The schema is read from the real server, so a parameter added or dropped turns lines 199-211 red.
- test_ch_edge_probe_passes_its_arguments_through: agree Keep. `server.py:460` passes the four in the order line 220 expects; inputs are distinct strings.
- test_ch_run_chart_passes_its_arguments_through: agree Keep. Distinct inputs, red on a swap.
- test_a_leftover_timing_argument_is_dropped_without_a_word: DISPUTE. Investigate -> Keep. D326 item 2 is the source (finding 7), and the MCP page line 51 states it. `server.py` has no code naming `timing`, so the drop is the MCP library's own handling of an undeclared argument, which this test checks on the real server stack. I did not open the library.
- test_ch_kit_frame_probe_passes_its_seconds_through: agree Keep. `server.py:369`.
- test_the_helper_hands_ch_run_chart_its_three_parts: agree Keep. `server.py:89` hands `RunParts(self.frida, self.kit, self.sp)`; red if copies are built.
- test_the_kit_reads_the_pause_setting_in_the_env_documents_folder: agree Investigate, claim not checked only, with a part now checked: the typed file shape (`[game]` section, `pause_on_focus_lost = 1`) matches the user's real `Documents\Clone Hero\settings.ini` (section `[game]` at line 27, the key at line 48), and the key name is `game_files.PAUSE_ON_FOCUS_LOST` (game_files.py:71). The refusal text (`kit_tools.py:1084`) still needs a capture of a real `ch_kit_setup` refusal.
- test_ch_start_takes_human_and_passes_it_through: agree Keep. `server.py:156-175`; default False and the `human=True` keyword reach the tool.
- test_ch_play_chart_passes_its_arguments_through: agree Keep. Order matches the call at `server.py:383-412`; defaults None and 0 (D318 item 4).
- test_ch_open_song_takes_the_player_and_passes_it_through: agree Keep. `server.py:513`.
- test_ch_sp_compare_passes_the_schedule_through_and_takes_no_dump: agree Keep. Required `schedule_json` is D326 item 1; the width default is read from the schema.
- test_ch_compare_takes_the_run_settings_and_passes_them_through: agree Keep. `server.py:228-235`.
- test_ch_start_human_with_no_song_folder_is_refused_before_any_launch: agree Keep. Compares with the owner's `bot_run.HUMAN_NEEDS_SONG`; `_no_launch` fails on any launch.
- test_ch_kit_setup_takes_player_and_passes_it_through: agree Keep. `server.py:309-324`.
- test_ch_kit_press_takes_value_and_passes_it_through: agree Keep. The six-argument order `items, hold_frames, rewrite, on_main, read, value` (`server.py:327-340`) is what lines 366-367 expect.
- test_ch_kit_press_says_what_value_does: agree Rewrite. Line 371 is a phrase check; the description at `server.py:335` says "value sets".
- test_status_joins_the_game_and_the_frida_lines: agree Investigate, claim not checked only. Texts match `frida_tools.py:643-646` and `game_tools.py:848`.
- test_the_ch_entry_runs_the_front_and_the_worker_without_dash_i: DISPUTE. Investigate -> Keep. The reason exists (README:77, checked: finding 6) and the input is the real `.mcp.json`, where the `hydra` entry beside it does carry `-I`. Line 385 is a pin of the file, so a legitimate edit turns it red; the README says the file is the place the reasons are written down.

## Counts

Records checked: 44. Agreed: 32 (6 in test_mono_facts.py, 26 in test_server.py). Disputed: 12 (8 in test_mono_facts.py, 4 in test_server.py). Test cases without a record: none (this file's 44 plus the 124 reached in `verify/D8.md` make the 168).
