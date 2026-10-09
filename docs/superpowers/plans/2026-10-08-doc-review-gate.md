# Doc review gate: a fresh agent reads every document before it goes out

Planning agent ad040ddb8e8d7335e wrote this plan on 2026-10-08. The user answered its questions and Tasks 1 to 7 were built on 2026-10-09; section 2a says where the build differs from the text below.

## 1. What gets built, and why

Every document Claude publishes gets read by a fresh agent first. "Fresh" means a new agent that has none of the author's conversation. The hooks refuse the publish step until that agent has signed the document off, and only you can skip the review, by typing a line in chat.

The reason is in `docs/research/ai-generated-docs-review.md`. AI-written documents keep shipping four kinds of failure a reader cannot see from the text: a number with no source, a reference that exists but does not say what the sentence claims, a summary that quietly drifts from its source, and a confident sentence covering a guess. The author cannot catch these in its own work. Models rate their own output more kindly than they rate others' output (the research doc's [S5]), and a model checking its own reasoning without outside facts does not reliably improve it ([S4]). So the check has to come from a second agent that starts from the sources, not from the author's draft.

The build mirrors the derive-once gate that already guards code. That gate has three parts: a command hook that records a reviewer's submit and refuses a submit from the session that wrote the code, a git-side judge that refuses to move `main` without a CLEAN record for the exact change, and a waiver hook that counts only a line you typed. This plan adds the same three parts for documents, plus two new gates for publish steps that are not git commits: the Artifact tool, the plan and question tools, and `gh`.

Four publish steps are gated, by your decision:

