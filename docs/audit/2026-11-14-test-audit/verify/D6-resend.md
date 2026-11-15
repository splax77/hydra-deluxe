Agent aeebab63ea6b08978 (T5-V-RESEND-D6), re-send verifier for slice D6. "By reading" means I read the code and it holds. I ran nothing, so nothing here is "proven by" a run.

# Verify D6, re-send

## What I covered

The first verifier (`verify/D6.md`) left 8 records of `test_launch_core.py` and 125 of `test_kit_tools.py` unreached: 133 in all. I opened every one of those test bodies line by line (`test_launch_core.py` in full; `test_kit_tools.py` lines 1-1814, including the fake game driver at :1451-1562). I opened this production code: `launch_core.ts` 40-198 (the whole decision), `kit_tools.py` 48-135 and 255-352 (constants, `pad_axis`, `press_message`), `lane_velocity_337E10.asm` 20-124, and SC-13 in `docs/clone-hero/06-scoring.md` 145-153.

What I did not open, so I could not compare it: `kit.ts`, `kit_tools.py` from line 355 on (the printing code behind the long typed sentences), `play_chart.py`'s `play_chords`, `frida_attach.py`. For every record that types a printed sentence, I read the test and agree or dispute on the test's own faults and its question 3. I did not check each typed sentence against the production string. A wording mismatch would turn the test red, which the main session's build will show.

Every record in this part is "Investigate, claim not checked only", with two exceptions the auditor made (two Removes of `:794`-style repeats, below). None can be Keep: the real thing for each is a saved real reply, a real settings file or the real game, and none is in `records-engine/` or `records-screen/`.

## Counts

