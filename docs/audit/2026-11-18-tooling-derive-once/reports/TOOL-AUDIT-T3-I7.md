# TOOL-AUDIT-T3-I7 census (scout I7)

Repo read: C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout), commit af9dfac4e246df6b77c4b906f5c7054f47beaa82. All files are in git. Pages are in docs/agents/.

Files read in full: ui-testing.md, clone-hero-mcp.md, test-rubric.md, test-rubric-examples.md, this-machine.md, premise-map.md, owner-map.md, public-doc-style.md, handoff.md, integrate.md, issue-tracker.md, triage-labels.md, domain.md, doc-screenshots.md.

Not read (assumed to be I5/I6's; I was not given those lists and was told to read nothing in the audit folder): agent-rules.md, audit-method.md, brief-preamble.md (read only as my binding preamble, not censused), derive-once-review.md, doc-review.md. If any of these is not on I5 or I6, nobody has it.

Row format: id | question | file:line | value (quoted) | claims to own? | who reads/acts | who is asked to supply.
Abbreviations: UT = ui-testing.md, CH = clone-hero-mcp.md, TR = test-rubric.md, TE = test-rubric-examples.md, TM = this-machine.md, PM = premise-map.md, OM = owner-map.md, PS = public-doc-style.md, HO = handoff.md, IN = integrate.md, IT = issue-tracker.md, TL = triage-labels.md, DM = domain.md, DS = doc-screenshots.md. "main" = main session. "-" = no ownership claim in the text.

## ui-testing.md

I7-001 | Which GUI check does a change need (three cases) | UT:7-11 | "1. Behaviour and text ... 2. What the user sees ... 3. How a report or other analysis-backed view draws" | "One rule covers every GUI check" | main session, GUI-change authors | -
I7-002 | Must a layout/width/spacing/open-close change be seen in the real dev app before merge | UT:10 | "Before merge the change is also seen in the real dev app, with the user's hydra_ui.ini and a copy of their database, and the captures are shown to the user" | - | main | main shows user
I7-003 | If user says merge before real-app check | UT:10 | "say plainly that it is still owed" | - | main | -
I7-004 | Rendering checks: which data to run on | UT:11 | "Run the real app on testdata/input, never a whole-library analysis"; "A copy of the user's database is for the Library view only, never for starting an analysis" | - | main | -
I7-005 | Who builds/runs hydra_uitest | UT:17,27 | "The main session builds and runs these; an agent does neither (agent rule 14)"; "Agents do not run hydra_uitest at all" | - | every agent, main | -
I7-006 | Build commands | UT:20,24 | ".\build_cpp.ps1 -Target hydra_uitest"; ".\build-cpp\Release\hydra_uitest.exe --all" | - | main | -
I7-007 | hydra_uitest command-line options table | UT:31-40 | "--all", "--test <name>", "--jobs <n>", "--list", "--script file.txt", "--live --pipe <name>", "--keep-temp", "--shots <dir>", "--db <file>", "--ini <file>" | points to "kDefaultJobs in tests/ui/uitest_main.cpp", "run_live in tests/ui/uitest_live.cpp decides when it ends" | main | -
I7-008 | Default job count | UT:33 | "kDefaultJobs in tests/ui/uitest_main.cpp" | names owner | main | -
I7-009 | Pass/fail output form and exit code | UT:42 | "[PASS]/[FAIL] per test"; "N of M passed"; "exit code is 0 only when everything passed"; "ctest runs --all too" | - | main, CI | -
I7-010 | One run mode: process per test | UT:44 | "each test its own hydra_uitest --test <name> process, at most --jobs at once"; "A script runs only on its own" | - | main | -
I7-011 | Full-suite duration measured | UT:44 | "about 90 s at the default job count, measured on 2026-10-10 at commit 4c037634 over 1 run" | - | main | -
I7-012 | Number of test charts | UT:46,48 | "The 98 test charts"; "testdata/input, 98 charts" (see also TR B6/B7 rows I7-129, I7-130) | - | test authors | -
I7-013 | How a test holds a running batch | UT:46 | "BatchGate before it starts the batch ... allow(n) ... started() ... BatchGate gate(3)" | - | test authors | -
I7-014 | Test start state | UT:48 | "settings INI (song folder = testdata/input, 98 charts; 'open report automatically' off; search depth 2) and an empty DB"; canned API "one user, alice, id 111"; seams "audio::set_headless, net::set_fetcher, app::set_path_overrides" | - | test authors | -
I7-015 | Command-file verbs and their meanings | UT:71-91 | verbs window, click, check/uncheck, type, open, key, wait, wait-idle, wait-text, expect-text/expect-not-text, text, state, dump, rect, screenshot, timeout, reset, quit, # | points to owners: "open_titled_reason", "key_named and parse_chord in tests/ui/uitest_commands.cpp", "AppState::any_job_running", "CommandSession::timeout in tests/ui/uitest_commands.h" | uitest interpreter, agents using the hydra MCP | -
I7-016 | What counts as a number for timeout/wait | UT:93 | "hydra::parse_finite_number's rule (src/core/strutil.h)"; "timeout 90s, wait abc and a bare wait fail" | names owner | interpreter | -
I7-017 | Where a line is split at its bar | UT:95 | "split_bar in tests/ui/uitest_commands.cpp is the one place a line is split at its bar" | claims one place | interpreter | -
I7-018 | Guard 1: refuse a library-row lookup while search is empty | UT:99 | "refused while the search box is empty"; absolute ref ending in a row's title refused | "item_ref in tests/ui/uitest_commands.cpp owns both" | click/check/uncheck/type verbs | -
I7-019 | Guard 2: retry bare label as **/<label>; refuse when two or more widgets carry it | UT:99 | "tried once more as **/<label> ... reply starts with 'used ref **/<label>'"; "drawn_items_labelled" | owned by item_ref (UT:99) | verbs | -
I7-020 | Guard 3: refused keys while library focused and search empty | UT:99 | "PageDown or End" ... "kPageKeys", "library_key_refusal" in uitest_commands.cpp; "Up and Down ... go through" | names owner | key verb | -
I7-021 | Rule against scrolling the library without the user's yes (no hook reads shell text) | UT:99 | "the rule against scrolling the library without the user's yes is the only thing that binds there" (see also I7-084 for the library rule in CH) | - | every agent | -
I7-022 | Clicking a song starts analysis; wait-idle waits | UT:101 | "Clicking a song starts its analysis (D87 item 1)" | - | test authors | -
I7-023 | Ref syntax | UT:105-111 | bare label; "//Window/Label"; "**/Label"; "##id as written"; "### labels are addressed by the part after ###"; "escape / and # in the title with a backslash" | - | agents, test authors | -
I7-024 | Label cheat-sheet (version bar, toolbar, analysis settings, library, song panel, tabs, batch, report windows, startup, modals) | UT:117-133 | table of ~150 widget labels, e.g. "Scan library", "###analysissettings", "##librarytable", "##DetailsTabs", "###pathreport", "###dmreport" | owners named: "named_version", "version_bar_status", "analyze_shown_label", "settings_button_label (src/ui/library_dialogs.cpp)", "StoreOpenProgress::label() in src/ui/library_jobs.cpp owns the lines" | tests and agents that click by label | -
I7-025 | Delay before a startup screen draws | UT:132 | "kViewProgressDelaySeconds" | names owner | - | -
I7-026 | Analyzing box delay | UT:124 | "Analyzing chart... with Cancel once it has run 0.15 s" | - | screen | -
I7-027 | Ref to a control in the analysis settings popup | UT:122 | "//$FOCUSED/**/<label>" (works through a Hydra engine patch listed in THIRD_PARTY_NOTICES.txt) | - | test authors | -
I7-028 | Report windows: how a C++ test/script finds them; how --open-report works | UT:139-141 | "window_named(\"###pathreport\")"; "Hydra.exe --open-report" opens once, "AppState::open_path_report_once_shown"; close via "WM_CLOSE ... class ImGui Platform, title Path report — Hydra" | - | main, scripts | -
I7-029 | Which tests are in the two report-window groups | UT:145,147 | lists "report-window-sort ... report-windows-both" and "report-window-open-path ... report-window-out-of-date" | - | test authors | -
I7-030 | Path Builder uitest runner and ids | UT:151-165 | "hydra_pathbuilder_uitest"; "##fill_<tick>", "##band_<index>", "##timeline", "##label_saved", "##label_activations", "##scrollbar", "##choice_<tick>", "##end_<tick>"; "kTimelineScrollbarId"; "any_ref in tests/pb_ui/pbtest_harness.h writes the ref" | names owners | main, test authors | -
I7-031 | Attached mode command and log | UT:170-175 | "Hydra.exe --uitest scan"; "hydra_uitest.log"; "--uitest-log <file>"; installer "ship preset" leaves GUI tests out | - | main | -
I7-032 | MCP server registration and code location | UT:181 | ".mcp.json at the repo root"; "tools/hydra_ui_mcp.py" | - | Claude Code, main | -
I7-033 | Python mcp package install | UT:185 | "py -m pip install mcp" | - | main | -
I7-034 | Helper never builds; who makes missing exes | UT:185 | "its reply says to ask the main session, which makes it with build_cpp.ps1" | - | helper, main | -
I7-035 | Headless vs window mode | UT:189-191 | "hydra_start('headless')" = hydra_uitest.exe --live on testdata/input; "window" = real Hydra.exe --live on snapshot copy | - | main | -
I7-036 | Library folder for the snapshot | UT:193 | "library_dir if given, else the HYDRA_LIBRARY_DIR variable, else DEFAULT_LIBRARY_DIR in the helper"; files "hydra.db, hydra_settings.ini, hydra_ui.ini" | "snapshot_library is the only code that knows the library folder's path" | helper | -
I7-037 | How hydra.db is copied | UT:195 | "SQLite's online backup ... whenever a hydra.db-wal sits beside it"; else plain copy; "copy_database in the helper says why" | - | helper | -
I7-038 | Three guards keeping the real hydra.db out of window mode | UT:197-201 | "names only its snapshot as --db, --ini, --ui-ini"; "reset_app"; "testtemp::is_under_temp in tests/temp_util.h"; refusal text "refused: a live seed must be a copy under the temp folder" | - | helper, Hydra live test | -
I7-039 | Batch in window mode obeys library-run rule | UT:203 | "the library-run rule in docs/agents/brief-preamble.md applies: narrow the library with a search first" (second copy of the brief-preamble rule; compare I5/I6) | points to brief-preamble | main | -
I7-040 | Window close/stop behaviour | UT:203 | "hydra_stop(keep_window=True)"; "If the user closes the window, Hydra exits and the next tool reply says the session is stopped" | - | helper | -
I7-041 | Where live code exists | UT:205 | "HYDRA_UITEST_ATTACHED, which the ship preset leaves out" | - | build | -
I7-042 | Tool list of hydra MCP | UT:209-229 | 16 tools: hydra_start(mode, library_dir=None, keep=False), hydra_stop(keep_window=False), hydra_reset, hydra_open_song(title), hydra_click, hydra_type, hydra_check, hydra_key, hydra_window, hydra_wait_idle(timeout=None), hydra_wait_text, hydra_text(contains=None), hydra_state, hydra_dump, hydra_rect, hydra_screenshot(name), hydra_run(script) | "The tool list in tools/hydra_ui_mcp.py is the owner; this table is a copy of it." (explicit copy) | main | -
I7-043 | Helper scratch folder name and cleanup | UT:213,282 | "hydra_live_<tag> in the claude folder of the Windows temp folder"; "The SessionStart cleanup sweep removes a kept folder after a week with no writes" | - | helper, SessionStart hook | -
I7-044 | hydra_run stop message form | UT:229 | "stopped at line N of M" | - | helper | -
I7-045 | Real-app record: purpose and file | UT:235-237 | "realapp.jsonl in the repo's git common dir"; one JSON line per window session; "REALAPP_FILE" in tools/hydra_ui_mcp.py names it | "The field names below are that gate's contract" | helper writes; a merge gate for src/ui changes reads | -
I7-046 | Real-app record fields | UT:241-249 | seven fields: kind "realapp", commit, dirty, commands, shots, started, stopped | "realapp_record in tools/hydra_ui_mcp.py writes the line" | helper; the src/ui merge gate | -
I7-047 | Which endings write a record | UT:251 | exit 0 via hydra_stop, hydra_stop(keep_window=True), new hydra_start, user closing window, helper's own exit; crash/nonzero/killed/won't quit write nothing; text "no real-app record written" | - | helper | -
I7-048 | Open-dialog note | UT:259 | "open_modal_note in tests/ui/uitest_engine.cpp writes" the one line | names owner | replies | -
I7-049 | Pipe protocol: hello line, request/reply JSON | UT:263-276 | hello `{"ok":true,"hello":{...}}` with mode, scratch, pid, timeout, log, crash_dump; reply `{"ok":..,"text":"..","timeout":..}`; "--pipe <name>" | "tests/ui/live_protocol.h writes these lines, and tests/test_live_protocol.cpp pins them" | helper, Hydra | -
I7-050 | Session-start wait cap | UT:267 | "testwait::kUitestProcessCap (tests/wait_util.h)"; "start_live_session in tests/ui/uitest_live.cpp" | names owner | helper | -
I7-051 | Dev hold-open env var | UT:269 | "HYDRA_UITEST_HOLD_OPEN_S"; "OpenHold::StartupDrawn in tests/ui/uitest_harness.h" | - | developers | -
I7-052 | Reply wait caps | UT:280 | "REPLY_MARGIN_S", "HELLO_TIMEOUT_S" ("All its numbers live at the top of tools/hydra_ui_mcp.py") | claims one home for helper numbers | helper | -
I7-053 | Dead/silent Hydra wording | UT:282 | "Hydra exited with code X"; "Hydra did not answer within N s" (helper kills it) | - | helper | -
I7-054 | Reply length cap | UT:284 | "REPLY_CAP_CHARS" | names owner | helper | -
I7-055 | Who owns what: C++ owns every command rule; helper only carries messages | UT:288-290 | "Hydra's C++ side owns every command rule ... a question about a command is answered in C++, never in Python"; "docs/adr/0030-hydra-owns-every-live-command-rule.md" | explicit owner claim | helper, test authors | -
I7-056 | Where a test goes and how it is registered | UT:294-296 | four area files with entry tables; "register_paths_tests", "register_report_window_tests", "report_window_flow_tests()", "register_tests in tests/ui/uitest_tests.cpp" | - | test authors | -
I7-057 | Test-writing rules of thumb | UT:317-327 | "Wait on app state ... never on frame counts"; BatchGate, ViewGate, OpenGate; "reset_app(h) blocks until that open ends"; "wait_until yields one extra frame"; screenshot name "startup-library-read" | - | test authors | -
I7-058 | Real-app set-up steps | UT:346-352 | exe_dir in src/app/config.cpp; "-Target Hydra"; copy "Hydra.exe, the two mimalloc DLLs and the assets and resource folders from build-cpp\Release"; "C:\Program Files\Hydra"; "tools/capture_window.ps1" | names owners | main | -
I7-059 | Live window takes no real mouse | UT:354 | user quote "my mouse doesnt work in it" | - | main | -
I7-060 | Traps: IM_CHECK_FLOAT_EQ_EPS; engine patches; stale excluded exes; capture_window type name; server runs its own checkout's build; helper not reloaded; Escape abort; no accessibility tree | UT:360-367 | "IM_CHECK_FLOAT_NEAR_EQ for a tolerance"; "// Hydra patch:"; "THIRD_PARTY_NOTICES.txt lists them"; "engine-wildcard-patches test"; "HydraCaptureV1"; user quote "you created the server, you figure it out"; "clear_abort", "engine_stopping" | "Each trap names the file that owns the behaviour" | main, test authors | -
I7-061 | Hydra settings/ini location | UT:346 | "hydra.db and hydra_settings.ini from the folder its exe sits in (exe_dir ...). hydra_ui.ini same folder unless --ui-ini" (cf. TM database location, I7-157) | names owner | main | -

## clone-hero-mcp.md

I7-062 | Which process runs the ch server; plans it came from | CH:3 | "tools/ch_mcp/front.py" runs "tools/ch_mcp/server.py"; five removed plans each with "git show <hash>:<path>" | - | Claude Code, main | -
I7-063 | When a live run may start | CH:7 | "The user says in chat when a run may start"; user must be asked | - | main | user says go
I7-064 | Agents never run hydra_replay (main makes the file) | CH:7 | "An agent never runs hydra_replay (agent rule 14), so the main session makes that file first" | - | agents, main | -
I7-065 | Where the list of ch tools lives | CH:11 | "The list of tools lives in one place: build_server() in tools/ch_mcp/server.py"; front adds "ch_reload" and "ch_call" from "FRONT_TOOLS in tools/ch_mcp/front.py" | claims one place | main | -
I7-066 | What the server backs up | CH:15 | "the registry key below ... backup in tools/ch_mcp/game_files.py owns it"; no game folder file backed up/restored | "owns it" | ch_start, ch_stop | -
I7-067 | Game files the server leaves alone | CH:15 | "C:\Users\Patrick\Documents\Clone Hero\scorestats.json, profiles.ini, profiles.bin, and scoredata.bin, scoresext.bin and user.bin in the LocalLow folder" | - | server | -
I7-068 | User ruling quoted on game save file | CH:15 | "stop messing with the game's save file. just let it stay." (2026-11-13) and "Leave the line changed" | - | server design | -
I7-069 | Machine-quiet check | CH:15 | "about 6 s"; "tools/ch_mcp/quiet.py owns the check"; a not-quiet machine refuses the launch before any backup | "owns" | ch_start | -
I7-070 | Machine recorder lifecycle | CH:17 | "tools/ch_mcp/machine_recorder.py"; "RECORDER_EXIT_CAP_S"; "tools/ch_mcp/recorder_link.py owns that"; ch_reload refuses while it runs | "owns" | ch_start, ch_stop, ch_reload | -
I7-071 | Rewired registry key backup and restore | CH:19 | "HKCU\Software\srylain Inc.\Clone Hero"; "rewired.reg"; replies "rewired registry: unchanged" / "rewired registry: restored" | - | ch_start, ch_stop | -
I7-072 | settings.ini line changed for the run | CH:21 | "pause_on_focus_lost = 0"; reply "settings.ini: pause_on_focus_lost set to 0 for this run (not put back after the run)"; user chose "Leave the line changed" 2026-11-13 | - | ch_start | -
I7-073 | Never bring game window to front | CH:23 | game launched minimized/not active; spawn form can't; "tools/ch_mcp/launch_patch.json" key "launch" set to "suspended"; "_real_spawn in tools/ch_mcp/bot_run.py"; foreground count is "information, not a pass condition"; user quote "why does it matter if the game is focused briefly?" | - | ch_status, ch_kit_status, ch_stop | -
I7-074 | Keyboard/mouse off during run | CH:25 | "Rewired's keyboard device and its mouse device off"; both devices must read back off else setup refuses | "ch_kit_setup in tools/ch_mcp/kit_tools.py and setup in tools/ch_mcp/agent/kit.ts own how the switch is made and checked" | ch_kit_setup | -
I7-075 | Kit's map and bindings | CH:27 | "KIT_MAP_CATEGORY_ID", "KIT_MAP_LAYOUT_ID", "KIT_BINDINGS" in tools/ch_mcp/kit_tools.py; "map_problems"; fault reporting "settled", "faultOf" in kit.ts, "fault_lines" | names owners | ch_kit_setup | -
I7-076 | Key route lock | CH:29 | "The allow key run hook in C:\Users\Patrick\.claude\gates\percall.py refuses each such call unless the user has typed exactly 'allow key run' in chat in this session, and each such message lets one call through" | - | hook percall.py; user types it | user types phrase
I7-077 | Play stays offline: one-byte patch | CH:31 | "bot_run.launch_command"; "tools/ch_mcp/launch_patch.json"; allowed 2026-10-31; "IN-61 on docs/clone-hero/07-internals.md" | - | launch agent | -
I7-078 | Library charts a live run may play (allow list by plan) | CH:31 | "the test chart" only; player-refinements: "Bomber and Mouths Like Sidewinder Missiles"; player-limits: "Rescued, Ghost Of Indecision, Get Ur Freak On and Yandere Complex"; also "Pitch Dark by CHON ... as a human player (2026-11-12 'Human run, allowed')" | - | live-run planners/main | -
I7-079 | ch_open_song Online-menu rule; menus never opened | CH:31 | "No tool opens the Online menu, Settings, Practice or the control mapper" | - | ch_open_song | -
I7-080 | Allowed game-method calls per agent | CH:33 | "tools/ch_mcp/agent/menu.ts" list; "tools/ch_mcp/agent/kit.ts" list "must equal the names in tools/ch_mcp/kit_methods.json"; test "tools/ch_mcp/tests/test_frida_attach.py" | each agent holds its own list | Frida agents, test | -
I7-081 | Launch agent write conditions | CH:33,78 | "applyPatch in tools/ch_mcp/agent/launch_core.ts owns that decision"; writes only if first bytes and patch site match launch_patch.json; "defer" flag | "owns" | launch agent | -
I7-082 | Method-watch skip rule | CH:33 | "shared_rvas in tools/ch_mcp/method_watch.py decides which" | decides | state agent | -
I7-083 | Online-check refusal and step file rules | CH:35 | "online_refusal in tools/ch_mcp/menu_tools.py says which reads count as closed"; "Every item in it is a kit: item, and item_problem ... refuses a key name, a lane or typed text" | names owners | ch_open_song | -
I7-084 | Every wait has a cap; no auto-retry; nothing runs over the song library | CH:37,39 | "Every wait has a cap and says so in its reply"; "No tool retries on its own"; "Each tool takes one song folder or one chart folder" (cf. I7-004 and brief-preamble library rule) | - | every ch tool | -
I7-085 | Live-run plan must name pass/fail lines, run cap, end with ch_stop | CH:43 | "Each live run names its pass and fail lines before it starts, has a cap on how many times it runs, and ends with ch_stop's restore lines" | - | live-run planners | -
I7-086 | ch_start direct route and its caps | CH:45 | "ch_start(song_folder, human=True)"; "bot_run.launch_human owns the order and the three caps"; "bot_run.DEFAULT_PLAYER"; "player=\"ProDrums,Expert\""; "npm run build:launch"; "tools/ch_mcp/build_stamp.py owns that check" | "owns" | ch_start | -
I7-087 | Kit player | CH:47 | "player=\"System\"" for a human launch; "KIT_PLAYER stays Player1"; user ruling 2026-11-06 "Keep Player1"; direct-start plan question 7 | names owner KIT_PLAYER | ch_kit_setup | -
I7-088 | SP watch thresholds and half | CH:48 | "gaps over 50 ms between samples"; "sp_half_u64 ... engine +0x1B8"; "sp_tools.ready_lines owns the line"; ch_sp_compare "needs resolution ... refuses without it"; "a bot run cannot answer a Star Power question" | "owns" | SP tools | agent supplies resolution
I7-089 | Auto-player timing rules | CH:49 | "The player works out no window, lead or Star Power end itself"; file gives "conversion", "retimed=inside|plain|late"; "kept_gems"; "declined"; "path_index (default 0)"; "path_index_refusal in tools/ch_mcp/game_tools.py owns that check" | "owns"; schedule owns the numbers (hydra_replay schedule) | ch_play_chart | -
I7-090 | Kit route vs keys route timing; removed timing argument | CH:51 | "Neither ch_play_chart nor ch_run_chart takes a timing argument ... the server drops the argument without a word"; test "test_a_leftover_timing_argument_is_dropped_without_a_word"; "FRAME_PROBE_SECONDS"; "D341" | - | server | -
I7-091 | ch_run_chart contract | CH:52 | "ch_run_chart(song_folder, score_json, schedule_json, kit_player=\"System\")"; "ENGINE_SEARCH_CAP_S"; builds-<n>.json, press-<n>.csv; "PRESS_LOG_COLUMNS in tools/ch_probe/experiments/overlay_logs.py" (one home); "schedule_chart_folder" | "have one home" | ch_run_chart | -
I7-092 | Edge-probe contract | CH:53 | "probes JSON list of {\"index\":..,\"lead_ms\":..}"; positive lead early, negative late; chart "C:\Clone Hero\songs\Hydra Probe\Edge Walk Pair"; "tools/ch_probe/experiments/edge_probe.py owns how a probe chord is moved" | "owns" | ch_edge_probe, ch_run_chart | -
I7-093 | Menu walk steps | CH:54 | "tools/ch_mcp/menu_steps.json" marked unverified; "player_screen_refusal", "_end_refusal" in menu_tools.py | names owners | ch_open_song | -
I7-094 | Trace cap | CH:55 | "trace_hits.TRACE_CAP_MARGIN_S (D331)"; "prep_inputs.REPLAY_CAP_S" | names owners | ch_trace_start | -
I7-095 | Go-request rule before asking the user for a run | CH:58 | "the main session's message shows that step's Calls: line exactly as the plan holds it ... names each (never tried) call ... each unchecked Premise map row"; "No hook checks this message" | "docs/agents/premise-map.md owns the shape" | main | user answers go
I7-096 | Stats file handling | CH:60 | "C:\Users\Patrick\Documents\Clone Hero\scorestats.json"; "wait_stats in tools/ch_mcp/bot_run.py" | - | ch_wait_stats | main copies for menu runs
I7-097 | ch_compare settings | CH:62 | "checksum, playback_speed, no_fail and modifiers. Give all four or none"; "ch_compare in tools/ch_mcp/game_tools.py and compare_scorestats.compare own what is checked" | "own" | ch_compare | caller supplies the four
I7-098 | ch_reload rules | CH:64 | refuses while anything open; changes to "tools/ch_mcp/front.py" or the `instructions` string need server reconnect | - | main | -
I7-099 | ch_call rules | CH:66 | refuses unknown names and "ch_call"/"ch_reload" as inner tool; judged as the inner call by the key lock; "Claude Code 2.1.317 ... protocol 2026-07-28"; plan step LS-R3a dropped 2026-11-07 | - | server, percall.py | -
I7-100 | Engine search cap | CH:68 | "engine_finder.ENGINE_SEARCH_CAP_S" | names owner | watcher, player | -
I7-101 | Menu walk/method watch kept; start_candidates | CH:70 | "ch_open_song, menu_steps.json, the method watch and start_candidates.json stay on disk and registered" (user choice 2026-10-31, question 4); "songClock in tools/ch_mcp/agent/menuhooks.ts" | - | server | -
I7-102 | Destroyed GameManager skip | CH:72-74 | "m_CachedPtr"; "tools/ch_mcp/agent/gamemanager.ts"; state word "menu (no live GameManager)"; "IN-36", "IN-03" | - | state agent | -
I7-103 | Game version check | CH:78 | "reads only the version string in the game's own version file" | - | ch_attach, ch_trace_start, ch_method_watch_start | -

## test-rubric.md

I7-104 | Who asks the test questions | TR:5 | "A reviewer asks them of each new or changed test (derive-once-review.md, step 5). A test audit asks them of every existing test" (derive-once-review holds the same step: see I5/I6) | - | reviewers, auditors | -
I7-105 | Two kinds of acceptable number | TR:21-25 | "rule number ... fine"; "snapshot number ... never fine"; "a raw result ... fails too"; examples 85, 7285, 10, 9 | - | reviewers, auditors | -
I7-106 | Answer form | TR:29 | "Answer each one in writing, in a sentence or two. Name files, lines and functions." | - | auditors | -
I7-107 | Question 1 to 12 (list) | TR:33-94 | twelve numbered questions: claim; owner; real chart; direct check; traced number; display vs engine; failure; deleted line; stand-in; same rule elsewhere; scenario real; what was read | - | auditors, reviewers | -
I7-108 | Where the engine's record is read | TR:45 | "hydra_replay.exe dump then hydra_replay.exe schedule print ... sp_end_steps and squeezes ... paid_by and ends_sp"; auditors "ask the main session for the output and name the chart" | - | auditors, main | auditor asks main
I7-109 | Required output shape for Q3 | TR:47-51 | two lines "Engine: ..." and "Claim: ..." | - | auditors | -
I7-110 | Hard rule when claim differs from the record | TR:55-63 | Q2 and Q3 fail; verdict "never Keep" and never a Rewrite that keeps the claim; "Investigate when a decision or ruling backs the claim's side"; "Rewrite when nothing backs it" | - | auditors | -
I7-111 | Decision record never answers Q3 | TR:65 | "When a decision and the engine's record disagree, say so and send both to the user" (D9) | - | auditors | user gets both sides
I7-112 | What passes as a direct check | TR:67-74 | "A rule number ... with the page and rule id", "A property", "The owner's answer", "The rule applied to the chart's named inputs"; fails: "a snapshot number, a floor with no reason, and a raw result" | - | auditors | -
I7-113 | What gives a number a reason | TR:76-78 | "a Clone Hero rule (cite the page and rule id under docs/clone-hero/, and copy its Label: line); a line under ## Confirmed in docs/agents/rulings/ that names the value; the test's own input; a property"; never a snapshot, copy, or saved expected-output file | - | auditors | -
I7-114 | No golden files | TR:78 | ruling in "docs/agents/rulings/code-and-engine.md": "we don't need golden anymore, remove it all" | cites ruling | auditors | -
I7-115 | Display tests expect the engine's record | TR:80 | "The test never types the value the display should show, and never calls a function only the display uses to get it" | - | auditors | -
I7-116 | Failure of a test must be traceable to a commit | TR:84 | "Find the commit that added the test (git log -S on its name)" | - | auditors | -
I7-117 | Marking of claims | TR:86 | "reasoned" vs "confirmed"; "Agents cannot build, so every agent claim starts as reasoned" | - | auditors | -
I7-118 | Stand-ins to flag | TR:88 | "floor such as acts > 1000", "silent skip such as if (song.is_empty()) continue;", "count", "two Hydra functions compared with each other" | - | auditors | -
I7-119 | Same-rule search | TR:90 | "grep tests/ for the function, the field and the words. One rule gets one verdict" | - | auditors | -
I7-120 | Verdict vocabulary | TR:98-105 | "Keep", "Rewrite", "Remove", "Investigate"; "'Keep, with a flag' is not a verdict"; any "no/partly/not read/claim not checked" rules Keep out | - | auditors | Investigate goes to user (exit 2 of audit-method.md)
I7-121 | Production-code finding on its own line | TR:107 | "kind A in docs/agents/derive-once-review.md" | - | auditors | -
I7-122 | Filled-in record shape | TR:115-152 | fields Read, 1 Claim ... 11 Real, Verdict, Finding; commit "003d76a3" | - | auditors | -
I7-123 | Test helpers vs expected values | TR:183 | "A helper that names a tick ... or test::tick_ms ... may never produce an expected result" | - | test authors | -
I7-124 | Audit-run set-up: calibration, order, corpus mutation | TR:189-205 | calibration file "must hold at least one test whose framing is the bug"; do not use "tests/test_sp_end.cpp, tests/test_sp_meter_curve.cpp"; "Display tests go first"; main prepares engine's record "for a handful of real charts"; four mechanical checks (corpus change, plant bugs, display vs engine, standing scan); only the main session runs the first two (rule 14) | - | main, auditors | -
I7-125 | Missing-coverage pass | TR:205 | "list the rules on the docs/clone-hero/ pages and the filed rulings, and flag each one no test checks" | - | main | -
I7-126 | Test count of snapshot example numbers (374 rows, 97 vs 98 charts, 1000 floor) | TR:11,13 | "374 saved rows"; "tests still expected 97"; "acts > 1000" (see I7-129 to I7-131) | - | auditors | -
I7-127 | Pitch Dark engine values quoted | TR:15,48 | "tick 128520", "tick 128400", "chord 243" (also in TE: I7-128) | - | auditors | -

## test-rubric-examples.md

I7-128 | Pitch Dark squeeze-in step | TE:23,129 | "chord 243, at tick 128520"; function gave "tick 128400" (second copy of I7-127 values) | - | auditors | -
I7-129 | Corpus chart count (B6) | TE:101 | "CHECK(charts == 98);" (same 98 as I7-012, I7-014) | - | auditors | -
I7-130 | GUI shown count (B7) | TE:113 | "IM_CHECK_EQ(shown(), (size_t)98);" | - | auditors | -
I7-131 | Loop floor (B8) | TE:123 | "CHECK(acts > 1000);  // a floor the user approved (D43)" | - | auditors | -
I7-132 | Blight squeeze-out values | TE:74 | "chord 228 at tick 64800 ... SP end is tick 65280" | - | auditors | -
I7-133 | Bank rise ticks (B5) | TE:85 | "{13440, 14400}"; "src/search/engine.cpp line 2015" | - | auditors | -
I7-134 | Fill reach numbers (B13) | TE:179-188 | "7285", "7200 ms", "125 BPM with 480 ticks a beat", fill "9120 ... 9600", "FL-19 in docs/clone-hero/04-fills.md" (four beats before a fill's start), "FL-23" (Hydra's own 85 ms reach, "which the game does not have") | cites rule pages | auditors | -
I7-135 | Combo multiplier steps | TE:221-244, 288 | "x2_from = 10; x3_from = 20; x4_from = 30"; "SC-05 in docs/clone-hero/06-scoring.md"; x4 cap | cites SC-05 | auditors | -
I7-136 | G1 owner statement | TE:268-273 | "The engine owns it: category_scores calls it ... multiplier_step calls it ... No file under src/app, src/ui or tools calls it." (a claim about callers, with line numbers scoring.cpp:57, model.cpp:369) | - | auditors | -
I7-137 | Old test commit | TE:210 | "3b699581" (bulk checkpoint) | - | auditors | -
I7-138 | Page says what was removed and why | TE:340 | six earlier good examples dropped; "G1 was written to fill that gap" | - | readers | -
I7-139 | G1 stays only while its record holds | TE:218 | "If a later reader finds an answer in that record that is wrong, G1 comes off this page; it does not stay with a note." | - | auditors | -
I7-140 | Display test to write with the Preview fix (G2) | TE:336 | "takes every chart in testdata/input and skips none ... types no number but zero" | - | future planners | -

## this-machine.md

I7-141 | Which Python runs scripts | TM:7-9 | "Run Python as py"; "C:\Users\Patrick\AppData\Local\Python\pythoncore-3.14-64\python.exe"; Python "3.14.5"; SQLite "3.50.4"; bare python = "C:\Python314\python.exe" 3.14.7 | "which the user named for general use" | every agent | -
I7-142 | gh version, path, account, scopes | TM:13 | "2.97.0", "C:\Program Files\GitHub CLI\gh.exe", account "patseguin1-commits", scopes "gist, read:org, repo and workflow" | - | every agent | -
I7-143 | GitHub repo and how text is posted | TM:15 | "splax77/hydra-deluxe"; "Text never goes to GitHub through a hand-typed gh command. It goes through C:\Users\Patrick\.claude\gates\publish.ps1" (also IT:5, I7-210; project CLAUDE.md) | - | every agent | -
I7-144 | Release tag, files, version home | TM:17 | tag/title "v" plus version, e.g. "v3.0.0"; two files "installer and its .sig"; "The version number is set in one place, the project line of CMakeLists.txt, which reads VERSION 3.1.2 today" | claims one place | main | -
I7-145 | Hooks folder location | TM:19 | "C:\Users\Patrick\.claude\gates", git repo at "C:\Users\Patrick\.claude"; mutation tool "--target" | - | main | -
I7-146 | Ghidra and Java paths and versions | TM:23 | Ghidra "12.1.3", paths under "C:\Users\Patrick\Downloads\ghidra_12.1.3_PUBLIC_20260817\...", "analyzeHeadless.bat", "Eclipse Temurin JDK 25 (25.0.4.1)" at "C:\Program Files\Eclipse Adoptium\jdk-25.0.4.101-hotspot\bin\java.exe"; "No GHIDRA_INSTALL_DIR variable is set" | - | agents, main | -
I7-147 | REA registration | TM:25 | "npx -y rea-agents@6.5.0 mcp"; scope local in "C:\Users\Patrick\.claude.json"; skill folder "C:\Users\Patrick\.claude\skills\reverse-engineer-anything" | - | main | -
I7-148 | Clone Hero decompile location | TM:27 | "Ghidra's full decompile of Clone Hero build 6142 lives outside the repo. docs/clone-hero/code/README.md names its path" | points to owner | agents | -
I7-149 | Visual Studio installs | TM:35-39 | Build Tools 2026 "18.9.12120.119", Community 2026 "18.9.12112.369", Build Tools 2022 "17.14.37614.0" with their folders | - | build | -
I7-150 | How cmake is found | TM:41 | "Find-CMake in tools\find_cmake.ps1"; fallback "vswhere -all"; "-DCMAKE_GENERATOR_INSTANCE=..." | names owner | main | -
I7-151 | Shell is not elevated; wpr refused | TM:47-49 | "Medium Mandatory Level"; "wpr -start was refused ... 0xC5585011 (LS-T1 run 1 ...)"; "The user has ruled that no elevated window is opened" | - | every agent | -
I7-152 | PowerShell | TM:51 | "pwsh ... PowerShell 7.6.9 at C:\Program Files\PowerShell\7\pwsh.exe"; "RemoteSigned" | - | agents | -
I7-153 | Clone Hero log folder | TM:53 | "C:\Users\Patrick\AppData\locallow\srylain Inc_\Clone Hero\" with "Player.log" and "Player-prev.log" | - | agents | -
I7-154 | WinDbg/cdb | TM:57-63 | Get-AppxPackage command for "Microsoft.WinDbg"; "-pv", "-y <build-cpp\Release>", "~*kc 40; qd" | - | main | -
I7-155 | Offline stall harness | TM:67 | "tools/ch_probe/experiments/stall_harness.py"; "at most once per run" cdb attach | - | main | -
I7-156 | Opus model for agents | TM:71 | "ANTHROPIC_DEFAULT_OPUS_MODEL ... claude-opus-5-5" in "C:\Users\Patrick\.claude\settings.json" (also see CLAUDE.md rule 2) | - | main | user picks the model
I7-157 | Database files | TM:77 | dev "C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra.db"; installed "C:\Program Files\Hydra\hydra.db"; "db_path() in src/app/config.cpp" | names owner | agents | -
I7-158 | Read a DB copy never the live file | TM:81-83 | copy "hydra.db-wal and hydra.db-shm" too; read with "py" sqlite3; "winsqlite3.dll"; "There is no sqlite3 command-line tool here" | - | agents | -
I7-159 | DB tables and join key | TM:85 | "charts, meta, profiles, records and record_paths"; "Their columns are defined once, in src/store/record_store.cpp"; join "a result row's song_id is the chart's md5"; "docs/adr/0028-..." | "defined once" | agents | -
I7-160 | One-chart re-analysis | TM:89-91 | "hydra_batch.exe --db <copy> --redo <folder holding one chart>"; "load_tool_settings()"; "charts_by_md5 index ... (D76)"; whole-library --redo needs "allow library run" ("CLAUDE.md rule 8") | - | agents, main | user types allow library run
I7-161 | Quote only numbers the user can see | TM:95 | "say which path it comes from ... and whether the app displays it"; "build_path_buttons() and build_activations() in src/app/path_view.cpp" | names owners | agents | -
I7-162 | Page staleness rule | TM:3 | "Every path and version ... checked on 2026-10-31 ... the machine wins. Fix the line in the same commit" | - | agents | -

## premise-map.md

I7-163 | Where the Premise map shape lives | PM:5 | "This page is the one home of the map's shape, and of the shape of a live step's call list." | explicit one-home claim | plan authors, reviewers | -
I7-164 | Premise map table | PM:13-19 | heading "## Premise map"; columns Premise, Kind, Source, Status; id like "P1:" | - | planners, reviewer, docs test | planner fills
I7-165 | Row rules | PM:21-35 | one Kind per row; "checked" plus date; Source holds at least one backticked token; unchecked Status names "a task in the plan", "question N", or "a live step"; "blocks RG1, RG2" or "blocks nothing"; "Waits for:" line | - | planners, reviewer, docs test (tests/test_docs_match_code.cpp) | -
I7-166 | Six Kind words and what each means | PM:39-46 | "code", "data", "chart", "run", "ruling", "unchecked"; run folders "C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\"; ruling sources "## Confirmed", session card quoted words, or message quoted by main | - | planners | -
I7-167 | What counts as a source | PM:50-52 | handoff, another plan, session card, memory note, unlabeled CH block "does not count" | - | reviewer | -
I7-168 | Live-step Call list | PM:56-68 | heading "### LS-" under "## The live steps"; "Calls:" line; markers "(seen: <run id>)" or "(never tried)"; "A step holds at most one (never tried) call"; two looser readings accepted by the user 2026-10-31 ("Take all five", question 10) | - | planners, plan reviewer, docs test | -
I7-169 | Plan reviewer's three checks | PM:74 | open each checked row, cold-read every task, try each unchecked row | - | plan reviewer | -
I7-170 | Go request shows the call list | PM:78 | points to CH page (I7-095) | - | main | -

## owner-map.md

I7-171 | Where the Owner map shape lives | OM:3 | "This page is the one home of the map's shape and of the two stops that go with it." | explicit one-home claim | scouts, planners, reviewers | -
I7-172 | When a plan needs an owner map | OM:13 | "Every plan that changes what Hydra decides or shows ... A plan that only moves files, edits docs or changes hooks needs none. The main session decides, and says which in the planner's brief." | - | main | -
I7-173 | Who writes it | OM:17 | "A scout writes it before the planner starts"; main puts the map "unchanged" into the planner's brief | - | scout, main, planner | -
I7-174 | Table shape and statuses | OM:21-27 | heading "## Owner map"; columns Question, Owner, Its own words, Status; ids "O1:"; statuses "found", "split", "carry out", "none" | - | scout, planner, docs test | scout fills
I7-175 | Question wording rule | OM:29 | "in game words, never in the shape of a design" | - | scout | -
I7-176 | Stop 1: none stops work | OM:37 | "A row with Status none stops the work ... Nobody writes a new owner for it"; exception "carry out" | - | scout, main | user decides
I7-177 | Stop 2: contradicting user's words | OM:39 | "the agent stops and returns a report, not a plan"; reviewer signs off "BLOCKED" (also in brief-preamble "When blocked": I5/I6) | "is the one home of this rule" is claimed by brief-preamble, not here; see preamble | scout, planner, reviewer | user decides
I7-178 | What the planner may do | OM:43 | "uses each row's owner as it is. It never builds a new value out of several owners. It never adds a condition" | - | planner | -
I7-179 | Reviewer's check | OM:47 | opens each owner, reads its words; lists every question steps answer with map closed | - | plan reviewer | -
I7-180 | Counter-example owner | OM:31 | "SPSqueeze::needs_no_timing (src/core/model.h) ... 'not this one'" | - | scouts | -

## public-doc-style.md

I7-181 | Which pages are public | PS:3 | "README.md, docs/README.md, docs/UserGuide.md, docs/differences-from-public-hydra.md, and the GitHub release notes" (project CLAUDE.md says "four public pages and the GitHub release notes") | - | writers, hooks that gate docs | -
I7-182 | Voice rules | PS:11-23 | "you", present tense, active; no "I"/"we"; app is "Hydra Deluxe" ("Hydra" in notes before 2.0.0); "Every number says where it comes from ... Never invent a number, a timing or an example song" | - | writers | -
I7-183 | Banned words | PS:27-31 | "seamless, robust, powerful, comprehensive, effortless, crucial, pivotal, key (as an adjective), vibrant, showcase, leverage, empower, enhance, foster, delve, intricate, landscape, testament, streamline"; "serves as", "stands as", "acts as", "represents"; "It's worth noting", "Note that", "In this section", "Let's", "Simply", "just", "In order to", "To summarize", "In conclusion" | - | writers, doc reviewer (a doc-review gate may match words; compare I5/I6) | -
I7-184 | Sentence-shape rules | PS:39-49 | no "not X, but Y"; no reflex threes; no trailing "-ing" clause; no self-asked questions; "No em dashes in prose"; short sentences | - | writers | -
I7-185 | Format rules | PS:53-63 | sentence-case headings, no duplicate headings on a page, no bold-label bullets, UI labels in `code` style, no emoji, link text not "here", condition before instruction | - | writers | -
I7-186 | Structure rules | PS:67-73 | README short; User Guide follows screen order; mechanics "explained once" and linked | - | writers | -
I7-187 | Release-note rule | PS:77-79 | "describes the app as it was at that release ... keeps every fact the note states ... If a note looks wrong about its own release, leave the sentence and report it"; opens with "one or two plain sentences" | - | writers | -
I7-188 | Do-nots | PS:83-87 | no AI-detector chasing; no dropping facts; no claim "you can't point to in the code, the release notes or a measured run" | - | writers | -
I7-189 | Rule provenance | PS:7 | "two web searches on 2026-10-10" with named sources | - | readers | -
I7-190 | Plain-English rules owner | PS:3 | internal docs "follow the user's 'How to explain things' rules in ~/.claude/CLAUDE.md" (preamble also says that file is "the one home of those rules") | points to CLAUDE.md | writers | -

## handoff.md

I7-191 | Handoff file location and name | HO:7 | "docs/handoffs/<YYYY-MM-DD>-<slug>-handoff.md" | - | /handoff command, checker | -
I7-192 | Handoff shape | HO:11-15 | prose (goal, done, next, rulings quoted with session id) and table under exactly "## Claims" with columns Kind, Value, Claim, How to check; Value in backticks | - | writers, checker | session writing the handoff fills
I7-193 | Seven claim kinds | HO:17-25 | "commit", "branch", "path", "function", "test", "ruling", "state" | - | checker | -
I7-194 | Checker output heading | HO:27 | "## Checker output" | - | writers | -
I7-195 | Hash rule | HO:35 | "Write every hash in full, all 40 characters" | - | writers, checker | -
I7-196 | No scratch paths in a handoff | HO:37 | "Name no scratch path" (docs test skips docs/handoffs/ so rule holds "by instruction") | - | writers | -
I7-197 | Checker command and exit codes | HO:45-57 | "py -I tools/check_handoff.py <handoff.md> [--repo <root>] [--since <YYYY-MM-DD>]"; statuses ok/missing/not-on-main/not-pushed/manual; exit 0, 1, 2 | the script is "in Hydra's tools/ folder" | checker, continuing session | -
I7-198 | Scrape mode for old handoffs | HO:59 | sections "EXISTS", "MISSING", "CHANGED SINCE <date>"; summary "checked N claims: E exist, M missing, C changed since <date>" | - | checker | -
I7-199 | Who runs the checker when | HO:65-67 | /handoff right after writing; continuing session before work (also brief-preamble "Continuing from a handoff": I5/I6) | - | /handoff, sessions | -

## integrate.md

I7-200 | Role: integrator joins a wave's branches | IN:3-5 | "You write no new feature code. You only fix what the join itself breaks" | - | integrator agent | -
I7-201 | What main gained since fork | IN:13-15 | "git log --first-parent <fork>..main --oneline"; "git diff <fork>..main -- tests/single_owner/rules/" | - | integrator | -
I7-202 | Fold copies into owners | IN:21-23 | "A task line that answers the same question without calling the owner is a copy. Fold it into the owner in the join, and name it in the merge commit body"; "If a fold would change a displayed text, a score or a stored record, stop and report it. Do not pick." | - | integrator | -
I7-203 | Join procedure | IN:27-31 | "git merge --no-ff --no-commit <task branch>"; commit with separate -m and the preamble's trailers; hold last merge until precheck clean; rebuild appended rule files as "wave branch's version plus the other task's added rows"; never rebase/amend/reset/force-move; conflict between two answers = stop and report | - | integrator | -
I7-204 | Check the joined branch | IN:35-39 | "build-cpp\Release\hydra_precheck.exe --repo <worktree path> --range main...<wave branch>"; "If the main checkout has none, stop and report it"; large-range note passed on | - | integrator | -
I7-205 | Tests the main session runs after hand-over | IN:43-46 | `build-cpp\Release\hydra_tests.exe -tc="single-owner*"`; `build-cpp\Release\hydra_tests.exe -sf=*docs_match_code*`; "hydra_precheck.exe --repo <worktree path> --tests main...<wave branch>" (rule 1 of agent-rules; I5/I6) | - | main | -
I7-206 | Failure of build or tests | IN:48-50 | "a fresh executor fixes it on the wave branch"; "Never loosen a scan row to make it pass"; "Never amend" | - | main, executor | -
I7-207 | Hand over to review | IN:54-56 | "Stop changing the tree once the last merge is committed. The gate keys a review on the exact tree"; "a fresh Sonnet reviewer with docs/agents/derive-once-review.md"; integrator cannot review | - | integrator, main | -
I7-208 | Integrator report contents | IN:60 | wave branch, worktree, head hash, each task branch with merge hash, owners gained, folds, conflicts, precheck output | - | integrator | integrator supplies

## issue-tracker.md

I7-209 | Where issues live and how to operate | IT:3-16 | GitHub issues, `gh` CLI; "publish.ps1 -Kind issue -Title \"...\" -BodyFile <file>"; "publish.ps1 -Kind issue-comment -Target <number> -BodyFile <file>"; list command with --json fields; "gh issue edit", "gh issue close" | - | agents, main | -
I7-210 | Posting goes through one script | IT:5 | "Text reaches GitHub only through C:\Users\Patrick\.claude\gates\publish.ps1 ... the settings refuse the posting gh commands typed by hand. Reading with gh is not restricted." (also TM:15, I7-143, and project CLAUDE.md) | claims the single route | publish.ps1, settings | -
I7-211 | PRs as a request surface | IT:20 | "PRs as a request surface: no." (flag read by /triage) | - | /triage | -
I7-212 | External-PR filter | IT:25 | authorAssociation "CONTRIBUTOR, FIRST_TIME_CONTRIBUTOR, or NONE" kept; OWNER/MEMBER/COLLABORATOR dropped | - | /triage | -
I7-213 | Wayfinder map and child conventions | IT:40-47 | label "wayfinder:map"; child labels "wayfinder:research/prototype/grilling/task"; "Part of #<map>" top line; "Blocked by: #<n>, #<n>" line; "gh api is denied"; claim "gh issue edit <n> --add-assignee @me"; frontier = first open unassigned unblocked child in map order | - | /wayfinder | -

## triage-labels.md

I7-214 | Five triage labels | TL:7-11 | "needs-triage", "needs-info", "ready-for-agent", "ready-for-human", "wontfix", each equal to its name; meanings in the table (the project CLAUDE.md states "each label string equal to its name") | - | /triage skill, agents | -
I7-215 | How a skill role maps to a label | TL:13 | "use the corresponding label string from this table" | - | skills | -

## domain.md

I7-216 | What an exploring skill reads first | DM:7-9 | "CONTEXT.md" at repo root, "CONTEXT-MAP.md" if present, "docs/adr/" | - | engineering skills | -
I7-217 | Missing domain files | DM:11 | "If any of these files don't exist, proceed silently" | - | skills | -
I7-218 | Use glossary terms | DM:43 | "use the term as defined in CONTEXT.md. Don't drift to synonyms" (CLAUDE.md says only the user changes a term's meaning: agent-rules, I5/I6) | - | agents | -
I7-219 | ADR conflicts surfaced | DM:49 | "surface it explicitly rather than silently overriding", with the form "Contradicts ADR-0007 ..." | - | agents | -

## doc-screenshots.md

I7-220 | Command that retakes all doc pictures | DS:5-7 | "pwsh -NoProfile -File tools\doc_shots.ps1" | - | main | -
I7-221 | The one list of pictures | DS:11,29 | "tools/doc_shots/shots.txt is the one list of the pictures"; "every file [in docs/images/] is a picture the shot list takes" | explicit one-list claim | script, main | -
I7-222 | Runner data for pictures | DS:11 | "testdata/input", scratch settings; "never reads your song library or your settings" | - | script | -
I7-223 | Main picture crop rule | DS:15 | "a rect line directly before a screenshot line that has no ref; the script then keeps only the part of the frame below that rectangle"; comment at top of shots.txt "says the same" | second statement in shots.txt | script | -
I7-224 | Shrink step and size limits | DS:17 | "tools/doc_shots/shrink.py"; "optimize=True"; prints "size:" and "total:" lines; "the constants at the top of tools/doc_shots/shrink.py hold" the limits | names owner | script | -
I7-225 | Who runs it | DS:19 | "the main session runs it, never an agent (agent rule 14)"; build target "hydra_uitest" | - | main | -
I7-226 | When to run | DS:23 | "before each release"; "after any change to a label or a layout that one of the pictures shows"; then open every changed picture, commit on a branch, get reviewed | - | main | -
I7-227 | Settings reset in shots | DS:25 | shots open the settings popup, click "Reset to defaults", check button reads "Analysis settings: defaults" | - | shots.txt | -

## Hook gaps seen
One gap exists: UT:99 itself says no hook reads shell text, so a shell script that sends keys or wheel messages to Hydra's window from outside the command channel can scroll the library; only the written rule binds there. No steps given.

## Same value in two of my own files (cross-refs)
- 98 test charts: I7-012, I7-014, I7-129, I7-130.
- Pitch Dark tick 128520 / 128400, chord 243: I7-127, I7-128.
- publish.ps1 as the only route to GitHub: I7-143, I7-210.
- Library-run narrowing (search first / no whole-library): I7-004, I7-039, I7-084, I7-160.
- Premise map call-list / go-request: I7-095, I7-168, I7-170.
- Tool list copy: I7-042 says it is "a copy" of tools/hydra_ui_mcp.py; I7-065 says CH defers to build_server().
- Real-app record fields (I7-046) vs "that gate's contract" (I7-045): the gate itself is outside my files.
- Opus model: I7-156 (TM) vs project CLAUDE.md rule 2 (outside my files).

Total rows: 227 (I7-001 to I7-227).
