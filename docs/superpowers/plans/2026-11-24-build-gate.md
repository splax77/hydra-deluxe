# The build gate, from scratch

Date: 2026-11-24. Task BG-P1, session 2eba124d. This plan replaces nothing: the plan-run gate is already gone from both repos (Hydra `a7989e6d2`, hooks `b0ee4a3`). Nothing here rebuilds it.

## The short version

A session asks for a build pass and says what it wants to build: the folder, the preset and the targets (one or more names, as `build_cpp.ps1 -Target` takes them; P28). The gate answers one question: does that folder already hold a passing build that covers what is in it now? If yes, the pass is refused with one line saying which build covers it. In every other case the pass is handed out, exactly as today.

The gate never counts builds. It never asks for a typed line. It never looks at a plan, a phase or a merge. A move of `main` never changes its answer, because it compares the branch with its own last build, not with `main`.

The three-at-once limit and the one-at-a-time limit stay where they are, in `tools/build_slot.ps1`, unchanged.

## What the user asked for

These are the user's words, as the main session's brief quotes and confirms them (premises P1 to P7).

- "the whole point of this work was for different kinds of builds to be recognized as different while still limiting unnecessary builds."
- "when multiple sessions are working at the same time, main is moving a lot, and every time main moves the session thinks a new build is needed. so we get stuck in a loop."
- "if I have to manually approve everything then all this work was pointless"
- "a gate that breaks the instant you build it isnt a good gate"
- "why are you making this so complicated?"
- From the Excel export session's card: "you dont need to test the new combination every time main moves."

## The cause of the loop

No code causes the loop. It is instruction text, and a missing sentence.

I looked for every place that answers "does this tip need a build". There is none in code. `New-BuildPass` in `tools/build_slot.ps1` (lines 197 to 224) hands out a pass after four checks: the session id is usable, a preset is named, the session holds no other pass, and a slot is free. It never looks at what will be built (P8). The hook in the hooks repo only refuses the tool to agents (`gates/percall.py` lines 231 to 234); for a main session it reads nothing (P9). `build_cpp.ps1` claims the pass and builds (P10). The precheck does not cause it either: its range `main...<tip>` starts at the fork point git works out, so a move of `main` alone does not change the test list of an untouched branch (P11).

Three pieces of text produce the loop together.

The first is `docs/agents/derive-once-review.md` line 202: "When main moves under a reviewed branch, merge main into the branch." It is unconditional. Each move of `main` therefore puts a new merge commit on the branch, which is a new tip (P12).

The second is the build text, which is written per tip and names no exception. Step 2 on line 209 of the same page says "Build the branch and run the merge's tests yourself". Line 222 says "If the reviewer's fixes touch C++, build that tip again" and "Then merge that tip". `model-postures.md` line 11, which the main session gets on every prompt, says "build its branch" and "After sign-off, rebuild if the reviewer changed code, run the merge's tests, then merge" (P13, P14).

The third is the gap. No page says a passed build still covers the branch after a clean merge of `main`. The review side has that sentence (line 202: "A conflict-free merge needs nothing more"). The build side has none. The only place it is written down is a memory note, which is a session's note and not a source.

So a session that follows the pages merges `main` in, sees a tip no build covers, and builds. Then `main` moves again. The card for the Excel export session records the result: nine builds against a planned two, caused by "the session's own rule that every move of main needs a retest" (`docs/sessions/2026-11-23-c8e04ae2.md` line 15).

The fix is therefore mostly a rule change on three pages, plus one small check so the rule does not depend on a session remembering it.

## The design

### One question, one owner

The question is "does this folder already hold a passing build that covers what is in it now". Nothing answers it today, in `src/core`, `src/app`, `src/ui`, `tools` or the hooks repo. The search was for every reader of the pass, the lock, `HYDRA_BUILD_PASS` and `hydra_build_pass` in both repos; the hits are the files in P8 to P10 and their tests.

The new owner is one function in a new hooks file, `gates/build_rules.py`. It sits beside `gates/review_rules.py` because it needs two answers that file already owns, and it calls them instead of copying them:

- Which files count as code: `_is_code` and `_code_changes` in `review_rules.py` (P15).
- Whether a merge is git's own clean merge: `git merge-tree --write-tree`, the same call `check` in `review_rules.py` makes (P16).

### What gets recorded

After every build it runs, the main checkout's `tools/build_slot.ps1` appends one line to `build_results.jsonl` in the repo's git common dir. That is the folder where `realapp.jsonl` already lives, on the same pattern: Hydra's tool owns the file's name and fields, and the hooks repo only reads them (P17).