Records checked: 133 (8 + 125). Agreed: 132. Disputed: 1 (`test_a_ghost_makes_a_byte_of_the_threshold_minus_one`). Not reached: none. Test cases with no record: none (the first verifier's count of 168 stands; every name below has a record heading in `records/D6.md`).

## test_launch_core.py (8 records)

All eight agree with "Investigate, claim not checked only".

- `test_matching_bytes_write_once_then_hook` (:115): by reading, `applyPatch` (`launch_core.ts:161-198`) reads entry, reads site, writes, reads back, hooks, in that order, and the call list at :127-133 matches it. The status fields are the ones `Status` holds (:43-53). Question 3 stays open: the memory is `Map`-backed (driver :40-57).
- `test_a_differing_entry_writes_nothing_and_hooks_nothing` (:135): by reading, `ts:169-180` reads both runs before the compare, so two reads and no write is right; the reason text (`ts:175-178`) holds the found bytes, the expected bytes and "nothing was written".
- `test_a_differing_patch_site_writes_nothing_and_hooks_nothing` (:147): same branch (`ts:173`); agree.
- `test_a_short_read_is_a_difference` (:157): by reading, `same()` (`ts:71-73`) compares length first, so a two-byte read is refused. The record's "red if the comparison used `startsWith`" holds.
- `test_a_write_that_does_not_take_is_refused_and_not_hooked` (:162): by reading, `ts:181-189`; `patched_at_ms` stays null because it is set after the read-back (`ts:190`).
- `test_a_failing_read_is_refused_with_its_error` (:170): by reading, the `catch` at `ts:193-196` builds "patching ... failed: Error: read refused"; one read call because the throw is on the first read.
- `test_no_status_carries_an_error_key` (:177): agree. By reading, `test_frida_attach.py:1424-1448` runs the same five statuses (it imports `LC.run` and `LC.apply`) through `LiveAgent`, so the consumer side is tested there and the record's overlap note is right. The `reason` presence check at :191 holds for the four apply statuses. For "waiting" only the absence of `error` is checked (:192), as the record says.
- `test_the_loader_runs_are_counted_and_the_first_reads_the_site` (:194): by reading, `loaderRan` (`ts:143-154`) reads the site on run 1 only. The 2 is the test's own `loader_runs=2`, so the count check is the driver's loop counting, which is the production counter.

## test_kit_tools.py (125 records)

### ConstantsTest

- `test_items_take_the_kit_prefix_and_refuse_unknown_names` (:318): agree. The expected list is typed. `parse_items` is not opened by me (caller `menu_tools.py:228`). The record's real input, a real `menu_steps.json`, is a plain input file and would finish question 3.

### SetupTest

All agree, Investigate: `test_refused_without_ch_attach`, `test_refused_when_the_settings_line_is_1`, `test_refused_when_the_settings_line_is_missing`, `test_a_stale_or_missing_kit_agent_is_refused_before_any_attach`, `test_backup_then_attach_then_setup_and_the_flags_are_printed`, `test_a_map_that_differs_is_refused_naming_the_element_and_torn_down`, `test_a_keyboard_left_on_is_refused_and_torn_down`, `test_a_mouse_left_on_is_refused_and_torn_down`, `test_both_left_on_name_both_in_one_refusal`, `test_a_reply_with_no_keyboard_or_mouse_read_back_is_refused`, `test_a_setup_reply_missing_mouse_now_is_refused_and_torn_down`, `test_a_disabled_map_or_a_missing_binding_is_a_difference`, `test_the_definition_1_map_passes_both_checks`, `test_the_ls_d2b_run1_map_is_refused_naming_kick_7_and_element_22`, `test_a_button_binding_on_the_kick_is_refused_naming_button_and_definition_1`, `test_every_element_is_type_checked_not_only_the_kick`, `test_a_binding_with_no_element_type_is_refused`, `test_an_element_bound_as_both_kinds_is_refused_as_a_button`, `test_a_setup_with_no_reply_ends_at_its_cap_and_tears_down`.

Notes by reading. `:329`, `:333` and `:339` each check that nothing happened before the refusal (`self.events == []`), which can fail. `:345` rewrites the real `kit.ts` stamp check on a temp copy; `BS.check(path)` is the owner's own function, so the second half of that test (:355) compares production with production: a copy, but the first half (:350-352) types the sentence. `:453` hardcodes 10 (the binding count) where the table has `len(K.KIT_BINDINGS)`. `:469` takes `OLD_BINDINGS` typed in this file from the ls-d2b-run1 reply (the record's real input: that run's `replies.txt`, not opened by me). `:502` and `:508` produce the exact sentence of `binding_problems`; whether an element can be bound as both kinds in a real map is not shown anywhere I looked (the record's own note). `:515` is a timing claim run on `FakeClock` (`sleep` moves it), not the real clock; the brief asks for the real mechanism, so "claim not checked" stands.

### SetupForRunTest

All agree, Investigate: `test_a_setup_that_passes_gives_the_game_player_and_the_reply`, `test_a_setup_that_is_refused_gives_no_player`, `test_a_setup_on_the_wrong_id_is_torn_down_and_gives_no_player`, `test_a_kit_already_set_up_gives_no_player_and_stays`.

`test_the_kit_tools_lend_no_press_and_the_harness_press_stays` (:553): agree, Investigate. By reading, `hasattr(K.KitTools, "kit_press")` is a structure pin on a name, not behaviour. The first verifier quoted D342 as the user's "Yes, plan IG6", which backs the design claim; it is a decision-log line, not a filed ruling.

### MapProblemsTest

All agree, Investigate, except one Remove that I also agree with: `test_no_map_names_the_ids`, `test_a_map_at_layout_0_is_refused_naming_both`, `test_the_right_map_disabled_is_refused`, `test_two_enabled_maps_at_the_ids_are_refused`, `test_setup_on_system_with_no_map_is_refused_and_torn_down`.

`test_the_drum_map_passes` (:590): agree, Remove. By reading, :591 is `map_problems(setup_reply()["maps"]) == []` and :466 asserts `K.map_problems(maps) == []` on the same fixture inside `test_the_definition_1_map_passes_both_checks`, so the removal loses nothing.

### SetupPlayerTest

