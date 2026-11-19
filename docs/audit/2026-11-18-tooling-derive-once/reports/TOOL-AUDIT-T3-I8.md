# TOOL-AUDIT-T3-I8 inventory (scout I8)

Task id TOOL-AUDIT-T3-I8. Session f6a384b5-9eed-4607-95cb-681ec635d569. Agent a54f72de02774c3b0.

## What I read and at which commit

- Memory notes: `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\`, MEMORY.md plus 53 note files, all read in full. Not in git (the `.claude` repo's `.gitignore` line 6 ignores `/*` there; `git check-ignore` confirmed). The `archive` folder was not read, as the dispatch said.
- Skills: `C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md` and `reverse-engineer-anything\SKILL.md` plus its 6 reference files (android-applications, connection-and-recovery, evidence-workflows, javascript-applications, native-and-artifacts, runtime-observation), all read in full. Tracked in the repo at `C:\Users\Patrick\.claude`, HEAD `38f9649fe5bdcab5035ebbde9a65bd1fd92f5ef3`. I did not read `skills\synced\` (not one of the two skills named).
- `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\commands\handoff.md`: repo HEAD `af9dfac4e246df6b77c4b906f5c7054f47beaa82`; last commit touching the file `fdd23c64ff7a1f052ed80d9e5973693dfc32426a`.
- Preamble read: `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md` (main checkout, as the brief says).

Files not reached: none, apart from `memory\archive\` and `skills\synced\` (out of scope).

Row format: id | question | file:line | value quoted | owner claim | who reads or acts | who is asked to supply. "none" means the text makes no owner claim. Memory notes are not the user's words; where a row quotes the user, the quote is the note's own report of it. Notes reached on the file system: MEMORY.md is the index loaded into every main session; the note files are read when a session opens one.

Short file names: `M:` = a memory note by its file name (all in the folder above). Lines are those of the Read tool.

## A. MEMORY.md (index)

- I8-001 | What do the index's "RULE" and "TOP RULE" labels mean | M:MEMORY.md:1 | "A RULE label means a session wrote the line down as binding on sessions; it is never cited as the user's ruling." | claims where the user's words live: "The user's words are the lines under `## Confirmed` in Hydra's `docs/agents/rulings/`." | main session and agents that read the index | none asked
- I8-002 | Where the user's rulings are kept | M:MEMORY.md:1 and :49 | "`docs/agents/rulings/` (index `README.md`; only `## Confirmed` lines are the user's words)" | none | main session | none
- I8-003 | Where this machine's tools and paths live | M:MEMORY.md:49 | "this machine's tools and paths in `docs/agents/this-machine.md`" | none | main session | none
- I8-004 | Which GUI check a change needs | M:MEMORY.md:49 | "which GUI check a change needs at the top of `docs/agents/ui-testing.md`" | none | main session | none
- I8-005 | Where finished memories live | M:MEMORY.md:48 | "Finished and superseded memories live in archive/ (not indexed); open one only when a task points there." | none | main session | none
- I8-006 | Index entry lines (47 note links with one-line rules, restating each note's rule) | M:MEMORY.md:2-47, :50-56 | each line restates the note's rule, e.g. :25 "no whole-library test without the user's yes in chat; use testdata/input." (same rule as I8-141); :44 "floor 5 witnessed / 20 usual / 10 real." (see I8-107); :36 "~3,000 fps h264 / 1,900 AV1" (see I8-190). The index line for each note is a second statement of that note's rule. Rows for each note's own rules are below; I note the index twin only where the numbers or words differ or add something. | none | main session | none
- I8-007 | Index twin differences: index :20 says merge-tests rule is "grep tests for changed files; whole suite only for shared code" while the note (merge-tests-touch-changed-files.md:15) says to run what `hydra_precheck.exe ... --tests` prints and "Never type the list by hand" | M:MEMORY.md:20 vs M:merge-tests-touch-changed-files.md:15 | see both quoted | none | main session | none. Related rows I8-129, I8-130.
- I8-008 | Index :39 gives fan-out numbers "~8 jobs per agent, 3-4 agents per round" | M:MEMORY.md:39 | same figures as I8-111 and I8-112 | none | main session | none
- I8-009 | Index :55 one-build-per-wave entry exists in the index but the note is the newest unfiled-by-date file; index :55 says "say the build count before launching work" | M:MEMORY.md:55 | "batch a wave's reviewed C++ branches into one build; say the build count before launching work." | none | main session | none. See I8-142 to I8-144.

## B. Memory notes, one group per note

### approved-work-is-not-reasked.md
- I8-010 | When may a session re-ask about work the user already approved | :19 | "Ask only when a finding makes the approved work wrong or impossible, not merely less often visible." Also :19 "report the new numbers in a line and carry on with the approved tasks." | none | main session | none
- I8-011 | What is a chosen mockup's values | :15 | "A chosen mock's values are the design; today's look is never an option beside it." | none | main session | none
- I8-012 | Figures from the cited cases: counted startup bar warm read "about 130 ms" (under "the 0.15 s screen delay"), T7 "about 8 ms", "8 threads", floor tint "0.45" vs "0.2" | :11, :13, :15 | those numbers | none | main session (as examples) | none

### ask-only-game-or-permission-questions.md
- I8-013 | Which questions may be put to the user | :11 | "Put to the user only questions about what Clone Hero or Hydra does (a split, a rule choice) or a permission (library read, a risky run, window focus). Decide documentation policy, label standards, block wording and tool fallbacks myself" | none | main session, planners | none (the user is the asker's target)
- I8-014 | What decides a record counts | :13 | "The README already said a video counts only by its public YouTube id." (the README is `docs/clone-hero/README.md`) | none | main session | none

### ch-disco-noflip-rule.md
- I8-015 | Is CH ignoring `drums0dnoflip` a bug and what does Hydra do | :11, :15 | "The user confirmed on 2026-09-27 ... 'this is a bug in clone hero, not an intentional feature. it will be fixed in a future update.' Hydra's reading (flip off) is the intended one." and "never change Hydra's parser to copy this." | none (cites "session card 1b0a7cd8 line 18") | main session, planners | none
- I8-016 | How to price a run that beats optimal on an affected chart | :15 | "price the run against a scratch copy with the marker renamed (`drums0dNOFLIP`) and say it is priced under the bugged game rule." | none | main session | none
- I8-017 | Splax video layout numbers | :15 | "score=596,298,124,34 disc=708,660,56,56 at 1080p" and "isn't in layouts.json" (layouts.json is in the video-tools repo) | none | main session. See I8-087, I8-088 for layouts.json owner statements.
- I8-018 | Where the facts live | :17 | "Facts moved to docs/clone-hero/02-note-list.md (NL-17 to NL-23)." | claims moved home | main session | none

### ch-runs-must-not-need-focus.md
- I8-019 | May a CH live run need the window focus or the keyboard | :11 | "Clone Hero live runs ... must run autonomously ... Input goes in from inside the game (Frida method calls / field sets), not through Win32 SendInput." | none | main session | none
- I8-020 | Quoted user words | :13 | "yes we need to do autonomous runs. that waws the whole point of this" (2026-10-30) | none | main session | none
- I8-021 | When may a key-driven run start; what unlocks keys | :17 | "Never start a key-driven (SendInput) run while the user may be using the machine; ask first." and "The key route is locked until the user types 'allow key run'." | none | main session; the `ch` MCP hook (named in the ch server text "a hook refuses them unless the user has typed allow key run") | the user is asked to type "allow key run"
- I8-022 | Is brief focus a failure | :17 | "Brief focus is not a failure: ... the focus count is information only." | none | main session | none
- I8-023 | Traps: show window, Rewired key | :19 | "Un-minimizing the game window with SW_SHOWNOACTIVATE still stole focus, so never show the window." and "Rewired saves bindings to HKCU\Software\srylain Inc.\Clone Hero ... the server backs that key up and restores it." | none | main session | none
- I8-024 | Where the kit is described | :17 | "`docs/agents/clone-hero-mcp.md` describes all of it." | claims it describes all | main session | none

### check-an-edge-case-can-happen.md
- I8-025 | When may an edge case reach the user as a question | :15 | "for every edge case in a plan or question, find the rule in the engine or the Clone Hero pages that lets a real path reach it, and a real chart that shows it. If none, drop it from the questions" | none | planners, reviewers (via briefs: "Planner and reviewer briefs say this too") | none
- I8-026 | SP activation facts cited | :11 | "activation needs 2 bars and SP running out leaves 0 or 1 (`src/search/engine.cpp` `branch_activate` and `create_deactivated_path`)" | none | main session | none. See I8-196 (skill) for the code owner statements elsewhere.

### check-long-captures-early.md
- I8-027 | When to check a long capture's first output | :11, :15 | "read its first output after one interval and confirm it holds real data" and title line 3 "within a minute or two" | none | main session | none
- I8-028 | Preferred capture method | :15 | "Prefer non-invasive capture (ETW/WPR, read-only memory reads) over hooking hot functions in a live app." | none | main session | none

### check-the-screen-against-the-data.md
- I8-029 | What to do when the user says a display is wrong | :11, :15 | "compare the drawn thing with the data it should show before answering" and "name the exact element that disagrees ... Only then say whether it is a bug." | none | main session | none

### comments-explain-current-code.md
- I8-030 | What may a code comment hold | :11, :15 | "Code comments explain the current code ... no 'was 85 from 1.5.0, 60 since 2026-09' and no old values or version timelines." and "history goes in the plan, the decision record (D-numbers), ADRs or CONTEXT.md." | none (also stated in brief-preamble "If you write code": "A comment names where a rule lives; it never restates the rule") | main session, executors, reviewers, planners | none. Compare I8-031.
- I8-031 | The note's rule differs from the preamble's: note forbids history in comments; preamble forbids restating the rule in comments and allows "why a rule exists" | M:comments-explain-current-code.md:11 vs brief-preamble.md "If you write code" | both quoted above | none | executors | none. Noting the two, no judgment.

### counts-on-screen-must-agree.md
- I8-032 | What to do when two counts of one thing disagree | :11, :16 | "that is a derive-once bug to fix, not a design detail to explain." and "ask which single count both places share, recommend one, then fix it." | none | main session, planners | none
- I8-033 | Never recommend keeping a split | :22, :23 | "Never recommend keeping one side 'unchanged' or 'as today'." and "Never invent a purpose for a split. Say where it came from (commit, author) and that nobody chose it, then ask 'one rule for both?'" | none | main session, planners | the user is asked "one rule for both?"
- I8-034 | Before a display question, show what Hydra already writes | :20 | "Before asking a display question, show what Hydra already writes for the same fact, quoting the code's owners." | none | main session | none
- I8-035 | Sibling paths | :24 | "When a ruling names one path (the Scan library button), check the sibling paths (`hydra_batch`, the set-aside rescan) and ask about them too." | none | main session | none
- I8-036 | "Out of scope" lines | :25 | "A plan's 'Out of scope (for the user to confirm)' line needs a recorded yes, or it is asked again." | none | main session, planners | the user is asked to confirm
- I8-037 | Unit shown to the user | :17 | "A unit the user never sees defined ('records', 'distinct analyses') is itself the bug." | none | main session | none
- I8-038 | Scope of the rule | :18, :19 | "The rule covers anything Claude or an agent reads (tool replies, logs), not only the GUI. One owner per numbering." and "A page that covers part of the library says so" | claims "One owner per numbering" | main session | none

### derive-display-from-engine-truth.md
- I8-039 | How many places derive a fact | :11 | "Every function or rule is derived exactly once in the project, and every other module calls that owner. The engine is the source of truth." | states the derive-once rule (agent rule 15 is the repo's owner statement) | main session, executors | none
- I8-040 | Where a backing row for "only once" lives | :20 | "Back the promise with a row in the owner's area file under `tests/single_owner/rules/` (ADR 0029) so a second copy fails the single-owner scan." | claims the scan owns it | main session, planners | none. Related repo rule list (repo-scans-read-main-checkout.md I8-134).
- I8-041 | The engine names for SP answers | :21 | "grep `src/core` and `src/search` for the engine's answer (`Chord::activation_note`, `kSpActivationBars`, `sp_ready_ms`, the squeeze rating, `app::song_length_ms`)" | names owners | planners | none
- I8-042 | Where a display's SP end comes from | :15 | "`Activation::deact_tick`, stamped by the engine" | claims it as the fix and the owner | main session | none
- I8-043 | Where squeeze events are drawn on a chart-time highway | :23 | "A squeeze is where the SP end and the squeezed chord meet in play, and the engine names that chord (`sqout_tick`, a SqIn step's `tick`) ... every SP event tied to a squeeze is drawn at that chord." | states the rule | main session, planners | none. Same subject as I8-166 (skill, SqOut measurement).
- I8-044 | Boundary table before dispatch | :24 | "When a plan splits work across a boundary ... write a table of each question and the side that owns it before dispatch. Brief each task with its side. A stand-in owns only transport." | none | planners, main session | none
- I8-045 | Verification mismatch | :25, :26 | "Treat a verification that needs an unexplained variant or 'close enough' as a live bug." and "Prefer one real-world observation (a gameplay video) over another internal consistency check." | none | main session | none
- I8-046 | Display plans | :22 | "A plan for a display never offers 'the plan's reading' of a gap, an edge or an exception ... Each value on screen maps to one stored engine field, named in a table" | none | planners | none

### dmleaderboards-comparison.md
- I8-047 | URL of the leaderboard API | :14 | "`https://dmbot-kb5j.onrender.com/api`" | none | main session | none
- I8-048 | Routes and fields | :14-17, :43 | "`/api/all-users`", "`/api/user/{discordId}/scores`", "`identifier` (32-char MD5 chart hash)", other routes "`/api/leaderboard/{song}`", "`/api/user/{id}/stats`", "`/api/songs/{md5}`", "`/api/users/compare` (POST)" | none | main session | none
- I8-049 | Join key between leaderboard and Hydra | :22-24 | "leaderboard `identifier` is Clone Hero's song id == Hydra's `hyhash` (src/app/dm_report.h; ADR 0028 for .sng/.srb ...)" | none | main session | none
- I8-050 | Latency and fetch rules | :19-20 | "warm ~650ms, cold 30s+; Cloudflare rejects empty User-Agent. -> fetch on a background job with a real UA + long timeout." | none | main session | none
- I8-051 | Are above-optimal scores bugs | :26-29 | "Above-optimal scores are NORMAL, not a bug ... label them 'above optimal'; never flag as impossible/error." | none | main session | none
- I8-052 | Upload behaviour of the backend | :31-45 | "keeps one drums row per chart per user and replaces it only on a higher score, or on an equal score with a higher play count"; "`play_count` is Clone Hero's own count"; "`posted` = the last upload that replaced the row" | none | main session | none. Cites docs/clone-hero/07-internals.md IN-55.
- I8-053 | Which speeds Hydra compares | :47-51 | "Hydra still compares only 100% scores, because its optimal is for normal speed." and "dmleaderboards ranks scores at all speeds on one board per chart" (user, 2026-10-10, with a screenshot) | none | main session | none

### drum-lanes-d112-2026-10.md
- I8-054 | Where drum lane pads are set | :11 | "Pads come from `light_drum_lanes` in src/parse/song.cpp, called once in the loader tail `finish_loaded_song`, after Note Shuffle." | none | main session | none
- I8-055 | Lane lighting under Note Shuffle | :13 | D254: "with Note Shuffle on, a lane lights every pad its notes land on, on purpose. The user: 'clone hero only draws the blue. this will be an intentional deviation from the game. it looks better with both.'" | cites "docs/agents/rulings/ui-and-display.md and NL-57" | main session | none
- I8-056 | Lane notes starting late | :13 | "A lane whose notes start late still counts, by the user's choice; the game's cut-off is NL-58." | none | main session | none
- I8-057 | Testdata count | :17 | "In testdata, 4 of 50 lanes differ between the Onyx and CH pad rules" | none | main session | none
- I8-058 | Hit routine slot | :17 | "the hit routine at vtable slot +0x1E8 ... (page 05 reads that slot as the plain hit handler)" | none | main session | none

### early-fill-orange-85-d250.md
- I8-059 | Early-fill orange reach | :11 | "`kHitWindowMs` (85 ms, D270 call 8), read by `fill_refuses`" in src/core/model.h | names the owner | main session | none
- I8-060 | Early-fill white slack | :11 | "`kEarlyFillSlackMs` (60 ms), read by `is_e0`" in src/core/model.h | names the owner | main session | none
- I8-061 | What "orange" means | :17 | "'orange' means a required early fill (positive difficulty), not the Extreme tier." | none | main session | none
- I8-062 | Library run result | :13 | "73 best paths ... 25 scored higher, each with a required early fill between 60.1 and 81.9 ms" and the user kept them: "Keep them. The extra E fills in the notation are fine." (D250 item 5) | none | main session | none. Description (line 3) says "D250 (2026-10-15)".

### engine-tie-merge-prune.md
- I8-063 | Max tied paths kept | :14-15 | "`Rules::max_tied_paths` (src/core/rules.h, default 4, settable in hydra_rules.ini)" | none (names the engine as owner: "The mechanism is in src/search/engine.cpp `reduce_iteration_paths` / `reduce_group`") | main session | none
- I8-064 | How to find a specific path | :22-26 | "never try to 'find' a specific path by widening the band. Pin it: `hydra_replay target` (`search_target` in src/search/pather.h). `--path <json> [--index N]` or `--acts "actTick:deactTick[:sqoutMs],..."`" | none | main session | none. Same rule as I8-086, I8-179 (skill).

### every-brief-carries-status-rule.md
- I8-065 | Must every brief carry a status-line instruction | :11, :15 | "'i don't care if an agent is read only, no status updates is unacceptable.'" and "every Agent prompt, whatever its model or type (Explore, Plan, worker, executor), tells the agent to write a status line every 5 minutes with the helper `C:\Users\Patrick\.claude\gates\status_append.ps1`" | claims the preamble owns the form: "the 'Status lines' section of the Hydra repo's `docs/agents/brief-preamble.md` owns the exact form" | main session when writing briefs; every agent | the brief writer (main session) is asked to include it
- I8-066 | Status-line interval | :15 | "every 5 minutes" | see brief-preamble | every agent | every agent. Same value as MEMORY.md:45 "every 5 minutes" (I8-067) and agent rule 7.
- I8-067 | Does a gate check briefs for the sentence | :15 | "No gate checks a brief for this sentence today, so the brief has to carry it." (this contradicts MEMORY.md? no; but description line 3 says "the dispatch gate now denies briefs that lack it" while line 15 says "No gate checks a brief for this sentence today": both in this note) | none | main session | none. Both quotes are in one file, I note them side by side.

### every-number-says-where-it-comes-from.md
- I8-068 | Every number needs its source | :11-14, :22-25 | "Every number in a doc, artifact or reply says where it comes from the first time it appears" and "Hex constants get their decimal value and the bits they stand for." | none | main session | none

### fc-video-workflow.md
- I8-069 | Where the FC video tool lives | :11 | "`C:\Users\Patrick\Downloads\Hydra\video-tools\` (its own git repo: ... `fcvideo.py`, `check_refs.py`, `refs.json`, `layouts.json` and its README) plus `hydra_replay.exe`" | claims the README owns recipes: "The README is the recipe book: every flag, the eleven-step checklist for a new video, layouts" | main session | none
- I8-070 | Regression check | :13 | "`py check_refs.py` ... re-runs the nine known videos ... Run it after any edit to the tool. Never change an expected number to make a case pass." | none | main session | none. Same instruction as I8-189 (skill: "Run `py check_refs.py` after any edit to the tool").
- I8-071 | The two checklist rules | :15 | "report the headline the moment engine and evidence agree, and never rewrite the video. Both are now README checklist items 0 and 5." | claims they live in the README | main session | none
- I8-072 | Review gate for tool work | :17 | "tool work ran as one Opus implementer plus one Sonnet reviewer ... Keep the reviewer gate" | none | main session | none
- I8-073 | Issue record | :15 | "GitHub issue splax77/hydra-deluxe#1 records the four" tool gaps | none | main session | none

### git-traps-in-shared-checkout.md
- I8-074 | Where executor worktrees are cut | :16 | "run `git worktree add .claude\worktrees\<name> -b claude/<name> main` yourself, leave `isolation` out, name the worktree in the brief, and have the executor check its base commit first." | none | main session | main session, executors
- I8-075 | Reviewer isolation | :17 | "A reviewer never sets `isolation` either ... a hook refuses a reviewer dispatch that sets `isolation` (`docs/agents/derive-once-review.md`, step 4)." | cites a hook and a doc | main session | none
- I8-076 | Review key | :18 | "The key is the branch tip, `git rev-parse <branch>`. If a merge is refused, run `git merge --abort` at once and check `.git/MERGE_HEAD` is gone." | none | main session | none. Same "review key is the branch tip" statement as I8-100.
- I8-077 | Staging rule | :19 | "Stage each file by name. Never `git add -A` or `git add -u` in the shared checkout." | none | main session, every agent | none. Preamble says the same for `add -A` / `add .` (preamble "Committing").
- I8-078 | Before commit checks | :20 | "check `.git/MERGE_HEAD` and `git diff --cached --name-only`" | none | main session | none
- I8-079 | Trailers | :21 | "The `Task:`, `Agent:` and `Session:` trailers must sit in the message's last paragraph, or the ref gate refuses the commit. Pass `Co-Authored-By` as a `--trailer` too" | cites the preamble form | executors, reviewers | none
- I8-080 | Base staleness figure | :13 | "Four executors once started on a base 361 commits old." | none | main session | none

### hooks-never-guess.md
- I8-081 | May a hook fill in a value | :11, :15 | "'any hook that requires guessing is unacceptable.'" and "list every value a hook fills in itself (default, fallback, inferred from wording) and remove it: require the value in one exact form, refuse when it is missing, and record it at the point it is known." | none | planners, reviewers of hooks | none
- I8-082 | What the stop hook assumed | :13 | "the stop hook's document verdicts assumed every review starts at `main` ... 'from the merge base with main' ... stamped 362 unread pages CLEAN and timed out." | none | main session | none
- I8-083 | Relation to D107 | :17 | "removing guesses never reverses a user decision or adds a second review (D107)." | none | main session | none. Note file's modified time shows 2026-11-19, after today's date by the machine clock; no judgment.

### hydra-deluxe-rename.md
- I8-084 | Which names say "Hydra Deluxe" and which keep "Hydra" | :13 | "visible name only: window title, installer AppName, Start Menu shortcut, Add/Remove entry, and docs. `Hydra.exe`, `hydra.db`, `hydra_*.ini`, `C:\Program Files\Hydra`, `C:\Users\Patrick\Documents\Hydra`, the AppUserModelID `Hydra.Hydra` and the installer AppId stay the same" | none | main session | none
- I8-085 | Repo name and remote | :11 | "`splax77/hydra-deluxe` (current home ...)", branch "`main`", version "v2.0.0 (5be5956 ...)" | none | main session | none. Repo name `splax77/hydra-deluxe` also in the Hydra CLAUDE.md (outside my files).
- I8-086 | Installer file name | :18 | "`HydraDeluxe-<ver>-setup.exe`" (also I8-146) | none | main session | none
- I8-087 | Window title | :11 | "The window title is now the bare name 'Hydra Deluxe' with no version" | none | main session | none

### labels-are-not-reasons.md
- I8-088 | Labels as reasons | :11, :16-20 | "A label is not a check." "Briefs never set quotas ('at least 5 good examples'). Say that none is a valid answer." "Only the rule itself is a rule number (85); a number built from one (7285) is a raw result." | none | main session, planners | none
- I8-089 | "Rule number" value | :19 | "Only the rule itself is a rule number (85)" (see I8-059 for the 85 owner) | none | main session | none

### lean-agent-fanout.md
- I8-090 | Batch size of small jobs | :20 | "Batch small uniform jobs (merges, rewrites) about 8 per agent." | none | main session | none
- I8-091 | Agents per discovery round | :21 | "use 3-4 agents and pass each the previous round's 'what I searched' notes" | none | main session | none
- I8-092 | Parallel launch | :11, :16-17 | "Approved independent tasks run in parallel, each in its own worktree, launched in the same turn." "fork from X's join tip as soon as its code exists." | none | main session | none. Same instruction as agent rule 6 (repo).
- I8-093 | Serialise when | :17 | "Serialize only on a real dependency ..., a timing run, or a merge that needs a gate or a yes." | none | main session | none
- I8-094 | Comparison runs | :19 | "Comparison runs launch together, each with its own scratch DB and output folder. Only a run that measures time stays alone." | none | main session | none

### measure-small-explain-first.md
- I8-095 | Size of a timing run | :15 | "the smallest run that decides it: a search-narrowed few hundred charts, every output line timestamped by phase, fastest of three" | none | main session | none
- I8-096 | Bisect rule | :15 | "Bisect by merging one branch onto the exact baseline commit. Never build in the baseline worktree without first copying its exe out." | none | main session | none
- I8-097 | Explain before a run | :11, :15 | "say what changed since the last run and why this one will end differently" (quoted from a hook) | cites "A hook also flagged the fifth bench_run launch" | a hook (unnamed) and main session | none

### merge-tests-touch-changed-files.md
- I8-098 | Which tests a merge runs | :11, :15 | "A merge runs only the tests that read the changed files. The whole suite runs only when shared code changed." and "run what `build-cpp\Release\hydra_precheck.exe --repo <path> --tests <range>` prints on its `tests:` and `always:` lines. Never type the list by hand" | claims the rest is held by the repo: "`docs/agents/agent-rules.md`, rule 1, holds the rest" | main session | none
- I8-099 | Who runs them | :11 | "The main session runs them (agent rules 1 and 14)." | none | main session | none
- I8-100 | Cited miss | :15 | "a hand list missed test_preview_load_progress and CI went red twice" | none | main session | none

### mimalloc-link-order.md
- I8-101 | Who owns mimalloc linking | :11 | "`hydra_use_mimalloc` in CMakeLists.txt owns the linking (docs/development.md)." | claims "owns the linking" | main session | none
- I8-102 | How to check redirect | :15 | "`MIMALLOC_VERBOSE=1` and look for 'malloc is redirected'" and "`app::malloc_redirected()` (src/app/allocator.cpp)", tests "tests/test_library_jobs.cpp and tests/ui/uitest_engine.cpp check it" | names owners | main session | none
- I8-103 | dumpbin path | :15 | "`C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\dumpbin.exe`" | none | main session | none. A tool path; `docs/agents/this-machine.md` holds machine paths per MEMORY.md:49.

### min-path-delta-proof.md
- I8-104 | Minimum nonzero path-score delta | :4 (desc) and :21-23 | "floor 5 (witnessed), 20 when no window touches notes 1-29, real-chart min observed 10 (The Cars)" and "Floors: 5 unconditional; 10 if no plain cymbal at odd multiplier is coverable; 20 if no window touches notes 1-29; 50 for cymbal-free charts." | none; note flags: "its file and line pointers no longer hold, so re-check against src/search/engine.cpp" | main session | none
- I8-105 | SP-score term values | :21 | "w = v*d*m, v in {50,65}, d in {1,2}, m in {1..4}" | none | main session | none
- I8-106 | Display-only gain | :25 | "`MultSqueeze::gain()` (src/core/model.cpp) is display-only; its one consumer is multsqueeze_views in src/app/path_view.cpp." | names an owner and consumer | main session | none
- I8-107 | Corpus numbers | :29-31 | "18,610 charts, 13 with an activation covering a note before combo 30"; "Golden corpus: 49,719 gaps, all == 0 mod 20, min 20." | none | main session | none. Library size differs across notes (see I8-141: "18,811"; I8-175: "19,000-chart"; sp-end note etc.). Index MEMORY.md:44 says "floor 5 witnessed / 20 usual / 10 real".

### names-from-data-not-recall.md
- I8-108 | Where proper names come from | :15 | "Every proper name in a deliverable must trace to data in hand (files, DB, web lookup). If a name can't be sourced, label it unknown and say so" | none | main session, agents | none. Same rule as skill I8-188.
- I8-109 | Data fact | :17 | "Feeble Words (DIT track 10) is a full drum tacet" | none | main session | none

### new-screens-start-from-a-clickable-mockup.md
- I8-110 | When a mockup is needed | :11, :15 | "A new screen (or a new program's window) starts as a clickable mockup with real numbers from one chart. The user clicks through the main task in it ... and approves it before a plan is written." | none | main session, planners | the user is asked to click through the mock
- I8-111 | Options in a mockup | :17 | "A mockup shows the one recommended design, with no option switches ... one or two at most [questions], never as toggles" | none | main session | none
- I8-112 | One GUI test per screen plan | :15 | "Every plan for a screen includes one GUI test that does the whole main task by clicking on screen, not by calling the engine." | none | planners | none

### no-full-library-runs.md
- I8-113 | Full library runs | :11 | "Claude never runs a full library test unless it is truly necessary and the user has said yes in chat to that run." | cites its other homes: "Written into CLAUDE.md rule 8, `docs/agents/brief-preamble.md` (Tests) and `~/.claude/model-postures.md` (Fable). No hook enforces it yet." | every agent | the user is asked in chat
- I8-114 | What counts as a full library test | :11 | "`hydra_batch` with no arguments, a whole-library `tools/compare_db.py` check, a whole-library timing run" | none | every agent | none. The preamble names the guards differently: "`hydra_batch` given no chart folder refuses ... unless it also gets `--whole-library`" and a hook for `--whole-library`. (Note: this note says "No hook enforces it yet" at :15; the preamble says a hook refuses any shell command with `--whole-library`.)
- I8-115 | Test corpus size | :11 | "`testdata/input` (about 115 charts)" | none | every agent | none. Another corpus size appears in another note (measure-small: "326-chart run", questions-describe-the-real-rule:24 "`[N/97]`").
- I8-116 | How to ask | :17 | "the question names it a 'full library run' in those words, says how many runs and how long each takes (estimate), and the brief then stays inside exactly that: no time cap or run count the user didn't see, no second run." | none | main session | the user is told runs, time
- I8-117 | Real-window start counts as a library run | :19 | "`hydra_start('window')` on the real library starts a catch-up analysis, which is a library run; give it `library_dir` with a small folder." | none | main session | none
- I8-118 | Library size cited | :17 | "builds over all 18,811 charts" | none | main session | none

### one-build-per-wave.md (not in the first-written position; indexed at MEMORY.md:55)
- I8-119 | How many builds per wave | :11 | "review each branch, then put the reviewed ones on one combined branch, build it once and run all their tests. A branch that breaks the combined build comes back out and goes to a fresh executor. Never plan one build per branch." | none | main session, planners | none
- I8-120 | When to say the build count | :15 | "before launching work that will need builds, say how many builds it needs and get a yes. Python-only branches need no build." | none | main session | the user is asked yes
- I8-121 | Concurrent builds | :13 | quoted user: "3 builds at once? what? i didnt accept doing lots of builds." Agent rule 14 (repo) says "At most three builds run at once." This note's figure "3" is the user's reaction to a count, not a limit | none | main session | none. (Modified time shows 2026-11-19, later than today's date.)

### one-reviewer-rule-d107.md
- I8-122 | How many reviewers per change | :11 | "A change gets ONE fresh reviewer. It reviews code and docs, fixes every finding itself, commits, signs off, and the change merges." | claims D107 ("D107 item 5") | main session; reviewers; the SubagentStop hook "(gates\agent_stop.py)" | none
- I8-123 | Reviewer limit | :11, :15 | "the progress hook, gates\progress.py, gives every agent the wrap-up warning at 100 calls or 20 minutes" and "the reviewer cap is 150 calls like everyone else" | cites the hook and the preamble | every agent | none. Matches preamble (100 calls or 20 minutes warn; 150 calls or 30 minutes stop).
- I8-124 | What the Subagent stop hook records | :15 | "The SubagentStop hook (gates\agent_stop.py) records the verdict for the branch's new tip after the reviewer commits its own fixes, code and documents alike" | names the hook | the hook | none
- I8-125 | Review key | :15 | "the review key is the branch tip (`git rev-parse <branch>`), and the ref gate's refusal names any key main still needs." | none | main session | none. Same as I8-076.
- I8-126 | Cost figures | :13 | "six review agents, ~781k tokens and two refused merges" | none | main session | none
- I8-127 | Brief homes | :15 | "docs/agents/derive-once-review.md, doc-review.md (fix-round.md retired)" | none | main session | none

### one-time-scale-per-screen.md
- I8-128 | Time scales on a screen | :11, :15 | "A screen shows time on one scale. Never put a whole-song overview strip directly above a zoomed timeline across the same width" and "check any timeline mockup on a short song as well as a long one." | none | main session, planners | none

### open-the-source-before-offering-work.md
- I8-129 | Before offering costly work | :11, :15 | "open a handful of the source items that work would redo" and "read 3 to 5 of those items in their source files, count what they hold by script" | none | main session | none
- I8-130 | Blank summary column | :11 | "A blank column in a summary file ... means the summary lacks a value, not that the source is empty." | none | main session | none

### plans-reviewed-before-shown.md
- I8-131 | When may a plan be shown | :15 | "write the plan on its own branch, dispatch a fresh reviewer, and only then show it." | claims gate gap: "the gate only catches ExitPlanMode and Artifact, so a plan pasted into chat slips past it." | main session | none. The repo's CLAUDE.md doc-review section (outside my files) states the same review rule.

### question-the-design-before-patching.md
- I8-132 | Failed-run limit | :11 | "When the same goal fails twice and each fix works around the last failure, stop and put the design itself to the user, with the alternatives" ("2+ failed runs" in MEMORY.md:17) | none | main session | the user is asked
- I8-133 | Where to search before | :15 | "Search `docs/sessions/` and the plans first, and tell the user if the alternative was already tried or ruled out." | none | main session | none

### questions-describe-the-real-rule.md
- I8-134 | How to word a decision question | :16 | "Quote the real predicate and give one example from each end of its range" | none | main session, planners | none
- I8-135 | Where rule lists live | :11 | "Agent rules 11 to 13 hold the 'You will see' list and the visible-change rule. This memory holds what they lack." | claims agent rules 11 to 13 own it | main session | none
- I8-136 | Before-and-after batch proof | :24 | "run `hydra_batch --db <scratch> testdata\input` before and after. Strip the `[N/97]` prefix (completion order varies) and the final timing line, sort both, and diff. An empty diff means scores and paths are identical." | none | main session | none. "97" is a chart count.
- I8-137 | Which paths to hold the user to | :19, :20 | "hydra is expected to keep the same functionality it has today." (user's standing words, :13) | none | main session | none
- I8-138 | Narrowing | :20 | "a narrowing is its own question with both behaviours spelled out." | none | main session | the user

### relay-status-in-plain-english.md
- I8-139 | How to relay an agent's status to the user | :15 | "one sentence naming the task in plain words (which task, what it just did, what comes next)", "with a second sentence only when the evidence disagrees", "Skip lines from test fixtures (names starting `test_`). Never paste the raw line or a blockquote of it." | none | main session | none
- I8-140 | Evidence line | :15 | "check the line against the EVIDENCE line the hook puts under it (call counts, last five tools, new commits)" | names the hook output | main session | none. Preamble (outside my files) says the stall hook adds this evidence.
- I8-141 | User's words on status | :11 | "'the status updates should be brief but relayed in plain english.'" (2026-09-05) | none | main session | none

### render-checks-use-the-corpus.md
- I8-142 | What check a visual change needs | :11 | "A layout change is not done until someone has seen it in the real app, with the user's `hydra_ui.ini` and a copy of their database, before merge. Which check a change needs ... is in `docs/agents/ui-testing.md`; follow that page rather than this memory." | defers to `docs/agents/ui-testing.md` | main session | none
- I8-143 | Capture zoom | :18 | "Zoom it 2x and check gaps, column alignment and shared baselines before showing it" | none | main session | none
- I8-144 | Real DB copy use | :21 | "Never ask for a full library run when `testdata/input` answers the question. The user's real database copy is for the Library view only, never for starting an analysis." | none | main session | none
- I8-145 | Transition check | :19 | "Scroll to the middle of the list, click a song, compare before and after, close, and compare again." (see also I8-148, no-scroll rule; this step conflicts in wording with search-before-clicking-a-song.md:11 "Never scroll through the user's library ... without asking in chat first") | none | main session | none. Both rows noted.

### repo-scans-read-main-checkout.md
- I8-146 | Which tree the repo scans read | :11 | "`hydra_tests.exe "-sf=*single_owner*"` and `"-tc=docs match code*"` read `sourcetree::root()`, the source tree baked in at build time." | names `sourcetree::root()` | main session | none
- I8-147 | Pre-merge scan of a branch | :15 | "apply `tests/single_owner/rules/*.json` patterns to the branch's changed files before merging (a small script does it ...). Then run the real scans on main right after the merge as the final check." | none | main session | none
- I8-148 | Cited value | :11 | "its test used `0.003`, which the probe rule gives to `KEY_HOLD_S`" | none | main session | none

### screen-words-not-code-names.md
- I8-149 | How to describe a badge | :15 | "describe every badge with the words and figures on its screen ... quote only figures Hydra already shows. Don't put forward my own formula as a choice." | none | main session | the user is asked what a display should compare when no value is defined
- I8-150 | Cited badge | :11 | "`squeeze in -83.3 ms`", "`SPSqueeze::needs_no_timing`", "43.3 ms short, 280.4 ms spare" | none | main session | none

### search-before-clicking-a-song.md
- I8-151 | May a session scroll the library | :11 | "Never scroll through the user's library to reach a song or a spot without asking in chat first and saying how it will be done. That covers every method" | none | main session, every agent using the `hydra` MCP | the user is asked in chat
- I8-152 | How to open a song | :17 | "`hydra_open_song("<title>")`, or `hydra_type("**/##search", "<title>")`, check `hydra_text(contains="of 19")` shows the small filtered count, then `hydra_click("**/<title>")`." | none | main session | none. The `hydra` MCP server text says the same: "To open a song, use hydra_open_song with its exact title".
- I8-153 | Guard | :17 | "Since SEARCH-GUARD (merged 2026-10-24), Hydra's command channel refuses a library-row click while the search is empty (owner `item_ref` in tests/ui/uitest_commands.cpp); posted window messages bypass that guard, so the rule still binds there." | claims owner `item_ref` | main session | none. This also states a gap: posted window messages get past the guard.
- I8-154 | Wheel notches | :13 | "1,300 wheel notches" (a cited case) | none | main session | none

