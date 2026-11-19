# TOOL-AUDIT-T5-W09 (second-round verifier, ledger L05)

All claims are by reading. I ran nothing. Short names: HY = C:\Users\Patrick\Downloads\Hydra\hydra-test; GATES = C:\Users\Patrick\.claude\gates; MEM = the memory folder; BP = HY\docs\agents\brief-preamble.md. For the user's words I searched HY\docs\agents\rulings\*.md for "guess" (three hits, none about hooks, see L-F15-06); I did not re-search the others beyond the greps named in each row, so "none found" below means only those greps.

### L-F15-03
**Verdict:** CONFIRMED (the row's single-home claims hold). I add that there are three join procedures, not two.
**Homes as found:**
- HY\docs\agents\integrate.md:13 "Run `git log --first-parent <fork>..main --oneline` ..."; :15 "`git diff <fork>..main -- tests/single_owner/rules/`"; :27 "`git merge --no-ff --no-commit <task branch>` ... Hold the last task's merge uncommitted until the precheck in step 4 is clean"; :31 "Never rebase, amend, reset or force-move a branch. ... stop and report both sides"; :50 "Never amend."
- GATES\README.md:160 "The main session merges `main` into the task branch inside the worktree ... then fast-forwards `main`: `git -C C:\Users\Patrick\.claude merge --ff-only <branch>`". This is the hooks repo, not Hydra.
- HY\docs\agents\derive-once-review.md:155 "Sometimes main moves after a branch was reviewed, and the branch has to merge main in to catch up." Then the "Resolution review" with `git show --remerge-diff <M>`. The row's "155-163,191" reads right; the page has no `merge --` command line of its own.
- BP:111 and owner-map.md "The stops": the stop-and-report rule; integrate.md:23 and :31 apply it to the join.
**Splits:**
- Integrator joins task branches onto a wave branch with `--no-ff --no-commit` (integrate.md:27); the main session fast-forwards `main` with `--ff-only` (GATES\README.md:160, hooks repo); a reviewed branch catches up by merging main in (derive-once-review.md:155). Each answers "how does a branch join" for a different actor and repo. They differ for an integrator versus the main session versus a reviewer. No clash found in the words.
- The commit-hold versus "commit after each finished step" split stays under L-F13-09 (integrate.md:27 against BP:71).
**Ruling:** none found (not searched beyond the review-and-hooks and agents-and-workflow Confirmed lines named in the ledger preamble; I did not reopen them).
**Open:** none by reading.

### L-F15-04
**Verdict:** CONFIRMED-CHANGED. The pointers are right, but the note's merge rules (the part "no page holds") are only partly unhomed.
**Homes as found:**
- MEM\stitched-chart-auto-fills.md:11 "The game fact and the fix ... are FL-27, FL-28 and FL-30 on docs/clone-hero/04-fills.md"; "How to apply: the merge rules no page holds": ENABLE_CHART_DYNAMICS once for the whole chart, flatten velocities in songs without dynamics, reset disco flip at every song start, drop the BEAT track, "`ffmpeg -ss` before `-i` mis-seeks ogg by about 16 ms".
- HY\docs\clone-hero\04-fills.md:276 FL-27 (Rule: "any written fill on a track stops generated fills for that whole track"), :286 FL-28, :306 FL-30 ("A stitched chart ... loses its generated fills unless every song's fills are written"; :310 the fix: take each source's `is_fill` chords from `hydra_replay score`). The row said FL-27 and FL-28 bodies were unread; I read both. They agree with the note and with FL-30.
- HY\docs\clone-hero\02-note-list.md:153 "dynamics turn on only at a drum-track text event equal to `ENABLE_CHART_DYNAMICS` or `[ENABLE_CHART_DYNAMICS]`, compared exactly with no trim ...; the flag starts at 0 for each pass over a drum track". That is the game's rule for the flag; the note's "turn it on once for the whole chart" is advice on a stitched chart that the page does not give.
- MEM\ch-disco-noflip-rule.md (facts moved to 02-note-list.md NL-17 to NL-23; NL-21 at :253). The note's "reset disco flip at every song start" and "drop the BEAT track" have no hit in docs\clone-hero (grep for "disco flip", "BEAT track", "ffmpeg", "stitch" there: the only stitch hit is 04-fills.md; `ffmpeg` has no hit in docs\clone-hero).
**Splits:** None between homes. One gap: three or four stitching rules (flip reset, BEAT track, velocity flatten, 16 ms seek) live only in the memory note, and the note says so itself. Separately, FL-30 :310 says the stitched chart "was never loaded in Clone Hero", so the whole stitched-chart game claim rests on FL-27's code reading, not a run.
**Ruling:** none found; the user's 2026-09-27 flip-bug words are on a card, quoted in the memory note, not read under `## Confirmed` here.
**Open:** whether the 16 ms figure is true needs an ffmpeg run on an ogg (named only; not run).

### L-F15-06
**Verdict:** CONFIRMED, with one fix to the row's quoting.
**Homes as found:**
- MEM\hooks-never-guess.md front matter "TOP RULE: a hook never fills in a value it was not told or cannot read from an exact source; no defaults, fallbacks or prose-matching." Body: "The user, 2026-11-18: 'any hook that requires guessing is unacceptable.'" (a note, not a filed ruling). It also says a rule the user set is not a guess.
- MEM\MEMORY.md:3 (index line).
- HY\docs\superpowers\plans\2026-10-25-hooks-from-scratch.md:108 "Each rule has one home. The numbers (100, 150, 20, 30, 5, 10) sit at the top of `progress.py` and nowhere else in code; the docs quote them". This is a rule about where numbers live, not about hooks filling in values. It is a different question from the note's, so it is not a second home of the same rule; the row called it "same kind of rule for numbers" which overreaches slightly.
- BP:109 "never guess a way around a hook": a different question (an agent not routing round a refusal).
**Splits:** None between homes. The note's rule is a memory note with no hook, test or doc page behind it; nothing in the repo or gates states it as a rule for hook code. I did not audit the hooks for guessed values (the row's Open still stands).
**Ruling:** rulings\*.md grep "guess": code-and-engine.md:10 "python? why are you testing python code against a c++ app? i need real numbers if you want me to choose something, not guesses" (about timings, not hooks), :64 (omitted long line, not read), README.md:57 (not read). None is about hooks, so none names both sides of anything here.
**Open:** whether any hook today fills in a value itself is untouched; it needs a read of every hook (not done).

### L-F15-07
**Verdict:** CONFIRMED-CHANGED. The row's development.md cite is wrong; the owner and tests are right.
**Homes as found:**
- Code owner: HY\src\app\allocator.h:25-28 "Whether this process's malloc and free really go to mimalloc. False when mimalloc.dll loads after another DLL that allocates ... or under the Debug runtime ... For tests." `bool malloc_redirected();`. allocator.cpp:34 `return mi_is_redirected();`, :48 `return false;` (the no-mimalloc branch).
- Callers: tests\test_library_jobs.cpp:739 `CHECK(hydra::app::malloc_redirected())` (guarded by `NDEBUG && HYDRA_MIMALLOC`, comment at :735-736), :770 `REQUIRE`; tests\ui\uitest_engine.cpp:157 returns false with "hydra_uitest: malloc is not going to mimalloc; mimalloc.dll must come first in the exe's imports". Only these three call sites exist in src, tests, tools, docs.
- MEM\mimalloc-link-order.md body: "after any CMake change near mimalloc, run an exe with `MIMALLOC_VERBOSE=1` and look for 'malloc is redirected', or read `dumpbin /imports`" and, in the same paragraph, "`app::malloc_redirected()` ... says it in code, and tests/test_library_jobs.cpp and tests/ui/uitest_engine.cpp check it."
- HY\docs\development.md:231-238 describes mimalloc, the two DLLs and `hydra_use_mimalloc`; it does NOT name `malloc_redirected` (the row's "231-236" cite). The redirect tests are mentioned at development.md:337-340 (ASan build leaves them out "and so is the GUI runner's start check for it") and :373 ("the mimalloc tests fail").
- Bisect rule: MEM\measure-small-explain-first.md body ("Bisect by merging one branch ..." the line is cut in my output) and MEM\MEMORY.md:53 "bisect on exact base". grep "bisect" in docs\agents finds nothing, and in MEM (not archive) only MEMORY.md:53 and that note. So it is a memory-only rule, as the row said.
**Splits:** the manual check in the note (an environment variable and dumpbin, run by a person after a CMake change) and the in-code check (`malloc_redirected`, run by two test programs, one at every GUI runner start) answer "is malloc redirected in this exe". They differ for: a Debug build (code says never redirected; the note's manual check would also read "not redirected"); an ASan build (tests skip it, development.md:337-340); a one-off exe that no test links. The note itself names the in-code check, so it is a two-routes pointer, not a drift. The dumpbin path in the note names MSVC 14.44.35207 (a version-pinned path; whether it exists was not checked).
**Ruling:** none found.
**Open:** whether the string "malloc is redirected" is what `MIMALLOC_VERBOSE=1` prints needs a run of an exe (named, not run).

### L-F15-08
**Verdict:** CONFIRMED (code values and the README text match the row; line numbers shift a little).
**Homes as found:**
- C:\Users\Patrick\Downloads\Hydra\video-tools\frames.py:60 "Codecs that never go to the card, whatever anyone asks for. Only h264 is in here, and the reason is correctness, not speed."; :74 `NEVER_NVDEC = {"h264"}`; :213-215 comment "4 was enough there; 16 is a multiple of every alignment a video decoder is likely to want", `CROP_ALIGN = 16`.
- video-tools\README.md:96 "`--decoder cpu|nvdec` ... never for h264, which stays on the processor even under `--decoder nvdec`"; :508-510 "h264 is a rule rather than a preference: it never goes to the card ..."; :541-544 "the rectangle's four amounts are now rounded down to multiples of 16"; :88 `--speed FRACTION` ("Never applied for you: a run without it fails to align"); :642-648 "Runs played below full speed" with `py fcvideo.py "<the video>.mkv" --speed 0.5`.
- MEM\video-decode-ceilings.md: "NVDEC ... GPU crop does not pay ... ties CPU on h264"; the ~3,000 fps h264 CPU ceiling is a decode-only figure; README:524-527 gives 2,617 (CPU decode + crop) and 2,798 (NVDEC with crop) for h264 under a different measure. Different measurements, not a clash.
- C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:53 "A half-speed recording is read from the original file with `--speed 0.5`, never a re-encode."; :55-56 "Never widen the search band ... the targeted search, which the tool runs for you". MEM\MEMORY.md:51 (index).
**Splits:**
- The skill and the memory note tell the invoker what to pass; the README and frames.py state that the tool enforces it (h264 refused for the card, crop rounded). They answer one question for two actors; both agree on the value today.
- SKILL.md:53 "never a re-encode" versus README:174 and :477, which list a layout `splax-halfspeed` covering "both the original half-speed recording and the sped-up re-encode of it". The README's own rule is :56 "Never re-encode or rewrite the video"; the re-encode in :174 is a recording someone else made. Not a split in rule; a reader could mis-read it.
- Delete rule: video-tools is a separate repo outside HY, so no `worktrees` root of BP:43 covers it. A recursive delete there by an agent is refused by the hook (reading BP:43 only; delete_rule.py not reopened by me).
**Ruling:** none found.
**Open:** none by reading. Whether the NVIDIA h264 parser fade fault (frames.py comment: 292 frames differ in the last 45 s of the Swim file) still holds needs a run on that file (not run).

### L-F15-09
**Verdict:** CONFIRMED-CHANGED (the staging half is now read and holds; the release half stays a weak split).
**Homes as found:**
- MEM\windows-installer.md: "`gh release create --target` needs the full SHA; a short one gives HTTP 422." and "Staging always goes through `cmake --install` of the ship preset into a clean `stage` folder ... The script fails if a .db or settings .ini leaks into the stage."
- HY\installer\build_installer.ps1:86 `$leaked = Get-ChildItem $stage -Recurse -Include *.db, *_settings.ini, *_ui.ini`; the row's "78-91" is close. Also throws if a `.pdb` leaks (~:75-77) and checks no shipped exe holds a repo path. The note says "settings .ini"; the script's globs are `*_settings.ini` and `*_ui.ini` (also `*.db`). The note's "settings .ini" is a loose paraphrase, and the `_ui.ini` and `.pdb` guards are not in the note.
- HY\CMakeLists.txt:21 `project(Hydra VERSION 3.1.2 LANGUAGES C CXX RC)`; build_installer.ps1 parses it by regex `project\(Hydra VERSION (\d+\.\d+\.\d+)` ("single source of truth in CMakeLists.txt"). The note's version-bump trap uses "2.10.1" as its example (an old number), the real one is 3.1.2.
- HY\docs\development.md:1011 `-GhArgs '--draft','--target','main',...` passes the branch name `main`. GATES\publish.ps1 has no "target" or "422" text (grep returned none), so the gate does not convert a branch to a SHA.
**Splits:**
- Note "needs the full SHA" versus development.md:1011 `--target main`. A branch name is not a short SHA, so both can hold; the note does not say a branch name is allowed. Differ for an operator who reads only the note and rewrites the command to a SHA, or one who reads the doc and trusts a branch name.
**Ruling:** none found.
**Open:** whether `--target main` works needs a real `gh release create` (not run; that posts to GitHub). Whether a short SHA gives 422 is a GitHub-side fact, unchecked.

### L-F15-13
**Verdict:** CONFIRMED-CHANGED. The refusal tags, the 300 cut and the two tool tests exist as the row says. The row's "SHELL_TOOLS versus Bash only" pair is not a code split: the Bash-only test is inside the rule and the caller sends both shells in.
**Homes as found:**
- GATES\bash_windows_rule.py:210-229 PARTS tags `[length]`, `[backslashes]`, `[cmd]`, `[ref]`, `[path quote]`; :230 `TAGS = tuple(tag for tag, _, _ in PARTS)` (the tag list is derived from PARTS in code); the `[length]` text uses `% MAX_CHARS`.
- GATES\common.py:47 `"reason": reason[:300]` (a literal 300 in the refusal log). tests\test_bash_windows_rule.py:307 `assertLess(reason.index("Instead:"), 300, "the refusal log keeps the first 300 characters")`: a second literal 300 in a test, not tied to common.py.
- GATES\README.md:81 (omitted long line, not read by me, so the row's "README:81 names the 300" is unchecked).
- GATES\progress.py:20 `SHELL_TOOLS = ("Bash", "PowerShell")`; GATES\bash_windows_rule.py:233-236 `reasons()` returns [] unless `tool_name == "Bash"`; GATES\percall.py:146-151 runs delete_rule, `_library_refusal`, bash_windows_rule and media_rule for every call whose tool is in `progress.SHELL_TOOLS` and which is not a status call. So PowerShell calls reach the rule and are passed by it. BP:33 (the five shapes, "Run commands with the PowerShell tool") agrees with this.
- test_bash_windows_rule.py:140 builds a status line from `progress.STATUS_PREFIX`, so that test ties the status-call exemption to the code owner (a derive-once pattern).
- tests\test_bash_windows_rule.py:50,57,77,... use `rule.MAX_CHARS` by reference. The shapes in BP:33 are not held by any test (no test reads the preamble; test_docs_numbers.py:192-196 holds only the number, per the ledger and not reopened).
**Splits:**
- The Bash-only check sits in the rule; the shell check sits in progress.py and percall.py. Two code places answer "which tools are shell tools", but they answer two questions (a Bash-only rule, and a "which calls get the shell checks" gate). They differ for a PowerShell call: percall passes it through, the rule ignores it. By reading, no drift.
- Literal 300 in common.py:47 and test_bash_windows_rule.py:307: a code value copied into a test as a number (K1).
**Ruling:** none found.
**Open:** README:81 and the tag list in BP:33 not compared by me; the five-shape count in BP:33 matches PARTS (five tags) by reading.

### L-F15-14
**Verdict:** CONFIRMED (homes read; one extra copy found in test_media_rule.py).
**Homes as found:**
- GATES\media_rule.py:6 `MEDIA_ROOT = r"C:\ytdl"  # the main session's free media folder (D189)`; :48 uses it; :53 the reason text "ffmpeg or yt-dlp would write %s, not a literal absolute path inside %s (media guard, D189 and D273 in ...". GATES\README.md:21 states the rule and names `MEDIA_ROOT`, not the folder.
- Tests: test_media_rule.py hard-codes `C:\ytdl` in at least 8 places (:54,55,58,59,62,65,66,69,73-75,80,86,87,112,152,176,205) and asserts `assertIn(r"C:\ytdl", reason)` at :80; test_refusals.py:61-67 hard-codes `C:\ytdl\in.webm` and "media guard". `MEDIA_ROOT` appears in tests only 3 times (test_media_rule.py). So the folder name is copied by literal into two test files. test_media_rule.py:80 holds the reason text to the folder by literal, not by `MEDIA_ROOT`.
- Run modes: GATES\common.py:54-58 `run(...)` "mode 'closed': a crash denies the call. mode 'loud': a crash prints a systemMessage"; `if mode not in ("closed", "loud"): raise ValueError`; README:97-98 states the same two meanings in prose ("closed" and "loud"). test_common.py:84-110 not reopened by me.
- Background wordings: GATES\agent_stop.py:46-55 `_BACKGROUND_STARTED` (four wordings: "was moved to the background (ID: ", "did not complete within its N s timeout and was moved to the background (ID: ", "running in background with ID: ", "was manually backgrounded by user with ID: ") and `_TASK_ENDED` (the `<task-notification>` tags); the comment says they are "as read from real agent transcripts (gates/README.md, the agent_stop paragraph)". tests\test_agent_stop.py:100-126 builds records in two of those wordings and the notice (`moved_record`, `asked_record`, `notice_record`), each with a comment "worded as a real transcript has it". The other two wordings are not built there (by this read of :100-126). README:41 describes the same cases in prose. My own session's notice for a timed-out command has the "did not complete within its 120s timeout and was moved to the background (ID: ..." shape, matching the second pattern.
- BP:77 "Never use run_in_background". The hook side: agent_stop.py treats a backgrounded command as "parked" (a tolerated case); shell_output.py skips the trim for `run_in_background` (per the row, not reopened by me); I found no refusal of `run_in_background` in progress.py, percall.py or agent_stop.py (grep over agent_stop, shell_output, progress, percall for run_in_background found only comments and the agent_stop and test lines above).
**Splits:**
- BP:77 forbids background runs; agent_stop.py and README:41 plan for one (an agent that stopped to wait is "parked"). Differ for an agent that did background anyway. No hook refuses it.
- Folder `C:\ytdl` is owned by `MEDIA_ROOT` and copied by literal into two test files, so a change to the root edits the code and about 18 test lines.
- Wordings in agent_stop.py are copied from real transcripts; only two of the four are copied again in the test.
**Ruling:** none found.
**Open:** whether Claude Code still emits these four wordings needs a live transcript (not run). Hook gap: yes, GATES\progress.py, percall.py (no refusal of `run_in_background`, though BP:77 forbids it).

## Ids not reached
None. All eight ledger ids were opened: L-F15-03, L-F15-04, L-F15-06, L-F15-07, L-F15-08, L-F15-09, L-F15-13, L-F15-14. Not read in full: GATES\README.md:81, rulings\code-and-engine.md:64, rulings\README.md:57, tests\test_common.py:84-110, delete_rule.py, and the cut-off line in MEM\measure-small-explain-first.md.
