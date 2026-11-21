Agent id: ae1adcde13628fd5b (task T5-IR2, record file D5.md, verdict Investigate)

# T5-IR2: Investigate records of D5.md read by the three test rulings

I read all 111 records in D5.md whose verdict is Investigate (the brief says 110; the extra one is 2688, whose verdict sits inline on its check line). The test file is `tools/ch_mcp/tests/test_game_tools.py` (TG below). The answers are short, because one fault covers almost all of them.

## How far the code reading went

I read the test code itself for TG:1-330 (fixtures and rigs), 1360-1469 (the chain rig), 1784-2424, 2433-2662, 2851-2960 and 3128-3328. For the rest (marker tests 2667-2848, recorder tests 2962-3127, probe and dropped-press tests 3342-3555) I read the record's description of each check and the rigs they all use (`ToolRig`, `FakeReg`, `FakePopen`, `_FakeTime`), not the test bodies line by line. Faults marked in those rows rest on the rig, which I read, and on the record's quotes of the checks. I did not run anything.

## What I found

Every test in the file that is not a real-process test runs on fakes the real tool never sees. TG's own header says so: "No game, no keys and no real game files: the launch, the clock and the game are test_bot_run's fakes... live_play is a fake that delivers rows." The pieces, all read in code:

- `FakeQuiet` (TG:150): made-up process list, CPU reading, folder scan, memory and power answers.
- `FakeLivePlay`, `FakeLiveInGame`, `FakeWatch`, `FakeGamePlayer`, `FakeKitPart` (TG:194-274): fake player, fake foreground watch, fake kit.
- `ChainFrida`, `ChainKit`, `ChainSp` (TG:1363-1420): fake game attach, fake kit setup, fake Star Power watch, with typed replies ("setup reply", "watcher ended: stopped").
- `ToolRig` (TG:276): test_bot_run's `Rig` (fake game and clock), `FakeReg` (fake registry key), `FakePopen` (fake recorder process).
- `FAKE.start()` (TG:177): fake dumps stand in for `hydra_replay`.
- `schedule_of` / `SCHEDULE_JSON` (TG:82-96): a hand-built schedule, not `hydra_replay schedule` output. Its `path_source` is typed.

Under the brief's rule, a fake game, fake kit, fake clock, fake reply, fake registry and fake recorder are X9. So I mark X9 on every record that goes through that rig. Three kinds of record escape it. The three `pid_running` / atexit tests (3137, 3166, 3171) use a real process, though 3166 and 3171 test Python, not Hydra. The tests that load a saved fixture file from `tools/ch_probe/tests/` (2183 and 2202 load `freak_limits_schedule.json`, a schedule file from the real tool) still run through the fake game and kit, so they stay X9 on the rig, not on the fixture.

The record file itself says, in its shared header (C-RUN, lines 9-10), that every test runs "on fakes (fake game, fake kit, fake clock, temp folders)" and that the claim is "not checked". So the record names X9 in a general way for all of them. I count that as "named (shared)". The records do not call it a fault; they give Investigate "claim not checked only". I mark a record "named (own)" only when its own text calls out the fake or typed value for that check.

X10: the check requires a typed product message, label or column list. I do not mark checks that read a constant by name (for example `GT.PLAY_ALREADY_RUNNING` at TG:1937, `B.HUMAN_NEEDS_SONG` at 2338), as the brief says.

X11: a value typed into the test (an exception text, a pid, a name) required back unchanged in the reply.

X12: a check whose comment names its source as a plan, run or person's chart.

Snapshot: none beyond what Rewrite records already hold, which are not mine.

## Table

Codes: SH = X9 named by the record's shared header only; OWN = the record's own text names the fake or typed value. Rig-only means the fault is the fake rig, with no further fault.

