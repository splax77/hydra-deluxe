# Derive-once review gate Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers-extended-cc:subagent-driven-development (recommended) or superpowers-extended-cc:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Code reaches `main` only after a fresh agent has checked that it doesn't answer a question the codebase already answers, and copies that a grep can catch fail the test suite.

**Architecture:** Two layers. The first is a table-driven test in the repo, `tests/test_single_owner.cpp`. Each row names a question, the one file allowed to answer it, and the text that answers it. Known copies sit in a baseline list that can only shrink. The second is a hook at user scope. It denies `git merge` and `git commit` on `main` until a reviewer agent has submitted a `CLEAN` verdict for that exact change, or until you waive it by typing a phrase in chat.

**Tech Stack:** C++17 and doctest for the scan test. PowerShell 5 hooks in `C:\Users\Patrick\.claude\hooks`, run through the existing `dispatch_bash_ps.ps1` dispatcher. Git for diff ranges, commit trailers and tree hashes.

**Spec:** This plan has no separate spec. It comes from the 2026-10-03 conversation. You asked "how can i enforce claude to actually write good code in the future?" and then "write the review gate plan". The design argued there: rules written as text don't hold, checks that fail on their own do, and a scan can't catch the same rule in different words, so a second agent reviews before merge.

## Why this exists

On 2026-10-03 a session promised that long-path handling would live in one place. The conversion did. But the separate question "does the Windows shell take this path" ended up written five times, and nothing failed. The same day's Preview speedups added four more second copies of existing rules. Each came with tests proving identical results, so the copies passed.

The audit history shows why reading alone doesn't fix this. The 2026-09 audit missed a double rating inside one function. The 2026-10-03 audit kept finding new items in every completeness round. The fixes have to sit outside the model that writes the code.

## Prior art

