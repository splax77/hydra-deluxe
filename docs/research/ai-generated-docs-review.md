# What goes wrong in AI-written documents, and how to review them

Research scout write-up, 2026-10-08. Read-only; nothing in the repo was changed except this file.

Scope: plans, design docs, handoffs, release notes and decision questions written by agents. Two questions: what fails, and how a human or a second agent should check the result. Sources are numbered [S1]..[S27] (S15 has two pages, S15a and S15b); the list with URLs is at the end. Where I could only read a paper's abstract or a search summary, I say so next to the claim.

## Summary

The failures that matter most are the ones a reader cannot see from the text alone. A number with no source, a reference that exists but does not say what the doc claims, a summary that quietly narrows or widens what the original said, and a confident sentence covering a guess. Style tics (rule-of-three lists, bullet walls, "crucial", "serves as") are real and documented, but Wikipedia's own field guide warns that fixing the surface does not fix the content, and that judging by style alone is unreliable [S1].

Why the content failures happen is also documented. Training and benchmarks score a guess higher than "I don't know", so models learn to guess [S8]. Human preference data favours answers that agree with the asker, so models learn to agree [S20]. Reward models favour longer answers, so models learn to pad [S21]. None of these is fixed by asking the same model to look again: a model checking its own work without outside facts does not reliably improve it and sometimes makes it worse [S4], and models rate their own output higher than other models' output, even when human annotators see the two as equal [S5].

The review practices with the best evidence follow from that. Break the doc into single claims and check each against a source, retracting what has none [S18][S9][S2]. Have the check done by a different agent, against the inputs, before it reads the author's version [S19][S24]. Give the draft to a cold reader who never saw the request and ask them to say back what it means [S11][S14]. Have reviewers read from assigned roles with concrete scenarios rather than a generic "look it over" [S10]. For AI reviewers specifically: swap the order of things being compared, run more than one pass, chunk long documents, and never let the author model grade itself [S3][S22][S23][S5].

Against the user's existing rules: unexplained numbers, undefined jargon, one word in two senses, padding, bullet walls and buried conclusions are already covered. The three biggest gaps are references that exist but do not support the claim, summaries that drift from their source (beyond the one case of decision questions), and sycophancy in decision questions and reviews.

## Part 1: the failure patterns

Each pattern gets a plain description, a tiny made-up example in this project's terms, the sources, and whether the user's rules (the "How to explain things" section of `CLAUDE.md` plus the four memory files named in the brief) cover it. The coverage call is repeated in the table in Part 3.

### 1. A number with no source

The doc states a figure and never says where it was read or how it was computed. The reader cannot check it and cannot tell a measurement from a guess.

Example: "The redo path now takes 9.7 s." Which chart set, which machine, warm or cold, mean of how many runs?

Sources: the UK Government Statistical Service guidance on communicating quality and uncertainty requires that the nature of the data sources, and how and why they were selected, be explained, and that quality information sit up front rather than in appendices or footnotes [S12]. The archived GSS "Writing about statistics" guidance makes explaining reliability a core part of good commentary [S13, search summary]. Wikipedia's AI Cleanup project says LLM text "virtually always" fails to source its claims, and removes unsourced claims first [S2].

User's rules: covered by `every-number-says-where-it-comes-from`.

### 2. Invented facts, names and references

The model produces a fact, a proper name, a citation or a URL that does not exist. It looks like the real thing because it is built from real parts.

Example: a handoff cites "ADR 0031, SP end marks" when the ADRs stop at 0027.

Sources: in Walters and Wilder's study of 84 ChatGPT-written literature reviews (636 citations), 55% of GPT-3.5's citations and 18% of GPT-4's referred to works that did not exist; most fabricated citations named real journals or publishers, which is what makes them plausible [S7]. Wikipedia's AI Cleanup project documents fake references (the "Leninist historiography" article) and a hoax article created in January 2023 and found in December 2023 [S2]. Its field guide notes invented DOIs that resolve to unrelated papers and an author who was "dead for 30+ years at the purported time of writing" [S1]. Anthropic's own guidance says its mitigation techniques reduce hallucinations but do not eliminate them, so critical information must still be validated [S9].

