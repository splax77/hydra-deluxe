Agent id: aabb092fea910677a (task T5-KR1, record file D8.md)

# T5-KR1: Keep records of D8.md read by the three test rulings

I read all 91 Keep records of D8.md and every check of each test case named, in the six files under `tools/ch_mcp/tests/`. Short names for them in the table: MW is test_method_watch.py, MF is test_mono_facts.py, PS is test_press_schedule.py, Q is test_quiet.py, RL is test_recorder_link.py, SV is test_server.py.

How I called the faults. X9: the test feeds the code a hand-written payload, reader answer, process row, fake process, fake pipe or state object that the real tool would never hand it. X10: the expected value is the code's own message text, command string, schema default or key set, typed again. X11: a value typed in (an error text, a pid, a file name, a player) is required back unchanged. X12 and Snapshot: none found in the Keep records' checks. The PS file's docstring and the SV file's comments cite removed plans, but no check takes its expected value from them.

"Named" says whether the record's own text already flagged it. I count a record as naming it when it says "typed", "fake" or "the test's own input" about that value. That is a description, not a verdict: no record called any of these a fault, because the rubric had no question for it.

Three judgment calls the reader may want to overrule. First, every test in Q that runs through `FakeReaders` (made-up process list, CPU reading, folder scan) is marked X9. Second, PS entries and plays are hand-built, but they are the scheduler's own input built by the case to the rule, and real Node runs the real scheduler, so I did not mark them; the one exception is PS :175, where the score the scheduler reads is made up. Third, the pass-through tests in SV (value goes in as a typed string, comes out in a recorded tuple) are marked X11. Distinct values do show an argument-order swap, so that mark is the weakest one in the file.

## Table

Columns: record (file:line of test), faults, faulty checks, reason, named.

