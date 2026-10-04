Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task SE1: the Settings owner (findings 322, 311, 31, 56, 138, 139, 72, 66, 68, 134, 200, 199)

Task id: SE1. Base: main at 81a2519. Branch: claude/p7-se1 (worktree `.claude\worktrees\p7-se1`, made as the preamble says).

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, wave 1 table, "SE1 Settings owner". Decisions: D51 Q14 (clamp to the nearest edge of the range the box enforces; volume 0 to 100 everywhere), Q15 (a `#` comment after a value in hydra_settings.ini is ignored) and Q16 (a 1-bar cap stays allowed) in `docs/audit/2026-10-04-phase-7-questions.md`, plus the code-only call for 68 (on/off text is 0 or 1 only). Finding texts: `docs/audit/2026-10-03-derivation-audit.md`, headings `#### 322.` and so on.

## Goal

Every question about a setting gets one answer in `app::Settings`: what each key is called, what range it may hold, what replaces a value outside it, how a line of the file is split, and what text means "on". Today those answers are spread over `load_file`, `save_file`, two UI boxes and `hydra_replay`. After this task, `Settings` owns them and the other places (fixed in waves 2 and 3, not here) call it. T1 forks from your commit and reads your clamp and `parse_bool`, so name them clearly in your report.

## What the code does today

`Settings::load_file` in `src/app/config.cpp` reads each key its own way. It keeps `preview_volume` only inside 0 to 100, `hit_window_ms` only above 0 and `sp_cap` only at 1 or more; anything else keeps the default. It reads `depth_value`, `depth_mode`, `mslimit_value` and `backendlimit_value` with a bare `atoi` and no check, so `depth_value=-1` reaches the search and crashes it (311), `mslimit_value=900` searches at 900 ms (322), and `depth_mode=2` files the result under lens 2 while the search runs by scores (56). It writes `value == "1"` seven times (68). It skips a line only when `#` is its first character, so `view_difficulty=Hard # practice` silently becomes Expert (66). `save_file` types the same 17 key names again (200).

`Settings::backend_limit` takes `std::abs` of the stored value, so `backendlimit_value=-30` shows -30 in the box and hides rows past 30 ms (31). `chartmode_key` pastes `view_difficulty` raw, where `difficulty()` next to it parses the word in any case and falls back to Expert; a key built from an uncleaned word matches no row (134). The 0..500 and -500..500 box ranges in `paths_tab.cpp` and `settings_bar.cpp` are hand copies of `kSqueezeWindowMs` (138). The cap floor of 1 is typed in five places and `hydra_replay`'s message types Clone Hero's 4 by hand (139). The percent-to-gain step (divide by 100) is typed twice in `preview_controller.cpp` (72).

`load_rules_file` in `src/app/rules_file.cpp` splits its lines its own way (cut at any `#`, trim, split at the first `=`, throw on a line with no `=`). Its `key == "..."` chain spells the seven rules names, and `fixed_cap_text` in `src/core/rules.cpp` spells them a second time for the fingerprint (199).

## What changes

**One key table in `Settings`** (`src/app/config.h`/`.cpp`) carries each key's name, its default and its range. `load_file` and `save_file` both walk it, so no key name is typed twice. The ranges are the ones the boxes enforce today, built from the constants in `src/core/model.h`, never from a typed 500 or 4: Path limit (`mslimit_value`) from -`kSqueezeWindowMs` to `kSqueezeWindowMs`; Backend limit (`backendlimit_value`) 0 to `kSqueezeWindowMs`; `depth_value` 0 and up; `depth_mode` 0 or 1; `preview_volume` 0 to 100; `sp_cap` 1 and up; `hit_window_ms` above 0.

**One clamp, run on load**, pulls a value outside its range to the nearest edge (D51 Q14). It is a public member, so the boxes (SE2) and `hydra_replay` (T1) can call the same one later. Two keys keep today's reading for a value below the floor, because existing tests and earlier decisions pin them: `sp_cap` at 0 or junk (and the retired `auto`) reads as the default `kCloneHeroSpCap`, not 1 (the ui-redesign decision 6 and the test named below); `hit_window_ms` at 0 or below reads as the default, because "above 0" has no edge to land on. Say so in the table's comment. A `kMinSpCap` constant beside `kCloneHeroSpCap` would belong in `src/core/model.h`, which you do not own; keep the floor in the table and see Open questions.