A line holds: the worktree folder, the commit that was checked out, the paths git reported as changed or untracked when the build started, the preset, the targets, the build folder, the exit code, and whether the preset compiles the commit's own files or switched copies. The last field matters for the mutation preset. Its build compiles copies that `rewrite.py` switches for whichever source files the session named, so the commit alone does not say what was built (P18).

The line is a description of what the build folder holds. It is not a count and nobody adds it up.

### The rule

A build is covered, and its pass refused, only when all of these are true. If any one is false, or cannot be read, the pass is handed out.

1. The newest record for this folder and this preset that lists the targets asked for is a passing one, and its build folder still exists. A passing record that a later failed build of the same folder, preset and targets follows covers nothing, because that build may have left the folder half-written. Folders are compared after normalizing slashes and case, as Windows treats them.
2. That build compiled the commit's own files, and no code file was changed or untracked when it started.
3. No code file is changed or untracked in the folder now.
4. Every commit on the branch since the built commit is one of two kinds. Either it changes no code file (this kind waits for question 1). Or the commit is a merge that is not itself part of `main`, it is git's own clean merge (the merge of its two parents, as `git merge-tree --write-tree` makes it), and its second parent is already part of `main`. A commit that is part of `main` is never in this kind, so the main checkout's own merges of code, and a detached worktree sitting on a `main` commit, are never covered by it (case 7). A merge commit that changes a code file against its first parent can only be covered by this second kind; any other commit is judged by the first kind.

Rule 4's second kind is the user's sentence about not testing the new combination every time `main` moves. A merge fixed by hand does not match git's own merge, so it is not covered, and a build is allowed.

"Since the built commit" follows the branch's own line of commits (git's first-parent chain: at each merge, the side the branch was on). If the built commit is not on that line, the build is not covered.

### What makes builds different kinds

A preset has its own build folder, so a `mutation` build is never mistaken for a `default` build. A different target is a different build too: a folder that holds a passing `hydra_tests` does not cover a request for `Hydra`. A request naming several targets is covered only when one record lists all of them. A pass always names its targets; there is no "all default targets" request, so the gate never has to guess what an empty list built. A session that built the tests and then needs `Hydra.exe` for a real-app check gets its second pass.

A mutation build is never refused, because rule 2 never holds for it. I dropped the brief's idea that a mutation build must wait for a passed default build of the same tip. Case 6 builds the mutation preset for tests that are already on `main`, where no default build may be on record, so that rule would have refused the first real use.

### It refuses only on proof

A missing record, an unreadable line, a folder the tool was not told about, a git call that fails: each one hands the pass out, with a one-line note saying what could not be read. The gate can fail to save a build. It cannot stop a build that is needed. This is the answer to "a gate that breaks the instant you build it isnt a good gate".

### Where each half runs

The check runs in the hooks repo's `gates/percall.py`, which is one live copy for every session on the machine. The record is written by Hydra's `tools/build_slot.ps1`, from the checkout the session's `hydra` MCP server was started in: `.mcp.json` names `tools/hydra_ui_mcp.py` relative to that folder (P30), and the server runs the `build_slot.ps1` beside it (P19). A main session runs in the main checkout, so it gets the main checkout's copy, whatever old copy a branch carries. The reply prints the build command, which runs that same copy in its existing `-Repo` mode (P20) and keeps the pass in the environment, as today:

`$env:HYDRA_BUILD_PASS='<pass>'; <that checkout>\tools\build_slot.ps1 -Repo <folder> -Preset <preset> -Target <targets>`

A session started in an older worktree prints today's command and writes no record. That is safe: with no record the gate hands the pass out.

A session can still run `.\build_cpp.ps1` inside a worktree under its pass, as today, and so can `installer\build_installer.ps1`. Those builds write no record. The only effect is that the gate cannot refuse a later build of that folder.

### What the gate does not touch

It has no say in a merge. Git's ref gate is unchanged. So removing or changing the build gate is an ordinary reviewed change, and no step of it asks the build gate for anything (case 10).

### The page changes

Line 202 of `derive-once-review.md`, and the sentence at line 166 that says a reviewed branch "has to merge main in to catch up", stop telling sessions to merge `main` into a reviewed branch. `main` takes the branch as git's own clean merge; the ref gate already accepts that (P12). A session merges `main` into its branch only when git reports a conflict, and the hand fix is then a new commit that gets its resolution review and a build.

