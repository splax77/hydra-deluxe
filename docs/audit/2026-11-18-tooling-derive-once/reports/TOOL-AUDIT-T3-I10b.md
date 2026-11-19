# TOOL-AUDIT-T3-I10b (re-send inventory scout)

Repo read: C:\Users\Patrick\Downloads\Hydra\hydra-test, commit af9dfac4e246df6b77c4b906f5c7054f47beaa82 (main checkout, tools/). Rows are I10b-001 onward. Paths are relative to tools/. "Owner claim" is quoted where the text says it; "no" means no ownership claim read. Coverage is partial: see "Not reached" at the end. I read headers, constant blocks and the comments beside each constant, not every function body.

Row format: id | question | file:line | value | owner claim | who acts | who is asked.

## ch_mcp (Clone Hero MCP server), Python
I10b-001 | CPU share at which the machine counts as quiet before a launch | ch_mcp/quiet.py:29 | QUIET_CPU_PCT = 30 | no | ch_start (agents via the ch server) | -
I10b-002 | Seconds the CPU share is averaged over | quiet.py:30 | QUIET_SAMPLE_S = 5 | no | ch_start | -
I10b-003 | How young a build folder's newest file must be to count as a build at work | quiet.py:31 | BUILD_RECENT_S = 60 | no | ch_start | -
I10b-004 | How long the quiet check takes, as told to agents | ch server instructions text (MCP "instructions") | "checks the machine is quiet (about 6 s, and it can refuse)" | no | every agent reading the tool text | - (a prose statement near I10b-002 and I10b-005; no code row says 6)
I10b-005 | Longest the build-folder scan may run | quiet.py:46 | SCAN_CAP_S = 3.0 | comment: "The user's ruling of 2026-11-07: how long the folder scan may run" | ch_start | -
I10b-006 | Which process names count as a running compiler (refuse a launch) | quiet.py:35 | COMPILER_NAMES = ("cl","link","MSBuild","cmake","ninja","clang-cl","lld-link","rc","mt","tsc","esbuild"); comment: node and python "stay out on purpose" | no | ch_start | -
I10b-007 | An MSBuild started with a given flag is an idle helper, not a compiler | quiet.py:~40 | regex `\s/nodemode:1(\s|$)`; "The user's ruling of 2026-11-07" | cites ruling | ch_start | -
I10b-008 | Names reported but never a reason to refuse | quiet.py:49 | REPORTED_NAMES = ("Spotify.exe","MsMpEng.exe") | no | machine_report.py (reused, see I10b-042) | -
I10b-009 | Caps for the bot live steps: poll, extra cap, exit check | bot_run.py:66-68 | POLL_INTERVAL_S 1.0; POLL_CAP_EXTRA_S 60.0; EXIT_CHECK_S 30.0 | comment: "The user's caps for the live steps: question 5 of the feasibility plan" | ch_start / bot runs | -
I10b-010 | Default player when none given | bot_run.py:63 | DEFAULT_PLAYER = "Drums,Expert" | no | ch_start | -
I10b-011 | Human-launch caps (attach, arm, patch, poll) | bot_run.py:88-91 | 20.0 s, 5.0 s, 30.0 s, 0.5 s | comment: "the direct-start plan's question 1, answered by the user on 2026-10-31" | ch_start human=True | -
I10b-012 | Launch forms and states | bot_run.py:99-107 | "suspended","spawn"; "idle","waiting","patched","refused" | no | ch_start | -
I10b-013 | What a human launch with no song says | bot_run.py:110 | HUMAN_NEEDS_SONG "refused: a human launch needs a song folder, because the game's ..." | no | agents | -
I10b-014 | The one-byte patch that makes the command-line loader build a human player | launch_patch.json | loader_rva 0x3968D0, patch_rva 0x396EC5, expect "BA 01 00 00 00", write "BA 00 00 00 00", entry_expect "40 56 57 41 55 48 81 EC C0 00 00 00", build 1.1.0.6142, launch "suspended" | json "about": "The one-byte patch the launch agent (agent/launch.ts) makes ..." | launch agent (agent/launch.ts, launch_core.ts) | -
I10b-015 | Where the il2cpp dumps live | build_facts.py:28; launch_patch.json "dump"; kit_methods.json "dump"; ch_mcp/README.md | r"C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps" and ...\6142\dump.cs | README: "it lives outside it, in ..." | dump tools, kit agent | - (same path in four places)
I10b-016 | Dumper path and cap | dump_build.py:20,25 | DEFAULT_DUMPER r"C:\Users\Patrick\Downloads\Il2CppDumper-net7-v6.7.46\Il2CppDumper.exe"; DUMPER_CAP_S = 600 | README names DUMPER_CAP_S as the cap | dump_build.py | -
I10b-017 | Files a good dump must have | dump_build.py:28-34 | REQUIRED_FILES ("dump.cs","script.json","stringliteral.json"); REPORTED_FILES adds il2cpp.h; STAGES ("Dumping...","Generate struct...","Generate dummy dll..."); DONE_LINE "Done!" | no | dump_build.py | -
I10b-018 | Default build number and facts names | dump_facts.py:53-54,381 | DEFAULT_BUILD "6142"; NAMED_SIGNATURES ("GameManager","ScoreManager","DrumsPlayer"); ENGINE_SIGNATURE "drums_engine" | no | dump_facts.py | -
I10b-019 | Where the game lives | game_files.py:31 | GAME_DIR = r"C:\Clone Hero\1.1 june\Clone Hero" | no (also typed in ch_mcp/README.md commands) | all ch tools | -
I10b-020 | Game save folders and files | game_files.py:36-41 | Documents\Clone Hero; AppData\LocalLow\srylain Inc_\Clone Hero; "scorestats.json"; "settings.ini" | no | game_files / ch_stop | -
I10b-021 | What is backed up and restored around a run | game_files.py:44-77; MCP text | registry key r"HKCU\Software\srylain Inc.\Clone Hero" saved as "rewired.reg"; settings.ini gets pause_on_focus_lost = "0" for the run; backup prefix "backup-", manifest "backup.json", suffix ".after-run"; REG_CAP_S 30.0; LEFT_AS_SAVED "left as the game saved it" | MCP text: "No file in the game's folders is backed up or restored: settings.ini keeps the run's line" | ch_start / ch_stop | -
I10b-022 | song.ini key used for a song's title | game_files.py:52-55 | "song.ini", section "song", key "name" | no | ch_open_song, install | -
I10b-023 | Chart files a song folder may hold | install_chart.py:37 | CHART_FILES = ("notes.chart","notes.mid") | no | install | -
I10b-024 | Install wait for the silent-audio ffmpeg call | game_tools.py:45 | INSTALL_CAP_S = 60.0 | comment: "the plan's question 5d" | ch_install_chart | -
I10b-025 | Scratch and log name prefixes | game_tools.py:47-57 | "ch_live_", "press-", "builds-", marker "ch_open_run.json" | no | game_tools | -
I10b-026 | Press routes, refusal, and the typed permission | game_tools.py:70-79; server.py:390-391,465-477; MCP text | ROUTES ("kit","keys"); NO_ROUTE_NO_KIT "refused: no route given and the kit is not set up; call ch_kit_setup first"; "a hook refuses them unless the user has typed allow key run" | no | ch_play_chart, ch_key, ch_press; a hook (not read) | the USER is asked to type "allow key run"
I10b-027 | State read caps | frida_tools.py:43-49 | ATTACH_DELAY_S 5.0; ATTACH_CAP_S 30.0; READ_CAP_S 5.0 | comments cite 5a, 5b, 5c of a plan | ch_attach, ch_state | -
I10b-028 | Engine flags bit for dynamics stop | frida_tools.py:68 | DYNAMICS_STOP_BIT = 0x10 ("SC-16 in docs/clone-hero/06-scoring.md") | cites doc | state words | -
I10b-029 | Refusal texts for traces and attach | frida_tools.py:56-61 | NO_GAME_MANAGER; TRACE_RUNNING "a trace is already running; call ch_trace_stop first"; NOT_ATTACHED "not attached; call ch_attach first" | no | agents | -
I10b-030 | Menu field kinds and page words | frida_tools.py:150-167; menu_fields.json (177 lines) | ROW_ADDS_SCROLL {"Confirm":False,"Difficulty":False,...}; NO_ROW_PAGE "Ready" | no | ch_menu | -
I10b-031 | Front server timing | front.py:40-47 | WORKER_START_CAP_S 30; WORKER_EXIT_CAP_S 10; FRONT_EXIT_CAP_S 90; FRONT_POLL_S 0.5 | no | ch front | -
I10b-032 | Front tool names | front.py:55-60 | "ch_reload","ch_call","ch_open_items","nothing open" | no | ch front | -
I10b-033 | Kit controller identity | kit_tools.py:59-88 | KIT_DEFINITION_ID 1, "MIDI Drums", map category 1, layout 2, tag "hydra-kit", KIT_REQUIRED_ID 6 ("the literal the game's velocity gate passes to ContainsController"), KIT_PLACEHOLDERS = 6, placeholder tag "hydra-placeholder-" | comments: "The fact's home is block IN-76 of docs/clone-hero/07-internals.md"; "(the plan's owner table)" | kit setup | -
I10b-034 | What can be pressed on the kit | kit_tools.py:153-165 | KIT_ELEMENTS: red 0 Snare, yellow 1 Tom1, blue 2 Tom2, green 3 Tom3, yellow_cymbal 4 Cymbal1, blue_cymbal 5 Cymbal2, green_cymbal 6 Cymbal3, start 18 StartPad at 1.0, select 19 SelectPad at 1.0, kick 7 Kick | comment: "The one table of what can be pressed on the kit, by the name a caller uses" | kit tools, menu steps, play_chart | -
I10b-035 | Kit element to game action ids | kit_tools.py:170-181 | KIT_BINDINGS {0:(1,"Red"),1:(2,"Yellow"),2:(3,"Blue"),3:(0,"Green"),4:(37,"YellowCymbal"),5:(38,"BlueCymbal"),6:(39,"GreenCymbal"),7:(4,"Orange"),18:(16,"Start"),19:(6,"Select")} | "The bindings the game's drum map must give the kit"; "The action ids are the game's action table" | kit setup check | -
I10b-036 | Kit press timing and caps | kit_tools.py:96-123 | KIT_HOLD_FRAMES 2; KIT_SETUP_CAP_S 10; KIT_PRESS_CAP_S 1; KIT_REWRITE False; KIT_PRESS_ON_MAIN False; KIT_WATCH_MAX_S 60.0 ("the user's answer to the plan's K7 question, 2026-10-30, 'Cap at 60 s (Recommended)'"); KIT_RATE_WINDOW_S 1.0; KIT_EVENT_WAIT_S 0.05 | cites user answers | ch_kit_press, ch_kit_watch | -
I10b-037 | Rewired devices switched off for a run | kit_tools.py:129 | KIT_DEVICES = ("keyboard","mouse") ("the user's rulings of 2026-10-30") | cites rulings | ch_kit_setup / ch_stop | -
I10b-038 | Frame probe | kit_tools.py:734-740 | item "green", 5.0 s, prefix "frame_probe-" | no | ch_kit frame probe | -
I10b-039 | Play stop reasons and event words | kit_tools.py:709-730 | STOP_REASONS done, past_end, stopped, cap, frozen, jumped_back; events skipped/sent/dropped/stop; PRESS_EVENT_FIELDS | no | play_chart, reports | -
I10b-040 | Machine recorder rates and files | machine_recorder.py:41-60 | RATE_PER_S 50 ("Question 12 of the plan"); RECORDER_CAP_EXTRA_S 60 ("Question 18"); samples.csv, processes.csv, threads.csv; stop words | cites plan | recorder | -
I10b-041 | Windows thread state to words | machine_recorder.py:98 | STATE_READS_AS {2:running,1:ready,3:ready,7:ready,5:waiting} | no | recorder, report | -
I10b-042 | Report thresholds | machine_report.py:39-52 | LONG_STEP_FACTOR 2; READY_STRETCH_MIN 2 ("Question 16"); LONGEST_LISTED 10; STEP_PERCENTILE 99 ("Design point 2"); ALWAYS_LISTED_NAMES = quiet lists ("Question 2 of the plan and quiet.py's factors list"); SETTINGS_KEYS ("framerate","vsync") | cites plan | ch_stop report | -
I10b-043 | Recorder folder and log names | recorder_link.py:23-29 | RECORDER_EXIT_CAP_S 2; "machine-"; "recorder.log" | no | recorder link | -
I10b-044 | Menu walking caps and screens | menu_tools.py:37-57 | MENU_STEP_CAP_S 10.0; SEARCH_DOWN_CAP 10; MENU_POLL_S 0.5; SEARCH_NEXT_ITEM "kit:blue"; screens PlayerProfileMenu, MainMenu, SongSelect, PlayerSelectionMenu | no | ch_open_song | -
I10b-045 | Route from title to song | menu_steps.json | steps with items like "kit:start" and expect words; "verified": false; two wiki.clonehero.net URLs as sources | json "about": the presses ch_open_song makes | ch_open_song | -
I10b-046 | Game methods the agents may call | kit_methods.json (55 lines); agent/kit.ts:44 ALLOWED_CALLS; agent/menu.ts:40 | list of methods with dump_line numbers | json "about": "Every game method the kit agent may call" | kit agent | -
I10b-047 | Method watch | method_watch.py:42-61 | METHOD_WATCH_CAP_S 120.0; HOOK_KINDS all/listed/by_name; start_candidates.json | no | method watch | -
I10b-048 | Mono reader cap and assemblies | mono_facts.py:35-45 | READER_CAP_S 300.0; GAME_ASSEMBLIES | no | mono_facts | -
I10b-049 | Star Power watch | sp_tools.py:49-66 | GAP_MS 50 ("question 7 of the run timing plan ... answered by the user"); HALF_COL "sp_half_u64"; CSV_PREFIX "sp_watch-"; refusal texts | cites user | ch_sp_watch | -
I10b-050 | Trace caps | trace_hits.py:57-74 | TRACE_CAP_MARGIN_S 30.0 ("Hydra's song end for the traced folder's chart plus this margin"); WALL_CAP_PAUSE_S 120.0 ("approved by the user on 2026-10-28") | cites user | ch_trace_start | -
I10b-051 | Song end used for a run | hydra_end.py:21-26 | FIELD "song_length_ms"; folder prefix "hydra_end_" | trace_hits comment: "the end is D331's" | trace cap | -
I10b-052 | Foreground poll | foreground.py:22 | FOREGROUND_POLL_S 0.1 | no | key routes | -
I10b-053 | Build stamp rules | build_stamp.py:32-43 | STAMP_FORMAT 1; BUILD_INPUTS ("package.json","package-lock.json","tsconfig.json"); folder "tools/ch_mcp/agent"; NO_REPLAY_BUILD text | no | ch_run_chart record, reload | -
I10b-054 | Dynamics words | kit_tools.py:262-264; compare_scorestats.py:45-46 | "none","ghost","accent" / "accent","ghost" | no (two files state them) | compare tools | -
I10b-055 | Disassembler default size | chdis.py:37-38 | script.json; DEFAULT_MAXBYTES 0x1200 | no | chdis | -
I10b-056 | Agent timing and limits (TypeScript) | agent/clock.ts:14; kit.ts:72,82; trace.ts:37,43; menu.ts:54 | SAMPLE_MS 250; TICK_MS 2; WATCH_LIST_MAX 20; ARG_SAMPLE_LIMIT 200; END_POLL_MS 250; PLAYER_SCREEN_FRAME_RVA 0x32F590 | no | Frida agents | -
I10b-057 | README tells a person what to run | ch_mcp/README.md | `py tools/ch_mcp/dump_build.py --game "C:\Clone Hero\1.1 june\Clone Hero" --out "...\ch-dumps\6142"`; dump_facts.py `--version 1.1.0.6142-final` | README describes "The server" | whoever runs the dump | the runner supplies the build number