**Three small owners.** `backend_limit()` drops `std::abs`; the clamp already made the value non-negative. `chartmode_key()` builds its difficulty word from `difficulty_name(difficulty())`, so an uncleaned word can no longer reach a key. `depth_mode` gets one enum reader on `Settings` that turns the stored int into the search's `DepthMode` (1 is points, anything else scores, as `to_analysis_settings` does today); `to_analysis_settings` calls it, and wave 2 and 3 callers (`within_label`, `batch_settings_summary`) will too. Add one percent-to-gain helper for the volume (a 0..100 percent to a 0.0..1.0 gain) for the Preview to call in wave 2; it is the only new function with no caller in this task, and you say so in a comment.

**One line splitter and one `parse_bool`** go in `src/core/strutil.h`/`.cpp`. The splitter trims, cuts at any `#`, splits at the first `=`, and returns the trimmed key and value, or nothing for a blank, comment-only or `=`-less line. `parse_bool` accepts exactly "0" and "1" and returns absent for anything else. `load_file` and `load_rules_file` both call the splitter and each keeps its own policy for a bad line: settings skip it, rules throw (D51 Q15 makes a trailing `# comment` in hydra_settings.ini ignored, the way the rules file already does). `load_file`'s seven `== "1"` tests become `parse_bool` calls; an absent answer keeps the field as it was.

**One rules field table.** `src/core/rules.h`/`.cpp` gains one (name, member) table of the seven rules fields. `fixed_cap_text` walks it to write the fingerprint, and `load_rules_file` walks it to match keys, so a field added to one side cannot be missed by the other. The fingerprint text must stay byte for byte what it is today (the `%.17g` number form, `sqout_rule=first_note` / `whole_chord`, the line order), or every stored result reads Stale. The retired `auto_cap_ladder` / `auto_budget_s` keys stay accepted and ignored, as today. This task changes no stored result; the pinned fingerprint tests prove it.

No scan row: `tests/test_single_owner.cpp` is not yours (see Open questions).

## Owned files (only these may change)

- `src/app/config.h`, `src/app/config.cpp`
- `src/core/strutil.h`, `src/core/strutil.cpp`
- `src/app/rules_file.cpp`
- `src/core/rules.h`, `src/core/rules.cpp` (the field table; the plan row says "the fingerprint's field list in `src/core/rules.*`")
- `tests/test_config.cpp`, `tests/test_rules.cpp`, `tests/test_strutil.cpp`

Not yours: `settings_bar.cpp`, `paths_tab.cpp`, the preview files, `path_view.cpp`, `library_dialogs.cpp`, `tools/replay.cpp`, `src/core/model.h`.

## Test cases to add

In `tests/test_config.cpp`, next to "malformed INI lines are tolerated":
1. `settings: a value outside its range loads at the nearest edge (D51 Q14)`. Pins, through a temp INI: `depth_value=-1` reads 0; `mslimit_value=900` reads 500 and `mslimit_value=-900` reads -500 (write the pins as `kSqueezeWindowMs`, cast as the field is); `backendlimit_value=-30` reads 0 and `backend_limit()` is 0.0 with the limit on; `preview_volume=150` reads 100 and `-5` reads 0; `depth_mode=2` reads 1 and `depth_mode=-1` reads 0.
2. `settings: a # after a value is a comment (D51 Q15)`. Pins: `view_difficulty=Hard # practice` reads Hard and the key starts "Hard"; `depth_value=7 # seven` reads 7.
3. `settings: on/off keys take 0 or 1 only`. Pins: `view_prodrums=true` leaves the default true untouched; `view_prodrums=0` reads false; `legacy_fills=yes` stays false.
4. `chartmode_key builds its difficulty word from difficulty()`. Pins: `view_difficulty` set in memory to "easy" gives a key starting "Easy "; "Legendary" gives "Expert ".
5. `settings: load and save name the same keys`. Pins: a saved default `Settings` reloads field for field equal (the existing round-trip case already does this; extend it or add the comparison, but add no second round-trip helper).