All agree, Investigate: `test_no_player_sends_player1`, `test_a_given_player_is_sent_as_is_and_named_first`, `test_a_name_no_player_has_is_printed_as_the_agents_refusal`, `test_a_faulted_setup_prints_the_fault_then_tears_down`, `test_a_fault_outside_every_module_prints_a_readable_line`, `test_an_error_with_no_fault_prints_no_fault_lines`, `test_a_refusal_before_the_attach_still_names_the_given_player`.

Notes by reading. `:613` and `:621` rest on the filed ruling the record cites, `docs/agents/rulings/clone-hero.md:79` ("Keep Player1"); I did not re-open it. `:647` expects a register line in a fixed order (:655-657) that is the production formatter's order retyped; the real reply it stands for is `ls-d1a-run1\replies.txt`, not opened. In `:680`, the second assertion (:687) can only fail if the first (:686) already failed; it adds nothing.

### PressTest

All agree, Investigate: `test_green_and_kick_send_elements_3_and_7_with_the_hold`, `test_a_given_hold_is_sent`, `test_a_reply_that_never_comes_ends_at_the_press_cap`, `test_refused_before_setup_and_while_a_play_session_runs` (:724, the PressTest one; the FrameProbeTest twin is :1164, listed below).

By reading, `type(...) is float` at :706 and :715 is a real property check (`True == 1.0` in Python) and can fail. `:717` is a timing claim on the fake clock: "claim not checked" on the real mechanism.

### PressOptionsTest

All agree, Investigate: `test_a_press_with_read_sends_the_names_and_prints_seen`, `test_read_takes_a_list_each_name_once_and_refuses_a_name_that_is_not_text`, `test_none_sends_the_constants_and_a_given_value_overrides_them`, `test_rewrite_and_on_main_take_only_true_or_false`.

### PadAxisTest

By reading, `pad_axis` (`kit_tools.py:274-295`) is `axis(v) = 0.5 + v/254`, accent and plain at v = 127, ghost at v = threshold - 1, and it refuses a threshold outside 1 to 127.

The game's own code states both ends. `lane_velocity_337E10.asm` lines 85-96 and 102 are `subss 0.5; addss (doubles); mulss 127.0; Math.Round`, which is `round((axis - 0.5) * 2 * 127)`, the test's `byte()` at :786-787 (`(axis - 0.5) * 254`). SC-13 (`06-scoring.md:149`, Label "Proven in the game's code") says "The bot writes 127 for an accent and the threshold minus 1 for a ghost" and "The MIDI handler turns velocity v into axis 0.5 + v / 254". That is an independent owner for the formula and for the ghost value, with a live threshold read of 100 in the same entry.

- `test_a_ghost_makes_a_byte_of_the_threshold_minus_one` (:798): DISPUTE. Verdict should be Keep, not Investigate. Its three assertions (:799-801) are `pad_axis("ghost", t)` through the asm's formula, for t = 1, 64, 127, and the expected side (threshold minus 1) is SC-13's statement of the bot, which is the game's code and a different file from the test. The record said the asm was "not opened"; it is open now and agrees. Not a Keep for the plain gem: see the next entry.
- `test_an_accent_and_a_plain_gem_press_at_full_strength_for_any_threshold` (:789): agree, Investigate, with a correction. The accent half is covered by SC-13 (the bot writes 127; 0.5 + 127/254 = 1.0). The plain half is not: its only source is the comment at `kit_tools.py:267-273` ("A plain gem presses at the accent's velocity"), and SC-13's own text says the game then marks such a plain gem's hit as accent, with whether that pays still the open question Q-51. A choice made in a comment is not an owner.
- `test_a_plain_or_accent_gem_needs_no_threshold` (:794): agree, Remove. By reading, `pad_axis` never reads `threshold` for "none" or "accent" (`kit_tools.py:285-286`), and :789 already makes both calls. One loss: :789 would stay green if the `= None` default were taken away, :794 would not.
- `test_a_threshold_with_no_ghost_strength_is_refused_naming_the_field` (:803): agree, Investigate. By reading, the message holds "OFF_DYN_THRESHOLD" and the threshold's text for 0, 128 and None (`kit_tools.py:290-294`). The range 1 to 127 is the byte range of the asm (round of a 127-scaled value), which SC-13 does not state as a threshold range, so that part rests on the code's own wording.
- `test_a_word_hydra_replay_never_writes_is_refused` (:810): agree, Investigate. The word list owner named in the code is `dynamic_str` in `src/core/model.cpp` (`kit_tools.py:260-261`); I did not open it.

