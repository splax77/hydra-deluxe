## every start of the installed exe checks, and a copy never does
claim: "every start of the installed exe checks, and a copy never does" | from: name tests/test_update_state.cpp:180 | rests on: a name | facts: 3, 8, 14 | result: the facts back this
claim: "Each start checks: no time since the last check holds one back." | from: comment tests/test_update_state.cpp:183 | rests on: a comment | facts: 3 | result: the facts back this
claim: "before the first tick" | from: comment tests/test_update_state.cpp:187 | rests on: a comment | facts: none | result: no fact backs this
claim: "Settings as a start at the running version leaves them, so no" | from: comment tests/test_update_state.cpp:119 | rests on: a comment | facts: 18 | result: the facts back this
claim: "Each start of the installed Hydra checks for updates at once and the bar says" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:554 | rests on: nothing | facts: 3, 4 | result: the facts back this
claim: "a copy that was not installed never checks and says only the installed copy does." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:554 | rests on: nothing | facts: 8, 11 | result: the facts back this
claim: "the check is running before the first tick" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:557 | rests on: nothing | facts: 3 | result: the facts back this
claim: "Typed sentences copied from D260." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:557 | rests on: nothing | facts: none | result: no fact backs this
claim: "The five ticks at 182 have no reason written" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:558 | rests on: nothing | facts: none | result: no fact backs this
claim: "the check starts in the same call" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:558 | rests on: nothing | facts: 8 | result: the facts back this
claim: "so the typed side has a source, but not the engine's." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:559 | rests on: nothing | facts: 4, 11 | result: the facts contradict this
claim: "A once-a-day wait holding back the check" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:560 | rests on: nothing | facts: none | result: no fact backs this
claim: "The five-tick loop stands in for "never, however long" with no reason for 5." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:562 | rests on: nothing | facts: none | result: no fact backs this
open: "every start of the installed exe checks, and a copy never does" | settled by: a run of the installed exe and a plain copy for longer than five ticks, reading update_check_running each tick

## a newer release is offered; Later keeps an Update button and writes nothing
claim: "a newer release is offered; Later keeps an Update button and writes nothing" | from: name tests/test_update_state.cpp:214 | rests on: a name | facts: 22, 31-34, 27, 30 | result: the facts back this
claim: "The offer of the canned v9.9.9, from the start-up check." | from: comment tests/test_update_state.cpp:166 | rests on: a comment | facts: 22, 40 | result: the facts back this
claim: "the phase draws its own three" | from: comment tests/test_update_state.cpp:224 | rests on: a comment | facts: 26 | result: no fact backs this
claim: "The start at the running version wrote no INI; Later writes none either." | from: comment tests/test_update_state.cpp:226 | rests on: a comment | facts: 27, 30, 44 | result: the facts back this
claim: "The resting bar's Update starts the download as the offer's does." | from: comment tests/test_update_state.cpp:245 | rests on: a comment | facts: 37-39 | result: the facts back this
claim: "The download's body: copies sample.bin unless replaced." | from: comment tests/test_update_state.cpp:61 | rests on: a comment | facts: 41 | result: the facts back this
claim: "with a process that never ends, unless" | from: comment tests/test_update_state.cpp:70 | rests on: a comment | facts: 42 | result: no fact backs this
claim: "A newer release is offered with an amber dot" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:571 | rests on: nothing | facts: 24, 25 | result: no fact backs this
claim: "after Later the bar still says it is available with an Update button" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:571 | rests on: nothing | facts: 31-34 | result: the facts back this
claim: "nothing is written to the settings file" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:571 | rests on: nothing | facts: 27, 30 | result: the facts back this
claim: "that Update button starts the download." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:571 | rests on: nothing | facts: 37-39 | result: the facts back this
claim: "checked on a scratch INI (`ScratchPaths`), not a real user settings file" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:573 | rests on: nothing | facts: 44 | result: the facts back this
claim: "Lines 209-216: the INI is unchanged after Later." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:574 | rests on: nothing | facts: 30 | result: the facts back this
claim: "Update downloads and starts the installer from the made-up folder." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:574 | rests on: nothing | facts: 39, 43 | result: the facts back this
claim: "Typed texts from D260." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:574 | rests on: nothing | facts: none | result: no fact backs this
claim: ""Later still writes nothing to the INI" is stated there too." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:575 | rests on: nothing | facts: none | result: no fact backs this
claim: "the engine value shown (the version 9.9.9) is read from the release (201)." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:576 | rests on: nothing | facts: 23 | result: the facts back this
claim: "one long test carries four claims" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:579 | rests on: nothing | facts: none | result: no fact backs this
open: "A newer release is offered with an amber dot" | settled by: a search of the version bar's draw code for the colour of Dot::Available, or a capture of the bar at an offer