## ch_probe (hit-window probe)
I10b-058 | Where addresses live | ch_probe/constants.py:3-7 | "This is the ONE place the numbers live. Every other module imports from here; nothing else hard-codes an address." | CLAIMS OWNER (quoted) | all ch_probe modules; dump_facts.py:383 points to it | - (agent/menu.ts:54 and launch_patch.json also hold RVAs; see I10b-014, I10b-056)
I10b-059 | Function RVAs and field offsets of build 6142 | constants.py:38-156,326-342 | RVA_WINDOW_FORMULA 0x20DDDA0; DRUMS_ENGINE_CTOR 0x20DF680; NOTE_PROCESSING 0x20DCC90; HIT_CHECK 0x100DAB0; NOTE_SCORE_ADD 0x20DEEC0; NOTE_DYNAMICS 0x20DE770; OFF_TOTAL_WINDOW 0x20; BACK 0x30; FRONT 0x38; SONG_CLOCK 0x100; SCORE 0x94; ROUND_START 0x2E0; SP_AMOUNT 0x68; SP_MAX 0x1B0; SP_HALF 0x1B8; SP_ACTIVE 0x42; SP_MULTIPLIER 0x48; SP_END_TIME 0x120; SP_OVERFLOW 0x130; FLAGS 0x198; PRECISION_MODE_BIT 0x1000; DYN_THRESHOLD 0x2C0; NOTE_COUNT 0x8C; listener 0x1A0; GM 0x20; clock offsets 0xB0, 0x18, 0x20, 0x38 | claims the one place (I10b-058) | probe tools | -
I10b-060 | Hit window constants as expected | constants.py:229-235,168-171 | EXPECT_NORMAL_BACK_S 0.085; EXPECT_NORMAL_FRONT_S 0.0375; EXPECT_PRECISION_BACK_S 0.040; precision front 0.025 (comment at :171); tolerance 0.5 ms | claims the one place | probe | -
I10b-061 | Whole-window cap and floor | constants.py:250-267 | WINDOW_CAP_MS 171.4313; WINDOW_FLOOR_MS 96.2932; CAP_FROM_GAP_MS 2*85 = 170; FLOOR_UP_TO_GAP_MS 2*37.5 = 75; match tolerance 0.01; change tolerance 1e-6 | comment: measured | probe; Hydra's own core/note_window.h states the same rule (not read by me) | -
I10b-062 | Probe timing and spacing | constants.py:295-317 | ENGINE_EMPTY_WINDOW_S 0.001; ENGINE_CLOCK_MOVED_S 1e-6; ROUND_START_MAX_GAP_MS 10.0; CLOCK_FRESH_S 0.002; CLOCK_MAX_FILL_S 0.05; INPUT_SETTLE_MS 250; PROBE_SPACINGS_MS tuple | no | probe | -
I10b-063 | Stall hunt | constants.py:349-358 | LOG_WRITER_CAP_S 5.0; STALL_THRESHOLD_MS 40.0; STALL_RUN_MINUTES 10.0; STALL_HUNT_RUNS_MAX 3 ("D335 item 3, 'Yes, those numbers (Recommended)'") | cites user answer | stall_harness | -
I10b-064 | Game process names | constants.py:270-273 | "GameAssembly.dll", "Clone Hero.exe", window title "Clone Hero" | no (game_files.py has PROCESS_NAME separately) | probe | -
I10b-065 | Menu key pacing | input_driver.py (MENU_KEY_GAP_S, MENU_KEYS), imported by game_tools.py:39 | values not read | - | key route | -
I10b-066 | ch_probe spec | ch_probe/SPEC.md (303 lines), README.md (102 lines) | constants.py says its numbers are "transcribed from the handoff spec" | SPEC is named the source by constants.py | - | not read