**A pre-commit review gate** is a published pattern for Claude Code ([IMTI, "The Pre-Commit Review Gate"](https://imti.co/pre-commit-review-gate/)). A PreToolUse hook denies `git commit` until an adversarial sub-agent has reviewed the diff and written a `Verdict: CLEAN` artifact tied to that exact diff. The deny message carries the reviewer prompt. This plan copies that shape. It adds three things the article doesn't have: the reviewer must not be an author, a review can't be written by the main session, and only you can waive it.

**Architecture fitness functions** are tests that guard a structural rule ([InfoQ](https://www.infoq.com/articles/fitness-functions-architecture/); [ArchUnit](https://www.archunit.net/) is the Java version). C++ has no ArchUnit equivalent. The usual answer is a custom source scan, which is what `tests/test_long_paths.cpp` already does for `win32_path`. This plan generalises that one scan into a table.

**Baselines that only shrink** come from static analysers such as PHPStan. The baseline records every existing violation, and the check fails on any new one ([PHPStan baseline burn-down](https://richdynamix.com/articles/phpstan-baseline-burn-down-legacy-laravel)). This plan adds the reverse check too: a baseline entry that no longer matches fails, so a fixed copy has to be removed from the list.

## Global Constraints

- A hook never answers `ask` inside a subagent. Nobody can answer the prompt, so the agent hangs forever. Every hook here prints either `{}` or a `deny`.
- `fable_delegate_gate.ps1` limits the orchestrator's own non-doc edits (3,000 characters per edit, 10,000 per turn). Hook scripts and test code are written by executor subagents.
- Merge Tasks 1 and 2 into `main` before Task 4 registers the gate. After Task 4, every code merge needs a review, this plan's included.
- A subagent can't edit `C:\Users\Patrick\.claude\settings.json`. The orchestrator does the registration steps in Task 4 itself.
- `C:\Users\Patrick\.claude` is not a git repo. Hook changes are unversioned, so each hook gets its test file in `hooks\tests`, as the existing nine hooks do.
- Stage files by name, never `git add -A`. Other sessions leave edits in this checkout.
- Every commit carries `Task:`, `Agent:` and `Session:` trailers (the existing `git_attribution_gate.ps1`).
- Every agent brief carries the status-line rule: append one line to `C:\Users\Patrick\.claude\hooks\state\status\<agent id>.md` every 10 tool calls or 5 minutes.
- Nothing in Hydra's own behaviour changes. This plan only adds a test, a doc, a marker file and hooks.

**User decisions (already made):**
- "add both to the fix list, write the review gate plan" (2026-10-03). The shell path-length fold is on the audit's fix list. It is not part of this plan. This plan only adds its scan rule, with today's copies in the baseline.
- Gate scope: "Merges + commits on main" (2026-10-03, decision 1 below).
- Findings: "Block until fixed or waived" (2026-10-03, decision 2 below).
- Old findings: "Don't block, list it" (2026-10-03, decision 3 below).
- Gated paths widened at install (2026-10-03, session ade9655b): `third_party/`, `installer/`, `CMakePresets.json` and `build_cpp.ps1` are gated too, as is the marker file itself. The one owner of the list is `Test-DeriveOnceGatedPath` in `hooks\lib\derive_once_rules.ps1`; the marker and CLAUDE.md point at it rather than restating it.
- Execution: "Subagent-Driven (this session)" (2026-10-03).

**Decisions 1 to 3 below were answered as recommended on 2026-10-03.** Decision 4 was not asked; the plan uses its default (Opus) unless the user says otherwise.

1. **What the gate covers.** Default: `git merge` while on `main`, and `git commit` while on `main`, when the change touches `src/`, `tests/`, `tools/` or `CMakeLists.txt`. Docs-only changes pass freely. Commits on worktree branches pass freely; the review happens when they merge. Alternative: merges only, so direct commits on `main` go unchecked.
2. **What blocks.** Default: a `FINDINGS` verdict blocks until the code is fixed and reviewed again, or you waive it. Alternative: findings are reported but never block.
3. **Old findings the change only moves.** Default: a copy the change *adds* or *extends* blocks. A copy already listed in `docs/audit/2026-10-03-derivation-audit.md` that the change only moves is reported under "Touched, already on the fix list" and doesn't block. Blocking on those would hold up every merge in a file with old findings. Alternative: any touched copy blocks.
4. **Reviewer model and cost.** Default: the reviewer runs on the session's model (Opus), as the audit verifiers did. One review reads the diff plus the code it touches. Expect about 100k to 250k tokens per merge, judging by today's verifier runs (160k to 240k each, with a wider brief). Alternative: Sonnet, which is cheaper and weaker at "same rule, different words".

---

## File map

| File | New or changed | What it does |
|---|---|---|
| `tests/test_single_owner.cpp` | new | The rule table, the baseline of known copies, the scan, and a self-test that every rule matches its own examples |
| `CMakeLists.txt` | changed | Adds the new test file to `hydra_tests` |
| `docs/agents/derive-once-review.md` | new | The reviewer's brief: method, output format, how to submit |
| `.derive-once-gate` | new | Marker file. The hook gates only repos that have it |
| `CLAUDE.md` | changed | One short section pointing to the review brief |
| `C:\Users\Patrick\.claude\hooks\derive_once_review_gate.ps1` | new | Records submitted reviews; denies merges and commits on `main` without one |
| `C:\Users\Patrick\.claude\hooks\derive_once_submit.ps1` | new | Copies a review into the hooks state folder so you can read it |
| `C:\Users\Patrick\.claude\hooks\derive_once_waiver.ps1` | new | Records a waiver when you type `waive derive-once <key>` |
| `C:\Users\Patrick\.claude\hooks\tests\test_h15_derive_once.ps1` | new | Tests for the three hooks against a throwaway repo |
| `C:\Users\Patrick\.claude\hooks\dispatch_bash_ps.ps1` | changed | Runs the new gate with the other Bash/PowerShell gates |
| `C:\Users\Patrick\.claude\settings.json` | changed | Registers the waiver hook and allows the submit helper |

---

### Task 1: The single-owner scan test

**Goal:** A table-driven test fails the build when a line outside a rule's owner file answers that rule's question, unless the line is in a baseline that can only shrink.

**Files:**
- Create: `tests/test_single_owner.cpp`
- Modify: `CMakeLists.txt:429` (add the file after `tests/test_long_paths.cpp`)

**Acceptance Criteria:**
- [ ] `hydra_tests.exe -tc="single-owner*"` passes on today's `main`, with the six known copies listed in the baseline.
- [ ] Adding a line `if (p.size() < MAX_PATH) return;` to any `src/` file other than `src/core/winstr.cpp` makes `single-owner rules hold across src/ and tools/` fail and name that file and line.
- [ ] Deleting one baselined line (for example `long n = std::ftell(f);` in `src/parse/midi.cpp`) makes the same test fail with "baseline entry no longer matches".
- [ ] `single-owner rules match their own examples` fails if any rule's pattern misses one of its `must_match` lines or hits one of its `must_not_match` lines.
- [ ] The full `hydra_tests.exe` run has no new failures.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="single-owner*"` → `[doctest] Status: SUCCESS!`

**Steps:**

- [ ] **Step 1: Write the test file.** The first rule is the shell path-length question from today's fix list. The second is "how big is this file", owned by `read_file_bytes` since the 2 GB fix. The third is the long-path prefix, owned by `win32_path`. Each rule carries example lines. The self-test proves the pattern matches what the rule claims, which is the gap the long-path scan had (`recursive_directory_iterator` slipped past it). Baseline entries are keyed by file and trimmed line text, because line numbers move.

```cpp
// One owner per rule, checked where a grep can check it. Each row names a
// question, the only file allowed to answer it, and a pattern for the text
// that answers it. A matching line anywhere else fails here, unless the
// baseline lists it with the fix that will remove it. A baseline entry that
// no longer matches also fails, so the list only shrinks.
#include "doctest.h"

#include <filesystem>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {

struct OwnerRule {
    std::string question;               // plain English, shown on failure
    std::string pattern;                // ECMAScript regex, matched per line
    std::vector<std::string> owners;    // repo-relative files allowed to match
    std::string decided_by;             // ADR, CONTEXT.md or the user's words
    std::vector<std::string> must_match;
    std::vector<std::string> must_not_match;
};

struct KnownCopy {
    std::string question;   // equals a rule's question
    std::string file;       // repo-relative, forward slashes
    std::string line_text;  // the line with leading and trailing space trimmed
    std::string removed_by; // the fix that deletes it
};

const std::vector<OwnerRule>& rules() {
    static const std::vector<OwnerRule> r = {
        {"Does the Windows shell take a path this long?",
         R"([<>]=?\s*MAX_PATH\b)",
         {"src/core/winstr.cpp"},
         "ADR 0020; one-owner fix on the 2026-10-03 fix list",
         {"if (path.size() < MAX_PATH) return path;",
          "if (copy.native().size() >= MAX_PATH) return {};",
          "return s.size() < MAX_PATH ? s : L\"\";"},
         {"wchar_t tmp[MAX_PATH + 1];", "GetTempPathW(MAX_PATH + 1, tmp);"}},
        {"How many bytes does a file hold?",
         R"((^|[^\w])(std::)?f(tell|seek)\s*\()",
         {"src/core/winstr.cpp"},
         "ADR 0020 (read_file_bytes reads files over 2 GB)",
         {"long n = std::ftell(f);", "std::fseek(f, 0, SEEK_END);", "fseek(f, 0, SEEK_SET);"},
         {"_fseeki64(f, 0, SEEK_END);", "const long long n = _ftelli64(f);"}},
        {"How is a long path prefixed for Win32?",
         R"(\\\\\\\\\?\\\\)",
         {"src/core/winstr.cpp"},
         "ADR 0020",
         {"return L\"\\\\\\\\?\\\\\" + full;"},
         {"return L\"\\\\\\\\\" + s.substr(8);"}},
    };
    return r;
}

const std::vector<KnownCopy>& known_copies() {
    static const std::vector<KnownCopy> k = {
        {"Does the Windows shell take a path this long?", "src/app/report_files.cpp",
         "if (n == 0 || n > MAX_PATH) return {};",
         "fix list: one owner for the shell path limit"},
        {"Does the Windows shell take a path this long?", "src/app/report_files.cpp",
         "if (copy.native().size() >= MAX_PATH) return {};",
         "fix list: one owner for the shell path limit"},
        {"Does the Windows shell take a path this long?", "src/app/report_files.cpp",
         "if (path.size() < MAX_PATH) return shell_open(path);",
         "fix list: one owner for the shell path limit"},
        {"How many bytes does a file hold?", "src/parse/midi.cpp",
         "std::fseek(f, 0, SEEK_END);", "round 7 candidate N2-5 (MidiFile::from_file)"},
        {"How many bytes does a file hold?", "src/parse/midi.cpp",
         "long n = std::ftell(f);", "round 7 candidate N2-5 (MidiFile::from_file)"},
        {"How many bytes does a file hold?", "src/parse/midi.cpp",
         "std::fseek(f, 0, SEEK_SET);", "round 7 candidate N2-5 (MidiFile::from_file)"},
    };
    return k;
}

std::string trim(const std::string& s) {
    const size_t a = s.find_first_not_of(" \t\r");
    if (a == std::string::npos) return {};
    const size_t b = s.find_last_not_of(" \t\r");
    return s.substr(a, b - a + 1);
}

}  // namespace

TEST_CASE("single-owner rules match their own examples") {
    for (const OwnerRule& r : rules()) {
        const std::regex re(r.pattern);
        for (const std::string& line : r.must_match) {
            INFO(r.question << " should match: " << line);
            CHECK(std::regex_search(line, re));
        }
        for (const std::string& line : r.must_not_match) {
            INFO(r.question << " should not match: " << line);
            CHECK_FALSE(std::regex_search(line, re));
        }
    }
    // Every baseline entry names a real rule.
    std::set<std::string> questions;
    for (const OwnerRule& r : rules()) questions.insert(r.question);
    for (const KnownCopy& c : known_copies()) {
        INFO("baseline entry names no rule: " << c.question);
        CHECK(questions.count(c.question) == 1);
    }
}

TEST_CASE("single-owner rules hold across src/ and tools/") {
    const fs::path root = fs::u8path(HYDRA_SOURCE_DIR);
    std::vector<std::pair<OwnerRule, std::regex>> compiled;
    for (const OwnerRule& r : rules()) compiled.emplace_back(r, std::regex(r.pattern));

    std::set<size_t> seen;  // indexes into known_copies() that matched a line
    std::vector<std::string> problems;
    int files = 0;
    for (const char* sub : {"src", "tools"}) {
        for (const fs::directory_entry& e : fs::recursive_directory_iterator(root / sub)) {
            const fs::path ext = e.path().extension();
            if (ext != ".cpp" && ext != ".h") continue;
            ++files;
            const std::string rel = fs::relative(e.path(), root).generic_u8string();
            std::ifstream in(e.path());
            std::string line;
            int lineno = 0;
            while (std::getline(in, line)) {
                ++lineno;
                const std::string t = trim(line);
                if (t.empty() || t.compare(0, 2, "//") == 0) continue;
                for (const auto& [rule, re] : compiled) {
                    bool owner = false;
                    for (const std::string& o : rule.owners) owner = owner || rel == o;
                    if (owner || !std::regex_search(line, re)) continue;
                    bool known = false;
                    for (size_t i = 0; i < known_copies().size(); ++i) {
                        const KnownCopy& c = known_copies()[i];
                        if (c.question == rule.question && c.file == rel && c.line_text == t) {
                            seen.insert(i);
                            known = true;
                        }
                    }
                    if (!known)
                        problems.push_back(rel + ":" + std::to_string(lineno) + ": answers \"" +
                                           rule.question + "\", which belongs to " +
                                           rule.owners.front() + ": " + t);
                }
            }
        }
    }
    for (size_t i = 0; i < known_copies().size(); ++i) {
        if (seen.count(i)) continue;
        const KnownCopy& c = known_copies()[i];
        problems.push_back("baseline entry no longer matches (remove it): " + c.file + ": " +
                           c.line_text);
    }
    CHECK(files > 100);  // the scan found the sources
    std::ostringstream report;
    for (const std::string& p : problems) report << p << "\n";
    INFO(report.str());
    CHECK(problems.empty());
}
```

- [ ] **Step 2: Register the file.** In `CMakeLists.txt`, add `tests/test_single_owner.cpp` on the line after `tests/test_long_paths.cpp` (line 429). `HYDRA_SOURCE_DIR` is already defined for the whole `hydra_tests` target at line 456. Update that line's comment to say both tests use it:

```cmake
    HYDRA_SOURCE_DIR="${CMAKE_SOURCE_DIR}"  # test_long_paths and test_single_owner scan src/ and tools/
```

- [ ] **Step 3: Build and run.** Run `.\build_cpp.ps1 -Target hydra_tests` and then `.\build-cpp\Release\hydra_tests.exe -tc="single-owner*"`. Expected: both cases pass. If the scan reports a copy not in the baseline, don't widen the pattern or add an owner to make it pass. Report the line to the orchestrator. It is either a new finding or a pattern bug.

- [ ] **Step 4: Prove it bites.** Temporarily add `if (p.size() < MAX_PATH) return;` inside any function in `src/app/report_files.cpp`, rebuild, and confirm the test fails and names that line. Revert it. Then temporarily delete `long n = std::ftell(f);` from `src/parse/midi.cpp` (the build may fail; if so, change it to `long n = 0;` instead), rebuild, confirm "baseline entry no longer matches", and revert. Paste both failure outputs into your final report.

- [ ] **Step 5: Run the whole suite.** Run `.\build-cpp\Release\hydra_tests.exe`. Expected: no failures beyond any that already fail on `main` before your change. Compare against a run on `main` if anything fails.

- [ ] **Step 6: Commit.**

```bash
git add tests/test_single_owner.cpp CMakeLists.txt
git commit -m "Single-owner scan test: one file per rule, baseline only shrinks" --trailer "Task: single-owner-scan" --trailer "Agent: <your agent id>" --trailer "Session: <session id>"
```

---

### Task 2: The reviewer brief, the marker file and the CLAUDE.md pointer

**Goal:** A fresh agent given only `docs/agents/derive-once-review.md`, a key and a diff range can produce a review in the exact format the gate reads.

**Files:**
- Create: `docs/agents/derive-once-review.md`
- Create: `.derive-once-gate`
- Modify: `CLAUDE.md` (add one section under "## Agent skills")

**Acceptance Criteria:**
- [ ] The brief states the output format with literal `Key:` and `Verdict: CLEAN` / `Verdict: FINDINGS` lines, matching the gate's regexes in Task 3.
- [ ] The brief carries the status-line sentence with `hooks\state\status`.
- [ ] `.derive-once-gate` exists at the repo root and is tracked.
- [ ] `CLAUDE.md` names the brief and the waiver phrase in plain sentences.

**Verify:** `Select-String -Path docs/agents/derive-once-review.md -Pattern '^Key: ','^Verdict: ','hooks\\state\\status'` → three or more hits; `git ls-files .derive-once-gate` → `.derive-once-gate`

**Steps:**

- [ ] **Step 1: Write the brief.** Create `docs/agents/derive-once-review.md` with exactly this content:

````markdown
# Derive-once review

You are reviewing a change before it reaches `main`. You did not write it. Your one question: does this change answer a question that something else in the codebase already answers?

Append one line to `C:\Users\Patrick\.claude\hooks\state\status\<your agent id>.md` every 10 tool calls or 5 minutes, form `HH:MM done ... | next: ...`. Use `& "C:/Users/Patrick/.claude/hooks/status_append.ps1" <your agent id> "<line>"` to write it.

## The rule

Every fact, rule or calculation in Hydra is derived in exactly one place. Everything else calls that place. It never re-derives the fact, copies the formula, or keeps a parallel constant. Display code reads what the engine stored; it never works a game fact out again.

## What you get

The orchestrator gives you a **key** (a 40-character hash) and a **range**. For a merge the range is `main...<key>`; read it with `git diff main...<key>` and `git log main..<key>`. For a direct commit on `main` the range is the staged change; read it with `git diff --cached`. Read-only: do not edit, stage, commit or check out anything in the repo.

## How to review

1. **List the questions the change answers.** For each added or changed decision, write the question in plain words: "is this note inside the SP window", "how long is this stem", "is this path too long for the shell". A loop, a comparison, a rounding, a fallback, a constant and a cache index each answer a question.
2. **Search by question, not by name.** For each question, look for any other code that answers it, under any name, sign or loop shape. Look inside the changed function too: two copies in one function count. Grep for the inputs the question reads (the fields, the constants, the units) as well as for similar names.
3. **Compare as truth tables.** When you find another answer, write both as inputs → answer and compare the tables. Put the edges in: exactly 0, exactly on a window or leeway edge, exactly 1.0, `<` against `<=`, first against last on ties, ms against ticks, an empty list.
4. **Check new numbers and special cases.** Every new threshold, tolerance, time, ratio, limit, fallback or ordering rule needs a decision from the user: an ADR in `docs/adr/`, `CONTEXT.md`, or the "User decisions" header of a plan. A code comment or a plan's task steps don't count.
5. **Check tests.** A test that recomputes what production code computes, instead of calling it or pinning a literal, is a copy. Frozen copies of old code kept for comparison are copies too.
6. **Check docs the change touches.** A doc that states a rule differently from the code is a finding.
7. **Check what is already known.** Grep `docs/audit/2026-10-03-derivation-audit.md` for the functions involved. A copy listed there that the change only moves goes under "Touched, already on the fix list". A copy the change adds or extends is a finding.

Rules: a grep that finds nothing proves nothing, so say where you looked. Never call code wrong without ground truth (the engine, a test or a run). Every name comes from the repo, not memory. You may run `build-cpp\Release\hydra_tests.exe` or `hydra_replay` to prove a disagreement, one at a time.

## Output

Write your review to a file in your scratchpad, then submit it. The file must contain these two lines exactly, each on its own line:

```
Key: <the 40-character key you were given>
Verdict: CLEAN
```

Use `Verdict: FINDINGS` instead if there is at least one finding. Write `CLEAN` only when there are none. Then, in plain English, one short paragraph each:

- **Questions this change answers**: each one, and the function that owns it after this change.
- **Findings**: for each, the question, every copy (function, file, lines), the truth table when the copies are worded differently, the input where they disagree if they do, and the owner you propose.
- **Touched, already on the fix list**: audit findings this change moves without fixing.
- **Proposed scan rules**: for any finding a grep can guard, a row for `tests/test_single_owner.cpp`: the question, the owner file, and a pattern with two lines it must match and one it must not.
- **Where I looked**: files and functions read.

Submit with:

```
& "C:/Users/Patrick/.claude/hooks/derive_once_submit.ps1" <key> "<full path to your review file>"
```

Your final message: the verdict, the review file path, and one plain sentence per finding.
````

- [ ] **Step 2: Create the marker.** Create `.derive-once-gate` at the repo root with this content:

```
This repo is gated: merges and commits on main that touch src/, tests/, tools/
or CMakeLists.txt need a derive-once review. See docs/agents/derive-once-review.md.
```

- [ ] **Step 3: Point CLAUDE.md at it.** Add this section to `CLAUDE.md` after "### GUI testing":

```markdown
### Derive-once review

Merges and commits on `main` that touch code need a review from a fresh agent first. A hook enforces it. Dispatch the reviewer with `docs/agents/derive-once-review.md`, the key and the range the hook's message gives. Only the user can skip a review, by typing `waive derive-once <key>` in chat. Never ask the user to waive one to save time.
```

- [ ] **Step 4: Commit.**

```bash
git add docs/agents/derive-once-review.md .derive-once-gate CLAUDE.md
git commit -m "Derive-once review brief and gate marker" --trailer "Task: derive-once-brief" --trailer "Agent: <your agent id>" --trailer "Session: <session id>"
```

---

### Task 3: The gate, the submit helper and the waiver hook, with tests

**Goal:** Three hook scripts that, fed hook input on stdin, deny an unreviewed merge or commit on `main` in a gated repo, record a reviewer's submission with the reviewer's real agent id, and record your waiver, all proven by `test_h15_derive_once.ps1` against a throwaway repo.

**Files:**
- Create: `C:\Users\Patrick\.claude\hooks\derive_once_review_gate.ps1`
- Create: `C:\Users\Patrick\.claude\hooks\derive_once_submit.ps1`
- Create: `C:\Users\Patrick\.claude\hooks\derive_once_waiver.ps1`
- Create: `C:\Users\Patrick\.claude\hooks\tests\test_h15_derive_once.ps1`

**Acceptance Criteria:**
- [ ] `test_h15_derive_once.ps1` prints only `PASS` lines and exits 0.
- [ ] The gate never prints `ask`. A grep of the three scripts for `'ask'` finds nothing.
- [ ] A submit by the main session (no `agent_id`) is denied.
- [ ] A `CLEAN` review whose reviewer also appears in an `Agent:` trailer of the merged commits doesn't open the gate.
- [ ] A background-task notification that contains the waiver phrase doesn't create a waiver.
- [ ] A repo without `.derive-once-gate`, a branch other than `main`, and a docs-only change all get `{}`.

**Verify:** `powershell -NoProfile -ExecutionPolicy Bypass -File C:\Users\Patrick\.claude\hooks\tests\test_h15_derive_once.ps1` → all `PASS`, exit code 0

**Steps:**

- [ ] **Step 1: Write the test first.** Create `hooks\tests\test_h15_derive_once.ps1`. It builds a repo in `%TEMP%`, points the hooks at a temp state folder through `DERIVE_ONCE_STATE_DIR`, and checks each case.

```powershell
# Tests for derive_once_review_gate.ps1, derive_once_submit.ps1 and derive_once_waiver.ps1
$ErrorActionPreference = 'Continue'
$hooks = 'C:\Users\Patrick\.claude\hooks'
$gate = Join-Path $hooks 'derive_once_review_gate.ps1'
$waiverHook = Join-Path $hooks 'derive_once_waiver.ps1'
$fail = 0

$root = Join-Path $env:TEMP ('h15-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
$repo = Join-Path $root 'repo'
$plain = Join-Path $root 'plain'
$env:DERIVE_ONCE_STATE_DIR = Join-Path $root 'state'
New-Item -ItemType Directory -Force -Path $repo, $plain | Out-Null

function G([string]$dir, [string[]]$a) { & git -C $dir @a 2>$null | Out-Null }
foreach ($d in @($repo, $plain)) {
  G $d @('init', '-q', '-b', 'main')
  G $d @('config', 'user.email', 't@example.com')
  G $d @('config', 'user.name', 't')
  New-Item -ItemType Directory -Force (Join-Path $d 'src'), (Join-Path $d 'docs') | Out-Null
  Set-Content (Join-Path $d 'src\a.cpp') 'int a;'
}
Set-Content (Join-Path $repo '.derive-once-gate') 'gated'
foreach ($d in @($repo, $plain)) { G $d @('add', '-A'); G $d @('commit', '-q', '-m', 'base') }

G $repo @('checkout', '-q', '-b', 'feat')
Add-Content (Join-Path $repo 'src\a.cpp') 'int b;'
G $repo @('commit', '-q', '-am', 'feat work', '--trailer', 'Agent: writer1')
G $repo @('checkout', '-q', 'main')
G $repo @('checkout', '-q', '-b', 'docsonly')
Set-Content (Join-Path $repo 'docs\x.md') 'doc'
G $repo @('add', 'docs/x.md'); G $repo @('commit', '-q', '-m', 'docs')
G $repo @('checkout', '-q', 'main')
G $plain @('checkout', '-q', '-b', 'feat')
Add-Content (Join-Path $plain 'src\a.cpp') 'int b;'
G $plain @('commit', '-q', '-am', 'feat work')
G $plain @('checkout', '-q', 'main')
$feat = (& git -C $repo rev-parse feat).Trim()

function Invoke-Gate([string]$command, [string]$agentId, [string]$cwd) {
  if (-not $cwd) { $cwd = $repo }
  $msg = @{ session_id = 'test-h15'; cwd = $cwd; hook_event_name = 'PreToolUse'; tool_name = 'PowerShell'; tool_input = @{ command = $command } }
  if ($agentId) { $msg['agent_id'] = $agentId; $msg['agent_type'] = 'general-purpose' }
  return (($msg | ConvertTo-Json -Compress -Depth 6) | powershell -NoProfile -ExecutionPolicy Bypass -File $gate) -join ''
}
function Invoke-Waiver([string]$prompt) {
  $msg = @{ session_id = 'test-h15'; hook_event_name = 'UserPromptSubmit'; prompt = $prompt }
  return (($msg | ConvertTo-Json -Compress -Depth 6) | powershell -NoProfile -ExecutionPolicy Bypass -File $waiverHook) -join ''
}
function Get-Decision([string]$out) {
  $o = $null
  try { $o = $out | ConvertFrom-Json } catch { return 'PARSE-ERROR' }
  if ($o.hookSpecificOutput -and $o.hookSpecificOutput.permissionDecision) { return [string]$o.hookSpecificOutput.permissionDecision }
  return 'none'
}
function Check([string]$name, [string]$got, [string]$want) {
  if ($got -eq $want) { Write-Output "PASS $name" } else { Write-Output "FAIL $name (got $got, want $want)"; $script:fail++ }
}
function New-Review([string]$key, [string]$verdict) {
  $p = Join-Path $root ('review-' + [guid]::NewGuid().ToString('N').Substring(0, 6) + '.md')
  Set-Content $p ("# Derive-once review`nKey: $key`nVerdict: $verdict`n")
  return $p
}
$submit = '& "C:/Users/Patrick/.claude/hooks/derive_once_submit.ps1" '
$merge = 'git merge --no-ff feat -m "Merge feat"'

Check 'unreviewed merge of a src change is denied' (Get-Decision (Invoke-Gate $merge '')) 'deny'
Check 'docs-only merge passes' (Get-Decision (Invoke-Gate 'git merge --no-ff docsonly -m "Merge docs"' '')) 'none'
Check 'merge --abort passes' (Get-Decision (Invoke-Gate 'git merge --abort' '')) 'none'
Check 'repo without the marker gets no opinion' (Get-Decision (Invoke-Gate $merge '' $plain)) 'none'
Check 'the main session cannot submit a review' (Get-Decision (Invoke-Gate ($submit + $feat + ' "' + (New-Review $feat 'CLEAN') + '"') '')) 'deny'
Check 'a review whose Key line differs is refused' (Get-Decision (Invoke-Gate ($submit + $feat + ' "' + (New-Review ('0' * 40) 'CLEAN') + '"') 'reviewer2')) 'deny'
Check 'a FINDINGS review is recorded' (Get-Decision (Invoke-Gate ($submit + $feat + ' "' + (New-Review $feat 'FINDINGS') + '"') 'reviewer2')) 'none'
Check 'merge after a FINDINGS review is denied' (Get-Decision (Invoke-Gate $merge '')) 'deny'
Check 'a review by an author is recorded' (Get-Decision (Invoke-Gate ($submit + $feat + ' "' + (New-Review $feat 'CLEAN') + '"') 'writer1')) 'none'
Check 'merge reviewed only by its own author is denied' (Get-Decision (Invoke-Gate $merge '')) 'deny'
Check 'a CLEAN review by a fresh agent is recorded' (Get-Decision (Invoke-Gate ($submit + $feat + ' "' + (New-Review $feat 'CLEAN') + '"') 'reviewer3')) 'none'
Check 'merge after a fresh CLEAN review passes' (Get-Decision (Invoke-Gate $merge '')) 'none'

# Direct commits on main
Add-Content (Join-Path $repo 'src\a.cpp') 'int c;'
G $repo @('add', 'src/a.cpp')
Check 'unreviewed commit of staged src on main is denied' (Get-Decision (Invoke-Gate 'git commit -m "x"' '')) 'deny'
Check 'commit -am on main is denied' (Get-Decision (Invoke-Gate 'git commit -am "x"' '')) 'deny'
$tree = (& git -C $repo write-tree).Trim()
Invoke-Waiver ('<task-notification> waive derive-once ' + $tree.Substring(0, 10) + ' </task-notification>') | Out-Null
Check 'a notification cannot waive' (Get-Decision (Invoke-Gate 'git commit -m "x"' '')) 'deny'
Invoke-Waiver ('ok, waive derive-once ' + $tree.Substring(0, 10)) | Out-Null
Check 'the user waiver opens the commit' (Get-Decision (Invoke-Gate 'git commit -m "x"' '')) 'none'

# Off main
G $repo @('stash', '-q')
G $repo @('checkout', '-q', 'feat')
Add-Content (Join-Path $repo 'src\a.cpp') 'int d;'
G $repo @('add', 'src/a.cpp')
Check 'a commit on a feature branch passes' (Get-Decision (Invoke-Gate 'git commit -m "x"' '')) 'none'

$env:DERIVE_ONCE_STATE_DIR = $null
Remove-Item -LiteralPath $root -Recurse -Force -ErrorAction SilentlyContinue
if ($fail -gt 0) { exit 1 } else { exit 0 }
```

These tests feed the gate the submit command, but the helper itself never runs here. Step 5 runs the helper for real.

- [ ] **Step 2: Run it and watch it fail.** Run the Verify command. Expected: `FAIL` lines, because the gate script doesn't exist yet.

- [ ] **Step 3: Write the gate.** Create `hooks\derive_once_review_gate.ps1`:

```powershell
# derive_once_review_gate.ps1
# PreToolUse on Bash and PowerShell, run by dispatch_bash_ps.ps1.
# In a repo whose top folder holds .derive-once-gate, code reaches main only
# after a fresh agent's derive-once review says CLEAN, or the user waives it.
# Job 1: a call to derive_once_submit.ps1 is recorded here, with the caller's
#        real agent id. The helper itself only copies the file for the user.
# Job 2: git merge and git commit on main are denied until a matching record
#        or a waiver exists, when the change touches src/, tests/, tools/ or
#        CMakeLists.txt.
# Prints {} when it has no opinion. Never prints ask: a subagent can't answer one.

$ok = '{}'
$stateDir = 'C:\Users\Patrick\.claude\hooks\state\derive_once'
if ($env:DERIVE_ONCE_STATE_DIR) { $stateDir = $env:DERIVE_ONCE_STATE_DIR }  # tests only
$reviewsLog = Join-Path $stateDir 'reviews.jsonl'
$waiversLog = Join-Path $stateDir 'waivers.jsonl'
$brief = 'docs/agents/derive-once-review.md'

function New-Deny([string]$reason) {
  $o = @{ hookSpecificOutput = @{ hookEventName = 'PreToolUse'; permissionDecision = 'deny'; permissionDecisionReason = $reason } }
  return ($o | ConvertTo-Json -Compress -Depth 6)
}

function Invoke-Git([string]$dir, [string[]]$gitArgs) {
  $out = & git -C $dir @gitArgs 2>$null
  if ($LASTEXITCODE -ne 0) { return $null }
  return @($out)
}

function Read-Jsonl([string]$path) {
  if (-not (Test-Path -LiteralPath $path)) { return @() }
  $rows = @()
  foreach ($l in Get-Content -LiteralPath $path) {
    try { $rows += ($l | ConvertFrom-Json) } catch {}
  }
  return $rows
}

try {
  $raw = [Console]::In.ReadToEnd()
  if (-not $raw) { Write-Output $ok; exit }
  $m = $raw | ConvertFrom-Json
  $cmd = [string]$m.tool_input.command
  if ([string]::IsNullOrWhiteSpace($cmd)) { Write-Output $ok; exit }
  $agentId = ''
  if ($m.agent_id) { $agentId = [string]$m.agent_id }

  # ---- job 1: record a submitted review
  $sub = [regex]::Match($cmd, 'derive_once_submit\.ps1["'']?\s+([0-9a-f]{40})\s+(?:"([^"]+)"|''([^'']+)''|(\S+))')
  if ($sub.Success) {
    $key = $sub.Groups[1].Value
    $file = $sub.Groups[2].Value + $sub.Groups[3].Value + $sub.Groups[4].Value
    if (-not $agentId) {
      Write-Output (New-Deny 'A derive-once review has to come from a fresh agent, not the session that wrote the code. Dispatch a reviewer agent with the brief in docs/agents/derive-once-review.md and let it submit.')
      exit
    }
    if (-not (Test-Path -LiteralPath $file)) { Write-Output (New-Deny ('The review file ' + $file + ' does not exist.')); exit }
    $text = Get-Content -LiteralPath $file -Raw
    $kLine = [regex]::Match($text, '(?m)^Key:\s*([0-9a-f]{40})\s*$')
    $vLine = [regex]::Match($text, '(?m)^Verdict:\s*(CLEAN|FINDINGS)\s*$')
    if (-not $kLine.Success -or -not $vLine.Success -or $kLine.Groups[1].Value -ne $key) {
      Write-Output (New-Deny 'The review file needs a "Key: <the key you were given>" line that matches the submit command, and a "Verdict: CLEAN" or "Verdict: FINDINGS" line. Fix the file and submit again.')
      exit
    }
    $hash = (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash
    New-Item -ItemType Directory -Force -Path $stateDir | Out-Null
    $rec = @{ key = $key; verdict = $vLine.Groups[1].Value; reviewer = $agentId; sha256 = $hash; session = [string]$m.session_id; time = (Get-Date).ToString('s') }
    Add-Content -LiteralPath $reviewsLog -Value ($rec | ConvertTo-Json -Compress)
    Write-Output $ok
    exit
  }

  # ---- job 2: gate merges and commits on main
  $gitPre = 'git\s+(?:-[cC]\s+\S+\s+|--\S+(?:=\S+)?\s+)*'
  # Quoted text (commit messages, here-strings) is dropped before parsing, so a
  # message that says "merge" or "-a" can't trip anything.
  $clean = [regex]::Replace($cmd, '@''[\s\S]*?''@|@"[\s\S]*?"@|"[^"]*"|''[^'']*''', ' ')
  $isMerge = $clean -match ($gitPre + 'merge\b')
  $isCommit = $clean -match ($gitPre + 'commit\b')
  if (-not ($isMerge -or $isCommit)) { Write-Output $ok; exit }
  if ($isMerge -and $clean -match '--abort\b|--continue\b|--quit\b') { Write-Output $ok; exit }

  $dir = [string]$m.cwd
  $c = [regex]::Match($cmd, 'git\s+-C\s+(?:"([^"]+)"|''([^'']+)''|(\S+))')
  if ($c.Success) { $dir = $c.Groups[1].Value + $c.Groups[2].Value + $c.Groups[3].Value }
  if (-not $dir -or -not (Test-Path -LiteralPath $dir)) { Write-Output $ok; exit }
  $top = Invoke-Git $dir @('rev-parse', '--show-toplevel')
  if (-not $top) { Write-Output $ok; exit }
  if (-not (Test-Path -LiteralPath (Join-Path $top[0] '.derive-once-gate'))) { Write-Output $ok; exit }
  $branch = Invoke-Git $dir @('rev-parse', '--abbrev-ref', 'HEAD')
  if (-not $branch -or $branch[0] -ne 'main') { Write-Output $ok; exit }

  $key = $null; $files = @(); $authors = @(); $range = ''
  if ($isMerge) {
    $after = [regex]::Match($clean, ($gitPre + 'merge\b([^;&|\r\n]*)')).Groups[1].Value
    foreach ($tok in ($after -split '\s+')) {
      if (-not $tok -or $tok.StartsWith('-')) { continue }
      $sha = Invoke-Git $dir @('rev-parse', '--verify', '--quiet', ($tok + '^{commit}'))
      if ($sha) { $key = $sha[0]; break }
    }
    if (-not $key) { Write-Output $ok; exit }  # nothing nameable; git will complain itself
    $range = 'main...' + $key
    $files = Invoke-Git $dir @('diff', '--name-only', ('HEAD...' + $key))
    $authors = Invoke-Git $dir @('log', '--format=%(trailers:key=Agent,valueonly)', ('HEAD..' + $key))
  } else {
    $mh = Invoke-Git $dir @('rev-parse', '--verify', '--quiet', 'MERGE_HEAD')
    if ($mh) {
      $key = $mh[0]
      $range = 'main...' + $key
      $files = Invoke-Git $dir @('diff', '--name-only', ('HEAD...' + $key))
      $authors = Invoke-Git $dir @('log', '--format=%(trailers:key=Agent,valueonly)', ('HEAD..' + $key))
    } else {
      if ($clean -match ($gitPre + 'commit\b[^;&|\r\n]*\s(?:--all\b|-[a-zA-Z]*a[a-zA-Z]*\b)')) {
        Write-Output (New-Deny 'On main, stage files by name and commit without -a, so the derive-once review covers exactly what you commit.')
        exit
      }
      $files = Invoke-Git $dir @('diff', '--cached', '--name-only')
      $tree = Invoke-Git $dir @('write-tree')
      if (-not $tree) { Write-Output $ok; exit }
      $key = $tree[0]
      $range = 'the staged change (git diff --cached)'
      if ($agentId) { $authors = @($agentId) } else { $authors = @('orchestrator') }
    }
  }

  $src = @($files | Where-Object { $_ -match '^(src|tests|tools)/' -or $_ -eq 'CMakeLists.txt' })
  if ($src.Count -eq 0) { Write-Output $ok; exit }
  $authorSet = @($authors | ForEach-Object { ([string]$_).Trim() } | Where-Object { $_ })

  foreach ($w in (Read-Jsonl $waiversLog)) {
    $p = ([string]$w.prefix).ToLowerInvariant()
    if ($p.Length -ge 7 -and $key.StartsWith($p)) { Write-Output $ok; exit }
  }
  $last = @(Read-Jsonl $reviewsLog | Where-Object { [string]$_.key -eq $key }) | Select-Object -Last 1
  if ($last -and [string]$last.verdict -eq 'CLEAN' -and ($authorSet -notcontains [string]$last.reviewer)) { Write-Output $ok; exit }

  $why = 'nobody has reviewed it for derive-once yet'
  if ($last -and [string]$last.verdict -ne 'CLEAN') { $why = 'its derive-once review found problems (' + (Join-Path $stateDir ($key + '.md')) + ')' }
  elseif ($last) { $why = 'its latest review came from an agent that also wrote these commits' }
  $reason = 'This puts ' + $src.Count + ' changed code file(s) on main, but ' + $why + '. ' +
    'Every rule in Hydra has one owner, so a fresh agent has to check the change first. ' +
    'Dispatch a new agent that did not write this code, with the brief ' + $brief + ', the key ' + $key + ', and the range ' + $range + '. ' +
    'It submits with: & "C:/Users/Patrick/.claude/hooks/derive_once_submit.ps1" ' + $key + ' "<review file>". ' +
    'If it finds problems, fix them and review again. Only the user can skip this, by typing: waive derive-once ' + $key.Substring(0, 10)
  Write-Output (New-Deny $reason)
} catch {
  Write-Output '{}'
}
```

- [ ] **Step 4: Write the helper and the waiver hook.** Create `hooks\derive_once_submit.ps1`:

```powershell
# derive_once_submit.ps1 <key> <review file>
# Copies a derive-once review into the hooks state folder so the user can read
# it. derive_once_review_gate.ps1 records who submitted it before this runs.
param([string]$Key, [string]$File)
$dir = 'C:\Users\Patrick\.claude\hooks\state\derive_once'
if ($env:DERIVE_ONCE_STATE_DIR) { $dir = $env:DERIVE_ONCE_STATE_DIR }
New-Item -ItemType Directory -Force -Path $dir | Out-Null
Copy-Item -LiteralPath $File -Destination (Join-Path $dir ($Key + '.md')) -Force
Write-Output ('Submitted the derive-once review for ' + $Key + '.')
```

Create `hooks\derive_once_waiver.ps1`:

```powershell
# derive_once_waiver.ps1 -- UserPromptSubmit.
# When the user types "waive derive-once <7 to 40 hex characters>", record a
# waiver so derive_once_review_gate.ps1 lets that one change onto main.
# Background-task notifications arrive through this hook too; they never count.
$stateDir = 'C:\Users\Patrick\.claude\hooks\state\derive_once'
if ($env:DERIVE_ONCE_STATE_DIR) { $stateDir = $env:DERIVE_ONCE_STATE_DIR }
try {
  $m = [Console]::In.ReadToEnd() | ConvertFrom-Json
  $prompt = [string]$m.prompt
  if (-not $prompt) { Write-Output '{}'; exit }
  if ($prompt -match '(?i)task-notification|SYSTEM NOTIFICATION|Monitor event|<event>') { Write-Output '{}'; exit }
  $hits = [regex]::Matches($prompt, '(?i)\bwaive\s+derive-once\s+([0-9a-f]{7,40})\b')
  if ($hits.Count -gt 0) { New-Item -ItemType Directory -Force -Path $stateDir | Out-Null }
  foreach ($h in $hits) {
    $rec = @{ prefix = $h.Groups[1].Value.ToLowerInvariant(); session = [string]$m.session_id; time = (Get-Date).ToString('s') }
    Add-Content -LiteralPath (Join-Path $stateDir 'waivers.jsonl') -Value ($rec | ConvertTo-Json -Compress)
  }
  Write-Output '{}'
} catch {
  Write-Output '{}'
}
```

- [ ] **Step 5: Run the tests and the helper.** Run the Verify command. Expected: every line `PASS`, exit code 0. Then run the helper once by hand against a scratch file with `DERIVE_ONCE_STATE_DIR` set to a scratch folder, and confirm the copy appears. Then run the whole hook suite, which must stay green: `Get-ChildItem C:\Users\Patrick\.claude\hooks\tests\test_h*.ps1 | ForEach-Object { powershell -NoProfile -ExecutionPolicy Bypass -File $_.FullName }`. Paste the PASS and FAIL counts in your report.

- [ ] **Step 6: No commit.** The hooks folder is not a git repo. Report the four file paths and the test output instead.

---

### Task 4: Register the hooks (orchestrator)

**Goal:** The dispatcher runs the gate on every Bash and PowerShell call, the waiver hook runs on every prompt, and agents can run the submit helper without a permission prompt.

**Files:**
- Modify: `C:\Users\Patrick\.claude\hooks\dispatch_bash_ps.ps1` (the `$gates` array)
- Modify: `C:\Users\Patrick\.claude\settings.json` (`permissions.allow` and `hooks.UserPromptSubmit`)

**Acceptance Criteria:**
- [ ] `$gates` in `dispatch_bash_ps.ps1` lists `derive_once_review_gate.ps1` after `git_attribution_gate.ps1`.
- [ ] `settings.json` allows `PowerShell(& "C:/Users/Patrick/.claude/hooks/derive_once_submit.ps1"*)`.
- [ ] `settings.json` runs `derive_once_waiver.ps1` on `UserPromptSubmit`.
- [ ] The full hook suite still passes after registration.

**Verify:** `Select-String -Path C:\Users\Patrick\.claude\hooks\dispatch_bash_ps.ps1,C:\Users\Patrick\.claude\settings.json -Pattern 'derive_once'` → three hits (gate, helper allow, waiver hook)

**Steps:**

- [ ] **Step 1: Add the gate to the dispatcher.** In `dispatch_bash_ps.ps1`, change the `$gates` array to:

```powershell
$gates = @(
  'bash_risky_gate.ps1',
  'bash_listing_gate.ps1',
  'git_attribution_gate.ps1',
  'derive_once_review_gate.ps1',
  'rerun_gate.ps1',
  'fanout_track.ps1',
  'fable_delegate_gate.ps1'
)
```

- [ ] **Step 2: Register in settings.json.** Add `"PowerShell(& \"C:/Users/Patrick/.claude/hooks/derive_once_submit.ps1\"*)"` to `permissions.allow`, next to the `status_append.ps1` entry. Add this command to the existing `UserPromptSubmit` hook list, in the same shape as the `handoff_gate.ps1` entry there:

```json
{ "type": "command", "command": "powershell -NoProfile -ExecutionPolicy Bypass -File \"C:\\Users\\Patrick\\.claude\\hooks\\derive_once_waiver.ps1\"" }
```

Settings edits are picked up live by a file watcher; no restart is needed.

- [ ] **Step 3: Re-run the hook suite.** Run all `test_h*.ps1`. Expected: same PASS count as before plus the new file's checks, and no FAIL lines.

---

### Task 5: Prove it end to end without merging anything

**Goal:** On the real repo, the gate denies a real branch's merge, a real reviewer agent writes and submits a review from the brief, and the gate then allows the same merge, all without running `git merge`.

**Files:**
- No repo files change. A throwaway worktree and branch are created and removed.

**Acceptance Criteria:**
- [ ] Feeding the gate a `git merge --no-ff gate-probe` input with `cwd` set to the main checkout prints `deny`, and the reason names the key.
- [ ] A reviewer agent dispatched with only the brief, the key and the range writes a file with valid `Key:` and `Verdict:` lines and submits it. `hooks\state\derive_once\<key>.md` exists afterwards.
- [ ] Feeding the gate the same input again prints `{}` if the verdict was `CLEAN`.
- [ ] The worktree and the `gate-probe` branch are removed, and `main` has no new commit from this task.

**Verify:** `git log -1 --format=%h main` before and after → the same hash; `git branch --list gate-probe` → empty

**Steps:**

- [ ] **Step 1: Make a probe branch.** From the main checkout:

```powershell
git worktree add ..\wt-gate-probe -b gate-probe
```

In `..\wt-gate-probe\src\core\strutil.cpp`, add one comment line at the top: `// gate probe: delete me`. Commit it there with trailers, staging by name.

- [ ] **Step 2: Ask the gate, without merging.** Feed it a hook input by hand:

```powershell
$in = @{ session_id = 'probe'; cwd = 'C:\Users\Patrick\Downloads\Hydra\hydra-test'; hook_event_name = 'PreToolUse'; tool_name = 'PowerShell'; tool_input = @{ command = 'git merge --no-ff gate-probe -m "probe"' } } | ConvertTo-Json -Compress -Depth 6
$in | powershell -NoProfile -ExecutionPolicy Bypass -File C:\Users\Patrick\.claude\hooks\derive_once_review_gate.ps1
```

Expected: a `deny` whose reason names the key (the hash of `gate-probe`) and the range `main...<key>`.

- [ ] **Step 3: Dispatch a real reviewer.** Give a fresh agent only this: "Read and follow `docs/agents/derive-once-review.md`. Key: `<key>`. Range: `main...<key>`." The brief carries the status-line rule. Expected verdict: `CLEAN`, because the change is a comment. If it says `FINDINGS`, read why: either the brief misled it or it found something real. Either way, report it to the user before going on.

- [ ] **Step 4: Ask the gate again.** Repeat Step 2. Expected: `{}`.

- [ ] **Step 5: Clean up.** Remove the worktree and delete the branch. Then confirm `main` didn't move:

```powershell
git worktree remove ..\wt-gate-probe
git branch -D gate-probe
git log -1 --format=%h main
```

---

## Self-review

Every part of the design has a task. The scan test is Task 1, the reviewer brief Task 2, the gate, helper and waiver Task 3, registration Task 4, and the live loop Task 5.

Names match across tasks. Every place uses `derive_once_review_gate.ps1`, `derive_once_submit.ps1`, `derive_once_waiver.ps1`, `DERIVE_ONCE_STATE_DIR`, `.derive-once-gate`, `reviews.jsonl` and `waivers.jsonl` with the same spelling. The gate's regexes (`^Key:\s*([0-9a-f]{40})`, `^Verdict:\s*(CLEAN|FINDINGS)`) match the format the brief prescribes.

## Known limits

**A scan only catches copies that look alike.** The same rule in different words gets past Task 1. That's what the reviewer is for, and the reviewer can miss things too. The 2026-10-03 audit never had a round come back empty.

**The gate trusts the hook's `cwd`.** A command like `cd C:\other; git merge x` is judged in the session's working folder. A merge typed into a terminal outside Claude isn't seen at all.

**The state log is a plain file.** An agent that writes `reviews.jsonl` directly could forge a record. Writing there needs a permission the subagent hooks deny, so this is unlikely, but nothing proves it can't happen.

**Rewording a commit changes its key.** After a review, editing a commit message changes the branch hash, so the merge needs a new review. Rebasing changes it too, but agents don't rebase.

**The gate sees what the shell scanner sees, and no more.** Which words are commands is decided in one place, `hooks\lib\shell_scan.ps1`. That covers top-level commands, `$(...)` and backticks (also inside double quotes), here-doc bodies that expand, and the script given to `bash -c`, `sh -c`, `powershell -Command`, `eval` and `Invoke-Expression`. It does not look inside other launchers (`wsl`, `ssh`, `xargs`, `Start-Process`, `docker`), and it reads `sudo -u root git ...` wrongly. A merge sent through one of those is invisible. Fix the scanner and the gate follows.

**A merge whose target the gate can't read as text is denied.** `git merge $b` and `git merge $(cat branch.txt)` are denied, because the gate can't know what the variable holds. Brackets are command separators, so `(git merge feat)` is judged as a plain merge of `feat`.

**Redirects come from the scanner.** `Get-ShellScan` lists each command's redirects separately from its arguments, so the gate never reads `2>&1` or `> f` as a branch or file name. A quoted `">"` stays an ordinary argument. A PowerShell redirect whose target is an expression, like `> (Join-Path a b)`, gets no target.

**A commit shares its call with nothing that changes the repo.** On `main`, `git commit` is denied when the same command also runs any git call beyond status, diff, log, show, rev-parse, ls-files, cat-file, merge-base and branch --list or --show-current. That includes `git commit ...; git push`. Split the calls. It costs one extra step, and it closes `checkout <rev> -- path`, `restore --staged`, `apply --index` and `stash pop` in one stroke.

**A submit call must be the whole command, with plain paths.** It records only when the scanner finds exactly one command: no other command before or after it, no nested command, no here-doc, no redirect (`> f`, `2>&1`), and no word that expands (`$key`, `$(...)`, backticks). The helper's path, the key and the review file's path also must not contain `$`, a backtick, brackets, `;`, `&`, `|`, `<` or `>`, even inside quotes. A reviewer whose scratchpad has one of those copies the review to a plain path first. A trailing `# comment` is fine, because a comment is not a command. A command that only mentions the helper (a commit message, `Get-Content` on it) is not a submit call and is left alone.

**Text the shell can't close is read loosely.** If a quote, here-doc or `$(` is left open, the gate tries the other shell's rules (PowerShell text in the Bash tool, or the reverse). If neither closes, it uses the scanner's loose reading, which splits commands on `;`, `|`, `&`, line breaks and brackets and does not read quotes or comments. A stray quote can't hide a merge. The price is a false deny: in loose text a `# comment` after a merge reads as extra branch names.

**If the scanner itself fails, a marked repo denies any command that mentions git** (or the submit helper), and an unmarked folder gets no opinion.

### Added at install time (2026-10-03)

The git-side gate now judges what reaches main and checks the trailers. The command-line gate only records a reviewer's submit, so most of the merge and commit limits above no longer decide anything. The limits below are about the install-time additions.

**The real-author check reads only messages it can see.** A subagent's commit or merge is refused when an `Agent:` line names someone else. The gate reads `-m`, `--trailer`, a here-doc or text piped to `-F -`, and a `-F` file it can open. It does not read a message reused by `-C`, `-c` or `--reuse-message`, by `git cherry-pick`, `rebase` or `am`, a hand-edited `.git/MERGE_MSG`, a `-t` template, a message built at run time (`-m "$(cat file)"`), or a trailer alias set with `-c trailer.<x>.key`. Those commits still need all three trailers to pass the git-side gate, but their `Agent:` value is not compared with the agent that ran them.

**Origin is trusted, so it is guarded on the command line only.** Commits already on any branch of origin skip the trailer rule, and the gate asks origin itself with one `git ls-remote`. The command-line gate refuses `git remote set-url/rename/remove/add` with origin, config writes of origin's URLs or `url.*.insteadOf`, the same keys through `-c`, `--config-env` or `GIT_CONFIG_*`, and shell writes to `.git/config`. A script or interpreter that edits `.git/config` or `~/.gitconfig` as a file is not seen.

**The ls-remote needs the network.** It only runs when a new commit lacks a trailer. Offline, such a commit is refused even when origin has it.

**The pre-push check is skipped by `git push --no-verify`,** so the command-line gate refuses that flag for everyone in Claude Code. A push made some other way (the GitHub API, `gh`, another tool) is not seen.