### search-web-before-finalizing-plans.md
- I8-155 | Prior-art search before a plan | :15 | "before ExitPlanMode, run WebSearch/WebFetch on the problem class ... add a short 'what others do' section with links." | none | planners, main session | none
- I8-156 | "Left alone" lists | :15 | "Do not leave a 'left alone' list of related defects; either fold them into the plan or ask." | none | planners | none

### sp-end-is-one-point-leeway-is-delgars.md
- I8-157 | Where Star Power ends | :11, :13 | "there should never be two different answers for where star power ends. ever. the 3ms rule is for whether a backend is included in the optimal score or uncounted." (user, 2026-10-29) and "the SP end is the bare end (offset 0 / the end tick) for every question about where SP ends" | claims the leeway owner: "`Rules::backend_leeway_ms`, 3 ms, `core::counted_without_squeeze`" | main session, planners | none
- I8-158 | Leeway value | :13 | "3 ms" (`Rules::backend_leeway_ms`) | names the owner | main session | none. Same figure, "3ms", in MEMORY.md:7 and the skill row I8-165? (the skill uses 85 not 3). Citation of docs/clone-hero/05-star-power.md (SP-42).
- I8-159 | Open conflict named | :17 | "`rate_activation` (src/core/squeeze_rating.cpp, called from src/app/path_view.cpp) still picks a plain backend row's scale side with `counted_without_squeeze`, the leeway (D117), while the squeezed-out row uses the bare end (`paid_by_sp_walk`). No ruling in docs/agents/rulings/ answers it yet; ask with both sides shown." | none | main session | the user is to be asked with both sides
- I8-160 | Method for any "two values" claim | :16 | "find what question each test answers, from the code's own comments and from where it came from" | none | main session | none