## Skip this version names the release as skipped; a newer one is offered
claim: "Skip this version names the release as skipped; a newer one is offered" | from: name tests/test_update_state.cpp:254 | rests on: a name | facts: 49-53, 59-60 | result: the facts back this
claim: "no button, so nothing starts" | from: comment tests/test_update_state.cpp:266 | rests on: a comment | facts: 53, 54 | result: the facts back this
claim: "A fresh start honors the skip and still names the release." | from: comment tests/test_update_state.cpp:269 | rests on: a comment | facts: 55-58, 63 | result: the facts back this
claim: "The next start with 9.9.10 out offers it." | from: comment tests/test_update_state.cpp:281 | rests on: a comment | facts: 59, 60, 62 | result: the facts back this
claim: "the bar still names the release with "Skipped.", no button" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:587 | rests on: nothing | facts: 50-53 | result: the facts back this
claim: "the skip is saved and a fresh start honors it" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:587 | rests on: nothing | facts: 49, 55-58 | result: the facts back this
claim: "a newer release is offered again." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:587 | rests on: nothing | facts: 59, 60 | result: the facts back this
claim: "Settings claim checked on a scratch INI only" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:589 | rests on: nothing | facts: 63 | result: the facts back this
claim: "the INI holds "9.9.9"" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:590 | rests on: nothing | facts: 49 | result: the facts back this
claim: "and Update does nothing." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:590 | rests on: nothing | facts: 54 | result: the facts back this
claim: "Lines 252-260: a fresh start reads the same." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:590 | rests on: nothing | facts: 55-58 | result: the facts back this
claim: "Lines 263-267: 9.9.10 is offered." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:590 | rests on: nothing | facts: 59, 60, 62 | result: the facts back this
claim: "D260's item 2 text (cut off in my output; the full item was not read)." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:591 | rests on: nothing | facts: none | result: no fact backs this

## the same or an older release lands in the resting Up to date
claim: "the same or an older release lands in the resting Up to date" | from: name tests/test_update_state.cpp:289 | rests on: a name | facts: 67-71, 75, 76 | result: the facts back this
claim: "When the latest release is the running version or older, nothing is offered" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:603 | rests on: nothing | facts: 67-70, 75, 76 | result: the facts back this
claim: "the bar reads "Up to date"" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:603 | rests on: nothing | facts: 71 | result: the facts back this
claim: "with a green dot." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:603 | rests on: nothing | facts: 73 | result: no fact backs this
claim: "For the two tags (the running version and v0.0.1)" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:606 | rests on: nothing | facts: 75, 76 | result: the facts back this
claim: "phase none, no text, no release or found" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:606 | rests on: nothing | facts: 67-70 | result: the facts back this
claim: "v0.0.1 is the test's own "older"." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:607 | rests on: a name | facts: 76 | result: the facts back this
open: "with a green dot." | settled by: a search of the version bar's draw code for the colour of Dot::UpToDate, or a capture of the bar

## a failed start-up check shows in the resting status with its plain_error
claim: "a failed start-up check shows in the resting status with its plain_error" | from: name tests/test_update_state.cpp:310 | rests on: a name | facts: 86, 87 | result: the facts back this
claim: "The errors the transport throws for GitHub, and the sentences they show." | from: comment tests/test_update_state.cpp:312 | rests on: a comment | facts: 87, 90-92 | result: the facts back this
claim: "The fetch waits for `go` before it throws, so the running state can" | from: comment tests/test_update_state.cpp:323 | rests on: a comment | facts: none | result: no fact backs this
claim: "the bar says "Hydra couldn't check for updates when it started."" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:619 | rests on: nothing | facts: 86 | result: the facts back this
claim: "with GitHub's plain sentence after it" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:619 | rests on: nothing | facts: 87 | result: the facts back this
claim: "while the fetch is held, it reads "Checking for updates..."." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:619 | rests on: nothing | facts: 81, 82 | result: the facts back this
claim: "the HTTP error is a thrown `KindedError`, not a saved real reply" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:621 | rests on: nothing | facts: 90, 92 | result: the facts back this
claim: "Lines 312-319: while held, running and "Checking..."." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:622 | rests on: nothing | facts: 81, 82 | result: the facts back this
claim: "The two sentences are typed." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:622 | rests on: nothing | facts: 91 | result: the facts back this
claim: "The sentences are from D259's "Six rulings during the build"" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:623 | rests on: nothing | facts: none | result: no fact backs this
claim: "The loop of 3 ticks at 313 has no reason." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:623 | rests on: nothing | facts: none | result: no fact backs this
claim: "The three ticks stand in for "the check is still held"" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:627 | rests on: nothing | facts: none | result: no fact backs this
claim: "so `update_check_running()` is true until the gate opens." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:630 | rests on: nothing | facts: none | result: no fact backs this
open: "with GitHub's plain sentence after it" | settled by: a capture of the bar after a real failed fetch, and a saved real HTTP 403 body from GitHub

