Agent id: adfce35c80bf1b850 (task T5-IR4, record file E3.md)

# T5-IR4: Investigate records of E3.md read by the three test rulings

E3.md holds 104 records with verdict Investigate. I read every one, then read the test case it names in the four files under `C:\Users\Patrick\.claude\gates\tests\`. Short names: RF is test_refusals.py, RD is test_review_dispatch.py (it also holds the waiver tests), RJ is test_ref_judge.py, RR is test_review_rules.py. I ran nothing.

How I called the faults.

X9 (made-up input): a hand-typed Agent or prompt payload (RD), a fake `review_rules.py` that the test writes (RJ `rules("allow")` and `rules("refuse")`, which also stands in for the real rules), a stub rule appended to the real one, made-up transcripts (`_typed`, `_answer`, `human()`), hand-written verdict, waiver and real-app records (`clean()`, `record()`, `waive()`). A git repo the test builds and commits into is input to the real code, so I did not mark it. Real git with the real judge and no stand-in is "none".

X10 (typed text pinned): the expected value is a refusal sentence or phrase typed in the test ("no review key", "is not a descendant", "delete main").

X11 (echo): a value typed into the input and required back unchanged (session "sess-1", prompt id "pid-1", the fake rule's own refusal text, WORDS in stderr).

X12: none found. The only "pinned from one run" lines are the `BRANCH_COMMIT` and hash constants in RD (:26-29) and a doc key in RR (:331-335). Those two cases (`test_prompt_hash_is_utf8_sha256_pinned`, `test_doc_key_matches_the_publish_gate_vector`) are not Investigate records, so they are outside this count. Snapshot: only :982 in RR, and only weakly.

The "Named" column says whether the record already named the fault. "group" means the record file's shared answers (2:G-RD, 3:G-RJ, 3:G-RR, 3:G-TX) say the input is hand-typed or hand-built; that describes the input but does not call it a fault, and no record called any of these a fault, because the rubric had no question for it. "yes" means the record's own text says "text copy", "copy of" or "fake". "no" means it does not say so.

Judgment calls. First, every case that runs through the fake `review_rules.py` in RJ is X9 even though real git and the real judge run, because the thing that decides main is the stand-in. Second, hand-written verdict records in RR are X9: the real hook would write them from a real reviewer, and the case invents them. Third, the X11 marks on payload echoes are the weakest marks.

## Table

| Record (file:line) | Faults | Reason | Named |
|---|---|---|---|
| RF:52 crash deny logged | X9, X11 | hand payload `call()`, stand-in hook that raises; "boom here" typed in and required back (:58) | no |
| RF:60 ask writes line | X9, X10 | hand-written PowerShell call; typed "media guard" (:67) | yes |
| RF:69 allow writes nothing | X9 | three lambdas stand in for hooks (:70-72) | yes |
| RD:76 isolated reviewer refused | X9, X10 | hand-typed Agent payload; typed "leave isolation out", "branch's own worktree" (:81) | X9 group, X10 no |
| RD:85 no isolation writes handover | X9 | hand-typed payload | group |
| RD:90 isolation on other agent | X9 | hand-typed payload | group |
| RD:103 handover tip and branches | X9, X11 | hand-typed payload; session "sess-1", time 1000.0, cwd typed in and required back (:112-114) | X9 group, X11 no |
| RD:123 upper case one tip | X9 | hand-typed payload | group |
| RD:128 doc key beside commit | X9 | hand-typed payload | group |
| RD:135 doc key alone | X9, X10 | hand-typed payload; typed "no review key" (:137) | X10 yes, X9 group |
| RD:139 two commits refused | X9, X10 | hand-typed payload; typed "one commit" (:144) | X10 no, X9 group |
| RD:147 key on main | X9, X10 | hand-typed payload; typed "main" (:148) | no |
| RD:150 no local main | X9, X10 | hand-typed payload; typed "no local main branch" (:152) | X10 yes, X9 group |
| RD:154 unknown key | X9, X10 | hand-typed payload; typed "does not know" (:156) | no |
| RD:161 no key gated repo | X9, X10 | hand-typed payload; typed "no review key" (:163) | no |
| RD:165 no key doc-gated repo | X9, X10 | hand-typed payload; typed "no review key" (:167) | no |
| RD:169 no key, no marker | X9 | hand-typed payload | group |
| RD:175 no key outside repo | X9 | hand-typed payload | group |
| RD:183 hex runs of other lengths | X9, X10 | hand-typed payload; typed "no review key" (:186) | no |
| RD:189 non-reviewer agent | X9 | hand-typed payload | group |
| RD:194 workflow call not matched | X9 | hand-typed payload | group |
| RD:198 dispatch outside a repo | X9, X10 | hand-typed payload; typed "crashed" (:208) | no |
| RD:241 key only in other repo | X9 | hand-typed payload naming a second repo | group |
| RD:247 doc key beside other repo commit | X9 | hand-typed payload | group |
| RD:251 key known at cwd | X9 | hand-typed payload | group |
| RD:255 two other repos | X9, X10 | hand-typed payload; typed "does not know" (:258) | no |
| RD:260 other repo key on main | X9, X10 | hand-typed payload; typed "already on main" (:262) | X10 yes, X9 group |
| RD:264 named subfolder | X9, X10 | hand-typed payload; typed "does not know" (:268) | no |
| RD:270 more folders than cap | X9, X10 | hand-typed payload; typed "does not know" (:277) | no |
| RD:279 cap exactly reached | X9 | hand-typed payload | group |
| RD:297 derive-once phrase recorded | X9, X11 | hand-built UserPromptSubmit payload; "pid-1", session, time typed in and required back (:299-301) | X9 yes, X11 no |
| RD:303 doc-review phrase | X9, X11 | hand-built payload; typed values required back | group / no |
| RD:308 library allow | X9, X11 | hand-built payload; typed values required back (:314-316) | group / no |
| RD:318 key allow | X9, X11 | hand-built payload; typed values required back (:324-326) | group / no |
| RD:328 phrase inside a sentence | X9 | hand-built payloads | group |
| RD:345 missing prompt id | X9, X11 | hand-built payload; field name typed in and required in the warning (:349) | group / no |
| RJ:136 commit without trailers | X9, X10 | fake review_rules.py (:138); typed `--trailer "Task:` and "Session:" (:143-144) | yes / yes |
| RJ:147 commit with trailers | none | real git, real judge, no stand-in | |
| RJ:153 commit origin has | none | real git, real judge | |
| RJ:170 merge default message | X9 | fake review_rules.py (:172) | yes |
| RJ:184 revert without trailers | X9, X10 | fake review_rules.py; typed "missing Task:" (:191) | yes / yes |
| RJ:201 non-ff move of main | X9 | fake review_rules.py (:203) | yes |
| RJ:210 packed main | X9, X10 | fake review_rules.py; typed "is not a descendant" (:219) | yes / yes |
| RJ:221 origin-tracked branch | none | real git and judge, no fake rules | |
| RJ:232 local-only branch | none | real git and judge, no fake rules | |
| RJ:239 deleting main | X9, X10 | fake review_rules.py; typed "delete main" (:245) | yes / no |
| RJ:248 deleting packed main | X9, X10 | fake review_rules.py; typed "delete main" (:256) | yes / no |
| RJ:268 symbolic ref | X10 | typed "symbolic" required in stderr (:272) | no |
| RJ:274 non-ff push | X10 | typed "is not a descendant" (:282) | no |
| RJ:285 ff push passes | none | real git push through real judge | |
| RJ:293 deleting main on remote | X9, X10 | fake review_rules.py; typed "delete main" (:299) | yes / no |
| RJ:303 rules present and allowing | X9 | fake review_rules.py records the calls (:305-309) | yes |
| RJ:311 rules present and refusing | X9, X11 | fake rule's own text "fake review refusal" typed in the fake, required back (:317) | X9 yes, X11 no |
| RJ:335 push of local main | X9 | fake review_rules.py (refuse) | yes |
| RJ:349 push of commit local main lacks | X9, X11 | fake rules; own refusal text required back (:358) | X9 yes, X11 no |
| RJ:362 non-ff push of local main | X9, X10 | fake review_rules.py; typed "is not a descendant" (:372) | yes / no |
| RJ:746 unapproved line, side branch | X9, X10, X11 | made-up transcripts and a stub rule (:683-694, 678); WORDS typed in and required in stderr (:742); typed "approval test", "ruling_rules.py --check" (:743-744) | X9 group, rest no |
| RJ:754 unapproved line on main | X9, X10, X11 | same as :746 | group / no |
| RJ:761 merge with unapproved line | X9, X10, X11 | same as :746 | group / no |
| RJ:771 approved line then merge | X9, X11 | made-up transcripts; session file name typed and required back (:778) | group / no |
| RJ:786 removing lines passes | X9 | stub rule appended to the real review_rules (:678, 706) | no |
| RR:100 ff to clean key | X9 | verdict record written by hand with `clean()` (:77-81) | group |
| RR:106 unreviewed tip refused | X9, X10 | hand-written verdict context; typed phrase "waive derive-once " (:112) | group / no |
| RR:114 blocked verdict | X9 | hand-written verdicts | group |
| RR:129 cause: no verdict | X10 | expects typed cause text "no reviewer verdict is recorded for" (:132) | yes (copy) |
| RR:140 cause: commits after | X9, X10 | hand-written verdict; typed cause text (:146-147) | group / no |
| RR:149 cause: differing merge | X9, X10 | hand-written verdict; typed cause text (:160-162) | group / no |
| RR:182 cause: merge, unreviewed commits | X9, X10 | hand-written verdict; typed cause text (:192-193) | group / no |
| RR:195 two causes joined | X9, X10 | hand-written BLOCKED verdict; typed ". Also: README.md changes..." (:201) | X10 yes, X9 group |
| RR:203 clean merge of old main | X9 | hand-written verdict | group |
| RR:224 conflict resolution refused | X9 | hand-written verdict | group |
| RR:252 CI setup is code | none | real commit judged by real rules, no stand-in | |
| RR:260 marker gate lines | X9 | hand-written verdict (:262) | group |
| RR:282 new branch, old zero | X9 | hand-written verdicts (:287-289) | group |
| RR:295 public doc without record | X10 | typed phrase "waive doc-review " (:301) | no |
| RR:303 public doc with record | X9 | hand-written verdict (:305) | group |
| RR:308 same path earlier record | X9 | hand-written verdict (:309) | group |
| RR:313 internal doc not gated | none | real commit, real rules | |
| RR:317 marker change needs record | X9 | hand-written verdicts (:318, 321) | group |
| RR:324 marker deletion refused | none | real commit, real rules | |
| RR:328 doc key drops BOM and CRLF | none | compared with `hashlib` on the cleaned bytes; independent oracle | |
| RR:341 no markers passes | none | real commit, real rules | |
| RR:358 human-typed waiver counts | X9 | hand-written transcript record (turnOrigin, promptId, :349-352) and waiver (:355) | group (3:G-TX) |
| RR:364 sdk waiver | X9 | hand-written transcript and waiver | group |
| RR:370 waiver of another kind | X9 | hand-written transcript and waiver | group |
| RR:376 clean merge, waived tip | X9 | hand-written transcript and waiver | group |
| RR:386 merge, unwaived untyped tip | X9 | hand-written transcript and waiver | group |
| RR:395 doc waiver | X9 | hand-written transcript and waiver | group |
| RR:425 ui change needs real-app record | X9, X10 | hand-written verdict via `ui_tip`; typed "waive real-app ", "1 file(s) under src/ui" (:429-430) | group / no |
| RR:433 real-app record in range | X9 | hand-written real-app record (:406-409) and verdict | group |
| RR:439 record before non-ui commit | X9 | hand-written records | group |
| RR:446 later ui commit not covered | X9, X10 | hand-written records; typed "waive real-app " (:451) | group / no |
| RR:453 record on the branch | X9 | hand-written records | group |
| RR:461 record outside range | X9 | hand-written records | group |
| RR:466 dirty record | X9 | hand-written records | group |
| RR:471 unused window | X9 | hand-written records | group |
| RR:485 created main | X9, X10 | hand-written records; typed "waive real-app" (:490) | group / no |
| RR:494 real-app waiver | X9 | hand-written transcripts and waivers | group |
| RR:503 clean merge, waived tip (RealApp) | X9 | hand-written transcript and waiver | group |
| RR:512 ui path outside marker paths | X9 | hand-written verdict (:514) | group |
| RR:519 waiver hook records phrase | X11 | made-up key typed in, required back in the parsed phrase (:520-521) | no |
| RR:524 non-ui change skips real-app | X9 | hand-written verdict; `realapp_checked` replaced by a mock (:528) | group |
| RR:532 deleting a ui file | X9 | hand-written verdicts (:535, 537) | X9 group, copy note only |
| RR:982 every historical main move | Snapshot | the moves are real data (good), but exactly one known exception, typed in the data file, is required (:1004-1006) | yes (pin) |

## Counts

Counted from the table by script on its 104 rows. Investigate records read: 104 (RF 3, RD 33, RJ 25, RR 43).

Records with at least one fault: 94 (93 with X9, X10 or X11, plus RR:982 which is Snapshot only). Records with none: 10 (RJ :147, :153, :221, :232, :285; RR :252, :313, :324, :328, :341).

Records per fault (a record can hold several): X9 88, X10 37, X11 14, X12 0, Snapshot 1.

Records whose fault the record already named in its own text: 22 of 94 (the rows with "yes" in the last column). For the other X9 rows, the record file shared answers describe the input as hand-typed or hand-built, but no record called it a fault, because the rubric had no question for it.


## Caveats

I did not open the real `~\.claude` hook code beyond what was needed to see what each test feeds it, and I trusted none of the record's reasons; every mark comes from the test code I read.