### sp-squeeze-mechanics.md
- I8-161 | How far Hydra looks for squeeze candidates | :13-15, :4 | "`kSqueezeWindowMs` = 500 ms of the deactivation node (src/core/model.h; `within_squeeze_window` is the one check, used by src/search/graph.cpp)" | claims "the one check" | main session | none. The skill states the same 500 ms and puts the constant in `src/search/graph.h` (I8-163): different file named.
- I8-162 | Squeeze tool path | :23-24 | "C:\Users\Patrick\Downloads\Hydra\squeeze-tools\ (squeeze_scout.py)" | none | main session | none. Same path as skill I8-164.
- I8-163 | Video intro offset | :18-19 | "song_ms = video_ms + 19995.46 (constant, zero drift)" | none | main session | none
- I8-164 | Where the game's SqIn/SqOut rule is | :11, :4 | "docs/clone-hero/05-star-power.md (SP-37)" | none | main session | none

### stitched-chart-auto-fills.md
- I8-165 | How to keep auto fills in a stitched chart | :11 | "write the `is_fill` chords from `hydra_replay score` as explicit fills ... FL-27, FL-28 and FL-30 on docs/clone-hero/04-fills.md." | none | main session | none
- I8-166 | Merge rules for stitching | :16-19 | "Turn on `ENABLE_CHART_DYNAMICS` once ...", "Reset disco flip at every song start.", "Drop the BEAT track.", "`ffmpeg -ss` before `-i` mis-seeks ogg by about 16 ms" | none | main session | none
- I8-167 | Path of the built chart | :13 | "`C:\Clone Hero\songs\Misc Downloads\Dream Theater Collection`" | none | main session | none

