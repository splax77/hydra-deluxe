# TOOL-AUDIT-T5-L05 (ledger compiler L05; finders F13, F14, F15)

Compiled from: F13, V13, F13b, V13b, F14, V14, F15, V15 in docs/audit/2026-11-18-tooling-derive-once/reports/. No other report read. Nothing searched again, nothing run. Short names: GATES = C:\Users\Patrick\.claude\gates; HY = C:\Users\Patrick\Downloads\Hydra\hydra-test; BP = HY\docs\agents\brief-preamble.md; TM = HY\docs\agents\this-machine.md; SF = GATES\settings.fragment.json; UT = HY\docs\agents\ui-testing.md; CH = HY\docs\agents\clone-hero-mcp.md; MEM = the memory folder; "RR" = GATES\review_rules.py; "RJ" = GATES\ref_judge.py.

Rulings: no report found any `## Confirmed` line that names both sides of any split below. Where a Confirmed line was found it is quoted in the row. Files the reports say they searched: rulings\review-and-hooks.md and agents-and-workflow.md (F13, read), grep of all rulings\*.md for elevated/administrator/admin window/python/amend/rebase/hand-typed/gh issue/stage/add -A/guess/scroll (F13, F13b, F14, V13, V14, F15 in part).

---

## F13 rows

### L-F13-01: Which git settings and environment does a gate pin so git answers about the right repo and tells no lies about history?
- **Source:** I3-004, I3-005, I3-030; F13 group A; V13 item 1.
- **Kind:** K1, K2, K6, K8.
- **Verdict:** CONFIRMED (V13, by reading).
- **Homes:**
  - RJ:34-35 PINNED: four settings (quotePath=false, showSignature=false, trailer.separators=:, useReplaceRefs=false). Its git() drops GIT_CONFIG_PARAMETERS and GIT_CONFIG_COUNT and sets GIT_NO_REPLACE_OBJECTS=1 and GIT_GRAFT_FILE.
  - RR:60-62 `_GIT_PINS` (three settings, no trailer.separators) and `_GIT_ENV_DROP` (GIT_DIR, GIT_WORK_TREE, GIT_INDEX_FILE, GIT_COMMON_DIR, GIT_PREFIX, GIT_REPLACE_REF_BASE). Comment: "Inputs that could make git answer for another repo or lie about history." RR:135-136 (finder only) "every gate but ref_judge.py calls git through this."
  - HY\tools\check_handoff.py:24-26 (V13; finder said 25-27): same three pins and same six drops word for word, no owner named; comment at 37-38 "so they cannot point it at another repo".
  - GATES\publish_gate.py:51-52 refuses when GIT_DIR or GIT_WORK_TREE is set.
  - HY\tools\effort_test_faults.py:97 passes only `-c core.quotePath=false`.
- **Splits:**
  - trailer.separators pinned in RJ, not in RR or check_handoff.
  - RJ drops GIT_CONFIG_PARAMETERS/COUNT; RR and check_handoff drop GIT_DIR and five others; RJ keeps GIT_DIR (git sets it inside a hook).
  - RJ sets NO_REPLACE_OBJECTS and a missing grafts file; the other two do not.
  - Drop (RR, check_handoff) versus refuse (publish_gate) for GIT_DIR/GIT_WORK_TREE.
  - Differ when git runs from inside a hook, and when a commit has replaced objects or grafts.
- **Ruling quotes:** None found (V13 grepped rulings for python/amend/rebase/hand-typed/gh issue; nothing on git pins).
- **Tests:** None named holding them equal.
- **Open:** Whether RR's `log` reads of trailers need trailer.separators (finder did not check; RR:391 reads diff-tree only).
- **Hook gap:** none.

### L-F13-02: Who owns the shape of a status-line call, and where is its text copied?
- **Source:** I5-065, I5-067; F13 group E; V13 item 2; F15 I9-058.
- **Kind:** K1, K5, K6, K8.
- **Verdict:** CONFIRMED (V13).
- **Homes:**
  - GATES\status_append.ps1:1 (V13; finder said :5) "The call shape: the brief preamble's "Status lines" section."
  - BP:19 "The status-call check in progress.py owns the exact shape." BP:15 the command.
  - GATES\progress.py:22-27 STATUS_PREFIX and _STATUS_CALL; comment "spelled as the allow rule in settings.fragment.json names it".
  - SF:4 `PowerShell(& "C:/Users/Patrick/.claude/gates/status_append.ps1"*)`.
  - C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md:100 (V13: this path, not under the Hydra repo) second full command, "(not Bash)", different line form.
  - Finder only: HY derive-once-review.md:9 and doc-review.md:9 (command again); MEM every-brief-carries-status-rule.md:15; progress.py:88-90,114,125-127 refusal texts print the shape.
- **Splits:**
  - status_append.ps1 says the preamble owns the call shape; BP:19 says progress.py owns it (circular pointers).
  - writer-brief.md:100 omits BP:19's no-$/backtick/double-quote rule; card writers skip the preamble, so only the hook enforces it for them.
  - SF:4 and STATUS_PREFIX are two code-side copies; finder says no test ties them (V13: not grepped, "unchecked").
- **Ruling quotes:** None on call shape (V13). Finder cites review-and-hooks.md:34 (2026-09-04) "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes", agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable." These are about the duty, not the shape; V13 did not re-read them.
- **Tests:** test_docs_numbers.py:182-190 holds BP equal to STATUS_PREFIX; test_progress.py pins STATUS_CMD (finder). Nothing ties writer-brief:100 or SF:4 (finder).
- **Open:** Whether a test ties SF:4 to STATUS_PREFIX (not grepped by V13).
- **Hook gap:** none.

### L-F13-03: What is the 30,000-character ceiling in the preamble, and who owns it?
- **Source:** I5-073, I5-156; F13 group F; V13 item 3.
- **Kind:** K7, K8.
- **Verdict:** CONFIRMED (V13 grepped GATES\*.py and README for 30,000/30000: only tests send a 30000-character string).
- **Homes:** BP:35 "Above 30,000 characters it arrives as Claude Code's own notice" and "shell_output.py ... owns the number and the lists". GATES\shell_output.py:15 `CAP = 4000` only. Finder only: probe_results.md:123,127,222 (measurements); README:23 says "Claude Code's ceiling" with no number.
- **Splits:** BP's "shell_output.py owns the number" holds for 4,000, not 30,000 (a Claude Code limit). Differ for output between 4,000 and 30,000 versus above.
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers.py:198-202 holds "4,000" to CAP; nothing holds 30,000.
- **Open:** probe_results.md not reopened by V13.
- **Hook gap:** none.

### L-F13-04: Which destructive git commands does the delete rule refuse, and who is asked versus refused?
- **Source:** I5-154, I3-096..098; F13 groups C and G; V13 item 4.
- **Kind:** K1, K2, K3, K5, K6.
- **Verdict:** CONFIRMED (V13) for the BP-versus-code list; the ask-list part is finder only (V13: "not re-opened by me beyond the finder's reading").
- **Homes:**
  - BP:43 lists git clean, reset --hard, reset --merge, checkout -f, worktree prune, stash drop, introduced "such as", and ends "`delete_rule.py` ... is the one home".
  - GATES\delete_rule.py:32-35 also refuses switch --force/-f/--discard-changes, checkout ., restore ., branch -D, stash clear, worktree remove.
  - README:19 lists the longer set (finder).
  - SF:60-63 (finder only) ask rules for the main session: `Bash(rm *)`, `Bash(git clean *)`, `Bash(git reset --hard*)`, `PowerShell(Remove-Item *)`.
- **Splits:**
  - BP:43's list is shorter than delete_rule.py's (an example list, not a copy, per V13).
  - Agent is refused outright; main session is only asked, and only for four patterns. SF `git reset --hard*` does not match `git reset --merge`, and SF has no ask for checkout ., restore ., stash drop, worktree remove, branch -D (finder only). Differ for the main session running those forms.
- **Ruling quotes:** Finder: README:19 quotes "trying to ban specific command patterns is a losing battle because there will always be another pattern nobody thought to check." (2026-10-31); finder could not find it under `## Confirmed` in the two files read. review-and-hooks.md:17 (2026-10-25) "Keep rule, move worktrees (Recommended)" chose the delete rule's design; it does not speak to the list. Neither names both sides.
- **Tests:** None named holding BP:43 or SF equal to delete_rule.py.
- **Open:** SF ask-rule claims not re-read by V13.
- **Hook gap:** none stated.

