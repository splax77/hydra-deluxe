# What Anthropic recommends for maintaining and changing an existing codebase with Claude (Claude Code)

Scope note: every finding below comes from an Anthropic-authored page (anthropic.com, claude.com/blog, code.claude.com docs) unless it is explicitly marked **third-party**. Research done 2026-10-05. Pages were read through a summarizing fetch tool, so short quotes are as returned by that tool. Docs pages (code.claude.com) are living documents with no publication date; they reflect the state on 2026-10-05.

## 1. Claude Code best practices: CLAUDE.md, explore-plan-code-commit, tests, small diffs, subagent review, hooks, headless/CI, multiple Claudes

### Takeaway
Anthropic's core message is that the context window is the scarce resource and that Claude needs a check it can run. The workflow it recommends is explore, then plan, then implement against a test or other pass/fail check, then commit. A short, pruned CLAUDE.md holds what Claude cannot infer from code. Hooks enforce anything that must always happen. A fresh-context reviewer (subagent or second session) checks the diff, but only for correctness and stated requirements, because chasing every review finding causes over-engineering.

### Cited Findings
- The original engineering post "Claude Code: Best practices for agentic coding" (anthropic.com/engineering/claude-code-best-practices) now 308-redirects to the docs page code.claude.com/docs/en/best-practices. The page says its patterns "have proven effective across Anthropic's internal teams." — [Anthropic docs: Best practices](https://code.claude.com/docs/en/best-practices)
- The original post dates from April 2025. **Third-party** confirmation of the date: Simon Willison linked it on 2025-04-19 — [simonwillison.net](https://simonwillison.net/2025/Apr/19/claude-code-best-practices/). The April 2025 version's advice included "think / think hard / ultrathink" phrases and committing before big tasks, per **third-party** summaries — [MarkTechPost](https://www.marktechpost.com/2025/04/21/anthropic-releases-a-comprehensive-guide-to-building-coding-agents-with-claude-code/).
- Organizing constraint: "Claude's context window fills up fast, and performance degrades as it fills." It calls context "the most important resource to manage." — [Best practices](https://code.claude.com/docs/en/best-practices)
- Verification first: "Give Claude a check it can run: tests, a build, a screenshot to compare." Without one, "'looks done' is the only signal available, and you become the verification loop." — [Best practices](https://code.claude.com/docs/en/best-practices)
- Four ways to gate the stop, in rising strength: ask in the prompt; a `/goal` condition re-checked by a separate evaluator each turn; a Stop hook that "blocks the turn from ending until it passes"; or a verification subagent so "the agent doing the work isn't the one grading it." — [Best practices](https://code.claude.com/docs/en/best-practices)
- "Have Claude show evidence rather than asserting success": test output, the command run, or a screenshot. — [Best practices](https://code.claude.com/docs/en/best-practices)
- "Address root causes, not symptoms": example prompt says "address the root cause, don't suppress the error." — [Best practices](https://code.claude.com/docs/en/best-practices)
- Four-phase workflow: Explore (plan mode, read-only), Plan (detailed plan, editable with Ctrl+G), Implement ("verifying against its plan", write tests, run suite, fix failures), Commit (descriptive message, PR). Skip the plan "If you could describe the diff in one sentence." Planning helps most when the approach is uncertain, many files change, or the code is unfamiliar. — [Best practices](https://code.claude.com/docs/en/best-practices)
- Test-first bug fixing: "write a failing test that reproduces the issue, then fix it." — [Best practices](https://code.claude.com/docs/en/best-practices)
- Follow existing patterns rather than inventing new ones: point Claude at an existing example ("HotDogWidget.php is a good example. follow the pattern") and "build from scratch without libraries other than the ones already used in the codebase." Ask subagents whether there are "existing OAuth utilities I should reuse." — [Best practices](https://code.claude.com/docs/en/best-practices)
- For large features, have Claude interview you and write SPEC.md, then "start a fresh session to execute it." Good specs "name the files and interfaces involved, state what is out of scope, and end with an end-to-end verification step." — [Best practices](https://code.claude.com/docs/en/best-practices)
- CLAUDE.md: run `/init`, then refine. "keep it short and human-readable." Test for each line: "Would removing this cause Claude to make mistakes?" If not, cut it. "Bloated CLAUDE.md files cause Claude to ignore your actual instructions!" — [Best practices](https://code.claude.com/docs/en/best-practices)
- CLAUDE.md include: bash commands Claude can't guess, non-default style rules, test instructions, repo etiquette, project-specific architectural decisions, env quirks, gotchas. Exclude: things derivable from code, standard conventions, detailed API docs, frequently-changing info, tutorials, "File-by-file descriptions of the codebase", and "write clean code"-style platitudes. — [Best practices](https://code.claude.com/docs/en/best-practices)
- "Treat CLAUDE.md like code: review it when things go wrong, prune it regularly, and test changes by observing whether Claude's behavior actually shifts." Emphasize ("IMPORTANT") one line only: "If you emphasize many lines, none of them stands out." Check it into git; it "compounds in value over time." `/doctor` proposes cuts for content derivable from the codebase. — [Best practices](https://code.claude.com/docs/en/best-practices)
- Occasional knowledge goes in skills, which load on demand, not CLAUDE.md. — [Best practices](https://code.claude.com/docs/en/best-practices)
- Hooks: "Use hooks for actions that must happen every time with zero exceptions." "Unlike CLAUDE.md instructions which are advisory, hooks are deterministic." Examples: run eslint after every edit; block writes to the migrations folder. Failure-pattern fix: "If Claude already does something correctly without the instruction, delete it or convert it to a hook." — [Best practices](https://code.claude.com/docs/en/best-practices)
- Context hygiene: `/clear` between unrelated tasks. "If you've corrected Claude more than twice on the same issue... Run /clear and start fresh." "A clean session with a better prompt almost always outperforms a long session with accumulated corrections." Named failure patterns: kitchen-sink session, correcting over and over, over-specified CLAUDE.md, "trust-then-verify gap" ("If you can't verify it, don't ship it"), infinite exploration. — [Best practices](https://code.claude.com/docs/en/best-practices)
- Subagents for investigation keep file reads out of the main context. — [Best practices](https://code.claude.com/docs/en/best-practices); [Common workflows](https://code.claude.com/docs/en/common-workflows)
- Writer/Reviewer pattern with two sessions: "A fresh context improves code review since Claude won't be biased toward code it just wrote." Variant: one Claude writes tests, another writes code to pass them. — [Best practices](https://code.claude.com/docs/en/best-practices)
- Adversarial review step: "Before treating a task as done, have a subagent review the diff in a fresh context and report gaps." Example prompt checks the diff against PLAN.md, that edge cases have tests, and "nothing outside the task's scope changed. Report gaps, not style preferences." — [Best practices](https://code.claude.com/docs/en/best-practices)
- Warning about reviewers: "Chasing every finding leads to over-engineering: extra abstraction layers, defensive code, and tests for cases that can't happen." Tell the reviewer to flag only gaps affecting correctness or stated requirements. — [Best practices](https://code.claude.com/docs/en/best-practices)
- Headless: `claude -p` for CI, pre-commit hooks, scripts; JSON / stream-json output. Fan-out: `/batch` splits a change across 5 to 30 subagents, each in its own worktree; or loop `claude -p` over a file list with `--allowedTools` and `--permission-mode dontAsk`. "Test on a few files, then run on all of them." — [Best practices](https://code.claude.com/docs/en/best-practices)
- Parallel sessions: git worktrees so edits don't collide; cross-session messaging; agent teams (experimental). — [Best practices](https://code.claude.com/docs/en/best-practices); [Common workflows](https://code.claude.com/docs/en/common-workflows)
- Checkpoints/rewind let you try risky changes, but "This isn't a replacement for git." — [Best practices](https://code.claude.com/docs/en/best-practices)
- Install a code-intelligence (LSP) plugin for typed languages for "precise symbol navigation and automatic error detection after edits." — [Best practices](https://code.claude.com/docs/en/best-practices)
- Refactoring recipe: find deprecated API usage, get recommendations, refactor "while maintaining the same behavior," run tests. Tips: keep backward compatibility when needed and "Do refactoring in small, testable increments." Review Claude's generated PR and ask it to highlight risks. — [Common workflows](https://code.claude.com/docs/en/common-workflows)
- "Prefer running single tests, and not the whole test suite, for performance" appears as an example CLAUDE.md workflow rule. — [Best practices](https://code.claude.com/docs/en/best-practices)

### Inferences
- The guide's anti-debt levers are: verification gates, plan-before-edit for multi-file work, pointing at existing patterns, scope checks in review, and a pruned CLAUDE.md. "Small diffs" is not stated as a standalone rule; it shows up as "small, testable increments" for refactors and "one feature at a time" in the harness post (section 2).
- Anthropic now explicitly warns that adversarial review can itself add debt (over-engineering). That is a newer nuance not present in the April 2025 version as summarized by third parties.

### Gaps
- I could not read the original April 2025 post text directly because it redirects; its exact original wording and date (believed 2025-04-18) are confirmed only by third-party pages.

## 2. Context engineering, long-running agents, harnesses, building agents, writing tools

### Takeaway
Anthropic treats context as a finite budget that rots as it grows. For work that spans many context windows it recommends compaction, external notes files, and subagents with clean contexts. For long coding runs it recommends an initializer that sets up a feature list, a progress log, an init script and git, followed by a coding agent that does one feature per session, tests it end to end, commits, and leaves the repo mergeable.

### Cited Findings
- "Effective context engineering for AI agents," published 2025-09-29. "context is a finite resource." Introduces "context rot": recall accuracy drops as tokens grow. — [Anthropic Engineering](https://www.anthropic.com/engineering/effective-context-engineering-for-ai-agents)
- Same post: system prompts should sit in a "Goldilocks zone" (specific enough to guide, flexible enough not to be brittle). Tools should be self-contained with minimal overlap; bloated tool sets create decision ambiguity. — [Anthropic Engineering](https://www.anthropic.com/engineering/effective-context-engineering-for-ai-agents)
- Three long-horizon techniques: compaction (summarize history, keep architectural decisions), structured note-taking (e.g., NOTES.md persisting across resets), and sub-agent architectures returning condensed summaries. Prefer "just-in-time" retrieval through tools over pre-loading everything. — [Anthropic Engineering](https://www.anthropic.com/engineering/effective-context-engineering-for-ai-agents)
- "Effective harnesses for long-running agents," published 2025-11-26. Problem: "each new engineer arrives with no memory of what happened on the previous shift." — [Anthropic Engineering](https://www.anthropic.com/engineering/effective-harnesses-for-long-running-agents)
- Observed failure modes: trying "to one-shot the app," running out of context mid-feature, "leaving the next session to start with a feature half-implemented and undocumented"; a later agent would "see that progress had been made, and declare the job done." — [Anthropic Engineering](https://www.anthropic.com/engineering/effective-harnesses-for-long-running-agents)
- Clean-state standard: end each session with "code that would be appropriate for merging to a main branch: there are no major bugs, the code is orderly and well-documented," so "a developer could easily begin work on a new feature without first having to clean up an unrelated mess." — [Anthropic Engineering](https://www.anthropic.com/engineering/effective-harnesses-for-long-running-agents)
- Harness pieces: a JSON feature list, a claude-progress.txt log, git commits with descriptive messages as recovery points, an init.sh script, one feature at a time, end-to-end testing with browser automation. — [Anthropic Engineering](https://www.anthropic.com/engineering/effective-harnesses-for-long-running-agents)
- Test protection: "It is unacceptable to remove or edit tests because this could lead to missing or buggy functionality." JSON chosen because "the model is less likely to inappropriately change or overwrite JSON files compared to Markdown files." — [Anthropic Engineering](https://www.anthropic.com/engineering/effective-harnesses-for-long-running-agents)
- Session start routine: `pwd`; "Read the git logs and progress files"; read the feature list and pick the highest-priority unfinished feature. End-to-end testing worked "once explicitly prompted to use browser automation tools and do all testing as a human user would." — [Anthropic Engineering](https://www.anthropic.com/engineering/effective-harnesses-for-long-running-agents)
- Future direction named in the post: specialized roles such as testing, QA, and "code cleanup" agents. — [Anthropic Engineering](https://www.anthropic.com/engineering/effective-harnesses-for-long-running-agents)
- "Building effective agents," published 2024-12-19. Principles: simplicity, transparency (show planning steps), and a carefully crafted agent-computer interface. Patterns include evaluator-optimizer and orchestrator-workers. Coding-agent appendix: "automated testing helps verify functionality, human review remains crucial for ensuring solutions align with broader system requirements." — [Anthropic Engineering](https://www.anthropic.com/engineering/building-effective-agents)
- "Writing effective tools for agents," published 2025-09-11. "More tools don't always lead to better outcomes"; namespace tools; return "high signal information"; use pagination, filtering, truncation for token efficiency; iterate tools against evaluations, using Claude Code to improve them. — [Anthropic Engineering](https://www.anthropic.com/engineering/writing-tools-for-agents)

### Inferences
- The harness post's "clean state" rule and "don't edit tests" rule are Anthropic's most direct statements against accumulating debt during autonomous runs.
- The "less likely to overwrite JSON" finding suggests putting machine-tracked state (task lists, pass/fail flags) in structured files rather than prose.

### Gaps
- None of these posts gives measured debt or quality outcomes; they are design guidance with qualitative observations.

## 3. How Anthropic uses Claude Code on its own codebase

### Takeaway
By May 2026 Anthropic says more than 80% of code merged into its codebase was authored by Claude, and per-engineer output rose about 8x. It pairs this with automated multi-agent Claude review on every PR, risk-tiered human review, and logged, human-sampled automated approvals. Anthropic itself flags that lines of code overstate the gain and that Claude's code only reached rough quality parity with human code in 2026.

### Cited Findings
- "When AI builds itself": as of May 2026, "more than 80% of the code we merge into Anthropic's codebase was authored by Claude," up from "low single digits" before Claude Code launched in February 2025. — [Anthropic Institute](https://www.anthropic.com/institute/recursive-self-improvement)
- Same report: engineers merged about 8x as much code per day in Q2 2026 vs 2024; LOC per engineer was flat 2021-2024 then climbed in 2025. Caveats: LOC is "an imperfect measure, as it measures quantity over quality" and 8x is "almost certainly an overstatement of the true productivity gain." Median of 130 surveyed staff (March 2026) estimated about 4x output. — [Anthropic Institute](https://www.anthropic.com/institute/recursive-self-improvement)
- Quality: Claude-written code was "somewhat worse than human-written code" in late 2025 and is "roughly at parity today." Success on open-ended tasks reached 76% in May 2026. — [Anthropic Institute](https://www.anthropic.com/institute/recursive-self-improvement)
- Anthropic uses "an automated Claude reviewer" before merge; retrospective analysis says it would have "caught roughly a third of the bugs behind past incidents." — [Anthropic Institute](https://www.anthropic.com/institute/recursive-self-improvement); [Claude blog: secure AI-native SDLC](https://claude.com/blog/how-anthropic-secures-its-ai-native-software-development-lifecycle)
- Secure SDLC post: multiple review agents per PR, each "designed and scoped to a specific, narrow focus"; "Human accountability is still central to our process"; "Tiering our codebase by risk"; "Entire codebases have strict human approval processes." Every automated approval is "logged with the signals and reasoning behind it, and a risk-weighted sample is reviewed by humans." Pre-merge also uses `/security-review` and SAST posted on PRs; post-deploy uses AI DAST in staging, pentests, bug bounty. — [Claude blog](https://claude.com/blog/how-anthropic-secures-its-ai-native-software-development-lifecycle)
- "How AI is transforming work at Anthropic," published 2025-12-02 (survey plus Claude Code data): Claude used in 59% of daily work (from 28% a year earlier); self-reported productivity +50%; 27% of Claude-assisted work would not otherwise have been done; merged PRs per engineer per day +67%; only 0-20% of work can be "fully delegated." — [Anthropic Research](https://www.anthropic.com/research/how-ai-is-transforming-work-at-anthropic)
- Same study: 8.6% of Claude Code tasks are "papercut fixes" such as refactoring for maintainability, work that previously was not worth doing by hand. Engineers keep high-level design and planning. Consecutive tool calls per task rose from 9.8 to 21.2 and human turns fell from 6.2 to 4.1. Concerns: skills atrophy and a "paradox of supervision" (supervising Claude needs the skills that over-reliance erodes). — [Anthropic Research](https://www.anthropic.com/research/how-ai-is-transforming-work-at-anthropic)
- "How Anthropic teams use Claude Code," published 2025-07-24: new hires read CLAUDE.md files to learn codebases; Claude is a "first stop" to find relevant files; teams run autonomous loops where Claude "writes the code for the new feature, runs tests, and iterates continuously"; Security Engineering moved toward test-driven development with Claude. — [Claude blog](https://claude.com/blog/how-anthropic-teams-use-claude-code)

### Inferences
- Anthropic's own answer to review load is "more AI review plus risk-tiered humans," not "humans review everything."
- The 8.6% papercut share is the closest Anthropic number to "Claude used to pay down debt."

### Gaps
- No Anthropic figure found on defect rates, rework, or code churn for Claude-authored code over time, beyond the quality-parity claim.
- The "slot machine" checkpoint-and-roll-back practice is in the teams case study PDF; my fetch summary did not surface the quote, so it is not cited here.

## 4. Code review with Claude

### Takeaway
Anthropic offers three layers: local `/code-review` (fresh subagent, correctness bugs) plus `/simplify` (reuse, simplification, efficiency, abstraction level) and `/security-review`; a managed multi-agent "Code Review" on GitHub PRs with a verification step to drop false positives; and Claude in your own CI through GitHub Actions or GitLab. Review rules are tuned with CLAUDE.md and a review-only REVIEW.md.

### Cited Findings
- Code Review launched 2026-03-09 per **third-party** reports — [InfoQ](https://www.infoq.com/news/2026/04/claude-code-review/). Anthropic's launch post: output per engineer grew "200%" a year, creating a review bottleneck; PRs with substantive review comments went from "16%" to "54%"; "less than 1% of findings are marked incorrect"; PRs over 1,000 lines get findings 84% of the time (avg 7.5 issues); under 50 lines 31% (avg 0.5). Reviews take about 20 minutes and cost $15-25. — [Claude blog: Code Review](https://claude.com/blog/code-review)
- How it works: several agents analyze the diff "in the context of your full codebase," each for a different class of issue; "a verification step checks candidates against actual code behavior to filter out false positives"; results are deduplicated, ranked by severity, posted inline. Severities: Important, Nit, Pre-existing. The check run is always neutral, so it never blocks merges by itself; you can gate in your own CI by parsing its output. — [Docs: Code Review](https://code.claude.com/docs/en/code-review)
- Default focus is correctness, "not formatting preferences or missing test coverage." — [Docs: Code Review](https://code.claude.com/docs/en/code-review)
- CLAUDE.md violations introduced by a PR are flagged as nits, and the check is two-way: if a PR makes a CLAUDE.md statement outdated, "Claude flags that the docs need updating too." — [Docs: Code Review](https://code.claude.com/docs/en/code-review)
- REVIEW.md tuning advice: redefine severity, cap nits ("report at most five nits"), skip generated code and anything CI already enforces, add repo-specific "Always check" rules, require "a `file:line` citation" before posting behavior claims, and for re-reviews "suppress new nits and post Important findings only" so a fix doesn't reach "round seven on style alone." "a long REVIEW.md dilutes the rules that matter most." — [Docs: Code Review](https://code.claude.com/docs/en/code-review)
- `/code-review` reviews the branch diff "in a fresh subagent," supports `--fix`, `--comment`, effort levels (low/medium = fewer, higher-confidence findings), and `ultra` (multi-agent cloud review). — [Docs: Code Review](https://code.claude.com/docs/en/code-review); [Docs: Commands](https://code.claude.com/docs/en/commands)
- `/simplify`: "Four review agents run in parallel, covering reuse of existing helpers, simplification, efficiency, and whether the change is at the right level of abstraction. The review doesn't look for correctness bugs." — [Docs: Commands](https://code.claude.com/docs/en/commands)
- `/security-review`: analyzes the branch diff against origin's default branch for "injection, auth issues, and data exposure." `/verify` builds and runs the app and observes the result "rather than relying on tests or type checks." — [Docs: Commands](https://code.claude.com/docs/en/commands)
- Self-hosted alternative: run Claude in your own CI via GitHub Actions or GitLab CI/CD. — [Docs: Code Review](https://code.claude.com/docs/en/code-review)

### Inferences
- Anthropic splits "is it correct?" (`/code-review`) from "is it duplicated or over-abstracted?" (`/simplify`). A team worried about duplication should run both; the bug reviewer alone will not catch a re-implemented helper.
- The "Pre-existing" severity lets review surface old debt without blaming the current PR.

### Gaps
- No Anthropic data found on how often `/simplify` findings are accepted, or on duplication rates before and after.

## 5. Duplication, single source of truth, refactoring legacy code, large migrations

### Takeaway
Anthropic has no standalone "avoid duplication" guide. The anti-duplication advice is spread across: point Claude at existing patterns and utilities, keep libraries to those already used, use LSP for exact references, and run `/simplify`'s reuse check. For legacy code and migrations, Anthropic recommends: front-load human effort on a rulebook and dependency map, have a "built-in referee" (tests or equivalence checks), migrate one component at a time, and fix the process that generates code rather than patching outputs.

### Cited Findings
- "How Claude Code works in large codebases," Anthropic Applied AI team, published 2026-05-14: CLAUDE.md hierarchy with a "root file for the big picture, subdirectory files for local conventions"; "Claude works best when it's scoped to the part of the codebase that's actually relevant"; exclude generated and third-party code; write light directory maps when layout is unconventional; per-subdirectory test and lint commands instead of full suites. — [Claude blog](https://claude.com/blog/how-claude-code-works-in-large-codebases-best-practices-and-where-to-start)
- Same post: LSP "returns only the references that point to the same symbol, so filtering happens before Claude reads anything"; stop hooks that propose CLAUDE.md updates while context is fresh; plugins so "every developer gets the same context"; name a "DRI: one person with ownership over Claude Code configuration"; review config "every three to six months" after major model releases. — [Claude blog](https://claude.com/blog/how-claude-code-works-in-large-codebases-best-practices-and-where-to-start)
- Reuse guidance in the best-practices page: follow an existing example file; "build from scratch without libraries other than the ones already used"; ask subagents for "existing ... utilities I should reuse." — [Best practices](https://code.claude.com/docs/en/best-practices)
- `/simplify` checks "reuse of existing helpers" and abstraction level. — [Docs: Commands](https://code.claude.com/docs/en/commands)
- Code Review flags when a PR makes CLAUDE.md out of date, which keeps the instruction file as a maintained source of truth. — [Docs: Code Review](https://code.claude.com/docs/en/code-review)
- "How Anthropic runs large-scale code migrations with Claude Code" (published 2026-07-16 per search-result metadata): six steps, namely a rulebook and dependency map, stress-test the rules, translate, compile, run tests, verify behavioral equivalence. Core idea: "you don't fix the code. You fix the process (loop) that produced the code." Needs "a built-in referee." "Front-load the human hours. The rulebook and the stress test are the most time-consuming." Smaller models for fan-out, larger for review and rule writing; independent reviewers with "disagreement between reviewers goes to a third agent." — [Claude blog](https://claude.com/blog/ai-code-migration)
- Bun Zig-to-Rust case: "A million lines of code were produced in less than two weeks"; 5.9B uncached input + 690M output tokens, about $165,000 at API pricing; Rust build 19% smaller, 2-5% faster. — [Claude blog](https://claude.com/blog/ai-code-migration). Search-result summaries also report 100% of Bun's test suite passing before merge and 19 regressions found after merge, all fixed; I did not confirm these two numbers on the primary page (**treat as unverified**) — [heise, third-party](https://www.heise.de/en/news/AI-Porting-Claude-Rewrites-Bun-Codebase-in-Rust-11294318.html).
- COBOL modernization post: automated discovery that traces "execution paths" and "data flows between modules"; flags "modules with high coupling" as risky and "isolated components" as early candidates; humans set priorities; work "one component at a time, with validation at each step"; "AI designs preliminary function tests that verify migrated code produces identical outputs to legacy COBOL." — [Claude blog](https://claude.com/blog/how-ai-helps-break-cost-barrier-cobol-modernization)
- Anthropic also publishes a "Code Modernization Playbook" PDF and a "Scaling agentic coding across your organization" PDF. — [Anthropic resources](https://resources.anthropic.com/hubfs/Code%20Modernization%20Playbook.pdf); [Anthropic resources](https://resources.anthropic.com/hubfs/Scaling%20agentic%20coding%20across%20your%20organization.pdf)

### Inferences
- "Fix the loop, not the code" is Anthropic's closest statement to a single-source-of-truth principle: when outputs are wrong, change the rulebook, prompt, CLAUDE.md, or hook that produced them, so the fix applies everywhere.
- Across COBOL, migration, and harness posts, the shared rule is the same: an executable equivalence or test oracle comes before any large change.

### Gaps
- I did not find an Anthropic page that names "duplication" or "DRY" as an explicit risk of AI-written code, or gives duplication metrics.
- The COBOL post's publication date was not shown in my fetch; it is believed to be early 2026 but is unconfirmed.
- I did not read the two PDFs.

## 6. Anthropic research on how developers use Claude for coding

### Takeaway
Anthropic's Economic Index found Claude Code use is mostly automation (79%) rather than augmentation, with heavy front-end/web work and startups over-represented. It did not break out refactoring or maintenance. Anthropic's internal study puts "papercut" quality fixes at 8.6% of Claude Code tasks.

### Cited Findings
- "Anthropic Economic Index: AI's impact on software development," published 2025-04-28: 79% of Claude Code conversations were automation vs 49% on Claude.ai. "Feedback Loop" (autonomous but human passes errors back) was 35.8% on Claude Code vs 21.3% on Claude.ai; "Directive" was 43.8% vs 27.5%. — [Anthropic Research](https://www.anthropic.com/research/impact-software-development)
- Same report: JS/TS 31%, HTML/CSS 28%, Python 14%, SQL 6%; top tasks UI/UX components (12%) and web/mobile app development (8%); startups 32.9% vs enterprise 23.8% of Claude Code work. — [Anthropic Research](https://www.anthropic.com/research/impact-software-development)
- That report gives no separate figure for refactoring, debugging, or maintenance versus new features. — [Anthropic Research](https://www.anthropic.com/research/impact-software-development)
- Internal study: 55% of engineers use Claude for debugging daily, 42% for code understanding, 37% for new features; 8.6% of Claude Code tasks are papercut fixes such as refactoring for maintainability. — [Anthropic Research](https://www.anthropic.com/research/how-ai-is-transforming-work-at-anthropic)
- Later Economic Index reports exist (September 2025 "Uneven AI adoption", March 2026 "Learning curves", June 2026 "Cadences"). — [Sep 2025](https://www.anthropic.com/research/anthropic-economic-index-september-2025-report); [Mar 2026](https://www.anthropic.com/research/economic-index-march-2026-report); [Jun 2026](https://www.anthropic.com/research/economic-index-june-2026-report)

### Inferences
- Debugging and code understanding are the most frequent internal uses, ahead of new features. That suggests Anthropic's own heavy use is on existing code.

### Gaps
- I did not read the 2026 Economic Index reports, so I can't say whether they add coding-specific maintenance or refactoring numbers.
- No Anthropic study found that measures long-term maintainability or technical debt of Claude-written code.
