# Does "derive once" have a formal name, standard, or literature?

Short answer for the report writer: the idea is well named in the literature but not in any binding standard. The closest names are DRY (Hunt and Thomas), SPOT / Single Point of Truth (Raymond, after Kernighan), Once and Only Once (Beck / XP), and "system of record vs derived data" (Kleppmann). Database normalization is the oldest formal version of it, for stored data only. ISO/IEC 5055 is the only standard found that measures duplication, and it measures copy-pasted code, not duplicated rules or counts. No standard requires "one owner per fact" or treats two disagreeing on-screen counts as a defect.

## 1. DRY: exact wording, edition, and the "knowledge, not code" clarification

### Takeaway
The exact DRY wording is confirmed from the publisher's own excerpt of the 20th Anniversary Edition. That edition explicitly says the first edition was misread as "don't copy-paste code", and that DRY is about duplicated knowledge and intent. This is the closest published match to "derive once", including the rule that a derived value should be computed, not stored as a second copy.

### Cited Findings
- Exact definition, 20th Anniversary Edition (Thomas and Hunt, Pearson/Addison-Wesley, copyright 2020, ISBN-13 978-0-13-595705-9), Topic 9 "The Evils of Duplication", Tip 15: "Every piece of knowledge must have a single, unambiguous, authoritative representation within a system." — [PragProg excerpt PDF](https://media.pragprog.com/titles/tpp20/dry.pdf)
- Same edition, page 4 of the excerpt, section "DRY is More Than Code": the authors say that in the first edition they "did a poor job of explaining" DRY, and many readers took it to mean only "don't copy-and-paste lines of source", which they call "a tiny and fairly trivial part". — [PragProg excerpt PDF](https://media.pragprog.com/titles/tpp20/dry.pdf)
- Same section: "DRY is about the duplication of knowledge, of intent." The acid test given: when one facet changes, do you have to change it in several places and formats (code and docs, schema and the structure that holds it)? — [PragProg excerpt PDF](https://media.pragprog.com/titles/tpp20/dry.pdf)
- Same topic, page 6, "Not All Code Duplication is Knowledge Duplication": two identical validation functions (age and quantity) are "a coincidence, not a duplication", because they encode different knowledge. — [PragProg excerpt PDF](https://media.pragprog.com/titles/tpp20/dry.pdf)
- Same topic, page 8, "DRY Violations in Data": a `Line` class that stores `length` next to `start` and `end` is a DRY violation; the fix is to make length a calculated field. If you later cache it for speed, "localize the impact" so only the owning class keeps it in sync. — [PragProg excerpt PDF](https://media.pragprog.com/titles/tpp20/dry.pdf)
- The same section cites Meyer's Uniform Access Principle (Object-Oriented Software Construction): callers should not be able to tell whether a value is stored or computed. — [PragProg excerpt PDF](https://media.pragprog.com/titles/tpp20/dry.pdf)
- The same definition sentence is reused verbatim by Raymond for SPOT and credited to The Pragmatic Programmer (see section 2). — [Art of Unix Programming, SPOT rule](https://www.linuxtopia.org/online_books/programming_books/art_of_unix_programming/ch04s02_2.html)

### Inferences
- The user's rule maps almost one-to-one onto 20th-edition DRY: "single authoritative representation" is the owner; "knowledge, not code" is why two displays computing the same count separately is a violation even when the code looks different; the `Line.length` example is the "derive, don't store a second copy" rule.
- One difference: the user's rule adds "a missing fact gets a stored field instead of being re-derived elsewhere". DRY's `Line` example points the other way for derivable values (compute, don't store). The two agree once you separate source facts (store once) from derived values (compute once, in one owner). The user's stored-field rule is about facts that cannot be recomputed from what is stored, which DRY would also want stored once.

### Gaps
- First edition (1999) page number and tip number for the definition were not verified from a primary copy. Commonly cited as the same sentence in "The Evils of Duplication"; I could not confirm the page or tip number, so do not cite one.

## 2. Related named principles and how close each is

### Takeaway
Several named principles say the same thing from different angles. SPOT and DRY share one sentence. Once and Only Once is the XP/refactoring version, aimed at code. Kleppmann's "system of record vs derived data" is the closest data-architecture match. Redux and React state guidance are the closest UI-layer match. Parnas's information hiding is the root idea of "one module owns one decision", but it is about change, not duplication.

### Cited Findings
- **SPOT (Single Point Of Truth)**, Eric S. Raymond, The Art of Unix Programming (2003), ch. 4: restates the DRY sentence and says Raymond renamed it SPOT following a suggestion by Brian Kernighan. Constants, tables and metadata should be declared once and imported. It warns that stale caches and the code that syncs them are a common source of bugs. — [Art of Unix Programming, SPOT rule](https://www.linuxtopia.org/online_books/programming_books/art_of_unix_programming/ch04s02_2.html)
- Raymond also applies SPOT to data structures: states should map one-to-one onto real-world states, and the structure should not be able to represent states that cannot exist. — [search summary of TAOUP ch. 4](http://www.catb.org/esr/writings/taoup/html/) (secondary snippet; I could not fetch the catb.org page, connection refused)
- **Once and Only Once (OAOO)**, Kent Beck / Extreme Programming, recorded on the original c2 wiki: "say everything once and only once"; it is one of the rules of simple design and is aimed at the program itself. — [xp.c2.com OnceAndOnlyOnce](http://xp.c2.com/OnceAndOnlyOnce.html); [O'Reilly, Extreme Programming Pocket Guide ch. 19](https://www.oreilly.com/library/view/extreme-programming-pocket/9781449399849/ch19.html). Beck is quoted as saying that designs without duplication are easy to change — [Beck, "Embracing change with extreme programming", IEEE Computer 1999 (PDF)](https://www.cs.kent.edu/~jmaletic/cs63902/Papers/Beck99.pdf) (quote surfaced via search snippet; not read in full).
- **System of record vs derived data**, Martin Kleppmann, Designing Data-Intensive Applications (O'Reilly, 2017), Part III intro: a system of record (source of truth) holds the authoritative version; each fact is represented exactly once, typically normalized; if another system disagrees, the system of record is by definition correct. Derived data can be rebuilt from the source if lost. — [O'Reilly Part III page](https://www.oreilly.com/library/view/designing-data-intensive-applications/9781491903063/part03.html) (page returned 403 to me; wording taken from search snippet of that page)
- **Redux style guide**, "Keep State Minimal and Derive Additional Values": keep the stored state minimal and derive values such as filtered lists, sums and "number of todos remaining" from it, usually in selector functions. It also says the "single source of truth" principle "has been over-interpreted" — it does not mean every value must live in the store. — [Redux Style Guide](https://redux.js.org/style-guide/)
- **React docs**, "Choosing the State Structure": "Avoid redundant state" — if a value can be calculated from props or state during render, do not put it in state. "Avoid duplication in state" — duplicated data is hard to keep in sync. The page compares this to database normalization. — [react.dev](https://react.dev/learn/choosing-the-state-structure)
- **Information hiding**, David Parnas, "On the Criteria To Be Used in Decomposing Systems into Modules", Communications of the ACM 15(12), Dec 1972 (pp. 1053–1058): each module is characterized by its knowledge of a design decision which it hides from all others; start from a list of hard design decisions or ones likely to change. — [MIT copy of the paper](http://sunnyday.mit.edu/16.355/parnas-criteria.html) (fetch failed; wording from search snippet), [The Morning Paper summary](https://blog.acolyer.org/2016/09/05/on-the-criteria-to-be-used-in-decomposing-systems-into-modules/)
- **Make illegal states unrepresentable**, Yaron Minsky (Jane Street, Effective ML talk, 2010): use types so invalid combinations of data cannot be built. — [functional-architecture.org](https://functional-architecture.org/make_illegal_states_unrepresentable/); [effective-ml notes](https://gist.github.com/jonschoning/8394412)
- **Database normalization**: William Kent, "A Simple Guide to Five Normal Forms in Relational Database Theory", CACM 26(2), Feb 1983, pp. 120–125. His 2NF example: a warehouse address repeated in every part record can become inconsistent, with records showing different addresses for one warehouse. — [Kent, simple5](https://www.bkent.net/Doc/simple5.htm). The common teaching summary is "one fact, one place" and the failure is called an "update anomaly". — [Built In, normalization vs denormalization](https://builtin.com/articles/denormalization) (secondary)

### Inferences
- Closeness ranking to "derive once": (1) DRY 20th ed. and SPOT — same rule, same words; (2) Kleppmann system of record / derived data — same rule for data, and it states the tie-break ("the system of record wins"), which matches "the owner's number is right"; (3) React/Redux "derive, don't store" — same rule at the UI layer; (4) normalization — same rule for stored tables only; (5) OAOO — same instinct, but aimed at code; (6) Parnas — explains *why* one owner helps (a change lands in one place), but says nothing about duplicates directly; (7) illegal states unrepresentable — a neighbor (prevent contradictions by construction) rather than the same rule.
- Kent's warehouse example is the database version of "two screens show different numbers for one thing": two copies of one fact that drifted apart.

### Gaps
- I could not read the catb.org, MIT (Parnas) or O'Reilly (Kleppmann) pages directly; their wording above comes from search snippets of those pages or mirrors. Codd's original 1970/1971 papers were not checked.

## 3. Formal standards: do any require single-source or forbid duplication?

### Takeaway
No standard found requires "one owner per fact". ISO/IEC 5055:2021 (from CISQ) is the only one that measures duplication: it counts copy-pasted code (CWE-1041) and lists "duplicated business logic" as a system-level maintainability weakness. ISO/IEC 25010 names the quality (maintainability, modularity) but sets no duplication rule. MISRA C has a narrow "declare once" rule for C identifiers.

### Cited Findings
- **ISO/IEC 5055:2021** (Automated Source Code Quality Measures, based on CISQ). Its maintainability measure is built from CWE weaknesses; one is **CWE-1041 "Use of Redundant Code"**: "The product has multiple functions, methods, procedures, macros, etc. that contain the same code." CWE-1041 is a member of CWE-1307 "CISQ Quality Measures - Maintainability" and of CWE-1130 (CISQ 2016). Mitigation: merge common logic into one function and call it everywhere. MITRE marks it prohibited for vulnerability mapping because it is a quality issue. — [CWE-1041](https://cwe.mitre.org/data/definitions/1041.html)
- Reported default threshold for CWE-1041 in the ISO 5055 measure: copy-pasted instructions may be at most 10% of the instance's instructions, adjustable before analysis. — search snippet of [ISO 5055 weakness descriptions PDF](http://selab.ise.shibaura-it.ac.jp/ISO%205055%20Weakness%20Descriptions%20by%20Measure.pdf) (PDF itself would not load; treat as unverified)
- CISQ's own page lists maintainability problems at unit level ("Unstructured and duplicated code", "Hard coding of literals") and at system level ("Duplicated business logic", "Compliance with initial architecture design"). — [CISQ Code Quality Standards](https://www.it-cisq.org/standards/code-quality-standards/)
- The maintainability measure is reported to contain 29 parent weaknesses. — search snippet summarizing [ISO 5055 weakness descriptions PDF](http://selab.ise.shibaura-it.ac.jp/ISO%205055%20Weakness%20Descriptions%20by%20Measure.pdf) (unverified)
- **ISO/IEC 25010:2023** defines maintainability as the degree of effectiveness and efficiency with which a product can be modified by its maintainers, with five subcharacteristics: modularity, reusability, analysability, modifiability, testability. Modularity: composed of discrete components so a change to one has minimal impact on others. No duplication rule appears in these definitions. — [arc42 quality model, ISO 25010](https://quality.arc42.org/standards/iso-25010); [SonarSource explainer](https://www.sonarsource.com/resources/library/iso-iec-25010-explained/)
- **MISRA C:2012 Rule 8.5** (Required): "An external object or function shall be declared once in one and only one file" — one declaration in one header keeps declarations and definition consistent. — [MathWorks Polyspace, Rule 8.5](https://www.mathworks.com/help/bugfinder/ref/misrac2012rule8.5.html)

### Inferences
- ISO 5055 catches the *code* half of the user's rule (copy-paste) and names the *knowledge* half ("duplicated business logic") only as a category. A clone detector will not find two different-looking functions that compute the same count, which is the case the user's reviews target.
- ISO 25010 gives vocabulary (modularity, modifiability, analysability) to justify the rule, but not the rule itself.
- MISRA 8.5 and C++'s One Definition Rule are "single source" rules for declarations only, not for business facts.

### Gaps
- ISO/IEC/IEEE 12207 (life-cycle processes), IEEE 1012 (verification and validation) and SEI CERT coding standards: I found no duplication or single-source rule in them. These are process or security standards; I did not read their full text, so this is "not found", not "confirmed absent".
- The full ISO 5055 text is paywalled; the exact CWE list and threshold wording should be checked against the standard before quoting.

## 4. Is there a name for "two displays disagree about one count"?

### Takeaway
There is no single standard name for it. Three nearby terms exist: "update anomaly" (database), "inconsistency with the system of record" (Kleppmann), and Nielsen's "consistency and standards" heuristic, which is about words and conventions, not numbers.

### Cited Findings
- Nielsen heuristic #4, "Consistency and standards" (set refined in 1994): "Users should not have to wonder whether different words, situations, or actions mean the same thing. Follow platform and industry conventions." — [NN/g, 10 Usability Heuristics](https://www.nngroup.com/articles/ten-usability-heuristics/)
- Nielsen heuristic #1, "Visibility of system status": keep users informed through appropriate feedback. — [NN/g](https://www.nngroup.com/articles/ten-usability-heuristics/)
- Database term: an update anomaly is when one fact is stored in several places and only some copies change; Kent's warehouse example shows different records giving different addresses for one warehouse. — [Kent, simple5](https://www.bkent.net/Doc/simple5.htm); [Built In](https://builtin.com/articles/denormalization)
- Kleppmann: when a derived system disagrees with the system of record, the system of record is correct by definition. — [O'Reilly Part III](https://www.oreilly.com/library/view/designing-data-intensive-applications/9781491903063/part03.html) (snippet)
- Pragmatic Programmer frames the failure as a contradiction: two expressions of the same thing, one changed, the program "brought to its knees". — [PragProg excerpt PDF](https://media.pragprog.com/titles/tpp20/dry.pdf)

### Inferences
- The user's "two counts on screen disagree = bug" is best described as a visible update anomaly, or a derived view drifting from its system of record. Nielsen #4 supports it only loosely, since the heuristic is about meaning and conventions, not numeric agreement.

### Gaps
- I found no usability standard (ISO 9241 etc.) that names numeric disagreement between screens. Not searched in depth.

## 5. Counter-arguments and how the literature reconciles them

### Takeaway
The main pushback (Rule of Three, Sandi Metz's "wrong abstraction", WET) is about merging code that merely looks alike. The 20th-edition DRY text resolves this itself: coincidental code similarity is not knowledge duplication. So the critiques do not apply to one fact computed in two places, which is the user's target.

### Cited Findings
- Sandi Metz, RailsConf 2014 talk "All the Little Things", expanded in the blog post "The Wrong Abstraction" (20 Jan 2016): "duplication is far cheaper than the wrong abstraction"; advice: "prefer duplication over the wrong abstraction"; if the abstraction is wrong, go back to duplicating. — [Sandi Metz blog](https://sandimetz.com/blog/2016/1/20/the-wrong-abstraction)
- Rule of Three, attributed to Don Roberts and popularized by Martin Fowler in Refactoring: do it once, wince the second time, refactor the third time. Premature extraction risks a wrong abstraction. — [Wikipedia, Rule of three](https://en.wikipedia.org/wiki/Rule_of_three_(computer_programming)) (pointer only; primary is Fowler, Refactoring, 1999)
- WET ("Write Everything Twice") is argued in practitioner posts as a reaction to over-applied DRY. — [DEV Community, "Stop trying to be so DRY"](https://dev.to/wuz/stop-trying-to-be-so-dry-instead-write-everything-twice-wet-5g33) (opinion piece)
- Reconciliation in the primary DRY text: identical validators for age and quantity are "a coincidence, not a duplication". — [PragProg excerpt PDF](https://media.pragprog.com/titles/tpp20/dry.pdf)
- Redux itself warns its "single source of truth" principle "has been over-interpreted". — [Redux Style Guide](https://redux.js.org/style-guide/)

### Inferences
- The clean line the literature draws: merge when two places encode the *same fact or rule* (knowledge duplication); leave alone when two places merely share *shape* (incidental similarity). The user's "derive once" rule sits on the first side, so Metz and the Rule of Three are not arguments against it. They are arguments against forcing shared helpers onto look-alike code that answers different questions.

### Gaps
- No empirical study was found that measures bug rates from duplicated derivations vs wrong abstractions. Not searched in depth.

## 6. Tools that enforce single-source rules

### Takeaway
Tools enforce the code-copy half well and the knowledge half poorly. Clone detectors find copy-paste. Architecture-test libraries (ArchUnit) and custom source-scan tests can enforce "only the owner may compute X". No off-the-shelf tool finds two different implementations of one business rule.

### Cited Findings
- PMD CPD: token-based copy-paste detection using Karp-Rabin matching, with a configurable minimum token count (default 100). — [DEV Community, duplicate code checkers](https://dev.to/rahulxsingh/13-best-duplicate-code-checker-tools-in-2026-1cnk) (secondary)
- SonarQube: built-in duplication metrics (duplicated lines, blocks, files, density) that feed quality gates; a 3% duplication threshold on new code is the cited default, and a mining study found duplicated-lines density used in 88.3% of SonarQube Cloud quality gates studied. — [DEV Community](https://dev.to/rahulxsingh/13-best-duplicate-code-checker-tools-in-2026-1cnk); [arXiv 2508.18816](https://arxiv.org/pdf/2508.18816)
- ArchUnit: unit tests over compiled Java bytecode that check package and class dependencies, cycles and custom architectural rules (for example "controllers must not depend on repositories"). — [reflectoring.io](https://reflectoring.io/enforce-architecture-with-arch-unit/)
- Redux recommends putting derivations in memoized selector functions (Reselect / `createSelector`), giving each derived value one named owner. — [Redux Style Guide](https://redux.js.org/style-guide/)

### Inferences
- An ArchUnit-style rule ("only module X may call the raw counting query") or a source-scan test is the practical way to enforce "derive once" mechanically; clone detectors only catch the trivial case.

### Gaps
- CQRS read models were not researched in this pass; no sources gathered, so no claims are made about them.