### L-F13-05: Which `worktrees` folders may an agent delete inside, and where do task worktrees actually live?
- **Source:** I5-154; F13 group G (unchecked), F13b C2; V13 item 4 (new split), V13b "Worktree folders".
- **Kind:** K1, K2, K6.
- **Verdict:** CONFIRMED (V13b; V13 raised it first).
- **Homes:**
  - GATES\delete_rule.py:14-18 WORKTREE_FOLDERS = HY\.claude\worktrees and C:\Users\Patrick\.claude\worktrees (comment "The worktrees folders of the two repos").
  - BP:43 names the same two paths; HY\docs\agents\derive-once-review.md:197 names the same two paths.
  - GATES\cleanup_sweep.ps1:172-179: `<root>\.claude\worktrees` in any repo, plus `<root>\worktrees` only in the repo the hook lives in.
  - Finder only: GATES\tests\test_delete_rule.py:28-29, 159-161 pin the two by literal; HY\tools\new_worktree.ps1:6,29 makes only .claude\worktrees.
  - Real state (V13b, git worktree list): 98 worktrees under HY\worktrees\ (including this audit's own), 3 under HY\.claude\worktrees\.
- **Splits:** delete_rule, BP:43, derive-once-review and new_worktree name HY\.claude\worktrees; most Hydra task worktrees sit in HY\worktrees\, which cleanup_sweep names only for the hooks repo. Differ for an agent deleting its own build folder inside HY\worktrees\<x>.
- **Ruling quotes:** review-and-hooks.md:17 (2026-10-25), quoted by finder and V13b: "The delete rule lets an agent recursively delete only inside its scratch folder or a worktree that sits in a `worktrees\` folder. Ten Hydra worktrees sit beside the repo as `C:\Users\Patrick\Downloads\Hydra\wt-*` instead. ..." chose "Keep rule, move worktrees (Recommended)". Says "a `worktrees\` folder" without a path; does not name both sides.
- **Tests:** test_delete_rule.py pins the two folders by literal (finder); nothing ties the list to where worktrees are cut.
- **Open:** A run of a delete inside HY\worktrees would settle it (none run). new_worktree.ps1 not reopened by V13b.
- **Hook gap:** none stated.

### L-F13-06: Which Python runs scripts and hooks, and how is it spelled?
- **Source:** I6-183, I6-184, I5-099a; F13 groups C, G, H; F13b C6; V13 item 5; V13b "sync_settings launch path".
- **Kind:** K1, K2, K3, K6.
- **Verdict:** CONFIRMED (V13, V13b).
- **Homes:**
  - SF:6 `PowerShell(python.exe -I C:/Users/Patrick/.claude/gates/ruling_rules.py --check *)`; GATES\ruling_rules.py:108 CHECK_COMMAND, same bare `python.exe`.
  - GATES\sync_settings.py:20 bare `python.exe -I ...`; README:110 uses the full pythoncore path (V13b).
  - TM:4-8 "Use `py`" (pythoncore 3.14.5); a bare `python` reaches C:\Python314\python.exe 3.14.7.
  - GATES\README.md:110,114,145 full pythoncore path. BP:33 `python -I <file>`; BP:97 `py -I`.
  - Finder only: publish.ps1:16, install_ref_gate.ps1:10, ten hook registrations in SF (lines 97-229) hold the full path.
- **Splits:**
  - BP:33 `python -I` versus TM "Use `py`".
  - ruling_rules.py:108, SF:6 and sync_settings.py:20 bare `python.exe` versus README:114's rule that hooks use the full pythoncore path.
  - Differ for which install a bare `python.exe` reaches.
- **Ruling quotes:** None found.
- **Tests:** None holds the path equal across homes (finder; V13 "not re-grepped").
- **Open:** Which install a bare `python.exe` reaches inside a PowerShell call needs a run (`Get-Command python.exe`); TM's version claims unchecked.
- **Hook gap:** none.

### L-F13-07: May an agent rewrite or move a branch, and what does the ref gate actually refuse?
- **Source:** I3-011, I3-012, I3-013; F13 group B; V13 item 6.
- **Kind:** K1, K3, K5, K7.
- **Verdict:** CONFIRMED (V13) for the scope gap. I3-012 (only the user deletes main) and I3-013 (symbolic ref) are single-home, finder only.
- **Homes:**
  - BP:71 "Never amend, rebase, reset or move a branch" and, in its last sentence, "The ref gate refuses a move of `main`, or of any branch origin also has, that is not a plain step forward".
  - HY\docs\agents\integrate.md:31 "Never rebase, amend, reset or force-move a branch."; :50 "Never amend."
  - RJ:135-137 `shared = ref == MAIN or origin has refs/remotes/origin/<name>`; refusal text RJ:99-103.
  - Finder only: reviewer.md:19 "Never move the branch anywhere else"; RJ:120-121 and :153-154 "Only the user deletes main."; RJ:111-115 symbolic ref; memory\archive\fleet-control-hooks-2026-09.md:19.
- **Splits:** Instructions forbid any rewrite of any branch; the gate refuses only main or a branch origin has. Differ for a local branch origin lacks (an agent's own task branch). Nothing refuses `reset --soft` or `commit --amend` there.
- **Ruling quotes:** None found.
- **Tests:** None named.
- **Open:** None.
- **Hook gap:** none stated.

### L-F13-08: Do hooks refuse hand-typed `gh` commands that post text or change issue state?
- **Source:** I3-085..087, I6-187; F13 group C and side notes; V13 item 7.
- **Kind:** K3, K6, K7.
- **Verdict:** CONFIRMED-CHANGED (V13). The clash is real for `gh issue close <n> --comment "..."` (issue-tracker.md:14), which posts text. `gh issue edit ... --add-label` (:13) and `--add-assignee` (:46) post no text, so the finder's claim against "text never goes through hand-typed gh" overreaches for those two.
- **Homes:**
  - HY\docs\agents\issue-tracker.md:13,14,46 hand-typed `gh issue edit` and `gh issue close --comment`.
  - TM:15 "Text never goes to GitHub through a hand-typed `gh` command."; doc-review.md:19 "Once the new settings are live, they refuse the posting `gh` commands typed by hand" (finder's quote; V13 read it as ending that way).
  - SF deny list: V13 grepped for "issue close" and "issue edit": count 0; present are `issue create`, `issue comment` and others.
- **Splits:** issue-tracker.md:14 versus TM:15 (and doc-review.md:19 as the finder reads it). Differ for closing an issue with a comment by hand.
- **Ruling quotes:** None found.
- **Tests:** None holds the SF deny list equal to publish.ps1's kinds (finder).
- **Open:** None.
- **Hook gap:** hook gap: yes, GATES\settings.fragment.json.

### L-F13-09: Should an integrator hold the last merge uncommitted, or commit after each finished step?
- **Source:** I6-107, I6-110, I6-112; F13 group H; V13 item 8.
- **Kind:** K3.
- **Verdict:** CONFIRMED (V13).
- **Homes:** HY\docs\agents\integrate.md:27 "Hold the last task's merge uncommitted until the precheck in step 4 is clean". BP:71 "Commit before your 100th tool call, and after each finished step." BP:3 says where brief and preamble disagree, stop and report. The 100-call warning is by the progress hook.
- **Splits:** integrate.md:27 versus BP:71. Differ for an integrator whose precheck runs past 100 calls. Second line: integrate.md:31 "force-move" versus BP:71 "move a branch" (same family as L-F13-07).
- **Ruling quotes:** None found.
- **Tests:** None.
- **Open:** Which wins is stated in neither page.
- **Hook gap:** none.

### L-F13-10: Does a dispatch gate deny briefs that lack the status sentence? (memory note contradicts itself)
- **Source:** I5-065; F13 side notes; V13 item 9.
- **Kind:** K8.
- **Verdict:** CONFIRMED (V13; V13 grepped GATES\review_dispatch.py for "status" and "yet": nothing).
- **Homes:** MEM\every-brief-carries-status-rule.md front matter "...the dispatch gate now denies briefs that lack it" versus body "No gate checks a brief for this sentence today".
- **Splits:** front matter versus body of one note. Differ for what a reader believes a gate checks.
- **Ruling quotes:** None; a memory note is not the user's words.
- **Tests:** None.
- **Open:** None.
- **Hook gap:** none.

### L-F13-11: Is "no elevated window" a user ruling, and what trace method may a session use? (wpr, ETW)
- **Source:** I6-198, I7-151, I8-028; F13b B; F14 S4; V13b "I6-198"; V14 S4; F15 group F.
- **Kind:** K1, K2, K3, K7, K8.
- **Verdict:** CONFIRMED (V13b, V14 S4). F15 group F (single home, tension with the ch MCP) is finder only.
- **Homes:**
  - TM:45-49 "`Medium Mandatory Level`"; "`wpr -start` was refused ... `0xC5585011`"; ends "The user has ruled that no elevated window is opened."; TM:63 restates ("`cdb` attaches from this unelevated shell, unlike `wpr`").
  - Card HY\docs\sessions\2026-11-07-7d216054.md:11,21 (user words "i dont accept that you need an elevated run. figure out another solutio."); plan docs\superpowers\plans\2026-11-07-ch-run-timing.md:12,27,60 (wider: "No elevated window, no trace, no PresentMon, no install, no machine change, and nothing that works around the Windows administrator prompt"); plan 2026-11-12-player-stall-fixes.md:62; tools\ch_mcp\machine_recorder.py:6 (finder).
  - MEM\check-long-captures-early.md:15 "Prefer non-invasive capture (ETW/WPR, read-only memory reads) over hooking hot functions in a live app." and MEMORY.md:18 "prefer ETW over hooks".
- **Splits:**
  - TM:49 claims "The user has ruled"; V13b/V14 grepped rulings\ for elevated/administrator and found no line, so no filed ruling holds it (the words exist only in a card and a plan).
  - The memory note recommends ETW/WPR; TM:49 says wpr is refused from agent shells. A memory note is not the user's words.
  - Plan :27 widens the ban to PresentMon, install and machine change; TM keeps only wpr (finder).
  - Finder only: the ch MCP's own Frida hooks and `ch_trace_*` are the opposite practice of the note's "prefer ETW over hooks"; no page reconciles them.
- **Ruling quotes:** None found under `## Confirmed` (searched all rulings\*.md for elevated, administrator, admin window).
- **Tests:** None.
- **Open:** None. No hook enforces it (finder's GATES grep).
- **Hook gap:** none.

### L-F13-12: What do you do when CMake refuses a Visual Studio install? (install table copy)
- **Source:** I6-197; F13b A; V13b.
- **Kind:** K1.
- **Verdict:** CONFIRMED (V13b read TM only; did not re-run vswhere nor reopen CMakeCache or find_cmake.ps1).
- **Homes:** TM:35-43 install table, "build-cpp cache names the 2026 Build Tools", `-DCMAKE_GENERATOR_INSTANCE="<folder>,version=<version>"`, "falls back to `vswhere -all`". Finder only: HY\tools\find_cmake.ps1:12-18 does the fallback; HY\build-cpp\CMakeCache.txt:773.
- **Splits:** None today. Table versions go stale at the next VS update and nothing holds them to vswhere (finder).
- **Ruling quotes:** None found.
- **Tests:** None.
- **Open:** Finder readings of find_cmake.ps1 and CMakeCache unchecked by V13b.
- **Hook gap:** none.

### L-F13-13: How many builds may run at once, who may start one, and where does the build state live?
- **Source:** I5-151; F13 group G; F13b C1; V13b "Three builds".
- **Kind:** K1; finder's K3 and K6 candidates.
- **Verdict:** CONFIRMED-CHANGED (V13b). Finder's K3 worry (ruling "one build per session" not enforced) REFUTED: tools\build_slot.ps1:195 refuses a second pass for the same session ("One build at a time per session."). Finder's K6 candidate (two state folders for one question) REFUTED as stated: ~\.claude\hooks\state holds only `build_slots` today; the names the finder listed there are in gates\state; no GATES file mentions hooks\state or build_slots.
- **Homes:** HY\tools\build_slot.ps1:1-3 "the one place that knows the slot rule, the pass rule and their numbers", :75 `$BuildSlotCount = 3`; HY\CLAUDE.md rule 14; agent-rules.md:20; BP:57; GATES\percall.py:19-20,107-111 (agents refused); test_build_pass.ps1:117 expects "all 3 build slots are taken" (finder). Finder only: build_slot.ps1:10,78 and bench_run.ps1:8-20 keep state under ~\.claude\hooks\state.
- **Splits:** None left after V13b.
- **Ruling quotes:** agents-and-workflow.md:8 (2026-11-07), quoted by finder and V13b (V13b did not re-read it): "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." :9 "any tests that require a build are done by the main session, not agents."
- **Tests:** test_build_pass.ps1:117 (the 3 in a literal).
- **Open:** Count of three in the docs and test not reopened by V13b.
- **Hook gap:** none.

### L-F13-14: What is the command-length limit, and where is it stated?
- **Source:** I5-072, I5-158; F13 group F, F13b C3; V13b "Command-length limit".
- **Kind:** K1, K5.
- **Verdict:** CONFIRMED (V13b). BP:33 does carry the git commit exemption (finder had left that open).
- **Homes:** GATES\bash_windows_rule.py:16 MAX_CHARS = 1000, :172-174 `_too_long`, :211-214 refusal text; BP:33 "A command over 1,000 characters" and "A `git commit`'s own text does not count toward that limit", ends "`bash_windows_rule.py` ... owns the rule and the number"; README:27; test_bash_windows_rule.py:53 pins 1000.
- **Splits:** None on the number. See L-F15-12 for the commit-text wording.
- **Ruling quotes:** review-and-hooks.md:40-43 are Unconfirmed lines (2026-11-06: 1,000, commit exemption); finder says the Confirmed lines hold nothing on the number. None under `## Confirmed`.
- **Tests:** test_docs_numbers.py:192-196 holds BP's "1,000" equal to MAX_CHARS.
- **Open:** None.
- **Hook gap:** none.

### L-F13-15: Who may use the Clone Hero key route, and what exact text unlocks it?
- **Source:** I5-150; F13 group G; F13b C4; V13b "Key route text"; F15 group E (tests).
- **Kind:** K1, K3, K5.
- **Verdict:** CONFIRMED (V13b read review_rules.py:54-55,86 and the start of CH:29; did not see the "exactly" wording in full, did not reopen ch_call forms).
- **Homes:** RR:54-55 `KEY_ALLOW_TEXT = "allow key run"`, `_KEY_ALLOW` matches with optional trailing `.` or `!`, ignores case, :86 fullmatch on stripped text; GATES\percall.py:16-18 KEY_TOOLS, :96 message; CH:29 (names ch_key, ch_press, ch_play_chart with route keys); README:13,37; the ch MCP instruction text. Finder only: MEM\ch-runs-must-not-need-focus.md:17 "ask first".
- **Splits:** CH:29 "exactly `allow key run`" (finder's quote) versus code accepting "Allow key run." and "allow key run!". CH names three tools; README:13 also lists `ch_call` forms (finder). Memory says "ask first"; the hook wants a typed allow. Differ for case and trailing punctuation.
- **Ruling quotes:** None checked by V13b. F15 lists clone-hero.md Confirmed lines about focus; see L-F14-11.
- **Tests:** test_percall_key.py:42,146-162 holds the refusal text to `KEY_ALLOW_TEXT` by reference (the doc line is not held); finder (F15): test_posture.py:217 sends the literal "allow key run" (a literal copy).
- **Open:** test_posture.py literal not checked by V15.
- **Hook gap:** none.

### L-F13-16: What fixes the ruling-filing question text, labels and "up to four lines"?
- **Source:** F13b C5; V13b "Up to four lines".
- **Kind:** K1, K5, K7.
- **Verdict:** NOT VERIFIED for the text copies. "Up to four lines": OPEN (V13b: rulings\README.md:41 cut off in its output; ruling_rules.py not searched).
- **Homes (finder only):** GATES\ruling_rules.py:100-101 `APPROVE, DECLINE = "File it", "Do not file"`, `QUESTION = "File this ruling in {path}?"`; HY\docs\agents\rulings\README.md:41 (question text, labels, "One box may hold up to four lines"); HY\CLAUDE.md; agent-rules.md:33.
- **Splits:** None. "Up to four lines" has no owner in our code (finder grepped ruling_rules.py for 4/four; likely the question tool's own limit).
- **Ruling quotes:** None found.
- **Tests:** test_ruling_rules.py holds QUESTION and labels by reference (finder); README text not held.
- **Open:** Not reached by V13b beyond one line.
- **Hook gap:** none.

### L-F13-17: Which kinds of GitHub post exist, what does each need, and which hand-typed forms are denied?
- **Source:** I3-085..087, I6-226, I6-227, I6-231, I7-209; F13 group C; F14 I6-226, I6-227, I6-231; V14 (I6-231); F15 group C.
- **Kind:** K1, K3, K5, K6.
- **Verdict:** I6-231: CONFIRMED (V14, K3 weak). The kind lists: NOT VERIFIED (neither verifier re-checked the six-kind copies).
- **Homes:**
  - GATES\publish.ps1:1,8 ValidateSet (pr, release, release-edit, issue, issue-comment, pr-comment); :48-50 required -Target/-Title; :18-38 refused flag letters (bFnfwetT etc.).
  - HY\docs\agents\doc-review.md:19 names all six; issue-tracker.md:9,12 command forms; SF:9-57 deny list covers the same six plus `gh api`; README:65,73.
  - V14: issue-tracker.md:30-32 "When a skill says 'publish to the issue tracker' ... Create a GitHub issue." omits the route, while :5 and :9 say posting goes through publish.ps1.
- **Splits:** issue-tracker.md:30 gives the action without the required route; the page is consistent when read whole (V14, weak). Also see L-F13-08.
- **Ruling quotes:** None found.
- **Tests:** test_publish.py pins the kinds and flag letters (test_publish.py:326,361); no test holds doc-review.md:19 or the SF deny list equal to the ValidateSet.
- **Open:** None reached.
- **Hook gap:** none beyond L-F13-08.

### L-F13-18: Is the by-hand ruling-check permission rule held equal to its owner?
- **Source:** I3-080, I9-035; F13 group C; F15 group P; V15.
- **Kind:** K1.
- **Verdict:** CONFIRMED (V15) that test_docs_numbers.py:139 builds `PowerShell(%s *)` from `ruling_rules.CHECK_COMMAND`. Other copies not reopened.
- **Homes:** ruling_rules.py:107-110 CHECK_COMMAND; SF:6; live settings.json:12 (finder); README:61; HY\docs\agents\rulings\README.md:40 `ruling_rules.py --check <file>` (finder: pointer without the interpreter).
- **Splits:** None here; see L-F13-06 for bare `python.exe`.
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers.py:137-140 and :142 hold SF and the live settings to CHECK_COMMAND.
- **Open:** rulings\README.md not opened by F13.
- **Hook gap:** none.

### L-F13-19: What are the handoff-check and precheck commands, and are their copies equal?
- **Source:** I6-012, I6-114, I6-090, I6-098, I6-104, I6-046, I8-240, I7-197; F13 group H; F14 I7-197; F15 group O.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED (neither V13 nor V14 nor V15 reopened it).
- **Homes (finder only):**
  - Handoff: HY\tools\check_handoff.py:7 usage `py -I tools/check_handoff.py <handoff.md> [--repo <root>] [--since <YYYY-MM-DD>]`; HY\docs\agents\handoff.md:45; HY\.claude\commands\handoff.md:12; BP:97. Status words and MANUAL_KINDS agree with handoff.md:39-55 (check_handoff.py:32-33). V15 side: finder says all agree today.
  - Precheck: HY\tools\precheck\main.cpp:4-12,53-57 owner; derive-once-review.md:24,99; integrate.md:37,46; agent-rules.md:7; all agree today (finder).
  - Hook-tests command: README:145 and HY\docs\agents\derive-once-review.md? (finder: DOR:124) both `-I -m unittest discover -t . -s gates/tests -p`.
- **Splits:** None (all agree today per finder).
- **Ruling quotes:** None found.
- **Tests:** tools\test_check_handoff.py tests the script, not the docs; test_docs_match_code.cpp:1555,1577 mention split_row and claims_rows.
- **Open:** Not reached.
- **Hook gap:** none.

### L-F13-20: Does the ref gate's refusal advice and shim answer agree with the preamble and README?
- **Source:** I3-019, I3-090, I5-193; F13 groups B and D.
- **Kind:** K1, K3.
- **Verdict:** NOT VERIFIED (no verifier reached it).
- **Homes (finder only):**
  - RJ:225-232 refusal advice: `git commit -m "Your subject" --trailer "Task: <task>" --trailer "Agent: <your agent id, or orchestrator>" --trailer "Session: %s"`; BP:65-67 (agent form, plus `Co-Authored-By`); README:166; MEM git-traps-in-shared-checkout.md:21; integrate.md:27.
  - ref_gate.sh:8-18 and RJ:8-10,31 the verdict strings; README:53,162; install_ref_gate.ps1:12-13.
- **Splits:** RJ allows "orchestrator" as Agent; BP tells everyone "your agent id". RJ's text does not repeat BP:65 "Never pass the message on stdin". BP:65 names `Co-Authored-By: Claude Opus 5.5` while the session's own attribution reminder gives another line (finder side note).
- **Ruling quotes:** None found.
- **Tests:** test_ref_judge.py installs and runs the shim (verdict strings exercised together, not asserted equal by name).
- **Open:** Not reached.
- **Hook gap:** none.

### L-F13-21: Preamble rules with no check: read with the Read tool, foreground only
- **Source:** I5-070, I5-087; F13 group F, G.
- **Kind:** K3, K7.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** BP:27 (Read tool, never sed/head/tail/cat); BP:35 (same Read-with-offset way for output files); writer-brief:99 weaker wording; BP:77 "Run every helper and long command in the foreground"; GATES\agent_stop.py:44-51 and README:41 treat a background command as "parked"; progress.py:77-79. Finder: no hook enforces either (grep of percall/bash_windows_rule/shell_output for head|tail|sed: no rule; no refusal for background). F15 I9-042: shell_output.py:75 only skips the trim for `run_in_background`.
- **Splits:** The hook side tolerates a background command that BP:77 forbids. This session's own tool text allows cat/sed through Bash in auto mode (finder side note).
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** Not reached.
- **Hook gap:** none stated.

### L-F13-22: How is the whole-library run guard stated, and where is its flag spelled?
- **Source:** I5-149, I6-211; F13 group G; F14 I6-211; V14 "I6-211".
- **Kind:** K1, K3, K5, K6.
- **Verdict:** CONFIRMED (V14) for the flag being a literal in two code homes and BP:49 spelling out both guards. The phrase-versus-rule-8 K3 point and the rest: NOT VERIFIED.
- **Homes:**
  - GATES\percall.py:14 `LIBRARY_FLAG = "--whole-library"`; HY\src\cli\batch.cpp (comment at :3; finder cites kWholeLibraryFlag at :82); compare_db.py:29 says src\cli\batch_scope.h owns the rule (finder).
  - RR:51 `LIBRARY_ALLOW_TEXT = "allow library run"`, :81 is_library_allow (finder).
  - BP:49; agent-rules.md:14; audit-method.md:125; TM:91 ("a whole-library `--redo` needs the allow", only home of that); README:13,37; HY\CLAUDE.md rule 8 "asked for in chat first" (no phrase); MEM no-full-library-runs.md:11-15.
- **Splits:** CLAUDE.md rule 8 says "explicit permission, asked for in chat first"; the hook wants the exact whole-message phrase (finder, K3). TM:91 says a whole-library `--redo` needs the allow; the hook keys only on the flag string. The corpus size inside the same rule is split, see L-F14-01.
- **Ruling quotes:** review-and-hooks.md:47 (F14) / :16 (F13), 2026-10-25: "i want to be able to give explicit approval for a full library run." Not reopened by V14.
- **Tests:** None holds the flag literal in percall.py equal to the one in batch.cpp (finder). Hook tests: test_review_rules.py, test_review_dispatch.py, test_progress.py, test_posture.py (they test the hook, not the docs).
- **Open:** None reached beyond the flag literals.
- **Hook gap:** none.

### L-F13-23: Prose copies of hook behaviour: percall order, settings contents, hook-test commands, PowerShell hook tests, rollback
- **Source:** I5-148, I5-177, I5-187, I5-188, I5-193; F13 group G.
- **Kind:** K1, K5.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** README:11 and :116 (percall order and "only hook on PreToolUse *"; percall.py not opened by finder); README:73,108-110 and sync_settings.py docstring (owner of keys; `merged` and `first_difference` bodies not read); README:145,152; BP:47; README:17 "agents still run [hook tests]" versus BP:47 "Never run all the hook tests"; README:164-174 and :166 versus RJ:228-230.
- **Splits:** README:17 versus BP:47 are two phrasings (one file versus all).
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers.py:142-170 checks fragment shape; no test on the PreToolUse count (finder).
- **Open:** Weak search for I5-188 (README plus tests listing only).
- **Hook gap:** none.

### L-F13-24: Side notes from F13
- **Source:** F13 side notes.
- **Kind:** K1, K5.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** README:5 says the page "does not repeat" the preamble's rules, while README:19,23,25,27 restate delete, trim and Bash rules in full. test_docs_numbers.py:44 CALL_COUNT regex covers "N tool calls" in five docs. Numbers copied by literal in tests: test_bash_windows_rule.py:53 (1000), test_shell_output.py:172 (ERROR_CHARS 1000), test_publish.py:247 (usage text) and :361 (letters bFnfwetT).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** as listed.
- **Open:** Not reached.
- **Hook gap:** none.

---

## F14 rows

### L-F14-01: How many charts are in testdata/input?
- **Source:** F14 S1; I6-211; V14 S1; F15 I8-136.
- **Kind:** K2.
- **Verdict:** CONFIRMED-CHANGED (V14). Still two answers (98 versus about 115). V14's count under HY\testdata\input: 98 .ini, 72 .mid, 44 .chart, 113 folders holding a .chart or .mid, 120 folders; the finder's "111" did not reproduce (V14 got 116 .chart/.mid files).
- **Homes:** UT:46,48 "98 charts"; HY\tests\test_engine_sweeps.cpp:656 `CHECK(charts == 98)` (V14); HY\tests\ui\uitest_batch_reports.cpp:389 "Analyze 98 charts that have no result yet?"; agent-rules.md:14 "(about 115 charts)". Finder only (F15): MEM\questions-describe-the-real-rule.md:24 "Strip the `[N/97]` prefix" (F15 counted 98 song.ini and 116 chart/mid files; did not run the tool).
- **Splits:** UT 98 versus agent-rules.md about 115 versus the memory note's 97. Differ between a brief writer reading agent-rules and a UI tester reading UT.
- **Ruling quotes:** None found.
- **Tests:** test_engine_sweeps.cpp:656 and uitest_batch_reports.cpp:389 hold 98.
- **Open:** Whether `[N/97]` is what the tool prints needs a run.
- **Hook gap:** none.

### L-F14-02: Which shell runs scripts: pwsh or powershell?
- **Source:** I6-199; F14 S2; V14 S2.
- **Kind:** K1, K2.
- **Verdict:** CONFIRMED (V14). The two are compatible (the Bypass flag makes the second work) but a reader gets two instructions.
- **Homes:** TM:51 "Run scripts with `pwsh`... Windows PowerShell (`powershell.exe`) has no policy set here, so a plain call... is refused"; GATES\README.md:152 "`powershell -NoProfile -ExecutionPolicy Bypass -File ...`". Finder only: tools\*.ps1 headers say `pwsh -NoProfile -File`; stall_harness.py find_cdb (:178) and quiet.py:311 call powershell; hook registrations in SF:73-88 run powershell.exe with -ExecutionPolicy.
- **Splits:** TM:51 versus README:152 (above).
- **Ruling quotes:** None found.
- **Tests:** None.
- **Open:** None.
- **Hook gap:** none.

### L-F14-03: How do you copy a live hydra.db?
- **Source:** I6-207, I7-036; F14 S3; V14 S3.
- **Kind:** K1, K2.
- **Verdict:** CONFIRMED (V14).
- **Homes:** TM:81 plain copy of hydra.db plus -wal and -shm; UT:195 helper uses SQLite online backup through a read-only connection when a -wal is beside it; UT:350 step 3 copy while the installed Hydra is closed. Finder only: hydra_ui_mcp.py:55-56,137-147 (snapshot_library owns the library path in code); the installed path also in TM:77, UT:350 and test_live_protocol.cpp:64.
- **Splits:** three procedures; differ for Hydra open (UT:195), Hydra closed (UT:350), a scout reading by hand (TM:81).
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** None.
- **Hook gap:** none.

### L-F14-04: How do you attach cdb and read a stall? (command copied from the doc into code)
- **Source:** I6-202, I7-154; F14 S5; V14 S5.
- **Kind:** K1, K2.
- **Verdict:** CONFIRMED (V14).
- **Homes:** TM:63 "Pass the build's symbol folder with `-y`", example `cdb -pv -y <build-cpp\Release> -p <pid> -c "~*kc 40; qd"`, TM:67 "at most once per run". HY\tools\ch_probe\experiments\stall_harness.py:121 CDB_COMMAND and launch_cdb: `[cdb_path, "-pv", "-p", str(pid), "-c", CDB_COMMAND]`, no `-y`; its comment says the command "is the one docs/agents/this-machine.md ... shows working". Finder only: ch_probe\constants.py:351-358 (STALL_THRESHOLD_MS 40, STALL_RUN_MINUTES 10, STALL_HUNT_RUNS_MAX 3).
- **Splits:** TM:63 (with `-y`) versus stall_harness.py (without). Differ for a stack with no symbols versus one with.
- **Ruling quotes:** Finder: rulings\clone-hero.md:16 (2026-11-12) "Yes, those numbers (Recommended)" matches the constants; not reopened by V14.
- **Tests:** test_stall_harness.py uses a fake cdb and does not hold the command equal to the doc (finder).
- **Open:** Whether the harness sets _NT_SYMBOL_PATH elsewhere (V14: a run would settle it).
- **Hook gap:** none.

### L-F14-05: Does the build command in the UI-testing page match what the build owner requires?
- **Source:** I7-006; F14; V14.
- **Kind:** K3.
- **Verdict:** CONFIRMED-CHANGED (V14): the UT example is confirmed; the finder's quote that build_cpp.ps1:12-13 "stops before configure" without HYDRA_BUILD_PASS was not reproduced by V14 (it saw lines 1-9 and the tail), so that quote is unverified.
- **Homes:** UT:18-20 bare `.\build_cpp.ps1 -Target hydra_uitest`; HY\build_cpp.ps1:1-13 usage header; build_slot.ps1:208; UT:16-17 points to development.md "Building under Claude Code"; UT:27 agents never build; HY\CLAUDE.md rule 14.
- **Splits:** UT:20 example omits the pass the owner requires (if the unverified quote is right).
- **Ruling quotes:** None found.
- **Tests:** None.
- **Open:** build_cpp.ps1:12-13 text not seen by V14.
- **Hook gap:** none.

### L-F14-06: Is a Confirmed "mouse doesn't work" line a design fact for the live window?
- **Source:** I7-059; F14; V14.
- **Kind:** K3.
- **Verdict:** CONFIRMED (V14). The quote reads as a complaint; the doc treats it as a design fact. It does not name a split.
- **Homes:** UT:354 "The live window takes no input from the real mouse (the user, 2026-10-29: ...)"; rulings\ui-and-display.md:43 (2026-10-29, session c9b0cb46).
- **Splits:** None named; for the main session to judge.
- **Ruling quotes:** rulings\ui-and-display.md:43 under `## Confirmed`: "the window is jsut in the library list and my mouse doesnt work in it".
- **Tests:** None.
- **Open:** None.
- **Hook gap:** none.

### L-F14-07: Which menus can ch_open_song reach? (doc outcome versus code mechanism)
- **Source:** I7-079, I7-083; F14; V14.
- **Kind:** K1, K3.
- **Verdict:** CONFIRMED-CHANGED (V14): the doc states an outcome; the code enforces a narrower mechanism.
- **Homes:** CH:31 "No tool opens the Online menu, Settings, Practice or the control mapper."; tools\ch_mcp\menu_tools.py:191-216 online_refusal, :220 item_problem, :234 load_steps (refuse while an Online read is not closed; the step file may list only `kit:` items); nothing in code names Settings, Practice or the mapper. Finder only: CH:35, menu_steps.json "about" (verified false; cites a removed plan path), README.md:75-77.
- **Splits:** CH:31 (four menus named) versus code (online flag plus kit-only items).
- **Ruling quotes:** None found.
- **Tests:** tests\test_menu_tools.py (finder: not read).
- **Open:** Whether a kit item can reach Settings is a run question (ch_open_song).
- **Hook gap:** none.

### L-F14-08: Does ch_compare need all four settings or none, and who decides?
- **Source:** I7-097; F14; V14.
- **Kind:** K4, K6.
- **Verdict:** CONFIRMED (V14), a small drift.
- **Homes:** tools\ch_mcp\game_tools.py:954-972 (refuses "give all four of the run's settings, or none"); compare_scorestats.py:202 builds RunSettings only when checksum, speed and no_fail are all given (does not test modifiers). CH:62 names both owners; server.py:228-236 docstring.
- **Splits:** the tool needs four given-or-none; the CLI keys on three. Differ when modifiers are the only one missing.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** None.
- **Hook gap:** none.

### L-F14-09: Where is the realapp.jsonl file name defined?
- **Source:** I7-038; F14; V14.
- **Kind:** K1, K6.
- **Verdict:** CONFIRMED (V14).
- **Homes:** GATES\review_rules.py:49 `REALAPP_FILE = "realapp.jsonl"` (comment says the helper owns it); HY\tools\hydra_ui_mcp.py:81 same literal. Two repos. Finder only: guards at HY\tests\temp_util.h:50 is_under_temp, tests\ui\uitest_live.cpp:266, test_live_protocol.cpp:58-75.
- **Splits:** None in value; two code homes.
- **Ruling quotes:** None found.
- **Tests:** none found comparing them (finder; V14 did not search).
- **Open:** None.
- **Hook gap:** none.

### L-F14-10: Is the rule "never scroll the library without the user's yes" held by anything, and do two notes agree on when scrolling is allowed?
- **Source:** I7-018..021, I8-145, I8-151..153, I8-253; F14 I7-018/019/020/021; F15 groups I, J; V15 split 4.
- **Kind:** K1, K3, K5, K7.
- **Verdict:** Scroll-wording gap: CONFIRMED-CHANGED (V15, a wording gap rather than a clean split). The rest (no hook for the real window, posted messages bypass the guard, agents cannot "ask in chat", `kPageKeys` doc list versus array): NOT VERIFIED (V14 "not reopened"; V15 did not reach group J).
- **Homes:**
  - MEM\render-checks-use-the-corpus.md:19 "Scroll to the middle of the list, click a song, compare before and after, close, and compare again." (V15: the note's title ties it to the corpus).
  - MEM\search-before-clicking-a-song.md:11 "Never scroll through the user's library to reach a song or a spot without asking in chat first" (names "the user's library"); :13 records the user's anger at 1,300 wheel notches; :17 how to reach a song and "posted window messages bypass" the guard.
  - UT:99 three guards; owners HY\tests\ui\uitest_commands.cpp: `item_ref` (:239), `kPageKeys` (:76), `library_key_refusal` (:213); uitest_live.cpp:481-505 pins refusal wording for PageDown. UT:216, the hydra MCP text (hydra_ui_mcp.py:757) "use hydra_open_song with its exact title". Finder only: UT:99 says PageDown or End while the kPageKeys array starts PageUp, PageDown, Home ... (rest of array not opened); MEMORY.md:10 TOP RULE.
- **Splits:**
  - render-checks note :19 (scroll in the middle of the list) versus search-before-clicking note :11 (never scroll the library). Differ for corpus (headless, testdata/input) versus real-window library snapshot; neither sentence says so.
  - Finder only: "ask in chat" binds the main session; an agent has no user channel and cannot ask.
  - Finder only: UT:99's example key list may differ from the kPageKeys array.
- **Ruling quotes:** rulings\ui-and-display.md:27 (2026-10-30, per F14) "ive had enough of this shit. something like 10 times now you waste my time by scrolling thorugh the entire library for minutes to reach the song you want with no explanation. this should never happenw tihout my approval." and :48 (2026-10-24) "why are you scrolling through the entire library??? why do you keep forgetting search exists?". V14 confirmed only that the file carries a 2026-10-29 line at :43, not :27 and :48. Neither quote names both sides.
- **Tests:** uitest_live.cpp:481-505 pins the PageDown refusal; item_ref tests exist, not opened (finder).
- **Open:** Not reached.
- **Hook gap:** hook gap: yes, HY\tests\ui\uitest_commands.cpp (the guard is test-engine only; finder: posted window messages bypass it; F14 side note: a shell script sending wheel or key messages from outside the command channel is covered by no hook; UT:99 says so itself).

### L-F14-11: Is "never bring the game window to the front" a rule, and do the memory note and CH page agree?
- **Source:** I7-073, I8-019, I8-023; F14 I7-073; F15 group E; V15 split 7.
- **Kind:** K1, K3, K5.
- **Verdict:** I7-073 F14 reading: NOT VERIFIED (V14 "not reopened"). F15 possible split: CONFIRMED-CHANGED to a narrow wording gap (V15).
- **Homes:**
  - CH:23 "The game window is never brought to the front", then names the one launch that cannot ask for a hidden window (the spawn form of the human launch, whose `launch` ships set to `suspended`) (V15); CH:7, CH:19 (registry key backup), CH:29.
  - MEM\ch-runs-must-not-need-focus.md:11,19 ("TOP RULE", "so never show the window"; user quote "yes we need to do autonomous runs. that waws the whole point of this").
  - HY\tools\ch_mcp\launch_patch.json:5 `"launch": "suspended"`; bot_run.py:80,157-160,508,630 (SW_SHOWMINNOACTIVE); README.md:75. Finder only: game_files.py (registry backup); 07-internals.md:581.
- **Splits:**
  - Memory "never show the window" versus CH:23. V15: both say never; the page names the one launch route where the ask is impossible. Whether a brief grab at spawn is allowed is stated in neither home.
  - Finder (F14): the memory note (TOP RULE, input from inside the game) is stronger than CH:23 ("count is information, not a pass condition"); CH:23 quotes only the first half of a ruling.
- **Ruling quotes:** rulings\clone-hero.md under `## Confirmed`, from the finders: :38 "yes we need to do autonomous runs. that waws the whole point of this" (session a5f17ca5); :39 "why does the game need to be focused? hydra doesnt need to be focused when you test it"; :34 (2026-10-31, session a379bc75) "why does it matter if the game is focused briefly? it sounds to me like you're trying to over engineer a solution to a problem that isnt real" (V15 read :34 itself); :43 chose "No one needed (Rec.)"; :8-9 "stop messing with the game's save file. just let it stay." V15: :34 does not name the memory note's "never show". None names both sides.
- **Tests:** HY\tools\ch_mcp\tests\test_foreground.py (finder, not read for equality).
- **Open:** Whether a brief focus at spawn is allowed.
- **Hook gap:** none.

### L-F14-12: Word lists for public-doc style versus the user's global style list
- **Source:** I6-258, I6-259, I6-260; F14.
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** HY\docs\agents\public-doc-style.md:27,29,31; C:\Users\Patrick\.claude\CLAUDE.md:5 throat-clearing list ("It's worth noting", "In order to", "It's important to note"), overlapping PD:31 on two phrases, different on the rest. Different readers.
- **Splits:** overlap with different words; no hook or test greps any of these words (finder searched GATES, HY\tools, HY\tests).
- **Ruling quotes:** None found.
- **Tests:** None.
- **Open:** Not reached.
- **Hook gap:** none.

### L-F14-13: Doc-picture retake command and the settings-button "defaults" label
- **Source:** I6-243, I7-220, I7-227; F14 I6-243; F15 group D; V15.
- **Kind:** K1.
- **Verdict:** Command: NOT VERIFIED. Label owner: CONFIRMED (V15: library_dialogs.cpp:133 `"defaults"`, :137 `"Analysis settings: " + summary`); copies not reopened.
- **Homes:** `pwsh -NoProfile -File tools\doc_shots.ps1` in HY\docs\agents\doc-screenshots.md:5-7, HY\tools\doc_shots.ps1:2, HY\docs\development.md:980, card docs\sessions\2026-10-26-93098c69.md:30. Label copies (finder): tools\doc_shots\shots.txt:17,20,83; docs\UserGuide.md:121-139; tests\test_batch_text.cpp:81-82; tests\single_owner\rules\ui.json:598; "Reset to defaults" owner src\ui\settings_bar.cpp:226.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_batch_text.cpp:81-82 and ui.json:598 hold the owner equal to the literal; shots.txt `expect-text` fails at picture time; test_docs_match_code.cpp ~2036 checks pictures against the shot list, not the command.
- **Open:** Not reached beyond the owner.
- **Hook gap:** none.

### L-F14-14: Second version-refusal gate in the CH tools
- **Source:** I7-103; F14.
- **Kind:** K1, K6.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** tools\ch_mcp\build_facts.py:31-65 (game's own version file; error at :45); a different gate build_stamp.check (build_stamp.py:210, called from frida_tools.py:402 and method_watch.py:437) asks whether the built agent JS is stale; CH:78 describes only the first.
- **Splits:** CH:78 names one gate; code has two. Version-file reader body not opened.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** Not reached.
- **Hook gap:** none.

### L-F14-15: Label cheat-sheet in the UI-testing page versus the code
- **Source:** I7-024; F14.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED (finder: partial; V14: "not checked").
- **Homes:** UT:117-133 (about 150 labels). Owners (finder): named_version, version_bar_status (src\ui\app_state.cpp:1166), analyze_shown_label (src\ui\library_toolbar.cpp:44), settings_button_label (src\ui\library_dialogs.cpp:136), StoreOpenProgress (src\ui\library_jobs.cpp); spot-checked labels exist in src.
- **Splits:** None found.
- **Ruling quotes:** None found.
- **Tests:** none holds the table equal.
- **Open:** Most of the 150 labels unchecked.
- **Hook gap:** none.

### L-F14-16: Other UI-test and CH-tool rows where a doc copies code or the Python writes what the C++ reads
- **Source:** I6-210, I6-229, I7-007, I7-015, I7-016, I7-017, I7-019, I7-022, I7-023, I7-027, I7-028, I7-030, I7-031, I7-033, I7-035, I7-040, I7-075, I7-077, I7-081, I7-082, I7-090, I7-098, I7-099, I7-102, I7-160; F14.
- **Kind:** K1, K6 (weak).
- **Verdict:** NOT VERIFIED (V14: K1 echo rows "stay as the finder wrote them, unverified").
- **Homes (finder only):** all agree today per the finder. Examples: UT:31-40 options versus hydra_ui_mcp.py:374,468-476 flags; UT:71-91 verbs versus tool docstrings in hydra_ui_mcp.py (~780-840) and the " | " bar written in Python and read by split_bar in uitest_commands.cpp:106-120 (K6 weak); UT:105-111 refs versus MCP instructions string :752-760; CH:27 kit map ids (kit_tools.py:64-65,170) also in plans 2026-11-08-kick-axis-bind.md:186; CH:51 FRAME_PROBE_SECONDS = 5.0 (kit_tools.py:737; test_kit_tools.py:1056,1155-1157 hold it); CH:66 protocol date (test_front.py:39-41,542-562); CH:31/78 one-byte patch (test_launch_patch.py, test_launch_core.py); I6-229 UT/IT:25 one home; I7-022 one home; I7-033 `.mcp.json` starts hydra with `py -I` and ch with `py` (no -I).
- **Splits:** None found.
- **Ruling quotes:** None found.
- **Tests:** as listed; for most rows none holds the doc equal.
- **Open:** Not reached.
- **Hook gap:** none.

### L-F14-17: Side notes from F14
- **Source:** F14 side notes.
- **Kind:** K1, K2.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** UT:44 "about 90 s ... measured on 2026-10-10 at commit 4c037634" (a dated number in a doc); UT:48 canned dmleaderboards values (alice, id 111) with no cited home; HY\.claude\worktrees\plan-agent-supervisor\docs\agents\issue-tracker.md is a stale full copy of the issue-tracker page; TM:7 and GATES\README.md:114 both state the python.exe path (see L-F13-06). The finder's first report attempt was refused by the whole-library hook because the text held the flag string (the hook matches on command text alone).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** None.
- **Open:** Not reached.
- **Hook gap:** none.

---

## F15 rows

### L-F15-01: Which `git add` forms may an agent use?
- **Source:** I8-077, I8-244, I7-203 (commit part); F15 group A; V15 split 1.
- **Kind:** K1, K3, K5, K7.
- **Verdict:** CONFIRMED (V15; V15 did not repeat the "no hook reads git add" search).
- **Homes:** BP:63 "Never `git add -A` or `git add .`."; MEM\git-traps-in-shared-checkout.md:19 "Never `git add -A` or `git add -u` in the shared checkout."; finder only: handoff.md:14 and session_cards_start.ps1:287 ("by name"), integrate.md:27,31,50, derive-once-review.md:100, doc-review.md:95, MEMORY.md index line, README:134.
- **Splits:** each omits the other's extra word: BP bans `add -A` and `add .` but not `add -u`; the note bans `add -A` and `add -u` but not `add .`. Differ only for whether an agent may `add -u` (and whether `add .` is allowed).
- **Ruling quotes:** None found (finder searched rulings\*.md for "stage", "add -A").
- **Tests:** none named.
- **Open:** None.
- **Hook gap:** none.

### L-F15-02: When must a session check `.git/MERGE_HEAD` and the staged list before a commit?
- **Source:** I8-078; F15 group A.
- **Kind:** none (single home).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** MEM\git-traps-in-shared-checkout.md:20. Different question in code: GATES\ref_judge.py:336 `current("MERGE_HEAD")`.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** Not reached.
- **Hook gap:** none.

### L-F15-03: What did main gain since the wave forked, and how does the integrator join branches?
- **Source:** I7-201, I7-203; F15 group B.
- **Kind:** K1, K5.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** HY\docs\agents\integrate.md:13 `git log --first-parent <fork>..main --oneline`, :14-15 `git diff <fork>..main -- tests/single_owner/rules/`, :27, :31, :50; derive-once-review.md:155-163,191 (a different step); README:160 main session merges with `--ff-only`; BP:111 and owner-map.md "The stops" for contradiction. See L-F13-09 for the commit-hold split.
- **Splits:** None. Two join procedures by actor (`--no-ff --no-commit` versus `--ff-only`), not in conflict.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** Not reached.
- **Hook gap:** none.

### L-F15-04: Do the stitched-chart merge rules agree with the Clone Hero pages?
- **Source:** I8-165, I8-166; F15 group K.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** MEM\stitched-chart-auto-fills.md:11,16-19 and MEMORY.md:54; HY\docs\clone-hero\04-fills.md FL-27/FL-28/FL-30 (:306-310; FL-27 and FL-28 bodies unread); 02-note-list.md:153 (exact-compare for ENABLE_CHART_DYNAMICS); MEM\ch-disco-noflip-rule.md; the "ffmpeg -ss before -i mis-seeks ogg by about 16 ms" figure appears only in the stitching note.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** Not reached.
- **Hook gap:** none.

### L-F15-05: "Never widen the search band": where is the rule?
- **Source:** I8-064, I8-206, I8-249; F15 group G; V15.
- **Kind:** K1, K3, K7.
- **Verdict:** CONFIRMED (V15), count adjusted: five homes carry the rule and one gives only the reason; all agree.
- **Homes:** MEM\engine-tie-merge-prune.md:22; MEM\fc-video-workflow.md:19; MEM\MEMORY.md:34; C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:58; C:\Users\Patrick\Downloads\Hydra\video-tools\README.md:53; squeeze-tools\README.md:11 "widening it would blow up the search" (reason only). Owner of target search: HY\tools\replay.cpp (usage :100-144); `search_target` in src\search\pather.h (not opened).
- **Splits:** finder only: the skill says the tool runs the targeted search for you, the notes tell an agent to run `hydra_replay target` by hand (two actors, K3 flavour).
- **Ruling quotes:** None found.
- **Tests:** none (prose rule, nothing checks it).
- **Open:** None.
- **Hook gap:** none.

### L-F15-06: May a hook fill in a value itself? ("hooks never guess")
- **Source:** I8-081; F15 group H.
- **Kind:** K7.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** MEM\hooks-never-guess.md:11,15; MEMORY.md:3; HY\docs\superpowers\plans\2026-10-25-hooks-from-scratch.md:108 (same kind of rule for numbers); BP:109 (different question).
- **Splits:** None. The user's words in the note are unfiled; finder searched rulings\*.md for "guess": two unrelated hits (code-and-engine.md:10, :64).
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** No audit of hooks for guessed values was done by the finder.
- **Hook gap:** none.

### L-F15-07: Is malloc redirected? (manual check beside automatic tests); bisect rule
- **Source:** I8-096, I8-102; F15 group I.
- **Kind:** K1, K6.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** MEM\mimalloc-link-order.md:3,15 (`MIMALLOC_VERBOSE=1`, look for "malloc is redirected"); code HY\src\app\allocator.h:28 / allocator.cpp:34 `malloc_redirected()`; tests test_library_jobs.cpp:739,770 and tests\ui\uitest_engine.cpp:157 call it; development.md:231-236. Bisect: MEM\measure-small-explain-first.md:15 and MEMORY.md:53 only.
- **Splits:** A manual prose step and automatic tests both answer "is malloc redirected" (K6-ish, finder).
- **Ruling quotes:** None found.
- **Tests:** the two named tests.
- **Open:** Not reached.
- **Hook gap:** none.

### L-F15-08: FC video tool rules (h264 never to the card, crop align, command, half speed, layouts)
- **Source:** I8-181, I8-182, I8-200, I8-207, I8-208; F15 group K.
- **Kind:** K1, K3.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** C:\Users\Patrick\Downloads\Hydra\video-tools\frames.py:57-78 `NEVER_NVDEC = {"h264"}`, :213-215 `CROP_ALIGN = 16`; video-tools\README.md:56-57,88,96,108-121,508-510,532; MEM\video-decode-ceilings.md:11-19; MEMORY.md:51; analyzing-sp-squeezes\SKILL.md:42,45-46,53,59,60,62. The folder is a separate repo outside HY, so the delete rule's worktree roots do not cover it (BP:43; finder note).
- **Splits:** finder: the skill and note tell the invoker, the README says the tool refuses (owner is the tool).
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** Not reached.
- **Hook gap:** none.

### L-F15-09: Installer staging and release command traps
- **Source:** I8-188, I8-189; F15 groups K/L; V15 split 5.
- **Kind:** K1, K3.
- **Verdict:** Release `--target`: CONFIRMED-CHANGED, weak (V15). Installer staging: NOT VERIFIED.
- **Homes:** MEM\windows-installer.md:19 "`gh release create --target` needs the full SHA; a short one gives HTTP 422."; HY\docs\development.md:1011 passes `'--target','main'` (a branch name) through publish.ps1. Staging (finder only): windows-installer.md:16; HY\installer\build_installer.ps1:78-91 (cmake --install, throws if `*.db`, `*_settings.ini`, `*_ui.ini` leak); development.md:108; the note says "settings .ini", the script's globs are `*_settings.ini, *_ui.ini`. Finder only: CMakeLists.txt:21 owns the version; windows-installer.md:20.
- **Splits:** note "needs the full SHA" versus a working command that passes the branch `main`. A branch name is not a short SHA so both can hold; the note does not say a branch is allowed.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** Whether `main` works needs a release create (not run).
- **Hook gap:** none.

### L-F15-10: Is there a hook for the whole-library run? (stale memory note)
- **Source:** I8-253; F15 group L; V15 split 3.
- **Kind:** K2, K8.
- **Verdict:** CONFIRMED (V15).
- **Homes:** MEM\no-full-library-runs.md:15 ends "No hook enforces it yet."; GATES\percall.py:4,14 `LIBRARY_FLAG = "--whole-library"` enforces it on both sides; README:13; BP:49 "A hook then refuses". Finder only: percall.py:116,129 `_library_refusal`, review_rules.py:50, agent-rules.md:27.
- **Splits:** note "No hook enforces it yet" versus percall.py. A memory note is not the user's words.
- **Ruling quotes:** None found (see L-F13-22 for the library-run ruling).
- **Tests:** test_posture.py, test_progress.py, test_percall_rewrite.py, test_review_rules.py mention whole-library (finder).
- **Open:** None.
- **Hook gap:** none.

### L-F15-11: Where does `kSqueezeWindowMs` live, and what do other docs say?
- **Source:** I8-192, I8-199; F15 group M; V15.
- **Kind:** K1, K2, K8.
- **Verdict:** CONFIRMED (V15).
- **Homes:** HY\src\core\model.h:121 `constexpr double kSqueezeWindowMs = 500.0;` (src\search\graph.h:216 only mentions it in a comment). Docs naming the wrong file: analyzing-sp-squeezes\SKILL.md:10 and squeeze-tools\README.md:9 say `src/search/graph.h`. HY\docs\clone-hero\05-star-power.md:417,419 say "model.h line 111"; real line 121. V15: the page's other line numbers drift too (399 says 123; 295 says 115; 407 says 105 where kHitWindowMs is at 106 per finder), not rechecked each. Finder only: copies of 85 (kHitWindowMs model.h:106) in SKILL.md:18,24; "500 ms" in MEM\sp-squeeze-mechanics.md:3,13,17, MEMORY.md:38; squeeze_scout.py command in SKILL.md:15, README:38-39; the skill says not to read the answer from the GUI (SKILL.md:10,35).
- **Splits:** SKILL.md/README name graph.h, code is in model.h (wrong owner location).
- **Ruling quotes:** rulings\clone-hero.md (Confirmed section, finder, line ~11): "**2026-11-12** 'ok well i am ruling it now: hydra searching for squeezes up to 500 ms is an intentional deviation from the game, not a bug.' (session 375e58cd)". It covers the 500 ms value, not where the constant lives.
- **Tests:** none.
- **Open:** Remaining 05-star-power.md line numbers unchecked.
- **Hook gap:** none.

### L-F15-12: Does a git commit's text count toward the 1,000-character limit? (preamble wider than code)
- **Source:** I9-055; F15 group P; V15 split 6; see L-F13-14.
- **Kind:** K3.
- **Verdict:** CONFIRMED (V15). V15 narrows the finder: the test list has two commit spellings that still count (`git --no-pager commit -m "%s"` and `"git" commit -m "%s"`; test_bash_windows_rule.py:113-114), others in it are not commits (`echo git commit`, `git commit-tree`). Forms that do count as commits: env prefix, `-C`, `-c`, full path, quoted `commit` (test :105-107).
- **Homes:** BP:33 "A `git commit`'s own text does not count toward that limit, so a long commit message is fine."; bash_windows_rule.py:172-174 `_too_long` subtracts `_commit_text`.
- **Splits:** BP:33 says any git commit message is free; the code charges `--no-pager commit` and a quoted `"git" commit`. Differ for those two spellings.
- **Ruling quotes:** review-and-hooks.md:40 "Exempt git commit" is Unconfirmed (finder); none under `## Confirmed`.
- **Tests:** test_bash_windows_rule.py:66-122 (code held to the cases, not BP).
- **Open:** None.
- **Hook gap:** none.

### L-F15-13: Which hook tags, cut length and shell tools do the Windows-rule texts and tests copy?
- **Source:** I9-054, I9-056, I9-057, I9-058, I9-059; F15 group P.
- **Kind:** K1, K5, K6.
- **Verdict:** NOT VERIFIED (V15 did not reach them: "SHELL_TOOLS vs `tool_name != "Bash"`, status-prefix copies" listed as not checked).
- **Homes (finder only):**
  - Refusal tags `[length]`, `[backslashes]`, `[cmd]`, `[ref]`, `[path quote]` at bash_windows_rule.py:211-227; the 300-character cut at GATES\common.py:46 `reason[:300]`, README:81, test_bash_windows_rule.py:307 literal 300; BP:33 "five shapes" and examples; the Write-tool advice repeated in the message and BP:33 (test :59-63).
  - Two code places decide which tools are shell tools: progress.py:20 `SHELL_TOOLS = ("Bash","PowerShell")` versus bash_windows_rule.py:235 `tool_name != "Bash"` (different questions: any shell versus Bash only; percall.py:112-116 runs the rule for both sides).
  - percall.py:112 skips the rule for status calls; test :139-144 builds a status line from STATUS_PREFIX.
- **Splits:** None stated.
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers.py:192-196 holds only the number 1,000 to code; the shapes in BP are not held.
- **Open:** Not reached.
- **Hook gap:** none.

### L-F15-14: Media guard, crash modes, background-task wordings (tests copy hook values)
- **Source:** I9-012, I9-015, I9-042; F15 group P.
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED (V15: media guard literal and `run_in_background` hook gap not checked).
- **Homes (finder only):** GATES\media_rule.py:6 `MEDIA_ROOT = r"C:\ytdl"`, :53 reason wording; test_refusals.py:61-67 and test_media_rule.py:53-59 hard-code `C:\ytdl` and "media guard"; README:21 states the rule without the folder name; the test checks only the main-session ask. GATES\common.py:53-69 `run` modes "closed" and "loud"; README:97-98; test_common.py:84-110. GATES\agent_stop.py:45-51 `_BACKGROUND_STARTED`, `_TASK_ENDED`; test_agent_stop.py:100-126,239-247 copy the four wordings (comment: "copy the wording of real agent transcripts"); README:41; BP:77 says never use run_in_background and no hook refuses it.
- **Splits:** None stated.
- **Ruling quotes:** None found.
- **Tests:** none holds the test literal equal to MEDIA_ROOT.
- **Open:** Not reached.
- **Hook gap:** none stated.

### L-F15-15: REA skill: version pin, client ids, routing, summary rule
- **Source:** I8-215, I8-216, I8-217, I8-219, I8-221, I8-234; F15 group N; V15.
- **Kind:** K1, K2, K7.
- **Verdict:**
  - Pin: CONFIRMED-CHANGED (V15): six places read, not seven.
  - Client ids: CONFIRMED-CHANGED (V15).
  - Routing, "setup never installs", "detail: summary" repeats, I8-234: NOT VERIFIED.
- **Homes:**
  - Pin `rea-agents@6.5.0` at C:\Users\Patrick\.claude\skills\reverse-engineer-anything\SKILL.md:37,51,56,71,79 and TM:25; the registration in C:\Users\Patrick\.claude.json was read by neither finder nor V15 (the finder's seventh home).
  - Client ids: SKILL.md:37-42 lists 15 ids ending `grok_bot`; references\connection-and-recovery.md:46-50 lists 23 and does include `grok_bot` (finder said not). Reference adds qwen_code, trae, pi, hermes, qoder, droid, zcode, dsh; every SKILL id is in the reference.
  - Finder only: SKILL.md:91-205 routing and references\javascript-applications.md; SKILL.md:62-64,81-83 "never installs" stated twice; `detail: "summary"` at SKILL.md:122,138,162,242-246,260,266,297 and references\runtime-observation.md:67, javascript-applications.md:21.
- **Splits:** SKILL.md versus connection-and-recovery.md list different sets of "the current supported client"; a SKILL.md reader gets the shorter list. A version bump edits six or seven places.
- **Ruling quotes:** None found.
- **Tests:** none (finder: G\tests has no hit for "rea-agents").
- **Open:** I8-234 reference files not read; ~/.claude.json not read.
- **Hook gap:** none.

---

## Closing lists

Candidates the verifier REFUTED (whole rows): none. Two sub-claims were refuted inside L-F13-13 (V13b): the finder's "one build per session is not enforced" (build_slot.ps1:195 enforces it) and the finder's "two state folders for build locks" (hooks\state holds only build_slots; no gates file mentions it).

Rows narrowed by a verifier (CONFIRMED-CHANGED): L-F13-08, L-F13-13, L-F14-01, L-F14-05, L-F14-07, and parts of L-F14-10, L-F14-11, L-F15-09, L-F15-15.

Rows no verifier reached (NOT VERIFIED): L-F13-16, L-F13-19, L-F13-20, L-F13-21, L-F13-23, L-F13-24, L-F14-12, L-F14-14, L-F14-15, L-F14-16, L-F14-17, L-F15-02, L-F15-03, L-F15-04, L-F15-06, L-F15-07, L-F15-08, L-F15-13, L-F15-14. Parts not reached inside mixed rows: L-F13-17 (kind lists), L-F13-22 (all but the flag literals), L-F14-10 (all but the wording gap), L-F14-11 (F14's I7-073 reading), L-F14-13 (retake command), L-F15-09 (installer staging), L-F15-15 (routing, summary rule, I8-234).
