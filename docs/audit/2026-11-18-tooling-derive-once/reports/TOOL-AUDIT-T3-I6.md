# TOOL-AUDIT-T3-I6 census (scout I6)

Repo read: C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout), `git log -1` = af9dfac4e246df6b77c4b906f5c7054f47beaa82. All 14 files are in git. All 14 files read in full; none missed.

File codes used below (all under docs/agents/): DOR = derive-once-review.md, DR = doc-review.md, HO = handoff.md, IN = integrate.md, AM = audit-method.md, OM = owner-map.md, PM = premise-map.md, TM = this-machine.md, RR = rulings/README.md, IT = issue-tracker.md, TL = triage-labels.md, DM = domain.md, DS = doc-screenshots.md, PD = public-doc-style.md.
Row form: id | question | file:line | value (quoted) | owner claim | who acts | who is asked to supply.
"Owner claim: none" means the text makes no claim to be the owner. "see X" means the same value also appears in row X.

## derive-once-review.md (DOR)

I6-001 | which model runs the derive-once reviewer | DOR:5 | "You run on Sonnet." | none | reviewer agent | none (also DOR:202, DR:5; see I6-089)
I6-002 | may a reviewer use helpers or background jobs | DOR:5 | "You use no helper agents and start no background jobs; do every read yourself, in the foreground." | none | reviewer | none
I6-003 | which page binds the reviewer first | DOR:7 | "Read `docs/agents/brief-preamble.md` first." | none | reviewer | none
I6-004 | how often a reviewer writes a status line, in what form, by what call | DOR:9 | "every 5 minutes, form `HH:MM done ... | next: ...`"; call `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"`, "alone in its own call" | none (duplicates DR:9; see I6-090) | reviewer, status hook | reviewer supplies the line
I6-005 | where the status file lives | DOR:9 | `C:\Users\Patrick\.claude\gates\state\status\<your agent id>.md` | none | reviewer | none
I6-006 | the one rule a review enforces | DOR:13 | "Every fact, rule or calculation in Hydra is derived in exactly one place." | none (rule restated; agent rule 15 elsewhere) | reviewer | none
I6-007 | what the reviewer is handed | DOR:17 | "a **key** (a 40-character hash) and a **range**. The range is always `main...<key>`" | none | reviewer; orchestrator supplies | orchestrator supplies key, range, worktree, author name, task and session ids
I6-008 | when a reviewer must stop and report | DOR:17 | "a staged change on main, a key that is not a commit on a branch, or a brief that tells you not to commit" | none | reviewer | none
I6-009 | what a reviewer dispatch hook refuses | DOR:17 | "a reviewer dispatch with no key, a key already on `main`, a key git does not know or more than one commit" | none | reviewer-dispatch hook (unnamed) | none
I6-010 | may a reviewer write before its review is written | DOR:17 | "Until your review is written down, in a file in your scratchpad, you are read-only" | none | reviewer | none
I6-011 | what precheck prints and who owns its threshold | DOR:21 | "finds kinds B, C, E and F mechanically, and kind D's new numbers"; "The precheck owns that threshold and which files count; this page never restates them." | claims owner: precheck owns threshold | reviewer; hydra_precheck | none
I6-012 | how a reviewer runs precheck | DOR:24 | `build-cpp\Release\hydra_precheck.exe --repo <path> --range main...<key>` | none (also IN:37, DOR:99, DOR:116) | reviewer | none
I6-013 | the kinds of copy and their letters | DOR:28-38 | kinds A, B, C, D, E, F, H, and G ("kind G is under ..."); text of each kind | none | reviewer | none
I6-014 | where the pasted-block scan lives | DOR:38 | "the clone scan in `tests/single_owner/clones.cpp` (ADR 0025) already fails on any pasted block at least `kCloneWindowLines` code lines long" | claims owner of the length: `kCloneWindowLines` | reviewer | none
I6-015 | a finding on known_clones | DOR:38 | "A change that adds an entry to `known_clones` is a finding." | none | reviewer | none
I6-016 | the ten review steps and the questions in each | DOR:42-55 | steps 1-10 | none | reviewer | none
I6-017 | where a user decision for a new number must live | DOR:45 | "an ADR in `docs/adr/`, `CONTEXT.md`, a line under `## Confirmed` in `docs/agents/rulings/`, or the \"User decisions\" header of a plan. A code comment or a plan's task steps don't count." | none | reviewer | none
I6-018 | what counts as a decision entry | DOR:45 | "counts only when it quotes the user's words as they are found under `## Confirmed` or in a session card" | none | reviewer | none
I6-019 | test rubric page | DOR:46 | "`docs/agents/test-rubric.md`" | none | reviewer | none
I6-020 | which file lists known audit copies | DOR:48 | "`docs/audit/2026-10-03-derivation-audit.md`" ; "already on the fix list" | none | reviewer | none
I6-021 | who defines project words | DOR:51 | "`CONTEXT.md` decides what a project word means." | claims owner: CONTEXT.md | reviewer | none
I6-022 | what to do when a CONTEXT.md definition and code disagree | DOR:51 | "that is a question for the user, written in game terms with both readings. Change neither" | none (also preamble) | reviewer | user answers
I6-023 | what counts as a user's words for a visible change | DOR:52 | "a D-record, a plan's \"User decisions\", a line under `## Confirmed` ..., a session card, or the words your brief quotes. A line under `## Unconfirmed` is not the user's ruling" ; "The user ruled this on 2026-10-31 (\"Yes, add it (Recommended)\")." | none | reviewer | none
I6-024 | the Visible changes rule's example | DOR:52 | "D13 approved \"free\", not the sentence \"no more than 0.0 ms late\"" | none | reviewer | none
I6-025 | step 10 boundary table shape | DOR:53 | "a table with four columns" (value, what owner worked it for, what reader applies it to, same?) | none | reviewer | none
I6-026 | step 10 arithmetic rule | DOR:55 | "A rule needs an owner in the engine or a ruling quoted in the user's words ... With neither, it is a kind D finding"; second table, four columns | none | reviewer | none
I6-027 | worked example numbers | DOR:57-64 | "tick 198570", "158,224.76 ms", "chord 644", "tick 202440 (161,356.44 ms)", "1.1067", "158,254.77 ms", "24.12", "3.00", "158,268.99 ms", "14.21 ms", "1.8 ms" | none | reviewer (example only) | none
I6-028 | where the squeeze ratio between two ends is computed | DOR:57,64 | "`SPSqueeze::transfer`", "`frontend_transfer_scales` (`src/core/squeeze_rating.cpp`)", "`transfer_scale_between`" | claims `transfer_scale_between` "takes an anchor tick and an end tick" and works out the ratio between two ends | reviewer | none
I6-029 | where the bot shifts later presses | DOR:66 | "`retime_from_end` in `tools/ch_probe/experiments/play_chart.py` moves each later press by the read end minus the planned end, one for one" | none (a description of another tool's rule) | reviewer | none
I6-030 | the replay tool's SP-end lead | DOR:108 | "presses 15 ms before a Star Power end (`kSpEndLeadMs` in `tools/replay_schedule.cpp`)" | none | reviewer | none (example of a number needing a ruling)
I6-031 | the 3.00 ms frame of judging | DOR:64 | "3.00 ms is the one frame of judging the tool counts" | none | reviewer (example) | none
I6-032 | kind G rule: what may set a loop's cost | DOR:74 | "A file's size is fine. A number read out of the file is not: a tick, a count, a length, a tempo or a resolution." ; "966-byte file took 2.7 GB" | none | reviewer | none
I6-033 | rule for tool reading names | DOR:76 | "Every name comes from the repo, not memory." ; "a grep that finds nothing proves nothing" | none | reviewer | none
I6-034 | one review per change, all in one pass | DOR:78 | "There is one review per change." | none | reviewer | none
I6-035 | kind A fix ends in a scan row | DOR:80 | "The row goes in the owner's area file under `tests/single_owner/rules/`, and it needs every field `parse_rule` in `tests/single_owner/engine.cpp` requires. ADR 0029 lists those fields" | claims ADR 0029 owns the field list; `parse_rule` owns the required fields | reviewer | none
I6-036 | comment that restates a rule | DOR:82 | "A comment, header or doc line that spells out what a rule matches ... is kind D." | none | reviewer | none
I6-037 | developer tools wording rule | DOR:86 | "a comment, header or doc line that states a rule differently from code that behaves correctly is a note, not a finding" ; "The user decided this in D59." | none | reviewer | none
I6-038 | one review by one agent, no second round | DOR:90 | "The user decided in D107 (`docs/audit/2026-10-03-fix-decisions.md`)" ; "30 minutes from its first call and never restarts the count" | none (D107; also DR:91) | reviewer | none
I6-039 | fixes: one finding per commit | DOR:94 | "one finding per commit" ; commit body "carries the grep you ran and its one-line result" | none | reviewer | none
I6-040 | test filters a reviewer lists | DOR:99 | `build-cpp\Release\hydra_tests.exe -tc="single-owner*"` ; `build-cpp\Release\hydra_precheck.exe --repo <path> --tests main...<your last commit>` | none (also IN:43-46) | reviewer lists; main session runs | none
I6-041 | which findings a reviewer must not fix | DOR:106 | "a new number (a threshold, floor, tolerance or band), a change to a displayed text, a score or a stored record, a change to what a `CONTEXT.md` term means, or a choice between two behaviours" | none | reviewer | user asked via Notes
I6-042 | what holds the merge | DOR:106 | "three exceptions, which hold the merge until the user answers": a visible change marked "not asked", a kind H finding not fixed, a kind D step-10 rule not fixed; Note starts "Holds the merge:" ; "The user ruled this on 2026-11-12 (\"alright approve b\")." | none | orchestrator | user answers
I6-043 | out-of-time rule | DOR:110 | "at 100 tool calls or 20 minutes" ; "D107 item 5" | none (see DR:100, preamble) | reviewer | none
I6-044 | where fixes are committed | DOR:112 | "on the same branch, on top of the key you were given. Never move the branch anywhere else." ; hook "checks that it descends from the key you were handed" | none | reviewer; verdict-recording hook (unnamed) | none
I6-045 | who may build or run tests | DOR:116 | "You never build, and you never run `hydra_tests.exe`, `hydra_uitest.exe` or `hydra_replay.exe`" ; "agent rule 14" | none | reviewer | none
I6-046 | how to test a gates change | DOR:124 | `C:\Users\Patrick\AppData\Local\Python\pythoncore-3.14-64\python.exe -I -m unittest discover -t . -s gates/tests -p <file>` ; "`C:\Users\Patrick\.claude\gates\README.md` explains why that form" | none (python path also TM:7) | reviewer | none
I6-047 | report sections and sign-off line | DOR:128-151 | sections (Questions this change answers, Findings, Visible changes, Precheck lines dropped, Notes, Touched already on the fix list, Scan rows added, Where I looked); "SIGN-OFF: CLEAN" or "SIGN-OFF: BLOCKED"; "Write no hash, key or path with it." | none | reviewer; verdict hook | reviewer writes it
I6-048 | what a missing sign-off line counts as | DOR:151 | "A report with no sign-off line is recorded CLEAN" ; "Only `SIGN-OFF: BLOCKED` holds the merge." | none (also DR:125) | verdict hook | none
I6-049 | what a verdict fingerprints | DOR:151 | "it fingerprints every markdown file the range changes at that tip" | none | verdict hook | none
I6-050 | resolution review key and command | DOR:157-163 | `git show --remerge-diff <M>` ; "Your key is the merge commit, M." | none | reviewer | none
I6-051 | what a resolution review is allowed for | DOR:171 | "only for a branch whose tip before the merge already had its own full review" | none | main session | none
I6-052 | fix review: when allowed | DOR:175 | "the one case where a signed-off change gets another look" ; "the user approved it in D314" ; command `git log -p <signed tip>..<key>` | none | main session; reviewer | none
I6-053 | who runs the merge and what refuses it | DOR:189 | "Only the user can skip a review, by typing `waive derive-once <key>` in chat. Never ask the user to waive one to save time." | none; names "the ref gate" | ref gate; main session; user | user types the waive line
I6-054 | merge of main into a reviewed branch | DOR:191 | "The ref gate lets that merge onto `main` when git, merging old `main` with a commit a reviewer signed off, makes exactly the same tree with no conflicts." | none | ref gate | none
I6-055 | large range split | DOR:193 | "The precheck prints the range's size ... a large-range note ... over the threshold that `hydra_precheck.exe` owns (`tools/precheck/items.h`). A range with that note should go in as two or more merges." | claims `tools/precheck/items.h` owns the threshold | main session | none
I6-056 | where worktrees are cut | DOR:197 | `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\` or `C:\Users\Patrick\.claude\worktrees\` ; "the delete rule lets an agent delete only inside a worktree there" | none (a second path list; the preamble also holds it) | main session; delete-rule hook | none
I6-057 | how the reviewer is dispatched | DOR:200 | "`subagent_type: reviewer`, and leave `isolation` out" ; "the hook refuses a reviewer dispatch that sets `isolation`" ; "the full 40-character hash of the branch tip" | none | main session; dispatch hook | main session supplies key
I6-058 | what a non-Agent-tool reviewer's sign-off counts for | DOR:200 | "A reviewer launched from a Workflow script, or as any other agent type, gets no such record, so its sign-off counts for nothing." | none | verdict hook | none
I6-059 | what the main session gives the reviewer | DOR:202 | list: "this page, the key ..., the range `main...<key>`, the branch's worktree, the precheck output, the author's report, the author's name or agent id, and the task and session ids" | none | main session | main session supplies
I6-060 | model for reviewer | DOR:202 | "one fresh Sonnet reviewer per change" | none (see I6-001) | main session | none
I6-061 | when a not-asked visible change blocks | DOR:202 | "If its \"Visible changes\" marks any change \"not asked\", hold the merge" ; "agent rules 11 and 12" | none | main session | user asked
I6-062 | build-fail path | DOR:202 | "If its C++ fix fails that build, send a fresh executor ... If that fix fails the build too, stop and tell the user." | none | main session | none

## doc-review.md (DR)

I6-063 | what a doc reviewer's one question is | DR:3 | "would a reader who trusts this document be misled by it, or be unable to act on it?" | none | doc reviewer | none
I6-064 | doc reviewer model and helpers | DR:5 | "You run on Sonnet. You use no helper agents and start no background jobs" | none (see I6-001) | doc reviewer | none
I6-065 | doc reviewer status line | DR:9 | "every 5 minutes, form `HH:MM done ... | next: ...`" with the status_append.ps1 call | none (duplicate of I6-004) | doc reviewer | reviewer supplies line
I6-066 | where the numbered source tags point | DR:11 | "numbered sources at the end of `docs/research/ai-generated-docs-review.md`" | none | doc reviewer | none
I6-067 | which moves of main the doc gate covers | DR:17 | "a move of `main`, local or pushed, that changes `README.md`, `docs/README.md`, `docs/UserGuide.md` or `docs/differences-from-public-hydra.md`; an Artifact publish; a plan shown with ExitPlanMode; and text posted to GitHub" | none (the four public pages also named PD:3) | doc-review gate hooks (unnamed) | none
I6-068 | which repos the gate applies in | DR:17 | "repos whose `main` holds a `.doc-review-gate` marker, as this one does" | none | gate hooks | none
I6-069 | how short a text may pass without review | DR:17 | "A short text passes the last three with no review; `publish_gate.py` in the gates folder owns that limit." | claims owner: `publish_gate.py` | gate hooks | none
I6-070 | what happens after sign-off | DR:17,102 | "later edits to it pass with no new review" ; "Text posted to GitHub is the exception: it passes only as the exact text a reviewer signed off." ; "The marker file is the one exception: every change to it is reviewed." | none | gate hooks | none
I6-071 | which docs are not gated | DR:17 | "Internal docs (`CLAUDE.md`, `CONTEXT.md`, `docs/agents/`, ADRs, handoffs, committed plans) and questions to the user are not gated." | none | gate hooks; writers | none
I6-072 | the only way text reaches GitHub | DR:19 | "Text goes to GitHub only through `C:\Users\Patrick\.claude\gates\publish.ps1`"; kinds "`pr`, `release`, `release-edit`, `issue`, `issue-comment` or `pr-comment`" | none (duplicate of IT:5,9,12, TM:15; kinds list also IT) | publish.ps1; settings deny rules | writer supplies kind and body file
I6-073 | how a document is sent for review | DR:21 | "Commit the document on its own branch, then dispatch a reviewer ... The brief names the branch tip and the range, and no key for the document." | none | main session | none
I6-074 | which design note is superseded | DR:21 | "that page and the gates code win over the older design notes in `docs/superpowers/plans/2026-10-08-doc-review-gate.md` (removed; `git show ec91f8d1:...`)" ; "`C:\Users\Patrick\.claude\gates\README.md`" | claims the gates README and code win | readers | none
I6-075 | skipping a doc review | DR:21 | "Only the user can skip a review, by typing `waive doc-review <key>` in chat." | none | user; gate hook | user types the line
I6-076 | the four failure kinds | DR:25 | "A number has no source. A reference exists but does not say what the sentence claims. A summary quietly drifts ... A confident sentence covers a guess." | none | doc reviewer | none
I6-077 | what a doc reviewer is given | DR:29 | key (40 chars), range `main...<key>`, documents (`git diff --name-only main...<key>`), sources, author name | none | doc reviewer | main session supplies sources
I6-078 | missing sources rule | DR:35 | "If the dispatch is missing the sources, stop and report it." ; "No hook checks that the sources are there" | none | doc reviewer | none
I6-079 | cold read output | DR:41 | "at most five sentences, what it decides, what it asks the reader to do, and the three numbers the reader will rely on" | none | doc reviewer | reviewer writes Restatement
I6-080 | claim-check rules | DR:51-63 | numbers, references, paraphrases, unsourced claims, compliance claims, decision questions, plans (the listed checks) | none | doc reviewer | none
I6-081 | what counts as a ruling in a plan | DR:63 | "A ruling counts only when it quotes the user's words from a line under `## Confirmed` ..., a session card or the brief's quote" | none (same set as DOR:45,52, PM:45) | doc reviewer | none
I6-082 | plan owner-table rule | DR:63 | "A step that moves or converts an engine-owned quantity is not a \"live part\"... Sending a press and reading a value are live parts." | none | doc reviewer | none
I6-083 | where the style rules live | DR:81 | "`C:\Users\Patrick\.claude\CLAUDE.md`. That file is the one home of the style rules" | claims one home | doc reviewer | none
I6-084 | check not to do | DR:83 | "Do not judge whether the text \"reads like AI\"." | none | doc reviewer | none
I6-085 | doc review fix/sign-off flow | DR:91-100 | "D107"; "30 minutes from its first call"; "at 100 tool calls or 20 minutes" | none (duplicates DOR:90, DOR:110; see I6-038, I6-043) | doc reviewer | none
I6-086 | findings not to fix | DR:98 | "a new number, a change to what the user sees or what is stored, or a choice between two behaviours" | none (shorter copy of DOR:106) | doc reviewer | user via Notes
I6-087 | doc review sign-off lines | DR:117-125 | "SIGN-OFF: CLEAN" / "SIGN-OFF: BLOCKED"; "A report with no sign-off line is recorded CLEAN" | none (see I6-047, I6-048) | verdict hook | doc reviewer writes
I6-088 | doc review report sections | DR:108-115 | Restatement, Findings, Numbers checked, References opened, Notes, Where I looked | none | doc reviewer | reviewer writes

## handoff.md (HO)

I6-089 | where a handoff lives and how named | HO:7 | "`docs/handoffs/<YYYY-MM-DD>-<slug>-handoff.md`" ; "The date in the name is the day it was written." | none | `/handoff` command; checker | writer supplies slug and date
I6-090 | what the handoff command is | HO:3 | "`/handoff` command (`.claude/commands/handoff.md`)" | none | main session | none
I6-091 | what a handoff holds | HO:11-13 | "prose and one table" ; goal, what was done, what is next ; "quotes the user's rulings ... each with its session id" | none | handoff writer | writer supplies
I6-092 | the Claims table: heading, columns, kinds | HO:15-25 | heading "exactly `## Claims`" ; columns "Kind, Value, Claim, How to check" ; seven kinds: commit, branch, path, function, test, ruling, state ; "Write the Value column in backticks" | none | writer; checker | handoff writer
I6-093 | commit value format | HO:19,35 | "the full 40-character hash, in backticks" ; "Write every hash in full, all 40 characters." | none (40-char also DOR:17) | checker; writer | writer
I6-094 | state a handoff ends with | HO:27 | "After the table comes a heading `## Checker output`" | none | writer | writer pastes output
I6-095 | a claim without a row | HO:31 | "A claim with no row is not a claim, and the next session may not act on it." | none | next session | none
I6-096 | a failing row | HO:33 | "A row whose check fails is wrong until someone fixes the handoff." | none | next session | none
I6-097 | scratch paths in a handoff | HO:37 | "Name no scratch path." ; "The docs test's scratch-path check skips `docs/handoffs/`" | none | writer | none
I6-098 | checker command | HO:45 | `py -I tools/check_handoff.py <handoff.md> [--repo <root>] [--since <YYYY-MM-DD>]` | claims the script owns statuses | main session; agents | none
I6-099 | row status words and when exit is nonzero | HO:51-57 | "`ok`, `missing`, `not-on-main`, `not-pushed`, `manual`" ; "exits 0 only when every row the checker judges ... is `ok`; it exits 1 otherwise and 2 on a bad argument" | none | checker (tools/check_handoff.py) | none
I6-100 | which kinds the checker leaves manual | HO:39,55 | "`test`, `ruling` and `state` rows ... `manual`" | none | next session | none
I6-101 | scrape mode: how tokens are sorted | HO:59 | "A hex word is a commit. A branch name git knows is a branch. A token with a path separator or a known extension is a path. A token shaped like `<name>()` or `<Class>::<method>` is a function." ; summary form "`checked N claims: E exist, M missing, C changed since <date>`" ; exits 0/1/2 | none (a copy of the checker's rule) | checker | none
I6-102 | which mode applies | HO:47 | "chosen by whether the file has a `## Claims` heading" | none | checker | none
I6-103 | who runs the checker and when | HO:65-67 | "The `/handoff` command runs it right after writing a handoff" ; "The session that continues a handoff runs it again before any work starts" | none (also preamble "Continuing from a handoff") | `/handoff`; next session | none
I6-104 | repo flag | HO:47 | "Pass `--repo <root>` to check commits and paths in another repo, such as the hooks repo." | none | checker | none

## integrate.md (IN)

I6-105 | the integrator's job | IN:3,5 | "You join a wave's task branches onto the wave branch" ; "You write no new feature code." | none | integrator agent | none
I6-106 | what the brief names | IN:3 | "the wave branch, its worktree, the task branches in the order to join them, and the fork point" | none | integrator | main session supplies
I6-107 | list of what main gained | IN:13,15 | `git log --first-parent <fork>..main --oneline` ; `git diff <fork>..main -- tests/single_owner/rules/` | none | integrator | none
I6-108 | how to check a task branch calls owners | IN:21 | "grep the task branch's diff (`git diff <fork>...<task branch>`) for the inputs that owner reads" | none | integrator | none
I6-109 | stop condition on fold | IN:23 | "If a fold would change a displayed text, a score or a stored record, stop and report it." | none (a shorter copy of DOR:106) | integrator | none
I6-110 | join command | IN:27 | `git merge --no-ff --no-commit <task branch>` ; "Hold the last task's merge uncommitted until the precheck in step 4 is clean" | none | integrator | none
I6-111 | appended scan-row files | IN:29 | "Rebuild the file as the wave branch's version plus the other task's added rows, relative to the fork." | none | integrator | none
I6-112 | conflict rule | IN:31 | "Never rebase, amend, reset or force-move a branch." ; "stop and report both sides" | none | integrator | none
I6-113 | who builds and tests the joined branch | IN:35,41 | "You do not build the joined branch or run its tests" | none | main session | none
I6-114 | precheck on joined branch | IN:37 | `build-cpp\Release\hydra_precheck.exe --repo <worktree path> --range main...<wave branch>` | none (duplicate of I6-012) | integrator | none
I6-115 | tests the main session runs after hand-over | IN:43-46 | `build-cpp\Release\hydra_tests.exe -tc="single-owner*"` ; `build-cpp\Release\hydra_tests.exe -sf=*docs_match_code*` ; `... --tests main...<wave branch>` | none (the scan filter duplicates I6-040) | main session | none
I6-116 | what to do when scan row fails | IN:48 | "Never loosen a scan row to make it pass." | none | executor | none
I6-117 | after the last merge | IN:54 | "Stop changing the tree once the last merge is committed. The gate keys a review on the exact tree, so any later commit voids it." | none | integrator; gate | none
I6-118 | who reviews the joined range | IN:56 | "hands the key, the range and your precheck output to a fresh Sonnet reviewer with `docs/agents/derive-once-review.md`" ; "so you cannot review this range yourself" | none | main session | none
I6-119 | integrator's report | IN:60 | wave branch, worktree, head hash, each task branch with merge hash, owners gained, folds, conflicts, precheck output, stops | none | integrator | integrator writes

## audit-method.md (AM)

I6-120 | what an audit is | AM:3 | "agents reading Hydra's code to find places where it breaks its own rules" | none | main session | none
I6-121 | words: finding, split, exit, closure | AM:11-17 | definitions of four words | none | main session; all audit briefs | none
I6-122 | the four exits | AM:19-26 | "Fixed", "A question to the user", "Covered by a ruling", "Not a problem" ; "Every finding leaves by exactly one" | none ("the four exits" also in CLAUDE.md, DOR:106 and the brief) | main session | none
I6-123 | what a ruling closure needs | AM:25 | "a verbatim quote of their ruling, with its file and line, and the quote names both sides of the split" ; "When the user answered a batch with \"all recommended\", their words are the question's own text and its Recommended line." | none | main session; checker | none
I6-124 | what "not a problem" needs | AM:26 | "one sentence saying what each side answers, which a fresh agent agrees with" ; "\"On purpose\", \"by design\", \"deliberately\" and \"kept\" are not evidence." | none (word list also in the brief) | main session | none
I6-125 | "code-only" status | AM:28 | "\"Code-only\" is not an exit." ; "A split is never code-only" | none | main session | none
I6-126 | who the audit plan goes to | AM:34 | "It goes to the user and waits for their yes (CLAUDE.md agent rule 3)." | none | main session | user approves
I6-127 | audit scope | AM:36 | "The scope is the user's words" ; quotes "the first pass should cover everything" (D113, card `2026-10-11-8bffee2b`), "Leave nothing in a \"pre-existing, left alone\" pile." (D119) | none | main session | none
I6-128 | decisions an audit plan must list | AM:38 | "`docs/audit/2026-10-03-fix-decisions.md`, `docs/adr/` and the plans under `docs/superpowers/plans/`" | none | main session | none
I6-129 | cap on repeating stages | AM:40 | "Every stage that repeats needs a cap and a line saying what happens at the cap." | none | main session | none
I6-130 | calibration rule | AM:44-46 | "one agent gets a file with known problems and no hint of what they are" ; "src/squeeze_rating.cpp at 707c285" (`src/core/squeeze_rating.cpp` at 707c285) ; "If the calibration agent misses a known problem, stop and report" ; "must not describe the known problems, even as an example" | none | main session | none
I6-131 | what a finder reports | AM:50 | "each home ..., and a table of inputs and answers for each home, with the edges in" ; "A finder does not grade its own candidates." | none | finder agents | finder supplies
I6-132 | how finders split work | AM:52 | "by family of questions, not by folder" | none | main session | none
I6-133 | a split row must reach the user | AM:54 | "When a table shows two homes giving different answers for one input, that row is a split. It must reach the user" | none | main session | none
I6-134 | verifier rules | AM:58-64 | "A verifier never gets a candidate from a group it scouted." ; marks claims "by reading" or "proven by" ; "lists every split its tables show, as separate lines" ; side notes get an exit ; live-game claims stay open | none | verifier agents | verifier supplies
I6-135 | how exits are evidenced | AM:68-75 | per-exit evidence list ; "A closure needs a ruling that names both values. If the main session can't find one, the finding becomes a question." | none | main session | none
I6-136 | no exit for a case | AM:77 | "When the plan has no exit for a case, the finding becomes a question." | none | main session | none
I6-137 | closure check | AM:81-85 | "A fresh agent checks every closure: every finding that left by exit 1, 3 or 4." ; verdicts "PASS", "REOPEN", "GONE" | none | checker agents | checker supplies verdict
I6-138 | main session greps exit-3 quotes | AM:87 | "`docs/audit/2026-10-29-audit-of-audits/briefs/checkquotes.py` is one" ; "A checker that shortens file names must write a legend" | none | main session | none
I6-139 | where checkers search for rulings | AM:89 | "it searches the session cards, not just the decision log" ; card `2026-10-24-a124ccd9` | none | checker agents | none
I6-140 | question-writing rules | AM:93-102 | eight bullets (what the user will see, both sides' real values, an example from each end and the gap, no leaning + recommendation + strongest case against, ask about the split itself, show every value, re-read the code the day it goes out, never give a split a purpose nobody wrote down) | none (overlaps agent rule 11) | main session | main session writes questions
I6-141 | how many questions per message | AM:104 | "Send splits alone or in small groups. The user approved that rule on 2026-10-29 (\"yes both\"). The number is not the user's: \"five or fewer per message\" is the main session's own suggestion" | none | main session | user approved the rule; the 5 is the main session's own
I6-142 | recording | AM:108 | "Each answer becomes a D-record in `docs/audit/2026-10-03-fix-decisions.md`" ; triage file "`docs/audit/2026-10-03-fix-triage.json`" records "each finding's exit and its evidence" | none | main session | none
I6-143 | triage update time | AM:110 | "The merge that fixes a finding updates its row in the same step." ; "192 of 396" | none | main session | none
I6-144 | fix grouping and models | AM:114 | "executors on Opus, one worktree each, grouped so that no two of them own the same file. Each merge gets one fresh reviewer (D107)." | none (model rule is agent rule 2 elsewhere) | main session | none
I6-145 | what every audit brief carries | AM:118-127 | list: preamble, task id/session id/files/caps/status rule, four exits, every split its own line, the four claim words, "a search that finds nothing proves nothing", "no whole-library run happens without the user's `allow library run` in chat (CLAUDE.md agent rule 8)", where the user's words can be, exact ids for a second wave | none | main session writes briefs | none
I6-146 | M39 numbers | AM:13,135 | "from -10 ms to under 3 ms" ; "from -10 ms to under 10 ms" ; "kBackendInnerBandMs"; "c9c00cf7 (2025-08-12)"; "a note 5 ms after the end" | none (history; code owner named: `BackendSqueeze::summarystr` in `src/core/model.cpp`, AM:190) | readers | none
I6-147 | historic counts | AM:140-178 | "all 33 questions"; "26, 14, 11 and 12 new findings"; "44 more"; "219 closures ... 24 of them, about one in nine" ; "round six" ; "four of five side notes held" | none | readers | none
I6-148 | where the audit's own sources live | AM:190 | "`docs/audit/2026-10-03-derivation-audit.md`", "`docs/audit/2026-10-04-phase-3-5-questions.md`", "`docs/audit/2026-10-11-concept-audit/findings/V5.md`", "`docs/audit/2026-10-11-concept-audit/questions.md`", "`docs/audit/2026-10-29-audit-of-audits/`", handoff "removed; `git show 444631e6:...`" | none | readers | none

## owner-map.md (OM)

I6-149 | who owns an owner map's shape | OM:3 | "This page is the one home of the map's shape and of the two stops that go with it." | claims: "the one home" | scouts; planners; reviewers | none
I6-150 | which plans need an owner map | OM:13 | "Every plan that changes what Hydra decides or shows ... screens, the engine, scoring, the Path Builder, stored records and tools that report on them. A plan that only moves files, edits docs or changes hooks needs none." | none | main session | main session decides and says so in the planner brief
I6-151 | who writes it | OM:17 | "A scout writes it before the planner starts, in its own run." ; main session "briefs the scout with the user's requirement, in the user's words" | none | scout; main session | main session supplies the requirement
I6-152 | map shape | OM:21-25 | "One table, under a heading that is exactly `## Owner map`, with four columns" (Question, Owner, Its own words, Status) | none | scout; planner; plan reviewer; docs test (not named here) | scout fills
I6-153 | Status words | OM:25 | "`found`; `split` when two places answer it; `carry out` when the engine works it out but keeps it nowhere readable; or `none`, with the search that found nothing" | none | scout | scout supplies
I6-154 | row id form | OM:27 | "Start each question with a short id, such as `O1:`." | none | scout | scout
I6-155 | question wording rule | OM:29 | "A question is in game words, never in the shape of a design." | none | scout | none
I6-156 | when a name is not an owner | OM:31 | "If the owner's comment says it answers something else, it is not the owner" ; example `SPSqueeze::needs_no_timing` (`src/core/model.h`) | none | scout | none
I6-157 | one owner per question | OM:33 | "A question can have only one owner. If two places answer it, the row names both and its Status is `split`." | none (restates agent rule 15) | scout; planner | none
I6-158 | stop on none | OM:37 | "A row with Status `none` stops the work." ; "the main session takes it to the user before any plan is written" | claims the stops live here | scout; main session | user asked
I6-159 | stop on contradiction | OM:39 | "A result that contradicts the user's words stops the work." ; reviewer signs off `BLOCKED` | claims here ("the one home" per preamble) | scout; planner; plan reviewer | user decides
I6-160 | what a planner may do with the map | OM:43 | "never builds a new value out of several owners. It never adds a condition to an owner, and never uses an owner for a question its own words do not claim." ; "stops and asks the main session for the row" | none | planner | planner asks main session
I6-161 | who checks the map | OM:47 | "The plan's reviewer opens each owner and reads its own words against the row's question." ; "reads the plan's steps with the map closed and lists every question they answer" | none | plan reviewer | none
I6-162 | origin of the page | OM:7 | "On 2026-11-14 three plans in a row ..." ; user quotes "so why do planners keep failing to find the rules we already have? ..." and "Yes, write them" | none | readers | none

## premise-map.md (PM)

I6-163 | who owns the premise map's shape | PM:5 | "This page is the one home of the map's shape, and of the shape of a live step's call list." | claims: "the one home" | planners; plan reviewers; docs test | none
I6-164 | the rule | PM:9 | "A fact either names its source or it is marked `unchecked`." ; "An unchecked premise never carries into a task or a live step as settled." | none | planner | none
I6-165 | table heading and columns | PM:13-17 | "exactly `## Premise map`" ; columns Premise, Kind, Source, Status | none | planner; docs test | planner writes
I6-166 | row id form | PM:19 | "a short id, such as `P1:`" | none | planner | planner
I6-167 | row rules | PM:23-27 | "A row has one Kind." ; "There is no half-checked status." ; Source "holds at least one backticked token" | none | planner; docs test | planner
I6-168 | source must be for the exact thing | PM:29 | "A source backs a row only when it works the fact out for the thing the row names." | none (same idea as DOR step 10; see I6-025) | planner; reviewer | none
I6-169 | Status form | PM:31 | "`checked` and a date, such as `checked 2026-10-31`" ; unchecked: "the id of a task ... (one with a `### <id>:` heading); `question N` ...; or the id of a live step" ; "under the plan's heading that starts `## Questions`" | none | planner; docs test | planner
I6-170 | deferred premise blocks | PM:35 | "`blocks RG1, RG2` or `blocks nothing`" ; "a line that starts `Waits for:`" ; "The main session does not dispatch such a task until the step's result folder exists." | none | planner; main session | planner
I6-171 | the six Kind words | PM:39-46 | "`code`, `data`, `chart`, `run`, `ruling`, `unchecked`" ; "exactly one of the six Kind words" | none | planner | planner
I6-172 | where run results live | PM:44 | `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\` | none | planner | none
I6-173 | which Clone Hero page labels count as a data source | PM:42 | "Label is \"Proven in the game's code\" or \"Proven in a live run\"" ; "\"Hydra assumes it, never checked\" or \"Unknown\" is not a source" ; "`docs/clone-hero/README.md` says what each label means" | none (label words owned by docs/clone-hero/README.md) | planner | none
I6-174 | what counts as a ruling source | PM:45 | "a line under `## Confirmed` in a file under `docs/agents/rulings/`, in a session card's quoted words under `docs/sessions/`, or in the user's message as the main session quotes it in the reviewer's brief and confirms" | none (third copy of the same list; see I6-081, DOR:45) | planner; reviewer | none
I6-175 | what is not a source | PM:52 | "a handoff, another plan, a session card, a memory note, or a Clone Hero block with no proof label" | none | planner | none
I6-176 | live step heading shape | PM:56 | "a heading that starts `### LS` followed by a `-` or a digit, such as `### LS-D0` or `### LS2`, inside a section whose heading starts `## The live steps`" ; struck: "`### ~~LS`" | none | planner; docs test | planner
I6-177 | the call list line | PM:58-61 | "one line starts with `Calls:`" ; markers "(seen: LS-D0 run 1)" and "(never tried)" | none | planner; docs test | planner supplies calls
I6-178 | what "seen" means | PM:64-66 | "the same player, the same route and the same chart" ; two looser readings accepted "on 2026-10-31 (\"Take all five\", question 10 of `docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md`, removed; `git show 6804353f:...`)" | none | planner; reviewer | none
I6-179 | max never-tried calls | PM:68 | "A step holds at most one `(never tried)` call." | none | planner; docs test | planner
I6-180 | who checks the map | PM:74-76 | "The plan reviewer checks the facts three times." ; "The shape is also held by the docs test, `tests/test_docs_match_code.cpp`." | claims docs test holds shape | plan reviewer | none
I6-181 | go request | PM:78 | "Before a live run, the main session shows the user the step's call list." ; "`docs/agents/clone-hero-mcp.md`" | none | main session | user asked
I6-182 | plan doing a settle-read before showing | PM:33 | "Before a plan is shown, the planner tries each `unchecked` row against the code and the Clone Hero pages." | none | planner | none

## this-machine.md (TM)

I6-183 | which Python runs scripts | TM:7 | "Run Python as `py`." ; "`C:\Users\Patrick\AppData\Local\Python\pythoncore-3.14-64\python.exe`" ; "Python 3.14.5" ; "SQLite 3.50.4" | none (this path also in DOR:124; "`publish.ps1` calls that same file by its full path") | every agent | none
I6-184 | what bare python is | TM:9 | "`C:\Python314\python.exe`, which is Python 3.14.7" ; Store alias "`C:\Users\Patrick\AppData\Local\Microsoft\WindowsApps\python.exe`" ; "Use `py`" | none | agents | none
I6-185 | gh version, path, account | TM:13 | "`gh` is version 2.97.0, at `C:\Program Files\GitHub CLI\gh.exe`" ; account "`patseguin1-commits`" ; scopes "gist, read:org, repo and workflow" | none | agents | none
I6-186 | GitHub repo | TM:15 | "`splax77/hydra-deluxe`" | none (also IT; CLAUDE.md) | agents | none
I6-187 | text to GitHub | TM:15 | "Text never goes to GitHub through a hand-typed `gh` command." | none (duplicate of I6-072) | agents | none
I6-188 | release naming | TM:17 | "A release's tag and title are both `v` plus the version, such as `v3.0.0`, and it carries two files: the installer and its `.sig` signature." | none | release writer | none
I6-189 | where the version number is set | TM:17 | "The version number is set in one place, the project line of `CMakeLists.txt`, which reads VERSION 3.1.2 today." | claims: "set in one place" | release writer | none
I6-190 | where the hooks live | TM:19 | "`C:\Users\Patrick\.claude\gates`, outside this checkout, inside the git repo at `C:\Users\Patrick\.claude`" | none (same path in DOR, preamble) | agents | none
I6-191 | Ghidra and Java | TM:23 | "Ghidra 12.1.3 is at `C:\Users\Patrick\Downloads\ghidra_12.1.3_PUBLIC_20260817\ghidra_12.1.3_PUBLIC`"; headless runner `...\support\analyzeHeadless.bat`; "Eclipse Temurin JDK 25 (25.0.4.1), at `C:\Program Files\Eclipse Adoptium\jdk-25.0.4.101-hotspot\bin\java.exe`" ; "No GHIDRA_INSTALL_DIR variable is set" | none | reverse-engineering agents | none
I6-192 | REA server | TM:25 | "`npx -y rea-agents@6.5.0 mcp`" ; registered "in `C:\Users\Patrick\.claude.json`" ; skill "`C:\Users\Patrick\.claude\skills\reverse-engineer-anything`" | none | main session | none
I6-193 | where the Clone Hero decompile is | TM:27 | "`docs/clone-hero/code/README.md` names its path" | claims that page owns the path | agents | none
I6-194 | build number of Clone Hero decompile | TM:27 | "Clone Hero build 6142" | none | agents | none
I6-195 | Visual Studio installs | TM:35-39 | Build Tools 2026 "18.9.12120.119" `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools`; Community 2026 "18.9.12112.369" `C:\Program Files\Microsoft Visual Studio\18\Community`; Build Tools 2022 "17.14.37614.0" `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools` | none | main session | none
I6-196 | how cmake is found | TM:41 | "`Find-CMake` in `tools\find_cmake.ps1`" ; "`cmake` is not on PowerShell's PATH" | none (owner by name: Find-CMake) | main session builds | none
I6-197 | cmake refusal fix | TM:43 | `-DCMAKE_GENERATOR_INSTANCE="<install folder>,version=<its version>"` ; "The lasting fix is a repair in the Visual Studio Installer, which is the user's to run." | none | main session | user runs the repair
I6-198 | shell elevation | TM:47-49 | "`Medium Mandatory Level`" ; "`wpr -start` was refused ... with error `0xC5585011`" ; "The user has ruled that no elevated window is opened." | none | agents | none
I6-199 | which PowerShell runs scripts | TM:51 | "`pwsh`, which is PowerShell 7.6.9 at `C:\Program Files\PowerShell\7\pwsh.exe`" ; "RemoteSigned" ; "Windows PowerShell (`powershell.exe`) has no policy set here" | none | agents | none
I6-200 | Clone Hero log path | TM:53 | "`C:\Users\Patrick\AppData\locallow\srylain Inc_\Clone Hero\`, which holds `Player.log` and `Player-prev.log`" | none | agents | none
I6-201 | how to find cdb | TM:57-61 | "`(Get-AppxPackage -Name Microsoft.WinDbg).InstallLocation`" joined to `amd64` and `cdb.exe` ; "never from a typed path" | none | agents | none
I6-202 | how to attach cdb | TM:63 | "`-pv`" ; "`-y`, not `.sympath` inside `-c`" ; `cdb -pv -y <build-cpp\Release> -p <pid> -c "~*kc 40; qd"` ; "keep samples short" ; "at most once per run" (TM:67) | none | agents | none
I6-203 | stall harness path and load | TM:67 | "`tools/ch_probe/experiments/stall_harness.py`" ; "It reads only this checkout's `testdata` and `.git` as load, never the song library, and it never starts Clone Hero." | none | agents; the harness | none
I6-204 | which Opus model agents run | TM:71 | "ANTHROPIC_DEFAULT_OPUS_MODEL entry of its `env` block to `claude-opus-5-5`" ; "`C:\Users\Patrick\.claude\settings.json`" ; "The user picks this model by hand." | none (the Opus name also appears in the preamble's commit trailer as "Claude Opus 5.5") | main session | user picks the model
I6-205 | where hydra.db lives | TM:77 | "`hydra.db`, next to the program that opened it (`db_path()` in `src/app/config.cpp`)" ; dev "`C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra.db`" ; installed "`C:\Program Files\Hydra\hydra.db`" | claims `db_path()` | agents | none
I6-206 | read a copy of the database | TM:81 | "Copy the database to your scratch folder and read the copy." ; "copy `hydra.db-wal` and `hydra.db-shm` along with `hydra.db`" ; "WAL mode" | none | agents | none
I6-207 | how to read the database | TM:83 | "`py` and its `sqlite3` module" ; "`C:\Windows\System32\winsqlite3.dll`" ; "There is no `sqlite3` command-line tool here." | none | agents | none
I6-208 | tables and where columns are defined | TM:85 | "`charts`, `meta`, `profiles`, `records` and `record_paths`" ; "defined once, in `src/store/record_store.cpp`: ... `RecordStore::set_up_schema` ... `record_tables_sql()`" | claims: "defined once" | agents | none
I6-209 | the join | TM:85 | "a result row's `song_id` is the chart's `md5` (the hyhash ...; `docs/adr/0028-a-chart-id-is-clone-heros-song-id.md`)" | none (ADR 0028 named as the home) | agents | none
I6-210 | re-analyze one chart | TM:89 | `hydra_batch.exe --db <copy> --redo <folder holding one chart>` ; "`load_tool_settings()`" | none | agents | none
I6-211 | whole-library run guard | TM:91 | "a whole-library `--redo` on a copy of the real library is a full library run. It needs the user's `allow library run` first (CLAUDE.md rule 8)." ; "`charts_by_md5` index ... (D76)" ; "Try the chart corpus in `testdata/input` before you ask." | none (the phrase also in the preamble and AM:125) | agents; whole-library hook | user types `allow library run`
I6-212 | quote numbers the user can see | TM:95 | "before you quote a stored number, say which path it comes from (the optimal path, a path within the limit, or the best all-0 path) and whether the app displays it" ; "`build_path_buttons()` and `build_activations()` in `src/app/path_view.cpp`" | claims those functions build what the Paths tab shows | agents | agent says the path
I6-213 | machine-wins rule | TM:3 | "If a line here disagrees with what you see, the machine wins. Fix the line in the same commit as the work that found it." ; "checked on the machine on 2026-10-31" | none | agents | none

## rulings/README.md (RR)

I6-214 | where rulings live | RR:3 | "A ruling is filed here only after the user clicks \"File it\" for that exact line in a question box." | none | agents; the ref gate | user clicks
I6-215 | the eight topic files | RR:7-14 | "agents-and-workflow.md", "review-and-hooks.md", "ui-and-display.md", "code-and-engine.md", "clone-hero.md", "questions-and-audits.md", "docs-and-writing.md", "tests-and-runs.md" | claims it is the index | agents | none
I6-216 | what each heading means | RR:18-20 | "`## Confirmed` holds lines the user approved ... `## Unconfirmed` holds the lines sessions copied from memory notes on 2026-10-31" ; "No plan, review or question may cite one as the user's ruling." | none | agents; the ref gate | none
I6-217 | project word meaning | RR:22,34 | "A ruling that uses a project word does not change what the word means. `CONTEXT.md` decides that." | none (same as DOR:51; see I6-021) | agents | none
I6-218 | shape of a Confirmed line | RR:26-32 | two shapes: `- **2026-11-13** "change the display text to frontend and backend" (session 7d28ae9a)` and `- **2026-11-13** chose "Keep frontend/backend" to "Keep the words?" (session 7d28ae9a)` ; "One line holds one quote, copied letter for letter" ; session "is the first eight characters of its id, or the whole id" | none | ref gate (`ruling_rules.py`, named at RR:40,51) | session supplies the line
I6-219 | the filing steps | RR:38-43 | step 1 `ruling_rules.py --check <file>` (hooks repo's `gates` folder); step 2 question text "`File this ruling in docs/agents/rulings/<topic file>?`" with labels "`File it`" and "`Do not file`"; "One box may hold up to four lines"; step 3 add under `## Confirmed`; step 4 git gate refuses added lines whose shape is wrong, quote never typed, or no "File it" answer in a main session's transcript | none | main session; ref gate | user clicks File it
I6-220 | removal | RR:45 | "A ruling counts only once it is committed. Removing a line is always allowed." | none | ref gate | none
I6-221 | who can approve | RR:47 | "No agent can approve a line, because the gate reads only main-session transcripts." | none | ref gate | none
I6-222 | backfill route | RR:51 | "approved it on 2026-11-13 (\"Yes, no click\")" ; "`INSTALL_DATE` in `ruling_rules.py`, 2026-11-14" ; four conditions (words test, date before install day, typed before that day, same commit deletes an Unconfirmed line) ; "Moving or deleting the `## Unconfirmed` heading ... is refused the same way." | claims `INSTALL_DATE` owner is `ruling_rules.py` | ref gate | none
I6-223 | how an Unconfirmed line reads | RR:55-57 | date in bold, quote, "As the note records it:", bracketed session/note/rule ; "\"by date\"" ; "\"the note's only one\"" ; "\"not stated\"" | none | agents reading | none

## issue-tracker.md (IT)

I6-224 | where issues live | IT:3 | "Issues and specs for this repo live as GitHub issues. Use the `gh` CLI for all operations." | none | agents | none
I6-225 | posting path | IT:5 | "Text reaches GitHub only through `C:\Users\Patrick\.claude\gates\publish.ps1`" ; "the settings refuse the posting `gh` commands typed by hand" ; "Reading with `gh` is not restricted." | none (duplicate of I6-072) | agents; settings | none
I6-226 | create/comment forms | IT:9,12 | `publish.ps1 -Kind issue -Title "..." -BodyFile <file>` ; `publish.ps1 -Kind issue-comment -Target <number> -BodyFile <file>` | none | agents | agent supplies title and body file
I6-227 | list/read/labels/close | IT:10-14 | `gh issue view <number> --comments` ; `gh issue list --state open --json number,title,body,labels,comments --jq '...'` ; `gh issue edit <number> --add-label "..."` / `--remove-label "..."` ; `gh issue close <number> --comment "..."` | none | agents | none
I6-228 | PRs as a request surface | IT:20 | "**PRs as a request surface: no.**" ; "`/triage` reads this flag." | none | the `/triage` skill | none
I6-229 | external PR filter | IT:25 | "keep only `authorAssociation` of CONTRIBUTOR, FIRST_TIME_CONTRIBUTOR, or NONE (drop OWNER/MEMBER/COLLABORATOR)" | none | `/triage` skill | none
I6-230 | number space | IT:28 | "GitHub shares one number space across issues and PRs" | none | agents | none
I6-231 | "publish to the issue tracker" / "fetch the ticket" | IT:30-36 | "Create a GitHub issue." ; "Run `gh issue view <number> --comments`." | none | skills | none
I6-232 | wayfinder map shape | IT:40-47 | map "labelled `wayfinder:map`"; "Notes / Decisions-so-far / Fog body"; child "`Part of #<map>`"; labels "`wayfinder:<type>` (`research`/`prototype`/`grilling`/`task`)"; "`Blocked by: #<n>, #<n>`"; "`gh api` is denied" ; claim: "`gh issue edit <n> --add-assignee @me`" ; "append a context pointer (gist + link)" | none | `/wayfinder` skill | none
I6-233 | blocking and frontier rule | IT:45 | "A ticket is unblocked when every issue in that line is closed." ; "first in map order wins" | none | `/wayfinder` skill | none

## triage-labels.md (TL)

I6-234 | the five triage labels | TL:7-11 | "`needs-triage`", "`needs-info`", "`ready-for-agent`", "`ready-for-human`", "`wontfix`" (left column mattpocock/skills, right column "our tracker", same strings) | claims it maps roles to the repo's label strings | triage skills | none
I6-235 | meaning of each label | TL:7-11 | "Maintainer needs to evaluate this issue", "Waiting on reporter for more information", "Fully specified, ready for an AFK agent", "Requires human implementation", "Will not be actioned" | none | triage skills | none
I6-236 | how a skill names a role | TL:13 | "use the corresponding label string from this table" | none | triage skills | none
I6-237 | editing guidance | TL:15 | "Edit the right-hand column to match whatever vocabulary you actually use." | none | the user | user edits

## domain.md (DM)

I6-238 | what skills read before exploring | DM:7-9 | "`CONTEXT.md` at the repo root", "CONTEXT-MAP.md at the repo root if it exists", "`docs/adr/`" | none (CONTEXT.md named as the word home also in DOR:51) | skills | none
I6-239 | what to do if those files are missing | DM:11 | "proceed silently. Don't flag their absence; don't suggest creating them upfront." ; "The `/domain-modeling` skill ... creates them lazily" | none | skills | none
I6-240 | file layout | DM:15-39 | single-context tree and multi-context tree (`CONTEXT-MAP.md`, `src/<context>/CONTEXT.md`, `src/<context>/docs/adr/`) ; example ADR names "0001-event-sourced-orders.md" | none (generic template; Hydra is "Single-context" in CLAUDE.md) | skills | none
I6-241 | use glossary vocabulary | DM:43 | "use the term as defined in `CONTEXT.md`. Don't drift to synonyms the glossary explicitly avoids." | none (see I6-021) | skills | none
I6-242 | flag ADR conflicts | DM:49-51 | "surface it explicitly rather than silently overriding" ; form "_Contradicts ADR-0007 (event-sourced orders) — but worth reopening because…_" | none | skills | none

## doc-screenshots.md (DS)

I6-243 | command that retakes all pictures | DS:6 | `pwsh -NoProfile -File tools\doc_shots.ps1` | none | main session | none
I6-244 | where pictures go | DS:3,11 | "`docs/images/`" ; "saves the pictures straight into `docs/images/`" | none | doc_shots.ps1 | none
I6-245 | the shot list | DS:11,29 | "`tools/doc_shots/shots.txt`" ; "is the one list of the pictures" ; "Nothing else in `docs/images/` should exist" | claims: "the one list of the pictures" | doc_shots.ps1; writers | writers add steps and `screenshot` lines
I6-246 | data the runner reads | DS:11 | "the checked-in test charts in `testdata/input`, with a scratch settings file and an empty database of its own" ; "It never reads your song library or your settings" ; "[ui-testing.md](ui-testing.md) says what the scratch settings hold" | none | `hydra_uitest` | none
I6-247 | the shot checks | DS:13 | "`expect-text` and `rect` lines" ; "`!! failed at line N`" ; "Every wait in the list is capped by its `timeout` line." | none | shots.txt; `hydra_uitest` | none
I6-248 | the main-screen crop | DS:15 | "the whole frame minus the version bar at the top, because the version bar names the build" ; "a `rect` line directly before a `screenshot` line that has no ref" ; form "`screenshot <file> | <ref>`" | none (a second statement of the shots.txt comment: "The comment at the top of `tools/doc_shots/shots.txt` says the same") | doc_shots.ps1 | none
I6-249 | shrink step and limits | DS:17 | "`tools/doc_shots/shrink.py`" ; "Pillow's `optimize=True`" ; "one `size:` line per file and a `total:` line" ; "fails when a file or the whole folder is over the user's limits; the constants at the top of `tools/doc_shots/shrink.py` hold them" | claims constants in shrink.py own the limits | shrink.py | none
I6-250 | who runs it | DS:19 | "the main session runs it, never an agent (agent rule 14)" ; "`.\build_cpp.ps1 -Target hydra_uitest`" | none | main session | none
I6-251 | when to run | DS:23 | "before each release" ; "after any change to a label or a layout that one of the pictures shows" ; "open every changed picture ... no hover tooltip or version text crept in" ; "reviewed like any other change" | none | main session | none
I6-252 | settings reset in shots | DS:25 | "click `Reset to defaults`, then check that the settings button reads `Analysis settings: defaults`" | none (text of the app's button) | shots.txt | none

## public-doc-style.md (PD)

I6-253 | which pages a player reads | PD:3 | "`README.md`, `docs/README.md`, `docs/UserGuide.md`, `docs/differences-from-public-hydra.md`, and the GitHub release notes" | none (the first four also at DR:17) | writers; doc reviewers | none
I6-254 | internal docs follow another page | PD:3 | "Internal docs follow the user's \"How to explain things\" rules in `~/.claude/CLAUDE.md` instead" | none (see I6-083) | writers | none
I6-255 | accuracy first | PD:5 | "check every name and number against the code before you write it" | none | writers | none
I6-256 | rule sources | PD:7 | "two web searches on 2026-10-10" ; nine linked sources | none | readers | none
I6-257 | voice | PD:11-23 | "you", present tense, active voice ; "No \"I\" or \"we\"" ; app name "\"Hydra Deluxe\" (or \"Hydra\" in release notes from before 2.0.0 ...)" ; "Every number says where it comes from" ; "No hype" | none | writers | none
I6-258 | cut words list | PD:27 | "seamless, robust, powerful, comprehensive, effortless, crucial, pivotal, key (as an adjective), vibrant, showcase, leverage, empower, enhance, foster, delve, intricate, landscape, testament, streamline" | none | writers | none
I6-259 | "is" replaces | PD:29 | "\"serves as\", \"stands as\", \"acts as\" or \"represents\"" | none | writers | none
I6-260 | throat-clearing list | PD:31 | "\"It's worth noting\", \"Note that\", \"In this section\", \"Let's\", \"Simply\", \"just\" as filler, \"In order to\", \"To summarize\", \"In conclusion\"" | none (overlaps global CLAUDE.md throat-clearing list) | writers | none
I6-261 | one name per thing | PD:35 | "use the app's own label for every control" | none | writers | none
I6-262 | sentence shapes | PD:39-49 | no "not X, but Y"; no threes by reflex; no trailing "-ing" clause; no self-asked questions; "No em dashes in prose"; short sentences with varied length | none | writers | none
I6-263 | headings | PD:53 | "sentence case"; "Never give two headings in one page the same text. Keep a heading's text when other pages link to it" | none | writers | none
I6-264 | bullets and UI names | PD:55-59 | "Don't start bullets with a bold label and a colon" ; labels "go in `code` style, as the User Guide does now" | none | writers | none
I6-265 | other formatting | PD:61-63 | "No emoji." ; "Link text says where the link goes" ; "Put the condition before the instruction" | none | writers | none
I6-266 | page structure | PD:67-73 | README "stays short" ; User Guide "follows the screen" ; "Game mechanics are explained once. Link to the Hydra wiki or to the differences page" | none | writers | none
I6-267 | release notes rule | PD:77-79 | "A rewrite changes the voice and the shape, never the history." ; "keeps every fact the note states" ; "If a note looks wrong about its own release, leave the sentence and report it." ; "opens with one or two plain sentences" | none | writers | none
I6-268 | detector and fact rules | PD:83-87 | "Don't chase an AI-detector score." ; "Don't drop facts to make text shorter." ; "Don't add a claim you can't point to in the code, the release notes or a measured run." | none (the same "AI check unreliable" idea also at DR:83; see I6-084) | writers | none

## Cross-file notes (values stated in two or more of my own files; no judgement)

- Status line, every 5 minutes, status_append.ps1 call: I6-004 and I6-065.
- "Sonnet" reviewer, no helpers: I6-001, I6-060, I6-064; also IN:56 (I6-118).
- Key is a 40-character hash: I6-007, I6-057, I6-077, I6-093.
- publish.ps1 as the only way to GitHub: I6-072, I6-187, I6-225.
- Who can cite as the user's words (Confirmed / session card / brief quote; not Unconfirmed, memory note, handoff, plan "as ruled"): I6-018, I6-023, I6-081, I6-174, I6-216.
- CONTEXT.md decides project word meaning: I6-021, I6-217, I6-241.
- D107 one review, 30 minutes, 100 calls / 20 minutes: I6-038, I6-043, I6-085, I6-144.
- Sign-off lines and "no line counts as CLEAN": I6-047, I6-048, I6-087.
- Precheck command: I6-012, I6-114, I6-115; scan test filter: I6-040, I6-115.
- "allow library run" guard: I6-145, I6-211.
- "Not-fixed" findings go to the user (new numbers, shown text, stored records, behaviour choice): I6-041, I6-086, I6-109.
- Worktrees folders path pair: I6-056 (also preamble, not my file).
- Python path: I6-046, I6-183.
- Public pages list: I6-067, I6-253.
- Four exits and the four claim words: I6-122, I6-124, I6-145.

## Hook gaps noticed
- DOR:200 and DOR:202 say the hook records a reviewer verdict only when the reviewer is dispatched as `subagent_type: reviewer` from the Agent tool; a way past that exists for any other dispatch path (stated in the text itself). No steps given.
- DR:17 says text under a threshold passes the publish gate with no review; the threshold lives in a hook file I did not read.

## Files not reached
None. All 14 read in full. Rows written: 268 (I6-001 to I6-268).