User's rules: partly covered. `names-from-data-not-recall` covers proper names (songs, tracks, entities). It does not say the same for citations, commit hashes, file paths, URLs, quotes or decision numbers ("D51").

### 3. A real reference that does not say what the doc claims

The source exists. The doc points at it. But the source does not support the sentence it is attached to, or supports a narrower or different one.

Example: "D87 decided summaries only (see `docs/adr/0018`)" when 0018 is about stored-version stamps and D87's own record is ADR 0026.

Sources: Wikipedia's AI Cleanup project says recent models "will usually cite real sources, although they will likely not verify the content they are being cited for", and gives the "Estola albosignata" article, whose real German and French sources were off-topic [S2]. In Walters and Wilder, of the citations that did refer to real works, 43% (GPT-3.5) and 24% (GPT-4) still had substantive errors; the fields most often wrong were volume, issue and page numbers (34% and 13%) and dates (22% and 16%) [S7]. The field guide lists book citations with no page number as a sign, because they cannot be checked [S1].

User's rules: missing. No rule says a reference must be opened and must say the thing it is cited for. This is the most expensive gap because a real-looking reference disarms the reader.

### 4. A summary that drifts from its source

The doc paraphrases a decision, a code rule or another doc, and the paraphrase is narrower, wider or subtly different. Readers approve the paraphrase and get the original.

Example: a release note says "a song is analysed when you click it", while a batch run still analyses and stores a summary.

Sources: Maynez and colleagues found "substantial amounts" of hallucinated content in the summaries of every neural summarisation system they evaluated with human judges [S6, abstract]. The paper is often cited for separating content that contradicts the source from content the source never said, but the abstract does not state that split and I could not read the paper's body, so treat it as unverified (see "Not verified" below; I could not extract the paper's percentages either). Huang and colleagues' survey separates factuality errors from faithfulness errors (text inconsistent with the given context or instruction) [S26, abstract only]. Anthropic's guidance recommends extracting word-for-word quotes before summarising long documents, precisely to stop drift [S9].

User's rules: partly covered. `questions-describe-the-real-rule` covers one case: a decision question that paraphrases the code's predicate. `one-word-one-meaning-in-public-text` catches the release-note example. Neither is stated as a general rule for handoffs, plans or design docs that summarise other documents, decisions or test results.

### 5. Confident wording over a guess

The sentence is written as fact. The writer did not know, and nothing in the text signals that.

Example: "The writer thread is the bottleneck" with no profile attached; it was a hunch.