### stored-versions-one-rule.md
- I8-168 | Where stored-data stamps live | :13 | "every stamp on stored data lives in `src/store/stored_versions.h` as a `StampRule` ... `StampRule::is_current` is the only comparison." | claims "the only comparison" and "one rule" | main session | none
- I8-169 | When to bump the results stamp | :17 | "if analysis output changed since the last bump, bump `kResultsStamp` the way its comment in stored_versions.h says ... A change to how a saved figure is shown is not a bump." | none | main session at a release | none
- I8-170 | Release notes about re-analysis | :17 | "Before writing release notes about re-analysis, check stored_versions.h, never assume." | none | main session | none
- I8-171 | Related counts | :11 | "18,755" results (cited), ADR 0018 | none | main session | none

### time-the-real-code.md
- I8-172 | Where timings put to the user come from | :15 | "any timing put to the user as a choice is measured in Hydra's own code: `hydra_bench` (tools/bench.cpp) or a C++ test, built by the main session, on a copy of the data." | none | main session, planners, scouts ("Brief planners and scouts the same way") | none

### trace-pass-inputs-before-live-runs.md
- I8-173 | What to do before a live run | :11, :16-19 | "list every value the run's pass line depends on and trace each one to where it is set." "Mark each 'seen working on this exact target' or 'never tried'." | none | main session, scouts | the user is shown the list "with the go request"
- I8-174 | CI step timeout | :20 | "A CI step that has never run on a runner gets `timeout-minutes` of a few minutes on the step itself." (cited: "70 minutes under a 90-minute job cap") | none | main session | none
- I8-175 | Forced repro | :19 | "An intermittent bug with a known mechanism gets a forced repro first" | none | main session | none. Preamble (outside my files) says the same.
- I8-176 | Unproven step | :21 | "A step any review lists as unproven gets proven on a branch run before it reaches main." | none | main session | none