## tools/capture, doc_shots, fake_hydra_live
I10b-067 | How a capture starts Hydra | capture/shot.ps1, steps.ps1, report_shot.ps1 | all dot-source "..\capture_window.ps1" (Start-Hydra $Dir <wait>); shot.ps1 default Wait 10; steps.ps1 and report_shot.ps1 wait 12 | comments: "Setup and -Dir: see tools/capture_window.ps1" | person or agent running a capture | the caller supplies -Dir, -Out, -Prefix
I10b-068 | Window steps and ShowWindow codes | capture/steps.ps1:5-8 | min=6, restore=9, max=3; default steps start,min,restore,max; also click<X>x<Y> and esc | no | capture runs | the caller supplies steps
I10b-069 | Waits and message codes in captures | steps.ps1, shot.ps1 | 700 ms before shot; 3 s after show-state or esc; 6 s after click; 300/150 ms click timing; key message 0x0100/0x0101 with 0x1B | no | capture | -
I10b-070 | Path-report button position and wait | capture/report_shot.ps1:7-8,20-28 | ClickFromRight 80 ("2,560 - 2,480 on the user's maximized window"), ClickY 22; title match "Path report*"; wait up to 30 s; 10 mouse moves 50 ms apart | comment names place_control_at_right_edge in library_toolbar.cpp as the button's home | capture run | the caller may override
I10b-071 | Doc picture size limits | doc_shots/shrink.py:18-19 | MAX_FILE_BYTES = 500 * 1024; MAX_FOLDER_BYTES = 4 * 1024 * 1024; "The user's limits (D300 ...). tests/test_docs_match_code.cpp checks the same two at merge." | names two homes (here and the C++ test) | doc_shots.ps1 | -
I10b-072 | The list of doc pictures | doc_shots/shots.txt:1-10 | "This file is the one list of those pictures; the page that explains it is docs/agents/doc-screenshots.md"; timeout 120; files main-screen.png, paths-tab.png, preview-tab.png, stars-tab.png, dynamics-tab.png, analysis-settings.png, search-tips.png, path-report.png | CLAIMS OWNER (quoted) | doc_shots.ps1 | -
I10b-073 | State every picture starts from | shots.txt:12-20 | click "Reset to defaults"; expect-text "Analysis settings: defaults" | no | doc_shots runner | -
I10b-074 | Stand-in hydra session | fake_hydra_live.py:60- | SESSION_TIMEOUT_S = 25; BIG_TEXT = 1,000 lines of 49 characters plus newline; mode "window" if exe is hydra.exe else "headless"; commands "dump", "quit" | no | tests of the hydra MCP | -