| Record (TG line) | Faults | Faulty checks | Reason | Record named |
|---|---|---|---|---|
| 1784 first press returns | X9 | TG:1787-1795 | fake player, fake game, fake clock; reply from chain fakes | SH |
| 1799 decline reaches chords | X9, X10 | TG:1801-1805 | hand-built decline files and schedule; typed "declined: chords 1 (kick)" | SH; OWN for the typed line |
| 1848 failed attach | X9, X10 | TG:1849-1853 | fake attach; typed "the game is not running", regex of the stop wording | SH |
| 1855 failed setup | X9, X10 | TG:1856-1861 | fake kit returns None and "setup reply"; typed wording | SH |
| 1863 failed watch | X9, X10 | TG:1864-1870 | fake Star Power watch with typed lines | SH |
| 1872 no-press player ends | X9, X10, X11 | TG:1873-1881 | fake player returns typed message "no song"; wording typed; "no song" required back | SH |
| 1883 stops the watch too | X9, X10 | TG:1884-1890 | same fake player; typed "watcher ended: stopped" (typed by the fake itself) | SH |
| 1905 search timeout | X9, X10, X11 | TG:1906-1914 | engine search replaced by a raiser; "no playing song found in 120 s" typed in and required back; "normal-mode engines only" typed | SH |
| 1916 player raises | X9, X11 | TG:1917-1923 | fake player raises typed ValueError text; reply must hold it | SH |
| 1925 refused while running | X9 | TG:1926-1940 | fake play held open by an Event; expected is the constant by name (not X10) | SH |
| 1942 no quiet check | X9 | TG:1943-1949 | `FakeQuiet` and a patched `quiet_report`; checks calls on mocks | SH |
| 1981 plays in game | X9 | TG:1982-1988 | fake in-game player and fake kit | SH |
| 1997 status rows | X9, X10, X11 | TG:2001-2017 | hand-built rows (3.0, 2.9, "retime_missed") required back in a typed header and rows | SH; OWN ("test's own input values") |
| 2021 wrong folder loaded | X9, X10 | TG:2023-2030 | fake game reports a typed folder; typed sentence | SH |
| 2032 no song / failed read | X9, X10 | TG:2034-2041 | fake read raises typed `StateUnread("not attached")`; typed words | SH |
| 2043 song_folder mismatch | X9, X10 | TG:2046-2050 | hand-built schedule; typed refusal | SH |
| 2052 no chart / relative chart | X9, X10 | TG:2054-2064 | hand-built schedule doc with typed chart value; typed wording | SH |
| 2066 path_index | X9, X10, X11 | TG:2069-2091 | typed `path_source` index 2 in a hand-built schedule, and the refusal says "path 2" back; typed wording | SH |
| 2093 builds-N.json | X9, X10, X11 | TG:2095-2106 | typed build string "Nov  8 2026 17:56:03" required back; typed agent file names "state.js", "kit.js" | SH; OWN ("input echoed") |
| 2152 probe at its lead | X9 | TG:2121-2166 | rows hand-built from `press_plan`; lead 84 typed; fake player | SH |
| 2176 probe chord absent | X9 | TG:2137-2181 | expected text rebuilt by calling the producer (`edge_probe.probe_chords`), on a hand-built schedule | SH; OWN (copy of the producer) |
| 2183 probe on SP-end chord | X9, X12 | TG:2184-2191 | fake rig; comment cites "test_edge_probe's squeeze case" and Freak chord 387 from a saved schedule file; expected again from the producer | SH; OWN (not checked on a chart) |
| 2193 probes file refused | X9 | TG:2195-2200 | expected built from the producer's own exception text; fake rig | SH; OWN (copy) |
| 2202 no probes | X9 | TG:2206-2212 | expected chords from the same loader the code uses; fake rig | SH; OWN (copy) |
| 2214 no parts | X9, X10 | TG:2215-2217 | `ToolRig` fake; typed "refused:" | SH |
| 2219 foreground line | X9, X10 | TG:2220-2224 | `FakeWatch` builds its own line; typed sentence | SH; OWN (the fake builds the line) |
| 2242 ch_start order | X9, X10 | TG:2243-2264 | fake setter, fake launch, fake watch; typed settings sentence | SH |
| 2266 line stays after stop | X9, X10, X12 | TG:2227-2275 | typed `USER_SETTINGS` text, not a real settings.ini; comment cites "the user's choice of 2026-11-13" | SH |
| 2277 file without the line | X9, X10 | TG:2278-2284 | `ORIGINAL` fake settings file; typed warning sentence | SH |
| 2286 count so far | X9, X10, X11 | TG:2287-2298 | `watch.count = 2` set on the fake and required back in "2 times so far" | SH; OWN ("test's input set on the fake") |
| 2300 adopted starts no watch | X9 | TG:2301-2304 | fake rig | SH |
| 2306 registry key backup | X9 | TG:2310-2314 | `FakeReg` stands in for the registry; "the game changed this" typed | SH |
| 2335 human, no song | X9 | TG:2336-2342 | fake launch agent; expected is a constant by name; "runs only with --song" typed (X10 on TG:2339) | SH |
| 2344 human backup first | X9, X10, X11 | TG:2345-2359 | fake process start and fake launch agent; "4242" is the fake's pid; typed events list | SH; OWN (the fake's pid) |
| 2361 loader line three ways | X9, X10 | TG:2362-2378 | typed agent status dicts and byte strings; typed sentences | SH; OWN (fixtures) |
| 2380 four launch states | X9, X10 | TG:2382-2399 | typed agent status dicts and typed expected sentences | SH; OWN (typed texts) |
| 2402 status fails | X9, X11 | TG:2406-2410 | fake agent raises typed "session detached: process-terminated", required back | SH |
| 2413 detach before stop | X9, X10 | TG:2414-2421 | fake agent and fake game; typed lines | SH |
| 2423 failed human launch | X9, X10 | TG:2423-2430 (record) | fake agent refuses; typed ending line | SH |
| 2473 marker names pid and folder | X9 | TG:2474-2477 | fake rig; real marker file in a temp folder (not a fault) | SH |
| 2479 marker before setting and launch | X9 | TG:2480-2491 | patched setter and launch | SH |
| 2493 human start marker first | X9 | TG:2494-2504 | fake launch agent, fake process start | SH |
| 2506 ensure_backup marker | X9 | TG:2507-2510 | fake rig | SH |
| 2512 failed restore keeps marker | X9 | TG:2516-2523 | `restore_all` replaced by a raiser | SH |
| 2525 stop raises | X9 | TG:2528-2530 | `finish_run` replaced by a raiser | SH; OWN (patched) |
| 2532 second ch_start | X9 | TG:2533-2537 | fake rig | SH |
| 2539 marker cannot be deleted | X9, X10, X11 | TG:2542-2546 | `os.remove` replaced by raiser with typed "locked"; typed sentence | SH; OWN (copy of the f-string) |
| 2548 marker cannot be written | X9, X10, X11 | TG:2549-2554 | `os.rename` raiser, "locked" echoed; typed sentence | SH |
| 2558 launch fails | X9, X10, X11 | TG:2559-2567 | fake launch raises typed "no such game"; required back as last line | SH |
| 2569 failed human launch | X9 | TG:2570-2572 | fake agent status `{"state": "refused"}` | SH |
| 2579 restore fails too | X9, X10, X11 | TG:2582-2600 | raisers, typed "no such game" and "locked"; typed sentence | SH |
| 2614 older marker backup | X9, X10, X12 | TG:2615-2630 | manifest and backup folder hand-made by the test ("the shape of a backup the tools once took"); typed "left as the game saved it" | SH; OWN (hand-made fixture) |
| 2632 score file kept | X9 | TG:2633-2636 | typed file text "the user played after the backup" echoed | SH |
| 2638 live stop leaves files | X9, X11 | TG:2639-2643 | writes "changed by the game" and requires it back | SH |
| 2653 worker exit leaves backup | X9, X11 | TG:2654-2665 | same typed file text echoed; typed lines | SH |
| 2667 next worker adopts | X9, X10 | record 2673-2686 | fake registry, stamp patched, fake pid test; typed sentence "3 hours old" | SH |
| 2688 adopting restores nothing | X9, X11 | record 2693 | typed file text echoed | SH |
| 2695 adopted key restored | X9 | record 2699-2700 | `FakeReg` | SH; OWN (FakeReg) |
| 2702 refuses on marker backup | X9, X10 | record 2707-2714 | fake pid test, typed refusal, "song length" as a rig side effect | SH; OWN (stand-in) |
| 2716 adopted restore fails | X9, X10, X11 | record 2725-2731 | raiser typed "locked" echoed | SH |
| 2733 backup vanishes | X9, X10 | record 2738 | manifest removed by hand; typed "restoring failed: FileNotFoundError" | SH |
| 2741 no manifest | X9, X10 | record 2745-2748 | hand-made folder; typed "holds no", "nothing to restore" | SH |
| 2752 running pid not adopted | X9, X10 | record 2757-2761 | fake pid test says "running"; typed by-hand command copied from `bot_run._restore_hint` | SH; OWN (copy) |
| 2763 adopted nothing | X9, X10 | record 2766-2769 | fake rig; typed "nothing to restore" | SH |
| 2771 second worker's marker | X9, X10 | record 2775-2779 | fake rig; typed text | SH |
| 2783 gone folder | X9, X10, X11 | record 2790-2795 | typed pid 77 required back in the sentence | SH |
| 2799 unreadable marker | X9, X10 | record 2814-2820 | twelve made-up marker contents; typed sentence | SH; OWN (free inputs) |
| 2822 folder at marker path | X9, X10 | record 2825-2827 | folder made by the test; typed status text | SH; OWN (case source not named) |
| 2829 unreadable marker kept | X9, X10 | record 2833-2835 | made-up marker file; typed text | SH |
| 2837 pid test raises | X9, X10, X11 | record 2845-2848 | pid test replaced; "OSError: no answer" typed in and required back | SH |
| 2900 ch_start order | X9, X11 | TG:2901-2925 | `FakeQuiet`, `FakePopen`, fake game pid "4242" required back in the event list, patched sleep | SH; OWN (fake's pid) |
| 2927 not quiet | X9 | TG:2928-2930 | `FakeQuiet(cpu=limit+1)`; limit read by name (not X10) | SH |
| 2932 reply names recorder | X9, X10, X11 | TG:2933-2935 | fake popen pid 777 required back; typed sentence | SH |
| 2953 recorder fails to start | X9, X11 | TG:2954-2960 | fake popen raises typed "no python"; expected line built by producer | SH; OWN (producer copy) |
| 2962 start raises | X9, X10, X11 | record 2969-2970 | fake raises typed "broken"; typed sentence | SH |
| 2972 pid line unwritable | X9, X10, X11 | record 2980-2981 | fake pipe error "the pipe is closed" typed and required back | SH |
| 2983 launch fails, recorder stopped | X9, X10 | record 2991-2994 | fake popen, fake launch failures; typed "stopped, pid N" | SH |
| 2996 second ch_start | X9 | record 3000-3001 | `FakePopen` return codes | SH |
| 3005 engine goes out in order | X9, X11 | record 3012-3013 | the rig's typed ENGINE address (TG:2855) required back | SH; OWN (rig's address) |
| 3015 samples line | X9, X10 | record 3021-3022 | hand-written samples file (1000.25, 1000.27); typed sentence | SH; OWN (hand-written) |
| 3024 wrote nothing | X9, X10 | record 3029-3030 | fake recorder; typed "no samples yet in" | SH |
| 3032 never started | X9 | record 3038-3039 | producer-built expected line, fake popen | SH; OWN (producer copy) |
| 3041 engine write fails | X9, X10 | record 3047-3050 | fake pipe; typed sentence; expected first-press line from `GT.first_press_line(ROWS)` on typed ROWS | SH; OWN (producer copy) |
| 3054 stops close stdin | X9, X10 | record 3062-3066 | `FakePopen` events; typed "stopped, pid N, exit code 0" | SH |
| 3068 hung recorder terminated | X9, X10 | record 3076-3080 | fake popen that ignores stdin; typed sentence naming the constant | SH |
| 3082 recorder ends before hooks | X9 | record 3086 | fake event list | SH |
| 3088 stop raises | X9, X10, X11 | record 3096-3097 | typed "RuntimeError: broken" echoed; typed registry line | SH |
| 3099 open lines | X9, X10, X11 | record 3101-3103 | fake pid 777 required back in typed line | SH; OWN (pid typed in) |
| 3105 status recorder line | X9, X10 | record 3106-3108 | typed "machine recorder: none" and running line | SH |
| 3110 exited by itself | X9, X10 | record 3116-3121 | fake popen exits with typed code 3; typed sentence | SH |
| 3123 no recorder, no run | X9, X10 | record 3124 | typed whole reply; fake rig | SH |
| 3137 pid no process holds | none | TG:3138 | real Windows process API, real call; the number 0x7FFFFFFC is a picked input with no written reason (record already says this) | the record names it |
| 3166 terminated runs no atexit | none | TG:3169 | real child process; checks Python, not Hydra code; no X9 to X12 fault (the record raises the question already) | the record names it |
| 3171 normal exit runs atexit | none | TG:3174 | same, a control for 3166 | the record names it |
| 3203 anchor line | X9, X10 | TG:3195-3207 | hand-built squeeze dict ("numbers are test inputs"); typed expected line | SH; OWN (hand-built, free inputs) |
| 3209 late anchor | X9, X10 | TG:3210-3216 | same hand-built dict; typed line | SH; OWN |
| 3218 squeeze from its own end | X9, X12 | TG:3219-3240 | hand-built "snippet of the Pitch Dark schedule" (aim, end, conversion typed), chord time hand-picked so the answer is 15; comment cites "the plan's numbers" | SH; OWN (hand-picked) |
| 3242 no timing no anchor | X9 | TG:3243-3245 | hand-built squeeze dict | SH; OWN (not on a real chart) |
| 3261 end line | X9, X10 | TG:3262-3269 | hand-built rows and planned-ends map; typed lines; 3.1 later from typed inputs | SH; OWN (free inputs) |
| 3271 no new columns | X9 | TG:3272-3273 | typed ROWS fixture | SH |
| 3275 points between presses | X9, X12 | TG:3276-3284 | hand-built rows; comment says they are "Bomber's ... press-2.csv of the ls-r2ab-run1 run as the plan reads it" (not read in the test); "granted" rule typed | SH; OWN (file not opened) |
| 3286 split declined chord | X9 | TG:3287-3293 | hand-built rows | SH |
| 3295 status carries lines | X9, X10 | TG:3296-3315 | hand-built rows, hand-set "planned 1000" read; typed sentences | SH; OWN (hand-set) |
| 3342 probe at lead | X9, X10 | record 3345-3354 | fake kit player rows; typed "probes: chord 1 60 ms late" and status line | SH |
| 3370 probe press log | X9, X10 | record 3374-3375 | fake session folder; typed "press-1.csv" name | SH |
| 3377 probe chord absent | X9, X10 | record 3380-3382 | fake rig; typed refusal | SH |
| 3384 probe needs kit | X9, X10 | record 3386-3388 | `ToolRig` without kit; typed refusal | SH |
| 3424 dropped row | X9, X10 | record 3429-3433 | hand-built rows with a "dropped" row; typed "sent 1", "dropped: chords 153" | SH |
| 3443 split chord dropped | X9, X10 | record 3449-3450 | hand-built rows (index 153 twice); typed lines | SH |
| 3452 no dropped row | X9, X10 | record 3455 | hand-built rows; typed "dropped: none" | SH |
| 3543 900 ms block drops none | X9 | record 3545-3548 | `_FakeTime`, `_FakeGame`, `_FakeSongClock`, a blocking row writer; 0.9 s cited from "a past run's stall" in a comment (X12 for the 0.9, from LS-A2 run 1 per the record) | SH; OWN (fake clock) |