### use-source-facts-not-proxies.md
- I8-177 | Where song length comes from | :13 | "Song length now comes from the chart's own metadata: the song.ini or .sng song_length, or the .srb song_length_ms. The last Expert drum note is only a backup when the metadata is missing. Never open the audio just to get a length." | cites D75 (replaced D69); names `app::song_length_ms` in another note (I8-041) | main session | none

### verify-before-claiming-a-source-is-wrong.md
- I8-178 | Standard for a negative claim | :17-20, :29 | "a grep/search that doesn't find something only proves 'not in what I searched', never 'absent' or 'wrong'." and "State the search scope, not a global conclusion" | none | main session, agents | none. Same rule as the first line of this audit's brief and the preamble ("A search that finds nothing proves nothing").
- I8-179 | Before saying a tool is missing | :32-37 | "check `docs/agents/this-machine.md` and search for it (Get-Command, winget list, Get-AppxPackage), and say what was searched." | defers to `docs/agents/this-machine.md` | main session | none

### video-decode-ceilings.md
- I8-180 | Decode ceilings | :11 | "h264 ~3,000 fps CPU, AV1 (dav1d) ~1,900 fps; NVDEC ~2,300-2,700 fps for both. Today's fcvideo main pass runs 2,200-2,700 fps" | none | main session | none
- I8-181 | Which decoder the tool uses | :19 | "The tool never sends h264 to the card, even under `--decoder nvdec`." | none | main session | none
- I8-182 | Crop alignment | :19 | "align crop amounts to 16 and check the first frame against a CPU decode" | none | main session | none
- I8-183 | Paths and machine facts | :13, :17 | "`C:\Users\Patrick\Downloads\ffmpeg-cuda\ffmpeg`", "`C:\Users\Patrick\Downloads\ffmpeg-cuda\out\bin\ffmpeg.exe`", "16 cores, RTX 5070" | none | main session | none. This machine's tools and paths are claimed by `docs/agents/this-machine.md` per MEMORY.md:49.
- I8-184 | Evidence tables | :19 | "Evidence tables live in the video-tools README, Decoder section." | defers to README | main session | none

