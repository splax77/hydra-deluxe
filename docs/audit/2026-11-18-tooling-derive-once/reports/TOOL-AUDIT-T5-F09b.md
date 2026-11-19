# TOOL-AUDIT-T5-F09b (re-send of F09)

Rows taken, from F09's "Rows not reached" list: I6-148, I6-205, I6-206, I6-215, I6-193, I5-176, I5-190 (read as the "python path" item, see its own section), I4-092, I4-105.
Paths: Hydra = C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout, read there). Gates = C:\Users\Patrick\.claude\gates. SC = C:\Users\Patrick\.claude\tools\session_cards. Short names: AM audit-method.md, AR agent-rules.md, TM this-machine.md, UT ui-testing.md, GR gates\README.md, CP tools\check_pointers.py.
Not in any row: no `## Confirmed` ruling line speaks to I6-148, I6-205, I6-206, I6-215, I6-193, I5-176, I4-092 or I4-105 (searched docs\agents\rulings\*.md Confirmed sections for audit-source names, hydra.db, topic files, decompile, pointer check, CONTEXT.md, old cards). No test in gates\tests holds any of these homes equal (searched gates\tests for the same words), except where named.

## I6-148 where the audit's own sources live (the decision log, the old audits, the question files)
Homes:
- AM:190: "The M39 trail is in `docs/audit/2026-10-03-derivation-audit.md` (finding 306), `docs/audit/2026-10-04-phase-3-5-questions.md` (question 4), `docs/audit/2026-10-03-fix-decisions.md` (D48 and D50), `docs/audit/2026-10-11-concept-audit/findings/V5.md` (M39) and `docs/audit/2026-10-11-concept-audit/questions.md`... `docs/audit/2026-10-29-audit-of-audits/`" and a handoff "(removed; `git show 444631e6:...`)". All these files exist today (checked with ls).
- AM:142 names concept-audit questions.md; AM:164 names derivation-audit.md; AM:176 names audit-of-audits\summary.md; AM:108 names fix-decisions.md and fix-triage.json.
- AR:20 and AR:17-18 name fix-decisions.md and 2026-10-31-silent-changes.md.
- AM:38 tells the audit plan to read "the decisions and plans the audit must read: `docs/audit/2026-10-03-fix-decisions.md`, `docs/adr/` and the plans under `docs/superpowers/plans/`".
- brief-preamble.md:89 and derive-once-review.md:90, doc-review.md:91, test-rubric.md:65 each name fix-decisions.md (D107, D9).
- SC\writer-brief.md:30: "Decision log: `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\audit\2026-10-03-fix-decisions.md`. Its entries start with `**D<n>`." (full path copy; K1)
- tools\doc_shots\shrink.py:18, tools\mutate\run.py:91 name fix-decisions.md (comments).
- Gates code: none (searched gates\*.py, README.md).
- Each audit brief (the T5 brief itself, line 46) restates where the user's words live: that is the same list again, outside my scope to count.
Instructions per reader: AM tells an audit planner to read the log first; AR says nothing about audits' own sources; writer-brief tells a card writer where the log is by full path (card writers skip the preamble, so this copy is the only one they see); main session vs agent: no difference stated; worktree vs main checkout: the card writer's path is the main checkout's, the others are repo-relative.
Kinds: K1 (path of decision log in ~8 places, all agree today); K7 weak (no page names which page owns the list of "where the user's words live").
Splits: none. Pointer check (CP) reads docs\agents\*.md and would flag a missing file; it does not read writer-brief.md or tools\ comments, so those copies are unchecked.
Test holding equal: none for these paths. CP checks docs\agents pointers exist, which is not equality.

