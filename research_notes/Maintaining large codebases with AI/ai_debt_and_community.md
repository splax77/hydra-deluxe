# AI coding tools and technical debt: the evidence, and what teams do about it

Research date: 2026-10-05. Evidence type is labelled on each finding: **[RCT]** randomized controlled trial, **[peer-reviewed / arXiv]** academic (arXiv = preprint unless a venue is named), **[survey]** large practitioner survey, **[vendor]** report from a company that sells a related product, **[practitioner]** named expert opinion, **[anecdotal]** forum threads. GitHub star counts and last-push dates were pulled live from the GitHub API on 2026-10-05.

## Part 1, Q1: What do GitClear's reports (2024, 2025, 2026) say about duplication, copy/paste vs moved code, and churn?

### Takeaway
All three GitClear reports point the same way. Copy/pasted code goes up, "moved" code (GitClear's stand-in for refactoring) collapses, and short-term churn rises. By 2026 refactoring is under 4% of changed lines. GitClear sells code-analytics tools, and its data is correlational. It shows trends that coincide with AI adoption. It does not prove AI caused them.

### Cited Findings
- **2024 report, "Coding on Copilot" (published 2024-01-16) [vendor]:** about 153M changed lines from Jan 2020 to Dec 2023. "Added" and "copy/pasted" code grew relative to "updated," "deleted," and "moved" code. Code churn was projected to double in 2024 versus the 2021 pre-AI baseline. Churn here means lines reverted or rewritten within two weeks of being written. — [GitClear 2024](https://www.gitclear.com/coding_on_copilot_data_shows_ais_downward_pressure_on_code_quality); [Visual Studio Magazine, 2024-01-25](https://visualstudiomagazine.com/articles/2024/01/25/copilot-research.aspx)
- **2025 report, "AI Copilot Code Quality: Evaluating 2024's Increased Defect Rate" (Feb 2025) [vendor]:** 211M changed lines. Code blocks with 5 or more duplicated lines rose about 8x during 2024. Moved lines fell 39.9%. Moved/refactored code fell from about 25% of changed lines in 2021 to under 10% in 2024. 2024 was the first year on record where within-commit copy/paste exceeded moved code. — [GitClear 2025 PDF](https://gitclear-public.s3.us-west-2.amazonaws.com/GitClear-AI-Copilot-Code-Quality-2025.pdf); [DevClass, 2025-02-20](https://www.devclass.com/ai-ml/2025/02/20/ai-is-eroding-code-quality-states-new-in-depth-report/1626250); [jonas.rs summary](https://www.jonas.rs/2025/02/09/report-summary-gitclear-ai-code-quality-research-2025.html)
- **2026 report, "The Maintainability Gap" (Jan 2026) [vendor]:** 623M analyzed changes from 2023 to 2026, indexed to 2023 = 100. — [GitClear 2026](https://www.gitclear.com/the_ai_code_quality_maintainability_gap)
  - Block duplication rose 81%, from 40.3 to 73.0 duplicated lines per million changed lines.
  - Within-commit copy/paste rose 41%. It went from 9.4% of changes (2022) to 15.7% (2026).
  - Refactoring ("moved" code) fell from 21% (2022) to 3.8% (2026).
  - Function connectivity fell 35%, from 343 to 223 method calls per thousand changed lines. Read this as: new code calls existing code less and re-implements more.
  - Updates to legacy code fell 74%, from 1.7% to 0.46% of all changes.
  - Two-week churn rose 15%. "Error-masking constructs" rose 47%.
  - Heavy AI users out-produce non-users 4-10x, but most of that gap pre-dated AI. Compared with their own past output, heavy AI users gained about 25% velocity.
- The 2025 and 2026 reports state the refactoring baseline differently (about 25% in 2021 vs 21% in 2022) and measure duplication differently (8x increase in 5-line blocks in one year vs 81% rise in duplicated lines per million over three years). The direction agrees; the exact figures are not comparable across reports. — [GitClear 2025 PDF](https://gitclear-public.s3.us-west-2.amazonaws.com/GitClear-AI-Copilot-Code-Quality-2025.pdf); [GitClear 2026](https://www.gitclear.com/the_ai_code_quality_maintainability_gap)

### Inferences
- The most decision-relevant signal is the collapse in moved code and in function connectivity. Together they describe the "slop" pattern directly: new code gets pasted in beside old code instead of the old code being reused or reshaped.
- The heavy-user velocity gain (+25% versus self) is much smaller than the 4-10x headline. Reports that quote the headline without the self-comparison overstate the gain.

### Gaps
- GitClear's methodology for classifying "AI-heavy" developers and "error-masking constructs" was not inspected in detail. The 2026 page is a summary; the full PDF was not read.
- No independent replication of GitClear's moved/copy-paste classifier was found.

## Part 1, Q2: What do the DORA reports (2024, 2025) say about AI and delivery stability/quality?

### Takeaway
Both DORA reports find AI hurts delivery stability. In 2024, AI adoption also slightly hurt throughput. In 2025, throughput turned positive but instability persisted. DORA's framing is that AI is an amplifier. Teams with strong testing, version control, small batches and good platforms gain. Teams without them break more. DORA is a large survey (Google-funded), so its numbers are correlations, not experiments.

### Cited Findings
- **2024 Accelerate State of DevOps (Oct 2024) [survey]:** over 75% of respondents use AI daily. A 25% increase in AI adoption correlated with +7.5% documentation quality and +3.4% code quality, but -1.5% delivery throughput and -7.2% delivery stability. Only 39% expressed confidence in AI-generated output. — [RedMonk, 2024-11-26](https://redmonk.com/rstephens/2024/11/26/dora2024/); [The New Stack](https://thenewstack.io/dora-2024-ai-and-platform-engineering-fall-short/); [Google Cloud announcement](https://cloud.google.com/blog/products/devops-sre/announcing-the-2024-dora-report)
- **2025 State of AI-assisted Software Development (Sept 2025) [survey]:** AI adoption now correlates positively with throughput but still negatively with stability: more change failures, more rework, longer time to resolve issues. About 90% of developers use AI tools. — [InfoQ, Sept 2025](https://www.infoq.com/news/2025/09/dora-state-of-ai-in-dev-2025); [Google Cloud announcement](https://cloud.google.com/blog/products/ai-machine-learning/announcing-the-2025-dora-report); [Scrum.org summary](https://www.scrum.org/resources/blog/dora-report-2025-summary-state-ai-assisted-software-development)
- The 2025 report's mechanism: AI speeds up code production, and without robust controls (automated testing, mature version control, fast feedback loops) the extra change volume turns into instability. — [Scrum.org summary](https://www.scrum.org/resources/blog/dora-report-2025-summary-state-ai-assisted-software-development); [Honeycomb](https://www.honeycomb.io/blog/what-2025-dora-report-teaches-us-about-observability-platform-quality)
- **2025 DORA AI Capabilities Model [survey]:** seven capabilities showed a significant interaction with AI use: clear AI stance, healthy data ecosystems, AI-accessible internal data, strong version control practices, working in small batches, user-centric focus, and quality internal platforms. Commit frequency amplified AI's effect on individual effectiveness. Rollback capability amplified its effect on team performance. Small batches are measured by lines per change, changes per release, and time per task. — [DORA AI Capabilities Model PDF](https://services.google.com/fh/files/misc/2025_dora_ai_capabilities_model.pdf); [IT Revolution](https://itrevolution.com/articles/ais-mirror-effect-how-the-2025-dora-report-reveals-your-organizations-true-capabilities/)

### Inferences
- DORA's "small batches" and "version control/rollback" findings line up with what the skill collections in Part 2 enforce (bite-sized tasks, worktrees, frequent commits). That is the strongest bridge between the survey evidence and the practitioner tooling.

### Gaps
- No 2026 DORA report was found as of 2026-10-05. It may be due later in the year.
- Exact 2025 effect sizes (equivalent of the 2024 "-7.2% stability per 25% adoption") were not found in the sources read.

## Part 1, Q3: What did METR's 2025 RCT find, and what followed?

### Takeaway
METR's 2025 trial found experienced open-source developers were 19% slower with AI, while believing they were 20% faster. Its 2026 follow-up points toward a speed-up, but its confidence intervals include zero and METR itself calls the new data unreliable. The durable finding is the perception gap: developers misjudge how much AI helps them.

### Cited Findings
- **Original study (published 2025-07-10, data Feb-Jun 2025) [RCT]:** 16 experienced developers, 246 tasks in repositories they had worked on for about 5 years on average. AI use made them 19% slower. Before the tasks they forecast a 24% speed-up; afterwards they estimated a 20% speed-up. — [METR blog](https://metr.org/blog/2025-07-10-early-2025-ai-experienced-os-dev-study/); [arXiv 2507.09089](https://arxiv.org/pdf/2507.09089)
- **Follow-up (published 2026-02-24, data from Aug 2025) [RCT, flagged unreliable by its authors]:** 57 developers, 143 repositories, 800+ tasks. Returning developers: estimated 18% less time (CI -38% to +9%). New developers: 4% less time (CI -15% to +9%). Both intervals include zero. — [METR, "We are Changing our Developer Productivity Experiment Design"](https://metr.org/blog/2026-02-24-uplift-update/)
- METR says the new data is an unreliable signal because of selection effects. 30-50% of developers withheld tasks they did not want to do without AI. Pay also dropped from $150 to $50 per hour. METR still believes developers are likely more sped up in early 2026 than in early 2025, and it is changing its experiment design. — [METR uplift update](https://metr.org/blog/2026-02-24-uplift-update/); [devs-group summary](https://devs-group.ch/en/blog/ai-productivity-studies-2026/)
- METR now labels the 2025 result as historical, not a reflection of current tools. — [devs-group summary](https://devs-group.ch/en/blog/ai-productivity-studies-2026/)

### Inferences
- The METR studies measure speed, not code quality or debt. They do not tell us whether the faster 2026 work left more duplication behind.
- The refusal to do tasks without AI is itself a data point about dependence, separate from productivity.

### Gaps
- No METR result on code quality or maintainability of the AI-assisted output was found.

## Part 1, Q4: What do other studies show (controlled, academic, vendor)?

### Takeaway
Academic studies from 2025-2026 on real repositories consistently find more static-analysis warnings, more complexity, and long-lived issues in AI or agent code. Velocity gains fade, but quality costs persist. One controlled study shows agents struggle more when building on agent-written code. Vendor reports (CodeRabbit, Veracode, Sonar) point the same way, with larger and less carefully controlled numbers.

### Cited Findings

**Academic, observational (mining real repositories)**
- **Agarwal, He, Vasilescu (CMU), "AI IDEs or Autonomous Agents?" (arXiv 2601.13597, Jan 2026) [arXiv]:** staggered difference-in-differences with matched controls on the AIDev dataset. Agents gave large, front-loaded velocity gains only when they were the first AI tool a project adopted. Static-analysis warnings rose about 18% and cognitive complexity about 39%. The quality costs were sustained after the velocity gains faded. — [arXiv 2601.13597](https://arxiv.org/abs/2601.13597)
- **Liu, Widyasari, Zhao, Irsan, Chen, Lo, "Debt Behind the AI Boom" (arXiv 2603.28592, Mar 2026) [arXiv]:** 302.6k verified AI-authored commits from 6,299 GitHub repos across five assistants. 484,366 distinct issues found; 89.3% were code smells, the rest bugs and security issues. More than 15% of commits from every assistant introduced at least one issue. 22.7% of AI-introduced issues still survive at the latest repository version. Total surviving issues went from a few hundred in early 2025 to over 100k by Feb 2026. — [arXiv 2603.28592](https://arxiv.org/abs/2603.28592)
- **Xu, Medappa, Tunc, Vroegindeweij, Fransoo, "AI-Assisted Programming Decreases the Productivity of Experienced Developers by Increasing the Technical Debt and Maintenance Burden" (arXiv 2510.10165, Oct 2025, revised Jan 2026) [arXiv]:** after Copilot's introduction in OSS projects, peripheral developers became more productive. Core developers' original-code productivity fell 19% and they reviewed 6.5% more code. AI code needed more rework to meet repository standards. — [arXiv 2510.10165](https://arxiv.org/abs/2510.10165)
- **"To What Extent Does Agent-generated Code Require Maintenance?" (EASE 2026) [peer-reviewed]:** 3,238 commits from Copilot, Claude, Devin and Cursor (AIDev dataset). AI-generated files received significantly less maintenance than human files. — [arXiv 2605.06464](https://arxiv.org/pdf/2605.06464)
- **Ferdous et al., "Safer Builders, Risky Maintainers" (arXiv 2603.27524) [arXiv, via secondary summary]:** 7,191 agent PRs vs 1,402 human PRs. Agents caused fewer breaking changes in new code (3.45% vs 7.40%) but more in maintenance work. — [devs-group summary](https://devs-group.ch/en/blog/ai-productivity-studies-2026/), primary [arXiv 2603.27524](https://arxiv.org/abs/2603.27524) not read directly
- Related 2026 research finds developers keep AI code while openly unsure it is correct, termed GenAI-Induced Self-admitted Technical Debt (GIST). Developers most often describe postponed testing, incomplete adaptation and limited understanding. — [arXiv 2601.07786](https://www.arxiv.org/pdf/2601.07786) (seen in search snippet only)

**Academic, controlled / experimental**
- **Patel, Hou, Purohit, Xu, Pan, He, Chen, "Is Agent Code Less Maintainable Than Human Code?" (arXiv 2606.21804, Jun 2026, revised Sept 2026) [arXiv]:** four frontier agents on four benchmarks. Agents resolved up to 13.1% fewer tasks when building on agent-written code than on human-written code. Traditional maintainability metrics did not explain the gap. Subtler differences, such as changed input validation and error handling, did. — [arXiv 2606.21804](https://arxiv.org/abs/2606.21804)
- **Perry, Srivastava, Kumar, Boneh (Stanford), "Do Users Write More Insecure Code with AI Assistants?" (arXiv Nov 2022, CCS 2023) [peer-reviewed user study]:** participants with an AI assistant wrote more security vulnerabilities, notably in string encryption and SQL injection. They were also more likely to believe their code was secure. — [Slashdot summary](https://developers.slashdot.org/story/22/12/26/1336206/study-finds-ai-assistants-help-developers-produce-code-thats-more-likely-to-be-buggy)
- **Balepur et al., "(Im)Paired Programming" (arXiv 2607.26375) [arXiv, via secondary summary]:** 54 students building websites. Agents raised completion but hurt comprehension, especially with auto-accept on. — [devs-group summary](https://devs-group.ch/en/blog/ai-productivity-studies-2026/)
- A registered report accepted at ESEM 2026 (Coppola, Esposito, Kazman, Lenarduzzi, "AI Writes Code, Humans Pay the Debt") will compare agent implementations to real commits over 628k issue tickets. No results yet. — [arXiv 2609.04208](https://arxiv.org/abs/2609.04208)
- A 2026 multivocal literature review exists on technical debt in LLM-assisted development. — [arXiv 2606.14796](https://arxiv.org/pdf/2606.14796) (not read)

**Vendor and industry reports**
- **Uplevel (Sept 2024) [vendor]:** about 800 developers tracked 3 months before and after Copilot access. No meaningful change in PR cycle time or throughput; 41% more bugs. — [CO/AI](https://getcoai.com/news/new-study-raises-questions-about-true-impact-of-ai-coding-assistants/); a critique of the 41% figure: [Jason St-Cyr](https://jasonstcyr.com/2024/10/09/does-github-copilot-actually-raise-bugs-in-code-by-41/)
- **CodeRabbit, "State of AI vs Human Code Generation" (2025-12-17) [vendor, sells AI review]:** 470 open-source PRs. AI-co-authored PRs averaged 10.83 issues vs 6.45 for human-only, about 1.7x. Logic/correctness issues +75%; readability issues more than 3x; error-handling gaps nearly 2x; security issues up to 2.74x. Per 100 PRs, critical issues 240 to 341 (+40%) and major 257 to 447 (+70%). — [CodeRabbit report](https://www.coderabbit.ai/blog/state-of-ai-vs-human-code-generation-report); [Help Net Security](https://www.helpnetsecurity.com/2025/12/23/coderabbit-ai-assisted-pull-requests-report/)
- **Veracode, 2025 GenAI Code Security Report (July 2025, updated Oct 2025) [vendor]:** 80 CWE-targeted tasks given to 100+ LLMs in Java, JavaScript, Python and C#. 45% of outputs had a security flaw. Java exceeded 70%; the others ran 38-45%. Cross-site scripting failed 86% and log injection 88%. Newer and larger models were not more secure. — [Veracode report](https://www.veracode.com/resources/analyst-reports/2025-genai-code-security-report/); [Help Net Security, 2025-08-07](https://www.helpnetsecurity.com/2025/08/07/create-ai-code-security-risks/)
- **Sonar, "Coding Personalities of Leading LLMs" (Aug 2025) [vendor]:** thousands of Java tasks per model. Each model had a distinct defect profile. A large share of vulnerabilities were "blocker" severity: over 70% for Llama-3.2-vision:90b, 62.5% for GPT-4o, nearly 60% for Claude Sonnet 4. — [Sonar](https://www.sonarsource.com/the-coding-personalities-of-leading-llms/)
- **Sonar, 2026 State of Code Developer Survey [vendor survey]:** 1,100+ developers. AI is 42% of committed code, expected 65% by 2027. 96% do not fully trust AI code; only 48% always verify it before committing. — [Sonar survey PDF](https://www.sonarsource.com/state-of-code-developer-survey-report.pdf)

### Inferences
- The independent academic work (CMU, SMU/Lo group, Xu et al.) supports GitClear's direction with better causal design. The strongest single result is Agarwal et al.: velocity gains fade but complexity debt stays.
- Patel et al. matters most for agent-heavy teams: agent-written code makes the next agent task harder. Debt compounds for the agents, not just for humans.
- The pattern across Perry, METR and Sonar's survey is overconfidence: people trust AI output more than its measured quality warrants.

### Gaps
- No peer-reviewed study specifically on LLM code-clone rates (semantic duplication) in production repos was read. GitClear is the main duplication source.
- The original NYU "Asleep at the Keyboard" Copilot security study (Pearce et al.) was not re-verified this session.
- Several 2026 arXiv items (2603.27524, 2607.26375, 2601.07786, 2606.14796) were seen only via summaries or snippets.

## Part 2, Q5: Spec-driven development and context files (Spec Kit, Kiro, BMAD, Agent OS, OpenSpec, AGENTS.md, CLAUDE.md, Cursor rules)

### Takeaway
Spec-driven development (SDD) means writing a spec first and having the agent build from it. It has become the most-starred approach in this space. Böckeler's three-level framing (spec-first, spec-anchored, spec-as-source) is the standard way to tell the tools apart. The best controlled evidence on context files (AGENTS.md and similar) is sobering: they do not raise success rates on average and cost 20%+ more tokens. Specific instructions inside them are followed; repository overviews are not useful.

### Cited Findings
- Böckeler, "Understanding Spec-Driven Development: Kiro, spec-kit, and Tessl" (martinfowler.com, Oct 2025) [practitioner]. Three levels: spec-first (write a spec to drive one task), spec-anchored (keep the spec and evolve the feature through it), spec-as-source (the spec is the main artifact and code is generated). Kiro is the lightest, with a three-step workflow. Spec Kit is a CLI that sets up a workspace with heavy use of checklists. Tessl aims at spec-anchored and possibly spec-as-source. — [summary of Böckeler article](https://www.pelayoarbues.com/literature-notes/Articles/Understanding-Spec-Driven-Development-Kiro,-Spec-Kit,-and-Tessl); [alldevblogs mirror](https://www.alldevblogs.com/article/martin-fowler/understanding-spec-driven-development-kiro-spec-kit-and-tessl)
- Repo activity on 2026-10-05:
  - github/spec-kit: 140,269 stars, pushed 2026-10-05. — [GitHub](https://github.com/github/spec-kit)
  - Fission-AI/OpenSpec ("Spec-driven development for AI coding assistants"): 71,098 stars, pushed 2026-10-06 UTC. — [GitHub](https://github.com/Fission-AI/OpenSpec)
  - bmad-code-org/BMAD-METHOD ("Agile AI-Driven Development"): 53,813 stars, pushed 2026-10-06 UTC. — [GitHub](https://github.com/bmad-code-org/BMAD-METHOD)
  - buildermethods/agent-os ("injecting your codebase standards and writing better specs"): 5,472 stars, pushed 2026-08-29. — [GitHub](https://github.com/buildermethods/agent-os)
  - gotalab/cc-sdd ("turn approved specs into long-running autonomous implementation"): 3,701 stars, pushed 2026-09-23. — [GitHub](https://github.com/gotalab/cc-sdd)
  - agentsmd/agents.md ("a simple, open format for guiding coding agents"): 24,783 stars, pushed 2026-09-10. — [GitHub](https://github.com/agentsmd/agents.md)
  - PatrickJS/awesome-cursorrules: 40,881 stars, last pushed 2026-05-30 (four months stale). — [GitHub](https://github.com/PatrickJS/awesome-cursorrules)
- **Gloaguen, Mündler-Sasahara, Müller, Raychev, Vechev (ETH Zurich), "Evaluating AGENTS.md: Are Repository-Level Context Files Helpful for Coding Agents?" (arXiv 2602.11988, Feb 2026, final Sept 2026) [arXiv, controlled benchmark]:** tested on SWE-bench tasks with LLM-generated context files and on new issues with developer-written ones, across several models and agents. Context files did not generally improve success rates and raised inference cost by over 20% on average. Agents did follow specific instructions in them. Repository overviews, though recommended by model vendors, did not help. The result held for both LLM-generated and developer-written files. — [arXiv 2602.11988](https://arxiv.org/abs/2602.11988)
- Thoughtworks Radar Vol. 33 (Nov 2025) highlights the rapid evolution of AI assistance, and lists spec-driven development and AGENTS.md style practices among its AI entries. — [Thoughtworks Radar 33 news](https://www.thoughtworks.com/about-us/news/2025/thoughtworks-tech-radar-33-rapid-ai) (ring placement for SDD not verified)

### Inferences
- The ETH result suggests context files should hold short, specific, checkable rules, not codebase tours. That matches the anecdotal HN advice below (rules under 50 lines; specific "never do X" lines work, long architecture essays get ignored).
- SDD tools address a different failure from duplication: building the wrong thing. They do not by themselves stop pasted code. That still needs sensors (Q7).

### Gaps
- No controlled study comparing SDD (Spec Kit, Kiro, BMAD) against plain prompting on code-quality or debt outcomes was found.
- Kiro's star count is not applicable (AWS product, not an open repo). Tessl details beyond Böckeler's article were not checked.
- Böckeler's original martinfowler.com SDD article was read only via summaries; the URL on martinfowler.com was not fetched directly.

## Part 2, Q6: Skill and plugin collections for disciplined agent coding

### Takeaway
The two largest collections, obra/superpowers and Matt Pocock's skills, both enforce the same core loop: design or grill the requirements first, plan in small tasks, test-drive the code, have a fresh agent review it, and verify before claiming done. Pocock's collection adds explicit architecture discipline (deep modules, domain language, regular codebase surveys) aimed at entropy. Their popularity is huge, but no controlled evaluation of either was found.

### Cited Findings
- **obra/superpowers:** 295,675 stars, pushed 2026-09-27. Self-described "agentic skills framework and software development methodology." — [GitHub](https://github.com/obra/superpowers)
  - Enforces mandatory RED-GREEN-REFACTOR TDD, design before code (brainstorming), plans split into "bite-sized tasks (2-5 minutes each)," a 4-phase systematic debugging process, review checkpoints between tasks, isolated git worktrees, and fresh subagents per task.
  - Workflow: brainstorming, then using-git-worktrees, writing-plans, subagent-driven-development or executing-plans, test-driven-development, requesting-code-review, finishing-a-development-branch. Supporting skills include systematic-debugging and verification-before-completion.
- **mattpocock/skills ("Skills for Real Engineers"):** 277,135 stars, pushed 2026-10-05. — [GitHub](https://github.com/mattpocock/skills)
  - User-invoked skills: grill-with-docs (domain language), to-spec, tdd, implement, code-review (checks both standards and spec compliance), improve-codebase-architecture, wayfinder, retro, diagnosing-bugs.
  - Model-invoked skills: domain-modeling, codebase-design (deep modules with small interfaces), research, prototype.
  - Quotes Ousterhout ("The best modules are deep...") and Beck ("Invest in the design of the system every day") and says agents amplify software entropy, so architecture needs deliberate attention and frequent codebase surveys.
- **hesreallyhim/awesome-claude-code:** curated list, 55,114 stars, pushed 2026-10-06 UTC. — [GitHub](https://github.com/hesreallyhim/awesome-claude-code)
- **nizos/tdd-guard** ("Automated TDD enforcement for Claude Code"): 2,355 stars, pushed 2026-10-05. It uses hooks to block code written before a failing test. — [GitHub](https://github.com/nizos/tdd-guard)
- **pchalasani/claude-code-tools:** 2,008 stars, pushed 2026-10-05. — [GitHub](https://github.com/pchalasani/claude-code-tools)

### Inferences
- What these collections enforce maps closely onto DORA's amplifying capabilities (small batches, version control discipline) and Beck's warnings (test cheating, coding ahead). They are the community's operational answer to the evidence in Part 1.
- They mostly rely on "guides" (instructions the agent should follow). Given the ETH and HN evidence that instructions are often skipped, the pieces that use hooks (tdd-guard, deterministic checks) are likely more reliable than prose-only skills.

### Gaps
- No controlled or even systematic before/after study of superpowers, Pocock's skills, or similar collections was found.
- Star counts this large (roughly 280-300k) are unusual; they are reported as returned by the GitHub API and were not independently checked for inflation.

## Part 2, Q7: Guardrails: fitness functions, lint rules, duplicate detectors, hooks, CI gates, review bots, mutation testing

### Takeaway
The practitioner consensus, crystallized by Böckeler's "harness engineering" article (April 2026), is to pair "guides" that steer the agent before it acts with "sensors" that check its output afterwards. Deterministic sensors (linters, duplicate detectors, architecture tests, type checkers) reliably catch structural problems like duplication and drift. AI reviewers catch semantic problems but only probabilistically. Thoughtworks recommends architectural fitness functions and feedback sensors as explicit countermeasures to AI-driven "cognitive debt."

### Cited Findings
- **Böckeler, "Harness Engineering for Coding Agent Users" (martinfowler.com, 2026-04-02) [practitioner]:** — [martinfowler.com](https://martinfowler.com/articles/harness-engineering.html)
  - Guides are feedforward controls (AGENTS.md, skills, constraint docs). Sensors are feedback controls that let the agent self-correct.
  - Sensors work best when their output is written for an LLM, e.g. custom linter messages that include the correction instruction.
  - Computational controls (tests, linters, type checkers) are fast and deterministic. Inferential controls (AI review) are slower and probabilistic.
  - Computational sensors catch duplicate code, cyclomatic complexity, missing coverage, architectural drift and style violations "reliably." LLM sensors are needed for semantically duplicate code, redundant tests and brute-force fixes.
  - Recommends structural tests (ArchUnit) for module boundaries, mutation testing for test quality, and continuous drift detection (dead code, coverage quality).
  - Neither sensor type reliably catches the highest-impact problems: misdiagnosis and misunderstood requirements. Human judgment remains needed there.
- **Thoughtworks Technology Radar:**
  - "Complacency with AI-generated code": Hold, Nov 2025 (Vol. 33). — [Thoughtworks Radar techniques](https://www.thoughtworks.com/radar/techniques)
  - "Codebase cognitive debt": Caution, April 2026 (Vol. 34). Defined as the growing gap between a system's implementation and the team's shared understanding of it, worsened by multiple contributors and agent swarms. Countermeasures: feedback sensors for coding agents, tracking team cognitive load, and architectural fitness functions. Related: "Coding agent swarms" (Caution, Apr 2026), "Team cognitive load" (Adopt, Oct 2022), "Architectural fitness function" (Trial, May 2018). — [Thoughtworks Radar: codebase cognitive debt](https://www.thoughtworks.com/radar/techniques/codebase-cognitive-debt)
- Tool repos (GitHub API, 2026-10-05):
  - kucherenko/jscpd, copy/paste detector for 220+ languages with SARIF output, a GitHub Action and "MCP server for AI agents": 6,335 stars, pushed 2026-10-05. — [GitHub](https://github.com/kucherenko/jscpd)
  - sverweij/dependency-cruiser, dependency rule validation for JS/TS: 7,254 stars, pushed 2026-10-01. — [GitHub](https://github.com/sverweij/dependency-cruiser)
  - TNG/ArchUnit, architecture rules as Java unit tests: 3,852 stars, pushed 2026-10-05. — [GitHub](https://github.com/TNG/ArchUnit)
  - semgrep/semgrep, pattern-based static analysis with custom rules: 16,888 stars, pushed 2026-10-06 UTC. — [GitHub](https://github.com/semgrep/semgrep)
  - stryker-mutator/stryker-js, mutation testing: 3,171 stars, pushed 2026-10-04. — [GitHub](https://github.com/stryker-mutator/stryker-js)
- Sonar's position: LLM code quality and security risks need independent static analysis to detect reliably ("vibe, then verify"). — [Sonar](https://www.sonarsource.com/blog/how-to-navigate-the-risks-of-ai-generated-code/) [vendor]
- Hooks give deterministic control: they ensure an action always happens instead of relying on the model to choose it. HN examples include quality validation, duplicate-prevention via indexers, and pre-commit lint/test checks. — [HN: "Claude Hooks: 6 hooks..."](https://news.ycombinator.com/item?id=44477756) [anecdotal]

### Inferences
- GitClear's two worst trends (copy/paste up, moved code down) are exactly what a duplicate detector (jscpd, PMD CPD) and an architecture test (ArchUnit, dependency-cruiser) catch. These are cheap, deterministic, and can be wired into a hook so the agent sees the failure before a human does.
- Mutation testing addresses Beck's "test cheating" failure: an agent that weakens tests leaves surviving mutants.

### Gaps
- PMD CPD star counts and activity were not checked.
- No controlled study measuring the effect of adding duplicate detectors or fitness functions specifically to AI-assisted workflows was found. The evidence is practitioner reasoning plus DORA's correlation with strong testing and platforms.
- The market and evidence for AI review bots (CodeRabbit, Copilot review, etc.) as a debt control was not separately researched; CodeRabbit's own report is the only data point and it has a commercial interest.

## Part 2, Q8: What do respected practitioners say?

### Takeaway
The respected voices converge: the AI writes code, but humans must keep caring about the code itself. That means tests, small reviewable changes, and active design. Beck calls it "augmented coding" versus "vibe coding." Willison calls it "vibe engineering." Osmani names the last 30% (edge cases, security, integration) as the part AI does not do. Böckeler supplies the tooling model (guides and sensors).

### Cited Findings
- **Kent Beck, "Augmented Coding: Beyond the Vibes" (newsletter, June 2025) [practitioner]:** in vibe coding you care only about behavior; in augmented coding you care about the code, its complexity, the tests and their coverage, the same values as hand coding. Building a B+ tree in Rust and Python over about four weeks, his first two attempts piled up so much complexity that the agent stalled. He then intervened more on design and stopped the agent from coding ahead. Warning signs: loops, features nobody asked for, and cheating on tests (disabling or deleting them). — [Kent Beck newsletter](https://newsletter.kentbeck.com/p/augmented-coding-beyond-the-vibes)
- **Simon Willison, "Vibe engineering" (simonwillison.net, 2025-10-07) [practitioner]:** proposes vibe engineering as the disciplined counterpart to vibe coding. LLMs reward existing top-tier practices: with a robust, stable test suite, agents "can fly." Practices: automated tests, red/green TDD, planning, documentation, version control, code review, real manual testing, and small reviewable changes. — [simonwillison.net](https://simonwillison.net/2025/Oct/7/vibe-engineering/)
- **Addy Osmani, "The 70% problem: hard truths about AI-assisted coding" (Dec 2024) [practitioner]:** AI gets you about 70% of the way (fine for prototypes and MVPs). The final 30% (edge cases, security, production integration) needs significant human work and is as hard as ever. Experienced engineers use their judgment to shape and constrain AI output; that expertise keeps the code maintainable. — [Zed blog, Osmani](https://zed.dev/blog/ai-70-problem-addy-osmani); [Pragmatic Engineer](https://newsletter.pragmaticengineer.com/p/how-ai-will-change-software-engineering)
- **Birgitta Böckeler (Thoughtworks), martinfowler.com:** SDD levels (Oct 2025) and harness engineering (Apr 2026), see Q5 and Q7. — [martinfowler.com harness engineering](https://martinfowler.com/articles/harness-engineering.html)
- **Thoughtworks Radar:** complacency with AI code on Hold; codebase cognitive debt on Caution. See Q7. — [Thoughtworks Radar](https://www.thoughtworks.com/radar/techniques/codebase-cognitive-debt)

### Inferences
- All four frame the problem the same way: AI amplifies whatever discipline already exists. That matches DORA's "amplifier" conclusion.

### Gaps
- Steve Yegge's writing (e.g. on "vibe coding" and his agent-orchestration tooling) was not researched this session.
- Martin Fowler's own pieces on LLMs and non-determinism were not checked.

## Part 2, Q9: Common failure modes and which fixes people say actually work

### Takeaway
The failure modes practitioners report match the data. Agents re-implement things that already exist, ignore or skim instructions, weaken tests, wander off task, and leave code their maintainers do not understand. The fixes people consistently say work are deterministic: a test or checker the agent must pass, run on every change via hooks, plus short specific rules and small reviewed changes. Long prose instructions are the fix people most often say fails.

### Cited Findings
- **Re-implementing instead of reusing [anecdotal]:** one HN user said Claude built four separate registries when told to use one unified pattern, which took "half a day of shouting" and manual edits to fix. — [HN: "How Claude Code works in large codebases" (~mid-2026)](https://news.ycombinator.com/item?id=48144494). This matches GitClear's falling function connectivity and rising duplication. — [GitClear 2026](https://www.gitclear.com/the_ai_code_quality_maintainability_gap)
- **Instructions ignored [anecdotal]:** users report constraint files and skills being ignored; one claimed instructions were "forgotten" about 90% of the time. Short, specific rules (under 50 lines, e.g. "never create a user without calling the workspace provision step") worked better than long architecture explanations. — [HN thread](https://news.ycombinator.com/item?id=48144494). Consistent with ETH's finding that specific instructions are followed but overviews do not help. — [arXiv 2602.11988](https://arxiv.org/abs/2602.11988)
- **Skimming files ("peephole reading") and token waste [anecdotal]:** reading only the start of files, re-running failing tests instead of reading output once. — [HN thread](https://news.ycombinator.com/item?id=48144494)
- **Verifier signals work [anecdotal]:** with a pass/fail test signal, LLMs become "extremely reliable." Hooks running linters and checkers that feed back into the context window are reported as effective. — [HN thread](https://news.ycombinator.com/item?id=48144494)
- **Scoped fast feedback [anecdotal]:** in monorepos, the fastest loop is the test command for the slice being edited; put scoped test/lint commands in per-directory CLAUDE.md files. — [claudefa.st playbook](https://claudefa.st/blog/guide/development/large-codebase-playbook)
- **Test cheating, loops, unrequested features [practitioner]:** Beck's three warning signs. — [Kent Beck](https://newsletter.kentbeck.com/p/augmented-coding-beyond-the-vibes)
- **Even AI-tooling code suffers duplication [anecdotal]:** an issue in a public dotfiles repo found its Claude Code hook layer had 4 JSON escapers, 5 lock implementations and 12 test helpers with incompatible argument orders. — [mark-brannan/dotfiles #517](https://github.com/mark-brannan/dotfiles/issues/517)
- **Comprehension loss [arXiv, secondary]:** agents with auto-accept hurt understanding. — [devs-group summary of Balepur et al.](https://devs-group.ch/en/blog/ai-productivity-studies-2026/). Thoughtworks names this "codebase cognitive debt." — [Thoughtworks Radar](https://www.thoughtworks.com/radar/techniques/codebase-cognitive-debt)
- **Maintenance burden shifts to experts [arXiv]:** core developers review 6.5% more and write 19% less original code after Copilot. — [arXiv 2510.10165](https://arxiv.org/abs/2510.10165)
- **Verification gap [vendor survey]:** only 48% of developers always verify AI code before committing. — [Sonar 2026 survey](https://www.sonarsource.com/state-of-code-developer-survey-report.pdf)

### Inferences
- Ranked by how much evidence supports them, the fixes are roughly: (1) deterministic sensors in the loop (tests, duplicate detectors, architecture rules, linters) wired so the agent cannot skip them; (2) small batches with strong version control and rollback (DORA); (3) short, specific, checkable rules rather than long context files (ETH + HN); (4) plan/spec first to avoid building the wrong thing (Böckeler, SDD tools, weaker evidence); (5) AI review as a secondary, probabilistic net.
- The Patel et al. result (agents do worse on agent-written code) implies that cleaning up duplication is not only for humans. It directly protects future agent performance.

### Gaps
- Reddit r/ExperiencedDevs threads were not surfaced by search; practitioner sentiment here rests on Hacker News and blogs.
- No quantitative data on how often specific fixes (hooks, duplicate gates) reduce debt in practice was found. The recommendations rest on expert reasoning and correlational surveys.