### StrengthTest

All agree, Investigate: `test_a_mixed_chord_carries_one_strength_per_lane_the_kick_included`, `test_a_kick_with_no_value_gives_element_7_at_the_plain_value`, `test_start_and_select_press_at_full_whatever_value_is_given`, `test_no_press_message_writes_true`, `test_a_list_of_values_of_another_length_is_an_error`, `test_two_groups_carry_then_with_after_ms_8`, `test_a_bad_later_group_is_refused_before_anything_is_sent`, `test_the_fakes_send_the_real_reply_shape`, `test_ch_kit_press_fails_on_a_reply_without_its_frames`, `test_the_kit_press_sends_a_split_chord_in_one_message`, `test_the_kit_press_sends_the_chords_axes`, `test_ch_kit_press_sends_a_given_value_to_the_pads_and_kick_and_full_to_start`, `test_a_value_at_either_end_is_sent`, `test_a_value_outside_0_to_1_is_refused_and_nothing_is_sent`.

Agreed Removes: `test_a_red_ghost_and_a_kick_ghost_give_elements_0_and_7_at_the_ghost_value` (:828) and `test_a_press_with_no_values_presses_pads_as_plain_gems` (:860). By reading, `element_list` (`kit_tools.py:321-330`) treats every element the same way: a value from the list, or `pad_axis("none")` when none is given, so a kick has no branch of its own. :817 and :835 already cover the ghost kick and the plain default. Both Removes lose nothing.

Notes by reading. `test_no_press_message_writes_true` (:846) is a real property over every element and four value forms. `test_the_kit_refuses_a_repeated_element` (:885): agree, and the last line (`self.assertIsNotNone(tools)`, :899) is dead, as the record says. `test_two_groups_carry_then_with_after_ms_8` (:868): the 8.0 is the test's own input, not a production constant; I found no filed ruling for it. `play_chart.py` carries each press's own `after_ms` from the schedule (:160, :313), so the 8 ms is a schedule value and the test only needs a value above 0. `test_a_value_outside_0_to_1...` (:943): I did not open the production check; `True` and `"0.6"` as refused values are the type half of it.

### WatchTest

All agree, Investigate: `test_a_watch_with_no_true_read_says_no_key_reached_a_player`, `test_a_true_read_is_a_failure_naming_the_frame_and_the_reader`, `test_refused_while_a_play_session_runs_before_setup_and_on_a_bad_length`, `test_a_watch_that_sampled_nothing_is_a_failure_not_a_pass`, `test_a_reply_with_listed_reads_fails_even_if_its_count_says_zero`, `test_a_watch_is_allowed_at_the_cap_and_refused_just_above_it`, `test_a_watch_with_no_reply_ends_at_its_cap`.

The 60 s cap (`kit_tools.py:112-115`) has the user's answer in a session card, `docs/sessions/2026-10-31-a5f17ca5.md:29`: the user chose "Cap at 60 s (Recommended)" for `ch_kit_watch`. That is a card line, not a filed ruling. `:1014` is a timing claim on the fake clock.

### FrameProbeTest