### windows-installer.md
- I8-185 | Installer files and name | :11 | "`installer\hydra.iss` and `installer\build_installer.ps1` make `HydraDeluxe-<ver>-setup.exe` (`OutputBaseFilename` in hydra.iss)." | none | main session | none
- I8-186 | AppId | :14 | "The AppId GUID never changes, or upgrades stop replacing the install" | claims "all stated in hydra.iss and build_installer.ps1" | main session | none
- I8-187 | Install folder permissions | :15 | "`[Dirs] Permissions: users-modify`" | claims user's design | main session | none
- I8-188 | Staging | :16 | "Staging always goes through `cmake --install` of the ship preset into a clean `stage` folder ... The script fails if a .db or settings .ini leaks into the stage." | none | main session | none
- I8-189 | Release command traps | :19-20 | "`gh release create --target` needs the full SHA" and "Bump the version by editing the whole `project(Hydra VERSION ... LANGUAGES ...)` line in CMakeLists.txt." | none | main session | none

### ytdl-folders-not-a-project-matter.md
- I8-190 | Do empty C:\ytdl folders matter | :11, :15 | "Don't raise `C:\ytdl\videos` or `C:\ytdl\tracks` being empty as a question, a handoff item or a claim row." and "Drop this one on sight." | none | main session | none

## C. Skill analyzing-sp-squeezes (`C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md`)