| Record | Faults | Faulty checks | Reason | Named |
|---|---|---|---|---|
| MW :165 | X10 | MW:168 | refusal text "; no method watch started" typed | no |
| MW :175 | X9 | MW:72-74, 124-127, 172 | backup line and the agent's summary are hand-written stand-ins | yes (fake line) |
| MW :181 | X9, X10 | MW:182, 184-185 | made-up summary; refusal sentence typed | X10 yes, X9 no |
| MW :205 | X9, X10 | MW:206, 211-212 | made-up summary; refusal text typed | no |
| MW :253 | X10 | MW:258 | "npm run build:menuhooks" typed | yes |
| MW :324 | X9, X10 | MW:132-136, 325, 328 | three hand-written agent payloads; log file name typed | X9 yes, X10 no |
| MW :371 | X9, X10 | MW:36-49, 372, 375 | hand-built dump facts; message typed | X10 yes, X9 no |
| MW :377 | none | | real temp file, real loader | |
| MW :388 | none | | real temp file, real loader | |
| MW :396 | X10 | MW:399-401 | whole build command typed | yes |
| MF :155 | X9, X10 | MF:47-85, 156-158 | hand-written copy of mono_reader.ps1 output; message typed | no |
| MF :185 | X9, X10, X11 | MF:186-189 | made-up child reply; message typed; "BadImageFormatException" typed in, required back | X9 yes |
| PS :124 | none | | scheduler run on case-built frames | |
| PS :148 | none | | same | |
| PS :175 | X9 | PS:177, 181-183, 192 | score the game would read is made up (7, 57, 58, 59) | yes (typed scores) |
| PS :194 | none | | | |
| PS :205 | none | | | |
| PS :218 | none | | | |
| PS :224 | none | | | |
| PS :233 | none | | | |
| PS :249 | none | | | |
| PS :291 | none | | | |
| PS :297 | none | | | |
| PS :305 | none | | | |
| PS :311 | none | | | |
| PS :318 | none | | | |
| Q :87 | X9 | Q:91 | made-up process rows | no |
| Q :93 | X9 | Q:96 | made-up process rows | no |
| Q :123 | X9 | Q:124 | all readers (processes, CPU, scan) are made up | no |
| Q :126 | X9 | Q:127-128 | made-up CPU reading | no |
| Q :145 | X9 | Q:146 | made-up folder row | no |
| Q :148 | X9 | Q:149-152 | made-up sampler | no |
| Q :154 | X9, X10 | Q:155-158 | made-up process rows; "X.exe: running" lines typed | no |
| Q :178 | X9, X10, X11 | Q:179-183 | made-up reader failure; line typed; "no list" typed in, required back | X10 yes |
| Q :185 | X9, X10, X11 | Q:186-188 | same shape; "no counter" | X10 yes |
| Q :190 | X9, X10, X11 | Q:191-193 | same shape; "no disk" | X10 yes |
| Q :195 | X9, X10, X11 | Q:196-199 | made-up scan result; line typed; "f: denied" echoed | X10 and X11 yes |
| Q :201 | X9, X10 | Q:202-207 | made-up scan result; line typed ("cap of 3 s", "of 5") | X10 yes |
| Q :209 | X9, X10, X11 | Q:210-214 | made-up reader failures; lines typed; "m", "p" echoed | no |
| Q :228 | X9, X10 | Q:230-231 | made-up process row; line typed | X10 yes |
| Q :273 | none | | real temp files, real scandir | |
| Q :281 | none | | | |
| Q :286 | none | | | |
| Q :306 | none | | | |
| Q :313 | X10 | Q:317-318 | line typed, "(a file 2 s old)" | yes |
| Q :320 | none | | | |
| Q :324 | none | | | |
| Q :336 | none | | | |
| Q :343 | none | | | |
| Q :349 | none | | | |
| Q :364 | none | | | |
| Q :392 | X9, X10, X11 | Q:393-401 | made-up child reply; text typed; "bad" echoed | X9 yes |
| RL :130 | none | | checks how Popen is called, on a real log file | |
| RL :138 | X9, X10 | RL:140-143 | fake pid 777; line typed | yes |
| RL :145 | X9, X10 | RL:146-152 | fake pids 777, 778; lines typed | X10 yes |
| RL :155 | X9, X10 | RL:157-158 | fake pid in typed line | no |
| RL :160 | X9, X10, X11 | RL:161-166 | made-up start error; text typed; "no python" echoed | X10 yes |
| RL :169 | none | | | |
| RL :176 | X9, X10, X11 | RL:178-181 | made-up pipe failure; text typed; "the pipe is closed" echoed | yes |
| RL :183 | none | | | |
| RL :186 | X9, X10 | RL:188-193 | fake process exit code 0 and pid; stop line typed | X10 yes |
| RL :212 | X9, X10, X11 | RL:216-220 | made-up PermissionError("denied"); text typed and echoed | yes |
| RL :222 | X9, X10 | RL:228-233 | fake wait/close/terminate; "still runs" typed | no |
| RL :235 | X9, X10 | RL:238-247 | fake exit code 3; line typed | X10 yes |
| RL :260 | X10 | RL:263 | line typed | yes |
| RL :265 | X10 | RL:271-272 | line typed | yes |
| RL :274 | X9, X10 | RL:276-278 | hand-written samples.csv rows; line typed | yes |
| RL :280 | X9, X10 | RL:282-284 | hand-written rows; line typed | no |
| RL :286 | X9, X10 | RL:288-290 | hand-written rows with chosen line ends; line typed | X9 partly |
| RL :292 | X9, X10 | RL:294-295 | hand-written csv; substring typed | X10 yes |
| RL :297 | X9, X10 | RL:299-303 | fake exit code and rows; line typed | X10 yes |
| RL :305 | none | | compares the object with its own stop line | |
| RL :310 | none | | real child, real pipe, real files | |
| SV :73 | X10 | SV:74 | "nothing open" typed | yes |
| SV :80 | X10, X11 | SV:81-82 | line typed; "the-backup" set in and read back | yes |
| SV :92 | X9 | SV:88-90 | the reply is wrapped in a hand-written service envelope | no |
| SV :95 | X9, X10, X11 | SV:88-90, 96-97 | same envelope; expected line typed; "the-backup" echoed | yes |
| SV :112 | X9, X10 | SV:113-119 | hand-built "ended" state objects; "nothing open" typed | X10 yes |
| SV :128 | X9 | SV:129-148 | hand-built state objects set into private fields | no |
| SV :213 | X11 | SV:217-220 | "song", "s.json", "p.json" typed in, required back | no |
| SV :222 | X11 | SV:225-229 | same, plus 3 | no |
| SV :249 | X11 | SV:252-255 | 5 typed in, 5 required back | yes |
| SV :257 | none | | identity of the real parts | |
| SV :277 | X10, X11 | SV:278, 281-284 | default False typed; "song" echoed | no |
| SV :286 | X10, X11 | SV:290-296 | defaults 0 and None typed; strings echoed | no |
| SV :298 | X11 | SV:302-307 | "ProDrums,Expert" typed in and back | yes |
| SV :309 | X10, X11 | SV:311-313, 315-325 | absent names typed; 192 and file names echoed | X11 yes |
| SV :327 | X10, X11 | SV:329-342 | argument names and defaults typed; "AB", 100 echoed | X11 yes |
| SV :344 | none | | expects the owner's own text | |
| SV :349 | X10, X11 | SV:350-356 | default None typed; "System" echoed | X11 yes |
| SV :358 | X10, X11 | SV:359-367 | default None typed; 0.748 and "Green" echoed | X11 yes |

## Counts

Keep records read: 91 of 91. Not reached: none.

Records with at least one fault: 59. Records with none: 32 (MW 2, PS 13, Q 10, RL 5, SV 2).

By file, faulty of total: MW 8 of 10, MF 2 of 2, PS 1 of 14, Q 16 of 26, RL 16 of 21, SV 16 of 18.

Records per fault: X9 41, X10 45, X11 22, X12 0, Snapshot 0. (A record can hold several.)

Records whose fault the record already named in its own words ("typed", "fake", "the test's own input"), for at least one of its faults: 39 of 59. Records that named none of their faults: 20. No record called any of them a fault; the rubric had no question for it.

## Weak marks

SV pass-through X11 is the least firm mark, because distinct typed values do catch an argument-order swap. The Q X9 marks hold only if a made-up reader answer counts as made-up outside data; I believe it does under the brief, but those 15 are the largest block. PS escapes X9 only because I treated the case-built scheduler input as the unit's own input; a stricter reading would add the 13 PS records.
