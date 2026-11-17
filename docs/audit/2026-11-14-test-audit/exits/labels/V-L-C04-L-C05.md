Agent id: a140f6c198633e6de (task T8-E3-V-L-C04-L-C05, session f3bbb32b)

Checked against the batch tsvs, `keys.md` (revision 1), the counters' reasons, and the tests where the reason did not settle a key: `tools/ch_mcp/tests/test_bot_run.py` (lines 66-1170), `test_hydra_end.py` (37-210), `test_compare_scorestats.py` (20-194), `test_kit_tools.py` (155-560), and the A23 record at 266-320 for the dump rows. The `run` column was not used.

## Counts

| batch | rows in batch | lines in label file | rows I agree with | rows I change |
|---|---|---|---|---|
| L-C04 | 73 | 73 (every row once, same ids; checked by sorted diff) | 36 | 37 |
| L-C05 | 76 | 76 (every row once, same ids; checked by sorted diff) | 56 | 20 |

Every fault mark on a count row (X9, X10, X11, X12, snapshot) has a key of its family in both files after my changes. One was missing before: L-C05 `counts/T5-IR14.md:102` carries X11 and had no E-echo.

## Settlements of the scouts' doubts

1. A "FAKE dump" in `test_bot_run.py` and `test_hydra_end.py` is `FakeDumps`, which replaces `hydra_end.make_dump`, the function that runs `hydra_replay`. That is "a fake analyzer" (C-analyzer, keys.md). The dump's content is also a hand-built result, so F-result stays where the content is what the test checks. Rows 13, 22, 23, 26, 57, 58 and 59 of L-C04 and rows 18 and 19 of L-C05 get C-analyzer. In L-C05 rows 18 and 19 it replaces C-stub, because `FakeDumps` is the fake and not a stub program.
2. Fake agent against fake game. `FakeLaunchAgent` and the hand-written `ReplySession` replies carry the game's patch bytes, Rewired maps and statuses, so they stay O-gamemem (keys.md: "the game's method calls or messages"). `FakeGame`, `FakePopen`, `FakeReg`, `StuckGame` and `FakeFridaTools` stand in for programs and handles, so they stay C-stub. I did not change that split. Two L-C04 rows use the fake agent but it never speaks (see the table), so O-gamemem comes off them.
3. L-C04 `counts/T5-IR13.md:67` has no joined record. I opened the test (`test_compare_scorestats.py:41-79, 144-156`). `TARGET` is a hand-built Hydra dump, `_target_stats` is hand-built scorestats with 1095, 222935, 1229730 in it, and the comment cites "the numbers LS-O5's pass line names". So F-result, O-scorestats, Z-cite and S-snapshot are right. I add F-schedule because the dump is replay JSON fed to the reader (see 5).
4. L-C05 rows 46 to 64 as O-gamemem: kept. The replies are hand-written Rewired state (kit id, maps, keyboard_now, bindings) that the kit agent would read from the game. Hydra's own tool output is not what they hold. Rows 50, 51, 53, 54, 55, 57, 58 and 59 call the pure functions `devices_problem`, `binding_problems` and `map_problems` on a hand-built reply, so they carry no fake object. They keep O-gamemem alone.
5. Dump text. A hand-written dump read by a reader of the replay tool's JSON fits F-schedule ("schedules and replay JSON fed to the replay tool's readers"). L-C05 rows 16 and 18 and 107 already had it. L-C04 rows 66, 67, 68 (compare_scorestats reads a Hydra dump) and 92, 93 (`windows_from_json` reads a hand-written dump, record A23.md:266-320) get it too. F-result stays on all of them.
6. Rig. In `test_bot_run.py` every test in `BotRunTest`, `RunStepsTest`, `LaunchStepsTest`, `HumanLaunchTest` and `HumanStartRunTest` builds `Rig`, and the L-C04 scout keyed it only on rows whose reason said "Rig". I add C-rig to every row whose test builds a Rig. In `test_kit_tools.py` the class `Rig` builds `FakeGame`, `FakeFridaTools`, `FakeClock` and a fake Frida session. L-C05 had no C-rig at all, so I add it to the rows that call `self.tools(...)`.

## Rows I change

