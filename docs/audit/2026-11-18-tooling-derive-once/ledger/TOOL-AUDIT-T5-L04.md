# TOOL-AUDIT-T5-L04 (ledger compiler L04: finders F10, F11, F12)

Compiled from reports F10, V10, F11, V11, F11b, V11b, F12, V12 only. Nothing re-searched, nothing run. Short forms: H = C:\Users\Patrick\Downloads\Hydra\hydra-test, G = C:\Users\Patrick\.claude\gates, M = the memory folder. Where a verifier and its finder disagree, the verifier wins and the row says so. "Finder only" marks a claim the verifier did not reach. Where the verifier says a ruling was not re-read, the row says so.

---

## Finder F10 rows

### L-F10-01: How does an agent copy the live hydra.db (plain files with -wal/-shm, or SQLite online backup)?
- **Source:** I7-036, I7-037, I7-056(neighbour), I7-158, I7-058; F10, V10 split 1.
- **Kind:** K2, K6.
- **Verdict:** CONFIRMED. V10 adds: ui-testing and the helper also say a plain copy is right when no -wal exists.
- **Homes:**
  - H/docs/agents/this-machine.md:81 "So copy `hydra.db-wal` and `hydra.db-shm` along with `hydra.db` whenever they are there. The bare `hydra.db` alone can miss the newest writes."
  - H/docs/agents/ui-testing.md:195 "A plain copy of a database another program is writing can catch it halfway through a write, so the helper copies `hydra.db` with SQLite's online backup instead... whenever a `hydra.db-wal` sits beside it."
  - H/tools/hydra_ui_mcp.py:107-134 `copy_database` (backup when -wal exists, plain copy when none; docstring "`dest` needs no -wal or -shm of its own").
  - H/tools/compare_db.py:82-87 "the copy is taken through SQLite's backup API ... leaves a single-file copy with no `-wal`" (V10: line 85 uses the backup API).
- **Splits:** this-machine.md:81 answers "copy the three files as plain files"; ui-testing.md:195, hydra_ui_mcp.py and compare_db.py answer "use SQLite online backup". They differ when the installed Hydra is open and writing; with the app closed all agree on a plain copy. V10 framing: this-machine.md:81 is itself the plain-copy method the helper says can catch a half-written database.
- **Ruling quotes:** None found (finder read review-and-hooks.md and agents-and-workflow.md Confirmed lines; V10 re-read none).
- **Tests:** H/tools/test_hydra_ui_mcp.py tests the helper's own copy; nothing compares it with this-machine.md.
- **Open:** none beyond the situation above.
- **Hook gap:** no.

### L-F10-02: Which PowerShell version is installed (page says 7.6.9)?
- **Source:** I7-152; F10, V10 split 2.
- **Kind:** K2.
- **Verdict:** CONFIRMED.
- **Homes:** H/docs/agents/this-machine.md:51 "PowerShell 7.6.9 at `C:\Program Files\PowerShell\7\pwsh.exe`, RemoteSigned". Live value: `pwsh` reports 7.6.10 (finder and V10), same path and policy.
- **Splits:** the page says 7.6.9; the machine reports 7.6.10. They differ since the machine upgraded after the page's check date. The page says "the machine wins" (finder).
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** V10: only a run could say when the upgrade happened; not run.
- **Hook gap:** no.

### L-F10-03: Which repo owner name does the rename memory note carry?
- **Source:** I8-084, I8-085; F10, V10 split 3.
- **Kind:** K2.
- **Verdict:** CONFIRMED.
- **Homes:** M/hydra-deluxe-rename.md:3 (description) "repo now patseguin1-commits/hydra-deluxe"; same note line 11 moved to "`splax77/hydra-deluxe` (current home...)"; `git remote -v` origin is github.com/splax77/hydra-deluxe.git; H/CLAUDE.md:13 and this-machine.md:15 agree with line 11.
- **Splits:** note description (line 3) says patseguin1-commits; its body (line 11), the git remote, CLAUDE.md and this-machine.md say splax77. The description is stale (V10).
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** no.

### L-F10-04: Where is the game's SqIn/SqOut squeeze rule written down (note says page 05, SP-37)?
- **Source:** I8-164; F10, V10 split 4.
- **Kind:** K8.
- **Verdict:** CONFIRMED.
- **Homes:** M/sp-squeeze-mechanics.md:4,11 "the game's SqIn/SqOut rule is on reference page 05 (SP-37)" and "The game's side of a squeeze is on docs/clone-hero/05-star-power.md (SP-37)". H/docs/clone-hero/05-star-power.md:403-412 (SP-37) "Label: Hydra assumes it, never checked"; "checked only against Hydra's code and full-combo videos, never against the game's code"; its Game code line lists only RVAs of SP-36 and SP-29 (V10). The 500 ms value is `kSqueezeWindowMs = 500.0` (H/src/core/model.h:121, finder only).
- **Splits:** the note says the game's rule is at SP-37; SP-37 says it is Hydra's assumption, not read from the game.
- **Ruling quotes:** H/docs/agents/rulings/clone-hero.md:12 "ok well i am ruling it now: hydra searching for squeezes up to 500 ms is an intentional deviation from the game, not a bug." (finder quote, V10 did not re-read). It covers the 500 ms deviation, not where the game's rule lives; names neither side of this row.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** no.

### L-F10-05: Which data does a render check use (corpus vs the user's library)?
- **Source:** I7-004, I7-014, I8-144; F10, V10 split 5.
- **Kind:** K1, K3.
- **Verdict:** CONFIRMED-CHANGED (V10, "weak"). The note's own description holds both halves, so it is a wording tension inside one sentence, not a split between homes. Different questions: what to look at vs what to analyze.
- **Homes:**
  - M/render-checks-use-the-corpus.md:3 "not done until it is seen in the real app on the user's library and ini ... render checks run on testdata/input, never a whole-library analysis" (V10 quote); line 21 "Never ask for a full library run when `testdata/input` answers the question. The user's real database copy is for the Library view only, never for starting an analysis."
  - H/docs/agents/ui-testing.md:11 the same sentence, with the user's 2026-10-14 words "you dont need a full library run to check the rendering" (finder).
  - H/docs/agents/this-machine.md:91 and H/tools/compare_db.py:82-88 ask for the real database copy for timings (finder only; a different case).
- **Splits:** None between homes (V10). Finder listed note line 3 vs ui-testing.md:11; V10 says they answer different questions.
- **Ruling quotes:** the 2026-10-14 words are quoted inside ui-testing.md:11 (finder); not a `## Confirmed` line checked by anyone.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** no.

### L-F10-06: Which flag name and default path does a tool use to find hydra_replay.exe?
- **Source:** I8-193; F10 split 6, V10; also K6 across tools.
- **Kind:** K2, K6.
- **Verdict:** CONFIRMED-CHANGED. V10: the skill line 18 speaks of squeeze_scout, whose own text says "--replay at it" (squeeze_scout.py:153), while fcvideo.py:2906 uses `--hydra-replay`. So two tools use two flag names, not a skill disagreeing with its tool. V10 did not open squeeze_scout's argparse.
- **Homes:** skill analyzing-sp-squeezes SKILL.md:18 (`--replay`); video-tools\README.md:78 and fcvideo.py:2906-2907 `--hydra-replay`; default path typed in video-tools\replay.py:30 and squeeze-tools\squeeze_scout.py:38 (V10 confirmed replay.py:30 only; squeeze_scout path line not re-read).
- **Splits:** `--replay` (squeeze_scout) vs `--hydra-replay` (fcvideo, README) for the same job. Two tools each type the default build path.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** squeeze_scout's argparse and its path line not read by V10.
- **Hook gap:** no.

### L-F10-07: Does the squeeze skill's memory pointer reach a live note? (merged with F11b I8-255)
- **Source:** I8-202, I8-255; F10, V10, F11b, V11b.
- **Kind:** K8.
- **Verdict:** CONFIRMED. V11b: CONFIRMED-CHANGED on counts only (M top-level holds 66 `[[..]]` occurrences, 39 distinct, none missing; 14 distinct "(archived)" targets, the finder said 19, likely occurrences).
- **Homes:** skills/analyzing-sp-squeezes/SKILL.md:34 "see memory `sp-squeeze-mechanics`, `gameplay-video-score-analysis`"; sp-squeeze-mechanics.md is live; gameplay-video-score-analysis.md exists only in M\archive\ and MEMORY.md does not index it. MEMORY.md:48 "Finished and superseded memories live in archive/ (not indexed); open one only when a task points there."
- **Splits:** None. The skill calls both "memory"; one is archived.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** no.

### L-F10-08: Is "the install folder has users-modify permissions" a user decision?
- **Source:** I8-187; F10, V10.
- **Kind:** K8.
- **Verdict:** CONFIRMED as unsupported (V10: grep of docs/ for `users-modify` found no file; V10 searched docs only, not session cards outside docs).
- **Homes:** M/windows-installer.md:15 "The install folder has `[Dirs] Permissions: users-modify`. That is the user's design"; installer/hydra.iss:73 `Permissions: users-modify`.
- **Splits:** None. A claim of a user decision with no ruling, decision-log line or card found.
- **Ruling quotes:** None found (finder searched docs/agents/rulings, fix-decisions, docs).
- **Tests:** none named.
- **Open:** session cards outside docs not searched by V10.
- **Hook gap:** no.