## Counts

- Investigate records read: 111 (110 begin their verdict line with "Investigate"; 2688 carries it inline).
- Records with at least one fault: 108. The three with none are 3137, 3166 and 3171.
- Records per fault. X9: 108. X10: 68. X11: 27. X12: 5 (2183, 2266, 2614, 3218, 3275; 3543 cites a past run for its 0.9 s but I did not mark it). Snapshot: 0 beyond what the Rewrite records hold.
- Records whose fault the record already named: X9 is named in every one of the 108 through the shared header (C-RUN), as "fakes, claim not checked", not as a fault. 35 name a specific fake or typed value in their own text (marked OWN). No record names X10 or X11 as a fault; where they say "typed" they describe it, as with T5-KR1.

Counted from the table rows. X10 and X11 marks on tests I read only through their record (TG:2667-2848, 2962-3127, 3342-3555) are the weakest.

## After the user's ruling on hand-built input

The main session sent the ruling that input a test builds and feeds to the real code is not fake (hand-built charts, settings.ini files, notes, throwaway repos, fuzz bytes). Only a stand-in for the code's own output, a stand-in for outside data, or fake code is X9. I re-checked every X9 mark against it. No mark is cleared, and the count stays 108, because each row stands on one of three things:

- Every row that goes through `ToolRig` stands on fake code: `Rig` (fake game and clock), `FakeReg`, `FakePopen`, `FakeWatch`, `FakeLivePlay`, `ChainFrida`, `ChainKit`, `ChainSp`, `FakeQuiet`.
- The rows with no rig stand on a stand-in for the code's own output. 3203, 3209, 3218 and 3242 hand-build the squeeze and schedule data `hydra_replay schedule` writes. 3261, 3271, 3275, 3286, 3424, 3443 and 3452 hand-build press-log rows, which are the player's output. 3015 hand-writes the recorder's samples file, which is the recorder's output.
- The schedules built by `schedule_of` (TG:82) stand in for `hydra_replay schedule` output. They are not input to the engine, so the rule clears none of them.

What the ruling changes is the reason on some rows. For these, the hand-made file the real code reads is not the fault, and the X9 rests only on the fake rig around it: 2266 and 2277 (settings.ini text), 2614, 2733, 2741, 2799, 2822 and 2829 (hand-made backup folders, manifests and marker files read by the real restore and marker code). Read those rows as X9 on the rig alone. The typed settings text, marker bytes and manifests are not counted as faults. The rows 3137, 3166 and 3171 were already unmarked.

## The five clearest

1. 2093 (builds-N.json): types the build string "Nov  8 2026 17:56:03" into a hand-built schedule and requires it back, and the agent file names are typed.
2. 3218 (squeeze from own end): a hand-built "Pitch Dark snippet" with a chord time picked so the answer comes out 15, and a comment citing the plan's numbers.
3. 3275 (points between presses): hand-built rows whose comment names a real run's press log the test never opens.
4. 2614 (older backup): the manifest and backup folder are built by hand in the test from "the shape of a backup the tools once took".
5. 1905 (search timeout): the engine search is replaced by a raiser, its text typed in and required back, plus a typed suffix.