The build sentences on that page, on `docs/development.md` and in `model-postures.md` gain the missing rule: a passed build covers the branch until the branch itself gains a code commit. A move of `main` is never a reason to build.

## The ten cases

Each case is a test written before the code. K1's tests build real git repos in a scratch folder and write record lines the way the tool does. The gate is done only when all ten pass.

| # | Case | What the session does | What the gate says | What the user sees |
|---|---|---|---|---|
| 1 | Normal build, then merge | Asks for a `default` pass for its branch folder and `hydra_tests`, builds, runs tests, merges | Pass handed out. A second ask on the same commit: "no build needed", naming the build | Nothing |
| 2 | Failed build, fix, rebuild | Builds, it fails, a fix commit lands, asks again | Handed out both times. A failed build never covers anything, so even the same commit may be rebuilt | Nothing |
| 3 | `main` moves several times | Does nothing. If it merged `main` in cleanly anyway and asks | "no build needed": only clean merges of `main` since the passing build | Nothing |
| 4 | Docs-only | `main` gains docs: does nothing. The branch gains a docs commit and asks | "no build needed": no code file changed (waits for question 1) | Nothing |
| 5 | Hand-resolved conflict in code | Merges `main` in, fixes the conflict, commits, asks | Handed out: the merge is not git's own | Nothing |
| 6 | Mutation build of tests on `main` | Asks for a `mutation` pass for the main checkout and `hydra_tests` | Handed out, with or without a default build on record, and again for a second run | Nothing |
| 7 | Tool rebuild on `main` | Asks for a `default` pass for the main checkout and `hydra_precheck` | Handed out when `main` gained code since the last build there; refused when it gained only docs | Nothing |
| 8 | Two sessions; a fourth build | Each asks | Two passes. With three running, the fourth is refused, naming the three holders (today's text) | Nothing, or one sentence that the machine is full |
| 9 | Second build, same session | Asks while its first runs | Refused: "this session already holds one ... One build at a time per session." (today's text) | Nothing |
| 10 | Removing the gate | Lands a hooks change that deletes `build_rules.py` and its call | The ref gate judges it like any code change. With the file gone, every pass is handed out | Nothing |

When a pass is refused as covered, the session carries on with the binaries it has. It tells the user in one sentence only if the user asked for a build.

## Tasks

Six tasks. K1, K2 and K3 are in the hooks repo (`C:\Users\Patrick\.claude`). H1, H2 and H3 are in Hydra. K1, H1 and H3 start together. K2 starts from K1's branch once K1 commits. H2 starts from H1's branch once H1 commits. K3 runs only if question 2 is answered yes, and starts together with K1. The gate is live when K2 and H2 are both on `main`; either order is safe, because the hook hands the pass out when the call names no folder.

### K1: The one function that answers "is this build covered"

Repo: hooks. Owned files: `gates/build_rules.py` (new), `gates/tests/test_build_rules.py` (new). May run: `python -I -m unittest gates.tests.test_build_rules`, that one file only.

First repeat the search in P27, in both repos, and stop and report if anything else answers the question. Then write the tests, one per row of "The ten cases" that this function decides (1 to 7), each on a real git repo built in a scratch folder. Then write `covered(repo_dir, preset, targets)`, which returns whether the build is covered and one plain sentence saying why. It reads `build_results.jsonl` in the repo's git common dir. It calls `review_rules`' own `_code_paths`, `_code_changes` and `_is_code` for "which files are code", and `git merge-tree --write-tree` for "is this git's own merge". It holds no list of its own. The docs-only kind of rule 4 uses premise P21 and waits for question 1; on a "no", that kind and its tests are left out.

Acceptance criteria:

- Case 1: no record gives "not covered"; a passing record at the same commit, same preset, same targets gives "covered", and the sentence names the commit and the build folder.
- Kinds: the same record gives "not covered" for another preset, and for a target the record does not list.
- Case 2: a record with a non-zero exit code gives "not covered" at the same commit.
- Case 3: three clean merges of `main` above the built commit give "covered". The test fails if any one of the three is replaced by a merge of a branch that is not part of `main`.
- Case 4: a commit that changes only a file `_is_code` rejects gives "covered"; one that changes a file it accepts gives "not covered". The test reads the two paths' kinds from `_is_code`, not from a typed list.
- Case 5: a merge whose tree differs from git's own merge gives "not covered", both for a real conflict and for a clean merge with an extra hand edit.
- Case 6: a record marked as switched copies gives "not covered" at the same commit.
- Case 7: a folder on `main` whose branch gained a merge with code gives "not covered", even when the merge's second parent is part of `main`; one that gained only a docs commit gives "covered". The same holds for a detached worktree on a `main` commit.
- Newest record: a passing record followed by a failed record for the same folder, preset and targets gives "not covered"; a failed record for other targets does not change the answer.
- Several targets: a record listing `hydra_tests` covers a request for `hydra_tests`, and not one for `hydra_tests,Hydra`.
- Folder names: the record's folder written with other slashes or case still matches the folder asked about.
- Dirty code: a changed or untracked code file, in the record's start list or in the folder now, gives "not covered". A changed docs file does not.
- Proof only: a missing record file, a line that is not JSON, a record whose build folder is gone, a built commit that is not on the branch's own line, a folder that is not a git repo, and a `.derive-once-gate` file that `_code_paths` refuses (it raises) each give "not covered", and the sentence says what could not be read. One test per condition.

### K2: Ask the function when a main session asks for a pass

Waits for: K1's commit (branch cut from K1's). Repo: hooks. Owned files: `gates/percall.py`, `gates/tests/test_percall_build_gate.py` (new), `gates/tests/test_percall_key.py`, `gates/README.md`, `model-postures.md`. May run: `python -I -m unittest gates.tests.test_percall_build_gate` and `gates.tests.test_percall_key`.

Write the tests first. In the main-session branch of `percall.py`, a call to the build-pass tool that names a folder, a preset and a target is put to `build_rules.covered`. Covered: deny, and the reason is the function's sentence followed by "No build is needed. A move of main is never a reason to build." Anything else passes untouched. The agent refusal stays first and unchanged. In `model-postures.md`, line 11's build sentences gain the rule from "The page changes".

Acceptance criteria:

- A main-session call on a covered folder is denied with the function's sentence; the test builds the repo and record the way K1's tests do.
- A main-session call that is not covered passes, and the hook's output is the same as for a call with the hook's build branch removed.
- A call with no folder argument passes. A call when `build_rules.py` cannot be imported, or when it raises, passes, and the output carries one line naming the error. One test each.
- An agent's call is still refused with today's text (the existing test stays green).
- Case 10: a test moves `main` in a scratch hooks-shaped repo to a commit that deletes `gates/build_rules.py`, and `review_rules.explain_main_move` returns the same answer it returns for any other reviewed code change. A second test shows `build_rules` is imported by `percall.py` only, by reading each gate module's imports through `ast`.
- `gates/README.md` says in one paragraph what the build branch of `percall.py` does and that `build_rules.py` owns the rule. It restates no rule.

### K3: Name the files the build reads that the code list misses

Runs only if question 2 is answered yes. Repo: hooks. Owned files: `gates/review_rules.py`, `gates/tests/test_review_rules.py`. May run: `python -I -m unittest gates.tests.test_review_rules`.

Write the test first: each file question 2 names is accepted by `_is_code` with no `gate:` lines, and `src/x.cpp` and `docs/x.md` keep their answers. Then add the files question 2 names to `BUILTIN_CODE` (P26, P31). Nothing else in the file changes. Hydra's `.derive-once-gate` has only prose, so it keeps using the built-in list; adding `gate:` lines to it would replace the list instead.

Acceptance criteria:

- The new test is red before the edit and green after, and the whole of `test_review_rules.py` stays green.
- `build_rules.py` needs no change: it reads `_is_code`.

### H1: The pass names what it builds, and the build writes its result

Repo: Hydra. Owned files: `tools/build_slot.ps1`, `tools/test_build_pass.ps1`. May run: `pwsh -NoProfile -File tools\test_build_pass.ps1` (it compiles nothing).

Write the tests first. `-NewPass` takes `-Repo` and `-Target` (one or more names, comma-separated, as `build_cpp.ps1` splits them; P28) beside `-Preset`. All three go on the lock as new lines. `Read-BuildSlot` reads only four keys today (P29), so it learns `repo=` and `target=`, and the claim in `Enter-BuildPass` rewrites the lock's lines, so it carries both over (P29). The printed command becomes `$env:HYDRA_BUILD_PASS='<pass>'; <this checkout>\tools\build_slot.ps1 -Repo <folder> -Preset <preset> -Target <targets>`, with the pass in the environment as today (the `-Repo` mode claims the pass from it). The `-Repo` mode refuses a folder or target other than the pass's, before it claims the pass or builds. When its build ends, pass or fail, it appends the record line described in "What gets recorded". The switched-copies field is read from the built folder's own preset through `Read-CMakePresets` (P18). The slot rules are not touched.

Acceptance criteria:

- A pass asked for with a folder and target prints a command naming both and setting `HYDRA_BUILD_PASS`, and the lock holds both; read through `Read-BuildSlot`, before and after the pass is claimed.
- A pass for two targets (`a,b`) lets `-Repo` build `a,b`; `-Repo` with a target the pass did not name exits non-zero before building; the record lists the targets built.
- The `-Repo` mode run with another folder, or another target, exits non-zero before any build starts and writes no record line.
- After a build that exits 0 and one that exits non-zero (the test's stand-in `build_cpp.ps1`, as the file already uses), the record gains exactly one line each, and each field equals what git and the command line say: commit against `git rev-parse HEAD`, changed paths against `git status --porcelain`, exit code against the stand-in's.
- A preset whose cache sets `HYDRA_MUTATION` on writes the switched-copies field true; `default` writes it false. The test reads the real `CMakePresets.json`.
- Case 8 and case 9 stay green: the existing checks at lines 136 to 150 of the test file are not edited (P22).
- A `-NewPass` with no `-Repo` still hands out a pass, as today, so a session on an old tool reply is not stuck.

### H2: The tool takes the folder and the target

Waits for: H1's commit (branch cut from H1's). Repo: Hydra. Owned files: `tools/hydra_ui_mcp.py`, `tools/test_hydra_ui_mcp.py`. May run: `py -I -m unittest tools.test_hydra_ui_mcp.BuildPassTool`.

Write the tests first. `hydra_build_pass` gains `repo` and `target` arguments with no defaults, and hands them to `build_slot.ps1`. `target` is one or more comma-separated names, and the tool refuses an empty one before it asks the script (the script itself still hands out a pass with neither, for an old caller; H1's last criterion). Its description says what each is and points at `tools/build_slot.ps1` and the hooks' `build_rules.py` as the owners; it restates no rule.

Acceptance criteria:

- The tool's schema lists `preset`, `repo` and `target` as required.
- A call returns the command H1 prints, naming the folder and target given; the test runs the real `build_slot.ps1` on a scratch slot folder, as `BuildPassTool` already does.
- A slot refusal still comes back as the reply, word for word.

### H3: The pages stop telling sessions to rebuild

Repo: Hydra. Owned files: `docs/agents/derive-once-review.md`, `docs/development.md`, `docs/agents/agent-rules.md`, `CLAUDE.md`, `docs/agents/integrate.md`, `docs/superpowers/plans/2026-11-24-test-guard.md`. May run: nothing.

Make the edits in "The page changes", including the sentence at line 166 of `derive-once-review.md`. In `docs/development.md`, "Building under Claude Code" gains the new call shape, the one build command and a short paragraph on what the gate refuses, pointing at `build_rules.py`. Rule 14 on `agent-rules.md` gains one sentence and the hooks paragraph under rule 15 names the new check. `CLAUDE.md` changes only if rule 14's short form no longer matches. In the test-guard plan, M1's sentence that its build waits for "question 1 or a typed `allow build`" is replaced: M1's mutation build is handed out like any other (case 6).

Acceptance criteria:

- `derive-once-review.md` no longer tells a session to merge `main` into a reviewed branch when git's merge is clean, and says when it does (a conflict).
- Each of the three build sentences (P13, P14 lists them; `model-postures.md` is K2's) says a passed build covers the branch until the branch gains a code commit.
- No page names a typed line for a build.
- Every build command on the pages names its targets and matches the shape H1's tool prints (`HYDRA_BUILD_PASS` set, then the `build_slot.ps1 -Repo` call); the reviewer checks each against H1's branch.
- No page restates rule 1 to 4; each points at `build_rules.py`.

## Builds this work needs

One. The Hydra tasks change PowerShell, Python and pages, so nothing new compiles. Rule 1 still runs the two `always:` tests at every Hydra merge, and they need a built `hydra_tests`. So the main session builds the `default` preset, target `hydra_tests`, once, on the Hydra branch that joins H1, H2 and H3, and merges that branch once. The hooks tasks need no build.

## Global constraints

- Agents never build and never run `hydra_tests.exe`. Every task hands back uncompiled.
- No task adds a count, a limit, a time-out or a list of file kinds. A task that needs one stops and reports. K3 is the one edit to an existing list, the review gate's own, and only if question 2 is yes.
- No task changes `New-BuildPass`' slot checks or their messages.
- No task adds a typed line.

## Questions for the user

**Question 1. May the gate refuse a build after a commit that changes no code file?**

The real rule it would use is the review gate's own list of code files: anything under `src/`, `tests/`, `tools/`, `third_party/`, `installer/` or `.github/`, plus `CMakeLists.txt`, `CMakePresets.json` and `build_cpp.ps1`. A commit that touches none of those is a "docs commit" (question 2 asks about the few build inputs that are not on that list). From one end: a branch that gains an edit to `docs/UserGuide.md` after its build. From the other: a branch that gains a one-line fix in `src/ui/`. Today a reviewed branch that gains a docs commit already needs no new review.

- Yes (recommended): the first is refused as covered, the second is built.
- No: both are handed a pass. The pages still say a docs commit needs no build, but nothing stops one.

You will see: nothing in Hydra changes. With "yes", a session that asks for a build after a docs commit is told none is needed.

**Question 2. Two kinds of file the build reads are not on the code list.**

The review gate's list is the one answer to "which files are code", and the build gate uses it. Two groups of files feed the build and are not on it.

`Directory.Build.targets` is the root file that makes every Visual Studio project refuse to build without a pass (P26).

`assets/` and `resource/` hold the preview art, shaders, fonts and icons. A build copies both folders next to `Hydra.exe` and `HydraPathBuilder.exe`, and the app reads them from there (P31). The tests read `testdata/` straight from the source folder, so it needs no build and stays off.

Under question 1, an edit to any of these after a build would count as a "docs commit": a session that changed a shader and asked for a build would be told none is needed, and the exe would keep the old copy. From the other end, `src/ui/x.cpp` is already on the list and is built.

- Add them to the list (recommended): an edit to any of them needs a code review, as an edit to `src/` does, and allows a build. Task K3 does it.
- Leave them: an edit to them needs no review, as today, and the gate treats it as a docs commit, so the build is refused as covered.

You will see: nothing in Hydra changes. With "add them", the next change to a file under `assets/` or `resource/`, or to `Directory.Build.targets`, needs a reviewer, where today it needs none.

## Out of scope

- Recording builds a session starts with `.\build_cpp.ps1` directly inside a worktree.
- Any check on merges: today nothing in the ref gate asks for a build, and this plan leaves that as it is, so the build gate can never block a merge. Nothing the user asked for needs it.
- Refusing mutation builds.

## Prior art

Merge queues skip a re-test when the code that would land is the code CI already ran on: [Trunk's direct merge](https://docs.trunk.io/merge-queue/optimizations/direct-merge-to-main) and [Mergify's direct merge](https://docs.mergify.com/merge-queue/direct-merge). Build caches key a result on what went in, not on when it was asked for: [bazel-remote](https://github.com/buchgr/bazel-remote) keys an action's result on the hash of the action. A skip keyed on the tree is an open request for GitHub Actions, not a shipped feature ([skip-duplicate-actions #329](https://redirect.github.com/fkirc/skip-duplicate-actions/issues/329)). This plan takes one idea from them: the key is what was built. Nothing here depends on those pages being right.

## Promise map

| Promise | Plan line | Task | The check that proves it |
|---|---|---|---|
| A covered build is refused with one line naming the build | The rule; case 1 | K1, K2 | K1 "Case 1"; K2 first criterion |
| A different preset or target is a different build | What makes builds different kinds | K1 | K1 "Kinds" |
| A failed build never covers anything | Case 2 | K1 | K1 "Case 2" |
| A move of `main` never allows or needs a build; clean merges of `main` stay covered | The rule, 4; case 3 | K1 | K1 "Case 3", which fails if any merge is not of `main` |
| A docs-only commit stays covered (question 1) | The rule, 4; case 4 | K1 | K1 "Case 4" |
| A hand-fixed merge is not covered | The rule; case 5 | K1 | K1 "Case 5", both kinds of hand change |
| A mutation build is never refused | What makes builds different kinds; case 6 | K1, H1 | K1 "Case 6"; H1 fourth criterion |
| A tool rebuild on `main` is allowed when `main` gained code | Case 7 | K1 | K1 "Case 7" |
| Three builds at once; the fourth is refused | Case 8 | H1 | Existing check, `tools/test_build_pass.ps1` line 142, kept |
| One build at a time per session | Case 9 | H1 | Existing check, line 149, kept |
| The gate refuses only on proof; anything unreadable hands the pass out | It refuses only on proof | K1, K2 | K1 "Proof only", one test per condition; K2 third criterion |
| Removing or changing the gate is never blocked by it | What the gate does not touch; case 10 | K2 | K2 "Case 10", both tests |
| No typed line for any build | The short version; Global constraints | H3, K2 | H3 third criterion; K2 denial text holds no typed phrase (first criterion's test reads it) |
| The gate counts nothing | The short version; Global constraints | K1 | Reviewer reads `build_rules.py` for a count or limit; K1's tests pass with any number of earlier record lines ("Case 2" adds extras) |
| One copy of the gate runs: `main`'s | One copy of the gate runs | H1, H2 | H1 first criterion (the printed command names the main checkout's script); H2 second criterion |
| A pass builds only the folder and target it names | H1 | H1 | H1 second criterion |
| Every build through the printed command writes one true record line | What gets recorded | H1 | H1 third criterion |
| A build input the code list misses (assets, resource, `Directory.Build.targets`) is not wrongly called covered | Question 2 | K3 | K3 first criterion, a test red before the edit; runs only if question 2 is yes |
| The newest record decides; a later failed build voids an earlier pass | The rule, 1 | K1 | K1 "Newest record" |
| A request naming several targets is covered only when one record lists all | What makes builds different kinds | K1, H1 | K1 "Several targets"; H1 second criterion |
| The gate holds no file list of its own | One question, one owner | K1 | K1 "Case 4" reads kinds from `_is_code`; reviewer greps `build_rules.py` for a path pattern |
| The pages no longer say to merge `main` in on a clean merge, or to build per tip | The page changes | H3, K2 | H3 first and second criteria |
| The slot rules and their messages do not change | Global constraints | H1 | H1 fifth criterion |
| A pass asked for the old way is still handed out | H1 | H1, K2 | H1 last criterion; K2 "no folder argument" |
| The work itself uses one build | Builds this work needs | main session | The main session's merge report names the one build command |
| The user sees nothing for ordinary work | The ten cases | K2, H3 | K2 first and second criteria; H3 third criterion |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The user wants kinds of builds told apart and unnecessary builds limited: "the whole point of this work was for different kinds of builds to be recognized as different while still limiting unnecessary builds." | ruling | the main session's brief for `BG-P1`, quoting the user and confirming it against the transcript | checked 2026-11-24 |
| P2: The user's complaint: "when multiple sessions are working at the same time, main is moving a lot, and every time main moves the session thinks a new build is needed. so we get stuck in a loop." | ruling | the main session's brief for `BG-P1` | checked 2026-11-24 |
| P3: "if I have to manually approve everything then all this work was pointless" | ruling | the main session's brief for `BG-P1` | checked 2026-11-24 |
| P4: "a gate that breaks the instant you build it isnt a good gate" | ruling | the main session's brief for `BG-P1` | checked 2026-11-24 |
| P5: The limits: "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." | ruling | `docs/agents/rulings/agents-and-workflow.md` line 10, under Confirmed | checked 2026-11-24 |
| P6: "one build per session" means one at a time, never a cap on the total: the user answered "One at a time" to "Your 2026-11-07 'one build per session': one build in total, or one at a time?" | ruling | `docs/agents/rulings/agents-and-workflow.md:8`, filed on main in `36c29bba8` and not yet on this branch; the main session's brief for `BG-P1` quotes it | checked 2026-11-24 |
| P7: "you dont need to test the new combination every time main moves." | ruling | `docs/sessions/2026-11-23-c8e04ae2.md` line 29 | checked 2026-11-24 |
| P8: A pass is handed out after four checks, none about what is built; a session holding a pass is refused a second; a fourth pass is refused | code | `tools/build_slot.ps1:197-224` | checked 2026-11-24 |
| P9: The hook refuses the pass tool to agents and reads nothing for a main session | code | `C:/Users/Patrick/.claude/gates/percall.py:222-237`, `C:/Users/Patrick/.claude/gates/percall.py:250-263` | checked 2026-11-24 |
| P10: `build_cpp.ps1` claims the pass for its preset, builds and releases; it judges nothing else | code | `build_cpp.ps1:79-144` | checked 2026-11-24 |
| P11: The precheck's range starts at git's fork point of the two sides | code | `tools/precheck/range.cpp:28-33` | checked 2026-11-24 |
| P12: The page tells sessions to merge `main` into a reviewed branch whenever `main` moves, and the ref gate accepts git's clean merge without it | code | `docs/agents/derive-once-review.md:200-202` | checked 2026-11-24 |
| P13: The page's build steps are per tip with no exception for a move of `main` | code | `docs/agents/derive-once-review.md:209`, `docs/agents/derive-once-review.md:222`, `docs/development.md:112-142`, `docs/agents/agent-rules.md:20` | checked 2026-11-24 |
| P14: The main session's per-prompt text says build the branch, and rebuild if the reviewer changed code | code | `C:/Users/Patrick/.claude/model-postures.md:11` | checked 2026-11-24 |
| P15: `review_rules.py` owns which files are code, and the paths that differ as code between two commits | code | `C:/Users/Patrick/.claude/gates/review_rules.py:18-22`, `C:/Users/Patrick/.claude/gates/review_rules.py:158-186`, `C:/Users/Patrick/.claude/gates/review_rules.py:211-218` | checked 2026-11-24 |
| P16: `review_rules.check` asks git for its own merge with `merge-tree --write-tree`, and exit 1 means a conflict | code | `C:/Users/Patrick/.claude/gates/review_rules.py:598-611` | checked 2026-11-24 |
| P17: `realapp.jsonl` sits in the git common dir, written by Hydra's tool and only read by the hooks | code | `C:/Users/Patrick/.claude/gates/review_rules.py:47-49`, `C:/Users/Patrick/.claude/gates/review_rules.py:482-486`, `tools/hydra_ui_mcp.py:87-90` | checked 2026-11-24 |
| P18: The mutation preset sets `HYDRA_MUTATION` on, and its build compiles copies switched for the source files the session names; presets are read through `Read-CMakePresets` | code | `CMakePresets.json:70-78`, `docs/development.md:757-768`, `tools/cmake_presets.ps1:8-10` | checked 2026-11-24 |
| P19: The pass tool runs the `build_slot.ps1` beside the server's own file, with the session id from the environment | code | `tools/hydra_ui_mcp.py:64`, `tools/hydra_ui_mcp.py:97-98`, `tools/hydra_ui_mcp.py:798-820`, `tools/hydra_ui_mcp.py:959-967` | checked 2026-11-24 |
| P20: `build_slot.ps1 -Repo` claims the pass itself, runs the named worktree's `build_cpp.ps1`, and exits with its result | code | `tools/build_slot.ps1:362-386`, `tools/build_slot.ps1:248-268` | checked 2026-11-24 |
| P21: The user accepts that a commit changing no code file needs no build | unchecked | the main session's brief for `BG-P1` calls it "a design idea, not approved" | question 1 |
| P22: The fourth-pass and second-pass refusals already have tests | code | `tools/test_build_pass.ps1:136-150` | checked 2026-11-24 |
| P24: The test-guard plan's M1 still says its build waits for a typed `allow build` | code | `docs/superpowers/plans/2026-11-24-test-guard.md:188` | checked 2026-11-24 |
| P25: The two `always:` tests run at every Hydra merge and need a built `hydra_tests` | code | `docs/agents/agent-rules.md:7` | checked 2026-11-24 |
| P26: `Directory.Build.targets` is not on the review gate's code list | code | `C:/Users/Patrick/.claude/gates/review_rules.py:22` | checked 2026-11-24 |
| P27: No other code answers "does this tip need a build" | unchecked | a search of both repos for `hydra_build_pass`, `HYDRA_BUILD_PASS`, `build_slot` and `BUILD_PASS_TOOL` found only the files in P8 to P10, P19 and their tests | K1 |
| P28: `build_cpp.ps1 -Target` takes a list, splits each value on commas, and an empty list builds the default targets | code | `build_cpp.ps1:24`, `build_cpp.ps1:70`, `build_cpp.ps1:115-120` | checked 2026-11-24 |
| P29: `Read-BuildSlot` reads only the keys `pass`, `session`, `preset` and `expires`, and the claim in `Enter-BuildPass` rewrites the lock from fields it names, so an unnamed line is dropped | code | `tools/build_slot.ps1:128-133`, `tools/build_slot.ps1:262-264` | checked 2026-11-24 |
| P30: `.mcp.json` starts the `hydra` server from a path relative to the folder the session starts in | code | `.mcp.json:3-6` | checked 2026-11-24 |
| P31: A build copies `resource/` and `assets/` beside the app's exe in a post-build step, the app reads them from there, and neither folder is on the review gate's code list | code | `CMakeLists.txt:679-691`, `C:/Users/Patrick/.claude/gates/review_rules.py:22` | checked 2026-11-24 |