- I8-191 | How far Hydra looks for squeeze candidates | :10 | "Hydra only *considers* squeeze candidates within 500 ms of a deactivation node (`kSqueezeWindowMs`, src/search/graph.h)." | none | main session / whoever invokes the skill | none. Same figure as I8-161, which names `src/core/model.h` and `src/search/graph.cpp` instead of `src/search/graph.h`.
- I8-192 | Command for squeezes beyond the window | :15 | "`py C:\Users\Patrick\Downloads\Hydra\squeeze-tools\squeeze_scout.py "<chart name>"`" | none | main session | the invoker supplies a chart name
- I8-193 | Where the tool gets the path | :18 | "from `hydra_replay dump` (the repo's `build-cpp\Release\hydra_replay.exe`; `--replay` points elsewhere)" | none | tool | none
- I8-194 | The budget formula and W | :18, :24 | "budget `W*(1+r)` (W=85)", "Verdict: raw gap vs budget `85*(1+r)`; eff = gap×2/(1+r) on the nominal 170 scale." | none (the 85 is the same number as `kHitWindowMs` 85 ms, I8-059; the skill does not name an owner for it) | the tool and the invoker | none
- I8-195 | SqOut measurement anchor | :23 | "A SqOut gap is measured against the pre-collection end, not the final deactivation node" with the example "Enigma Machine: 4750 ms wrong vs 2083 ms right" | none. Related subject: I8-043 (memory: squeeze drawn at the squeezed chord) | invoker | none
- I8-196 | SP phrase collection extends SP | :22 | "A phrase whose completion note lands inside SP is collected -> SP extends +2 measures." | none | invoker | none. Related fact: "activation needs 2 bars" in I8-026.
- I8-197 | 2x bass kick pitch | :31 | "Ignore 2x-bass kick (pitch 95) ... It counts as a phrase's completion note in 2x Bass mode" | none | invoker | none
- I8-198 | Note at phrase-off tick | :32 | "Count a note at the phrase-off tick | Excluded — the engine closes the phrase before that tick's notes" | none | invoker | none
- I8-199 | Do not use the GUI for this | :33 | "Launch the GUI / hydra_uitest for this | uitest wipes its DB; the GUI can't show these candidates anyway" | none | invoker | none
- I8-200 | FC video command | :42 | "`py C:\Users\Patrick\Downloads\Hydra\video-tools\fcvideo.py "<video.mkv>" "<chart search>"`" and output "`<video>.fcreport.md`" with "the three scores (optimal, path ceiling, actual)" | none | invoker | the invoker supplies video and chart search
- I8-201 | Never modify the Hydra repo for analysis | :49 | "Never modify the Hydra repo (C:\Users\Patrick\Downloads\Hydra\hydra-test) for analysis tasks; the tool is standalone by design." | none | invoker | none
- I8-202 | Memory notes it points at | :34, :47, :49 | "memory `sp-squeeze-mechanics`, `gameplay-video-score-analysis`", "memory note `fc-video-workflow`", "Memory note `sp-squeeze-mechanics` has the verified case study and DB/blob format details." (no file named `gameplay-video-score-analysis.md` exists beside MEMORY.md; I searched the memory folder file list only, not the archive) | none | invoker | none
- I8-203 | Version check | :55 | "Check the game version on the results screen before trusting any comparison; a run that beats the engine's optimal is a version problem or a bug, never a real score, and a Clone Hero 1.0 run is priced with `--legacy-fills`." | none | invoker | none. Compare I8-051 ("Above-optimal scores are NORMAL, not a bug") and I8-016 (priced under the bugged game rule): three statements about above-optimal runs.
- I8-204 | Ceiling | :56 | "The ceiling is always the engine's number for the executed path; a window fitted to the screen hides backend losses." | none | invoker | none
- I8-205 | Loss kinds | :57 | "Every 'other' loss is a bug or a missing loss kind until proven otherwise; the known kinds are frontend, backend, dynamics and declared (multiplier-step hit order, via `--known-loss`)." | none | invoker | none
- I8-206 | Search band | :58 | "Never widen the search band to find a player's path; the engine folds tied prefixes" | none | invoker | none. Same as I8-064.
- I8-207 | Half speed | :59 | "A half-speed recording is read from the original file with `--speed 0.5`, never a re-encode." | none | invoker | none. Same rule as fc-video-workflow:15 ("never rewrite the video").
- I8-208 | Layouts | :60 | "A layout that passed once is in `layouts.json`; a new one needs `--roi` once, then paste the line the run prints." | none | invoker | the invoker pastes the printed line into layouts.json
- I8-209 | Regression check | :61 | "Run `py check_refs.py` after any edit to the tool, before saying the edit works." | none | invoker | none. Same as I8-070.
- I8-210 | One editor of the tool | :62 | "One session edits `fcvideo.py` at a time; everyone else works on a branch (the folder is a git repo)." | none | invoker | none
- I8-211 | Names in a report | :63 | "Every proper name in a report traces to a file, the database or a cited lookup, or prints as `unknown`." | none | invoker | none. Same as I8-108.
- I8-212 | Fleet hooks | :64 | "Note whether the fleet hooks held: one background executor per task, no nested background workers." | none | invoker | none. The preamble (outside my files) says agents run everything in the foreground.
- I8-213 | When to invoke | :3 (description) | "Use when asked whether a Star Power squeeze (SqIn/SqOut) is possible ... or when computing exact squeeze timings, SP phrase collection, or SP end positions beyond what Hydra displays." | none | the model's skill selector | none

## D. Skill reverse-engineer-anything (`C:\Users\Patrick\.claude\skills\reverse-engineer-anything\`)

This is a third-party REA guide (version "42", 207 tools). It decides no Hydra rule. I list only values a session is asked to type, pin or follow, plus any owner claims. Nothing in it names Hydra.

