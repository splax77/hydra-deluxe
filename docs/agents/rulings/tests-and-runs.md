# Rulings: tests and runs

Which tests run when, how big a run may be, and when the whole song library may be touched. The line format is in README.md beside this file. Newest first.

Under Confirmed are the user's exact words, filed with the user's approval or through the backfill route; under Unconfirmed are older lines written by sessions, which guide sessions as working rules but are never cited as the user's ruling.

## Confirmed
- **2026-11-16** "yes a test should fail instantly if its pass condition is impossible, not hang until it's forced to time out" (session 872cc6f3)
- **2026-11-15** "we have a test corpus we can use for testing database code" (session 88ea48bb)
- **2026-11-15** "tests should never be based on anything fake. testing fake made up data is useless and says nothing about how the app behaves in a real situation" (session 63c70014)
- **2026-11-15** "tests should not be pinned to typed constants. they should be pinned to the behavior of the code." (session 63c70014)
- **2026-11-15** "tests exist to verify the behavior of the code. a test citing something that isn't code is always wrong" (session 63c70014)
- **2026-11-15** "tests should test their intended behavior directly, not make up an arbitrary meaningless string" (session 63c70014)
- **2026-10-25** "i want to be able to give explicit approval for a full library run." (session dee9c61e)
- **2026-10-24** "Fix the doc line, and also add the repo-wide tests (docs-vs-code, single-owner scan, precheck self-test) to every merge run." (session c965ee5e)
- **2026-10-24** "this was supposed to be faster and now you're making it slower??? what the hell is happening here" (session 66725311)
- **2026-10-14** "why do we need to run the full suite every time a small change is made? this is a big waste of time. all the parts you didnt change will remain the same" (session d5fcf777)
- **2026-10-14** "you dont need a full library run to check the rendering. this is exactly the kind of time wasting i am trying to avoid." (session d6a46eba)
- **2026-10-11** "i want to combine 1, 2, and 4. do not change any code, create a handoff with your findings so the next session can plan." (session 58fb9477)
- **2026-10-10** "it's stilll going? at 1 hour? something is very wrong here. that is completely unacceptable" (session 187db0b9)
- **2026-10-08** "I did not approve a full library test and I did not approve a 40 minute test." (session e40fb55d)
- **2026-10-07** "just do a small test on a few charts" (session 04f776a5)
- **2026-10-07** "you aren't explaining where the difference is coming from or how you're going to find out" (session 04f776a5)
- **2026-10-04** "running the suite 10 times is unnecessary and hogging CPU performance that can be used for other tasks" (session a9a461da)
- **2026-10-04** "agents do not run the full test suite. they only run the tests relevant to the changes they make." (session a9a461da)
- **2026-10-03** "how about you run the giants locally with the copying fixes, then estimate how long they would take under the untouched public code?" (session a06ceb6d)
- **2026-09-29** "instead of waiting an hour, do 20 parallel runs on sequential sections of the library and combine the final times." (session 717bbff8)
- **2026-09-29** "alright thats enough. stop the analysis" (session 717bbff8)


## Unconfirmed

- **2026-10-24** As the note records it: Exception (user, 2026-10-24): every merge to Hydra's main, docs-only included, also runs the three repo-wide tests: docs-vs-code, the single-owner scan and the precheck self-test (commands in CLAUDE.md rule 1). (session: not stated; note: merge-tests-touch-changed-files; backs: CLAUDE.md rule 1)
- **2026-10-14** "all the parts you didnt change will remain the same" As the note records it: RULE (user, 2026-10-14): a merge runs only the tests that touch the changed files; the whole suite only when shared code changed. (session: not stated; note: merge-tests-touch-changed-files; backs: CLAUDE.md rule 1)
- **2026-10-11** As the note records it: The user said CLI benchmarks don't show where the real app spends time, so on 2026-10-11 I profiled Hydra.exe itself. And: The user chose to combine three changes: keep report rows instead of re-analyzing (stored in db vs in memory still undecided), read each chart file once instead of once per mode, cache fitted cell text in the report window. (session 2026-10-11-2178949a, by date; note: gui-profile-recipe-2026-10; backs: no page yet)
- **2026-10-08** "I did not approve a full library test and I did not approve a 40 minute test" As the note records it: How to ask (2026-10-08, after the user said "I did not approve a full library test and I did not approve a 40 minute test"): the question names it a "full library run" in those words, says how many runs and how long each takes (estimate), and the brief then stays inside exactly that: no time cap or run count the user didn't see, no second run. And: Claude never runs a full library test unless it is truly necessary and the user has said yes in chat to that run. (session: not stated; note: no-full-library-runs; backs: CLAUDE.md rule 8)
- **2026-10-07** "why do you need 19000 states? this is a gigantic waste of time. just do a small test on a few charts", then "you keep suggesting more runs but you aren't explaining where the difference is coming from or how you're going to find out" (session: not stated; note: measure-small-explain-first; backs: CLAUDE.md rule 8)
- **2026-10-07** As the note records it: the user, 2026-10-07: Claude is "way too happy to waste time for no reason at all". Earlier (2026-10-06) the user had already cut per-agent library scans to one join check and called extra re-check runs "completely unnecessary". (session: not stated; note: no-full-library-runs; backs: CLAUDE.md rule 8)
- **2026-10-06** As the note records it: On 2026-10-06 the user asked for a speed exploration of scanning and analysis, done only in scratch worktrees (no real code changed; main stayed at bc58282). (session: not stated; note: perf-exploration-2026-10-06; backs: no page yet)
- **2026-10-04** "you dont need to run the full test suite every time a small change is made. run only what is relevant to that change" (session: not stated; note: archive/no-repeat-suite-runs; backs: CLAUDE.md rule 1)
- **2026-10-04** "that is an insane amount of time for one agent to run." (session: not stated; note: archive/no-repeat-suite-runs; backs: CLAUDE.md rule 7)
- **2026-10-03** As the note records it: 2026-10-03 GIANTS (user chose this over a full local or cloud run). (session: not stated; note: archive/public-vs-deluxe-library-timing-2026-09; backs: CLAUDE.md rule 8)
- **2026-09-29** As the note records it: User stopped it and asked for 20 parallel sections instead. And: Depth 0 (score range 0, user asked). (session: not stated; note: archive/public-vs-deluxe-library-timing-2026-09; backs: no page yet)