### L-F10-09: Does this-machine.md hold every machine tool path (dumpbin, ffmpeg)?
- **Source:** I8-003, I8-103, I8-183; F10, V10.
- **Kind:** K7, K8.
- **Verdict:** CONFIRMED.
- **Homes:** M/MEMORY.md:49 and H/CLAUDE.md "This machine's tools and paths are in `docs/agents/this-machine.md`". this-machine.md has no dumpbin or ffmpeg line (grep, finder and V10). The paths live only in M/mimalloc-link-order.md, M/video-decode-ceilings.md and M/stitched-chart-auto-fills.md (V10).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** no.

### L-F10-10: How is an executor worktree created, and is the "leave isolation out" rule held by a hook?
- **Source:** I8-074; F10, V10.
- **Kind:** K5, K6, K7.
- **Verdict:** CONFIRMED.
- **Homes:** M/git-traps-in-shared-checkout.md:16 `git worktree add .claude\worktrees\<name> -b claude/<name> main`; H/tools/new_worktree.ps1:1-8,36 does the same job (`git -C $main worktree add $wt -b $branch $Base`, default base main); the note does not name the script. G/review_dispatch.py:149-150 denies reviewer dispatches that set `isolation`; no other gate file mentions `isolation` (V10), so the executor half is prose only. derive-once-review.md:200 and the note line 17 state the reviewer half.
- **Splits:** None. A typed command in a note and a script do the same job; the reviewer half is enforced, the executor half is not.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** no.

