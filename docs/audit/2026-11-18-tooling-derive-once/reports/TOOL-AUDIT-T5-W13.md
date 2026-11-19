# TOOL-AUDIT-T5-W13 second-round verification (ledger L09 rows)

All claims are by reading. Nothing was run. H = C:\Users\Patrick\Downloads\Hydra\hydra-test, G = C:\Users\Patrick\.claude\gates, M = memory folder, DOCS = H\docs\agents.
Ruling search: I searched DOCS\rulings\*.md by keyword (premise, handoff, screenshot, release tag, 500 KB, Reset to defaults, issue tracker, early fill, comments). Only code-and-engine.md:22, :23 and :56 bear on rows below. A keyword search can miss a ruling.

### L-F25-04
**Verdict:** CONFIRMED-CHANGED. The row said "None" for splits. There is one.
**Homes as found:** DOCS\premise-map.md:52 "A file that only reports on it does not count. That means a handoff, another plan, a session card, a memory note, or a Clone Hero block with no proof label." Same file :45 and :27 repeat the "reports on it" idea. H CLAUDE.md rule 15 "A rule copied from a tool, test or comment is never a source." DOCS\agent-rules.md:39 "A reading, a memory note, a handoff or a plan's "as ruled" never counts as the user's words." (about the user's words, a third list). C agents\planner.md:13 sends the reader to the page ("read it there, because this page does not repeat it"). reviewer.md:13 says to open each checked row's source.
**Splits:**
1. premise-map.md:52 names handoff, plan, card, memory note, unlabelled CH block, and does not name a comment, a test or a tool. Rule 15 names tool, test, comment, and does not name handoff, plan, card, memory note. They differ for a premise whose only source is a code comment or a test: the page counts a file read directly as `code` (premise-map.md:39), rule 15 refuses a comment in it.
2. agent-rules.md:39 lists four things not counting as the user's words; premise-map.md:45 lists three for `ruling`. Differences: the page lets a card or brief quote count, line 39 does not (same as L-F25-01).
**Ruling:** None found naming either side.
**Open:** whether a comment counts as "a file read directly" has no test; no run settles it.

### L-F25-05
**Verdict:** CONFIRMED (agree today; nothing checks it).
**Homes as found:** DOCS\this-machine.md:17 "A release's tag and title are both `v` plus the version ... VERSION 3.1.2 today." H CMakeLists.txt:21 `project(Hydra VERSION 3.1.2 ...)`. H docs\development.md:1011-1017 shows `-Target v<version> -Title v<version>`. G publish.ps1:49-50,57-59 pass -Target and -Title to gh as given; no check of their shape. H src\app\update_release.cpp:17-18 only the suffixes `-setup.exe`, `-setup.exe.sig`.
**Splits:** None by value. The "v plus version" rule lives in prose on two pages (this-machine.md, development.md); the version number is copied into this-machine.md ("3.1.2 today") and will drift when CMakeLists.txt changes. They differ at the next version bump.
**Ruling:** None found.
**Open:** none.

### L-F25-13
**Verdict:** CONFIRMED-CHANGED. Row said splits None. One date appears twice, plus a wording difference.
**Homes as found:** G ruling_rules.py:95-100 `CONFIRMED, UNCONFIRMED = "## Confirmed", "## Unconfirmed"`, `INSTALL_DATE = "2026-11-14"`, `APPROVE, DECLINE = "File it", "Do not file"`; :105 SESSION_NOTE. DOCS\rulings\README.md:30-31 shapes, :41 "`File it` and `Do not file`", :51 "`INSTALL_DATE` in `ruling_rules.py`, 2026-11-14" (the page names the code constant and copies its date), :51 also "approved it on 2026-11-13". Headings in README: `## How to file a ruling: the approval step` (README.md:36).
**Splits:**
1. INSTALL_DATE value 2026-11-14 is typed in README.md:51 and in ruling_rules.py:99. They differ when the constant changes.
2. README.md:43 says git refuses "any added line that is not in the right shape"; ruling_rules.py:8-9 and :373 let blank lines, headings, and a new file's one title line through. They differ for a heading edit or a title line (same as L-F27-11).
**Ruling:** None found.
**Open:** The labels and heading words are pinned by G tests\test_docs_numbers.py (not reopened).

### L-F25-14
**Verdict:** CONFIRMED (no split found; the live-GitHub half is open).
**Homes as found:** DOCS\issue-tracker.md:28-40 wayfinding: "Create it with `publish.ps1 -Kind issue`, then add the label with `gh issue edit <n> --add-label wayfinder:map`"; "Sessions cannot ... because `gh api` is denied." C settings.json:25 `Bash(gh api *)` in the deny block and G settings.fragment.json:15 the same (both lines read). DOCS\triage-labels.md:5-11 table, each label string equal to its name; H CLAUDE.md says "each label string equal to its name".
**Splits:** None by reading. `gh issue edit` and `gh issue create` are not in the deny lines I saw (create is, `edit` is not); that is consistent with the page, which sends create through publish.ps1 and uses `gh issue edit` raw.
**Ruling:** None found.
**Open:** whether the repo's real labels exist needs a `gh label list` run. The /wayfinder and /triage skills were not located.

### L-F25-15
**Verdict:** CONFIRMED (words differ, rule agrees).
**Homes as found:** DOCS\doc-screenshots.md:15 "a `rect` line directly before a `screenshot` line that has no ref; the script then keeps only the part of the frame below that rectangle." H tools\doc_shots\shots.txt:7-9 "A rect line directly before a full-frame screenshot also tells doc_shots.ps1 to keep only the frame below that rectangle." H tools\doc_shots.ps1:23-38 "A rect line directly before a screenshot line with no bar"; the code acts when `$commands[$i-1] -match '^rect\s'` and the line matches `^screenshot\s+(\S+)\s*$` (exactly one argument). Reset words: doc-screenshots.md:25 "click `Reset to defaults`, then check ... `Analysis settings: defaults`"; shots.txt has `click Reset to defaults`, `expect-text Analysis settings: defaults`.
**Splits:** (1) "no ref" (page) / "full-frame" (shots.txt) / "no bar" (script) / the regex (one argument). They differ for `screenshot file |` with an empty ref: page says "has no ref" (yes), code regex requires nothing after the file name, so a trailing ` |` is not matched. Edge case; no real shot uses it (I read shots.txt lines 1-25 only). (2) The two labels `Reset to defaults` and `Analysis settings: defaults` are typed in shots.txt and the page; their owner is the UI source (settings_bar.cpp / library_dialogs.cpp, not opened here).
**Ruling:** None found.
**Open:** a doc_shots run would settle the trailing-bar case.

### L-F25-16
**Verdict:** NOT REACHED in full (I did not open G review_rules.py:383-386 or ui-testing.md:241-249). Left open.
**Open:** open both; compare the page's seven fields with the gate's three conditions.

### L-F25-17
**Verdict:** CONFIRMED-CHANGED. Status words agree; a different split exists.
**Homes as found:** H tools\check_handoff.py:28-33 `_FULL_HASH`, `CLAIMS_HEADING = "## Claims"`, `MANUAL_KINDS = ("test","ruling","state")`, `TABLE_STATUSES = ("ok","missing","not-on-main","not-pushed","manual")`. DOCS\handoff.md:7 path `docs/handoffs/<YYYY-MM-DD>-<slug>-handoff.md`; :33-37 "A row whose check fails is wrong until someone fixes the handoff." and "Write every hash in full, all 40 characters"; the scratch-path sentence "(The docs test's scratch-path check skips `docs/handoffs/`, so this rule holds by instruction there ...)". H .claude\commands\handoff.md:11-13 (slug, status words, "Fix every row it marks `missing`"). H tests\test_docs_match_code.cpp:1233 `kNotPermanentDirs[] = {"docs/handoffs/", "docs/superpowers/"}`.
**Splits:**
1. The page says the scratch-path check skips `docs/handoffs/`; the test skips `docs/handoffs/` and `docs/superpowers/`. They differ for a scratch path written in a plan.
2. `_FULL_HASH` accepts upper-case 40-hex (`[0-9a-fA-F]`), the page says only "40 characters" (same as L-F27-05).
3. The command says a kebab-case slug of 2 to 6 words; the page names no word count. Not checked by the checker (it reads the claims table, not the file name; not opened for that).
**Ruling:** None found.
**Open:** none.

### L-F25-18
**Verdict:** CONFIRMED (K7 hold).
**Homes as found:** DOCS\clone-hero-mcp.md:52-53 refers to the Edge Walk probe chart folder. H tools\ch_probe\probe_songs.py:64 `DEFAULT_OUT = r"C:\Clone Hero\songs\Hydra Probe"`, :66 `EDGE_WALK = "Edge Walk"`, :68 `NAME_PREFIX = "Hydra Probe - "`, :300 `return NAME_PREFIX + name`. Test tools\ch_probe\tests\test_probe_songs.py:97 pins `P.EDGE_WALK == "Edge Walk"`. File names: game_tools.py:49 `PRESS_LOG_PREFIX = "press-"`, :52 `BUILDS_PREFIX = "builds-"`; copied as `press-<n>.csv`, `builds-<n>.json` in game_tools.py:1150-1151, server.py:423-425, :458 and the doc.
**Splits:** (1) The code names the folder as `NAME_PREFIX + name` = "Hydra Probe - Edge Walk" (name plus the " - " prefix) under DEFAULT_OUT; the row says the doc path ends in "Hydra Probe\Edge Walk Pair". The words "Edge Walk Pair" occur nowhere in probe_songs.py (grep of tools for "Edge Walk" found "Edge Walk" only). They differ for any caller who types the doc path: the folder it names is not the one the code writes. I did not read the doc line holding "Edge Walk Pair" (the grep for "Edge Walk" in the doc matched only lines 52-53 in my output with no "Pair" shown); open that line to be sure. (2) the file-name prefixes are typed in docstrings and doc text, pinned by no test I found.
**Ruling:** None found.
**Open:** exact doc line for "Edge Walk Pair".

### L-F25-19
**Verdict:** CONFIRMED-CHANGED. The copy list is real; the owner is not a build-number owner.
**Homes as found:** H tools\ch_mcp\build_facts.py:69-81 `current_build` returns the game's `version` string as written in its version file (full version such as "1.1.0.6142-final" in form, not the bare number). H tools\ch_mcp\dump_facts.py:381 `DEFAULT_BUILD = "6142"`; :27 `--version 1.1.0.6142-final`. H tests\test_docs_match_code.cpp:719 `kBuild11 = "6142"`. DOCS\this-machine.md:27 "build 6142"; H docs\clone-hero\README.md:4,70,88; docs\clone-hero\code\README.md:6,12; tools\ch_probe\constants.py:22,239; SPEC.md:76,299; tools\ch_mcp\README.md:12,20 (path `ch-dumps\6142`, `--version 1.1.0.6142-final`).
**Splits:** (1) `current_build` answers "what version is installed"; DEFAULT_BUILD and kBuild11 answer "which build do the facts pages describe". The two differ the day the game updates: the installed version changes, the typed 6142 does not. (2) Number form differs: "6142" vs "1.1.0.6142" vs "1.1.0.6142-final".
**Ruling:** None found.
**Open:** whether DEFAULT_BUILD is compared with current_build anywhere (not searched).

### L-F25-20
**Verdict:** CONFIRMED.
**Homes as found:** DOCS\doc-screenshots.md:13 "A missing element stops the run with `!! failed at line N`". H tests\ui\uitest_commands.cpp:366 `return refuse(message("\"%s\" is not on screen", ...))`, :385 `refuse(message("rect found no item ..."))`, :432 `"!! failed at line %d: "`. DOCS\ui-testing.md:97 "A failed engine action, like a click that finds nothing, prints `!! failed at line N: <line>`. A line the interpreter itself refuses (... an `expect-text` that does not hold, a `rect` that finds nothing, a wait that ran out) fails the test, and its log names the line as `line N: <reason>`." tests\ui\uitest_commands.h:19 `double timeout = 30.0`; shots.txt `timeout 120`.
**Splits:** doc-screenshots.md:13 says an expect-text or rect miss prints `!! failed at line N`; the code and ui-testing.md:97 say `line N: <reason>` for those two, `!! failed at line N` for engine actions. They differ for an expect-text or rect failure.
**Ruling:** None found.
**Open:** none; both messages stop the run.

### L-F25-21
**Verdict:** CONFIRMED.
**Homes as found:** H tools\hydra_ui_mcp.py:570-601: "Hydra closed its live channel but kept running for {reply_margin_s:g} s, so the helper killed it", "Hydra's window closed and Hydra exited with code 0: the user closed it, or a quit command was sent", "Hydra exited with code {describe_exit(code)}", and the scratch-folder comment "removes it once it has gone a week ... $ScratchAgeDays in cleanup_sweep.ps1 owns that age". G cleanup_sweep.ps1:54 `$ScratchAgeDays = 7`. DOCS\ui-testing.md:282 "Hydra exited with code X ... Hydra did not answer within N s ... removes it after a week".
**Splits:** (1) ui-testing.md:282 states two messages; the code has four (window closed with code 0; channel closed but still running). They differ for a clean window close and for a channel close with the process still alive. (2) "a week" is copied in the page (:282), and the code comment names `$ScratchAgeDays = 7` (a word, not a number, in the code). They differ if the 7 changes.
**Ruling:** None found.
**Open:** none.

### L-F25-22
**Verdict:** CONFIRMED-CHANGED. The refill_tick "split" is a time-stamped example, and its status is stated.
**Homes as found:** DOCS\test-rubric.md:111-113 "This record was written before the fix it asked for. The function and lines it names are read as they stood at commit 003d76a3"; :119-126 name `Activation::refill_tick (src/core/model.cpp:762-768)` and "Only the Preview calls it". DOCS\test-rubric-examples.md:9 "The Preview's own refill-tick function is gone". grep of src and tests for `refill_tick` found no file.
**Splits:** None between the two pages: each says the function is read as of 003d76a3 / gone. What remains: a reader of test-rubric.md:119-126 learns the function exists unless they read the :111-113 caveat. They differ for a reader who skips the caveat. Other findings (answers 1-11 vs question 12; "two lines" at :47 vs a third "They differ." line at :130; Pitch Dark values 128520 and 128400 typed at :15,48-49,130 and in examples) were not opened beyond :47-52 and :128-132, which do show the sample with three lines.
**Ruling:** None found.
**Open:** none for the first point.

### L-F25-23
**Verdict:** CONFIRMED.
**Homes as found:** DOCS\this-machine.md:85 "a result row's `song_id` is the chart's `md5` (the hyhash, the chart's id; `docs/adr/0028-a-chart-id-is-clone-heros-song-id.md`)". H docs\adr\0028-a-chart-id-is-clone-heros-song-id.md: folder chart id = MD5 of its notes file; `.sng` id = MD5 of the inner notes file; `.srb` id = the 16-byte checksum its metadata stores, "Hydra reads it and never recomputes it"; "`song_id_read` in src/app/analysis.cpp is the one owner". src\app\analysis.cpp:62-63 comment "which bytes make a chart's id is song_id_read's rule", :172.
**Splits:** this-machine.md:85 says the id is "the chart's `md5`" (a hash of the chart); the ADR says an `.srb` id is a stored checksum and a `.sng` id is a hash of the inner file. They differ for a `.srb` song, where the id is read, not an MD5 of the chart's file. The sentence points at the ADR, so the loose wording is covered by the pointer.
**Ruling:** None found. The ADR cites the user's decision of 2026-10-09 (a decision-log line, not a `## Confirmed` line).
**Open:** none.

### L-F25-24
**Verdict:** CONFIRMED.
**Homes as found:** H tests\ui\live_protocol.h:64-92 `reply_line`, `ok_line`, `fail_line`, `hello_line` (fields ok, text, timeout, hello{mode, scratch, pid, timeout, log, crash_dump}). H tools\fake_hydra_live.py:61 "The timeout the hello line and every reply carry. Deliberately not the real exe's starting value, so a test can tell the helper read it from the line." `SESSION_TIMEOUT_S = 25`; :105 and :146 write the same JSON shape in Python. H tools\hydra_ui_mcp.py:42-43 `REPLY_MARGIN_S = 15.0`, `HELLO_TIMEOUT_S = 60.0`. DOCS\ui-testing.md:265-267 hello field list, `testwait::kUitestProcessCap`. H tests\wait_util.h:22 `kUitestProcessCap = 5 minutes`. tests\ui\uitest_commands.h:19 real starting timeout 30.0.
**Splits:** (1) The JSON shape is written by C++ (live_protocol.h) and by Python (fake_hydra_live.py:105,145); pinned on the C++ side only (tests\test_live_protocol.cpp). They differ the day a field is added to one. (2) The fake's 25 s differs from the real 30 s by its own comment's design. (3) ui-testing.md:267 names `kUitestProcessCap` without giving its 5-minute value; the value lives only in wait_util.h.
**Ruling:** None found.
**Open:** whether the fake's field set is checked against the real hello line: not searched in the Python tests.

### L-F26-06
**Verdict:** CONFIRMED (the note's claim holds; there is no conflict).
**Homes as found:** M ch-disco-noflip-rule.md:15 "The Splax video layout (score=596,298,124,34 disc=708,660,56,56 at 1080p) isn't in layouts.json." C skills\analyzing-sp-squeezes\SKILL.md says "A layout that passed once is in `layouts.json`". C:\Users\Patrick\Downloads\Hydra\video-tools\layouts.json holds four layouts; line 3 `"score": [736, 418, 326, 84], "disc": [708, 648, 52, 84]`, line 5 `"score": [596, 456, 280, 68], "disc": [704, 650, 68, 68]`. Neither equals the note's numbers (a grep of the file for 596 and 708 hit only those two lines).
**Splits:** None. The note's numbers describe a layout the file lacks. Its x values partly coincide with two stored layouts (708 with line 3, 596 with line 5), so a reader could confuse them.
**Ruling:** None found.
**Open:** which recording the note's numbers came from (the note's own source is not stated).

### L-F26-12
**Verdict:** CONFIRMED. Same findings as L-F25-15.
**Homes as found:** see L-F25-15 (doc-screenshots.md:15, shots.txt:6-9, doc_shots.ps1:23-38).
**Splits:** wording "no ref" / "full-frame" / "no bar"; code requires exactly one argument. They differ for a screenshot line ending in an empty bar.
**Ruling:** None found.
**Open:** as L-F25-15.

### L-F26-13
**Verdict:** CONFIRMED (two different rules; no split).
**Homes as found:** M comments-explain-current-code.md:6-12 "no old values or version timelines ... history goes in the plan, the decision record, ADRs or CONTEXT.md". DOCS\brief-preamble.md:119 "A comment names where a rule lives; it never restates the rule." DOCS\derive-once-review.md:82 "A comment that restates a rule is kind D." (K-kind D, not in the finder's K list as read; as stated). DOCS\rulings\code-and-engine.md:22 (Confirmed) `**2026-10-15** "comments exist to explain the current code, not give a history lesson" (session c35a4b7d)`.
**Splits:** None. The note and the ruling answer "may a comment tell history" (no); the preamble and review answer "may a comment restate a rule" (no). They differ in the situation where a comment states a current value: the preamble forbids it, the note and ruling allow ("what a constant or rule means").
**Ruling:** The Confirmed line names the history side only; it does not name the restate side, so it does not cover the second split above.
**Open:** none; no gate checks comments (searched G\*.py for "comment": only compiler-comment parsing in review_rules.py and bash_windows_rule.py).

### L-F26-14
**Verdict:** CONFIRMED-CHANGED. The count "nine phrases" is off; the point stands.
**Homes as found:** G tests\test_agent_files.py:15-21 `SHAPE_PAGE = "docs/agents/premise-map.md"`, `PLANNER_PHRASES = ("## Premise map", SHAPE_PAGE, "Calls:", "never tried", "unchecked")` (5), `REVIEWER_PHRASES = ("Premise map", SHAPE_PAGE, "no row", "Calls:", "unchecked", "as settled")` (6): eleven strings, eight distinct. The test checks the words are present and (lines 41, 51) that the map text comes after the Promise map text. C agents\planner.md:13 and reviewer.md:13 contain them. H CLAUDE.md:55 repeats "Every new plan carries a Premise map ..." 
**Splits:** None. The test pins that words appear in the two agent files; it does not compare either file with DOCS\premise-map.md. They differ if the page changes a rule and the agent files keep their phrase.
**Ruling:** None found.
**Open:** none.

### L-F26-15
**Verdict:** CONFIRMED-CHANGED. The row called it a D-number mismatch. The real split is stale content in the memory note, plus an open record.
**Homes as found:** M use-source-facts-not-proxies.md:11 "D69 was then replaced the same day by D75 ... Song length now comes from the chart's own metadata: the song.ini or .sng song_length, or the .srb song_length_ms. The last Expert drum note is only a backup." DOCS\..\audit\2026-10-03-fix-decisions.md:1052 "D255 ... A song's length reads the chart's end event, as Clone Hero does. This amends D75 items 1 and 2." (the song.ini example, Rime of the Ancient Mariner, 6:17 vs 13:41). H src\app\analysis.h:215-216 "the owner's answer (app::song_length_ms, D255)". H tools\ch_mcp\hydra_end.py:33 `song_length_ms(json_path)` reads the field out of a hydra_replay dump. tools\ch_mcp\trace_hits.py:68-69 "the end is D331's". fix-decisions.md:1853 (Q15, user chose "Follow D255 (Recommended)"): player tools read `song.ini`'s first `song_length` line, Hydra keeps the last, so Hydra's end never reached the player (side (a): hydra_replay writes Hydra's end; (b): leave it).
**Splits:**
1. The memory note says the length comes from the chart's metadata (song.ini / .sng / .srb) with the last note as backup; D255 says it reads the chart's end event and amends D75 items 1 and 2. They differ for a chart whose song.ini `song_length` is wrong (the Rime case).
2. trace_hits.py cites D331 for the song end, analysis.h cites D255 for the length: two decision numbers for one owner (`app::song_length_ms`). I did not read D331.
3. song.ini `song_length` first-line (player tools) vs last-line (Hydra), as stated in fix-decisions.md:1853; whether that was fixed since I did not check.
**Ruling:** fix-decisions.md:1853 quotes the user choosing "Follow D255 (Recommended)" with the option text; that is a decision-log entry, not a `## Confirmed` line. Not a ruling file line.
**Open:** whether Q15's side (a) was built (read hydra_end.py callers); D331's text.

### L-F26-16
**Verdict:** CONFIRMED.
**Homes as found:** G common.py:36-48 `log_refusal` (writes time, hook, event, tool, session, agent, decision, reason); G README.md:79 "`common.log_refusal` writes it, and it is the only writer." G tests\test_refusals.py:16 `FIELDS = {"time","hook","event","tool","session","agent","decision","reason"}` (same eight). G common.py:11-12 `REVIEW_KINDS` handover (9 fields), verdict (5 fields); G tests\test_common.py:226-231 a handover dict with those nine and a verdict dict with those five typed by hand. README.md:75 (the one-place claim for review fields) not located by my grep for "review record".
**Splits:** None today. The tests type the field lists again, so they differ when a field is added to the writer.
**Ruling:** None found.
**Open:** README line for review record fields.

## Not reached (not fully checked)
L-F25-16 (G review_rules.py:383-386 and ui-testing.md:241-249 not opened).
Partly checked: L-F25-18 (doc line with "Edge Walk Pair" not read), L-F25-22 (rubric Q-count, Pitch Dark copies not read), L-F26-15 (D331, Q15 build status).