- I8-214 | Skill version and tool count | SKILL.md:6-7 | `version: "42"`, `tool_count: 207` | none | skill loader | none
- I8-215 | REA package version pinned | SKILL.md:36, :52, :56, :79, :84 | "`npx -y rea-agents@6.5.0`" | none | whoever runs the CLI | none. The same pin appears in 5 places in this file; android-applications.md:3-4 names "npm 4.1.0"; javascript-applications.md:169 names "Versions before 4.1.0".
- I8-216 | Supported client IDs, SKILL.md vs reference | SKILL.md:37-42 vs references/connection-and-recovery.md:46-50 | SKILL.md list: `claude_code, claude_desktop, codex, cursor, gemini_cli, windsurf, devin, opencode, antigravity, copilot_cli, commandcode, vscode, grok_build, omp, grok_bot`. The reference list adds `qwen_code`, `trae`, `pi`, `hermes`, `qoder`, `droid`, `zcode`, `dsh`. Two lists of one set. | none | the agent choosing a client | none
- I8-217 | Which first tool for each target | SKILL.md:91-205 | e.g. "ASAR or extracted JavaScript/Electron tree: `analyze_javascript_application`", "Managed PE/CLI assembly: `inspect_managed_artifact`", "Native executable, library, or analysis database: `open_binary`" (a routing list of about 25 target kinds) | none | the agent | none
- I8-218 | Setup requires approval | SKILL.md:50-57 | "obtain approval before setup writes configuration or installs Hopper" and "Add `--install-hopper` only if that separate installation was needed and explicitly approved." | none | the agent | the user is asked to approve setup writes
- I8-219 | What setup never installs | SKILL.md:62-64, :81-83 | "REA setup never installs or upgrades Node.js, npm, Homebrew, Java, Ghidra, IDA, JADX, Binwalk, or Unblob." | none | the agent | none. The sentence appears twice in this file.
- I8-220 | Ask which app | SKILL.md:219-223 | "If the user says only 'my app' without a name, path, or target already selected for this task, ask which app before searching for artifacts." | none | the agent | the user is asked which app
- I8-221 | Detail summary rule | SKILL.md:239-249 | `"detail": "summary"`, then `inspect_analysis_view`; "Do not repeat an identical analysis or rejected call without a changed input or resolved blocker." | none | the agent | none
- I8-222 | Conclusions must separate | SKILL.md:277-280 | "Every conclusion must distinguish observations, inferences, and unknowns. Cite Evidence IDs" | none | the agent | none
- I8-223 | Parallel workers | SKILL.md:310-313 | "When parallel workers are available and the questions do not depend on one another, assign distinct scopes" | none | the agent | none
- I8-224 | Close session | SKILL.md:341-342 | "Close an opened native session with `close_binary` when the investigation is complete." | none | the agent | none
- I8-225 | Defaults quoted | javascript-applications.md:36-38 | "Defaults are 1024 MiB and 300000 ms." (`max_heap_mb`, `analysis_timeout_ms`) | none | the agent | none
- I8-226 | Windows runtime reserve | javascript-applications.md:39-40 | "512 MiB runtime reserve" | none | the tool | none
- I8-227 | Java and JADX requirements | android-applications.md:16-19 | "headless JADX MCP JAR 0.7.1 and Java 17 or newer. `REA_JADX_MCP_JAR` selects the absolute JAR path; `JAVA_HOME` can select Java." | none | the agent | the user supplies the JAR
- I8-228 | Env variable names | native-and-artifacts.md / SKILL.md | `REA_TSHARK_COMMAND`, `REA_SIGROK_COMMAND`, `REA_WABT_BIN_DIRECTORY` (SKILL.md:131, :148, :154), `REA_JADX_HEAP_MIB` (android-applications.md:27) | none | the agent | the user supplies tool paths
- I8-229 | Tool versions demanded | SKILL.md:149, :154, :186 | "sigrok-cli 0.7.2", "WABT 1.0.42", "Node.js 24.x >=24.15 or 26+"; native-and-artifacts.md:30 repeats "Node.js 24.x >=24.15 or 26+" | none | the agent | the user supplies
- I8-230 | Output paging defaults | SKILL.md:134, :150, :187 | "Output defaults to 50 matching frames", "50 literal lines", "20 rows" | none | the tool | none
- I8-231 | Raw binary entry values | native-and-artifacts.md:240-256 | DOS COM "entry is `0x10100`"; raw-binary architectures "`x86`, `x86_64`, `arm`, `arm64`, or `6502`", 6502 range "`0x0000..0xffff`" | none | the agent | the caller supplies architecture and entry_point
- I8-232 | Approval in Codex | connection-and-recovery.md:19, :61-94 | "`MCP tool call requires approval, but approval policy is never`"; TOML `[mcp_servers.rea.tools.open_binary] approval_mode = "approve"` | none | the operator | the user/operator is asked to approve or to preapprove
- I8-233 | Passive capture limits | runtime-observation.md:49-57, evidence-workflows.md:51-56 | `finalization_ms`, "SIGTERM ... SIGKILL once", "V3 captures cannot be upgraded to V4" | none | the tool | none
- I8-234 | Other target-specific rules | references/native-and-artifacts.md (whole file), runtime-observation.md (whole), evidence-workflows.md (whole) | a long list of "use tool X for Y" and "preserve digests, limitations" rules; no Hydra values. One row stands for them. | none | the agent | none

## E. handoff.md (`C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\commands\handoff.md`)

- I8-235 | What the command is for | :2 | "Write a checkable handoff for the next session. Argument - a short kebab-case slug for the file name, such as test-audit-t8-questions" | none | the main session (user types the command) | the user or the main session supplies the slug
- I8-236 | Slug form | :8 | "short kebab-case slug (lowercase words joined by dashes, 2 to 6 words)" and "Do this silently ... never tell the user they gave a bad slug." | none | the main session | the caller supplies a slug
- I8-237 | Where a handoff shape lives | :10 | "Read `docs/agents/handoff.md`. It owns the shape a handoff takes and how its claims are checked; follow it, not memory." | claims "It owns the shape" | the main session | none
- I8-238 | Handoff file path | :11 | "`docs/handoffs/<today>-<slug>-handoff.md`, with today's date as `YYYY-MM-DD`" | none | the main session | none
- I8-239 | What the handoff holds | :11 | "the prose (goal, what was done, what is next), then the `## Claims` table with one row for every fact the next session will act on. Quote the user's rulings ... each in a `ruling` row. Write every hash in full and name no scratch path." | defers shape to docs/agents/handoff.md (second statement of the shape here) | the main session | none
- I8-240 | Checker command | :12 | "`py -I tools/check_handoff.py docs/handoffs/<today>-<slug>-handoff.md`" | none (the preamble, outside my files, gives the same command) | the main session | none
- I8-241 | Checker row words | :12 | "`missing`", "`not-on-main`", "`not-pushed`", "`manual` (test, ruling, state)" | the checker owns these words (not claimed here) | the main session | none
- I8-242 | Checker runs | :12 | "Then run the checker once more. If a row is still `missing` after that second run, leave it, and say which row and why in your report." | none | the main session | none. Fixed count of two runs.
- I8-243 | Checker output section | :13 | "Paste what the checker printed on that second run, in a code fence, under a `## Checker output` heading at the end of the handoff" | none | the main session | none
- I8-244 | Commit rule | :14 | "Commit the handoff on its own branch, staged by name, in the commit form the preamble's 'Committing' section gives (`docs/agents/brief-preamble.md`)." | defers to the preamble | the main session | none. Same as I8-077.
- I8-245 | Report | :15 | "Report the handoff's path, the branch and the commit's full hash." | none | the main session | none
- I8-246 | Stop launching work | :9 | "Stop launching work. Start no new agent and no new task until the handoff is written." | none | the main session | none

## F. Cross-file notes (noticed only; no judgment)

- I8-247 | Same value in several of my files: the status-line interval "every 5 minutes" is in M:every-brief-carries-status-rule.md:15 (I8-066), M:MEMORY.md:45, and M:relay-status-in-plain-english (via its link). Other homes (preamble, agent rule 7) are outside my files.
- I8-248 | The 500 ms squeeze window appears in M:sp-squeeze-mechanics.md:13 (I8-161, names src/core/model.h and src/search/graph.cpp) and in the analyzing-sp-squeezes skill :10 (I8-191, names src/search/graph.h).
- I8-249 | "Never widen the search band" appears in M:engine-tie-merge-prune.md (I8-064), M:fc-video-workflow.md:19, and the analyzing-sp-squeezes skill :58 (I8-206).
- I8-250 | "No full library runs" appears in M:no-full-library-runs.md (I8-113), M:measure-small-explain-first.md:15, M:render-checks-use-the-corpus.md:21, M:MEMORY.md:25 (I8-006).
- I8-251 | The 3 ms leeway is named in M:sp-end-is-one-point-leeway-is-delgars.md:13 (I8-157/I8-158) and in the index MEMORY.md:7.
- I8-252 | "Review key is the branch tip" is in M:git-traps-in-shared-checkout.md:18 (I8-076) and M:one-reviewer-rule-d107.md:15 (I8-125).
- I8-253 | Hook gap noticed, in general words: M:search-before-clicking-a-song.md:17 says a guard on library-row clicks can be bypassed by posted window messages; the note says the rule binds there only as an instruction. Also M:no-full-library-runs.md:15 says "No hook enforces it yet", while the preamble (outside my files) describes a hook for `--whole-library`. No steps given.
- I8-254 | Stated limits in the memory notes that differ from the figure the notes cite for the same thing: library size "18,811" (I8-118), "18,610" (I8-107), "18,755" (I8-171), "19,000-chart" (measure-small-explain-first.md:11), "of 19" (I8-152). One question (how many charts the library holds) is answered five different ways in my files, each as an example in a case story, not as a rule.
- I8-255 | A skill reference to a note that I could not find beside MEMORY.md: `gameplay-video-score-analysis` (I8-202). Notes and files `[[...]] (archived)` links inside notes (about 12 links, for example `[[squeeze-transfer-scale]] (archived)`) point at the archive folder, which I did not read.

## Row count

255 rows numbered I8-001 to I8-255 (some rows hold several closely tied values, as one row each question).