## the Updated note's OK offers a found release, or gives way to the resting status
claim: "the Updated note's OK offers a found release, or gives way to the resting status" | from: name tests/test_update_state.cpp:354 | rests on: a name | facts: 98-99, 101-108 | result: the facts back this
claim: "the "Updated to ..." note is shown first" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:636 | rests on: nothing | facts: 96, 109 | result: the facts back this
claim: "its OK then offers a found newer release" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:636 | rests on: nothing | facts: 98, 99 | result: the facts back this
claim: "or shows the skipped or up-to-date resting status." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:636 | rests on: nothing | facts: 101-104, 106-108 | result: the facts back this
claim: "note shown (342, neutral dot 343)" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:639 | rests on: nothing | facts: 96, 97 | result: the facts back this
claim: "with a skip it gives the skipped bar (352-358)" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:639 | rests on: nothing | facts: 101-104, 110 | result: the facts back this
claim: "with an equal release it gives Up to date (365-369)" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:639 | rests on: nothing | facts: 105-108 | result: the facts back this
claim: ""0.0.1" is the test's own earlier version." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:640 | rests on: a name | facts: 109 | result: the facts back this

## named_version is the app's name and the version
claim: "named_version is the app's name and the version" | from: name tests/test_update_state.cpp:394 | rests on: a name | facts: 115 | result: the facts back this
claim: "The one text of a version" | from: comment tests/test_update_state.cpp:392 | rests on: a comment | facts: 115 | result: the facts back this
claim: "The text of a version is "Hydra Deluxe" and the version." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:652 | rests on: nothing | facts: 115 | result: the facts back this
claim: "the text for 2.11.1 is "Hydra Deluxe 2.11.1", typed." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:655 | rests on: nothing | facts: 115, 116 | result: the facts back this
claim: "2.11.1 is the test's own input." | from: record docs/audit/2026-11-14-test-audit/records/C13.md:656 | rests on: nothing | facts: 116 | result: the facts back this
claim: "The name "Hydra Deluxe" is the visible name" | from: record docs/audit/2026-11-14-test-audit/records/C13.md:656 | rests on: nothing | facts: none | result: no fact backs this

## test_panel_close_cancels
claim: "test_panel_close_cancels" | from: name tests/ui/uitest_details.cpp:1079 | rests on: a name | facts: 120-124 | result: the facts back this
claim: "the analysis is provably still running at the close" | from: comment tests/ui/uitest_details.cpp:1084 | rests on: a comment | facts: 125, 127 | result: the facts back this
claim: "closing the panel mid-analysis cancels it, and nothing is saved for the song" | from: record docs/audit/2026-11-14-test-audit/records/A39.md:389 | rests on: nothing | facts: 120-124 | result: the facts back this
claim: "Nothing is saved for the cancelled song" | from: record docs/audit/2026-11-14-test-audit/records/A39.md:391 | rests on: nothing | facts: 123, 124 | result: the facts back this
claim: "If you close the panel while the click's analysis runs, the analysis stops" | from: record docs/audit/2026-11-14-test-audit/records/A39.md:392 | rests on: nothing | facts: 120, 121, 125 | result: the facts back this
claim: "the viewed song is cleared" | from: record docs/audit/2026-11-14-test-audit/records/A39.md:392 | rests on: nothing | facts: 122 | result: the facts back this
claim: "the first row stays "Not analyzed" with no record saved." | from: record docs/audit/2026-11-14-test-audit/records/A39.md:392 | rests on: nothing | facts: 123, 124 | result: the facts back this
claim: "The test reads `view_row_status` and `store->record_count()`." | from: record docs/audit/2026-11-14-test-audit/records/A39.md:393 | rests on: nothing | facts: 123, 124 | result: the facts back this
claim: "the store is a scratch store, not a real library database" | from: record docs/audit/2026-11-14-test-audit/records/A39.md:394 | rests on: nothing | facts: none | result: no fact backs this
claim: "1091 the job settles." | from: record docs/audit/2026-11-14-test-audit/records/A39.md:395 | rests on: nothing | facts: 121 | result: the facts back this
claim: "0 records is the claim itself." | from: record docs/audit/2026-11-14-test-audit/records/A39.md:396 | rests on: a claim word | facts: 124 | result: the facts back this
claim: "`record_count() == 0` and row status together stand for "nothing saved"." | from: record docs/audit/2026-11-14-test-audit/records/A39.md:400 | rests on: nothing | facts: 123, 124 | result: the facts back this
claim: "test_view_cancel_try_again :1118-1119 states the same "nothing saved" for Cancel." | from: record docs/audit/2026-11-14-test-audit/records/A39.md:401 | rests on: nothing | facts: none | result: no fact backs this
open: "the store is a scratch store, not a real library database" | settled by: a search of the uitest harness reset_app and scan_library for how the store is opened, plus a run against the real library's saved rows
count: records the facts contradict: 1 | records resting on a comment, name or claim word: 3 | total records in either count, each counted once: 4