## I6-205 where hydra.db lives
Owner: Hydra src\app\config.cpp:96-98: `std::string db_path() { if (!g_overrides.db_path.empty()) return g_overrides.db_path; return join_folder(exe_dir(), "hydra.db"); }` (config.h:50 declares it; `PathOverrides.db_path` at config.h:106). The K8 check: TM:77's claim "next to the program that opened it (`db_path()` in `src/app/config.cpp`)" is TRUE for the default path. It omits the override (`--db`), which UT:199 mentions.
Homes of the placement/paths:
- TM:77: dev copy `C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra.db`, installed `C:\Program Files\Hydra\hydra.db`.
- UT:346: "Hydra reads `hydra.db` and `hydra_settings.ini` from the folder its exe sits in (`exe_dir` in `src/app/config.cpp`)". UT:350: "from the installed folder, `C:\Program Files\Hydra`".
- tools\hydra_ui_mcp.py:55 `DEFAULT_LIBRARY_DIR = r"C:\Program Files\Hydra"`; :72 `SNAPSHOT_FILES = ("hydra.db", "hydra_settings.ini", "hydra_ui.ini")`. UT:193 repeats the three file names and the order of lookup (`library_dir`, `HYDRA_LIBRARY_DIR`, `DEFAULT_LIBRARY_DIR`).
- tools\compare_db.py:51 and :87 hard-code `C:\Program Files\Hydra\hydra_settings.ini` and `...\hydra.db?mode=ro`.
- UT:195 says "`snapshot_library` is the only code that knows the library folder's path". compare_db.py:87 also names the installed folder, so that sentence is wrong as written (K8 candidate). I did not run anything to confirm what compare_db.py does with it; its docstring tells an agent to type the path in a python one-liner.
- Memory notes: none name hydra.db's placement (grep of the memory top folder; archive not read).
- Gates, agents\, skills, CLAUDE.md files: none.
Table: TM says check which file (dev vs installed) before comparing; UT says never open the real one in window mode, snapshot only; compare_db.py says copy the installed one by backup; hydra_ui_mcp defaults to the installed folder. Main session vs agent: UT's snapshot is a main-session MCP tool; agents get TM's rule. Card writers: no home. Worktree vs main: TM's dev path is the main checkout's build-cpp; a worktree build has its own hydra.db next to its own exe, which TM does not say.
Kinds: K1 (installed folder path in 4 places); K8 (UT:195 "only code that knows the library folder's path"); K3 weak (TM:77 omits the override).
Splits: UT:195 "only code that knows" against compare_db.py:51,87 and TM:77/UT:350 naming the folder.
Test: tools\test_hydra_ui_mcp.py exercises hydra_ui_mcp.py's own constants only; nothing holds TM and the code equal.

