# R4d notes

## DL4 6, tests/test_model.cpp:275 "early-fill window: every default is kDefaultEarlyFillWindowMs"

The test claims that the search settings, an activation, a saved record and the search graph all start from `kDefaultEarlyFillWindowMs` (60.0, `src/core/model.h:112`), and that 60 sits inside 0 to `kHitWindowMs` (85). Four of those are internal structs with no screen. The screen only shows the app-level setting `Settings::early_fill_window_ms` (`src/app/config.h:195`, initialised from the same constant), which the panel's "Early fill window" box edits (`src/ui/settings_bar.cpp:156-167`, input `##earlyfillwindow`). The script therefore shows only the user-facing default and its range. It does not show SearchSettings, Activation, HydraRecord or ScoreGraph defaults: no screen shows those (internal initialisers; the record's window would only appear in a saved result, and no chart is analysed in this run).

Script lines: all of script.txt (one record). Expected output:
- First `text | Analysis settings` and `rect`: button line reads `Analysis settings: defaults`. The label is built by `settings_button_label` (`src/ui/library_dialogs.cpp:131`), "defaults" when nothing differs (line 128).
- After the click, `text | Early fill window` should show the row with 60 and "ms"; the dump line for `##earlyfillwindow` should list it. If the value does not show as text, the dump line may show it; else it is not readable from output (screenshot r4d_panel_default.png shows it).
- After typing 85: button label `Analysis settings: Early fill window 85 ms` (phrase at library_dialogs.cpp:100, format `format_ms_whole`). A different default (e.g. 85) would contradict the claim.
- After typing 60: label back to `defaults`. This is the key check that 60 is the default.
- After 0 then Reset to defaults: label reads `... Early fill window 0 ms`, then `defaults` and box 60 again.
- Range: the typed 85 and 0 are the two ends; the clamp is in `Settings::clamp` with `kMaxEarlyFillWindowMs` (config.cpp:201). I did not add a "type 200" step, so the upper clamp is not shown; add `type ... | 200` if wanted (expected: clamps to the max, label shows it). Check kMaxEarlyFillWindowMs equals 85 against the code before reading it as `kHitWindowMs`.

Labels checked: `Scan library`, `//Scanning charts/Continue` (from ui-testing.md example, not re-grepped in src); `###analysissettings` (settings_bar.cpp:305); `##earlyfillwindow` (settings_bar.cpp:160); `Reset to defaults` (settings_bar.cpp:226); `Early fill window` (library_parts.h:50). The button is drawn by the library toolbar (`library_toolbar.cpp:135`), so it needs the library screen; I scan first to be safe, the brief says it may be needed and I did not confirm it shows without a scan.

Uncertain: `dump $FOCUSED` as a window name (dump takes a name substring, and `$FOCUSED` may not work there); if it fails the script stops at that line. Replace with `dump | earlyfillwindow` if so. `key Escape` closes the popup (ImGui closes popups on Esc; settings_bar.cpp:269-275).