| batch | row | scout's keys | your keys | why |
|---|---|---|---|---|
| L-C04 | counts/T5-IR13.md:13 | C-rig, C-stub, C-clock, F-result, P-other | C-rig, C-stub, C-clock, C-analyzer, F-result, P-other | the reason says Rig fakes the dump, and `FakeDumps` replaces `make_dump` (test_hydra_end.py:37-61) |
| L-C04 | counts/T5-IR13.md:16 | C-clock, C-stub, O-scorestats, P-message, E-echo | C-rig, C-clock, C-stub, O-scorestats, P-message, E-echo | the test builds a Rig (test_bot_run.py:205) |
| L-C04 | counts/T5-IR13.md:17 | C-stub, P-message | C-rig, C-stub, P-message | builds a Rig (:222) |
| L-C04 | counts/T5-IR13.md:18 | C-stub, P-message, P-other | C-rig, C-stub, P-message, P-other | builds a Rig (:252) |
| L-C04 | counts/T5-IR13.md:19 | C-stub, P-message | C-rig, C-stub, P-message | builds a Rig (:264) |
| L-C04 | counts/T5-IR13.md:22 | F-result, E-echo | C-rig, C-analyzer, F-result, E-echo | the "FAKE dump" is `FakeDumps` and the test builds a Rig (:308-312) |
| L-C04 | counts/T5-IR13.md:23 | F-result, P-message | C-rig, C-analyzer, F-result, P-message | same, with a null dump (:316-320) |
| L-C04 | counts/T5-IR13.md:26 | F-result, E-echo | C-rig, C-analyzer, F-result, E-echo | the counted fake dump is `FAKE.charts`, and the test builds a Rig (:397-403) |
| L-C04 | counts/T5-IR13.md:29 | O-scorestats, E-echo, P-message | C-rig, O-scorestats, E-echo, P-message | builds a Rig (:423) |
| L-C04 | counts/T5-IR13.md:30 | C-stub, P-message | C-rig, C-stub, P-message | builds a Rig (:432) |
| L-C04 | counts/T5-IR13.md:31 | C-stub, P-message | C-rig, C-stub, P-message | builds a Rig (:443) |
| L-C04 | counts/T5-IR13.md:32 | C-stub, P-message | C-rig, C-stub, P-message | builds a Rig (:450) |
| L-C04 | counts/T5-IR13.md:34 | C-stub | C-rig, C-stub | builds a Rig (:496) |
| L-C04 | counts/T5-IR13.md:35 | C-stub, P-message | C-rig, C-stub, P-message | builds a Rig (:510) |
| L-C04 | counts/T5-IR13.md:36 | C-stub, P-message | C-rig, C-stub, P-message | builds a Rig (:529) |
| L-C04 | counts/T5-IR13.md:37 | C-stub, P-message | C-rig, C-stub, P-message | builds a Rig (:540) |
| L-C04 | counts/T5-IR13.md:40 | O-gamemem, P-other, E-echo, Z-cite | C-rig, O-gamemem, P-other, E-echo, Z-cite | `HumanLaunchTest.setUp` builds a Rig (:697) and `wire_human` fakes the seams |
| L-C04 | counts/T5-IR13.md:41 | O-gamemem, P-message, Z-cite | C-rig, O-gamemem, P-message, Z-cite | same |
| L-C04 | counts/T5-IR13.md:42 | C-stub, P-message | C-rig, C-stub, P-message | same |
| L-C04 | counts/T5-IR13.md:43 | O-gamemem, P-message, E-echo | C-rig, O-gamemem, P-message, E-echo | same |
| L-C04 | counts/T5-IR13.md:44 | O-gamemem, P-message | C-rig, O-gamemem, P-message | same |
| L-C04 | counts/T5-IR13.md:45 | O-gamemem, P-other | C-rig, O-gamemem, P-other | same |
| L-C04 | counts/T5-IR13.md:46 | O-gamemem | C-rig, O-gamemem | same |
| L-C04 | counts/T5-IR13.md:47 | O-gamemem, P-other | C-rig, O-gamemem, P-other | same |
| L-C04 | counts/T5-IR13.md:48 | O-gamemem, E-echo | C-rig, O-gamemem, E-echo | same |
| L-C04 | counts/T5-IR13.md:49 | C-rig, O-gamemem, P-message, P-name | C-rig, P-message, P-name | the fake agent is passed but never called (`self.events == []`, :874), so no game data is typed |
| L-C04 | counts/T5-IR13.md:54 | C-rig, O-gamemem, P-message | C-rig, P-message | same: the agent is never called (`self.events == []`, :1007) |
| L-C04 | counts/T5-IR13.md:56 | O-gamemem, P-message | C-rig, O-gamemem, P-message | `HumanStartRunTest.setUp` builds a Rig (:986) |
| L-C04 | counts/T5-IR13.md:57 | C-stub, F-result, P-message | C-stub, C-analyzer, F-result, P-message | `InstallChartTest.setUp` sets the fake dump (:1048) |
| L-C04 | counts/T5-IR13.md:58 | C-stub, F-result, E-echo | C-stub, C-analyzer, F-result, E-echo | same |
| L-C04 | counts/T5-IR13.md:59 | C-stub, P-name | C-stub, C-analyzer, P-name | same setUp feeds the fake dump to `install` |
| L-C04 | counts/T5-IR13.md:66 | F-result, O-scorestats, P-message, Z-cite | F-result, F-schedule, O-scorestats, P-message, Z-cite | `HYDRA` is a hand-written replay dump fed to the reader (test_compare_scorestats.py:32-39) |
| L-C04 | counts/T5-IR13.md:67 | F-result, O-scorestats, Z-cite, S-snapshot | F-result, F-schedule, O-scorestats, Z-cite, S-snapshot | `TARGET` is the same kind of dump (:43-50); the other four keys are confirmed from the test |
| L-C04 | counts/T5-IR13.md:68 | F-result, O-scorestats | F-result, F-schedule, O-scorestats | same dump |
| L-C04 | counts/T5-IR13.md:92 | F-result, E-echo | F-result, F-schedule, E-echo | a hand-written dump read by `windows_from_json` (A23.md:266-275) |
| L-C04 | counts/T5-IR13.md:93 | F-result, S-snapshot | F-result, F-schedule, S-snapshot | a hand-written old dump fed to the same reader (A23.md:306-320) |
| L-C04 | counts/T5-IR13.md:116 | F-result, P-pbticks | F-result, P-setting | " (optimal)" and " (best all-0)" are label words on a button, not fill ticks or clock text |
| L-C05 | counts/T5-IR14.md:18 | F-schedule, C-stub | F-schedule, C-analyzer | `FakeDumps` is a fake of the code that runs `hydra_replay` (test_hydra_end.py:37-61) |
| L-C05 | counts/T5-IR14.md:19 | C-stub, P-message, E-echo | C-analyzer, P-message, E-echo | the failure text is `FakeDumps`' own string (test_hydra_end.py:55-57, 179-183) |
| L-C05 | counts/T5-IR14.md:42 | C-stub | C-rig, C-stub | test_kit_tools.py:328 uses the `Rig` base and `self.tools(...)` |
| L-C05 | counts/T5-IR14.md:43 | C-reader, P-message | C-rig, C-reader, P-message | same (:333) |
| L-C05 | counts/T5-IR14.md:44 | C-reader, P-message | C-rig, C-reader, P-message | same (:339) |
| L-C05 | counts/T5-IR14.md:45 | C-stub, P-message | C-rig, C-stub, P-message | same (:345) |
| L-C05 | counts/T5-IR14.md:46 | O-gamemem, P-message | C-rig, O-gamemem, P-message | same (:359) |
| L-C05 | counts/T5-IR14.md:47 | O-gamemem, P-message | C-rig, O-gamemem, P-message | same (:380) |
| L-C05 | counts/T5-IR14.md:48 | O-gamemem, P-message | C-rig, O-gamemem, P-message | same (:392) |
| L-C05 | counts/T5-IR14.md:49 | O-gamemem, P-message | C-rig, O-gamemem, P-message | same (:406) |
| L-C05 | counts/T5-IR14.md:52 | O-gamemem, P-message | C-rig, O-gamemem, P-message | same (:441) |
| L-C05 | counts/T5-IR14.md:56 | O-gamemem, P-message | C-rig, O-gamemem, P-message | same (:481) |
| L-C05 | counts/T5-IR14.md:60 | C-clock, O-gamemem, P-message | C-rig, C-clock, O-gamemem, P-message | same (:515) |
| L-C05 | counts/T5-IR14.md:61 | O-gamemem | C-rig, O-gamemem | `SetupForRunTest` extends `Rig` (:524-531) |
| L-C05 | counts/T5-IR14.md:62 | C-stub | C-rig, C-stub | same (:534) |
| L-C05 | counts/T5-IR14.md:63 | O-gamemem, P-message | C-rig, O-gamemem, P-message | same (:539) |
| L-C05 | counts/T5-IR14.md:64 | O-gamemem | C-rig, O-gamemem | same (:546) |
| L-C05 | counts/T5-IR14.md:54 | O-gamemem, P-message | O-gamemem, P-other | the test asserts a typed element-id list and no message (test_kit_tools.py:461-467) |
| L-C05 | counts/T5-IR14.md:100 | O-reply, P-message | O-reply, P-message, P-other | the counter says "500 ms typed" as well as "cancelled"; the 500 is a typed number, not a message |
| L-C05 | counts/T5-IR14.md:102 | O-reply, P-message | O-reply, P-message, E-echo | the counter marks X11 and the row had no key of its family; the reason says the made-up body's ids are typed |