## I6-206 read a copy of the database (how to copy it)
Homes (the instruction differs):
- TM:81: "Copy the database to your scratch folder and read the copy... copy `hydra.db-wal` and `hydra.db-shm` along with `hydra.db` whenever they are there. The bare `hydra.db` alone can miss the newest writes."
- UT:195: the helper copies "with SQLite's online backup instead, through a read-only connection, whenever a `hydra.db-wal` sits beside it... So the snapshot has no `-wal` or `-shm` file of its own"; with no -wal "copies `hydra.db` as a plain file".
- Code: tools\hydra_ui_mcp.py `copy_database` (about lines 108-132): no `-wal` -> `shutil.copy2`; else `sqlite3 ... ?mode=ro` `.backup()`. Its comment: a read-only open of a WAL database with no -wal/-shm "creates them, which writes to the library folder".
- tools\compare_db.py:82-88: "the copy is taken through SQLite's backup API, which reads a consistent snapshot while the live connection keeps writing, and leaves a single-file copy with no `-wal`" with a python one-liner using `file:C:/Program Files/Hydra/hydra.db?mode=ro`.
- Store: src\store\record_store.cpp:838-847 sets `PRAGMA journal_mode=WAL` (the owner of "WAL mode"); src\store\library_file.cpp:139 notes side files.
- Memory notes and gates: none (the grep hit only the reverse-engineer skill's own WAL text, SKILL.md:186,188 and references\native-and-artifacts.md:38, about other databases).
Table: TM tells an agent to copy the three files by hand; UT/hydra_ui_mcp/compare_db tell a tool or agent to use the backup API and end with one file. At the limit (installed Hydra open): TM's file copy can catch a write halfway, which UT:195 says is the reason for the backup; compare_db.py says the same for the real database. Main session vs agent: the MCP tool is main-session; an agent reading TM does a plain copy. Card writers: no home. A read-only `mode=ro` on a copy of a WAL db with no -wal creates files (the hydra_ui_mcp comment), which TM does not warn about.
Kinds: K3/K2 (two instructions for the same job: TM's plain copy of three files vs the backup API; they disagree on method, and UT:195's own words call the plain copy unsafe while Hydra is writing); K1 (the WAL explanation twice, plus a third in compare_db.py).
Split: TM:81 "copy `hydra.db-wal` and `hydra.db-shm` along with `hydra.db`" against UT:195 / compare_db.py:82-88 "a plain copy of a database another program is writing can catch it halfway... use the backup".
Test: tools\test_hydra_ui_mcp.py:375-424 tests the helper (wal present, no wal). Nothing tests TM:81.

## I6-215 the eight topic files under docs\agents\rulings\
Homes:
- docs\agents\rulings\README.md:7-14 (the index) lists: agents-and-workflow, review-and-hooks, ui-and-display, code-and-engine, clone-hero, questions-and-audits, docs-and-writing, tests-and-runs, each with a one-line scope. Actual folder (ls): README.md plus exactly those eight; they agree today.
- Hydra CLAUDE.md "Rulings and this machine": "indexed by its `README.md`" (pointer, no list).
- tools\compare_db.py:26 names `docs/agents/rulings/tests-and-runs.md`. test-rubric.md:78 names code-and-engine.md.
- Gates: ruling_rules.py takes `--topic <file name>` as an argument and `is_rulings_path` (line 113); no list of names (searched gates\*.py, README.md:57-61). Tests ruling_rules / ref_judge use `ui-and-display.md` (test_ruling_rules.py:24,244,434,443; test_ref_judge.py:675) as an example, clone-hero.md once.
- Session-card writers: no home.
- README.md line 41 repeats the filing steps: "The question reads `File this ruling in docs/agents/rulings/<topic file>?`" - generic.
Table: README lists topics for an agent choosing which file to open ("Open only the topic file your task needs"); the hook accepts any file under the folder; no code checks the README list equals the folder.
Kinds: K7 weak (a list of file names kept only in the README, nothing checks it against the folder; a ninth file would not be flagged); K1 (names in tests/tools as examples).
Splits: none. Test holding equal: none; CP (tools\check_pointers.py) checks pointers to files exist, and skips pointers inside quoted ruling text (CP:234-245, `in_quotes`); it does not check a list is complete.

## I6-193 where the Clone Hero decompile is
Owner: docs\clone-hero\code\README.md:237: the decompile "lives outside it, beside that build's game dump, at `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\ghidra\ghidra_decompile_all.txt`". K8 check: TM:27's claim ("`docs/clone-hero/code/README.md` names its path and says what it holds") is TRUE; the file exists on disk (ls of ...\6142\ghidra shows ghidra_decompile_all.txt). TM:27 states no path: a pointer.
Other homes of the dump root `...\hydra-data\ch-dumps`:
- tools\ch_mcp\build_facts.py:28 `DUMPS_ROOT = r"C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps"` (used by chdis.py:218 and facts_path).
- tools\ch_mcp\kit_methods.json:3 and launch_patch.json:4 `"dump": "...\\ch-dumps\\6142\\dump.cs"`.
- tools\ch_mcp\README.md:7, :12, :20, :58 (the same root and 6142\dump.cs in commands). README:7: "The dump is too big for the repo, so it lives outside it, in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\<build>\`."
- docs\clone-hero\README.md:30 mentions "permanent records" rules only. tools\ch_mcp\tests\test_dump_facts.py:282 builds a ch-dumps path from `_REPO_ROOT`.
- Gates, memory, agents, skills: none (verify-before-claiming-a-source-is-wrong.md:16 only says "decompile files").
Kinds: K1 (dump root in 3 code/data files and 4 README lines; code owner is build_facts.DUMPS_ROOT for the Python, code/README.md:237 for the Ghidra file); K6 weak (kit_methods.json and launch_patch.json each hold their own full path to dump.cs instead of reading DUMPS_ROOT).
Splits: none (all agree today). Test: none holding them equal.

## I5-176 the pointer check at session start (GR:71 vs check_pointers.py)
Owner: Hydra tools\check_pointers.py. Compare GR:71 with the code:
- GR:71: checks "every file path and hook name in the memory notes, `CLAUDE.md` and `docs/agents/`"; "when nothing is broken it prints nothing, and when the check itself fails it says so in a `systemMessage`"; "always exits 0"; run by hand `py -I tools/check_pointers.py`.
- Code: `documents()` (CP:63-79) reads CLAUDE.md, every .md under docs\agents (including rulings\) and the top level of the memory folder; the archive is skipped (stated in its docstring: archive "holds retired notes"). The docstring (CP:1-17) matches GR:71 on exit codes: --session-start always exits 0 (CP:293-305). By hand it exits 0, 1 or 2 (CP:14-15); GR:71 says "always exits 0" which is true only for --session-start (K2, small: the by-hand form it tells the reader to run exits 1 when something is broken).
- Hook names: `GATES_DEFAULT = r"C:\Users\Patrick\.claude\gates"` (CP:31), `HOOK_EXTENSIONS` and `HOOK_FOLDERS = ("", "lib", "tests")` (CP: near 33-38): its own list of where hooks live. `MEMORY_DEFAULT` (CP:30) is a full copy of the memory folder path (K1; the folder name derives from the project path, which SC\common.py:33 `PROJECT_NAME` also holds as a constant: two places spell the same project folder).
- Registrations: settings.json:111 and settings.fragment.json:101 both run it with `-I -S ... --session-start`, timeout 15. sync_settings.py copies fragment to live (owner); gates\tests\test_sync_settings.py tests the sync mechanism on temp files; test_docs_numbers.py checks the live file carries the fragment (`sync_settings.check`, README text in GR:~144-148).
- The hook message printed (CP:299-303): "Broken file pointers in the agent docs and memory notes (tools/check_pointers.py; the memory notes are in %s). Fix them alongside the user's request:" - tells the main session to fix; a worktree against main checkout: GR:71 says it reads the main checkout's docs, and CP defaults `--repo` to "the one holding this script", which is the main checkout because the registered path is the main checkout.
- Hydra CLAUDE.md names it nowhere; docs\agents: grep for `check_pointers` in docs\agents\*.md, CLAUDE.md, memory: no hit.
Table: card writers and agents do nothing (it is a main-session SessionStart message). At the limit: exit code rule above.
Kinds: K2 (GR:71 "always exits 0" vs CP by-hand exits 1); K1 (the memory folder path, the gates path); K8: none (GR:71's owner claim is correct).
Test holding equal: tests of check_pointers itself: none found under tools\ with that name (I searched tools\ for test_check_pointers: not found); the docs-vs-code C++ test does not read it.

## I5-190 test-list for hook merges, and the python path (my reading of "I5-190-related python path")
F09 listed "I5-190-related python path" as shallow. I took I5-190's row (GR:156, the shared-code list for hook merges) and the python-interpreter path that the hook test commands carry.
A. Which hook tests a merge runs. Homes of "common.py, lib\, settings.fragment.json, tests\lib\":
- GR:156: "A merge runs the test files for the scripts it changed. It runs every test only when it touches what every hook shares: `common.py`, `lib\`, `settings.fragment.json` or `tests\lib\`."
- AR:7: "For hooks, the main session runs the whole suite only when the change touches shared code: `gates\common.py`, `gates\lib`, `gates\settings.fragment.json`, `gates\tests\lib`." Same sentence names `hydra_precheck.exe --tests` as the tool that "decides what counts as shared code and when the whole suite runs, so nobody types the test list by hand" (K8 candidate: tools\precheck\test_pick.cpp picks only tests\*.cpp for Hydra C++ (test_pick.cpp:25-35, 64-65, 178); a search of tools\precheck, tests\test_precheck_pick.cpp for gates, hook, fragment, .py found nothing. So for hooks the list exists only as text).
- Memory note merge-tests-touch-changed-files.md (body): "a merge runs only the tests that read the changed files. The whole suite runs only when shared code changed." with no list.
- Ruling: docs\agents\rulings\tests-and-runs.md:17 (`## Confirmed`; Unconfirmed begins at line 31): "- **2026-10-14** "why do we need to run the full suite every time a small change is made? this is a big waste of time. all the parts you didnt change will remain the same" (session d5fcf777)". It says only run what changed; it names no list. The Unconfirmed line 34 is a note's wording, a home.
- Dir facts: gates\lib exists (empty of .py per my ls), gates\tests\lib holds harness.ps1.
- Test holding the list equal: none. test_docs_numbers.py checks budget numbers and retired hook names, not this list.
B. The python interpreter path `C:\Users\Patrick\AppData\Local\Python\pythoncore-3.14-64\python.exe`:
- Owner of the mechanism: sync_settings.py (fragment -> settings.json). Copies: settings.fragment.json (9), settings.json (9, same hooks; kept equal by sync_settings and checked by test_docs_numbers via `sync_settings.check`), GR:110, GR:114, GR:145, install_ref_gate.ps1:10 (`$Python` default), publish.ps1:16, tests\test_shell_output.py:74, TM:7, derive-once-review.md:124. All agree today.
- Instruction conflict: TM:7 "Run Python as `py`. It starts the system Python 3.14.5 at `...python.exe`" vs GR:114/145 and derive-once-review.md:124 "with `C:\...\python.exe -I -m unittest discover...`": same file, two spellings (K1; K3 weak because `py` without -I loads the script folder, the reason GR:114 gives for -I -S).
Kinds: K1 (list in 3 places; python path in about 8 places); K8 (AR:7 "that tool decides" is false for hook merges); K7 (the hook shared-code list is a prose rule with no check, home unnamed between GR:156 and AR:7).
Splits: AR:7 owner claim (precheck decides) against GR:156 stating the list itself and precheck code holding no such list.

## I4-092 the default CONTEXT.md path used by the card pre-pass
Owner of the repo path: SC\common.py:36 `HYDRA_REPO = os.path.join(HOME, "Downloads", "Hydra", "hydra-test")`.
Homes:
- SC\prepass.py:456 `--context` default `os.path.join(common.HYDRA_REPO, "CONTEXT.md")`.
- SC\common.py:554-568 `build_tags(context_md)` parses CONTEXT.md bold terms with `^\*\*([^*]+)\*\*:` (the same file read as text with its own format rule). Hydra's CONTEXT.md heads terms in that form (checked: "**Chart**:" under "### Library").
- Hydra's other readers of CONTEXT.md: tools\precheck\decisions.cpp:317-321 names "CONTEXT.md" in git ls-tree and treats it as a decision source; check_3.cpp:534 message; tests\test_docs_match_code.cpp:128 reads it as a doc. They do not parse bold terms. docs\agents\domain.md and Hydra CLAUDE.md point at "`CONTEXT.md` ... at the repo root" (pointers).
- The repo path itself is spelled again in gates\delete_rule.py:17 (`WORKTREE_FOLDERS` first entry) and in session_cards_start.ps1:171 which reads common.HYDRA_REPO rather than copying it (good).
Kinds: single owner for the default path; K6 weak: the "which words are terms" rule is parsed only in common.build_tags (searched tools\precheck, tests, gates for the `**Term**:` pattern: not found elsewhere).
Splits: none. Test: SC\test_session_cards.py covers build_tags (not read in depth).
One home beyond the row: nothing else in scope states the CONTEXT.md path as a value.

## I4-105 the old card folder migrate.py reads
Homes:
- SC\migrate.py:98-99: `--old-cards` default `os.path.join(common.DATA_ROOT, "cards")`, `--old-index` default `os.path.join(common.DATA_ROOT, common.INDEX_FILE)`. Docstring lines 1-9: one-time copy; "old folder and the old index are only read, never changed"; a card already in the cards root with other bytes is "left alone" (printed message at line ~104).
- Owner of DATA_ROOT: common.py:34 `DATA_ROOT = os.path.join(PROJECTS_ROOT, PROJECT_NAME, "session-cards")`. INDEX_FILE common.py:42 "index.md". Local cards: common.local_cards_dir (common.py:263-269) = DATA_ROOT\local-cards.
- Copy: SC\writer-brief.md:25 "Data folder, called DATA below: `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\session-cards\`" (full path of the same folder; K1). writer-brief.md also says the main session's check moves a passing card "into the repo's `docs\sessions\` folder, or into `DATA\local-cards\`" (matches common.CARDS_ROOT and local_cards_dir; card writers skip the preamble so this is their only copy).
- On disk today the folder DATA\cards still exists (ls shows batches, cards, condensed, index.md, local-cards, ...), so the "one-time" migration source is still present; Hydra CLAUDE.md says the cards "live in the repo at `docs/sessions/`".
- Other readers: ruling_rules.py:215 builds its card search list from `cards.CARDS_ROOT, cards.local_cards_dir(cards.DATA_ROOT)` (derived from common, good); check.py:225,252,282 use local_cards_dir.
Table: migrate.py never writes to the old folder; check.py moves passing cards out of DATA into docs\sessions; the old DATA\cards folder is read by no other script (searched SC\*.py, gates\*.py, gates\*.ps1).
Kinds: K1 (DATA path in writer-brief.md); K7 weak (the "never overwritten / left alone" rule lives only in migrate.py and its docstring).
Splits: none. Test: SC\test_session_cards.py not read in depth for migrate cases.

## Side notes
- Hydra UT:195 "snapshot_library is the only code that knows the library folder's path" is contradicted by tools\compare_db.py:51,87 (see I6-205).
- gates\README.md:71 says `check_pointers.py` "always exits 0"; by hand it can exit 1 (CP:14).
- tools\check_pointers.py skips the memory archive and rulings quotes by design; it does not read SC\writer-brief.md, tools\ comments or gates README, so full-path copies there are unchecked.
- Hook gap: none noticed.

## Rows not reached
None. Depth limits: I did not run any test; I did not read test_session_cards.py or test_check_pointers (none found) in depth; `docs\development.md` and the plans were not read; archive memory notes not searched.