### L-F10-11: Which worktree folders may a subagent delete inside, and where do task worktrees go? (merged: F10 side note, F11 I9-061/I10-073, F11b I10-073, F12 SN-1)
- **Source:** I8-074 (side note), I9-061, I10-073, I1-046; F10, F11, F11b, F12, V10, V11 S1, V11b, V12 SN-1.
- **Kind:** K1, K2, K3, K6, K8.
- **Verdict:** CONFIRMED (V11 S1 CONFIRMED; V11b I10-073 CONFIRMED; V12 SN-1 CONFIRMED; V10 CONFIRMED). V11 notes whether the hook then refuses a delete from `H\worktrees\` is a run to settle; not run.
- **Homes:**
  - G/delete_rule.py:14-18 `WORKTREE_FOLDERS` = `H\.claude\worktrees` and `C:\Users\Patrick\.claude\worktrees`; judges literal absolute paths strictly inside a worktree directly in one of them.
  - G/cleanup_sweep.ps1:174-178 `Get-SweepPrefixes`: "<root>\.claude\worktrees in any repo, and <root>\worktrees in the repo this hook lives in" (a different set).
  - H/tools/new_worktree.ps1:25,29 "The main checkout owns .claude\worktrees"; makes `<main>\.claude\worktrees\<TaskId>`, branch `claude/<TaskId>`.
  - H/docs/agents/brief-preamble.md:43 and derive-once-review.md:197 write both delete_rule folders out.
  - G/README.md:136 "under `C:\Users\Patrick\.claude\worktrees\<task>` ... Keep it under a `worktrees` folder: the delete rule trusts only those"; model-postures.md:5 same words. V11: the file is at C:\Users\Patrick\.claude\model-postures.md, not in gates; F11's citation path "G/model-postures.md" was wrong, the text is right.
  - H/tools/effort_test_guard.ps1:30, tools/ch_mcp/tests/test_quiet.py:22, effort_test_taint.py:40-43 (finder only).
- **Splits:**
  - delete_rule.py names two exact folders; cleanup_sweep names `.claude\worktrees` of any repo plus `<root>\worktrees` for the hooks repo; new_worktree.ps1 makes only the Hydra `.claude\worktrees`. They differ for a worktree at `H\worktrees\<task>`.
  - This audit's own worktree is `H\worktrees\tooling-audit-records`, under none of the folders delete_rule.py names for Hydra (V11 CONFIRMED, V12 CONFIRMED by listing; `ls hydra-test/worktrees` shows many such folders).
  - README.md:136 and model-postures.md:5 say "a `worktrees` folder"; only two exact folders pass. README:136 names only the hooks-repo folder; the preamble names both.
  - The hooks-repo folder is typed twice (cleanup_sweep, delete_rule) with no test tying them (F11b S1c, V11b confirmed).
- **Ruling quotes:** H/docs/agents/rulings/review-and-hooks.md:17 (F11b, F12; V11b did not reopen) "**2026-10-25** chose "Keep rule, move worktrees (Recommended)" to "The delete rule lets an agent recursively delete only inside its scratch folder or a worktree that sits in a `worktrees\` folder. Ten Hydra worktrees sit beside the repo as `C:\Users\Patrick\Downloads\Hydra\wt-*` instead. ..."". It does not name `H\worktrees\` or `claude/<TaskId>`, so it does not name both sides.
- **Tests:** G/tests/test_delete_rule.py:24-31,243-250 pins delete_rule.py's folders against its own typed copies; no test ties cleanup_sweep, new_worktree or the docs to them.
- **Open:** whether the hook refuses a subagent's delete under `H\worktrees\` needs a run.
- **Hook gap:** yes, G/delete_rule.py.

### L-F10-12: Where is the installer file name built (HydraDeluxe-<ver>-setup.exe)?
- **Source:** I8-086, I8-185; F10, V10.
- **Kind:** K1, K6.
- **Verdict:** CONFIRMED.
- **Homes:** H/installer/hydra.iss:65 `OutputBaseFilename=HydraDeluxe-{#HYDRA_VERSION}-setup`; H/installer/build_installer.ps1:7,128 `"HydraDeluxe-$version-setup.exe"`; M/hydra-deluxe-rename.md:18 and M/windows-installer.md:11 type the name.
- **Splits:** None between values. V10 situation: the script looks the file up by its own copy of the name; an edit of one breaks the other.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** no.

### L-F10-13: Which code knows the library folder path (hydra_ui_mcp "the only place")? (merged with F11 I10-055)
- **Source:** I7-036, I10-055; F10, F11, V10, V11.
- **Kind:** K1, K6, K8.
- **Verdict:** CONFIRMED-CHANGED (V10). V10: compare_db.py:51,87 are docstring command examples for agents, not code paths. V11: H/tools/bench.cpp:975-986 does read the Program Files variables and pushes `C:\\Program Files\\Hydra`, so another code place knows it. Neither verifier searched src/.
- **Homes:**
  - H/tools/hydra_ui_mcp.py:17 "snapshot_library, the only code that knows that folder's path"; :55-56 `DEFAULT_LIBRARY_DIR = r"C:\Program Files\Hydra"`; :140 "This is the only place that knows the library folder's path".
  - H/tools/bench.cpp:975-986 (V11 confirmed).
  - H/tools/compare_db.py:51,87 (docstring examples per V10); H/docs/agents/this-machine.md:77; ui-testing.md:350; installer (finder).
- **Splits:** the "only place" claim vs bench.cpp reading and typing the same folder.
- **Ruling quotes:** None found.
- **Tests:** H/tools/test_hydra_ui_mcp.py tests `snapshot_library` only.
- **Open:** src/ not searched for install-folder knowledge.
- **Hook gap:** no.

### L-F10-14: How many charts does the test corpus hold (the typed "98")?
- **Source:** I7-014; F10, V10.
- **Kind:** K1.
- **Verdict:** CONFIRMED present; line counts CONFIRMED-CHANGED slightly (V10 counted one "98 charts" hit in ui-testing.md, the finder two lines).
- **Homes:** H/docs/agents/test-rubric.md:11 "A 98th chart was added to testdata/input, and tests still expected 97."; ui-testing.md:46,48; test-rubric-examples.md:101,113,116; H/tests/ui/uitest_batch_reports.cpp:389,481,494,507,514 (five lines holding 98, V10 confirmed). test-rubric.md:23 calls it "a snapshot number" (finder).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** the uitest literals hold the count to the corpus in code but not to the docs (V10).
- **Open:** the corpus chart count was not recounted by anyone.
- **Hook gap:** no.

### L-F10-15: Typed copies of machine values (Python path, gh path/account, gates folder, repo name, build folder etc.)
- **Source:** I7-141, I7-142, I7-145, I7-146, I7-148, I7-149, I7-150, I7-153; F10 group F.
- **Kind:** K1, K6.
- **Verdict:** NOT VERIFIED. V10 "Not reached": the 20 typed Python-path copies, gh scope check, Visual Studio build numbers.
- **Homes (finder only):** this-machine.md:7-9 Python path and versions (finder's live check agreed); about 20 typed copies of the interpreter path in G/publish.ps1:16, G/install_ref_gate.ps1:10, G/README.md:110,114,145, G/settings.fragment.json (6) and live settings.json (9), G/tests/test_shell_output.py:74, derive-once-review.md:124; gh path and account in this-machine.md:13 and settings; CLAUDE.md, progress.py:22 and many briefs type the gates path; tools/ch_mcp/game_files.py:38 `LOCALLOW_DIR` vs this-machine.md:53 typed (case differs).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** finder: G/tests/test_docs_numbers.py and test_sync_settings.py hold the live settings.json to the fragment (so the 6 and 9 copies); no test holds the path to this-machine.md, publish.ps1 or install_ref_gate.ps1.
- **Open:** gh scopes were not printed; Visual Studio build numbers not read.
- **Hook gap:** no.

### L-F10-16: What does an exploring agent read first (domain page read order)?
- **Source:** I6-238, I6-240, I7-216; F10 group A.
- **Kind:** K1, K3 (mild).
- **Verdict:** NOT VERIFIED (V10 "Not reached").
- **Homes (finder only):** H/docs/agents/domain.md:7-9 "`CONTEXT.md` at the repo root, or", "CONTEXT-MAP.md at the repo root if it exists", "`docs/adr/`"; H/CLAUDE.md:21 "Single-context: `CONTEXT.md` and `docs/adr/` at the repo root. See `docs/agents/domain.md`."; the repo has CONTEXT.md and docs/adr/ and no CONTEXT-MAP.md (finder ls).
- **Splits:** None between files. Finder's mild point: domain.md's "or" vs its own "if it exists" test.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** whole row unreached by the verifier.
- **Hook gap:** no.

### L-F10-17: Where do doc pictures go and what is the one list of them?
- **Source:** I6-244, I6-246, I7-221, I7-222, I10-079(F11); F10 group B, F11.
- **Kind:** K1, K8 (candidate).
- **Verdict:** NOT VERIFIED (V10 "Not reached"; V11 did not reach I10-079).
- **Homes (finder only):** H/docs/agents/doc-screenshots.md:3,11,29; H/tools/doc_shots/shots.txt:1-4 ("This file is the one list of those pictures"); H/tools/doc_shots.ps1:1,3-5.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none found comparing docs/images with shots.txt.
- **Open:** whole row unreached.
- **Hook gap:** no.

### L-F10-18: Library-folder resolution order and the snapshot file lists (merged with F11 I10-058, I10-078)
- **Source:** I7-036, I7-058, I10-058, I10-078; F10 group D/E, F11.
- **Kind:** K1, K3.
- **Verdict:** NOT VERIFIED (neither verifier reached it).
- **Homes (finder only):** order "library_dir, then HYDRA_LIBRARY_DIR, then DEFAULT_LIBRARY_DIR" stated in hydra_ui_mcp.py:140-141, its `hydra_start` text :766-768 and ui-testing.md:193; `SNAPSHOT_FILES` hydra_ui_mcp.py:71-72 vs the same three names in ui-testing.md:193,346-350, capture_window.ps1:8-10, this-machine.md:89, src/app/config.cpp:98,103. F11: ui-testing.md says case 3 writes a settings.ini and copies only the ui ini while capture_window.ps1 copies all three files.
- **Splits:** F11 minor K3 on the ui-testing case 3 wording vs the script (finder only).
- **Ruling quotes:** None found.
- **Tests:** none holds the docs list equal.
- **Open:** unreached.
- **Hook gap:** no.

### L-F10-19: Two ways to stand up the real app (manual steps vs hydra_start("window")) and where hydra.db/ini live
- **Source:** I7-058, I7-061, I7-157; F10 group E.
- **Kind:** K1, K6 (candidate).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** ui-testing.md:346-352 (manual steps) vs hydra_ui_mcp.py `WINDOW_EXE`/`BUILD_TARGETS`:60-64 and its snapshot copy; this-machine.md:77 and ui-testing.md:346 both name `exe_dir`/`db_path()` in src/app/config.cpp:31,96-103 and state the file names.
- **Splits:** manual step copies files; helper copies with online backup (same as L-F10-01).
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** unreached.
- **Hook gap:** no.

### L-F10-20: Where is a handoff written and what shape does it take? (merged with F11 I8-237, I8-238)
- **Source:** I7-191, I8-237, I8-238; F10 group G, F11.
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** H/docs/agents/handoff.md:7 `docs/handoffs/<YYYY-MM-DD>-<slug>-handoff.md`; H/.claude/commands/handoff.md:11-12 (same path twice; also restates the Claims table, hash and scratch rules; the slug rule of 2 to 6 lowercase words only at :8); H/tools/check_handoff.py:29,398-403 reads a date from the file name; settings.json:14 permission rule on docs/handoffs/**. docs/handoffs holds one folder (`2026-10-25-test-audit`) not matching the name (F10).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** H/tools/test_check_handoff.py tests the checker, not the command text or the name shape.
- **Open:** unreached.
- **Hook gap:** no.

### L-F10-21: Do the video-decode speed figures in the note appear in the video-tools README it points at?
- **Source:** I8-184; F10, V10.
- **Kind:** K8, K1.
- **Verdict:** NOT VERIFIED beyond a grep. V10: "PARTLY": grep of video-tools/README.md for 3,000/1,900/3000/1900 found nothing (supports the finder); neither read the README table cells.
- **Homes:** M/video-decode-ceilings.md:11 "h264 ~3,000 fps CPU, AV1 (dav1d) ~1,900 fps; NVDEC ~2,300-2,700 fps"; M/MEMORY.md:36; video-tools README.md:503 (Decoder section), :522 (table), :582 "about 894 fps against 2,557 on the processor" (finder).
- **Splits:** note figures vs README figures (finder).
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** README table cells unread.
- **Hook gap:** no.

### L-F10-22: REA skill asks the user for tool paths/versions that this-machine.md does not hold
- **Source:** I8-227, I8-228, I8-229; F10, V10.
- **Kind:** K4, K1.
- **Verdict:** NOT VERIFIED. V10: "not contradicted"; this-machine.md grep for REA_/JAR/sigrok/WABT returned nothing; V10 did not re-open the skill.
- **Homes (finder only):** skills/reverse-engineer-anything SKILL.md and references/native-and-artifacts.md:30 name `REA_JADX_MCP_JAR`, `REA_TSHARK_COMMAND`, `REA_SIGROK_COMMAND`, `REA_WABT_BIN_DIRECTORY`, "Java 17 or newer", "sigrok-cli 0.7.2", "WABT 1.0.42", "Node.js 24.x >=24.15 or 26+" (Node line repeated twice); this-machine.md:26 gives REA start text and GHIDRA_INSTALL_DIR only.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** unreached.
- **Hook gap:** no.

### L-F10-23: Smaller copied values in memory notes, docs and tools (one row, each unreached)
- **Source:** I7-056, I8-005, I8-018, I8-024, I8-047, I8-058, I8-069, I8-127, I8-133, I8-146, I8-162, I8-167, I8-179, I8-201; F10.
- **Kind:** K1, K3 (mild).
- **Verdict:** NOT VERIFIED (V10 "Not reached" lists the dmleaderboards URL, handoff-name check, the one-reviewer note cap number, hydra-deluxe-rename AppUserModelID).
- **Homes (finder only):**
  - Test-registration function names in ui-testing.md:294-296 vs tests/ui/uitest_paths.cpp:1054, uitest_report_windows.cpp:1618,1634, uitest_tests.cpp:12 (names match today).
  - M/dmleaderboards-comparison.md:13 URL vs H/src/net/dmbot_client.h:30 `kDefaultApiBase` (same string).
  - M/one-reviewer-rule-d107.md:15 "fix-round.md retired" (file absent, nothing names it) and the cap "150 calls" vs G/progress.py:14 `CALL_WARN, CALL_STOP = 100, 150`.
  - M/fc-video-workflow.md:11 "eleven-step checklist" (README has 11 items).
  - Two search lists for one rule: this-machine.md:29 vs M note (Get-Command, winget list, Get-AppxPackage).
  - CLAUDE.md:27 vs a memory note on looking in sessions first (note adds a duty to tell the user).
  - H/tests (scan.cpp, test_docs_match_code.cpp, source_tree.h) read `sourcetree::root()`; owner claim holds (finder, not opened).
  - Skill "standalone by design" (SKILL.md:49): a "by design" claim, one home.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** per item above; the cap number is not held by test_docs_numbers.py (it reads no memory).
- **Open:** unreached.
- **Hook gap:** no.

---

## Finder F11 rows

### L-F11-01: Where is the state root, and why do build slots and benches use a `.claude\hooks\state` folder? (merged: I9-017, I9-192, I10-034, S3)
- **Source:** I9-017, I9-018, I9-192, I10-034; F11, V11.
- **Kind:** K1, K2, K6.
- **Verdict:** CONFIRMED-CHANGED (V11). The finder said `C:\Users\Patrick\.claude\hooks` no longer exists. V11 found it exists: `hooks\state\build_slots` was made today, so the default in build_slot.ps1 is live, not dead. G\state has no build_slots folder; two state roots are in use.
- **Homes:**
  - G/common.py:10 `STATE_ROOT = dirname(abspath(__file__))/state` (owner, finder).
  - H/tools/build_slot.ps1:10 and :78 `'C:\Users\Patrick\.claude\hooks\state\build_slots'` (V11 confirmed); H/tools/bench_run.ps1:8-9,20; H/tools/compare_db.py:43 (V11 confirmed).
  - G/status_append.ps1:11 `$StateRoot = 'C:\Users\Patrick\.claude\gates\state'` (typed second owner; comment :8-10 says progress.py owns the layout; override HOOKS_TEST_STATE_ROOT); G/tests/lib/harness.ps1:30; G/cleanup_sweep.ps1:61 (finder only).
  - H/docs/agents/brief-preamble.md:13, derive-once-review.md:9, doc-review.md:9, G/README.md:75,88,134 type the status path (finder only).
  - G/tests/test_docs_numbers.py:25-27 RETIRED `hooks[/\\](state|tests)` forbids those words in five Hydra docs only; it does not reach H/tools scripts (finder only).
- **Splits:** build_slot.ps1:78 default slot folder `...\.claude\hooks\state\build_slots` vs the gates state root `...\.claude\gates\state`. They differ for any build pass (slot locks) and bench run. The cleanup sweep prunes only gates\state subfolders, so nothing clears the hooks one (V11 confirms the sweep does not reach hooks\state).
- **Ruling quotes:** None found.
- **Tests:** H/tools/test_build_pass.ps1:41 sets HYDRA_BUILD_SLOT_DIR to a temp folder, so no test holds the default equal to anything; no test holds status_append.ps1 equal to common.py (test_status_append.ps1 sets the env override). Finder only.
- **Open:** none beyond the above.
- **Hook gap:** no.

### L-F11-02: Which Visual Studio install does the build use (find_cmake.ps1 vs build_cpp.ps1 vs ci.yml)? (I10-072, S5)
- **Source:** I10-072, I7-149/I7-150(F10); F11, V11.
- **Kind:** K3, K6.
- **Verdict:** CONFIRMED-CHANGED (V11). find_cmake.ps1 tries PATH first, then `vswhere -latest ... VC.Tools`, and falls back to `-all -prerelease` itself, so it handles the pending-update case.
- **Homes:**
  - H/tools/find_cmake.ps1:6-17 (PATH, then `-latest -products * -requires ...VC.Tools.x86.x64`, else `-all -prerelease -products *`).
  - H/build_cpp.ps1:60-63 (the file is at the repo root, not tools/): `-all -prerelease ... -format json`, newest by installationVersion; comment says `-latest` skips an install with a pending update.
  - .github/workflows/ci.yml:326 `vswhere -latest -all -products * -property installationPath`.
  - H/docs/agents/this-machine.md:41 "`cmake` is not on PowerShell's PATH ... Today that is the 2026 Build Tools' copy" (finder; a dated fact).
- **Splits:** find_cmake (`-latest`, fallback `-all`), build_cpp (newest of `-all -prerelease`) and ci.yml (third form) differ when an install has a pending update or two installs exist; the cmake and compiler can come from different installs. Docs say cmake is not on PATH while find_cmake tries PATH first (finder, K3).
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** no.

### L-F11-03: What does HYDRA_MUTANT unset or empty mean (C++ vs Python)? (S4)
- **Source:** I10b-105; F11, V11.
- **Kind:** K6.
- **Verdict:** CONFIRMED (meaning of 0 left open).
- **Homes:** H/tools/mutate/hydra_mutant.h:41-53 (unset gives -1; empty, non-number or negative aborts with a message); H/tools/mutate/py_mutate.py:63 `int(os.environ.get('HYDRA_MUTANT') or 0)` (unset and empty both 0; a non-number raises ValueError; a negative is accepted); its own comment py_mutate.py:55-56 says "none when unset" (V11: the code does it with 0, not a none value).
- **Splits:** C++ unset = -1 and empty is refused; Python unset and empty both = 0. They differ on an empty value, a negative, and whether switch number 0 is ever a real switch.
- **Ruling quotes:** None found.
- **Tests:** none named (the format number: tests use run.MUTANTS_FORMAT; nothing ties it to rewrite.FORMAT, finder only).
- **Open:** whether 0 is ever a real switch number: V11 grep'd rewrite.py, run.py and hydra_mutant.h and found no statement; a read of rewrite.py's numbering or a run settles it.
- **Hook gap:** no.

### L-F11-04: Does the PowerShell test harness's named proof test exist? (I9-175, I9-176; also F11b I9-178 header claim)
- **Source:** I9-175, I9-176; F11, V11, F11b, V11b.
- **Kind:** K8.
- **Verdict:** CONFIRMED (V11 grep; V11b missing-file error).
- **Homes:** G/tests/lib/harness.ps1:5 "This file is the one home of how a test runs a hook, reads its verdict, counts checks and prints its summary line"; :11 "Its proof is tests\test_h39_harness.ps1"; :21 "Only test_h39_harness.ps1 changes them". `grep h39` in G hits only those two lines; `ls G/tests` shows no such file.
- **Splits:** None. A named proof file is missing.
- **Ruling quotes:** None found.
- **Tests:** the missing test is the test named.
- **Open:** none.
- **Hook gap:** no.

### L-F11-05: Who owns the name realapp.jsonl (two literals in two repos)?
- **Source:** I9-136; F11, V11.
- **Kind:** K1, K6, K8.
- **Verdict:** CONFIRMED.
- **Homes:** H/tools/hydra_ui_mcp.py:78-81 `REALAPP_FILE = "realapp.jsonl"` ("The hooks repo's ref gate reads it, so its field names are a contract (realapp_record writes them)"); G/review_rules.py:44-49 comment "Hydra's tools/hydra_ui_mcp.py owns its name (REALAPP_FILE) and its fields (realapp_record); this file only reads them" and then `REALAPP_FILE = "realapp.jsonl"` typed; G/README.md:55; G/tests/test_docs_numbers.py:54-55,179 (README must name `realapp.jsonl`); own constants in G/tests/test_review_rules.py:410,528 and H/tools/test_hydra_ui_mcp.py:104,638 (finder).
- **Splits:** None between values (both "realapp.jsonl"). The comment says one file owns the name; both type it.
- **Ruling quotes:** None found.
- **Tests:** none found holding the two equal; field names (kind, commit, dirty, n) read at review_rules.py:385 with no cross-repo test. V11 did not search the tests itself.
- **Open:** tests not searched by V11.
- **Hook gap:** no.

### L-F11-06: Are all game addresses in ch_probe/constants.py ("the ONE place")? (I10b-058, S7)
- **Source:** I10b-058; F11, V11.
- **Kind:** K8.
- **Verdict:** CONFIRMED.
- **Homes:** H/tools/ch_probe/constants.py:3-7 "This is the ONE place the numbers live. Every other module imports from here; nothing else hard-codes an address." Other hard-coded addresses (V11 read): ch_mcp/agent/menu.ts:54 `PLAYER_SCREEN_FRAME_RVA = 0x32F590`; ch_mcp/launch_patch.json:8 `0x3968D0` and :11 `0x396EC5`; ch_mcp/start_candidates.json:19 rva `0x8D090`. None of the four values is in constants.py. V11 note: they sit in ch_mcp, constants.py is in ch_probe, so a reader could say the claim is about ch_probe only; the sentence says "nothing else".
- **Splits:** the claim vs four hard-coded addresses in ch_mcp.
- **Ruling quotes:** None found.
- **Tests:** finder: test_frida_tools.py checks menu.ts's value against the dump's facts (per its comment; not opened).
- **Open:** none.
- **Hook gap:** no.

### L-F11-07: Which places count as a recorded decision (derive-once-review.md vs the precheck)? (I10b-084, S2)
- **Source:** I10b-084; F11, V11.
- **Kind:** K2, K6.
- **Verdict:** CONFIRMED.
- **Homes:** H/docs/agents/derive-once-review.md:45 lists "a line under `## Confirmed` in `docs/agents/rulings/`" (plus ADRs, CONTEXT.md, a plan's "User decisions" header, the decision log); H/tools/precheck/decisions.h:55-63 and decisions.cpp:317-338 read docs/adr/*.md, CONTEXT.md, plan "User decisions" and "Test limits" sections, and decision-log D-records; `grep -c rulings` gives 0 in decisions.cpp and decisions.h (V11). Also tools/session_cards/common.py:44 `DECISIONS_LOG`, writer-brief.md:30,78 (finder).
- **Splits:** the doc lists the rulings folder as a decision home, the precheck does not read it; the precheck reads "Test limits" sections the doc does not list. The doc says Git does not check "those other places", so the reviewer does (finder: a claim about intent, not a check).
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** no.

### L-F11-08: Where does replay scoring live (replay.cpp vs replay.h)? (I10b-100, S6)
- **Source:** I10b-100; F11, V11.
- **Kind:** K8.
- **Verdict:** CONFIRMED.
- **Homes:** H/tools/replay.cpp:28-29 "The scoring itself lives in core/replay.h so this tool and tests/test_replay share one implementation."; H/src/core/replay.h:1-12 "It owns no scoring rule" and points at chord_score_table (core/scoring.h), core::paid_by_sp, core::backend_row_value (core/backend_value.h), core::after_sp_end, core::sqout_position (finder). tools/replay_schedule.h:2-10 names core/note_window.h for the hit window (finder).
- **Splits:** replay.cpp says replay.h holds the scoring; replay.h says it holds none.
- **Ruling quotes:** None found.
- **Tests:** finder: tests/test_replay.cpp, test_replay_schedule.cpp, test_replay_windows.cpp include core/replay.h (the "share one implementation" half); V11 did not recheck.
- **Open:** src/core internals not read.
- **Hook gap:** no.

### L-F11-09: Is harness.ps1 the one way a test copies hooks? (F11b I9-178)
- **Source:** I9-178; F11b, V11b.
- **Kind:** K6, K8.
- **Verdict:** NOT VERIFIED. V11b: "UNCHECKED beyond one line" (read only the harness.ps1:5 header; did not reopen the Python tests).
- **Homes (finder only):** G/tests/lib/harness.ps1:217-238 `New-HooksCopy` (named hooks plus all of lib\ into `hooks-copy-<guid>` with empty state\; refuses a temp folder inside the source); users test_h52_cleanup_sweep.ps1:17,473 and test_h53_session_cards_start.ps1:19. Python tests each copy their own files: test_delete_rule.py:380, test_publish.py:205,257-258,313,408, test_ref_judge.py:62,433,702,710, test_review_dispatch.py:216.
- **Splits:** harness.ps1:5 "one home of how a test runs a hook" vs the Python tests that copy separately, each with its own file list and its own choice of lib\ and state\ (finder only).
- **Ruling quotes:** None found.
- **Tests:** none tie them.
- **Open:** Python copies not read for a refusal like the harness's.
- **Hook gap:** no.

### L-F11-10: The build-cpp\Release folder and exe names typed in about 30 places (F11b I10-056)
- **Source:** I10-056; F11, F11b, V11b.
- **Kind:** K1, K6.
- **Verdict:** NOT VERIFIED (V11b: not reopened).
- **Homes (finder only):** H/CMakePresets.json:10 `binaryDir "${sourceDir}/build-cpp"` and buildPresets "Release" (:97); H/tools/cmake_presets.ps1:1-3 "A preset's build folder and configuration, read from CMakePresets.json (their one owner)"; readers build_cpp.ps1:46-47 and installer/build_installer.ps1:60-61,122. Typed copies: hydra_ui_mcp.py:59-62, doc_shots.ps1:8, capture_window.ps1:7, check_shards.ps1:17, mutate/find.py:18,629, rewrite.py:516, ci.yml:77,92,98,115,166, fuzz.yml:40, warm-build-folder/action.yml:19, docs (agent-rules.md:7, brief-preamble.md:47, derive-once-review.md:24,99, integrate.md:37,43, this-machine.md:63,77, ui-testing.md), G/session_cards_start.ps1:285, model-postures.md:11, SKILL.md:18, M/merge-tests-touch-changed-files.md:15.
- **Splits:** None; all agree today.
- **Ruling quotes:** None found.
- **Tests:** test_hydra_ui_mcp.py:4 cites the path; nothing compares it with CMakePresets.json.
- **Open:** unreached.
- **Hook gap:** no.

### L-F11-11: Scratch folders (hydra_live_ under Temp\claude vs ch_live_ under Temp; "a week" vs 7) (I10-057)
- **Source:** I10-057; F11, F11b, V11b.
- **Kind:** K1, K6.
- **Verdict:** CONFIRMED (V11b, partly reread: cleanup_sweep.ps1:54 `$ScratchAgeDays = 7`; hydra_ui_mcp.py:67,306,600,770; game_tools.py:47,599,1402). V11b did not reopen Invoke-ScratchSweep's age rule or ui-testing.md:213.
- **Homes:** H/tools/hydra_ui_mcp.py:65-67 `SCRATCH_SUBFOLDER = "claude"`, :306, :344 `hydra_live_{tag}`, :600 "gone a week with no writes; $ScratchAgeDays in cleanup_sweep.ps1 owns that age", :770 "after a week with no writes"; H/docs/agents/ui-testing.md:213 "removes a kept folder after a week"; G/cleanup_sweep.ps1:54 `$ScratchAgeDays = 7`, :60 `Temp\claude`, :321-327,393-440 (finder F11b: a folder goes by age, the sweep names no `hydra_live_`); H/tools/ch_mcp/game_tools.py:47 `SCRATCH_PREFIX = "ch_live_"`, :599 root is `tempfile.gettempdir()` (Temp itself), :1402 `ch_live_<pid>_<n>`; C++ side tests/temp_util.h:33, test_live_protocol.cpp:77,81 (finder).
- **Splits:** hydra_live_ folders sit under Temp\claude (swept by age); ch_live_ folders sit directly under Temp, outside the sweep's root; nothing found that removes them (finder; V11b: whether anything removes ch_live_ was not searched). "A week" in prose (three places) vs 7 in code (agree today).
- **Ruling quotes:** None found.
- **Tests:** H/tools/test_hydra_ui_mcp.py:314-317 holds the helper's root to Temp\claude, not equal to cleanup_sweep.ps1:60.
- **Open:** whether anything removes ch_live_ folders was not searched by V11b.
- **Hook gap:** no.

### L-F11-12: Where do the il2cpp dumps live (DUMPS_ROOT vs typed paths and the two JSON "dump" values)? (I10b-015)
- **Source:** I10b-015; F11, F11b, V11b.
- **Kind:** K1, K6.
- **Verdict:** CONFIRMED (V11b: grep of tools/ch_mcp .py/.ts for `"dump"` found only dump_facts.py:1054, a diff label; kit.ts:19,63-64 only say the JSON lists dump lines; by grep, not a full read).
- **Homes:** H/tools/ch_mcp/build_facts.py:28 `DUMPS_ROOT = r"C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps"` (used by build_facts.py:103,109 and chdis.py:218); kit_methods.json:3 and launch_patch.json:4 "dump" (V11b: prose in data files, read by nothing); tools/ch_mcp/README.md:7,12,20,58; about 14 files in all (docs/clone-hero pages, names-6142.tsv:1, research doc; finder). start_candidates.json:4 points at build_facts.facts_path (pointer).
- **Splits:** None; all say `...\hydra-data\ch-dumps\6142\dump.cs` today.
- **Ruling quotes:** None found.
- **Tests:** test_dump_facts.py:280-284 refuses an output folder inside the git checkout; no test ties the JSON or README to DUMPS_ROOT (finder).
- **Open:** none.
- **Hook gap:** no.

### L-F11-13: Typed copies of roots and names (Hydra root, project folder name, gates folder, memory default, checkout path) (I9-023, I9-072, I10-001, I10-002, I10b-110, I9-001)
- **Source:** I9-001, I9-023, I9-072, I10-001, I10-002, I10b-110; F11.
- **Kind:** K1, K6.
- **Verdict:** NOT VERIFIED (V11 "Not reached" lists all of these).
- **Homes (finder only):** Hydra root typed in G/delete_rule.py:17, G/tests/replay.py:42, settings.fragment.json:101, settings.json:13-15,111, tools/session_cards/common.py:36 `HYDRA_REPO`, writer-brief.md:30,88, SKILL.md:49, H/tools/effort_test_taint.py:31 (about 9 homes). Project folder name in test_ref_judge.py:670, check_pointers.py:30, writer-brief.md:25,29, H/tools/agent_cost.py:31 (owner tools/session_cards/common.py:33 `PROJECT_NAME`). `MEMORY_DEFAULT` check_pointers.py:30 typed vs session_cards/common.py:35 derived. Three doc lists (test_docs_numbers.py DOCS 5, REVIEW_DOCS_HYDRA 3, check_pointers.py documents()). `AGENTS_DIR` test_docs_numbers.py:35 imported with a pointer comment by test_agent_files.py:11 (true).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** G/tests/test_docs_numbers.py holds five docs equal to progress.py numbers; no test imports the owners' constants for the fixtures.
- **Open:** unreached.
- **Hook gap:** no.

### L-F11-14: State folder layout, posture record, settings fragment vs live settings (I9-018, I9-038, I9-080, I9-121)
- **Source:** I9-018, I9-038, I9-080, I9-121; F11.
- **Kind:** K1, K6.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G/common.py:120,124,200 (owner) vs G/status_append.ps1:13-14 (re-derives $StatusDir, $AgentsDir) and G/cleanup_sweep.ps1:597-600 (folder names as strings); G/posture.py:14-19,50 vs cleanup_sweep.ps1:600 and README:35,75; G/sync_settings.py:36-39 `FRAGMENT`/`LIVE` vs test_docs_numbers.py:46-48 `SETTINGS` (second derivation); H/docs/agents/this-machine.md:71 typed path. The settings check runs on main only (skips when no settings.json sits beside gates).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers.py:142-146 holds live settings to the fragment; test_docs_numbers.py:54 holds `RESEND_MINUTES` name in README; no test ties PowerShell folder names to common.py.
- **Open:** unreached.
- **Hook gap:** no.

### L-F11-15: Media guard prose vs media_rule.py, and gh default repo, other repos named in a brief (I9-068, I9-069, I9-107, I9-110)
- **Source:** I9-068, I9-069, I9-107, I9-110; F11.
- **Kind:** K1, K5, K7, K8.
- **Verdict:** NOT VERIFIED (V11 "Not reached").
- **Homes (finder only):** G/media_rule.py (MEDIA_ROOT :6, TOOLS :7, TEMPLATE :11) vs G/README.md:21 and percall.py:4,115-117; brief-preamble.md does not mention the media guard. Repo name `splax77/hydra-deluxe` typed in H/CLAUDE.md:13, this-machine.md:15, M/hydra-deluxe-rename.md:11, test_publish.py:341-393; issue-tracker.md:16 says "Infer the repo from `git remote -v`". G/review_dispatch.py:36 `MAX_BRIEF_FOLDERS = 16`, :66-75 `judged_folder` docstring "approved on 2026-10-24" (a comment, not a Confirmed line); G/agents/reviewer.md:23 restates the rule; test_review_dispatch.py:224-286 docstring names "Get-JudgedFolder's rule", a function that no longer exists (searched G).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_media_rule.py pins behaviour; none holds README equal.
- **Open:** unreached.
- **Hook gap:** no.

### L-F11-16: Gate-file list in tests, trimmed-result save path, transcript layout readers (I9-151, I9-161, I9-184, I9-185)
- **Source:** I9-151, I9-161, I9-184, I9-185; F11.
- **Kind:** K1, K6.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G/tests/test_ref_judge.py:21 `GATE_FILES` hand-lists the gate's files (stale if ref_judge gains an import); G/shell_output.py:26-27,89-95 `saved_path` (one owner with pointers); transcripts layout answered in G/tests/replay_extract.py:25-28, tools/session_cards/common.py:32,503-513, H/tools/agent_cost.py:31,160, shell_output.py:95, cleanup_sweep.ps1:60, hydra_ui_mcp.py:65 with different depth rules.
- **Splits:** None (finder: purposes differ).
- **Ruling quotes:** None found.
- **Tests:** test_shell_output.py:133-134,374-389 pins the save path.
- **Open:** unreached.
- **Hook gap:** no.

### L-F11-17: Pointer-checker lists, "main" refs, mutation formats, and single-home rows (I10-004, I10-005, I10-009, I10-010, I10-021, I10-048, I10-074, I10-079, I10b-016, I10b-019, I10b-020, I10b-025, I10b-064, I10b-105 formats)
- **Source:** same ids; F11.
- **Kind:** K1, K6.
- **Verdict:** NOT VERIFIED (V11 "Not reached").
- **Homes (finder only):** G/check_pointers.py:33-35 `HOOK_FOLDERS`, `SKIP_FOLDERS`; RULINGS folder typed in check_pointers.py:42 and G/ref_judge.py:41 `RULINGS_DIR` (no test ties them); `refs/heads/main` typed in many gate files vs ref_judge.py:33 `MAIN`; `MUTANTS_FORMAT = 1` run.py:69 vs `FORMAT = 1` rewrite.py:86 (nothing ties them); game_files.py:31-41 constants vs typed copies in dump_build.py:9, clone-hero-mcp.md:15,60,62, this-machine.md:53; game_tools.py:47-57 vs stall_harness.py:796,854; constants.py:270-273 process names vs dump_build.py:55,57 and launch_patch.json:7; single-home rows build_slot.ps1:40-46 (260-character note), new_worktree.ps1:8-16, dump_build.py:20,25. The checker says archive "names retired things on purpose" (a claim).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** tools/test_check_pointers.py (checker behaviour only).
- **Open:** unreached.
- **Hook gap:** no.

---

## Finder F12 rows

### L-F12-01: Who owns the status-call shape (status_append.ps1, the preamble, progress.py, or the settings allow rule)? (SP4)
- **Source:** I1-019, I1-094; F12, V12.
- **Kind:** K1, K6, K8.
- **Verdict:** CONFIRMED. V12 adds: progress.py:21 itself says its prefix is spelled "as the allow rule in settings.fragment.json names it", so there are three owner claims, not two.
- **Homes:**
  - G/status_append.ps1:3-5 "one allow rule covers running this script. The call shape: the brief preamble's 'Status lines' section."
  - H/docs/agents/brief-preamble.md:19 "The status-call check in progress.py owns the exact shape." (and :15 states the shape too).
  - G/progress.py:21-27 `STATUS_PREFIX = '& "C:/Users/Patrick/.claude/gates/status_append.ps1"'` plus `_STATUS_CALL` regex.
  - G/settings.fragment.json:4 and live settings.json:10 allow rule `PowerShell(& "C:/Users/Patrick/.claude/gates/status_append.ps1"*)` (a second reader; any text after the prefix is allowed).
  - doc-review.md:9, derive-once-review.md:9, tools/session_cards/writer-brief.md:100, G/tests/replay.py:280 (second regex) type the shape (finder only; V12 did not re-read).
- **Splits:** status_append.ps1:5 names the preamble as home; preamble:19 names progress.py; progress.py:21 names the settings file. They differ for anyone changing the call shape. Also finder: preamble says `<your agent id>` bare; progress.py accepts bare or quoted; its printed line quotes the id (two spellings, not a disagreement).
- **Ruling quotes:** G/../agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable" (2026-09-05), :26 "the status updates should be brief but relayed in plain english" (finder; unchecked by V12). They say status is required, not the call shape.
- **Tests:** G/tests/test_docs_numbers.py:189 holds STATUS_PREFIX equal to the preamble; nothing holds the settings allow rule equal to STATUS_PREFIX (SN-2), nor doc-review.md:9, derive-once-review.md:9, writer-brief.md:100. test_progress.py:791 expects a worktree copy of the helper path is not a status call.
- **Open:** none.
- **Hook gap:** no.

### L-F12-02: Which git options take a value (delete_rule vs bash_windows_rule)? (SP1)
- **Source:** I1-043, I1-073; F12, V12.
- **Kind:** K2, K6.
- **Verdict:** CONFIRMED.
- **Homes:** G/delete_rule.py:30 `GIT_VALUE_OPTIONS = {"-c","-C","--git-dir","--work-tree","--namespace","--config-env","--attr-source"}` (`git_words` "the one reader of those", :105-111; reused by shell_output.py:130-132); G/bash_windows_rule.py:25 `GIT_OPTIONS_WITH_VALUE = ("-C", "-c")`, read at :146-153 by `_is_git_commit`.
- **Splits:** delete_rule lists seven value-taking options, bash_windows_rule two. `git --git-dir x commit -m <long text>` is a commit to delete_rule.git_words but not to `_is_git_commit`, so its text counts toward the 1,000-character limit there.
- **Ruling quotes:** None found naming both sides. Finder's cited Confirmed line review-and-hooks.md:10 "why did you only show me those two if there are more important patterns? yes block those too" (2026-11-06) is about the cmd, ref and path-quote shapes (V12: not about git options).
- **Tests:** none holds the two lists equal.
- **Open:** none.
- **Hook gap:** no.

### L-F12-03: Where is a shell command cut into pieces (three cutters)? (SP2)
- **Source:** I1-041, I1-084; F12, V12.
- **Kind:** K3, K6, K8.
- **Verdict:** CONFIRMED. V12 adds a mild K8 from the other side: delete_rule.py:76 claims "the one reader of where a command is cut" and bash_windows_rule.py:5-6 claims "_scan is the one reader of shell quoting" (two different things, both "one reader").
- **Homes:** G/delete_rule.py:29 `OPERATORS`, :75-77 `pieces()` (quote-blind, "the one reader of where a command is cut"); G/bash_windows_rule.py:19 `OPERATOR_CHARS = ";|&\r\n"` and its own quote-aware `_scan` (:5-6); G/media_rule.py:18-27 `_segments` (own loop, uses dr.OPERATORS.fullmatch; cuts only on an operator standing alone as a word); G/shell_output.py:55-60 reuses delete_rule.pieces; README:19 and preamble "cuts the command at its shell operators". Heredoc/assignment patterns bash_windows_rule.py:23-24 only; delete_rule.pieces and media_rule split heredoc bodies without knowing heredocs (finder).
- **Splits:** `echo "a;b" && cmake x` is cut inside the quotes by delete_rule and shell_output and not by bash_windows_rule. Media rule does not cut `a;b` or `ffmpeg x;rm y`.
- **Ruling quotes:** None found.
- **Tests:** none holds them equal (test_media_rule.py:164 and test_shell_output.py test separately; finder).
- **Open:** none.
- **Hook gap:** no.

### L-F12-04: What command word does a quoted first token name? (SP3)
- **Source:** I1-042, I1-071a; F12, V12.
- **Kind:** K3, K6.
- **Verdict:** CONFIRMED-CHANGED (V12). The finder framed one side "delete_rule leaves it unread" and the other "media_rule reads it". V12: there are four readers with three answers (leave, read, read, refuse-to-read), and the shell_output comment names the reason for its split. V12 notes "leaves a quoted delete unread on purpose" is a stated choice, a claim, not evidence.
- **Homes:** G/delete_rule.py:66-69 `word()` (does not strip quotes); G/delete_rule.py:2-6 and G/shell_output.py:44-52 comment "This lives here and not in delete_rule.word, because that rule leaves a quoted delete unread on purpose" (`_unquoted` strips a quoted first word); G/media_rule.py:14-15 `_word` strips quotes then calls dr._word; G/bash_windows_rule.py:137-143 `_command_at` refuses a first word that carries a quote; progress.py `command.startswith("git ")` is a fifth reader (finder only).
- **Splits:** for `"C:/tools/cmake.exe"`: delete_rule does not read it as a command; shell_output reads it as `cmake` and adds the exit-code tail; media_rule reads it and guards ffmpeg; bash_windows_rule does not read it.
- **Ruling quotes:** None found.
- **Tests:** G/tests/test_shell_output.py:99-100 pins delete_rule.word('"C:/tools/cmake.exe"') returning 'cmake.exe"' (delete_rule's side only).
- **Open:** none.
- **Hook gap:** yes, G/delete_rule.py (a quoted or wrapped delete is not read; the file's own docstring says so).

### L-F12-05: Is a `ch_call` to `ch_call` a key call needing `allow key run`, or refused outright? (SP5)
- **Source:** I1-007; F12, V12.
- **Kind:** K3, K6.
- **Verdict:** CONFIRMED.
- **Homes:** G/percall.py:58-82, :69-70 returns "ch_call to ch_call, which this check does not unwrap" (counted as a key call); H/tools/ch_mcp/front.py:491-504 `call_shape_refusal` refuses when `tool` is `ch_call` or `ch_reload` (:502-503) with docstring "The hook in percall.py reads the same two fields, so nothing is coerced here"; G/README.md:13; clone-hero-mcp.md:11,66; server.py:147-148.
- **Splits:** percall counts a `ch_call` to `ch_call` as a key call needing a typed `allow key run`; front.py refuses it outright without any allow, so the typed allow is spent on a call the server would refuse. V12 added: percall returns None (not a key call) when `tool` is not a string, while front.py:496 refuses it.
- **Ruling quotes:** None found.
- **Tests:** G/tests/test_percall_key.py:81 pins the hook side only; none holds front.py equal.
- **Open:** none.
- **Hook gap:** no.

### L-F12-06: Does a doubled backslash in a comment pass? (preamble says "outside double quotes") (SP6)
- **Source:** I1-077, S-A; F12, V12.
- **Kind:** K2, K8.
- **Verdict:** CONFIRMED. V12 did not open review-and-hooks.md, so the claim that "the user's ruling" in the docstring is not under Confirmed is unverified.
- **Homes:** G/bash_windows_rule.py:26,175-181 "A pair in double quotes passes ... (the user's ruling). A pair in a comment passes too"; code `not in (DOUBLE, COMMENT)`; refusal text :216-218; README:27 "a pair in double quotes passes"; H/docs/agents/brief-preamble.md:33 "A doubled backslash outside double quotes: put the text in a file ... or use forward slashes".
- **Splits:** the preamble's "outside double quotes" would include a comment; the code passes a comment. The pair in a comment is named by neither README nor preamble.
- **Ruling quotes:** review-and-hooks.md:10 "why did you only show me those two if there are more important patterns? yes block those too" (2026-11-06, finder) covers the cmd, ref and path-quote shapes; finder says nothing Confirmed speaks to the double-quote or comment pass. The quoted "ruling" text ("Exempt git commit") sits under `## Unconfirmed` at review-and-hooks.md:40,43 (finder only; V12 unchecked).
- **Tests:** none holds the preamble's prose equal; test_docs_numbers.py:192-196 holds only the 1,000.
- **Open:** none.
- **Hook gap:** no.

### L-F12-07: Which deletes and discards need a person's yes (delete_rule words vs the settings ask list)? (SP7; includes I1-044 git discard list)
- **Source:** I1-034, I1-044, I1-045, I1-046; F12, V12.
- **Kind:** K3, K6.
- **Verdict:** CONFIRMED. V12 did not re-read the live settings.json (its grep used a wrong path).
- **Homes:** G/delete_rule.py:19 seven delete words `{"rm","rmdir","rd","del","erase","remove-item","ri"}`; G/delete_rule.py:31-35 `GIT_DISCARD_WORDS`, `GIT_DISCARD_ACTIONS`; G/settings.fragment.json:59-63 ask list (`Bash(rm *)`, `Bash(git clean *)`, `Bash(git reset --hard*)`, `PowerShell(Remove-Item *)`; live settings.json:71-72 per finder); preamble:43 lists "such as `git clean`, `git reset --hard`, `git reset --merge`, `git checkout -f`, `git worktree prune` and `git stash drop`"; README:19 "`WORKTREE_FOLDERS`, `GIT_DISCARD_WORDS` and `GIT_DISCARD_ACTIONS` in that file are the one list" (finder); cleanup_sweep.ps1:25.
- **Splits:** the main session is asked on four patterns; agents are refused a wider list (rm, rmdir, rd, del, erase, remove-item, ri; git clean, reset --hard/--merge, checkout -f, restore ., switch -f, branch -D, stash drop/clear, worktree remove/prune, branch -d -f). They differ for `del`, `rd`, `rmdir`, `erase`, `ri` in the main session (not asked) and for a plain `rm` with no force flag by an agent (passes the hook; only recursive or forced are judged, delete_rule.py:161).
- **Ruling quotes:** finder cites review-and-hooks.md:11 "... if a hook is doing something unnecessary or contains a lot of arbitrary rules that needs to scrutinized" (2026-10-31) as speaking generally; it names neither side. review-and-hooks.md:17 (see L-F10-11) speaks to the worktree roots.
- **Tests:** none holds the settings ask list equal to DELETE_WORDS or GIT_DISCARD_*; test_delete_rule.py:266 pins one case.
- **Open:** none.
- **Hook gap:** no.

### L-F12-08: Which tool names always pass so a report can be handed back (REPORT_TOOLS)
- **Source:** I1-017; F12, V12.
- **Kind:** K6, K1.
- **Verdict:** CONFIRMED.
- **Homes:** G/progress.py:19 `REPORT_TOOLS = ("SubagentHandback","StructuredOutput")`; G/percall.py:102 literal `"SubagentHandback"` (V12: a save of hand-back text, a different job from the pass, but it spells one of the two names); percall.py:168 uses `progress.REPORT_TOOLS`; README:11; agents/reviewer.md:25; agent_stop.py:40; preamble:21.
- **Splits:** None between values. StructuredOutput appears only in progress.py.
- **Ruling quotes:** None found.
- **Tests:** none holds the percall.py:102 literal equal to REPORT_TOOLS.
- **Open:** none.
- **Hook gap:** no.

### L-F12-09: Which shell tools do the gates judge (the pair Bash/PowerShell)?
- **Source:** I1-031; F12, V12.
- **Kind:** K6.
- **Verdict:** CONFIRMED (minor). V12 did not read shell_output TAILS keys.
- **Homes:** G/delete_rule.py:168 `("Bash","PowerShell")`; G/progress.py:20 `SHELL_TOOLS = ("Bash","PowerShell")`; percall.py uses progress.SHELL_TOOLS at 47, 112, 133, 147 (V12); shell_output.py:35 `TAILS` keys (finder); bash_windows_rule judges Bash only (:235).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** no.

### L-F12-10: What marks a whole-library run (the `--whole-library` flag vs "any run over the whole library")?
- **Source:** I1-004; F12, V12.
- **Kind:** K1, K3.
- **Verdict:** CONFIRMED. V12 did not open H/src/cli/batch_scope.h.
- **Homes:** G/percall.py:14 `LIBRARY_FLAG = "--whole-library"`, checked :47-48 on shell tools only; H/src/cli/batch_scope.h:10 `kWholeLibraryFlag = "--whole-library"` (finder); compare_db.py:29 pointer; README:13; agent-rules.md:14; brief-preamble.md:49; this-machine.md:91; M/no-full-library-runs.md:11 (finder).
- **Splits:** the hook reads one flag spelling; agent-rules.md:14 and the memory note define the rule by what it covers (any run over the whole library); a compare or timing run, or `hydra_batch` with no folder, is covered by the app's guard but not by the hook (finder, V12 agrees on the per-situation point).
- **Ruling quotes:** H/docs/agents/rulings/review-and-hooks.md:16 "i want to be able to give explicit approval for a full library run." (2026-10-25, finder; V12 did not re-read). It says approval is wanted; it names neither side of the flag-vs-coverage difference.
- **Tests:** none holds the homes equal (test_percall_rewrite.py:73,135 and test_progress.py:425 use the flag as input; tests/replay.py:297 has a third copy of the rule's words).
- **Open:** none.
- **Hook gap:** no.

### L-F12-11: Which crash mode does each part of the prompt hook use (prompt.PARTS vs each script's `__main__`)? (I2-075, I1-056, I1-049, I1-011)
- **Source:** I1-011, I1-049, I1-056, I2-075; F12, V12.
- **Kind:** K1, K6.
- **Verdict:** CONFIRMED for the call sites (V12 read posture.py:55, waiver.py:50, percall.py:176; common.py and README:97-100 not re-read).
- **Homes:** G/prompt.py:17-18 `PARTS = (("posture","loud"),("percall","closed"),("waiver","loud"))` ("each mode is the one its script's __main__ uses", V12 did not display the comment); posture.py:55 loud; waiver.py:50 loud; percall.py:176 closed; delete_rule.py:187, publish_gate.py:135, review_dispatch.py:178 closed; agent_stop.py:297, shell_output.py:201, prompt.py:67 loud (finder); G/common.py:52-75 (owner) vs README:97-100.
- **Splits:** None; the modes agree today.
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers.py:148 holds that one process runs; test_posture.py:251 patches PARTS; no test holds PARTS' modes equal to each script's `__main__` mode; tests/test_common.py checks the modes in common only.
- **Open:** none.
- **Hook gap:** no.

### L-F12-12: Which commands get the exit-code tail, and is the list current (stale `mutation_probe.ps1`; test-program lists in rule text)? (I1-063)
- **Source:** I1-063, I1-064, I1-065, I1-069; F12, V12.
- **Kind:** K1, K8.
- **Verdict:** CONFIRMED. V12: H/tools/build_slot.ps1 exists on main; its header (lines 10-22) supports percall.py:19's ownership claim, but V12 did not read the whole file, so ownership of every pass rule is unchecked.
- **Homes:** G/shell_output.py:20-22 `BUILD_TEST_COMMANDS` lists `mutation_probe.ps1` and `hydra_precheck`; `mutation_probe.ps1` does not exist on main's tools/ (V12 `ls`: "No such file"); plan 2026-11-07-shell-output-to-disk.md:144 "(removed; `git show ed0aa857:tools/mutation_probe.ps1`)" and :144,:146 hold the full lists; README:25 names the constants only; preamble:35 and :47, CLAUDE.md rules 1/14, agent-rules.md rule 14, README:17, model-postures.md:9, agents/reviewer.md:21, executor.md:11 name hydra_tests, hydra_uitest, hydra_replay (agents may not run) and hydra_precheck (agents may run from main); build_cpp.ps1:4-9 header.
- **Splits:** the code adds the tail to hydra_precheck (an agent may run it) and to a script that is gone; the rule texts split the same programs into "agents may not run" and "agents may run".
- **Ruling quotes:** finder cites agents-and-workflow.md:8,9 (builds only by main session, 2026-11-07); V12 did not re-read. Names neither side.
- **Tests:** none holds the list equal to any doc (finder).
- **Open:** none.
- **Hook gap:** no.

### L-F12-13: How is a background task start/end recognised, and does the README hold the strings? (I2-046)
- **Source:** I2-046; F12, V12.
- **Kind:** K6, K8.
- **Verdict:** CONFIRMED.
- **Homes:** G/agent_stop.py:44-53 (`_BACKGROUND_STARTED`, `_TASK_ENDED`, `_STILL_RUNNING`) with comment "as read from real agent transcripts (gates/README.md, the agent_stop paragraph)"; V12 grep of G/README.md for "task-notification" found nothing, so the README holds no such strings; tools/session_cards/common.py:66-71 `TASK_NOTICE = re.compile(r"<task-notification>.*?</task-notification>", re.S)`, `DROPPED_HUMAN_PREFIXES`; G/review_rules.py:95-103 uses `"human"` origin (a third way); tests/test_agent_stop.py:121, test_session_cards.py:699-702 build notices (finder).
- **Splits:** None between values; two readers of one notice shape with different jobs (end-of-task detection vs stripping from cards).
- **Ruling quotes:** None found.
- **Tests:** none holds the two regexes equal (V12 beyond the finder not searched).
- **Open:** none.
- **Hook gap:** no.

### L-F12-14: Does the session-card start hook parse the summary lines the scripts print? (I2-109)
- **Source:** I2-109; F12, V12.
- **Kind:** K1, K5, K6.
- **Verdict:** CONFIRMED for the producers only (V12: batch.py:75 prints `sessions=%d batches=%d waves=%d`; prune_list.py:147 prints `paths=... held_back=... mb=...`). V12 did not open the consumer session_cards_start.ps1 or test_h53.
- **Homes:** G/session_cards_start.ps1:10-17,327-353 (runs prepass, batch, prune_list; reads one summary line each); G/tests/test_h53_session_cards_start.ps1:57-69 (fake scripts printing the same format), :121 asserts the order; tools/session_cards/test_session_cards.py:668 asserts real batch.py prints `sessions=0 batches=0`; hook text Get-CardNote :269-290 spells out commit steps and the docs-check command (also in H/CLAUDE.md) and says it relies on gates\review_rules.py explain_main_move (:267-268); finder only for these.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_h53 holds the hook vs its own fake format; nothing holds the real format equal to the fakes.
- **Open:** V12: read session_cards_start.ps1:327-353 and its test to settle whether the parse matches.
- **Hook gap:** no.

### L-F12-15: Quoted "user rulings" in hook comments and README that are not under `## Confirmed` (S-A)
- **Source:** I1-047, I1-077, I1-009(neighbour), S-A; F12.
- **Kind:** K8.
- **Verdict:** NOT VERIFIED (V12 "Not reached": the S-A claims; did not open the rulings files).
- **Homes (finder only):** G/bash_windows_rule.py:173 "A commit's own text does not count (the user's ruling)" and :178 "(the user's ruling)"; G/README.md:27 "the user ruled on 2026-11-06 'Exempt git commit'" (finder: under `## Unconfirmed`, review-and-hooks.md:40,43); G/README.md:19 attributes "trying to ban specific command patterns is a losing battle because there will always be another pattern nobody thought to check." to a 2026-10-31 user ruling (finder: found only in docs/sessions/2026-10-31-6777b546.md:25 and two plans, not under Confirmed; review-and-hooks.md:11 reads differently); README:21 "D189, brought back by D273" (decision log, not opened).
- **Splits:** None.
- **Ruling quotes:** finder only: review-and-hooks.md:11 "im not too concerned about the exact number as much as efficiency and functionality ..." (2026-10-31).
- **Tests:** none.
- **Open:** unreached.
- **Hook gap:** no.

### L-F12-16: Prose lists of what the delete, bash and media hooks refuse vs the code (literal-path classes, find tokens, five shapes, refusal texts, PATHCONV word)
- **Source:** I1-035, I1-036, I1-037, I1-038, I1-040, I1-045, I1-046, I1-047, I1-062, I1-074, I1-075, I1-078, I1-079, I1-080, I1-081, I1-083, I1-086..089; F12.
- **Kind:** K1, K3, K5, K7.
- **Verdict:** NOT VERIFIED (V12 "Candidates not reached" lists these; V12 says none contradicted anything it read).
- **Homes (finder only):**
  - Literal-path classes: delete_rule.py:24-26 `NOT_LITERAL` (nine character classes; space only through word splitting); refusal text :60-63 lists five; README:19 lists three; preamble:43 lists variables, wildcard, `..`, quote, space. `,` `%` `?` are refused by code and named in no prose.
  - `ref:.dotpath`: code needs a letter right after `:.`; preamble says "a dotted path" (looser).
  - MSYS_NO_PATHCONV=1: preamble names it only for cmd; README and code accept it for two shapes.
  - Preamble:33 states five shapes and fixes in the same words as bash_windows_rule.py:211-228 (K5).
  - Single code homes with prose pointers and no lists: DELETE flags (delete_rule.py:20-23,80-86), find tokens (:27-28,132-140), media_rule TOOLS (media_rule.py:7-11; searched ffmpeg/yt-dlp across gates, agents, model-postures, CLAUDE.md, session_cards, settings, Hydra docs/agents).
  - Shell_output error words/plan copies: plan shell-output-to-disk.md:142 states the full list; README:23 gives no values.
- **Splits:** None.
- **Ruling quotes:** review-and-hooks.md:10 "why did you only show me those two if there are more important patterns? yes block those too" (2026-11-06) speaks to cmd, ref and path-quote shapes (finder).
- **Tests:** none holds the prose lists equal; test_docs_numbers.py:192-196 holds only the 1,000 length.
- **Open:** unreached.
- **Hook gap:** yes, G/bash_windows_rule.py, G/media_rule.py and G/delete_rule.py (wrapped, quoted or longer-quoted forms are not read, stated in each file's own docs).

### L-F12-17: Is an id/status call validity decided in two places (progress.py and status_append.ps1), and other hook-text vs preamble copies? (I1-023, I1-024, I1-009, I1-010, I1-011 prose)
- **Source:** I1-009, I1-010, I1-023, I1-024; F12.
- **Kind:** K1, K5, K6.
- **Verdict:** NOT VERIFIED (V12 lists I1-009, I1-010, I1-023, I1-024 as taken from the finder).
- **Homes (finder only):** G/progress.py:80-86,111-115 ("This status call names %s, but your agent id is %s, so it is refused and not counted") and status_append.ps1:38 (refuses a name with no agent record, exit 1) both decide "is this name valid"; preamble:17 states both layers; progress.py:119-128 exempts `git ` commands and refuses other calls naming the helper, preamble:19 states the same; media ask rule percall.py:133-134 vs README:21; rewrite-last percall.py:1-6,142-158 vs README:11,25 and plan :80.
- **Splits:** None (finder: all agree).
- **Ruling quotes:** agents-and-workflow.md:25,26 as in L-F12-01.
- **Tests:** G/tests/test_progress.py and test_status_append.ps1 each test one side; none holds the two equal; none holds README sentences equal.
- **Open:** unreached.
- **Hook gap:** no.

### L-F12-18: Does percall's comment on build_slot.ps1 "owns what a pass allows" hold? (SN-5)
- **Source:** SN-5; F12, V12.
- **Kind:** K8.
- **Verdict:** NOT VERIFIED (V12: header lines 10-22 describe slot lock files and `-NewPass`/`-Claim` calls, which supports the claim; V12 did not read the whole file, so ownership of every pass rule stays unchecked).
- **Homes:** G/percall.py:19 "Hydra's tools/build_slot.ps1 owns what a pass allows"; G/README.md:15 "Hydra's `tools/build_slot.ps1` owns the limits on it"; H/tools/build_slot.ps1:10-22,73 (the limit "three" is in a comment there and also in CLAUDE.md rule 14, agent-rules.md:20, brief-preamble.md:57).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** whole of build_slot.ps1 not read.
- **Hook gap:** no.

---

## End summary

Candidates the verifier REFUTED: none. No row was refuted by any of V10, V11, V11b or V12. Rows the verifier changed (CONFIRMED-CHANGED): L-F10-05, L-F10-06, L-F10-13, L-F11-01, L-F11-02, L-F12-04 (the verifier's reading wins in each).

Rows no verifier reached (NOT VERIFIED): L-F10-15, L-F10-16, L-F10-17, L-F10-18, L-F10-19, L-F10-20, L-F10-21 (grep only, "PARTLY"), L-F10-22, L-F10-23, L-F11-09, L-F11-10, L-F11-13, L-F11-14, L-F11-15, L-F11-16, L-F11-17, L-F12-15, L-F12-16, L-F12-17, L-F12-18 (header read only).

Verifier-found extras folded in: model-postures.md lives at C:\Users\Patrick\.claude\model-postures.md, not in gates (V11 S8, in L-F10-11); py_mutate.py comment "none when unset" vs code 0 (V11 S9, in L-F11-03); percall returns None for a non-string `tool` while front.py refuses it (V12, in L-F12-05).