Re-state "malformed INI lines are tolerated" with its expectations as literals and a comment per line: `depth_value=not_a_number` still reads 0 (junk reads as 0, and 0 is the depth floor), `mslimit_value =  25` still reads 25, `view_prodrums=0` still false. Keep "sp_cap round-trips as a number; auto, zero, junk and pre-1.6 keys read as 4", "to_analysis_settings maps depth_mode onto the search's enum" (its in-memory 7 still reads Scores) and "chartmode_key names the view flags" passing unchanged.

In `tests/test_rules.cpp`: keep "rules: a # starts a comment anywhere on a line", "rules: a bad value or an unknown key is an error that names the key", "rules: every key in the file is read", "rules: the retired Auto keys are read and ignored" and the two fingerprint cases passing unchanged. Add `rules: the fingerprint text is byte for byte what 1.8.4 wrote`, pinning the default rules' `fingerprint()` and `retired_auto_fingerprint()` as the literal 64-bit values today's build prints (print them once before you change `rules.cpp`, and keep that run's output for the report).

In `tests/test_strutil.cpp`: `strutil: the INI line splitter trims, cuts at #, splits at the first =` (pins: `  a = b = c # x ` gives key "a" and value "b = c"; `# only` and `no equals` give nothing; `k=` gives an empty value) and `strutil: parse_bool takes 0 and 1 only` (pins: "1" true, "0" false, "true", "yes", "" and " 1" absent).

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe -sf=*test_config*`
- `build-cpp\Release\hydra_tests.exe -sf=*test_rules*`
- `build-cpp\Release\hydra_tests.exe -sf=*test_strutil*`

Nothing else. Never the full suite.

## Not in this task

- The boxes in `src/ui/settings_bar.cpp` and `src/ui/paths_tab.cpp` keep their own clamps for now; SE2 (wave 2) points them at yours. The Preview's volume slider, cap floors and `/ 100` are PV's (wave 2). `within_label` in `path_view.cpp` is E3's; `batch_settings_summary` is LB's.
- `hit_window_ms` stays an `int`. D51 Q15's decimal hit window (140) is LB's in wave 3, because its callers live in phase-3-owned files. Its table entry (above 0, default `kDefaultHitWindowMs`) is ready for that change.
- `hydra_replay`'s `settings_from`, `flag_bool` and cap message are T1's; it forks from your commit.
- The Paths tab line "A 1-bar cap can never activate Star Power." (Q16) is E3's.
- The uitest copy of 500 in `tests/ui/uitest_paths.cpp` is SE2's.

## Done when

- `load_file` and `save_file` share one key table; no key name appears in both by hand. Every number in `load_file` goes through the one clamp; no `== "1"` remains in `config.cpp`; no `std::abs` in `backend_limit`; `chartmode_key` reads `difficulty()`.
- `load_file` and `load_rules_file` both call the strutil splitter; the rules reader and `fixed_cap_text` walk one field table.
- Every case above is green, every existing case in the three filters passes with no pin edited, and the two fingerprint literals match today's.
- `git diff --stat 81a2519..HEAD` lists only the owned files. No score, path or stored record changes; the results stamp stays "2.1.0".

## Open questions

- D51 Q14's nearest-edge rule and the pinned `sp_cap` reading (0, junk and `auto` read as 4, not 1) are two rules in one table. The brief keeps both, since the cap rule traces to the user (ui-redesign decision 6). Confirm, or say the cap clamps to 1 like the rest.
- `depth_mode=2` in the file now loads as 1 (points) by nearest edge, where today it searches scores and files under 2. That follows Q14 literally; say if "anything not 1 is scores" should win on load instead.
- A scan row for the `== "1"` and typed-500 copies would live in `tests/test_single_owner.cpp`, which no phase 7 task owns. Recommended: the main session adds it at M7-1, or SE2 adds it with the box callers.
- `kMinSpCap` (finding 139's proposed home, `src/core/model.h`) is not in the plan row; the floor stays in the key table unless you say to add the file.

## Commits

One commit, trailers `Task: SE1` plus the preamble's others. Report as the preamble says, and name the clamp, the enum reader, the splitter and `parse_bool` by their exact names, because T1's brief refers to them as "SE1's".