1. A commit or merge that moves `main` and adds or changes Markdown under `docs/`, or the top-level `CLAUDE.md`, `CONTEXT.md` or `README.md` (the user's answer to Q4).
2. An Artifact publish (the Artifact tool with `action` publish or no action, and a `file_path`).
3. A plan shown with ExitPlanMode, and a decision question asked with AskUserQuestion.
4. GitHub text posted with `gh`: issue bodies and comments, PR descriptions and comments, release notes.

The key for every document is a fingerprint of its exact text (section 4). The reviewer is a fresh Sonnet agent that gets only the document and its declared sources. It reads cold first, then traces and recomputes every claim and number, then checks the document for drift against its sources, then checks style. One exchange, as in D61: the reviewer reports, the author fixes once, the reviewer fixes what is left and signs off the final text.

## 2. Open questions for you

**Answers (the user, in chat, 2026-10-09).** Each question below was settled on its recommendation. Q1: an edit gets the short review only when it changes at most 20 lines and none of the changed lines adds a fact (Q1-A with Q1-C as the guard). Q2: both the verbatim pass and the short question review (Q2-A and Q2-B). Two smaller numbers were not in the options the user saw: the 15 lines of surrounding text an edit reviewer gets (Q1) and the 60-line size above which a question counts as a plan (Q2-B). Both have no source. They are built as the planner proposed, each as one named constant, and the user was told so and may change them. Q3: the gate applies only in repos whose `main` has a committed `.doc-review-gate` marker. Q4: the commit rule covers Markdown under `docs/` and also `CLAUDE.md`, `CONTEXT.md` and `README.md` at the repo top. Q5 was not put to the user, because allowing `--web` and `--editor` would be a one-flag way around the gate they asked for. Those two flags are denied as proposed.

Each question states the exact rule the hook would apply, with an example that falls just inside and one that falls just outside. Numbers name their source. Where a number has no source, it says so; it is a proposal for you to replace.

**Q1. When does an edit get a review of just the edit, instead of a full review?** This is a new threshold, so it is your call. In every candidate the hook first finds the last reviewed version of the same document (section 3.6 says how), then compares the two texts line by line after normalizing line endings. Three candidate rules:

- **Q1-A, a line count.** Rule: the edit gets an edit review when the number of changed lines (lines added plus lines removed, as `git diff --numstat` counts them) is at most 20. The 20 comes from the IMTI pre-commit review gate, which uses 20 diff lines as its "typo fixes pass through" threshold (prior art, section 8); it is the only published number I found, and they use it to skip review entirely, which this plan does not. Just inside, under Q1-A alone: rewording three sentences in a handoff and fixing a path, 8 changed lines, edit review. (Under the chosen rule, Q1-A with Q1-C, the fixed path is a new fact, so this edit gets a full review.) Just outside: adding a new "Results" paragraph of 11 lines and deleting the 10 it replaces, 21 changed lines, full review.
- **Q1-B, a share of the document.** Rule: edit review when changed lines are at most 10 percent of the reviewed version's line count. The 10 percent has no source; it is my proposal. Just inside: a 200-line plan with 19 changed lines. Just outside: a 40-line release note with 5 changed lines (12.5 percent), full review, even though it is a smaller edit than the first example.
- **Q1-C, no new facts.** Rule: edit review when every added or changed line contains no digit, no URL, no file path, no commit hash and no decision id (`D` followed by digits), whatever the size; otherwise a full review. Just inside: rewriting a whole 30-line section in plainer words with no numbers in it. Just outside: changing "9.7 s" to "9.4 s" in one line, full review, because a changed number is exactly what a reviewer must trace.

Recommendation: Q1-A with Q1-C as a guard, so an edit gets the short review only when it is at most 20 changed lines and none of them adds a fact. Reason: two of the research doc's three biggest gaps are about facts (references that do not support the sentence, and drifted summaries), and so is the first failure in section 1, a number with no source. Q1-C sends every new fact to a full trace. Q1-A alone would let "9.7 s" become "9.4 s" with only a light look. The edit reviewer still gets the changed lines plus 15 lines of surrounding text on each side; the 15 has no source and you may pick another number.

**Q2. How do decision questions avoid a multi-minute review each time?** You decided to gate them, so dropping the gate is not offered. Two rules together are recommended:

- **Q2-A, a verbatim pass.** Rule: an AskUserQuestion call passes without a new review when every question string, every option label and every option description in the call appears verbatim (exact characters, after trimming spaces) inside one document that already has a CLEAN record in the doc-review state folder. Just inside: the plan's "Open questions" section has already been reviewed, and the call copies Q1's text and its three option labels word for word. Just outside: the call shortens one option description from "edit review when changed lines are at most 20" to "edit review under 20 lines"; one string no longer matches, so the whole call needs its own review.
- **Q2-B, a shorter review for standalone questions.** Rule: a question that is not covered by Q2-A gets a question review, not a document review. The reviewer is still a fresh Sonnet agent, but its brief has four checks and no cold read and no style pass: the predicate the question describes is quoted from the code and matches it; each option has a concrete example from each end; the options and their order do not lean toward one answer (the research doc's pattern 7); every number in the question names its source. Just inside: "Should path details be stored?" with options citing `src/store/...` lines. Just outside (gets a full document review instead): a question whose text is longer than 60 lines, because at that size it is a plan, not a question; the 60 has no source and is my proposal.

Recommendation: both. Q2-A removes the review for the common case, where the question was already in a reviewed plan, and Q2-B keeps the gate real for the rest. There is one measurement so far, and it is of a full document review, not a question review. In the manual trial on 2026-10-08, a fresh Sonnet reviewer took 7.5 minutes and 48 tool calls on the Note Shuffle Artifact: a 599-line page plus its script, images and sources. Those three figures come from a cross-session message to the planning session, sent by the Claude session named "AI-generated document quality issues", which ran the trial (transcript `5ecb5b0e-df76-4e3c-8d8a-509a93654368.jsonl`), not from the review file itself. The review is at `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\3a5029e4-d5b4-46bd-9f39-0f09ad308c3a\scratchpad\note-shuffle-review.md`. Paying that before every question would be far too slow, which is why Q2-A and Q2-B exist. A question review has four checks instead of a whole document's trace, so it should be much shorter, but nobody has measured one. Task 9 measures five and reports the median before anyone relies on a number.

**Q3. Where does the gate apply?** The hooks are user-scope and run in every project. Rule proposed: a publish step is gated when the hook's `cwd` is inside a git repository whose `main` holds a committed marker file `.doc-review-gate`, found the same way `Test-DeriveOnceRepo` finds `.derive-once-gate` (the marker at the main checkout's top folder, or in the tree of `main`'s tip). Just inside: a plan shown while the session works in `C:\Users\Patrick\Downloads\Hydra\hydra-test` or one of its worktrees. Just outside: a plan shown while working in `C:\ytdl`, which has no marker; it passes untouched until you add one. The alternative is gating every project, which would stop a plan in a scratch folder with no reviewer brief in reach. The one hole this leaves is `gh --repo splax77/hydra-deluxe` run from a folder outside Hydra; section 9 lists it.

**Q4. Which Markdown counts for a commit?** Your decision says Markdown under `docs/`. Rule as proposed: a path counts when it matches `^docs/.*\.md$` (case does not matter, as in `Test-DeriveOnceGatedPath`). Just inside: `docs/handoffs/2026-10-08-x.md`, `docs/adr/0019-y.md`. Just outside: `CLAUDE.md`, `CONTEXT.md`, `README.md` at the repo top, and `docs/agents/foo.txt`. Do you want the three top-level files added? They are documents in the same sense, and `CONTEXT.md` holds decisions. Recommendation: add them, as a second alternation in the same one regex.

**Q5. What happens to `gh --web` and `gh --editor`?** These open a browser or editor to type the body, so the hook can never see the text. Rule proposed: inside a gated repo, any `gh issue|pr|release create|edit|comment` with `--web` or `--editor` is denied, with the reason that the body must be given on the command line or in a file so it can be reviewed. Just inside: `gh pr create --web`. Just outside: `gh pr view 12 --web`, which only reads. Recommendation: deny; otherwise they are a one-flag bypass.

## 2a. As built (2026-10-09)

Tasks 1 to 7 were built on 2026-10-09 and differ from the text below in these places. Where they disagree, this section is what the hooks do.

The review record has two fields where section 3.8 and section 3.2 used one word, "base". `Base` keeps section 3.8's meaning only: the key the reviewer was handed, present only when it fixed the text. A separate `edit_base` records which reviewed version an edit review was compared against. The deny that decides an edit review saves it, and the submit gate copies it into the record. An edit record counts only when its `edit_base` is itself signed off. It does not have to equal `main`'s old blob, because a reviewed base plus a reviewed change covers the whole new text even if `main` moved in between. The deny also saves the review kind (full, edit or question), and the submit gate records that kind rather than trusting the reviewer's.

The submit gate's author check walks the key, the submitted `Base`, and each earlier review's `Base`. Its sources check walks the same chain, so sources declared on any key along it count. It refuses a reviewer who is any registered author on that chain, even a co-author. For a key that is a blob in the repo, it also counts the `Agent:` trailers of the commits that add or remove that blob. That commit walk stops at 50 commits and refuses when it hits the cap. The 50 has no source; it is the main session's proposal and the user was told.

`doc_review_sources.ps1` cannot see its caller's agent id, so the submit gate records the sources call. A call from the main session is recorded with the main session as author, never refused.

Pushes are judged too. A push that moves the remote's `main` is refused unless every document in the pushed range is signed off. The plan above covered only local moves of `main`, which let `git push origin <branch>:main` skip the check. A commit already on local `main` gets no opinion from the push rule, the same as derive-once's push rule.

Two fail-open risks were closed. A folder outside any git repo counts as ungated instead of throwing. Every library function whose empty answer would mean "allowed" now raises its errors even when the caller silences errors. The tests for this (`test_h31`) run under Windows PowerShell 5.1, the shell the hooks run in. That PowerShell 7 behaves the same rests on the library author's probe, noted in a code comment, not on a test.

The commit and push rules judge only documents that a change adds or modifies; a deleted document needs no review. The `.doc-review-gate` marker is protected too. An edit of it needs a review of its new text, as derive-once requires a review for its own marker. Deleting it goes further than derive-once: no review can allow it, and it is refused unless the user types the waiver.

The fact check behind Q1 ignores markup tags, so a closing tag like `</p>` does not count as a file path. A tag name with a digit, like `<h2>`, still counts as a digit, so an edited HTML heading gets a full review.

The `gh` gate also covers gh's `new` alias, and refuses a body file that another command in the same call names. It also gates the body of `gh pr merge`, and `gh api` text sent as a `body`, `title` or `description` field or as an `--input` file. It refuses `--fill`, `--fill-first`, `--fill-verbose`, `--generate-notes` and `--notes-from-tag`, because their text comes from commits or tags nobody reviewed. It leaves these unchecked on purpose: titles (a title-only edit, and the title or subject flags of issue, pr and release commands), `gh api` text inside a GraphQL query or in any field other than `body`, `title` and `description`, and `gh` started through `Start-Process` or a script file. So a PR or issue title is not reviewed.

The waiver tests live in their own file, `tests\test_h36_doc_review_waiver.ps1`, not in `test_h31`. Both waivers check "did the user type it" through one function, `Test-UserTypedPrompt` in `lib\user_typed.ps1`. `Test-DeriveOnceWaiverTyped` now calls it too. `test_h15`, unchanged since 2026-10-07, still passes: the main session's run on 2026-10-09 after the edit printed 264 PASS lines, no FAIL line and exit 0. A reviewer that fixed a plan, page or gh text in the author's file names that file as a third word of its submit. The gate accepts it only when the key helper gives it the review's key, and the submit helper keeps it in the `reviewed` folder, so the next edit can get an edit review. The question deny names both keys and the fixed file (section 3.4) when it finds a signed-off fix of a question the same author asked before. A fix that already went out is not named.

## 3. How the gate works, step by step

### 3.1 The shape every step shares

The author tries the publish step. The hook computes the document's key (section 4), looks for a CLEAN record for that key in `C:\Users\Patrick\.claude\hooks\state\doc_review\reviews.jsonl`, and either lets the call through or denies it. The deny reason is the author's instructions: the key, the path where the hook saved the exact text, whether a full review or an edit review is due, the brief path `docs/agents/doc-review.md`, and the exact submit command the reviewer will run. It ends with an `Instead:` line, as `library_run_gate.ps1`'s denies do.

Before a reviewer can be dispatched the author declares the document's sources (section 3.7). The deny says so when they are missing.

The reviewer reads, fixes, and submits with `doc_review_submit.ps1 <key> "<review file>"`. The submit gate records the review with the submitter's real `agent_id`, refuses a submit with no `agent_id` (the main session), and refuses a submit from the document's registered author. The author then repeats the publish step with the reviewer's final text. Its key is the one the reviewer signed, so it passes.

Every deny, allow-with-note and record write goes through one library, `C:\Users\Patrick\.claude\hooks\lib\doc_review_rules.ps1`, the way every derive-once rule lives in `lib\derive_once_rules.ps1`. The gates only read their input and call it.

### 3.2 Step 1: commits and merges that move `main`

This is judged on the git side, in the same place derive-once is judged: `git_ref_gate.ps1`, called by the `reference-transaction` shim that `install_git_ref_gate.ps1` already put into the repo's `.git/hooks`. That shim runs on every move of `main`, whatever command caused it, and only when `CLAUDECODE=1`, so your own terminal is never affected.

The new rule, `Get-DocReviewMainMoveRefusal Old New`, runs after the derive-once rule and refuses if either refuses. It lists the files changed from `Old` to `New` (`git diff --no-renames --name-only -z`), keeps those matching the Q4 regex, and for each one takes the blob id at `New` (`git rev-parse New:<path>`). That blob id is the document's key; section 4 says why it equals the key every other step computes. Each key needs a CLEAN record, or an edit-review record whose base is the blob id of the same path at `Old`, or a waiver. The deny lists every path that lacks one, with its key and whether a full or an edit review is due, so one reviewer can be given several keys at once.

Authors for the author-exclusion check come from the commits' `Agent:` trailers, as `Get-DeriveOnceAuthors` reads them today, plus whoever registered the document (section 3.7). A doc committed by the main session with no trailer has the main session as author, which cannot submit anyway.

Commits on a worktree branch are not judged; the review happens when the branch merges to `main`, as for code. A docs-only change that today passes the derive-once judge freely will now need a doc review; that is the point of the change.

### 3.3 Step 2: Artifact publishes

A new PreToolUse hook, `doc_review_tool_gate.ps1`, registered on the matcher `Artifact|ExitPlanMode|AskUserQuestion`. For the Artifact tool it acts when `tool_input.action` is `publish` or absent and `tool_input.file_path` is set and `tool_input.asset` is not true. It reads the file's bytes, computes the key, and checks for a record. Supporting files in `files` are not gated; only the page itself is. A publish from a `type_url` with no `file_path` has no text to review and passes. A `read`, `list`, `open`, `pin` or `delete` action passes.

When the gate denies, it copies the file to `state\doc_review\pending\<key>.<ext>` so the reviewer reads exactly what was about to go out, and records the caller as the document's author.

### 3.4 Step 3: plans and decision questions

The same hook. For ExitPlanMode the text is `tool_input.plan`; Claude Code fills that field from the plan file before hooks run (the Claude Code hooks reference, listed in section 8, says "Claude Code injects the plan content and file path before passing the input to hooks", and names the fields `plan` and `planFilePath`). The key is the hash of that text. The reviewer's fixes go into the file at `planFilePath`; the author calls ExitPlanMode again and the injected text carries the new key.

For AskUserQuestion the text lives only in the tool input: `tool_input.questions`, a list of objects with `question`, `header`, `options` (each `label` and `description`) and `multiSelect`. I confirmed that shape in three of this project's own transcripts (`C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\*.jsonl`). The hook serializes the `questions` list in one canonical form (keys sorted, no whitespace, UTF-8) and hashes that. On a deny it saves the canonical JSON to `state\doc_review\pending\<key>.json`. The reviewer writes its fixed version to `state\doc_review\fixed\<newkey>.json` with the key helper, submits the new key, and the author copies that JSON into its next AskUserQuestion call. If the author retypes the question and a character differs, the key differs and the call is denied; the deny names both keys and the fixed file to copy from. Q2-A's verbatim pass is checked before any of this: when every string in the call sits inside a reviewed document, the call goes through with an `additionalContext` note naming that document's key.

Both of these tools are called by the main session, so a deny lands on the session, which dispatches the reviewer. The hook never answers `ask`; a deny returns at once (the fail-loudly rule).

### 3.5 Step 4: `gh` text

A new gate in `dispatch_bash_ps.ps1`'s `$gates` list, `doc_review_gh_gate.ps1`, after `library_run_gate.ps1`. It reads the command line through the one shell scanner, `lib\shell_scan.ps1`, which already hands back each command's verb, words and here-doc bodies (`.Heredocs`, each with a `.Body`). It acts on a command whose verb is `gh` or `gh.exe` with any folder in front (the brief says `C:\Program Files\GitHub CLI\gh.exe` on this machine), whose first two words are one of `issue create`, `issue edit`, `issue comment`, `pr create`, `pr edit`, `pr comment`, `pr review`, `release create`, `release edit`, or `api` with a `body=` field. The text comes from, in order of what the scanner sees: `--body`/`-b <text>`, `--notes`/`-n <text>`, `--body-file`/`--notes-file`/`-F <path>` (the hook reads the file), `-F <path>` set to `-` with a here-doc on the same command (the body is the here-doc's `.Body`), and for `gh api` a `-f body=<text>` or `-F body=@<path>` word. A word that expands (`$var`, `$( )`, a backtick) is denied because the hook cannot know the text; the deny says to write the text to a file and pass the file. `-F -` fed by a pipe from another command is denied for the same reason. `--web` and `--editor` are Q5.

The hook saves the text to `state\doc_review\pending\<key>.md` and computes the key over it. Edits (`gh issue edit N --body`) look up the last reviewed text posted to the same target (`issue N`, `pr N`, `release <tag>`), which the gate records on every allowed post, so an edit review (Q1) is possible there too.

The scope check is the Q3 marker on the `cwd`'s repo.

### 3.6 Finding the last reviewed version (for Q1)

Every allowed publish writes one line to `state\doc_review\published.jsonl`: the key, the kind (commit, artifact, plan, question, gh), the target (the repo path for a commit, the file path for an artifact or plan, `issue N` or `pr N` or `release tag` for gh), the session and the time. Every reviewed text is kept at `state\doc_review\reviewed\<key>.<ext>` by the submit helper. When a publish is denied, the library finds the base as follows. For a commit, the base is the blob id of the same path at `Old` (`main`'s tip before the move), if that blob has a CLEAN record. For an artifact or plan, the base is the last published key for the same file path. For gh, the base is the last published key for the same target. Questions have no base and no edit review. If a base exists, the library computes the changed-line count between the base text and the new text and applies the Q1 rule, and the deny says which review is due.

### 3.7 Declaring sources

A review has nothing to trace against without the sources, so a sources list is required. The author writes a plain text file with one source per line: a file path with a line range, a whole folder, a commit hash, a decision id, a URL or another document's key, each followed by a few words saying what it supports. The list names every folder the author worked from, not only the files it quotes. The session that ran the trial reported that giving the trial reviewer (section 2, Q2) the full folder list worked well. That reviewer's drift findings compared the page against the design plan in its sources, which said more than the page did. Then it runs `doc_review_sources.ps1 <key> "<sources file>"`, which copies the file to `state\doc_review\sources\<key>.txt` and records the caller as the document's author. The submit gate refuses a review for a key that has no sources file, with the reason "the author has not declared this document's sources; run doc_review_sources.ps1 first". The publish gate's deny says the same when sources are missing, so the author learns it before dispatching anyone.

For a question (Q2-B) the sources file names the code lines of the rule the question describes. For an edit review it names the sources of the new lines only.

### 3.8 The reviewer's fixes and the key

Derive-once already solves this. Its brief says the reviewer signs off "the new tip after step 5", and `Get-DeriveOnceRefusal` accepts a CLEAN review from a reviewer who wrote some, but not all, of the commits. The equivalent here: the reviewer's final review file carries `Key: <new key>` for the text after its fixes and a `Base: <old key>` line naming the key it was given. The submit gate records both. On publish, the library accepts a CLEAN record for the new key when its `Base` chain reaches a key whose registered author is someone other than the reviewer. A CLEAN record with no `Base` is accepted only when the key itself was registered by someone other than the reviewer. So a reviewer can rewrite every line and still sign off, because it signed off a document someone else wrote first, and nobody can review their own text by registering it under a different key.

The reviewer computes the new key with `doc_review_key.ps1 <file>`, the one helper that calls the library's key function; nobody computes a key any other way.

### 3.9 Waivers

Only you can waive, by typing exactly `waive doc-review <7 to 40 hex characters>` as the whole message. The 7-to-40 range is `Get-DeriveOnceWaiverPrefix`'s rule (`lib\derive_once_rules.ps1`, line 106). The new `doc_review_waiver.ps1` on UserPromptSubmit records the prefix, the prompt id and the transcript path, exactly as `derive_once_waiver.ps1` does, and the waiver counts only once the transcript shows a `user` record with that prompt id, `turnOrigin` `human`, not meta, not a sidechain, not a tool result, whose whole text is the phrase. That check exists today as `Test-DeriveOnceWaiverTyped`. Task 6 moves it into a shared `lib\user_typed.ps1` so both gates call one function, and keeps `test_h15` green to prove nothing moved.

A waiver phrase inside a longer message, in a file an agent wrote, or in a tool result never counts. The hook cannot refuse anything, so on error it records nothing and shows a warning (`systemMessage` plus stderr), never a decision.

## 4. The key and the submit flow

The key is the git blob id of the document's exact text: SHA-1 over the bytes `blob <length>\0<text>`, 40 hex characters. One function, `Get-DocReviewKey`, computes it in .NET from the bytes after two normalizations: a UTF-8 byte-order mark is dropped and CRLF becomes LF. Why this and not a plain SHA-256: for a docs commit, the key then equals the blob git already stores for that file at that commit (`git rev-parse <commit>:<path>`), as long as the file is committed with LF endings, which `.gitattributes` or `core.autocrlf` ensures here. The git-side judge needs no extra storage and nothing can drift between "the text the reviewer saw" and "the text on main". The 40-hex shape also lets the reviewer brief's `Key:` regex and the derive-once submit shape be reused unchanged. A test pins that `Get-DocReviewKey` on a committed file equals `git rev-parse HEAD:<path>` for that file.

The submit flow is the derive-once one, renamed. `doc_review_submit.ps1 <key> "<review file>"` only copies the review into `state\doc_review\<key>.md`. The recording happens in `doc_review_submit_gate.ps1`, a gate in `dispatch_bash_ps.ps1`, which sees the call through the shell scanner with the same shape rule the derive-once gate applies (one command, depth 0, no here-doc, no redirect, no expanding word, none of `$ \` ( ) ; & | < >` in the helper path, key or file path). It then checks: an `agent_id` is present (else "a review has to come from a fresh agent"); the review file exists and holds `Key: <key>` matching the command, `Verdict: CLEAN` or `Verdict: FINDINGS`, and an optional `Base: <40 hex>`; a sources file exists for the key or its base; the submitter is not a registered author of the key or any key in its base chain, and not an `Agent:` trailer author for a commit key. It writes one JSON line to `reviews.jsonl`: key, base, verdict, reviewer, SHA-256 of the review file, session, time, and the review kind (`full`, `edit`, `question`). The allow rule `PowerShell(& "C:/Users/Patrick/.claude/hooks/doc_review_submit.ps1"*)` goes next to the derive-once one in `settings.json`, added by you or the main session, because a subagent cannot edit that file.

## 5. The reviewer brief, `docs/agents/doc-review.md`

Built from the research doc's Part 2 and Part 4. Outline only; the executor writes it in Task 7 and it gets its own review (section 6, Task 8).

Opening: you are reviewing a document before it goes out; you did not write it; you run on Sonnet; you use no helper agents and start no background jobs; read `brief-preamble.md` first; the status-line rule applies. Then what you get: the key, the saved text, the sources file, the kind of review (full, edit, question), and for an edit review the base text and the changed lines. Until your fix step you are read-only.

Step 1, the cold read, before anything else. Without the original request and without the sources, read the document once and write back in your own words, in at most five sentences, what it decides, what it asks the reader to do, and the three numbers the reader will rely on. Then open the sources. Where your restatement and the sources disagree, the document is wrong, not your reading. This is the paraphrase test and the "verify before reading the author's version" order from [S11], [S14] and [S19].

The step order below was tried once, in the manual trial on the Note Shuffle Artifact (section 2, Q2), and it worked well enough to keep: cold read, then trace and recompute every claim, then drift against the sources, then style. Giving that reviewer the full list of source folders, not only single files, also worked and is kept (section 3.7).

Step 2, the trace and recompute. Break the document into single claims. Where a number can be worked out again from its inputs (a sum, a table, a worked example), work it out again and compare. For each number: where was it read, on what inputs, how many runs, what spread; a number with none of these is a finding. For each reference (file, line, commit, decision id, ADR, URL, document key): open it and confirm it says the thing it is cited for; a reference that exists but supports a narrower or different sentence is a finding, and so is one that cannot be opened. For each paraphrase of a decision, a rule or a test result: put the original next to it and compare; a drift is a finding. For each confident claim with no source: it is a finding unless the text marks it unverified and says where the author looked. For a decision question: the predicate is quoted from the code, both ends have an example, the options do not lean, and the strongest case against the recommendation is present. A compliance claim ("tests pass", "reviewed", "no behaviour change") with no command, output line, diff or key behind it is a finding. Say where you looked; a search that found nothing proves nothing.

Step 3, drift against the sources. For each section, compare how sure the document sounds with how sure its sources are. A flat statement where the source says "inferred", "not seen" or "one run" is a finding. So is a caveat the source states loudly that the document plays down or leaves out. This was the trial's most useful step: its strongest findings were places where the page sounded more certain than its sources, and one where it played down a problem its sources state plainly.

Step 4, the reader. Does the first paragraph state the decision, result or ask? Would the reader who stops there know the answer? Is every term used in one sense, and one term per thing?

Step 5, style, last. Jargon glossed on first use; sentences short; no bullet walls of file:line with no sentences around them; no chat sentences addressed to the author's partner; no boilerplate sections nobody asked for. The brief says plainly: do not judge whether the text "reads like AI"; that check is unreliable ([S1]) and wastes the call.

Scoring: a wrong claim counts worse than a flagged gap, so a finding never says "just delete the caveat".

The exchange, from D61: write the review; if CLEAN, submit; if not, send the findings to the author once with SendMessage and end your turn; when the author replies, check each finding at the new text; fix what is left yourself; write the final review on the new key with the `Base:` line; submit. The one exception is a leftover that needs the user (a new number, a change to what the user sees) or is too large for about 30 tool calls: submit FINDINGS naming it and report.

Output: the review file with `Key:`, `Base:` (when you fixed anything) and `Verdict:` lines, then one short paragraph each for Restatement, Findings (one paragraph per finding: the claim, the source opened, what it actually says), Numbers checked, References opened, Notes, and Where I looked. Submit with `& "C:/Users/Patrick/.claude/hooks/doc_review_submit.ps1" <key> "<path>"`, in its own call, typed out in full.

The question-review variant (Q2-B) is a short section at the end: the four checks, no cold read, no style pass, same output lines.

## 6. The tasks, in build order

Executors run on Opus; reviewers on Sonnet; every brief carries the status-line rule and the preamble. Hook code and tests are written by executor agents, because `fable_delegate_gate.ps1` stops the main session writing non-doc files over its limit. `C:\Users\Patrick\.claude` is not a git repo, so every hook change comes with its test file, as the existing hooks do. Each task runs only its own test file, never the whole hook suite.

**Task 1, the rules library.** Writes `lib\doc_review_rules.ps1`: the marker check (`Test-DocReviewRepo`, `Test-DocReviewRepoAt`), the key function (`Get-DocReviewKey`), the state paths, the record readers and writers (reviews, published, authors, sources, waivers), the Q1 rule (`Get-DocReviewEditKind`), the Q2-A verbatim check, the base finder (section 3.6), the acceptance rule with base chains (section 3.8), the deny text builder with its `Instead:` line, the AskUserQuestion canonical serializer, and `Get-DocReviewMainMoveRefusal`. Ends with `$DocReviewRulesLoaded = $true` as the last line, like its sibling. Test: `tests\test_h31_doc_review_rules.ps1`, unit cases against a temp hooks copy and a throwaway repo (section 7 lists them). Nothing else can start until this lands.

**Task 2, the helpers and the submit gate.** `doc_review_submit.ps1`, `doc_review_sources.ps1`, `doc_review_key.ps1`, and `doc_review_submit_gate.ps1` added to `dispatch_bash_ps.ps1`'s `$gates` after `derive_once_review_gate.ps1`. Test: `tests\test_h32_doc_review_submit.ps1`. Runs after Task 1. Parallel with Tasks 3, 4, 5, 6.

**Task 3, the git side.** `git_ref_gate.ps1` loads both rules libraries and refuses when either refuses; the shim and installer do not change. Test: `tests\test_h33_doc_review_git.ps1`, built like `test_h17`: throwaway repos, the real installer, real git with `CLAUDECODE` set and unset. Parallel with 2, 4, 5, 6.

**Task 4, the tool gate.** `doc_review_tool_gate.ps1` for `Artifact|ExitPlanMode|AskUserQuestion` (section 3.3 and 3.4). Test: `tests\test_h34_doc_review_tools.ps1`. Parallel with 2, 3, 5, 6.

**Task 5, the gh gate.** `doc_review_gh_gate.ps1`, last in `dispatch_bash_ps.ps1`'s `$gates` (section 3.5). Test: `tests\test_h35_doc_review_gh.ps1`. Parallel with 2, 3, 4, 6. Tasks 2 and 5 both edit the `$gates` list; the merge is two added lines.

**Task 6, the waiver.** Moves `Test-DeriveOnceWaiverTyped`'s transcript check into `lib\user_typed.ps1` as `Test-UserTypedPrompt`, called by both rules libraries; writes `doc_review_waiver.ps1`. Tests: new cases in `test_h31`, and `test_h15_derive_once.ps1` rerun once to prove the move changed nothing. Parallel with 2 to 5. Also adds every new hook that calls `Read-HookInput` to `test_h19`'s list, which fails on any `Get-Content` without `-Encoding`.

**Task 7, the brief.** `docs/agents/doc-review.md` from the outline in section 5, and one short section in `CLAUDE.md` pointing at it, next to the derive-once section. Parallel with everything from Task 1 on. Test: none; it gets a review in Task 8.

**Task 8, bootstrap and registration.** Section 6a below. Runs after all of 1 to 7.

**Task 9, measure.** Five standalone question reviews (Q2-B) on real questions from this repo's transcripts, timed from dispatch to submit, median and spread reported in `docs/handoffs/`. Any number the user later relies on for Q2 comes from this run. After Task 8.

### 6a. Bootstrap: how this plan and the brief get their first review

The plan and `doc-review.md` are documents, and the gate does not exist yet. The order that keeps "every document reviewed by a fresh agent" true from the first commit:

Tasks 1 and 2 land first (the library, the helpers and the submit gate), with no publish gate registered yet. Now a reviewer can submit a real record. The main session then dispatches a fresh Sonnet agent with the brief outline from section 5 (the brief file itself is being written in Task 7 and is one of the two documents under review), the sources list for this plan (the research doc, the five hooks files read, the two transcripts grepped, the eight web pages opened), and the key from `doc_review_key.ps1`. The reviewer reviews this plan and `doc-review.md`, fixes once after my one reply, and submits CLEAN for each final key. Only then are the plan, the brief, the `CLAUDE.md` section and the `.doc-review-gate` marker committed to `main`; the git-side rule from Task 3 is already in place by then and sees the records. Last, the main session registers the tool gate matcher and the waiver hook in `settings.json` and adds the submit allow rule. From that moment every publish is gated, this plan's later edits included.

## 7. The hook test list

Every test runs against a copy of the hooks in a temp folder, as `test_h15`, `test_h17` and `test_h30` do, so records never land in the real `hooks\state`. Each case is one plain sentence; each file prints PASS/FAIL lines and exits 1 on any failure.

Step 1, commits (test_h33):
- A merge to main that adds `docs/x.md` with no record is refused, and the reason names the path, its key, the brief and the submit command, and ends with an `Instead:` line.
- The same merge after a fresh agent's CLEAN record for that blob id passes.
- A commit to main that changes `docs/x.md` where the new blob has an edit-review record whose base is the old blob passes.
- A merge that changes two docs with a record for only one is refused and names the one missing.
- A commit that touches only `src/` and `tests/` gets no opinion from the doc rule (the derive-once rule still judges it).
- A commit that touches only `docs/notes.txt` passes untouched.
- A repo with no `.doc-review-gate` marker gets no opinion.
- With `CLAUDECODE` unset the shim does nothing and the move goes through.

Step 2, Artifact (test_h34):
- A publish of a `.html` or `.md` file with no record is denied with the key and the saved pending path.
- The same publish after a CLEAN record for that key passes.
- A publish after the file was edited is denied, the deny names the new key, and the old record is not accepted.
- An `action` of `read`, `list`, `open`, `pin` or `delete` passes untouched.
- A publish with `asset: true` passes untouched.

Step 3, plan and question (test_h34):
- ExitPlanMode with a `plan` text that has no record is denied with the key.
- The same text after a CLEAN record passes.
- ExitPlanMode after the reviewer's fix, where the record's key is the new text and its `Base` is the old key registered by another agent, passes.
- AskUserQuestion whose canonical JSON has no record is denied and the pending JSON is saved.
- AskUserQuestion whose every string sits verbatim inside a reviewed document passes with an `additionalContext` note naming that document's key.
- AskUserQuestion where one option description differs by one word from the reviewed document is denied.

Step 4, gh (test_h35):
- `gh issue create --body "..."` in a gated cwd with no record is denied with the key of the body text.
- The same command after a CLEAN record passes.
- `gh pr create --body-file notes.md` reads the file and keys its text.
- `gh release create v1 --notes-file - <<'EOF' ... EOF` keys the here-doc body.
- `gh issue comment 5 --body "$text"` is denied because the word expands and the hook cannot see the text.
- `gh pr create --web` is denied (Q5).
- `gh issue view 5` and `gh pr list` pass untouched.
- The same create command in a cwd with no marker passes untouched.
- `gh.exe` called by its full `C:\Program Files\GitHub CLI\gh.exe` path is recognized.

Keys and stale keys (test_h31):
- `Get-DocReviewKey` of a committed LF file equals `git rev-parse HEAD:<path>`.
- The key of a CRLF copy of the same text equals the key of the LF text.
- A text with a byte-order mark keys the same as without.
- A one-character change gives a different key and the old record no longer matches.
- Q1 rule: 20 changed lines with no new digit, path, hash or URL is an edit review; 21 is full; 3 changed lines where one adds a number is full.

Author exclusion (test_h32):
- A submit from the main session (no `agent_id`) is denied.
- A submit from the agent that registered the document's sources is denied.
- A submit for a commit key from an agent named in the commits' `Agent:` trailers is denied.
- A submit from a fresh agent for a registered key with sources is recorded.
- A submit for a key with no sources file is denied naming `doc_review_sources.ps1`.
- A submit whose review file's `Key:` line differs from the command is denied.
- A submit with a `Base:` whose chain ends at a key registered by the submitter itself is denied.
- A submit with a `Base:` registered by another agent is recorded, and a publish of the new key then passes.
- A submit command carrying `$var`, a backtick, `;`, `|` or a second command is denied.

Waivers (test_h31 and test_h33):
- `waive doc-review <prefix>` typed by the user (a transcript record with `turnOrigin` human and the matching prompt id) lets the matching key through.
- The same phrase with `turnOrigin` `sdk` does not count.
- The phrase inside a longer message does not count.
- The phrase in a file an agent wrote, in a tool result, or in a sidechain record does not count.
- Hook input with no `prompt_id` records nothing and prints a visible warning, never a decision.

Fail loudly (every gate's test):
- Hook input that is not JSON gets a deny whose reason says the hook could not read its input and ends with `Instead:`.
- A gated tool call when `lib\doc_review_rules.ps1` is missing or cut off (no `$DocReviewRulesLoaded`) is denied naming the file.
- No gate ever prints `ask`, with or without `agent_id` (a case in `test_h21_no_ask.ps1`'s style).
- A gate error while checking an ungated tool call prints `{}`.

## 8. Prior art

I opened each of these; nothing below is from a search summary alone.

- **IMTI, "The Pre-Commit Review Gate"** (https://imti.co/pre-commit-review-gate/). A PreToolUse hook on Bash denies `git commit` until a sub-agent has written an artifact with `verdict: CLEAN` and a `reviewed_hash`, a 16-character SHA-256 of the diff; any edit changes the hash and "the next commit attempt re-blocks until a fresh review is performed". It skips review under 20 diff lines. The derive-once plan already copied this shape; this plan adds the stale-key behaviour for documents and takes the 20 as Q1-A's candidate number, while refusing to skip review entirely.
- **GitHub, "About protected branches"** (https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches/about-protected-branches). Two settings match this plan's two rules: "dismiss stale pull request approvals when commits are pushed that affect the diff" is the stale key, and "require approval of the most recent reviewable push" requires that push to be "approved by someone other than the person who pushed it", which is the author-exclusion check and the reason the reviewer's own fixes can still be signed off by it (it did not push the first version).
- **SLSA source track sprint, April 2025** (https://slsa.dev/blog/2025/04/slsa-source-sprint). Adds "Level 4 - Two-Party Review", calling two-party review "the strongest control we have against many of the threats listed for threat B - Modifying the source", and notes it is hard for single-maintainer projects, which is why this plan keeps the reviewer an agent rather than a second person.
- **OpenSSF SCM best practices, "Default Branch Should Require Code Review By At Least Two Reviewers"** (https://best.openssf.org/SCM-BestPractices/github/repository/code_review_by_two_members_not_required.html). Mandatory review is there "to comply with separation of duties principle"; a second identity "decreases the risk of an insider threat". The submit gate's `agent_id` check is this plan's separation of duties.
- **Write the Docs, "Docs as Code"** (https://www.writethedocs.org/guide/docs-as-code/). Documentation goes through the same version control and code review as software, and a team can "block merging of new features if they don't include documentation". Step 1 of this plan is docs-as-code with the review made mandatory by a hook instead of by convention.
- **CiteAudit** (https://arxiv.org/abs/2602.23452, abstract). A citation checker that "decomposes citation checking into metadata extraction, memory lookup, web-based retrieval, and final judgment", against "fabricated references that appear plausible but correspond to no real publications". The brief's trace step opens every reference for the same reason.
- **Claude Code hooks reference** (https://code.claude.com/docs/en/hooks, read in two parts). `agent_id` is "present only when the hook fires inside a subagent call", which is what the submit gate relies on; agent-type hooks have a 60-second default timeout and are "experimental and may change", which is why no gate here is an agent hook; `permissionDecision` precedence is "deny > defer > ask > allow"; ExitPlanMode's input carries `plan` and `planFilePath`, injected before hooks run; AskUserQuestion can be answered by a hook only with `allow` plus `updatedInput`, which this plan does not use.
- **The research doc** (`docs/research/ai-generated-docs-review.md`) supplies the review method itself: single-claim checking [S18][S9][S2], checking against inputs before reading the draft [S19][S24][S4], the cold reader [S11][S14], roles [S10], and the LLM-judge pitfalls (position, verbosity, self-preference, lost in the middle) [S3][S22][S5][S23].

Not found within the fetch cap: a published tool that gates an LLM's chat-level outputs (plans, questions) rather than files or commits. The search summaries for "LLM content review pipelines" pointed only at vendor blogs describing claim extraction and tiered human review, which I did not open and do not cite.

## 9. Risks

**Retyped questions miss the key.** AskUserQuestion text lives only in the tool input, so the author must reissue the exact JSON the reviewer signed. The pending and fixed files in the state folder are there to copy from, and the deny shows both keys, but a model that paraphrases will hit a second deny. Q2-A removes most of these cases, since reviewed plans already carry the questions.

**Latency.** A full document review is minutes, not seconds, and plans and release notes will wait for it. The one measurement so far is 7.5 minutes and 48 tool calls for a 599-line Artifact with its sources (the manual trial, as reported in a cross-session message from the session that ran it, not in the review file; section 2, Q2). That is one run on one document, so it gives no spread. Task 9 measures the question variant, and the first week of real reviews gives the document figure. If it is too slow, the levers are the edit review (Q1) and the verbatim pass (Q2-A), not a weaker brief.

**The reviewer agrees with the author.** Sycophancy is one of the research doc's three gaps. The brief fights it with the cold read before the sources and with "write your own recommendation before reading the author's" for decision questions, but a Sonnet reviewer can still nod along. The one measurable signal is a review with zero findings on a document that later turns out wrong; those should be logged and read back.

**The gh scope hole.** `gh --repo splax77/hydra-deluxe` run from a folder without the marker is not gated (Q3). The marker file could list the repository slugs it covers, so the gh gate could also match on `--repo`; that is a small follow-up if the hole is ever used.

**Hooks are unversioned.** `C:\Users\Patrick\.claude` is not a git repo. Every change here ships with its test file, and the test files are the only record of intent. A `.bak-*` copy before each edit, as the existing hooks do, is the rollback.

**User-scope hooks run everywhere.** Without the marker check, a plan in any scratch folder would be denied with a brief it cannot reach. The marker (Q3) is the only thing keeping the gate inside Hydra; a bug in `Test-DocReviewRepoAt` that returns true by default would gate every project, and its fail-closed path must say so plainly rather than deny silently.

**Documents outside `docs/`.** The user's answer to Q4 gates the top-level `CLAUDE.md`, `CONTEXT.md` and `README.md`; other Markdown outside `docs/` is not gated. Handoffs that land in a scratchpad and are never committed are never gated at all; only what is published is.

**Two gates on one commit.** A commit that touches both code and docs now needs a derive-once review and a doc review, from two different reviewers or one agent given both briefs. The deny text from `git_ref_gate.ps1` must list both clearly, or the author will satisfy one and be surprised by the other.

**The reviewer never replies.** As in derive-once, the author reports it and does not wait; the one-exchange rule has no second round.
