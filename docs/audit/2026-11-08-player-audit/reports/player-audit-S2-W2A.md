# player-audit-S2-W2A: server.py, sp_tools.py, Frida agents (launch, menu, menuhooks, clock, shared), five data files

Agent a5da9bdcc6aa45eaa. Read only. Nothing run. All claims "by reading" unless a run file is named (none was read).
Read in full: tools/ch_mcp/server.py, sp_tools.py, agent/launch.ts, launch_core.ts, menu.ts, menuhooks.ts, clock.ts, hooks_common.ts, gamemanager.ts, state.ts (to see what bundles menu.ts), the five JSON files, the built launch.js (whole), the built state.js bundled menu_fields block and menu section (by grep and ranges), spot checks of menuhooks.js and clock.js. Read in part: frida_tools.py lines 28-147 and 360-490, menu_tools.py 40-370, bot_run.py 505-725, hydra_ui_mcp.py (cap_reply and constants only, 30-60 and 255-290), sp_compare.py 85-130 and 200-370, game_tools.py run_cap_s (171-181, 604).

## How menu.ts is loaded (task item 4)
There is no menu.js and no build:menu script because menu.ts is not an agent of its own. state.ts line 17 imports "./menu.js"; frida-compile bundles it into state.js (state.js's own source list ends "gamemanager.ts", "../menu_fields.json", "menu.ts", "state.ts"). menu_fields.json is also baked into state.js (a `menu_fields_default` object at state.js:3492) AND read live by Python (frida_tools.py:70, :137). gamemanager.ts and hooks_common.ts are bundled the same way into clock.js, trace.js, menuhooks.js and state.js. package.json has only six build scripts (clock, trace, state, menuhooks, kit, launch). Nothing says which of them to rerun when a shared file (gamemanager.ts, hooks_common.ts, menu_fields.json) changes.

## Built .js against source (task item 3)
Built files: clock.js (Oct 30 21:08), kit.js, launch.js (Oct 31 13:17), menuhooks.js (Oct 31 10:14), state.js (Oct 31 11:06), trace.js. Each was built after the last commit that changed behaviour of its sources. Later commits to the sources (b24d9b8f Nov 7, 1eeb70df Oct 31 13:35, 97b34408 Nov 7) only reword comments (read in the diffs). launch.js read whole against launch.ts + launch_core.ts: same functions, same messages, same status fields. state.js: PLAYER_SCREEN_FRAME_RVA 3339664 = 0x32F590 matches; songLoaded, onlineMenuActive, ENUM_KIND, "menu.ts reads no kind named" present; the baked menu_fields_default has the same 15 fields, same keys, same dump lines, same enum words as menu_fields.json (only the "about" text differs, by the "(removed; git show ...)" gloss). menuhooks.js and clock.js: every function name and message string I grepped is present. So today the built code matches the source in behaviour. Nothing guarantees it tomorrow (candidate 1).

## Candidates

1. Nothing refuses a built agent that no longer matches its source (kind 1).
   Homes: frida_tools.py:376-379 (state agent: only `os.path.isfile`), bot_run.py:631 (launch agent: only `os.path.isfile`), trace_hits.py:300 (isfile only), package.json (six separate scripts), tools/ch_mcp/.gitignore line 2 (`agent/*.js`).
   Each .js is git-ignored, so git cannot say what code runs. The loaders only check the file exists. No hash, no mtime compare, no build stamp in the reply.
   Inputs and answers: .js missing -> loud line "run npm run build:x" (good). .js older than its .ts or a shared file (gamemanager.ts, hooks_common.ts, menu.ts, ../menu_fields.json, launch_core.ts) -> runs silently. Edit to menu_fields.json -> Python prints the new enum words and field order at once, while the agent keeps reading the baked field names until `build:state` is rerun (split: two homes of one list, one live and one baked). Fresh worktree -> no .js at all (they live only in the main checkout), so a server started from a worktree fails loudly.
   Split: menu_fields.json live (frida_tools.menu_fields) vs baked (state.js:3492).
   Open: confirm with `npm run build:state` into a scratch copy and diff the bundle; not done (no builds allowed).

2. SpTools reports a song-based run cap that may be the default cap (kind 2, kind 7).
   Homes: sp_tools.py:197-202 versus game_tools.py:171-181 (`run_cap_s`).
   sp_tools catches `bot_run.SongLengthMissing` and answers "no watch started". The real `run_cap_s` catches SongLengthMissing itself and returns `play_chart.RUN_CAP_S`. So for a folder with no length the watch starts with the default cap and the reply line still says "run cap: N s (game_tools.run_cap_s for <folder>)". The `except ... SongLengthMissing` in sp_tools is unreachable for that error. The test fake (tests/test_sp_tools.py:78-80) returns 240.0 for every folder and never raises or falls back, so no test sees either behaviour.
   Inputs: folder with length -> cap from length, line right. Folder with no length -> default cap, line wrongly says it came from the folder. Folder missing -> OSError path, refusal line. song_folder None -> says "no song folder given", right.
   Also sp_tools.py:353-362: `_song_end_ms` swallows the same error and returns None; the gap line then says "song length not known" (honest), but the cap line does not.

3. A reply cut at 12,000 characters drops the end, and several tools put their verdict last (kind 6).
   Homes: hydra_ui_mcp.py:267-289 (`cap_reply` keeps the start, drops the end), server.py:49-51, every tool wrapped in `capped`.
   ch_sp_compare ("one PASS or FAIL line per check"), ch_compare ("six lines, then pass or fail"), ch_notes ("the last line names the since to ask for next", yet CAP_HINT at server.py:46 tells the reader to use since, which is the very line that was cut), ch_run_chart ("first press chord index k at ms" is its last line), ch_stop (restore lines come last) can lose the part that matters. The cut note is the only sign. sp_tools deliberately puts the gap line first for this reason (sp_tools.py:251), but the other tools do not. ch_play_status says "cut from its end, so the file holds every row" but ch_sp_compare has no file.
   Inputs: reply <= 12,000 chars -> whole. Over by one line -> last line(s) dropped, note says how many. One line longer than the cap -> its start only.
   Open: a real long ch_sp_compare report; none read.

4. ch_run_chart cannot choose the dump's path (kind 4 and kind 3; a split between two tools).
   Homes: server.py:390-408 and game_tools.py:1026-1033 ("on the dump's path 0") versus server.py:362-387 (ch_play_chart has `path_index`) and sp_tools.py:286-312 (ch_sp_compare has `path_index`).
   A score JSON written for path k and passed to ch_run_chart with its dump gets its squeeze press times, declined fills and "order" lines from path 0 of the dump. Whether `play_chart.load_dump_chords` checks the dump's path against the score file's chords I did not read (play_chart is out of my file list). If it does not, a path-k run presses squeeze chords at path 0's times.
   Inputs: path 0 -> fine. Path k>0 -> wrong path, unless load_dump_chords refuses. No dump_json -> squeeze-out refused (stated). Open: read play_chart.load_dump_chords; or a run with a path-1 score.

5. ch_open_song's first two steps prove nothing, and the walk never checks instrument, difficulty or the page before Ready (kind 6, kind 8).
   Homes: menu_steps.json steps 1-2, menu_tools.py:290-307 (`_run_step`, `_wait`), docs/clone-hero/07-internals.md IN-79.
   IN-79 (labelled "Proven in a live run") says the profile screen and the main menu both read active from the moment the game opens, and the first Green "changed nothing the reads could see". Step 1 expects profile_screen_active and step 2 expects main_menu_active, so both words already hold before any press: `_wait` sleeps 0.5 s, reads, sees true, and reports "after 0.5 s". A lost press at step 1 or 2 is not caught until step 3 (song_select_active) times out. Step 5 presses Green for Ready with "the player screen's default instrument and difficulty" (menu_steps.json "guess"); menu.ts reads page, instrument and difficulty (menu_fields.json), but no step expects ProDrums or Expert. The final line (menu_tools.py:281-288) says "done: a song is loaded, but its folder is X, not the target" and returns it as an ordinary reply, not a refusal. menu_steps.json `"verified": false` is not read by any code (only a comment, menu_tools.py:117); the tool is registered as normal (server.py:466-475). The file's own "main menu item" step is "guessed to be Quickplay"; IN-79 says the second Green opens the song list, which fits, but the file still says unproven.
   Also menu_tools.py:151-162: a menu class that is "not found" has no onlineMenuActive, so `online_refusal` reads it as closed.
   Inputs: all presses land -> reaches player screen. Press 1 or 2 lost -> not noticed. Wrong default difficulty -> Ready pressed anyway. Wrong song selected -> "done: a song is loaded, but ..." line. Open: live run LS-K3 was struck.

6. The launch agent's "patched" is accepted without the loader having run on the patched byte (kind 6).
   Homes: bot_run.py:712-716 (returns HumanLaunch at state patched), bot_run.py:557-566 (`loader_ran`, shown only by ch_status), launch_core.ts:143-154 (`loaderRan`).
   `launch_human` returns as soon as the agent says "patched" (byte written and read back). Whether the loader then ran and found the new byte ("loader ran after patch: yes/no/not yet") is only a line in ch_status. A "no" (loader ran, bytes different at first run) does not fail the launch. launch_core.ts:151-153 also writes a read failure into `status.reason` while leaving state "patched", so a failed first-run read is a reason on a patched status, not a refusal.
   Inputs: loader_runs 0 -> "not yet"; runs>0 and bytes equal write -> yes; runs>0 and other bytes -> "no" (not enforced).
   Build identity rests on 12 entry bytes and 5 patch bytes only: launch_patch.json "build": "1.1.0.6142" is read by nothing (grep of bot_run.py, launch_core.ts, launch.ts: only `launch`, `module`, `loader_rva`, ... are used). A different build whose bytes happen to match is not told apart; one that differs is refused loudly (good).

7. One build, three spellings, never compared to the installed build (kind 5, kind 8).
   Homes: launch_patch.json `"build": "1.1.0.6142"`; menu_fields.json `"build": "6142"`; start_candidates.json `"game_version": "1.1.0.6142-final"`; menu.ts `PLAYER_SCREEN_FRAME_RVA = 0x32F590` (comment: build 6142); kit_methods.json dump path `...\ch-dumps\6142\dump.cs`; launch_patch.json and kit_methods.json hold the absolute path `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps\6142\dump.cs` (a user path in a checked-in file).
   ch_attach checks only that dumped facts exist for the installed build (frida_tools.py:371-375). menu_fields.json's build is read only by a test (test_frida_tools.py:595). So after a game update with new facts, ch_attach passes, the agent hooks a RVA and obfuscated field names from 6142. Failure there is loud (agent says "has no instance method at RVA ..."; readField returns "error: ..."), but ch_menu then prints a player screen with every field an error, and no tool says "this data is for build 6142".

8. Time conversion and fixed gaps (kind 5).
   - Seconds to ms: `C.s_to_ms` (constants.py:211, used in sp_tools.py:59, :266, frida_tools.py:450) versus `sp_compare._ms` (sp_compare.py:212-213: `row["clock_s"] * 1000.0`), which sp_compare.find_activations, gap_summary, flips_between, ready_between all use. Two homes of one conversion, both multiply by 1000, so no number differs today.
   - Ready level: sp_compare.py:114-122 copies kSpActivationBars/kCloneHeroSpCap from C++ as 2 and 4 ("copied because this script cannot read C++") and compares `amount >= 0.5 * sp_max` as a float. docs/clone-hero/05-star-power.md SP-16 says the game tests that half-bar (+0x1B8, a rounded stored integer, SP-... rounding "tie rule not read") is not above the meter. For an odd sp_max the float half and the game's rounded half can differ by one. Not checked against a run file.
   - GAP_MS = 50 (sp_tools.py:50): "the number LS-O2's hand count used"; user-answered question 7 per the comment; fine, but it is a second home if sp_compare or sp_watch hold their own gap number (I did not grep them).
   - Three clocks in one server: env.now (monotonic) for Frida, kit, menu, method watch; time.time for SpTools' wall_s (server.py:75-77 says why); menuhooks.ts `t_ms` and launch.ts `patched_at_ms` use the agent's Date.now, labelled as such.
   - ATTACH_DELAY_S = 5.0 fixed wait after a launch (frida_tools.py:41), user-approved per the comment.

9. sp_compare.find_activations never sees an activation already on at row 0 (kind 6, side note).
   Homes: sp_compare.py:230-245 (`i = 1`, needs the row before clear). A watch that starts while Star Power is already on, or whose first row has the flag set, reports no first activation and numbers the rest from 1. sp_tools `_activation_line` (sp_tools.py:471-472) would index rows[-1] for flip_row 0, but find_activations never gives 0, so no crash.

10. Small, in scope but outside the nine kinds' main lines.
   - server.py:105-115: `open_items` reads private fields (`kit._agent`, `sp._watch`, `method_watch._session`, `frida._trace`); a rename raises AttributeError at ch_open_items instead of listing open items.
   - sp_tools.py:431-440: a watch thread still alive after READ_CAP_S keeps its game handle and keeps writing the CSV; the reply says it ends with the server. A new ch_sp_watch_start is refused until it ends (WATCH_RUNNING), so it cannot be replaced.
   - sp_tools.py:370: `out` as an absolute path writes anywhere.
   - frida_tools.py:96: a state read with no `songLoaded` key (an older state.js) is shown as "loading", not as an error.
   - menu.ts:230-236 and 242-258: `isLivePlayerScreen` reads the class word at remembered addresses, including memory the collector may have reused; a throw counts as gone, so a successful read of reused memory is compared to the class handle. By reading, no crash path found; a live run would settle.
   - menuhooks.ts hooks "all" of PlayerSelectionMenu's methods including the per-frame method that menu.ts already hooks (two Interceptor hooks on one address); start_candidates.json lists no 0x32F590 in M5, so "all" is by name of class, not by the list.
   - kit_methods.json names match kit.ts ALLOWED_CALLS exactly (40 and 40, set difference empty, checked with a script).

## Focus and keyboard (kind 9)
server.py registers ch_press and ch_key ("after bringing the game window to the front") and ch_play_chart route "keys". The refusal is a hook outside the repo (`allow key run`, C:\Users\Patrick\.claude\gates); the Python in server.py has no check of its own, so with the hook absent nothing in this file stops a window raise. The kit route (ch_kit_press, ch_run_chart, ch_open_song) goes through the agent and needs no window by the server's own text. I found no code in these files that raises the window or sends input to the user's session otherwise. launch.ts/launch_core.ts/clock.ts/menuhooks.ts/menu.ts call only the allowed getters and write one byte via Memory.patchCode.

## Test fakes (kind 7)
- tests/test_sp_tools.py:78-80 FakeGame.run_cap_s never falls back (candidate 2).
- No test in test_server.py/test_frida_tools.py/test_launch_core.py/test_launch_patch.py (grep for state.js, menu_fields, .js) compares a built .js to its source or the baked menu_fields to the file.

## Open (needs a live run or an unread file)
- Does `play_chart.load_dump_chords` refuse a dump path that does not match the score file (candidate 4)?
- A real long ch_sp_compare / ch_notes reply cut at 12,000 (candidate 3).
- Rebuild state.js into a scratch folder and diff against the existing one (candidate 1).
- Whether ch_open_song steps 1-2 hold before their presses on this game build (candidate 5; IN-79 says yes for the active read).

## Not reached
In scope for this task but not read: frida_tools.py lines 148-360 and 490-711, kit.ts, trace.ts, state.ts beyond what imports it (state.ts was read in full), menu_tools.py 1-40 and 370-end, the rest of bot_run.py, front.py, game_tools.py outside run_cap_s, method_watch.py. Not assumed clean.