That is 37 rows for L-C04 and 20 for L-C05, the L-C05 count being the 15 C-rig rows plus 18, 19, 54, 100 and 102.

## The ends tables after my changes

L-C04: the P-pbticks line (both ends at row 116) is wrong now. No row in the batch carries P-pbticks, and P-setting takes row 116 as its least-fitting row (a screen word on a button). O-gamemem's least-fitting row 49 no longer carries O-gamemem; use row 55 (`counts/T5-IR13.md:55`) or 56. C-analyzer gains rows 13, 22, 23, 26, 57, 58 and 59, with 74 still the plainest and 76 the least fitting. F-schedule is new in the batch: plainest 93, least fitting 66. The other ends rows are in the batch and carry their keys (checked one by one).

L-C05: C-stub's plainest row 62 and least-fitting row 17 still carry it. C-analyzer is new: plainest 18, least fitting 19. C-rig is new: plainest 42, least fitting 60. P-other's ends (38 and 101) still carry it. Every other ends row is in the batch and carries its key.

## Side notes

- `docs/audit/2026-11-14-test-audit/exits/labels/L-C04.md:107` and `L-C05.md:112`: both scouts guessed O-gamemem against C-stub or O-payload for the fake agents and the Rewired replies. I settled it above. Other batches may repeat the split, so the main session may want to check the same rows there.
- keys.md has no key for a fake registry (`FakeReg`, `test_bot_run.py` and `test_game_files.py:154`). I kept the scout's C-stub, which fits it loosely. A fake Frida session (`FakeFrida`/`ReplySession`, `test_frida_attach.py:159-448`) would fit C-session ("a fake session") better than the O-gamemem-only labelling on rows that call `self.tools(...)`. I did not add C-session, because that is a wider change than the scouts' rows.
- L-C04 `counts/T5-IR13.md:60`: the counter's reason is cut off mid-sentence in the tsv ("...but that is this test's helper, not the "). I kept the scout's P-name, which matches the visible text. Open the count file for the full sentence before the main session picks an exit.
- L-C05 `counts/T5-IR14.md:37`: the typed `setup_message` dict (test_kit_tools.py:285-288) is a request payload, not a printed message. I left P-message, because the question for the user would be the same (the test types what the code builds), but P-other fits at least as well.
- `tools/ch_mcp/tests/test_bot_run.py:1046-1048`: the setUp comment says song.ini's song_length lines are "only bytes the copy keeps". The test still feeds a fake dump to the real `install`, so rows 57 to 59 belong under C-analyzer as well. Row 60 does not read it for its check.

## Not checked

Nothing was cut off by the call or time cap. I did not open the C++ tests behind L-C04 rows 88 to 116 or L-C05 rows 74 to 115, nor `test_launch_core.py` and `test_launch_patch.py` (L-C05 rows 21 to 32). Those rows are agreed from the counters' reasons alone, which name the typed values and the hand-built objects. I checked the key against the reason, but not the test text.
