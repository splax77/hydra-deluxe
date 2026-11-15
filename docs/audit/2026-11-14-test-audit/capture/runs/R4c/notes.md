# R4c notes

MISMATCH. The six DL3 records are tests/test_app_click.cpp lines 549, 636, 658, 697, 708, 720. Each line lands on a TEST_CASE. All six are about one song click's analysis in the song panel. None touches the Analyze library confirm, the running strip or the finished strip. CLEAN.md and RUNS.md call them "Analyze library batch screens", which is wrong. The batch appears only in 549, where the test holds a store save group.

The tests pause the click's worker at a seam (SaveGate, set_view_before_save_check_for_test and set_view_after_save_check_for_test). No script verb can do that, and `open` waits for the click to finish. So the races cannot be reached on screen. The script shows only the end state the tests check: the row reads what the store reads.

DL3 1 (549, click's save does not stop the window while a save group is open): not shown. The held save group exists only through the C++ HeldSaveGroup hook. The script's plain click shows only the Ready end state (`state`, `text | Burnout`).

DL3 5 (636, replaced before its save saves nothing) and DL3 9 (720, replaced during its save, row = store): the script runs `open Burnout`, then `open Acid Romance`. After the second open the panel should show Acid Romance, and the later `text | Burnout` should show a score and path, not "Not analyzed". Burnout reading Not analyzed or Stale would contradict. The race and the "saves nothing" half of 636 are not shown, because the first open is allowed to finish.

DL3 6 (658, a job let go after it wrote still gives the row the store's answer) and DL3 8 (708, closed during its save): the script closes the panel with X##closepanel, searches Burnout and prints its row. Confirm: a score and path, and `expect-not-text Analyzing chart` passes. The exact let-go timing is not shown.

DL3 7 (697, cancelled during save keeps the panel Cancelled): not shown. Cancel is offered only after 0.15 s of analysis (details_panel.cpp 212, 248) and these charts finish faster, so "Analysis cancelled." cannot be reached.

Batch screens, as the brief asked, though no DL3 record needs them: `click Analyze library...`, `text` and `dump //Analyze library` show the confirm box. `Start analyzing`, `wait-idle`, `text` and `dump Hydra | batchdone` show the finished strip. The running strip (##batchstrip: Pause, Stop) is visible only mid-run and needs BatchGate (C++ only), so it is not captured.

Labels checked in src/ui. "Analyze library..." is library_toolbar.cpp:87. Popup "Analyze library" and "Start analyzing" are library_dialogs.cpp:333-397, and the redo checkbox is 371. ##batchdone is 502, "Open report" 517, X##dismissdone 520, "Open automatically" 550. X##closepanel is details_panel.cpp:90, "Analyzing chart" 212, "Analysis cancelled." 248. Titles Burnout and Acid Romance exist in testdata/input song.ini files.

Unchecked: what the confirm shows when two charts already have results (the script clicks two songs first); and whether `type **/##search |` clears the box (the doc says a trailing bare bar clears it).