All agree, Investigate, except one Rewrite I also agree with: `test_the_probe_cap_is_the_users_five_seconds`, `test_the_probe_sends_frame_probe_with_the_engines_clock_address`, `test_the_whole_reply_is_saved_to_a_new_file_each_probe` (agree with the first verifier's dispute, below), `test_a_skipped_frame_fails_and_says_where`, `test_any_frame_rate_passes`, `test_a_read_back_that_never_reads_pressed_fails`, `test_any_button_alone_does_not_pass_the_read_back`, `test_a_hook_left_in_fails`, `test_a_failure_fails_and_is_printed`, `test_every_failed_part_is_named`, `test_a_reply_with_no_rows_fails`, `test_more_than_five_seconds_is_refused_before_anything_is_sent`, `test_refused_before_setup_and_while_a_play_session_runs` (:1164), `test_no_engine_found_is_said_and_nothing_is_sent`, `test_a_probe_with_no_reply_ends_at_its_cap`.

`test_a_clean_probe_prints_each_number_and_passes` (:1067): agree, Rewrite. By reading, :1070-1085 is twelve typed lines, many of them wording; the numbers (333.3 frames a second, 1/2/3 ms) are worked from `probe_reply`'s knobs by hand. The fault is in the test (a retyped printout), not only question 3.

`test_the_whole_reply_is_saved_to_a_new_file_each_probe`: agree with the first verifier's dispute of the Keep (same reply object both times, only file 1 read), so not repeated here.

The 5 s cap: the first verifier quoted D339 item 3, "Accept all four (Recommended)", and D341 for the frame rate; I did not reopen them. `:1104` and `:1054` are constant and rule pins backed by decision-log lines, not by a filed ruling.

### GamePlayerTest, GamePlayerFromKitTest, KitPressTest

All agree, Investigate: `test_events_queue_in_the_order_the_agent_sent_them`, `test_a_batch_with_an_error_queues_the_error_after_its_events`, `test_post_waits_the_press_cap_and_close_stops_the_listener`, `test_the_kit_tools_player_is_none_until_setup_and_then_listens`, `test_the_kit_press_reply_rides_into_the_press_row_and_the_log`, `test_a_faulted_kit_press_carries_the_fault_in_its_error`, `test_a_kit_press_error_with_no_fault_is_raised_unchanged`, `test_no_kit_press_before_setup`.

By reading, the first three use `FakeListenAgent` (:1193) with a real `queue`, so the claim is about ordering on the real mechanism with a fake agent. `:1262` runs `play_chart.play_chords` with fake clocks and checks real csv output on a temp file; the frames 1200/1202 and the ms values come from `PRESS_REPLY`, a typed reply.

### StatusTest, TeardownTest

All agree, Investigate: `test_the_frame_rate_comes_from_two_reads_one_second_apart`, `test_no_agent_says_so_and_still_prints_the_foreground_line`, `test_teardown_prints_each_check_and_detaches`, `test_a_mismatch_is_printed_as_a_failure`, `test_a_mouse_mismatch_is_printed_as_a_failure`, `test_refused_while_a_play_session_runs_and_with_nothing_attached`, `test_the_stop_hook_tears_down_then_detaches`.

By reading, in `:1314` the expected 160.0 is (1160 - 1000) over `KIT_RATE_WINDOW_S` = 1.0 (`kit_tools.py:118`) on the fake clock, so it is a rate worked from typed frames. `:1366` and `:1357` share the `teardown_lines` formatter (`kit_tools.py:596`, not opened in full), so the overlap note stands; :1366 differs in that it checks the exact five-line output and the detach line.

### KitIdTest

All agree, Investigate: `test_a_kit_at_id_6_passes_and_the_reply_names_all_seven_ids`, `test_a_kit_at_any_other_id_is_refused_with_the_ids_and_torn_down`, `test_a_reply_with_no_kit_id_is_refused_too`, `test_the_id_refusal_says_a_new_launch_puts_the_kit_back_on_id_6`, `test_a_setup_on_a_kit_already_set_up_does_not_advise_a_teardown_and_setup`.

By reading, the literal 6 and the count of six placeholders are confirmed by `lane_velocity_337E10.asm:52-54` (`mov edx, 0x14; mov r8d, 6; call ControllerHelper$$ContainsController`; 0x14 is 20, which is the driver's `ControllerType.Custom: 20` at test :1533). `:1425` pins `NEW_LAUNCH_NOTE` word for word (`kit_tools.py:78-81`), including "only a new launch", the claim the first verifier found stronger than IN-81 proves (split 2 of `verify/D6.md`). `:1433` pins `KIT_ALREADY_SET_UP` word for word (`kit_tools.py:82-85`); I compared it and the two texts are the same.

### KitBuiltFileTest, KitAgentTest

All agree, Investigate: `test_the_kit_tests_run_the_built_file`, `test_setup_makes_six_placeholders_then_the_kit_at_id_6`, `test_only_the_kit_is_handed_to_a_player`, `test_teardown_destroys_all_seven_and_checks_each`, `test_kit_gone_fails_when_a_placeholder_tag_remains`, `test_kit_gone_fails_when_the_kit_remains`, `test_a_setup_that_throws_after_the_third_placeholder_leaves_three_to_destroy`, `test_a_setup_that_finds_the_kit_creates_nothing`, `test_a_second_setup_in_one_agent_creates_nothing_and_teardown_destroys_seven`, `test_a_setup_after_a_throw_keeps_the_placeholders_it_finds`, `test_a_later_group_goes_down_at_or_after_its_gap_and_each_holds_its_own_frames`, `test_a_press_with_no_later_groups_replies_with_no_groups`, `test_a_later_group_without_a_gap_is_an_error`, `test_a_teardown_with_no_setup_record_finds_the_controllers_by_tag`.

By reading, these run the real built `kit.js` in real Node against `AGENT_DRIVER`, a fake game whose id counter never decreases on destroy (:1456, :1518-1524). The driver's id rule is the claim IN-81 states, copied into the fake; so "ids 0 to 5 then 6" (:1657) shows the agent's create order, not what the real game hands out. `:1696` and `:1722` count creates globally (`creates` at :1457), so `fail_on_create=4` fails the fourth create only and a later setup is not stopped by it; both tests read correctly. `:1729` is a timing claim on the real Node clock with a fake game frame of 3 ms (`frame_ms`, :1461): the gap check (`>= 8`, :1740) is a floor, so a busy machine cannot turn it red. `test_the_kit_tests_run_the_built_file` (:1600) tests the test file's own helpers (`built_kit_module`, `kit_skip_reason`), not production; an auditor may want it Removed, but that is the user's choice and the record's "Investigate" stands.

### KitIdLiteralTest, ReadPauseSettingTest

All agree, Investigate: `test_the_pattern_catches_each_form`, `test_no_literal_0_is_compared_with_the_kits_id`, `test_reads_the_game_sections_line`.

By reading, `:1790` tests a regex defined in the test file; `:1796` scans `kit.ts` and `kit_tools.py` for lines matching it. A text scan stands in for behaviour; the behaviour (id 0 refused as an id when it is not 6) is covered by :1405, which does not prove an id of 0 is read as a real id elsewhere. `:1805` uses a hand-written `settings.ini` with an `[audio]` decoy; the real thing is the game's `settings.ini`, which I did not read.

## Splits

1. `tools/ch_mcp/kit_tools.py:267-273` says a plain gem presses at the accent's velocity; SC-13 (`docs/clone-hero/06-scoring.md:147`) says the game marks a hit at or above the threshold as accent, and leaves whether that pays to Q-51. So the kit's plain gem is an accent press by choice. `:789` pins that choice (new line; the first verifier did not raise it).
2. The first verifier's four splits (`launch` pinned in one test and only its kind in another; `NEW_LAUNCH_NOTE` against IN-81's unexplained reset at `0x1EEB716`; IN-76's definition 6 map against the current definition 1 `KIT_BINDINGS`) stand. I found nothing that contradicts them and added nothing to them beyond the plain-gem line above.

## Rewrites that propose a saved expected-output file

None.

## Not reached

None. Every record named in the first verifier's "Not reached" list is covered above.
