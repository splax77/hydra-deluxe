# Script-first recheck: four questions, one per kind of flaw

The November 14 audit answered two questions for every test. Line 9 asked "does it use a stand-in?" and line 11 asked "is its input real?". A script found every stand-in in the tests at commit `539c6de0` and sorted each one by what it does to the test. `kinds.py` printed the counts below, and `kinds.md` holds the full list of tests for each kind.

A record is flagged when its line 11 starts "yes" or its line 9 starts "none". A test can have more than one kind, so the same test can appear under two questions.

Your words beside every question: "a fake server doesnt tell you anything about real data".

Each question asks the same thing. When a test's checks rely on a stand-in of this kind, does its record's "Real: Yes" or "Stand-in: None" stand, or is it contradicted? You may name tests where your answer does not hold.

## fake data

The stand-in hands the code under test a value, reply, file or setting in place of what the real code would get.

tests: 89
flagged records: 69

Wrong end. The update tests' fetcher returns a canned reply, and the record says no stand-in.
example code: tests/test_update_state.cpp:100 | return canned.reply;
example record: records/C13.md:579 | 9 Stand-in: None; one long test carries four claims

Right end. The record names the seam and says the installed case is not exercised.
example code: tests/test_update_release.cpp:228 | ScopedHook installed(hydra::app::set_installed_for_test, std::optional<bool>(true));
example record: records/C13.md:524 | 9 Stand-in: Line 226 stands on the machine's registry.

Between. The test stamps an old build onto a real chart's record, so it looks like one a user's older library holds. The record says no stand-in.
example code: tests/display_fixtures.h:100 | store.save_under_build_for_test(prepared, kOldBuildStamp);
example record: records/A4.md:107 | 9 Stand-in: none.

Full list: the "fake data" section of `kinds.md`. No unsorted hit is nearest this kind.

## forced wait

The stand-in holds a real step until the test lets it go, and hands back nothing of its own.

tests: 71
flagged records: 55

Wrong end. A hook holds the startup open until the app closes, and the record says no stand-in.
example code: tests/test_app_library.cpp:1220 | wait_until(cancelled, "the closing app to cancel the open");
example record: records/A6.md:457 | 9 Stand-in: none; leans on `wait_until`'s cap.

Right end. The record says the waits use a hook.
example code: tests/test_app_reports.cpp:73 | wait_until([&release] { return release.load(); }, "the test to release the report");
example record: records/A7.md:48 | Waits use a hook and `wait_until`; failure messages name the step.

Full list: the "forced wait" section of `kinds.md`. These unsorted hits are nearest this kind. Each is `SaveGate` in `tests/test_app_click.cpp` (it waits), reached through a hook word passed as an argument or a constructor parameter, which the rule does not follow:
unsorted: tests/test_app_click.cpp:494
unsorted: tests/test_app_click.cpp:503
unsorted: tests/test_app_click.cpp:521
unsorted: tests/test_app_click.cpp:533
unsorted: tests/test_app_click.cpp:540
unsorted: tests/test_app_click.cpp:562
unsorted: tests/test_app_click.cpp:594

## forced failure

The stand-in makes a real step fail or throw.

tests: 135
flagged records: 113

Wrong end. The analyzer is replaced by one that only throws, and the record says no stand-in.
example code: tests/test_app_click.cpp:789 | throw error;
example record: records/A5.md:320 | 9 Stand-in: none.

Right end. The test first gets the real error from a real unreachable address, then throws that error. The record names the fake throw.
example code: tests/test_library_jobs.cpp:1785 | [failure](const std::string&, const std::atomic<bool>*) -> std::string { throw failure; });
example record: records/A16.md:430 | 9 Stand-in: The fake throw stands in for a real unreachable server.

Full list: the "forced failure" section of `kinds.md`. These unsorted hits are nearest this kind. The first is a `start_redo_batch` caller at `tests/test_app_library.cpp:763` whose lambda throws on every path, but in two statements. The other two are `counting_factory`, which counts a call and returns the answer of a captured factory that throws:
unsorted: tests/app_state_fixtures.h:196
unsorted: tests/test_preview_controller.cpp:891
unsorted: tests/test_preview_controller.cpp:953

## test code run mid-step

The stand-in runs the test's own code in the middle of a real step. It does not wait, fail or hand anything back.

tests: 26
flagged records: 19

Wrong end. The hook saves a second record between a report's two reads, and the record says no stand-in.
example code: tests/test_app_reports.cpp:875 | hydra::test::save_batch_record(*store, md5, profile);
example record: records/delta-DL3.md:534 | 9 Stand-in: none.

Right end. The hook only counts lookups, and the record says so.
example code: tests/test_library_jobs.cpp:855 | job.set_lookup_hold_for_test([&lookups] { ++lookups; });
example record: records/delta-DL4.md:65 | 9 Stand-in: a hold hook counts lookups

Full list: the "test code run mid-step" section of `kinds.md`. No unsorted hit is nearest this kind. One known miss: `tests/test_app_library.cpp:883` waits through `testwait::tick_until_thrown`, which the rule's list of waits does not name, so it is counted here. You chose to leave the rule.

Left unasked, for your approval: 18 hit lines in 15 tests replace nothing. They are getters, hooks cleared with `{}`, and the real analyzer passed straight through. They are listed in the "replaces nothing" section of `kinds.md`. These 27 seam names are declared in `src/` and named in `tests/` but have no hit:
uncovered: set_all
uncovered: set_aside_old_library
uncovered: set_aside_path
uncovered: set_charts
uncovered: set_chip
uncovered: set_gain
uncovered: set_ghost_path
uncovered: set_hardest
uncovered: set_headless
uncovered: set_keep
uncovered: set_lane_flag
uncovered: set_path_overrides
uncovered: set_problem
uncovered: set_program
uncovered: set_query
uncovered: set_render_device
uncovered: set_scene
uncovered: set_search
uncovered: set_song_length_ms
uncovered: set_sort
uncovered: set_sqout
uncovered: set_status
uncovered: set_summaries
uncovered: set_summaries_for
uncovered: set_summary_for
uncovered: set_ui_scale
uncovered: set_volume
status: open