## tools/precheck (derive-once precheck, C++)
I10b-075 | Default range and options | precheck/main.cpp:10-12,41 | default range "main...HEAD"; repo default current folder; options --repo --range --whole-tree --rules-at --disable --list-checks --code-mask --tests; "It never writes to the repository" | no | agents (rule 14 lets them run it from main) | -
I10b-076 | The five checks and their kinds | precheck/checks.cpp:7-13 | 1 "Helpers defined twice (kind C)"; 2 "Recompute spellings in tests (kind B)"; 3 "New numbers with no decision (kind D)"; 4 "Scans outside the scan file (kind E)"; 5 "Second homes in src/ (kind F)" | no | precheck; reviewers | -
I10b-077 | Body size floor for "same function" | definitions.h:20 | kBodyTokenFloor = 30; "The user's decision is D49" | cites decision | check 1 | -
I10b-078 | Namespace lookback | definitions.h:25 | kNamespaceLookBack = 200 ("D49 names it") | cites | definitions.cpp | -
I10b-079 | Size of a range too big to review in one read | items.h:50 | kLargeRangeLines = 400; "D82 item 2 ... a starting value to be tuned later. This is the one place it lives." | CLAIMS OWNER (quoted) | items; the reviewer | -
I10b-080 | Range output prefixes | items.cpp:11-12 | "range size: ", "large range: " | no | reviewers | -
I10b-081 | Repeat sizes that count | check_5.cpp:26-28 | kMinRepeatedString 12; kMinRecordFields 3; kMinSharedFields 4; "the user's decision D113 item 8 ... This is the one place they live." | CLAIMS OWNER (quoted) | check 5 | -
I10b-082 | Numbers that are unit factors, not limits | check_3.cpp:430-432 | {"1000","1024","1000000"}, "decision D49" | cites | check 3 | -
I10b-083 | How far a cited decision reaches | check_3.cpp:490 | the line or "the three above" | cites D49 | check 3 | -
I10b-084 | Where decisions are looked up | decisions.cpp:317-338 | ls-tree of "docs/adr", "CONTEXT.md", "docs/superpowers/plans", "docs/audit" | no | check 3 | -
I10b-085 | Number canon limits | decisions.cpp:23-31 | kWholeNumberLimit 1e15; kMaxHexDigits 16; kBoldLevel 99 | no | decisions parsing | -
I10b-086 | Test picker limits | test_pick.h:22-30 | kCommonNameLimit 30 ("the user chose 30"); kPickSizeLimit 40 ("the user chose 40"); kHeaderIncluderLimit 20 | cites user | --tests picker | -
I10b-087 | Which test files are picked | test_pick.cpp:26-34,178,320 | top-level tests/*.cpp only; tests/single_owner/ and the ui folder left out; tests/test_main.cpp, tests/shards.txt and third_party/doctest/ run the whole suite; program names start "hydra_" | no | merge-test picker | -
I10b-088 | Why-text on a second home | check_5.cpp:30-33 | kWhy "a second home for an idea that already has one; ... (derive-once review, step 8)" | no | reviewers | -
I10b-089 | Hand-listed known copies and spellings | check_1.cpp:42-45; check_2.cpp:1-4,82 | lists with no scan row; "when a row lands, this entry goes" | no | checks 1 and 2 (scan rows live in check_rows.cpp) | -
I10b-090 | Scan row kind letters | check_rows.cpp:24,245-246 | maps "tests/source_tree.h" to "E" and tests/ paths to kinds | no | precheck | -

## replay*.cpp
I10b-091 | Game frame period | replay_schedule.cpp:26 | kFrameMs = 3.0 ("TM-49 ... 330 cap", "2.98 to 3.03 ms" SP-47; "The user set it: question 1 of ...2026-11-08-player-freak-fixes.md") | no | hydra_replay schedule | -
I10b-092 | Press judged one frame after the clock the player read | replay_schedule.cpp:31 | kJudgeEarlyMs = 1 * kFrameMs ("D319 item 3 'One frame'"; "Every rule here reads this one frame") | no | schedule | -
I10b-093 | Chord press lead | replay_schedule.cpp:34 | kPressLeadMs = 2.0 (D51) | no | schedule | -
I10b-094 | Lead of a chord that ends Star Power | replay_schedule.cpp:40 | kSpEndLeadMs = 15.0 ("The user set it: question 3 of the optimal-score run plan") | no | schedule | -
I10b-095 | Lag of a squeezed-out chord's late gem | replay_schedule.cpp:45 | kSpOutLagMs = 15.0 ("question 2 of ...player-refinements.md") | no | schedule | -
I10b-096 | Gap between order-split chord groups | replay_schedule.cpp:52 | kChordOrderGapMs = 8.0 ("8 ms", "Only where it matters", SC-38) | no | schedule | -
I10b-097 | Room a squeeze press must keep inside its window | replay_schedule.cpp:57 | kRoomMarginMs = 3.0 ("question 2 of ...player-freak-fixes.md") | no | schedule | -
I10b-098 | Window clamps and formula | replay_schedule.cpp:20 | "The window's clamps and formula constants are core/note_window.h's."; JSON "constants" holds window_front_ms, window_back_ms | points at another owner | schedule | -
I10b-099 | hydra_replay options and their meaning | replay.cpp:97-135,835-856 | --chart --rules --out --cap --ms --depth-mode --depth --efill --acts --path --index --ticks --verbose --prodrums --noteshuffle --difficulty --pretty; "--path and --acts cannot both be given"; --index default 0; a typed SqOut offset matched to the nearest chord "within %.0f ms" (kSqueezeWindowMs) | usage text | the user and main session (rule 14 forbids agents) | the caller supplies chart, path, depth
I10b-100 | Where scoring lives | replay.cpp:~32 | "The scoring itself lives in core/replay.h so this tool and tests/test_replay share one implementation." | CLAIMS OWNER (quoted) | replay | -
I10b-101 | Replay JSON field names | replay_json.cpp (202 lines); usage text | "warnings", "sections" (tick, ms, name), "measure" with "label" | usage text | readers of replay JSON | - (not read line by line)

## mutate (mutation tool)
I10b-102 | Time limit for a mutated run | mutate/run.py:86-100 | test_timeout_seconds = baseline_seconds + 5.0; TIME_LIMIT_RULE text; "The user chose it in session 60c3a576 ('own time plus 5s'), for this tool in place of the PIT formula of D82 item 3"; docstring: py_mutate.py "calls this too" | names two readers | run.py, py_mutate.py | -
I10b-103 | hydra_tests exit codes | run.py:71-75 | EXIT_PASSED 0; EXIT_FAILED 1 | no | run.py | -
I10b-104 | Longest command line | run.py:79 | MAX_COMMAND_LINE = 32767 | cites Win32 docs | run.py filter | -
I10b-105 | Switch name, build folder, formats | run.py:7-22,68; hydra_mutant.h | HYDRA_MUTANT env var; MUTANTS_FORMAT = 1; "build-mutation"; run folder <build>/runs/<date-time>; default processes = physical core count | no | the main session builds; run.py drives | the caller may pass --processes
I10b-106 | Rule: the runner never builds | run.py:37 | "The runner never builds and never reads compiler output. A missing build, or one older than its mutants.json, stops it" | no (restates agent rule 14) | run.py | -
I10b-107 | Test-file groups and wait helper home | timeout_split.py:46-62 | WAIT_HOME "tests/wait_util.h"; GROUPS ("red_already","fast","slow","between"); SLEEPS and LOOPS regexes; NOT_FUNCTIONS set | no | timeout_split | -
I10b-108 | First-use table | mutate/first_use_lines.txt (86 lines), first_use_table.md (91 lines) | a generated table of test lines, bugs reached and caught | no | people reading mutation results | -

## test_* files directly in tools/
I10b-109 | Model names and expected prices (test copy of the cost table) | test_agent_cost.py:21-24,99-120 | models "claude-fable-5-1","claude-opus-5-5","claude-sonnet-5-5","claude-haiku-5-5"; for 100k in / 200k out / 1M cache read / 300k 5-minute write: Fable 15.0000, Opus 6.1000, Sonnet 3.1500; Haiku 0.1060 at the 30,000 context limit and 0.5300 one token over; Opus 1,000 five-minute write 0.0050, one-hour 0.0080 | docstring: "The expected dollars are literals worked out by hand from the price table" (table is in tools/agent_cost.py, another scout's file) | the test | -
I10b-110 | Fixture checkout, worktree and branch shapes | test_effort_test_taint.py:18-21 | r"C:\Users\Patrick\Downloads\Hydra\hydra-test"; CHECKOUT + r"\.claude\worktrees\eff-rev-quiet-1"; "claude/eff-rev-quiet-1" | no | the test | -
I10b-111 | Handoff table shape | test_check_handoff.py:226 | columns "Kind | Value | Claim | How to check"; heading "## Claims" | no (copy of what check_handoff.py reads) | the test | -
I10b-112 | Build pass behaviour pinned | test_build_pass.ps1:1-12 | self-test for build_slot.ps1, build_cpp.ps1, cmake_presets.ps1, new_worktree.ps1 ("no longer builds"); fake cmake in a fixture | no | the test | -
I10b-113 | Build folder rule pinned | test_ci_build_folder.ps1:1-15 | a range changing CMakeLists.txt deletes the folder; a changed file gets "now", others the "script's old time" | no (restates ci_build_folder.ps1) | the test | -
I10b-114 | Release signature check | test_release_sign.ps1:1-12 | ECDSA checked against "the committed test public key" and testdata/update/sample.bin.sig; real key kept out of the repo by release_keygen.ps1 | no | the test | -
I10b-115 | compare_db fixture layout | test_compare_db.py:19-154 | stand-in schemas keyed by result_id and profile_id | no (copy of the store layout) | the test | -
I10b-116 | Hydra UI MCP tests | test_hydra_ui_mcp.py (1,125 lines) | not read | - | - | not reached

## Cross-references
I10b-004 sits beside I10b-002 and I10b-005. I10b-015 is one path stated in four files. I10b-054 is stated in two files. I10b-061 and I10b-098 point to Hydra's core/note_window.h. I10b-109 restates agent_cost.py's table. I10b-071 says a C++ test checks the same limits. I10b-058 and I10b-014 and I10b-056 each state RVAs.

## Hook gap noticed
The "a hook refuses ch_key, ch_press and route keys unless the user typed allow key run" claim lives in tool text only; I did not read the hook, so I cannot say whether it covers every route.

## Not reached
ch_mcp: bodies of game_tools.py, kit_tools.py, frida_attach.py, frida_tools.py, bot_run.py, front.py, server.py (read only constants, their comments and some doc lines); dump_facts.py, chdis.py, compare_dumps.py, compare_scorestats.py, method_watch.py, mono_reader.ps1, machine_*.py, trace_hits.py, sp_tools.py bodies; agent/*.ts bodies (kit.ts, press_schedule.ts, trace.ts, menu.ts and the others) beyond their top constants; generated agent/*.js, package files, node_modules; all ch_mcp/tests/*.py (about 30 files); menu_fields.json beyond its head.
ch_probe: debugger.py, engine.py, engine_finder.py, il2cpp_strings.py, input_driver.py, interfaces.py, ocr.py, prep_inputs.py, probe_chart.py, probe_songs.py, process.py, SPEC.md, README.md, ch_probe/tests/, ch_probe/experiments/ (all, including result data); constants.py was read only in part.
capture: all three scripts read. doc_shots: shrink.py read; shots.txt read to about line 30 plus its verb lines.
fake_hydra_live.py lines 60-185: pattern-scanned, not all lines read.
precheck: main.cpp and checks.cpp read; constants and header comments of definitions, items, test_pick, check_5 and decisions read; bodies of check_1 to check_5, check_rows, changed_names, git_text, range, records, test_pick, braces and code_mask not read in full.
replay*.cpp: constants and usage text read; function bodies not read.
mutate: run.py head read; find.py, report.py, rewrite.py, py_mutate.py, first_use.py, timeout_split.py, hydra_mutant.h, listener.cpp and all mutate/test_*.py only pattern-scanned.
test_* in tools/: head and constants only for the ones listed above; test_hydra_ui_mcp.py not read.
