# How Google keeps a very large codebase healthy (and what changes with AI-written code)

Notes current as of 2026-10-05. Every dated number carries its year. "SWE book" means *Software Engineering at Google* (Winters, Manshreck, Wright; O'Reilly, 2020), which Google hosts free at abseil.io. Its figures describe Google around 2019–2020.

## 1. Monorepo (Piper), trunk-based development, and the one-version rule: what Google publishes and why it says this cuts debt

### Takeaway
Google keeps nearly all its code in one repository, where everyone works on one shared copy ("trunk") and only one version of each library exists. Google's stated reason is that whoever changes a library must fix every caller right away, so the debt of "callers stuck on old versions" never piles up.

### Cited Findings
- Scale in January 2015 (published 2016): about 1 billion files, 9 million source files, 2 billion lines of code, 35 million commits of history, 86 TB, and 40,000 commits per workday. — [Potvin & Levenberg, CACM 59(7), July 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/) (text read via [mirror PDF](https://bowringj.people.charleston.edu/classes/csci%20362/docs/GoogleCodeRepo-78-potvin.pdf))
- Of those daily commits, more than 25,000 developers made about 16,000 and automated systems made about 24,000. The paper says commit growth after 2012 "continues primarily due to automation." — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- In 2014 about 15 million lines changed in about 250,000 files every week (human-reviewed code only). — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- The repository was used by 95% of Google's developers, and over 99% of its files were visible to all full-time engineers. — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Later figures (SWE book, 2020): Piper holds 80+ TB, takes 60,000–70,000 commits per workday, and has about 50,000 engineers editing it. — [SWE book ch. 16, Version Control](https://abseil.io/resources/swe-book/html/ch16.html)
- Trunk-based development: "The vast majority of Piper users work at the 'head'… Immediately after any commit, the new code is visible to, and usable by, all other developers." Development branches are "unusual and not well supported," and release branches take only cherry-picks from trunk. — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Feature flags take the place of feature branches, and "eventually flags are retired so old code can be deleted." — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Out of roughly 1,000 teams in the monorepo, "only a couple" have a development branch. The book says such branches should be "rare, and should be understood to be expensive." — [SWE book ch. 16](https://abseil.io/resources/swe-book/html/ch16.html)
- The one-version rule: "Developers must never have a choice of 'What version of this component should I depend upon?'" A monorepo makes the rule easy to keep: "it's usually more difficult to violate One-Version than it would be to do the right thing." — [SWE book ch. 16](https://abseil.io/resources/swe-book/html/ch16.html)
- The debt argument in Google's own words: in open source, "delays in updating create technical debt that can become very expensive." In a monorepo, "the person updating a library [updates] all affected dependencies at the same time. The technical debt incurred by dependent systems is paid down immediately as changes are made." — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- The diamond dependency problem: A needs B and C, but B needs D v1 and C needs D v2. The monorepo avoids it at the source level, and static linking avoids it for binaries. — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Workarounds for multiple versions, such as shading, are called "entirely lost labor… just working around technical debts." — [SWE book ch. 16](https://abseil.io/resources/swe-book/html/ch16.html)
- Google also cites DORA's "predictive relationship between trunk-based development and high-performing software organizations." — [SWE book ch. 16](https://abseil.io/resources/swe-book/html/ch16.html)
- Measured benefit from central maintenance: the compiler team tunes defaults using nightly builds of the whole codebase. From 2014 to 2015 this cut Java garbage-collection CPU by more than 50% and GC pause time by 10–40%. The team ships 20+ C++ compiler releases a year. — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Admitted costs: (1) tooling investment, (2) codebase complexity, such as unneeded dependencies and harder code discovery, and (3) ongoing code-health effort. "It is common for teams to not think about their dependency graph." The paper also notes that abandoned projects left in the repo "continue to be updated and maintained." — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Fix for dependency sprawl: since 2011, new APIs default to "private" visibility. The paper's lesson is that such mechanisms "should be put in place as soon as possible." — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- A side effect of everyone reading everyone's code: teams skip writing docs, users come to depend on implementation details, and those details then become hard to deprecate. — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Google kept Piper rather than move to Git, because Git would have meant splitting into thousands of repos. For comparison, Android was already split into 800+ Git repos (2016). — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)

### Inferences
- In Google's framing, the monorepo is not the point. The point is a single version of everything plus the duty to update all callers. The SWE book says a "virtual monorepo" across many repos could achieve the same thing ([ch. 16](https://abseil.io/resources/swe-book/html/ch16.html)).
- The growth in automated commits (24k vs 16k per day in 2015) shows that keeping the code healthy at this scale already depended on machine-made changes long before LLMs.

### Gaps
- I found no published Piper size or commit-rate figures newer than the 2020 SWE book.
- I found no controlled measurement showing the one-version rule reduces debt. The case is argued, not measured.

## 2. Code review: readability, OWNERS, small CLs, the Code Health group, and the eng-practices guide

### Takeaway
At Google every change is reviewed before it lands. Each change needs three kinds of approval: one for correctness, one from a code owner, and one for language "readability". Reviews are small and fast: a median of 24 lines changed and under 4 hours end to end (2018 data). The written standard says to approve any change that clearly improves code health, even if it isn't perfect.

### Cited Findings
- The three approvals are: an LGTM ("looks good to me") for correctness and comprehension; owner approval from the directory's OWNERS file; and readability approval "from someone with language 'readability'" confirming the code follows that language's style and best practices. "Most reviews have one person assuming all three roles." — [SWE book ch. 9, Code Review](https://abseil.io/resources/swe-book/html/ch09.html)
- OWNERS files are "hierarchically additive": a file is owned by everyone listed in OWNERS files above it in the tree. — [SWE book ch. 9](https://abseil.io/resources/swe-book/html/ch09.html)
- "Every directory has a set of owners who control whether a change to files in their directory will be accepted." — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Size and speed norms (2020): "small" changes are about 200 lines or fewer; about 35% of changes touch a single file; most reviews have exactly one reviewer; most changes are reviewed within about a day. — [SWE book ch. 9](https://abseil.io/resources/swe-book/html/ch09.html)
- The 2018 case study looked at about 9 million reviewed changes over two years, made by 25,000+ authors and reviewers. — [Sadowski et al., "Modern Code Review: A Case Study at Google," ICSE-SEIP 2018](https://sback.it/publications/icse2018seip.pdf) ([abstract](https://research.google/pubs/modern-code-review-a-case-study-at-google/))
- Findings from that 2018 study:
  - The median time to first feedback is under an hour for small changes and about 5 hours for very large ones.
  - The median for the whole review is under 4 hours. Comparable published figures elsewhere are 14.7–19.8 hours at Microsoft projects, 17.5 at AMD and 15.7 at Chrome OS, with another Microsoft study reporting 24 hours.
  - Over 35% of changes modify one file and about 90% modify fewer than 10 files.
  - Over 10% of changes modify a single line, and the median is 24 lines.
  - Over 80% of changes involve at most one reviewer.
  - The median developer authors about 3 changes a week and reviews about 4.
  — [ICSE-SEIP 2018](https://sback.it/publications/icse2018seip.pdf)
- The same study found that Google's main motivations for review are education, maintaining norms, gatekeeping and accident prevention. Finding bugs is not the primary one. — [ICSE-SEIP 2018](https://sback.it/publications/icse2018seip.pdf)
- The eng-practices guide (public, from about 2019) sets the senior principle: "reviewers should favor approving a CL once it is in a state where it definitely improves the overall code health of the system… even if the CL isn't perfect." It adds: "There is no such thing as 'perfect' code—there is only better code." — [google.github.io/eng-practices, The Standard of Code Review](https://google.github.io/eng-practices/review/reviewer/standard.html)
- The guide names the mechanism of decay: "Codebases degrade through small decreases in code health over time, especially when a team is under significant time constraints." — [eng-practices, Standard](https://google.github.io/eng-practices/review/reviewer/standard.html)
- On speed: "One business day is the maximum time it should take to respond to a code review request." Slow reviews push reviewers to approve mediocre code and discourage refactoring. Large CLs should be split. — [eng-practices, Speed of Code Reviews](https://google.github.io/eng-practices/review/reviewer/speed.html)
- Automation inside review: Tricorder static analysis and presubmit results show up directly in Critique (Google's review tool), with one-click suggested fixes. Owners can add their own presubmit checks for their directories. — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/); [SWE book ch. 19, Critique](https://abseil.io/resources/swe-book/html/ch19.html)
- The Code Health group (2017 post): "At Google, Code Health is an official group that is recognized by the company." Its work covers refactoring, simplicity and practices, and contributors are recognized and rewarded. It publishes the "Code Health" series on the Google Testing Blog, which grew out of the "Testing on the Toilet" flyers. — [Google Testing Blog, Max Kanat-Alexander, 3 Apr 2017](https://testing.googleblog.com/2017/04/code-health-googles-internal-code.html)

### Inferences
- Google treats review mainly as a way to spread knowledge and keep norms consistent, not as a bug filter. That explains why readability, a style-and-idiom certification, is a separate approval.
- Small CLs plus a one-day response limit are what keep the latency low. The guide links slow reviews directly to worse code health.

### Gaps
- I did not find a primary source on how readability certification is earned today (number of reviewed changes, duration) or its measured effect. The 2018 study mentions readability reviewers but the fetched text gave no figures.
- I found no review latency numbers newer than 2018.

## 3. Large-scale changes (LSCs): Rosie, ClangMR, the LSC process, automated refactoring, and deprecation

### Takeaway
Google treats sweeping codebase-wide fixes as routine and gives them their own tools. Rosie splits one huge patch into per-owner pieces, tests each piece, and routes it for review. A small committee decides which sweeps are worth reviewers' time. By 2020, 10–20% of changes in some projects came from these sweeps. Deprecation is treated as real work with owners and deadlines, because "code is a liability, not an asset."

### Cited Findings
- An LSC is "any set of changes that are logically related but cannot practically be submitted as a single atomic unit." — [SWE book ch. 22, Large-Scale Changes](https://abseil.io/resources/swe-book/html/ch22.html)
- Rosie (2016 description): a developer makes one big patch, by find-and-replace or with refactoring tools. Rosie then splits it along directory and ownership lines, tests each piece, sends it for review, and commits it automatically once tests and review pass. — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Cleanups run in two phases. First comes a large backward-compatible change. Then a small change removes the old pattern. — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Governance: "In 2013, Google adopted a formal large-scale change-review process that led to a decrease in the number of commits through Rosie from 2013 to 2014." The committee weighs benefit against "the costs of reviewer time and repository churn." — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Throughput (2020): at the peak of the `scoped_ptr` → `std::unique_ptr` migration, "more than 700 independent changes, touching more than 15,000 files per day." Throughput at the time of writing was described as "10 times that." LSCs made up a double-digit share (10–20%) of changes in some projects. One LSC removed "more than one billion lines of code" in three days. — [SWE book ch. 22](https://abseil.io/resources/swe-book/html/ch22.html)
- Global approvers are engineers with repository-wide approval rights for a given language or library. They use pattern-based tools to auto-approve changes that match and look by hand only at anomalies. — [SWE book ch. 22](https://abseil.io/resources/swe-book/html/ch22.html)
- Testing: TAP, Google's test platform, batches LSC shards into an "uber-change" roughly every three hours and runs the affected tests. — [SWE book ch. 22](https://abseil.io/resources/swe-book/html/ch22.html)
- Tooling: ClangMR, Refaster and Kythe use the single global view of the code to make semantic edits. "Old APIs can be removed with confidence, because it can be proven that all callers have been migrated." — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/); [SWE book ch. 22](https://abseil.io/resources/swe-book/html/ch22.html)
- Paradox noted: as a codebase grows, "the largest atomic change possible counterintuitively decreases." LSC tooling exists to prevent "haunted graveyards," systems nobody dares to touch. — [SWE book ch. 22](https://abseil.io/resources/swe-book/html/ch22.html)
- Central modernization efforts "can touch half a million variable declarations or function-call sites spread across hundreds of thousands of files." — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Automated code-health tooling: dead-code detection and removal, and the Clipper tool, which builds a reachability graph to find unused Java classes and underused dependencies. — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Deprecation policy (2020): "code is a liability, not an asset."
  - Advisory deprecation, meaning a warning with no deadline, rarely moves users. "Hope is not a strategy."
  - Compulsory deprecation needs a deadline and a funded team doing the migration. Otherwise it is an "unfunded mandate."
  - Warnings must be actionable and relevant, to avoid alert fatigue.
  - Backsliding is blocked with compiler annotations and visibility allowlists.
  - Deprecations need named owners.
  — [SWE book ch. 15, Deprecation](https://abseil.io/resources/swe-book/html/ch15.html)
- Hyrum's Law is the reason deprecation is hard: the more users a system has, the more of them rely on unintended behavior. — [SWE book ch. 15](https://abseil.io/resources/swe-book/html/ch15.html)

### Inferences
- The human cost of reviewing is the limit on LSCs, not compute. Google's 2013 committee, its global approvers, and later its LLM migration work all aim to save reviewer attention.
- The one-version rule and LSCs depend on each other. One version only works if someone can update every caller, and LSC tooling is what makes that possible.

### Gaps
- I found no published Rosie or LSC volume numbers after 2020.

## 4. Google's published view of technical debt

### Takeaway
Google's research says tech debt is as much a human problem as a code problem. In about five years of study, no code metric reliably predicted where engineers said debt was hurting them, so Google measures it mainly by asking engineers. Separately, Google's 2015 machine-learning paper argues that ML systems pile up debt faster than normal code.

### Cited Findings
- Jaspan & Green, "Defining, Measuring, and Managing Technical Debt," *IEEE Software* 40(3):15–19, May 2023. The abstract calls tech debt "a prime example of an entangled human and technical problem." — [research.google](https://research.google/pubs/defining-measuring-and-managing-technical-debt/); [IEEE/ACM DL](https://dl.acm.org/doi/abs/10.1109/MS.2023.3242137)
- The 10 categories of debt engineers report, in order of how often they hinder work:
  1. Migration needed or in progress
  2. Documentation
  3. Testing
  4. Code quality
  5. Dead or abandoned code
  6. Code degradation
  7. Missing team expertise
  8. Dependencies
  9. Migration poorly executed or abandoned
  10. Release process
  — [DX newsletter summary of Jaspan & Green 2023](https://newsletter.getdx.com/p/measuring-and-managing-tech-debt) (secondary; the order should be checked against the paper)
- Measurement: Google tested "117 metrics" against engineers' survey reports using linear regression. "No single metric predicted reports of technical debt from engineers." It relies on quarterly engineering-survey questions instead, such as how much debt was taken on deliberately, whether that was the right call, and how much was invested in paying it down. — [DX summary](https://newsletter.getdx.com/p/measuring-and-managing-tech-debt) (secondary)
- Management steps reported: a debt-management framework, a four-level maturity model with an assessment tool, classes and self-guided courses, forums, a talk series, and tooling for debt indicators. — [DX summary](https://newsletter.getdx.com/p/measuring-and-managing-tech-debt) (secondary)
- Google's 2025 AI-migration paper describes unfinished migrations directly as debt. Old JUnit3 tests "are technical debt and tend to replicate themselves, as developers might inadvertently copy old code." It adds: "Unfinished migrations tend to continuously slow down teams even if they are not working directly on them, they confuse new developers with obsolete patterns." — [Nikolov et al., "How is Google using AI for internal code migrations?" arXiv 2501.06972, Jan 2025](https://arxiv.org/abs/2501.06972)
- Sculley et al., "Hidden Technical Debt in Machine Learning Systems," NIPS 2015 (Google authors): "it is dangerous to think of these quick wins as coming for free… it is common to incur massive ongoing maintenance costs in real-world ML systems." The risk factors it names are boundary erosion, entanglement, hidden feedback loops, undeclared consumers, data dependencies, configuration issues, changes in the external world, and system-level anti-patterns. — [NeurIPS proceedings](https://papers.nips.cc/paper_files/paper/2015/hash/86df7dcfd896fcaf2674f757a2463eba-Abstract.html); [research.google](https://research.google/pubs/hidden-technical-debt-in-machine-learning-systems/)
- SWE book framing (2020): deprecation exists because "code is a liability, not an asset" ([ch. 15](https://abseil.io/resources/swe-book/html/ch15.html)). Multi-version workarounds are "working around technical debts" ([ch. 16](https://abseil.io/resources/swe-book/html/ch16.html)).

### Inferences
- "Migration needed or in progress" tops Google's debt list. That explains why Google aimed its first bespoke LLM tools at migrations (section 6).
- Because code metrics failed to predict debt, any AI-era "debt score" built only from static code metrics would cut against Google's own findings.

### Gaps
- I could not read the full IEEE Software text, which is paywalled. The categories, the 117-metric result and the maturity-model details come from a secondary summary (DX). The names of the four maturity levels were not confirmed.
- I found no Google figure for the share of engineers hindered by debt in a given quarter.

## 5. "Single source of truth": Blaze/Bazel, protobuf, generated code, one version

### Takeaway
Google applies "one source of truth" at every layer. There is one repository and one version of each library. BUILD files declare every dependency and decide how code is built. Third-party code is pinned to a single checked-in version. Shared data formats are defined once, as protocol buffers.

### Cited Findings
- "A single repository provides unified versioning and a single source of truth. There is no confusion about which repository hosts the authoritative version of a file." — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Blaze/Bazel is artifact-based. Engineers declare *what* to build in BUILD files, and the system decides *how*. "Reducing the flexibility exposed to the programmer, the build system can know more about what is being done at every step." — [SWE book ch. 18, Build Systems](https://abseil.io/resources/swe-book/html/ch18.html)
- Bazel sandboxes each action so builds are hermetic. It enforces strict dependency declaration, so a target must name all its direct dependencies. Targets are private by default. Google uses fine-grained targets: a typical production binary depends on "tens of thousands of targets," and Java follows a "1:1:1 rule" of one package, one target and one BUILD file per directory. — [SWE book ch. 18](https://abseil.io/resources/swe-book/html/ch18.html)
- External dependencies are pinned in source control under a single version: "Relying on 'latest' versions is a recipe for disaster and unreproducible builds." — [SWE book ch. 18](https://abseil.io/resources/swe-book/html/ch18.html)
- Open-source code lives in a reserved area of the repo, and "only one version of an open source project be available at any given time." Teams must upgrade when the library is upgraded. — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Build scale (2020): "millions of builds executing millions of test cases and producing petabytes of build outputs from billions of lines of source code every day." 83% of Googlers were satisfied with the build system. — [SWE book ch. 18](https://abseil.io/resources/swe-book/html/ch18.html)
- Changing a dependency triggers a rebuild of everything that depends on it, and widespread breakage is rolled back automatically. — [CACM 2016](https://cacm.acm.org/research/why-google-stores-billions-of-lines-of-code-in-a-single-repository/)
- Protobuf as the shared schema: Google's AI-migration work found the code to change by starting from "one or more fields defined in a protocol buffer — a standard mechanism for serializing structured data." This was the 32-bit to 64-bit ID migration. — [arXiv 2501.06972, 2025](https://arxiv.org/abs/2501.06972)

### Inferences
- Bazel's declared dependency graph is what makes LSCs and test selection safe. You can only "prove all callers have been migrated" if every dependency is declared.

### Gaps
- I did not find a Google primary source that frames protobuf as "define a schema once, generate code for every language" as a debt-control policy. I also found no Google policy document on whether generated code is checked in or generated at build time. Both are left unverified.

## 6. AI-assisted code at scale (2023–2026)

### Takeaway
Google's share of AI-written code rose fast. AI completed about half of code characters by mid-2024. By April 2026 Google's CEO said 75% of all new code is AI-generated and approved by engineers. Google's own papers say AI works best on the oldest kind of debt, unfinished migrations, where it roughly halves the time. Human review is now the bottleneck. The DORA research Google sponsors finds that more AI use goes with more delivery instability unless the basics are in place.

### Cited Findings
**How much code AI writes at Google (note that the measures differ)**
- June 2024: "50% of code characters" were completed with AI help, with a 37% acceptance rate. ML now addresses ">8%" of code-review comments. "Smart paste" accounts for about 2% of code in the IDE. — [Google Research blog, Chandra & Tabachnyk, 6 Jun 2024](https://research.google/blog/ai-in-software-engineering-at-google-progress-and-the-path-ahead/)
- January 2025: a 38% acceptance rate "assisting in the completion of 67% of code characters." This is defined as accepted AI characters divided by (typed characters + accepted AI characters). — [arXiv 2501.06972, Jan 2025](https://arxiv.org/abs/2501.06972)
- 22 Apr 2026, Pichai at Cloud Next: "75% of all new code at Google is now AI-generated and approved by engineers, up from 50% last fall." Engineers are "orchestrating fully autonomous digital task forces." One complex migration "done by agents and engineers working together was completed six times faster than was possible a year ago." — [blog.google, Cloud Next 2026](https://blog.google/innovation-and-ai/infrastructure-and-cloud/google-cloud/cloud-next-2026-sundar-pichai/)
- Earlier CEO figures: about 25% in October 2024 and over 30% by April 2025. Semafor notes these numbers are "pretty impossible to fact-check." — [Semafor, 24 Apr 2026](https://www.semafor.com/article/04/24/2026/google-ceo-says-75-of-companys-new-code-is-ai-generated); [Fortune, 30 Oct 2024](https://www.fortune.com/2024/10/30/googles-code-ai-sundar-pichai) (secondary; I did not read the Alphabet earnings transcripts)
- Controlled evidence: a randomized trial with 96 Google engineers found AI cut time on a complex enterprise task by "about 21%, although our confidence interval is large." — [Paradis et al., arXiv 2410.12944, Oct 2024](https://arxiv.org/abs/2410.12944)

**AI for migrations (the LSC playbook, now with LLMs)**
- The pipeline:
  1. Find the code to change with Code Search, Kythe and scripts.
  2. An LLM toolkit makes edits that must build and pass tests, and it updates tests when needed.
  3. An engineer checks the result.
  4. The LSC system shards the change and sends the pieces to code owners.
  — [arXiv 2501.06972](https://arxiv.org/abs/2501.06972)
- Results by migration:
  - 32→64-bit IDs: 80% of code modifications in landed changes were fully AI-written, and engineers estimated about a 50% cut in total time, review and landing included. "In most cases, the human needed to revert at least some changes the model made."
  - JUnit3→JUnit4: 5,359 files and 149,000+ lines migrated in 3 months, with about 87% of AI-generated code committed unchanged.
  - Joda-Time→java.time: about 89% time saved on small clusters, as estimated by tech leads.
  — [arXiv 2501.06972](https://arxiv.org/abs/2501.06972)
- The bottleneck is review: "The bottleneck in the process was the speed at which engineers could review the changes. We purposefully limited the number of changes we generate every week to avoid overwhelming reviewers." Also: "Code reviews and change rollouts still require a human operator and can quickly become bottlenecks." — [arXiv 2501.06972](https://arxiv.org/abs/2501.06972)
- Design lesson: combine LLMs with AST tools (which edit the parsed syntax tree, not raw text). AST tooling "has the advantage to be 'always correct'." Diffing the before/after AST catches model mistakes such as stray comments or edits to methods it shouldn't touch. "LLM planning capabilities are often not needed and add a layer of complexity that should be avoided." — [arXiv 2501.06972](https://arxiv.org/abs/2501.06972)
- A follow-up study of 32→64-bit ID migrations (39 migrations over 12 months by 3 developers; 595 changes; 93,574 edits):
  - 74.45% of changes and 69.46% of edits were LLM-generated.
  - Developers estimated a 50% time reduction.
  - Validation was layered: AST parse, build, tests, human check, then owner approval in Critique.
  - Hallucinations such as reformatting or irrelevant edits were noted.
  — [Ziftci et al., "Migrating Code At Scale With LLMs At Google," arXiv 2504.09691, Apr 2025](https://arxiv.org/html/2504.09691v1)

**AI inside code review**
- ML-suggested edits for reviewer comments (May 2023): the model addresses 52% of comments at a 50% precision target. Authors apply 40–50% of previewed suggestions. Authors spend about 60 minutes on average addressing reviewer comments, and the projected saving is "hundreds of thousands of hours annually." — [Google Research blog, 23 May 2023](https://research.google/blog/resolving-code-review-comments-with-ml/)
- AutoCommenter (2024): an LLM flags best-practice violations in review for C++, Java, Python and Go.
  - Developers rated 54% of its comments useful (raters: 60%), and about 40% of comments were resolved.
  - 66% of the targeted best practices are beyond traditional static analysis.
  - Its comments cover 68% of historical human comments that cite a best-practice URL.
  - It is used by "tens of thousands of developers every day." It went to general availability in October 2023.
  — [arXiv 2405.13565](https://arxiv.org/html/2405.13565v1); [research.google](https://research.google/pubs/ai-assisted-assessment-of-coding-practices-in-industrial-code-review/)
- A Google Cloud Office of the CTO article (28 Apr 2026) says: "AI-driven coding has shifted the software bottleneck from writing to reviewing." Its remedies are guardrails (linters, rules, mandatory AI-generated test coverage); reviewers focusing on architecture instead of style nitpicks; a "Conditional LGTM"; and AI-made reviewer guides that summarize what changed and the risks. — [Google Cloud Transform, Lee Boonstra, 28 Apr 2026](https://cloud.google.com/transform/when-ai-writes-the-code-who-reviews-it-cto-google-cloud) (describes agentic team practice in general, not necessarily Google's monorepo process)
- CodeMender (DeepMind, October 2025) is an agent that finds and patches security bugs. It upstreamed 72 fixes to open-source projects, some as large as 4.5M lines, and a human signs off on every fix. — [Google DeepMind blog](https://deepmind.google/blog/introducing-codemender-an-ai-agent-for-code-security/); [The Hacker News, Oct 2025](https://thehackernews.com/2025/10/googles-new-ai-doesnt-just-find.html)

**DORA (Google-run research on the wider industry)**
- 2024 (22 Oct 2024): "More than 75 percent" of respondents rely on AI daily.
  - A 25% increase in AI adoption was associated with +7.5% documentation quality, +3.4% code quality and +3.1% code-review speed.
  - The same increase was associated with an estimated −1.5% delivery throughput and −7.2% delivery stability.
  - 39% reported little or no trust in AI-generated code.
  - "AI does not appear to be a panacea."
  — [Google Cloud blog, 2024 DORA report](https://cloud.google.com/blog/products/devops-sre/announcing-the-2024-dora-report); [dora.dev 2024](https://dora.dev/research/2024/dora-report/)
- The 2024 report stresses that "fundamentals like small batch sizes and robust testing remain crucial." — [dora.dev 2024](https://dora.dev/research/2024/dora-report/)
- 2025 (23 Sep 2025, *State of AI-assisted Software Development*): 90% use AI at work, and more than 80% think it raised their productivity. 30% report little or no trust in AI code. AI adoption is now positively linked to throughput and product performance, but it "continue[s] to have a negative relationship with software delivery stability." "AI accelerates software development, but that acceleration can expose weaknesses downstream." 90% of organizations have adopted at least one internal platform, and platform quality is linked to getting value from AI. — [Google Cloud blog, 2025 DORA report](https://cloud.google.com/blog/products/ai-machine-learning/announcing-the-2025-dora-report)
- The 2025 DORA AI Capabilities Model has seven capabilities. Six are confirmed in the sources I read: version control practices, small batches, quality internal platforms, user-centric focus, AI-accessible internal data, and a clear AI stance. — [dora.dev 2025 year in review](https://dora.dev/insights/dora-2025-year-in-review/)

### Inferences
- Google points AI at the same debt it always fought with LSCs, namely migrations and deprecations. It keeps the old safety rails in place: build and test gates, sharding by owner, and owner review. The AI replaces the Refaster/ClangMR step, not the review step.
- Every Google source agrees that human review, not code generation, now limits throughput. Google deliberately slowed AI migration output to fit reviewer capacity.
- DORA's results (more AI, more instability, and small batches as a protective capability) point the same way as Google's long-standing rules on small CLs and fast reviews. These practices look more important with AI, not less.
- The headline percentages measure different things. "Characters completed with AI assistance" (2024: 50%; 2025: 67%) is not the same as "new code AI-generated and approved by engineers" (2026: 75%). They should not be read as one trend line.

### Gaps
- Google has not published how it defines "AI-generated" for the 75% figure. It has not published defect, rollback or incident rates for AI-written code compared with human-written code.
- I found no Google paper from 2025–2026 that measures whether AI-written code adds or reduces tech debt inside google3, Google's monorepo.
- I found no public update to the eng-practices guide or the readability process for AI-authored CLs.
- The seventh DORA 2025 capability was not confirmed in the sources I read.
