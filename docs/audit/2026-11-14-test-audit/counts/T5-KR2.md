Agent af6cd3c3bfb2422fc (task T5-KR2, session 63c70014-4328-49dc-9205-742235732752)

# T5-KR2: Keep records of D12, E3 and D7, re-read by the three test rulings

I read all 97 Keep records (D12 40, E3 33, D7 24) and the test code each one names. The test files are in this worktree (`tools/ch_probe/tests/`, `tools/ch_mcp/tests/`) and in `C:\Users\Patrick\.claude\gates\tests\`.

Judgement calls that move the counts:

- A hand-written stand-in for the game, a service, a transcript, a recording, a rules module or a judge counts as X9. So do the deliberately bad strings ("not json", "nonsense", "index,ms") the user's third ruling names.
- A test that checks a typed message, label or stop word counts as X10. The stop-word ones in D7 (:638, :896, :903, :915) are the weakest calls.
- Pure arithmetic and byte-layout tests, and real-mechanism tests (real git, real files, real processes, real argparse), count as none.
- "Named" means the record text or its shared answers named that fault, even when it did not treat it as a blocker.

Test lines are `file:line` of the check. "Rec" is the order in that file's Keep list.

## D12 (40 Keep records)

| Rec | Test (file:line) | Faults | Faulty check | Reason | Named |
|---|---|---|---|---|---|
| 1 | test_play_in_game.py:265 | X9 | :267 | agent event `seq 7` hand-built in a FakePlayer | yes (S-PG) |
| 2 | test_play_in_game.py:293 | X9 | :295 | made-up stop reason "melted" through a FakePlayer | yes (S-PG) |
| 3 | test_prep_inputs.py:132 | X9, X10 | :140-144, :147 | hand-written stub for hydra_replay with typed stderr; asserts the typed failure line | no |
| 4 | test_prep_inputs.py:151 | X9, X10 | :154-155 | stub fails with typed exit 3 and text, both required back in the message | no |
| 5 | test_prep_inputs.py:158 | X10 | :161 | pins the typed "hydra_replay not found:" line | no |
| 6 | test_probe_chart.py:44 | none | | tick arithmetic from the .chart rule | |
| 7 | test_probe_chart.py:59 | none | | tick arithmetic and round trip | |
| 8 | test_probe_songs.py:34 | none | | one tick = one ms, from the owner's grid | |
| 9-16 | test_process.py:39, 47, 51, 56, 60, 64, 68, 72 | none | | decoders checked against struct/IEEE layouts | |
| 17 | test_process.py:80 | none | | base + RVA arithmetic | |
| 18 | test_process.py:84 | none | | RVA 0 resolves to base | |
| 19 | test_process.py:92 | X9 | :95 | hand-built fake game memory | no |
| 20 | test_process.py:98 | X9 | :101 | hand-built fake game memory | no |
| 21 | test_process.py:104 | X9 | :107 | fake memory holding a typed constant 85.0 | no |
| 22 | test_process.py:110 | X9 | :115 | fake memory returning a short read | no |
| 23 | test_process.py:119 | X9, X11 | :120-122 | fake writer; bytes written come straight back from it | no |
| 24 | test_process.py:134 | X10 | :137 | typed 0.085/0.0375 s as the expected game constants | no |
| 25 | test_process.py:148 | X10 | :151 | same typed constants | no |
| 26 | test_process.py:153 | X10 | :156 | typed 85.0/37.5 | no |
| 27 | test_process.py:169 | X9, X10 | :163-171 | fake memory holding typed 0.085/0.0375 | no |
| 28 | test_process.py:177 | X9, X10 | :179 | fake memory with typed 0.025 against typed 0.085 | no |
| 29 | test_s2_window_constants.py:35 | X10 | :36-37 | checks the constants equal a committed measurement table, so typed number against typed number | no |
| 30 | test_score_compare.py:115 | X9 | :117 | hand-built `_SCORE` and presses, made-up route "mouse" | yes (S-SC) |
| 31 | test_score_compare.py:123 | X9 | :126 | hand-built Press and score | yes (S-SC) |
| 32 | test_score_compare.py:163 | X9 | :164 | hand-built presses, expected -1.5 follows from the typed 1.5 | yes (S-SC) |
| 33 | test_score_compare.py:167 | none | | spread of one number | |
| 34 | test_score_compare.py:172 | X9, X11 | :173-179 | typed row written then required back | no |
| 35 | test_score_compare.py:181 | X9 | :185 | made-up header text "index,ms" | no |
| 36 | test_score_compare.py:267 | none | | fields compared with the owner's column list | |
| 37 | test_score_compare.py:283 | X9, X11 | :284-292 | typed 1.5 and 212.4 required back | no |
| 38 | test_score_compare.py:308 | X9, X11 | :309-317 | typed 3.25 and "retime_missed" required back | no |
| 39 | test_score_compare.py:319 | X9 | :322 | hand-typed row under the owner's header | yes (record says the row is typed) |
| 40 | test_score_compare.py:326 | X9, X10 | :327-340 | four hand-written headers; regex pins the message start | no |

## E3 (33 Keep records)

| Rec | Test (file:line) | Faults | Faulty check | Reason | Named |
|---|---|---|---|---|---|
| 1 | test_refusals.py:75 | X9, X10, X11 | :79, :82-83 | hand-typed `call()` payload; typed "no, because" required back; typed message note | no |
| 2 | test_review_dispatch.py:211 | X9 | :218 | made-up input "not json" | no |
| 3 | test_review_dispatch.py:353 | X9 | :355 | made-up input "not json" | no |
| 4 | test_ref_judge.py:194 | none | | real git, real shim | |
| 5 | test_ref_judge.py:258 | none | | real git pack-refs, rules file really absent | |
| 6 | test_ref_judge.py:320 | none | | rules file really absent | |
| 7 | test_ref_judge.py:329 | none | | rules file really absent | |
| 8 | test_ref_judge.py:377 | X10 | :382 | typed "did not give a verdict" | yes |
| 9 | test_ref_judge.py:385 | X9, X10 | :388, :391 | hand-written judge that raises | no |
| 10 | test_ref_judge.py:393 | X9, X10 | :396, :399 | hand-written judge printing "allow please" | no |
| 11 | test_ref_judge.py:401 | X9, X10 | :405, :410 | judge replaced by a sleeping lambda; typed message | X10 yes |
| 12 | test_ref_judge.py:412 | X9, X10 | :415-416 | made-up line "not a ref update line" | yes (git never sends it) |
| 13 | test_ref_judge.py:418 | X10 | :429 | typed message | no |
| 14 | test_ref_judge.py:431 | X10 | :440 | typed trailer key "Session:" | no |
| 15 | test_ref_judge.py:442 | X9, X11 | :445, :448 | hand-written raising rules; "rules broke" required back | no |
| 16 | test_ref_judge.py:498 | X9, X10 | :499, :496 | hand-written "refuse" rules; typed "put back" | X10 yes |
| 17 | test_ref_judge.py:503 | X9, X10 | :504, :496 | same | X10 yes |
| 18 | test_ref_judge.py:510 | X9 | :511 | hand-written "allow" rules | no |
| 19 | test_ref_judge.py:518 | X9 | :519 | same | no |
| 20 | test_ref_judge.py:527 | X9, X10 | :528, :535-536 | fake rules; typed "Run git status" | no |
| 21 | test_ref_judge.py:538 | X9, X10 | :539, :549-550 | same | no |
| 22 | test_ref_judge.py:552 | X9 | :553 | fake rules | no |
| 23 | test_ref_judge.py:562 | X9 | :563 | fake rules | no |
| 24 | test_ref_judge.py:571 | X9, X10 | :572, :578 | fake rules; typed message | X10 yes |
| 25 | test_ref_judge.py:580 | X9, X10, X11 | :584, :596-606 | fake rules, mocks, typed "restore broke" required back | X9 yes (heavy mocking) |
| 26 | test_ref_judge.py:610 | X10 | :614 | typed selftest line | yes |
| 27 | test_ref_judge.py:626 | X9, X10 | :629, :632 | hand-written judge printing "WRONG" | no |
| 28 | test_ref_judge.py:637 | none | | real git config | |
| 29 | test_ref_judge.py:654 | X9 | :658 | hand-written old shim text | yes (marker typed from a comment) |
| 30 | test_ref_judge.py:795 | X9 | :798 | hand-written broken rules module | no |
| 31 | test_ref_judge.py:806 | X9, X11 | :809, :812 | broken module's text required back | no |
| 32 | test_ref_judge.py:814 | X10 | :819-820 | typed message words | no |
| 33 | test_ref_judge.py:832 | X9 | :836 | fake "refuse" rules | no |

## D7 (24 Keep records)

| Rec | Test (file:line) | Faults | Faulty check | Reason | Named |
|---|---|---|---|---|---|
| 1 | test_machine_recorder.py:161 | X12, snapshot | :97-108, :162-182 | typed counts pinned from one capture; comment cites Get-Process | yes |
| 2 | test_machine_recorder.py:229 | none | | real sample, real parser | |
| 3 | test_machine_recorder.py:252 | snapshot | :255 | typed 18 read from one saved sample | no |
| 4 | test_machine_recorder.py:417 | none | | real folder | |
| 5 | test_machine_recorder.py:638 | X10 | :640-642 | typed stop words | no |
| 6 | test_machine_recorder.py:708 | X9 | :710 | made-up lines "nonsense", "pid x" | no |
| 7 | test_machine_recorder.py:715 | none | | protocol lines the parent sends | |
| 8 | test_machine_recorder.py:822 | X11 | :823 | typed 9 required back | no |
| 9 | test_machine_recorder.py:896 | X10 | :901 | typed "stdin closed" | no |
| 10 | test_machine_recorder.py:903 | X10 | :913 | typed "cap" | no |
| 11 | test_machine_recorder.py:915 | X10 | :925 | typed "parent gone" | no |
| 12 | test_machine_report.py:230 | none | | pure function on a list | |
| 13 | test_machine_report.py:537 | X9, X10 | fixture, :539-540 | hand-built recording; typed message | no |
| 14 | test_machine_report.py:664 | X9, X10 | :665-671 | hand-built recording; typed refusal text | X10 yes |
| 15 | test_machine_report.py:675 | X9, X10 | :676-685 | hand-built recording; typed first line | no |
| 16 | test_menu_tools.py:226 | X9 | :226-231 | fake game, kit and clock | no |
| 17 | test_menu_tools.py:279 | X9, X10 | :280, :284-285 | scripted fake menu reads; typed refusal text | no |
| 18 | test_menu_tools.py:382 | none | | compares with the owner's constant | |
| 19 | test_menu_tools.py:414 | none | | real path functions | |
| 20 | test_menu_tools.py:419 | none | | real junction | |
| 21 | test_menu_tools.py:476 | none | | object identity and source scan | |
| 22 | test_menu_tools.py:502 | none | | checks the file against the owner's check | |
| 23 | test_menu_tools.py:541 | X9, X10 | :543-556 | hand-written steps file, FakeKit and FakeFrida; typed text | no |
| 24 | test_menu_tools.py:563 | X10 | :564 | typed "song_loaded" label pinned against a data file | no |

## Counts

- Keep records read: 97 (D12 40, E3 33, D7 24). None left unread.
- Records with at least one fault: 68 (D12 25, E3 28, D7 15). 29 are clean.
- Records per fault (a record can hold several):
  - X9 made-up input: 50 (D12 20, E3 23, D7 7).
  - X10 constant pin: 37 (D12 10, E3 17, D7 10).
  - X11 echo: 9 (D12 4, E3 4, D7 1).
  - X12 cited non-code: 1 (D7 rec 1).
  - Snapshot: 2 (D7 recs 1 and 3).
- Faulty records whose fault the record already named: 17 (D12 6, E3 9, D7 2).

## Limits

- I did not read the code of `FakeLoop`, `Fixture`, `FakeKit`, `FakeFrida` or `fakes.py` line by line. I judged them hand-built from how the tests use them.
- The X10 calls on typed stop words and message text (D7 recs 5, 9, 10, 11) are the least certain. Dropping them would take D7's faulty count to 11.
- I ran nothing.