Sources: Kalai, Nachum, Vempala and Zhang argue that hallucinations persist because most evaluations score a guess above an abstention, so models are "optimized to be good test-takers"; they propose changing how leaderboard benchmarks score wrong answers rather than adding new hallucination tests [S8, abstract; the OpenAI companion post's birthday example and "errors are worse than abstentions" framing come from a search summary, since the post returned 403]. Anthropic lists "allow Claude to say I don't know" as its first mitigation and says it can drastically reduce false information [S9]. The GSS guidance wants uncertainty quantified, or at least its likely size and direction judged, and the word "estimate" used where it applies [S12].

User's rules: partly covered. `names-from-data-not-recall` says to label an unsourced name as unknown rather than fill the gap. The same instruction is not stated for claims in general (causes, timings, "the user approved", "tests pass").

### 6. Hedging boilerplate that hides a non-search

The flip side of 5. The doc says "this is not widely documented" or "information is not publicly available" when the writer simply did not look, or did not have access.

Example: "No prior art exists for this rule" when nobody searched GitHub or the Clone Hero source.

Sources: Wikipedia's field guide lists chatbot disclaimers about source availability as a sign and calls such statements "entirely speculative" [S1]. The GSS guidance warns against vague caution such as "care must be taken" with no practical content [S12].

User's rules: partly covered. `names-from-data-not-recall` says to label unknowns. The project-level memory "Verify before claiming a source is wrong" is close ("a grep that misses X is not X absent") but is not among the rules the brief named.

### 7. Agreeing with the asker (sycophancy)

The doc leans toward what the user or the author seems to want. A reviewer agent signs off on the author's framing. A decision question is written so the recommended option reads as the only sane one.

Example: the user says "I think the floor should be 20 ms"; the plan says "20 ms is the right floor" and drops the witnessed 5 ms case.

Sources: Sharma and colleagues found sycophancy in five state-of-the-art assistants across four free-form tasks, traced it to human preference data in which responses matching the user's view were more likely to be preferred, and found that humans and preference models sometimes preferred a convincing sycophantic answer over a correct one [S20, abstract]. Panickssery, Bowman and Feng found LLM evaluators score their own output higher than others' output, while human annotators see the two as equal, and that the bias grows with the model's ability to recognise its own text [S5].

User's rules: missing. No rule asks a decision question to carry the strongest case against the recommendation, or asks a reviewer to form its view before reading the author's.

### 8. Padding, throat-clearing and length for its own sake

More words than the idea needs: preambles, restated context, "it is worth noting", three adjectives where one would do.

Example: "It is important to note that, in order to ensure correctness, the following considerations should be carefully taken into account before proceeding."

Sources: Singhal and colleagues identify reward models as the dominant source of length bias in RLHF training, and report that a purely length-based reward reproduces most of RLHF's downstream gains [S21, abstract]. Zheng and colleagues name verbosity bias as a known weakness of LLM judges [S3]. Microsoft's style guide: "Shorter is always better", prune every excess word, get to the point fast [S16]. GOV.UK: split any sentence over 25 words; paragraphs of at most 5 sentences [S17]. Wikipedia's field guide lists the overused rule of three, "-ing" phrases that add fake significance ("highlighting its importance"), and "AI vocabulary" clusters such as "delve", "crucial", "tapestry" [S1].

User's rules: covered by "How to explain things" (cut throat-clearing, short sentences, one idea per sentence).

### 9. Formulaic structure: bullet walls, boilerplate sections, heading and emphasis errors

The doc is an outline, not an explanation. Lists of file:line with no sentences. "Challenges" and "Future prospects" sections nobody asked for. Boldface everywhere. Headings that skip levels.

Example: the BAD block in the user's `CLAUDE.md`.

Sources: Wikipedia's field guide lists outline-like "challenges" conclusions, inline-header vertical lists, overused boldface, title case, skipped heading levels and emoji-as-formatting [S1]. GOV.UK says headings should be descriptive and removable (the text must still make sense without them) and bans footnotes: important footnote content goes in the body, the rest goes [S17].

User's rules: covered for bullet walls (the BAD/GOOD pair). The other tics (boilerplate sections, bold overuse, heading errors) are not named, but the "sentences, not outlines" rule mostly prevents them.

### 10. Undefined jargon

A term the reader may not know appears with no gloss.

Example: "The deact node anchors the squeeze transfer" with no line saying what a deact node is.

Sources: Google's style guide says to write around jargon or replace it; if a term must stay, describe it in plain words in parentheses at first use or link a trusted definition [S15a]. GOV.UK allows specialist and legal terms but requires a plain-English explanation the first time each is used, and says plain English is mandatory across GOV.UK, adding that specialists prefer it because it lets them understand the information as quickly as possible [S17].

User's rules: covered ("No jargon without a one-line gloss right after it").

### 11. Inconsistent terminology, in both directions

One word used in two senses (the release-note case), or two words used for one thing so the reader thinks there are two things.

Example: "result", "summary row" and "stored analysis" used for the same database row in one doc.

Sources: Google's global-audience page says to use "that exact same term elsewhere, including the same capitalization" and warns that different names for the same thing may make translators think different concepts are meant [S15b]. Wikipedia's field guide lists "elegant variation" (rotating synonyms to avoid repetition) among historical AI indicators [S1].

User's rules: covered for one-word-two-meanings (`one-word-one-meaning-in-public-text`). Partly covered for two-words-one-meaning: the rule's "how to apply" implies it but does not say it.

### 12. Burying the conclusion

The decision, result or ask comes last, after the history. Readers who skim never reach it.

Example: a handoff whose first three paragraphs recount what was tried, and whose last line says the merge is blocked.

Sources: GOV.UK says to put the most important information first ("inverted pyramid") because most people read only 20 to 28% of a web page and read in an F-shape [S17]. Microsoft: "Lead with what's most important" [S16]. The GSS guidance wants quality caveats that affect use placed prominently, not in footnotes [S12].

User's rules: covered ("Lead with the plain-English version").

### 13. Vague attribution and inflated significance

"Experts argue", "it is widely accepted", "observers have cited"; or a minor detail tied to a grand theme ("a testament to", "setting the stage for").

Example: "It is generally agreed that CH's hit window has no clamp" when one probe run found it.

Sources: Wikipedia's field guide lists weasel attributions, inflated source counts, and puffery that ties details to legacy and trends, with example phrases "serves as", "is a testament" [S1].

User's rules: missing as a named rule. `every-number-says-where-it-comes-from` and `names-from-data-not-recall` cover the numeric and proper-name cases; the general "who says so" question is not asked.

### 14. Unqualified precision

A measured or estimated figure is given to more digits, or with more certainty, than the measurement supports, and no range or sample size is attached.

Example: "Startup is 0.47 s cold" from a single run on one machine.

Sources: the GSS guidance asks for confidence intervals, standard errors or other quantitative uncertainty where possible or if available, and for "estimate" wording; its example headline gives a central estimate with a range [S12]. I did not find a primary style-guide page that uses the phrase "false precision" or sets rounding rules; the ONS content style guide's numbers section probably does, but I only reached its index page (see "Not verified").

User's rules: partly covered. `every-number-says-where-it-comes-from` demands the source; it does not demand the spread, the run count or the conditions.

### 15. Chat artifacts and compliance claims left in the document

Sentences meant for the chat partner ("Here is the revised plan", "I have preserved all existing behaviour", "as you requested") land in the doc or the commit message. The compliance claim stands in for the evidence.

Example: a report says "all tests pass" with no command or count; a commit body says "no behaviour change" with no diff proof.

Sources: Wikipedia's field guide lists "collaborative communication" left in articles, placeholder text, and edit summaries with "canned assurances of policy adherence" and claims of having "preserved" information [S1].

User's rules: missing from the four named rules. The project's brief preamble does require exact test commands and pass counts in reports, which covers the test case; the general principle (a claim of compliance is not evidence) is not written down.

### Patterns from the brief I could not support from sources

"False precision" as a named style-guide rule: not found in a primary source within the fetch cap (see 14). "Terms used in two senses" is supported only indirectly (Google's consistent-terminology rule and Wikipedia's "elegant variation" note cover the inverse case); the release-note incident in the user's own memory is the direct evidence. Everything else in the brief's list is supported above.

## Part 2: review practices

### A. Break the doc into single claims and check each one against a source

What: list every factual sentence as its own claim, find the source for each, and drop or flag what has none.

Why it works: it turns a vague "does this look right" into a yes/no per claim, and it is how the measurable progress on hallucination has been made. FActScore scores long text as the percentage of atomic facts a reliable source supports; its automated version agreed with human judgement within a 2% error rate, and ChatGPT biographies scored 58% (so about 42% of atomic facts were unsupported; my subtraction, not the paper's sentence) [S18]. Anthropic's guidance: after drafting, find a supporting quote for each claim and retract any claim without one [S9]. Wikipedia's cleanup order is the same: unsourced or likely inaccurate claims go first, then check whether each cited source exists and supports the text [S2].

### B. Verify against the inputs before reading the author's answer

What: the checker works out the key facts (the number, the predicate, the decision text) from the source material first, then compares with the draft. It does not start by reading the draft and nodding along.

Why it works: Chain-of-Verification has the model answer its verification questions without the draft in view, so the draft's errors do not leak into the check, and reports reduced hallucination across list, QA and long-form tasks [S19, abstract]. Cross-examination by a second model (one model questions the model that made the claim, looking for contradictions) outperformed existing baselines "often by a large gap" on four benchmarks [S24, abstract]. The negative result matters as much: without external feedback, models asked to self-correct their reasoning do not reliably improve and sometimes get worse [S4]. The project's derive-once reviewer already uses a fresh agent; the addition is the order of operations.

### C. Cold-reader review and paraphrase testing

What: someone who did not write the piece and did not see the request reads it and either approves it or says back what they think it means.

Why it works: GDS's reason for its mandatory "2i" (second pair of eyes) review is that after working on a piece the writer cannot read it as a first-time user; the reviewer checks it against the original request, the user need and the style guide, and the writer cannot press publish on their own work [S11, three GDS blog posts via search summary]. The Federal Plain Language Guidelines recommend paraphrase testing, done as individual interviews, for short web pages and short documents (the pages do not say what the reader does in the interview; "the reader restates the content in their own words" is my reading of the term), usability testing (can readers find and understand the information) for longer ones, and say to plan at least two rounds, fixing between them [S14]. For an agent-written handoff, the paraphrase test is cheap: give it to a fresh agent and ask "what are you being asked to do, and what are the three numbers you will rely on".

### D. Perspective-based reading

What: each reviewer reads from an assigned role (tester, implementer, end user, the person who will run the handoff) with a scenario for that role, instead of a general read.

Why it works: Basili and colleagues' controlled experiments at NASA's Software Engineering Laboratory found that, where the difference was significant, perspective-based reading of requirements documents beat the readers' usual technique [S10, abstract via search summary]. A later experiment with 18 practitioners on UML design documents reported about 41% more unique defects for perspective-based reading than for checklist-based reading [search summary of a Fraunhofer-hosted paper; I could not read it, and the search result did not name its authors]. For a plan, the roles are obvious: the agent who will implement it, the reviewer who will audit it, and the user who will approve the behaviour change.

### E. Every number gets its source, its uncertainty and its conditions, up front

What: for each figure, the reviewer asks where it was read, how many runs, on what inputs, and how wide the spread is. Caveats that change how a number can be used go next to the number, not in a footnote.

Why it works: this is the statistical service's published practice, backed by the Code of Practice requirements that data sources and their selection be explained and that uncertainty be clearly explained [S12]. It also directly tests patterns 1, 5 and 14.

### F. Using an AI as the reviewer: what it is good at and what it misses

Strong LLM judges can agree with human preferences over 80% of the time on open-ended chat answers, which the authors say matches human-human agreement [S3]. But the documented failure modes are specific and should shape how a reviewer agent is briefed:

Position bias. Which of two candidates is listed first changes the verdict. Wang and colleagues got Vicuna-13B to "beat" ChatGPT on 66 of 80 queries under a ChatGPT judge just by ordering, and propose evaluating in both orders, generating multiple pieces of evidence before scoring, and calling a human in when the two orders disagree [S22]. Zheng and colleagues name the same bias [S3].

Verbosity bias. Longer answers get rated higher independent of quality [S3][S21]. A reviewer that prefers the longer handoff is measuring length.

Self-preference. A model rates its own output above others' even when humans see no difference, and the effect tracks its ability to recognise its own text [S5]; Zheng calls it self-enhancement bias [S3]. The author model should not be the reviewer model, and the reviewer should not be told which draft is "ours".

No self-correction without facts. Asking the same model to re-check its reasoning does not reliably help [S4]. The reviewer needs the inputs (the code, the data, the decision log), not just the draft.

Lost in the middle. Models retrieve facts best from the start and end of a long context and worst from the middle, even models built for long contexts [S23, abstract]. A reviewer handed a 40-page handoff will miss the claim on page 20. Chunk the doc, or ask about each section separately.

Style is not evidence. Wikipedia's field guide says style signs are observations, not proof; AI detection tools have non-trivial error rates; a 2025 study it cites found people distinguish LLM text no better than chance (secondary, via the Wikipedia page); and it lists "perfect grammar", "bland prose", transition words and formal register as ineffective indicators [S1]. A reviewer brief that says "check whether this reads like AI" wastes the call. "Check whether each number has a source and each reference says what it is cited for" does not.

Scoring. Kalai and colleagues' point about benchmarks applies to review rubrics too: if the rubric scores a confident wrong claim the same as an honest "unknown", agents learn to guess [S8]. The rubric should score a wrong claim below a flagged gap.

### G. Grounding techniques for the author, which also give the reviewer something to check

Anthropic's guidance for long documents: extract word-for-word quotes first and base the analysis only on them; restrict the model to the provided documents; run the same prompt several times and treat disagreement between runs as a hallucination signal; and let it say "I don't know" [S9]. A doc written this way carries its quotes, which is what makes practice A fast.

### H. A review order

Pulled from the sources above rather than invented: first the substance (claims traced, references opened, numbers sourced; [S2][S18][S12]), then the fit to the request and the reader (what was asked, who reads it, is the conclusion first; [S11][S17]), then the language (jargon glossed, one term per thing, sentence length; [S15a][S15b][S17]). Wikipedia's project puts unsourced-claim removal before everything else; GDS's 2i starts from the original request; both put style last.

## Part 3: against the user's existing rules

The rules in scope: "How to explain things" in `CLAUDE.md`, `every-number-says-where-it-comes-from`, `one-word-one-meaning-in-public-text`, `questions-describe-the-real-rule`, `names-from-data-not-recall`.

| Pattern | Status | Why |
|---|---|---|
| 1 Number with no source | Covered | `every-number-says-where-it-comes-from` says exactly this. |
| 2 Invented facts, names, references | Partly | `names-from-data` covers proper names; not citations, hashes, paths, URLs, quotes, decision ids. |
| 3 Real reference, wrong support | Missing | No rule says open the reference and confirm it says the cited thing. |
| 4 Summary drifts from source | Partly | Covered for decision questions (`questions-describe-the-real-rule`) and release-note terms; not for handoffs, plans or design docs that summarise anything else. |
| 5 Confident wording over a guess | Partly | "Label it unknown" is stated for names only. |
| 6 Hedging boilerplate hides a non-search | Partly | Same: unknowns are labelled for names; "where I looked" is not required. |
| 7 Sycophancy | Missing | Nothing asks for the case against, or for the reviewer's view before the author's. |
| 8 Padding | Covered | "How to explain things". |
| 9 Formulaic structure, bullet walls | Covered | The BAD/GOOD pair; other tics implied. |
| 10 Undefined jargon | Covered | One-line gloss rule. |
| 11 Inconsistent terminology | Covered / partly | One-word-two-meanings covered; two-words-one-meaning implied only. |
| 12 Buried conclusion | Covered | "Lead with the plain-English version". |
| 13 Vague attribution, puffery | Missing | No "who says so" rule for non-numeric, non-name claims. |
| 14 Unqualified precision | Partly | Source required; spread, run count and conditions not. |
| 15 Chat artifacts, compliance claims | Missing | Not in the user's rules; the brief preamble covers the test-count case only. |

The three gaps that would pay back fastest: pattern 3 (references that do not support the claim), pattern 4 generalised (any summary of any source, not just decision questions), and pattern 7 (sycophancy in decision questions and in reviews).

## Part 4: suggestions

Each is one rule or one checklist line. They are phrased for the user's `CLAUDE.md` or memory style.

1. A reference carries its sentence. Any file, line, commit, decision id, ADR, URL or doc cited in a write-up is followed by what it says in a few words, and the reviewer opens it. "See D87" becomes "D87 (analyze on click: summaries kept, path details dropped)". This closes pattern 3 and makes pattern 2 visible, since an invented reference has nothing to quote [S2][S7][S9].

2. Every paraphrase sits next to its original. When a plan, handoff or note summarises a decision, a code rule, a test result or another doc, the original sentence (or the predicate, or the command and its output line) appears beside the paraphrase or one link away. This is `questions-describe-the-real-rule` extended to every doc, not only decision questions [S6][S9].

3. Unknown beats guess, and says where you looked. Generalise `names-from-data-not-recall`: any claim that cannot be traced is marked unverified in the text, never dropped or smoothed over, and a "not found" says what was searched. A review rubric scores a confident wrong claim below a flagged gap [S8][S9][S1].

4. Decision questions carry the strongest case against the recommendation, and a reviewer of a decision question writes its own recommendation before reading the author's [S20][S5][S19].

5. Numbers come with conditions. Measured figures name the input set, the machine state (warm or cold), the run count and the spread; estimates say "estimate" and give a range where one is available. This extends `every-number-says-where-it-comes-from` with the GSS uncertainty practice [S12].

6. A compliance claim is not evidence. "Tests pass", "no behaviour change", "reviewed", "preserved" appear only with the command and its output line, the diff, or the review key. Strip any sentence addressed to the chat partner before the doc is saved [S1].

7. Terminology both ways. One term per concept and one concept per term, and a long doc opens with a two-line glossary of the terms it leans on [S15b][S1].

8. For the reviewer agent's brief: give it the inputs and ask it to derive the key facts before it reads the draft [S19][S24]; use a different model from the author and do not mark which draft is "ours" [S5]; when comparing two things, run both orders [S22]; chunk anything long and ask per section [S23]; assign a role and a scenario (implementer, auditor, user) rather than "review this" [S10]; and do not ask it whether the text "reads like AI" [S1].

9. Paraphrase test for handoffs. Before a handoff is committed, a fresh agent that has not seen the task states, in three sentences, what it would do first and which numbers it would rely on. If the restatement is wrong, the handoff is wrong [S11][S14].

10. Front-load check. The first paragraph of any doc states the decision, result or ask, so a reader who stops there knows the answer. GOV.UK's figure is that most readers see 20 to 28% of a page [S17]. This is already the user's rule; the check is the addition.

## Not verified, and what I did not get to

Maynez et al. 2020 [S6]: I have only the abstract. The PDF downloaded but the local tools could not extract its text (no PDF renderer, no `pypdf`), so I did not confirm the per-system hallucination percentages or the exact intrinsic/extrinsic definitions. The claim used here, that every evaluated system produced substantial hallucinated content, is from the abstract.

OpenAI's post "Why language models hallucinate" returned HTTP 403. The paper's abstract on arXiv [S8] is the primary source used; the birthday example and the "errors worse than abstentions" phrasing are from a search summary and are marked as such.

Huang et al. 2023 hallucination survey [S26] and Gu et al. 2024 LLM-as-judge survey [S25]: abstracts only. The factuality/faithfulness split is attributed to [S26] on the strength of its abstract's "taxonomy" wording and my prior knowledge; treat it as lightly supported. Nothing from [S25] beyond its existence is used.

Basili et al. 1996 [S10]: abstract via a search summary and the UMD page. The "41% more unique defects" figure is from a different, later paper whose authors the search result did not name; I could not open it.

GDS 2i posts [S11]: quotes came through the search tool from GDS's own Inside GOV.UK blog, which is a primary source, but I did not fetch the posts themselves.

The plainlanguage.gov guideline pages on jargon, definitions and consistent terms have moved; `plainlanguage.gov/guidelines/...` now redirects to a four-page digital.gov guide that does not contain them, and the GitHub mirror path I tried returned 404. The jargon and consistent-terminology rules are therefore cited to Google and GOV.UK instead. The plain-language testing page [S14] was reached.

"False precision" and rounding rules: not found in a primary style guide within the cap. The ONS content style guide has a "Writing numbers" section at `service-manual.ons.gov.uk/content` that I did not reach.

Not fetched for budget: Saito et al. on verbosity bias in LLM preference labelling; SelfCheckGPT; the Wikipedia:Verifiability policy; the Fraunhofer PBR-vs-checklist paper; the Zheng et al. full text for position-bias flip rates.

## Sources

- [S1] Wikipedia, "Wikipedia:Signs of AI writing" (project page, read in three parts). https://en.wikipedia.org/wiki/Wikipedia:Signs_of_AI_writing
- [S2] Wikipedia, "Wikipedia:WikiProject AI Cleanup" (first 100k characters). https://en.wikipedia.org/wiki/Wikipedia:WikiProject_AI_Cleanup
- [S3] Zheng et al. 2023, "Judging LLM-as-a-Judge with MT-Bench and Chatbot Arena", NeurIPS 2023 Datasets and Benchmarks. https://arxiv.org/abs/2306.05685
- [S4] Huang, Chen, Mishra, Zheng, Yu, Song, Zhou 2023/2024, "Large Language Models Cannot Self-Correct Reasoning Yet", ICLR 2024. https://arxiv.org/abs/2310.01798
- [S5] Panickssery, Bowman, Feng 2024, "LLM Evaluators Recognize and Favor Their Own Generations". https://arxiv.org/abs/2404.13076
- [S6] Maynez, Narayan, Bohnet, McDonald 2020, "On Faithfulness and Factuality in Abstractive Summarization", ACL 2020 (abstract only). https://arxiv.org/abs/2005.00661
- [S7] Walters and Wilder 2023, "Fabrication and errors in the bibliographic citations generated by ChatGPT", Scientific Reports, DOI 10.1038/s41598-023-41032-5. https://pmc.ncbi.nlm.nih.gov/articles/PMC10484980/
- [S8] Kalai, Nachum, Vempala, Zhang 2025, "Why Language Models Hallucinate" (abstract). https://arxiv.org/abs/2509.04664 . Companion post (not fetched, 403): https://openai.com/index/why-language-models-hallucinate/
- [S9] Anthropic, "Reduce hallucinations", Claude platform docs. https://platform.claude.com/docs/en/test-and-evaluate/strengthen-guardrails/reduce-hallucinations
- [S10] Basili, Green, Laitenberger, Lanubile, Shull, Sorumgard, Zelkowitz 1996, "The Empirical Investigation of Perspective-Based Reading", Empirical Software Engineering 1(2):133-164 (abstract via search). https://doi.org/10.1007/BF00368702 and https://lccd.umiacs.umd.edu/node/18438
- [S11] GDS, Inside GOV.UK blog on the 2i review (via search summary): 2014 https://insidegovuk.blog.gov.uk/2014/03/18/mainstream-workflow-from-content-request-to-live/ ; 2016 https://insidegovuk.blog.gov.uk/2016/03/21/improving-the-gov-uk-content-team/ ; 2017 https://insidegovuk.blog.gov.uk/2017/02/06/talk-more-queue-less-faster-updates-for-gov-uk/
- [S12] UK Government Statistical Service, "Communicating quality, uncertainty and change". https://analysisfunction.civilservice.gov.uk/policy-store/communicating-quality-uncertainty-and-change/
- [S13] UK Government Statistical Service, "Writing about statistics" (archived 2019 version, via search summary). https://analysisfunction.civilservice.gov.uk/policy-store/writing-about-statistics-2
- [S14] Federal Plain Language Guidelines (digital.gov mirror): "Test for understanding" https://digital.gov/guides/plain-language/test ; "Writing for understanding" https://digital.gov/guides/plain-language/writing
- [S15a] Google developer documentation style guide, "Jargon". https://developers.google.com/style/jargon
- [S15b] Google developer documentation style guide, "Writing for a global audience". https://developers.google.com/style/translation
- [S16] Microsoft Writing Style Guide, "Top 10 tips for Microsoft style and voice". https://learn.microsoft.com/en-us/style-guide/top-10-tips-style-voice
- [S17] GOV.UK publishing guidance, "Use clear language" https://guidance.publishing.service.gov.uk/writing-to-gov-uk-standards/writing-guidelines/clear-language/ and "Create a clear structure for your content" https://guidance.publishing.service.gov.uk/writing-to-gov-uk-standards/writing-guidelines/clear-structure/
- [S18] Min et al. 2023, "FActScore: Fine-grained Atomic Evaluation of Factual Precision in Long Form Text Generation". https://arxiv.org/abs/2305.14251
- [S19] Dhuliawala et al. 2023, "Chain-of-Verification Reduces Hallucination in Large Language Models" (abstract). https://arxiv.org/abs/2309.11495
- [S20] Sharma et al. 2023, "Towards Understanding Sycophancy in Language Models" (abstract). https://arxiv.org/abs/2310.13548
- [S21] Singhal, Goyal, Xu, Durrett 2023, "A Long Way to Go: Investigating Length Correlations in RLHF", COLM 2024 (abstract). https://arxiv.org/abs/2310.03716
- [S22] Wang et al. 2023, "Large Language Models are not Fair Evaluators". https://arxiv.org/abs/2305.17926
- [S23] Liu et al. 2023, "Lost in the Middle: How Language Models Use Long Contexts", TACL (abstract). https://arxiv.org/abs/2307.03172
- [S24] Cohen, Hamri, Geva, Globerson 2023, "LM vs LM: Detecting Factual Errors via Cross Examination" (abstract). https://arxiv.org/abs/2305.13281
- [S25] Gu et al. 2024, "A Survey on LLM-as-a-Judge" (abstract only; not relied on). https://arxiv.org/abs/2411.15594
- [S26] Huang et al. 2023, "A Survey on Hallucination in Large Language Models", ACM TOIS (abstract only). https://arxiv.org/abs/2311.05232
- [S27] Walters and Wilder search-result corroboration (DOAJ and CUNY Academic Works records); used only to confirm the headline figures in [S7].
