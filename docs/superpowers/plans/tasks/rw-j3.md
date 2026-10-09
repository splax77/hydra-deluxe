Read docs/superpowers/plans/tasks/_rw-preamble.md first; it holds the rules.

# Task J3: the wave-3 join cleanup

Task id: RW-J3. Base: the main tip your prompt gives, after T7, T6 and T6c have all merged. Branch `claude/rw-j3-cleanup`, worktree `.claude\worktrees\rw-j3-cleanup`; make it as the preamble says.

T7 deleted the HTML pages, and T6 wired the windows. Three pieces of the old plumbing outlived both, because their last readers sat in the other task's files. Now that both are on main, delete them. You delete; you change no behaviour and no rule.

## The jobs

**1. The result structs' `html` field.** `GeneratedReport::html` and `GeneratedDmReport::html` (in `src/app/report.h` and `src/app/dm_report.h`). Their readers on T7's tip were `src/ui/dm_jobs.cpp` (about line 56), `src/ui/library_jobs.cpp` (about 617) and `tests/test_app_state.cpp` (about 1037 and 1216). Grep for every reader on your base, remove each, then remove the fields.

**2. The old report-file paths.** `report_html_path`, `reports_dir`, `set_documents_dir_lookup` and `kPathReportFileName` (and any dm twin of them) in `src/app/report_files.{h,cpp}` and `src/app/report.h`. The last reader on T7's tip was `tests/test_app_state.cpp` (about 1552-1553). `open_in_browser`, `copy_to_short_temp` and `write_report_file` stay for `hydra_fillcompare`.

**3. `src/ui/report_outcome.h`.** Its last reader on T7's tip was `tests/test_library_jobs.cpp` (about line 32, and the case "a report the browser refuses is saved, not failed", which tests a flow that no longer exists). Remove that case and the include, then `git rm` the header.

**4. A scan row from T6's review (finding 4).** Add it to `tests/test_single_owner.cpp`. Question: which slot fields feed a report window's input. Owner: `input_from_slot` in `src/ui/report_window.h`. Pattern: `= slot\.batch_finished` under `src/ui`, allowed only in `report_window.h`. Must match: `in.batch_finished = slot.batch_finished;`. Must not match: `slot.batch_finished = batch_finished;` (in `app_state.cpp`, which writes the slot). Follow the file's existing row format, and prove the row with a red run (put the line back in a window file, see it fail, take it out).

**5. Prove nothing is left.** `git grep` for `\.html\b` on the two result types, `report_html_path`, `reports_dir`, `set_documents_dir_lookup`, `kPathReportFileName`, `report_outcome`, `publish_report`, `show_in_folder`, `open_report_in_browser`, `open_dm_report_in_browser`, `report_file_exists` and `hydra_report`, outside `docs` and `.superpowers`. List each remaining hit and why it's right.

## Owned files

`src/app/report.h`, `src/app/dm_report.h`, `src/app/report_files.{h,cpp}`, `src/ui/dm_jobs.cpp`, `src/ui/library_jobs.cpp`, `src/ui/report_outcome.h` (delete), `tests/test_app_state.cpp`, `tests/test_library_jobs.cpp`, `tests/test_single_owner.cpp`, and the `.cpp` twins of the two report headers only if a field's removal needs a line there.

## Tests

`-sf=*test_app_state*`, `-sf=*test_library_jobs*`, `-sf=*test_single_owner*`, `-sf=*test_report.cpp*`, `-sf=*test_dm_report*`. No uitests unless a harness file changes, which it shouldn't.

## Return

`complete`, `branch`, `worktree`, `tip`, `report`, `questions`, `handoff`.
