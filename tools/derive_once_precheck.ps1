#Requires -Version 7.0
<#
derive_once_precheck.ps1: the mechanical half of a derive-once review.

Run it on a branch before the first reviewer starts. It prints one line per
item, kind letter first, so a reviewer can paste the list into the review and
spend the review on the copies a script cannot see (kind A, production code
answering a question twice).

    pwsh tools/derive_once_precheck.ps1                       # main...HEAD
    pwsh tools/derive_once_precheck.ps1 -Range c6debcd^2...c6debcd
    pwsh tools/derive_once_precheck.ps1 -WholeTree -Range main  # every line of main

The range works like git's. "A...B" checks what B adds since it forked from
A; "A..B" checks what B adds over A; a single commit checks that commit. Only
lines the range adds are checked, read from the range's last commit (B), so
the script needs no checkout, no build, and never edits anything. With
-WholeTree every line of B counts as added; check 3 is then skipped, because
"every number ever written" is not a review list. On main (2026-10-04,
23a5d97) -WholeTree lists 80 items: copies already in the tree, two of them
marked as known copies. -Disable turns checks off by number (the self-test
uses it to prove each check is needed).

Each line reads:  <kind> <file>:<line>  <what was found>  -- <why it counts>
The kind letters are the merge-gate audit's (2026-10-04, section 4): A a
production rule written twice, C a copied test helper or fixture, B a test
that recomputes what production computes, D a number with no decision, E a
source scan outside the one scan. F is the concept audit's kind (D113 item
6): a second home for an idea that already has one. A line ending "(known copy: ...)" is
already in the known copies of tests/single_owner/baseline.json with the fix
that removes it.

WHICH FILES

The script reads the .cpp, .h and .py files under src/, tests/ and tools/.
A test file is any file under tests/, and, under tools/, a file in a tests
folder or a Python file named test_*.py, at any depth (tools/tests/a.py and
tools/test_a.py count); every other file is production code
(Test-TestFile). Wherever a check below says "test file", it means this
rule. The script reads no .ps1 file, so it never reads itself or its
self-test. No check matches the added lines of the scan's own files against
its spellings, because their literals are pattern examples
(Test-SkippedFile). They are still read in three ways: the rows and known
copies are loaded (THE SCAN ROWS), check 1 compares the helpers the scan
defines with the others, and check 4 reads the rule rows a range adds.

THE SCAN ROWS

The single-owner scan owns every "this is a copy" pattern it has a row for.
Its rows live in tests/single_owner/rules/ (select_rule_files in engine.cpp says which
files) and its known copies in
tests/single_owner/baseline.json. This script does not keep its own copy of
those patterns: it reads both at the range's last commit, or at -RulesAt,
and applies the rows to the added lines the way the scan does (Get-RowHits).
An owner line passes, unless this range wrote that owner line; then it is
printed so the reviewer judges its reason. The kind comes from the file and
the row's owner: a line in production code is A; in a test file, a row
owned by tests/source_tree.h is E, a row owned by another test header is C,
and a row owned by production code is B. Checks 1, 2 and 4 print the C, the
B and A, and the E lines of this list. Each row's must-match and
must-not-match examples are tried too; a line starting "precheck:" says a
row could not be read as expected, because .NET reads its pattern
differently from the scan's std::regex. The patterns the checks below still
hold are for questions no row asks, each with a comment saying why.

Scoring an old range against today's rows: -RulesAt HEAD. At the range's own
last commit the scan already passes, so its rows mostly show known copies.
A commit from before the rules moved to JSON has no rule files, so a range
that ends there needs -RulesAt a later commit to apply any rows.

THE CHECKS

1. Helpers defined twice (kind C). Every function defined at namespace level
   in a C++ test file (see WHICH FILES), and every named lambda (auto f = [..](..) {), is matched against
   the other test files two ways: by name, and by body. For
   the body match, comments and the text inside string literals are dropped,
   namespace prefixes (std::, hydra::) are dropped, and every name that is not
   a C++ keyword or a member after "." or "->" is renamed in order of first
   use, so a copy with its function and variables renamed still matches.
   Bodies under 30 tokens are skipped by body (too many tiny helpers look
   alike; decision D49); they still match by name. A group is printed when one of its
   definitions starts on a line the range adds. Skipped: member functions
   inside a struct or class, copies inside one file, and the doctest macros
   (TEST_CASE, SUBCASE). Same-named helpers with different bodies are still
   printed, marked "bodies differ": two helpers with one name in two test
   files is how most copies start.
   The same check prints the scan rows' C lines (the MIDI chunk tags, the
   .srb deflate, the .chart header, the per-difficulty table, the tied-variant
   walks, the SqIn step check and the other fixtures a test header owns), and
   looks for one fixture spelling no row asks about: a MIDI variable-length
   delta encoded by hand outside tests/midi_util.h.

2. Recompute spellings in tests (kind B). The same check prints the scan
   rows' B lines (the SP-end offset, the typed 500 squeeze window, "== 1.0"
   on a transfer scale, the plain SP end rebuilt with plusmeasure, a test
   setting target_act_ticks) and their A lines. Then the added lines of C++
   and Python test files (see WHICH FILES) are matched against
   the kind-B spellings of merge-gate.md section 4 that no row matches: a
   typed 2.0 within two lines of a hit window (D49 records that two-line
   reach for the typed 500, now a row; the two-hit budget row catches only
   the budget's product shapes, and only in src/ and tools/); a tied-path count checked against a
   second count; an assertion whose expected side is arithmetic over other calls;
   an expected answer built from the predicate's own comparisons ("want = a
   == x || ..."); and, in Python tests, a fixture that decides with the
   module's own constants or asserts one module constant equals a formula of
   others (the scan reads no Python). A C++ statement split over up to four
   lines is read as one (D49). Skipped: comments and the text inside
   strings.

3. New numbers with no decision (kind D). Every numeric literal on an added
   code line of a file the script reads (see WHICH FILES; not comments,
   strings, #include, #pragma, #error or static_assert lines). The
   decision text is read at both ends of the range, the branch's last commit
   and the left side (main, for main...HEAD), because decisions are often
   recorded on main while the branch is open: docs/adr/*.md, CONTEXT.md, the
   "User decisions" block and any "Test limits" section of each plan in
   docs/superpowers/plans/, and the D-records in docs/audit/*decisions*.md
   (the user's numbered rulings, which reviewers count as decisions; the
   brief names only the first three places). If the line or the three lines
   above it (D49) cite a
   decision (D43, ADR 0014), the number must appear in that decision.
   Otherwise it must appear in one decision paragraph together with one of
   the line's own words (depth, window, floor...), so "40" in an unrelated
   sentence does not decide a depth of 40. Numbers are compared by value, so
   1'000'000, 1,000,000 and 1000000 are one number, 0xFFFF is 65,535, 500.0 is
   500, and "2^46" in a doc decides a 46.
   Skipped, and why:
   - 0, 1, 2 and their negatives: counts, first/second, off-by-one and
     halving are arithmetic, not thresholds (D49).
   - A number directly inside [ ]: an array size or an index. A number equal
     to an array size the file declares (key[256], std::array<T, 4>): a loop
     bound or limit that matches its container.
   - 1000, 1024 and 1000000 next to * or /: unit factors (ms to s, KiB; D49).
   - In production code, a number inside other arithmetic or passed to a
     call: that computes a layout or a conversion (pos += 8, fits(buf, 16)).
     Production numbers are checked where a threshold, limit, tolerance,
     time or fallback shows up: a comparison (a shift amount counts when
     its line compares, as in "x >= (1 << 30)"), the right side of a plain
     "=", a returned value, a std::min/max/clamp bound, or a duration.
     Table rows in braces are data and are skipped.
   - In test files, a number on an assertion line (any doctest CHECK,
     REQUIRE or WARN macro, or an assert) that is not in a < > <= >=
     comparison: that is a pinned result from a run, which is what a test
     should hold, not a threshold.
   - In test files, fixture data: tick lists, notes, offsets and other values set
     on a hand-built chart. Only these test numbers are checked: one in a
     comparison; a loop bound on a line about seeds, trials, repeats, samples,
     probes, frames or rounds (seeds 1-48 are a test limit), but not a loop
     over a braced list; a value assigned to a name that says limit (depth,
     cap, limit, filter, band, tolerance, floor, seed, workers, frames,
     timeout, deadline, window, settle...); a duration; and a sample size
     (begin() + N, std::min(.., N)).
   This trades a few misses for a short list: a pinned corpus count such as
   "changed == 9" is a test limit the reviewer asked about, but it reads like
   any other pin, so it is not listed.

4. Scans outside the scan file (kind E). The scan rows' E lines (a test
   using HYDRA_SOURCE_DIR or walking the source tree), then two spellings no
   row asks about, in an added line of a C++ or Python file the script reads
   under tests/ or tools/ (see WHICH FILES). In C++: an ifstream, fopen,
   _wfopen or open_file of a .cpp or .h path or of a path under src/ (the
   row matches only HYDRA_SOURCE_DIR). In Python: an open() of a .cpp or .h
   path, or a call to os.walk, os.listdir or any .glob/.rglob (glob.glob
   included) with "src" before the call's first closing parenthesis. A
   one-line text match: a bare glob(), a src path on the receiver as in
   Path("src").rglob(...), or src after a nested call's ")" is not flagged.
   Then every rule row the range adds to the rule files in
   tests/single_owner/rules/ is checked for an empty or missing must-match
   or must-not-match list, and for a negative lookahead that names variables
   inside its pattern (an exemption no owner line records, which applies in
   every file).

5. Second homes in src/ (kind F). The concept audit's clue probes (D113
   item 3; docs/audit/2026-10-11-concept-audit/leads.ps1 is the record of
   their one run), applied to what the range adds in src/ and compared with
   the rest of src/ at the range's last commit. The audit covers src/ only
   (D113 item 1), so tests/ and tools/ are on neither side. It prints a
   struct, class, enum or union whose name another src/ file also defines;
   a struct or class whose field names overlap a record in another file
   (the range added its name or one of the shared fields); a free function
   (as check 1 finds them) whose name another src/ .cpp file also defines;
   and a string literal that another src/ file also writes. How long a
   string and how many fields count are the $min* values in the check 5
   section, the user's decision D113 item 8. Each line names the other
   homes. Strings are read with the shared lexer, so a quote inside a
   comment never counts; raw strings and #include paths are left out. With
   -WholeTree it lists every such clue in src/, each pair from both sides.

What it cannot see: a production rule written twice (kind A) that no scan
row spells, a copy whose shape it does not know, a decision that exists only
as a reviewer's judgement, and code the range does not add. For kind F it
sees only shared names and shared text: a second home under new names (a
record whose fields are all renamed, an enum with other words for the same
states, a helper family with new names) passes, and so does a forward or
opaque declaration, a member function, a field of function type (any
declaration with a "(" reads as a method) and a second home in tests/ or
tools/. Step 8 of the derive-once review is where a reviewer looks for
those.

RANGE SIZE

After the items, the script prints how many lines the range changes. When
that is over the threshold it also prints a note saying the range should be
split before review. The note is a warning, not a gate: the exit code does
not change. The threshold is $largeRangeLines and the files that count are
Get-RangeSize's rule, both in the "range size" section below. The docs name
this script as the owner and never restate either. -WholeTree prints no size,
because the whole tree is not a change.
#>
[CmdletBinding()]
param(
    [string]$Range = 'main...HEAD',
    [string]$Repo = '.',
    [switch]$WholeTree,
    [string[]]$Disable = @(),
    # The commit whose tests/single_owner/ files give the scan rows and
    # known copies. Default: the range's last commit. Scoring an old range
    # against today's rows passes -RulesAt HEAD.
    [string]$RulesAt = ''
)

$ErrorActionPreference = 'Stop'
# The checks, in order: check N is the Nth function here. This list is the
# one place that says how many checks there are. -Disable's validation and
# the run at the end read it, and the self-test loads it from this text to
# turn each check off in turn.
$checks = @('Invoke-Check1', 'Invoke-Check2', 'Invoke-Check3', 'Invoke-Check4', 'Invoke-Check5')
# -Disable takes check numbers, as a list or comma-separated (-Disable 2 or -Disable 1,3).
$Disable = @($Disable | ForEach-Object { "$_" -split ',' } | Where-Object { $_ } | ForEach-Object {
    if ($_ -notmatch '^\d+$' -or [int]$_ -lt 1 -or [int]$_ -gt $checks.Count) {
        throw "-Disable takes check numbers 1 to $($checks.Count), not '$_'"
    }; [int]$_ })
$utf8 = [System.Text.UTF8Encoding]::new($false)
[Console]::OutputEncoding = $utf8
$OutputEncoding = $utf8
$inv = [System.Globalization.CultureInfo]::InvariantCulture

# ---------------------------------------------------------------- git access

function Invoke-Git([string[]]$GitArgs) {
    $out = & git -C $script:Root @GitArgs 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "git $($GitArgs -join ' ') failed: $(($out | Out-String).Trim())"
    }
    @($out | Where-Object { $_ -isnot [System.Management.Automation.ErrorRecord] } |
        ForEach-Object { "$_" })
}

$script:Root = $Repo
try { $script:Root = "$(@(Invoke-Git @('rev-parse', '--show-toplevel'))[0])".Trim() } catch { throw "not a git repository: $Repo" }
if (-not $script:Root) { throw "not a git repository: $Repo" }

# Which commit does this name mean (a branch, a tag, a hash, HEAD~2...)? The
# one place a name becomes a commit hash.
function Resolve-Commit([string]$Name) { @(Invoke-Git @('rev-parse', '--verify', "$Name^{commit}"))[0] }

# One side of "A...B" or "A..B" as a name; as in git, an empty side means HEAD.
function Get-RangeSide([string]$Side) { if ($Side) { $Side } else { 'HEAD' } }

$left = $null  # the range's left side, whose decisions count too (main, for main...HEAD)
if ($Range -match '^(.*?)(\.\.\.?)(.*)$') {
    $dots = $Matches[2]
    $tip = Resolve-Commit (Get-RangeSide $Matches[3])
    $left = Resolve-Commit (Get-RangeSide $Matches[1])
    # Three dots: what B adds since it forked from A. Two dots: over A itself.
    $base = if ($dots -eq '...') { @(Invoke-Git @('merge-base', $left, $tip))[0] } else { $left }
} else {
    $tip = Resolve-Commit $Range
    $base = if ($WholeTree) { $null } else { Resolve-Commit "$Range^1" }
}
$rulesRev = if ($RulesAt) { Resolve-Commit $RulesAt } else { $tip }

$script:FileCache = @{}
function Get-TipText([string]$Path, [string]$Rev = $tip) {
    $key = "${Rev}:$Path"
    if (-not $script:FileCache.ContainsKey($key)) {
        $lines = Invoke-Git @('show', $key)
        $script:FileCache[$key] = (($lines | ForEach-Object { $_.TrimEnd("`r") }) -join "`n")
    }
    $script:FileCache[$key]
}

# ---------------------------------------------------------------- file kinds
#
# What kind of file is this, and where does it sit? Every check asks through
# these functions, so each answer is written once.

# Which top folder of the repo is this file in (src, tools, tests...)? Empty
# for a file at the repo root.
function Get-TopFolder([string]$File) {
    $i = $File.IndexOf('/')
    if ($i -lt 0) { return '' }
    $File.Substring(0, $i)
}
# Is this a C++ file? The extensions are the scan's own: the single-owner
# scan reads a file only when its extension is exactly
# .cpp or .h, so the precheck reads the same files and no others. Check 4
# uses the same $cppExt to spot a source file's path written in a test.
$cppExt = '\.(cpp|h)'
function Test-CppPath([string]$p) { $p -cmatch ($cppExt + '$') }
# Is this a C++ source file, not a header? Check 5 compares free functions
# between source files only, as the audit's phase 1 probe did.
function Test-CppSourcePath([string]$p) { (Test-CppPath $p) -and $p -cmatch '\.cpp$' }
function Test-PyPath([string]$p) { $p -match '\.py$' }
# Is this a test file? Everything under tests/, and under tools/ a file in a
# tests folder or a Python file named test_*.py, at any depth (tools/tests/
# and tools/test_x.py count, as tools/test_derive_once_precheck.ps1 sits
# directly in tools/). Any file that is not a test is production code.
function Test-TestFile([string]$p) { (Get-TopFolder $p) -eq 'tests' -or $p -match '^tools/(?:.*/)?(?:tests/|test_[^/]*\.py$)' }

# The top folders the precheck reads, the same three the scan walks. Listing
# and diffing both use this one list, so a listed file is always diffed.
$codeFolders = @('src', 'tests', 'tools')
$allFiles = @(Invoke-Git (@('ls-tree', '-r', '--name-only', $tip, '--') + $codeFolders))

# Added lines per file: path -> HashSet[int] (1-based, in the tip's numbering).
$added = @{}
if ($WholeTree) {
    foreach ($f in $allFiles) { $added[$f] = $null }  # $null means every line
} else {
    $cur = $null
    foreach ($l in (Invoke-Git (@('diff', '--no-color', '--no-ext-diff', '--no-renames', '-U0', $base, $tip, '--') + $codeFolders))) {
        if ($l.StartsWith('+++ ')) {
            $cur = if ($l -eq '+++ /dev/null') { $null } else { $l.Substring(6) }
            if ($cur -and -not $added.ContainsKey($cur)) { $added[$cur] = [System.Collections.Generic.HashSet[int]]::new() }
        } elseif ($cur -and $l -match '^@@ -\S+ \+(\d+)(?:,(\d+))? @@') {
            $start = [int]$Matches[1]
            $count = if ($Matches[2]) { [int]$Matches[2] } else { 1 }
            for ($i = 0; $i -lt $count; $i++) { [void]$added[$cur].Add($start + $i) }
        }
    }
}
function Test-Added([string]$Path, [int]$Line) {
    if (-not $added.ContainsKey($Path)) { return $false }
    $set = $added[$Path]
    return ($null -eq $set) -or $set.Contains($Line)
}
function Get-AddedLines([string]$Path, [int]$Count) {
    if (-not $added.ContainsKey($Path)) { return @() }
    $set = $added[$Path]
    if ($null -eq $set) { return 1..$Count }
    @($set | Where-Object { $_ -le $Count } | Sort-Object)
}

# ------------------------------------------------------------------- lexing

# One pass blanks comments and the inside of string and char literals, keeping
# every newline so line numbers hold. Two views come out: code without
# comments (strings kept) and code without comments or string text.
$cppLex = [regex]::new(
    '(?<c>//[^\n]*|/\*[\s\S]*?\*/)|(?<s>R"(?<d>[^(\s"\\]{0,16})\([\s\S]*?\)\k<d>"|"(?:\\.|[^"\\\n])*"|(?<![\w])''(?:\\.|[^''\\\n]){1,8}'')',
    'Compiled')
$pyLex = [regex]::new(
    '(?<c>#[^\n]*)|(?<s>[rbfuRBFU]{0,2}(?:"""[\s\S]*?"""|''''''[\s\S]*?''''''|"(?:\\.|[^"\\\n])*"|''(?:\\.|[^''\\\n])*''))',
    'Compiled')
function Blank([string]$s) { [regex]::Replace($s, '[^\n]', ' ') }
function Blank-Inner([string]$s) {
    # keep the closing quote and an opening quote, blank the text between
    $q = $s.LastIndexOf($s[$s.Length - 1])
    $open = $s.IndexOfAny([char[]]@('"', "'"))
    if ($open -lt 0 -or $open -ge $s.Length - 1) { return $s }
    $s.Substring(0, $open + 1) + (Blank $s.Substring($open + 1, $s.Length - $open - 2)) + $s[$s.Length - 1]
}
# A text with its comments blanked, read with one of the two lexers above;
# with -AndStrings the text inside each string or char literal is blanked
# too.
function Remove-Comments([regex]$Lex, [string]$Text, [switch]$AndStrings) {
    $Lex.Replace($Text, { param($m)
        if ($m.Groups['c'].Success) { Blank $m.Value } elseif ($AndStrings) { Blank-Inner $m.Value } else { $m.Value } })
}
$script:LexCache = @{}
function Get-Views([string]$Path, [string]$Rev = $tip) {
    $key = "${Rev}:$Path"
    if ($script:LexCache.ContainsKey($key)) { return $script:LexCache[$key] }
    $text = Get-TipText $Path $Rev
    $rx = if (Test-PyPath $Path) { $pyLex } else { $cppLex }
    $noComments = Remove-Comments $rx $text
    $code = Remove-Comments $rx $text -AndStrings
    $v = [pscustomobject]@{
        Raw        = $text.Split("`n")
        NoComments = $noComments.Split("`n")
        Code       = $code.Split("`n")
        CodeText   = $code
        NcText     = $noComments
        LineStarts = $null  # filled by Get-LineOf on first use
        Braces     = $null  # filled by Get-Braces on first use
    }
    $script:LexCache[$key] = $v
    $v
}
# Which line (1-based) a character offset in a view's text is on. Every view
# keeps the file's newlines where they were, so one list of line starts
# serves them all; it is built once per view, on first use. This is the one
# place the script turns an offset into a line.
function Get-LineOf([object]$V, [int]$Offset) {
    if ($null -eq $V.LineStarts) {
        $starts = [System.Collections.Generic.List[int]]::new(); $starts.Add(0)
        foreach ($m in [regex]::Matches($V.CodeText, "`n")) { $starts.Add($m.Index + 1) }
        $V.LineStarts = $starts.ToArray()
    }
    # An exact hit is a line's first character; otherwise BinarySearch gives
    # the complement of the next line's index, which is this line's number.
    $i = [Array]::BinarySearch($V.LineStarts, $Offset)
    if ($i -ge 0) { $i + 1 } else { -bnot $i }
}
# Which "}" closes each "{", and which "{" encloses it. One pass with a stack
# over the view's code text (comments and string text blanked, so a brace
# there never counts), built once per view on first use. This is the one
# place the script matches braces. A "{" that never closes has no Close
# entry, and stays the enclosing block of everything after it.
function Get-Braces([object]$V) {
    if ($null -eq $V.Braces) {
        $close = @{}; $parent = @{}
        $stack = [System.Collections.Generic.List[int]]::new()
        foreach ($m in [regex]::Matches($V.CodeText, '[{}]')) {
            if ($m.Value -eq '{') {
                $parent[$m.Index] = if ($stack.Count) { $stack[$stack.Count - 1] } else { -1 }
                $stack.Add($m.Index)
            } elseif ($stack.Count -gt 0) {
                $close[$stack[$stack.Count - 1]] = $m.Index
                $stack.RemoveAt($stack.Count - 1)
            }
        }
        $V.Braces = [pscustomobject]@{ Close = $close; Parent = $parent }
    }
    $V.Braces
}
# The offset of the "}" that closes the "{" at $Open, or -1 when it does not
# close before $Limit.
function Get-CloseBrace([object]$V, [int]$Open, [int]$Limit) {
    $c = (Get-Braces $V).Close[$Open]
    if ($null -eq $c -or $c -ge $Limit) { -1 } else { $c }
}
# The "{" offsets directly inside the "{" at $Open, in file order. Each
# brace's children come from Get-Braces's parents, built once per view.
function Get-BraceChildren([object]$V, [int]$Open) {
    if (-not $V.PSObject.Properties['Children']) {
        $kids = @{}
        $braces = Get-Braces $V
        foreach ($o in ($braces.Parent.Keys | Sort-Object)) {
            $p = $braces.Parent[$o]
            if (-not $kids.ContainsKey($p)) { $kids[$p] = [System.Collections.Generic.List[int]]::new() }
            $kids[$p].Add($o)
        }
        $V | Add-Member -NotePropertyName Children -NotePropertyValue $kids
    }
    ,@(if ($V.Children.ContainsKey($Open)) { $V.Children[$Open] })
}

# ------------------------------------------------------------------- output

# How a warning line starts: the script could not read something as the scan
# does. The self-test loads this from this file and fails on every such line.
$warnPrefix = 'precheck: '
$items = [System.Collections.Generic.List[object]]::new()
function Add-Item([string]$Kind, [string]$File, [int]$Line, [string]$What, [string]$Why) {
    $items.Add([pscustomobject]@{ Kind = $Kind; File = $File; Line = $Line; What = $What; Why = $Why })
}

# Where the scan keeps its rows and its known copies. This script reads both at $rulesRev and keeps no copy of either.
$rulesDir = 'tests/single_owner/rules'
$baselineFile = 'tests/single_owner/baseline.json'
# The baseline's known copies: file -> list of (line text, fix)
$known = @{}
# Which .cpp, .h or .py file does the precheck not read line by line? The
# scan's own files, whose strings and literals are pattern examples: the
# rule files and the scan's C++ in tests/single_owner/, and the file the
# scan's C++ lived in before it moved there (old ranges still add lines to
# it). Every check that reads added lines asks here. This script and its
# self-test need no entry: they are .ps1, and every check reads only .cpp,
# .h and .py files. (Check 1's helper list still reads the scan's helpers,
# so a helper copied out of them is found; check 4 reads the rule rows a
# range adds.)
function Test-SkippedFile([string]$File) { $File.StartsWith('tests/single_owner/') -or $File -eq 'tests/test_single_owner.cpp' }

# The objects of one JSON array in a rules or baseline file, in file order,
# each with its file, its first and last line, and its opening brace: the
# objects listed under the key $Key of the object that opens at $Parent (-1
# for the file's top object). ConvertFrom-Json gives no lines, so they come
# from the brace matcher (Get-Braces); a JSON string lexes as a C++ string,
# so a brace or a key inside one never counts. An object's key is the last
# '"key": [' before it, outside its earlier siblings.
function Get-JsonObjects([object]$V, [string]$File, [int]$Parent, [string]$Key) {
    $out = [System.Collections.Generic.List[object]]::new()
    if ($Parent -lt 0) { $Parent = $V.CodeText.IndexOf('{') }
    $children = Get-BraceChildren $V $Parent
    $keyRx = [regex]::new('"[^"\n]*"\s*:\s*\[', 'RightToLeft')
    $from = $Parent + 1; $cur = $null
    foreach ($open in $children) {
        $m = $keyRx.Match($V.CodeText, $from, $open - $from)
        if ($m.Success) { $cur = $V.NcText.Substring($m.Index + 1, $V.NcText.IndexOf('"', $m.Index + 1) - $m.Index - 1) }
        $close = Get-CloseBrace $V $open $V.CodeText.Length
        if ($close -lt 0) { break }
        if ($cur -eq $Key) {
            $out.Add([pscustomobject]@{ File = $File; Open = $open; Line = (Get-LineOf $V $open); LastLine = (Get-LineOf $V $close) })
        }
        $from = $close + 1
    }
    ,$out
}
# Was any line of this JSON object added by the range? Only meaningful when
# the rows are read from the range's own last commit.
function Test-ElementAdded([object]$El) {
    if ($rulesRev -ne $tip) { return $false }
    for ($i = $El.Line; $i -le $El.LastLine; $i++) { if (Test-Added $El.File $i) { return $true } }
    $false
}

# The scan's rows, read from the rule files in $rulesDir at $rulesRev, and
# its known copies, read from $baselineFile there. The scan owns every
# question its rows ask; checks 1, 2 and 4 apply these rows to the added
# lines the way the scan does, and keep their own patterns only for
# questions no row asks. The field names are the scan's loader's.
$rows = [System.Collections.Generic.List[object]]::new()
$rowWarnings = [System.Collections.Generic.List[string]]::new()
# One JSON list field as a string array; a missing field is an empty list.
# The list comes back behind a leading comma, because PowerShell unrolls an
# array a function hands back, and a one-item array unrolls to its item: a
# row with one exempt file once read as the single characters of that file's
# name (M0 review 2, finding 1).
function Get-JsonStrings([object]$Value) {
    ,[string[]]@($Value | Where-Object { $null -ne $_ } | ForEach-Object { "$_" })
}
# The parsed objects of one JSON list field, matched in order with the line
# spans Get-JsonObjects found for them; the two readings must agree on the
# count, or the file is not what this reader expects.
function Get-JsonItems([object]$Value, [object]$Spans, [string]$What) {
    $items = @($Value | Where-Object { $null -ne $_ })
    if ($items.Count -ne $Spans.Count) { throw "$warnPrefix${What}: $($items.Count) objects parsed, $($Spans.Count) found by brace; cannot place them on lines" }
    ,$items
}
function New-StdRegex([string]$Pattern) {
    # The scan uses std::regex's ECMAScript grammar; .NET's ECMAScript mode is
    # the nearest reading. A pattern it refuses is read with .NET's own rules,
    # and the example check below says when the two readings differ.
    try { [regex]::new($Pattern, 'ECMAScript') } catch { [regex]::new($Pattern) }
}
# Which files are rules files is answered by select_rule_files in
# tests/single_owner/engine.cpp: .json files in the folder and in the folders
# one level down, in name order. This follows it (names sort byte by byte, as
# the scan's do).
$ruleFiles = @(Invoke-Git @('ls-tree', '-r', '--name-only', $rulesRev, '--', "$rulesDir/") | Where-Object { $_.Substring($rulesDir.Length + 1) -match '^([^/]+/)?[^/]+\.json$' })
[System.Array]::Sort($ruleFiles, [System.StringComparer]::Ordinal)
foreach ($rf in $ruleFiles) {
    $rv = Get-Views $rf $rulesRev
    $doc = (Get-TipText $rf $rulesRev) | ConvertFrom-Json
    $spans = Get-JsonObjects $rv $rf -1 'rules'
    $jrows = Get-JsonItems $doc.rules $spans "$rf rules"
    for ($i = 0; $i -lt $jrows.Count; $i++) {
        $j = $jrows[$i]; $el = $spans[$i]
        $olSpans = Get-JsonObjects $rv $rf $el.Open 'owner_lines'
        $jols = Get-JsonItems $j.owner_lines $olSpans "$rf ""$($j.question)"" owner_lines"
        $row = [pscustomobject]@{
            Question = "$($j.question)"; Owner = "$($j.owner)"; Pattern = "$($j.pattern)"; CallsOwner = "$($j.calls_owner)"
            OwnerFiles = (Get-JsonStrings $j.owner_files)
            Exempt = (Get-JsonStrings @($j.exempt | Where-Object { $null -ne $_ } | ForEach-Object { $_.file }))
            MustMatch = (Get-JsonStrings $j.must_match); MustNotMatch = (Get-JsonStrings $j.must_not_match)
            OwnerLines = [object[]]@(for ($k = 0; $k -lt $jols.Count; $k++) {
                $o = $jols[$k]
                [pscustomobject]@{ File = "$($o.file)"; Text = "$($o.line_text)".Trim(); Why = "$($o.why)"; Added = (Test-ElementAdded $olSpans[$k]) }
            })
            Scope = (Get-JsonStrings $j.scope)
            FunctionFile = "$($j.function_file)"; Function = "$($j.function)"
            ScanComments = ($j.scan_comments -eq $true)
            # A Python row (the probe's) reads only .py files; every other row reads C++.
            Python = ("$($j.language)" -eq 'py')
            File = $rf; Line = $el.Line; Added = (Test-ElementAdded $el); Rx = $null; CallsRx = $null
        }
        $row.Rx = New-StdRegex $row.Pattern
        if ($row.CallsOwner) { $row.CallsRx = New-StdRegex $row.CallsOwner }
        $rows.Add($row)
    }
}
if (@(Invoke-Git @('ls-tree', '--name-only', $rulesRev, '--', $baselineFile)).Count) {
    $bv = Get-Views $baselineFile $rulesRev
    $doc = (Get-TipText $baselineFile $rulesRev) | ConvertFrom-Json
    $spans = Get-JsonObjects $bv $baselineFile -1 'known_copies'
    $kcs = Get-JsonItems $doc.known_copies $spans "$baselineFile known_copies"
    for ($i = 0; $i -lt $kcs.Count; $i++) {
        $k = $kcs[$i]; $file = "$($k.file)"
        if (-not $known.ContainsKey($file)) { $known[$file] = [System.Collections.Generic.List[object]]::new() }
        $known[$file].Add([pscustomobject]@{ Question = "$($k.question)"; Text = "$($k.line_text)".Trim(); Fix = "$($k.removed_by)"; Added = (Test-ElementAdded $spans[$i]) })
    }
}
# The scan's one verdict (flags_line): the line matches the pattern and does
# not call the owner.
function Test-RowFlags([object]$Row, [string]$Line) {
    if (-not $Row.Rx.IsMatch($Line)) { return $false }
    -not ($Row.CallsRx -and $Row.CallsRx.IsMatch($Line))
}
# Each row's own examples must read the same here as in the scan's self-test.
foreach ($r in $rows) {
    foreach ($x in $r.MustMatch) { if (-not (Test-RowFlags $r $x)) { $rowWarnings.Add("row ""$($r.Question)"" does not flag its must-match example here: $x") } }
    foreach ($x in $r.MustNotMatch) { if (Test-RowFlags $r $x) { $rowWarnings.Add("row ""$($r.Question)"" flags its must-not-match example here: $x") } }
    foreach ($o in $r.OwnerLines) { if (-not (Test-RowFlags $r $o.Text)) { $rowWarnings.Add("row ""$($r.Question)"" does not flag its owner line here: $($o.Text)") } }
}
# How a row hit reads in an item's reason: the row's owner, then a note when
# the scan lists the line, either as a known copy (the fix that removes it)
# or as an owner line this range wrote. The self-test loads these three from
# this file to recognise the wording, so rewording one here cannot leave its
# whole-tree check reading nothing.
function Format-RowWhy([string]$File, [string]$Owner) { "the scan row in $File gives this question to $Owner" }
function Format-KnownCopy([string]$Fix) { "known copy: $Fix" }
function Format-OwnerLineNote([string]$Why) { "this range lists it as an owner line: ""$Why""; check that reason" }
function Get-KnownCopy([string]$File, [string[]]$LineTexts) {
    if (-not $known.ContainsKey($File)) { return $null }
    foreach ($t in $LineTexts) {
        foreach ($k in $known[$File]) { if ($t.Trim() -eq $k.Text) { return $k.Fix } }
    }
    $null
}

# Does the scan read this file under this row (in_scope, the function a row
# is limited to, owner files and exempt files)? A row with no scope reads
# src/ and tools/: that default is the scan's own in_scope rule, mirrored
# here on purpose because this script applies the scan's rows.
function Test-RowCovers([object]$Row, [string]$File) {
    $sub = Get-TopFolder $File
    $inScope = if ($Row.Scope.Count -eq 0) { $sub -eq 'src' -or $sub -eq 'tools' } else { ($Row.Scope -contains $sub) -or ($Row.Scope -contains $File) }
    if (-not $inScope) { return $false }
    if ($Row.Function -and $File -ne $Row.FunctionFile) { return $false }
    -not (($Row.OwnerFiles -contains $File) -or ($Row.Exempt -contains $File))
}
# The lines of a function-limited row's function (the scan's rule: from the
# line holding the function's text to the next line that is exactly "}").
function Get-FunctionLines([object]$Row, [object]$V) {
    $set = [System.Collections.Generic.HashSet[int]]::new()
    $in = $false
    for ($i = 0; $i -lt $V.Raw.Count; $i++) {
        $line = $V.Raw[$i]
        if (-not $in -and $line.Contains($Row.Function)) { $in = $true }
        if (-not $in) { continue }
        [void]$set.Add($i + 1)
        if ($line -eq '}') { $in = $false }
    }
    ,$set
}

# Every added line in a C++ file that a scan row flags, as the scan would see
# it. Kind: a line in production code (not a test file) is a production copy
# (A); in a test file, a row whose owner is tests/source_tree.h is a stray scan (E), a row
# whose owner is another test header is a copied fixture (C), and a row whose
# owner is production code is a recompute (B). Checks 1, 2 and 4 each print
# their own kinds from this one list.
$script:RowHits = $null
function Get-RowHits {
    if ($null -ne $script:RowHits) { return $script:RowHits }
    $hits = [System.Collections.Generic.List[object]]::new()
    foreach ($f in $added.Keys) {
        if (Test-SkippedFile $f) { continue }
        $py = Test-PyPath $f
        if (-not ((Test-CppPath $f) -or $py)) { continue }
        $covering = @($rows | Where-Object { $_.Python -eq $py -and (Test-RowCovers $_ $f) })
        if (-not $covering.Count) { continue }
        $v = Get-Views $f
        $trimmed = @($v.Raw | ForEach-Object { $_.Trim() })
        $funcLines = @{}
        foreach ($ln in (Get-AddedLines $f $v.Raw.Count)) {
            $t = $trimmed[$ln - 1]
            if (-not $t) { continue }
            $isComment = $t.StartsWith($(if ($py) { '#' } else { '//' }))
            foreach ($r in $covering) {
                if ($isComment -and -not $r.ScanComments) { continue }
                if ($r.Function) {
                    if (-not $funcLines.ContainsKey($r.Question)) { $funcLines[$r.Question] = Get-FunctionLines $r $v }
                    if (-not $funcLines[$r.Question].Contains($ln)) { continue }
                }
                if (-not (Test-RowFlags $r $v.Raw[$ln - 1])) { continue }
                # A listed line (owner line, then known copy) covers one line
                # of source: the n-th line with this text takes the n-th entry.
                $owners = @($r.OwnerLines | Where-Object { $_.File -eq $f -and $_.Text -eq $t })
                $copies = @(if ($known.ContainsKey($f)) { $known[$f] | Where-Object { $_.Question -eq $r.Question -and $_.Text -eq $t } })
                $nth = 0
                for ($k = 1; $k -le $ln; $k++) {
                    if ($trimmed[$k - 1] -ne $t) { continue }
                    if ($r.Function -and -not $funcLines[$r.Question].Contains($k)) { continue }
                    $nth++
                }
                $note = $null
                if ($nth -le $owners.Count) {
                    # An owner line passes, unless this range wrote it: then
                    # the reviewer judges its reason.
                    $o = $owners[$nth - 1]
                    if (-not $o.Added) { continue }
                    $note = Format-OwnerLineNote $o.Why
                } elseif ($nth -le $owners.Count + $copies.Count) {
                    $c = $copies[$nth - $owners.Count - 1]
                    $note = (Format-KnownCopy $c.Fix) + $(if ($c.Added) { '; this range added that entry' } else { '' })
                } elseif ($owners.Count + $copies.Count) {
                    $note = 'a second copy of a listed line; each entry covers one line'
                }
                $kind = if (-not (Test-TestFile $f)) { 'A' } elseif ($r.Owner -match 'tests/source_tree\.h') { 'E' } elseif ($r.Owner -match '\btests/') { 'C' } else { 'B' }
                $hits.Add([pscustomobject]@{ Kind = $kind; File = $f; Line = $ln; Text = $t; Row = $r; Note = $note })
            }
        }
    }
    $script:RowHits = $hits
    $hits
}
function Add-RowItems([string[]]$Kinds) {
    foreach ($h in (Get-RowHits)) {
        if ($Kinds -notcontains $h.Kind) { continue }
        $why = Format-RowWhy $h.Row.File $h.Row.Owner
        if ($h.Note) { $why += " ($($h.Note))" }
        Add-Item $h.Kind $h.File $h.Line "answers ""$($h.Row.Question)"": $($h.Text)" $why
    }
}

# Which calls are test assertions? The doctest assertion macros, and any
# assert*( or self.assert*( for C asserts and Python's unittest. Check 1
# does not take a doctest macro for a helper, check 2 reads an expected value
# inside an assertion, and check 3 takes a number on one as a pinned result.
# Case matters, and the pattern itself says so ("(?-i:"), so every reader gets
# the same answer whichever operator it uses: CHECK( is an assertion, a local
# helper named check( is not. The doctest macros are the whole assertion
# family third_party/doctest/doctest.h defines: CHECK, REQUIRE or WARN, with
# or without the FAST_ prefix, alone or with one of its suffixes (_EQ,
# _FALSE, _MESSAGE, _NOTHROW, _THROWS_AS...).
$doctestAsserts = '(?:FAST_)?(?:CHECK|REQUIRE|WARN)(?:_(?:EQ|NE|GT|GE|LT|LE|UNARY(?:_FALSE)?|FALSE(?:_MESSAGE)?|MESSAGE|NOTHROW(?:_MESSAGE)?|THROWS\w*))?'
$assertRx = "(?-i:\b($doctestAsserts|assert\w*|self\.assert\w*)\s*\()"

# ------------------------------------------- check 1: helpers defined twice

# Which words are C++ keywords? This one set answers it: the body match
# keeps them as they are instead of renaming them, and a definition is never
# named by one ("if (x) {" is not a function called if). The fixed-width
# integer names are not keywords, but they read as types, so they stay too.
# Part of the set is its own smaller question: the keywords that start a
# statement or an expression and never a declaration. A definition's
# return-type words hold none of them, so "return f(x) {" is not a
# definition of f; "static int f(x) {" is, though static and int are keywords.
$cppStatementKeywords = @('return', 'else', 'new', 'delete', 'throw', 'case', 'do', 'goto', 'co_return')
$cppKeywords = [System.Collections.Generic.HashSet[string]]::new([string[]]@(@(
    'alignas', 'alignof', 'auto', 'bool', 'break', 'catch', 'char', 'class', 'const', 'constexpr',
    'const_cast', 'continue', 'decltype', 'default', 'double', 'dynamic_cast', 'enum',
    'explicit', 'extern', 'false', 'float', 'for', 'friend', 'if', 'inline', 'int', 'long', 'mutable',
    'namespace', 'noexcept', 'nullptr', 'operator', 'private', 'protected', 'public', 'reinterpret_cast',
    'short', 'signed', 'sizeof', 'static', 'static_assert', 'static_cast', 'struct', 'switch',
    'template', 'this', 'true', 'try', 'typedef', 'typename', 'union', 'unsigned', 'using', 'virtual',
    'void', 'volatile', 'while', 'size_t', 'int8_t', 'int16_t', 'int32_t', 'int64_t', 'uint8_t', 'uint16_t',
    'uint32_t', 'uint64_t') + $cppStatementKeywords))
# Names that are not keywords but are still not helpers: the doctest test
# macros and main (and the doctest assertions, $doctestAsserts).
$notFunctionNames = [System.Collections.Generic.HashSet[string]]::new([string[]]@('TEST_CASE', 'SUBCASE', 'TEST_SUITE', 'main'))
$fnRx = [regex]::new('(?m)^[ \t]*(?:template\s*<[^;{}]*>\s*)?(?<pre>(?:[A-Za-z_][\w:]*(?:<[^;{}()]*>)?[\s\*&]+)+?)(?<name>[A-Za-z_]\w*)\s*\((?<params>[^;{}]*?)\)\s*(?:const\s*)?(?:noexcept\s*)?(?:->\s*[^;{}()]+?)?\{', 'Compiled')
$lamRx = [regex]::new('(?:\bauto|\bconst\s+auto)\s*&?\s*(?<name>[A-Za-z_]\w*)\s*=\s*\[[^\]]*\]\s*(?:\((?<params>[^;{}]*?)\))?\s*(?:mutable\s*)?(?:->\s*[^;{}()]+?)?\{', 'Compiled')
$tokRx = [regex]::new('[A-Za-z_]\w*|\d[\w.'']*|"\s*"|''\s*''|->|::|\S', 'Compiled')

function Get-Normalised([string]$Text) {
    $t = [regex]::Replace($Text, '\b[A-Za-z_]\w*\s*::\s*', '')
    $names = @{}
    $out = [System.Text.StringBuilder]::new()
    $prev = ''
    $n = 0
    foreach ($m in $tokRx.Matches($t)) {
        $tok = $m.Value
        if ($tok[0] -eq '"') { $tok = 'S' }
        elseif ($tok[0] -eq "'") { $tok = 'C' }
        elseif (($tok[0] -match '[A-Za-z_]') -and -not $cppKeywords.Contains($tok) -and $prev -ne '.' -and $prev -ne '->') {
            if (-not $names.ContainsKey($tok)) { $names[$tok] = "v$($names.Count)" }
            $tok = $names[$tok]
        }
        [void]$out.Append($tok).Append(' ')
        $prev = $m.Value
        $n++
    }
    [pscustomobject]@{ Key = $out.ToString(); Tokens = $n }
}

# The functions defined at namespace level in one file, and its named
# lambdas. -NamesOnly skips the body match's normalising, which check 5
# does not use and which is slow over all of src/.
function Get-Definitions([string]$Path, [switch]$NamesOnly) {
    $v = Get-Views $Path
    $code = $v.CodeText
    # Brace structure from Get-Braces: each '{' with its closing offset and
    # the block that encloses it. A function is a free function when every
    # enclosing block is a namespace.
    $braces = Get-Braces $v
    $nsMemo = @{}
    $isNs = {
        param($o)
        if (-not $nsMemo.ContainsKey($o)) {
            # Look back up to 200 characters for "namespace" or extern "C"
            # before this brace (decision D49): a parse buffer, long enough
            # for any namespace head.
            $from = [Math]::Max(0, $o - 200)
            $before = $code.Substring($from, $o - $from)
            $cut = $before.LastIndexOfAny([char[]]@(';', '{', '}'))
            $head = $before.Substring($cut + 1)
            $nsMemo[$o] = $head -match '\bnamespace\b[^;{}]*$|\bextern\s*"[^"]*"\s*$'
        }
        $nsMemo[$o]
    }
    $atTop = {
        param($o)
        for ($p = $braces.Parent[$o]; $p -ge 0; $p = $braces.Parent[$p]) { if (-not (& $isNs $p)) { return $false } }
        $true
    }
    $defs = [System.Collections.Generic.List[object]]::new()
    $add = {
        param($m, $isLambda)
        $open = $m.Index + $m.Length - 1
        $end = Get-CloseBrace $v $open $code.Length
        if ($end -lt 0) { return }
        if (-not $isLambda -and -not (& $atTop $open)) { return }
        $name = $m.Groups['name'].Value
        if ($cppKeywords.Contains($name) -or $notFunctionNames.Contains($name) -or $name -cmatch "^($doctestAsserts)$") { return }
        if (-not $isLambda -and ($m.Groups['pre'].Value -match "\b($($cppStatementKeywords -join '|'))\b")) { return }
        $norm = if ($NamesOnly) { [pscustomobject]@{ Key = $null; Tokens = 0 } } else { Get-Normalised ($m.Groups['params'].Value + ' ' + $code.Substring($open, $end - $open + 1)) }
        $first = Get-LineOf $v ($m.Index + ($m.Value.Length - $m.Value.TrimStart().Length))
        $last = Get-LineOf $v $end
        $defs.Add([pscustomobject]@{
            File = $Path; Name = $name; Line = $first; LastLine = $last; Key = $norm.Key; Tokens = $norm.Tokens
            Lambda = $isLambda
        })
    }
    foreach ($m in $fnRx.Matches($code)) { & $add $m $false }
    foreach ($m in $lamRx.Matches($code)) { & $add $m $true }
    $defs
}

function Invoke-Check1 {
    $testFiles = @($allFiles | Where-Object { (Test-TestFile $_) -and (Test-CppPath $_) })
    $defs = [System.Collections.Generic.List[object]]::new()
    foreach ($f in $testFiles) { foreach ($d in (Get-Definitions $f)) { $defs.Add($d) } }

    $groups = [System.Collections.Generic.List[object]]::new()
    # by name (free functions only; lambdas are local names)
    foreach ($g in ($defs | Where-Object { -not $_.Lambda } | Group-Object Name)) {
        $files = @($g.Group | Select-Object -ExpandProperty File -Unique)
        if ($files.Count -ge 2) { $groups.Add([pscustomobject]@{ By = 'name'; Defs = @($g.Group) }) }
    }
    # by body, names normalised; bodies under 30 tokens match by name only
    # (decision D49)
    foreach ($g in ($defs | Where-Object { $_.Tokens -ge 30 } | Group-Object Key)) {
        $files = @($g.Group | Select-Object -ExpandProperty File -Unique)
        if ($files.Count -lt 2) { continue }
        $names = @($g.Group | Select-Object -ExpandProperty Name -Unique)
        if ($names.Count -eq 1 -and -not $g.Group[0].Lambda) { continue }  # already a name group
        $groups.Add([pscustomobject]@{ By = 'body'; Defs = @($g.Group) })
    }
    foreach ($g in $groups) {
        $inRange = @($g.Defs | Where-Object { Test-Added $_.File $_.Line })
        if ($inRange.Count -eq 0) { continue }
        $anchor = $inRange[0]
        $others = @($g.Defs | Where-Object { $_ -ne $anchor } | ForEach-Object { "$($_.Name) at $($_.File):$($_.Line)" })
        $knownFix = $null
        foreach ($d in $g.Defs) {
            $v = Get-Views $d.File
            $fix = Get-KnownCopy $d.File $v.Raw[($d.Line - 1)..($d.LastLine - 1)]
            if ($fix) { $knownFix = $fix }
        }
        if ($g.By -eq 'name') {
            $same = @($g.Defs | Select-Object -ExpandProperty Key -Unique).Count -eq 1
            $what = "helper $($anchor.Name) is also defined as $($others -join ', ')" + $(if ($same) { ' (same body)' } else { ' (bodies differ)' })
        } else {
            $what = "$(if ($anchor.Lambda) { 'lambda' } else { 'helper' }) $($anchor.Name) has the same body, names changed, as $($others -join ', ')"
        }
        $why = 'a test helper written in two files; keep one in a shared header'
        if ($knownFix) { $why += " ($(Format-KnownCopy $knownFix))" }
        Add-Item 'C' $anchor.File $anchor.Line $what $why
    }

    # Copied fixtures a scan row names: the MIDI chunk tags, the .srb
    # deflate, the .chart header, the per-difficulty table, the tied-variant
    # walks and the SqIn step check are rows in tests/single_owner/rules/.
    Add-RowItems @('C')

    # Fixture spellings no scan row asks about yet, outside the shared header
    # that owns them. The MIDI row ("Which test helper writes MThd/MTrk
    # chunks?") matches only the chunk tags, so a variable-length delta typed
    # as bytes has no row; when a row for it lands, this entry goes.
    $spellings = @(
        @{ Rx = '&\s*0x7[Ff]\b[^;]*\|\s*0x80|0x80\s*\|\s*\(|\{\s*0x8[1-9A-Fa-f],\s*0x[0-7][0-9A-Fa-f]\s*\}'; Owner = 'tests/midi_util.h'
           What = 'a MIDI variable-length delta encoded by hand'; Why = 'the varlen writer belongs in tests/midi_util.h' }
    )
    foreach ($f in $added.Keys) {
        if (-not (Test-TestFile $f) -or -not (Test-CppPath $f) -or (Test-SkippedFile $f)) { continue }
        $v = Get-Views $f
        foreach ($ln in (Get-AddedLines $f $v.NoComments.Count)) {
            $text = $v.NoComments[$ln - 1]
            foreach ($s in $spellings) {
                if ($f -eq $s.Owner) { continue }
                if ($text -cmatch $s.Rx) {
                    $why = $s.Why
                    $fix = Get-KnownCopy $f @($v.Raw[$ln - 1])
                    if ($fix) { $why += " ($(Format-KnownCopy $fix))" }
                    Add-Item 'C' $f $ln "$($s.What): $($text.Trim())" $why
                }
            }
        }
    }
}

# --------------------------------------- check 2: recompute spellings (B)

function Invoke-Check2 {
    # Recomputes a scan row names: the SP-end offset, the typed 500 squeeze
    # window, "== 1.0" on a transfer scale, the plain SP end rebuilt with
    # plusmeasure, and a test setting target_act_ticks are rows in
    # tests/single_owner/rules/. A production line a row flags (kind A)
    # prints here too.
    Add-RowItems @('B', 'A')

    # Recompute spellings no scan row asks about. Each is a question the
    # scan has no row for yet; when a row lands, its entry here goes.
    $cpp = @(
        # A row owns part of this question, not this spelling. The row "What
        # is the two-hit budget at the identity scale?" gives the budget to
        # nominal_budget_ms and catches its product shapes (2 * w,
        # hit_window * 2, squeeze_budget_ms(1.0, ...)) in src/ and tools/.
        # It does not read tests/ and does not match a bare 2.0. This entry
        # covers that: a typed 2.0 near the words "hit window" in a test.
        # When the row widens to tests/, this entry goes.
        @{ Rx = '(?<![\w.''])2\.0*(?![\w.''])'; Ctx = '(?i)hit.?window'
           What = 'a typed 2.0 beside a hit-window check'; Why = 'the hit-window rule has one owner; call it or pin the literal' },
        # No row: the scan has no row for counting tied paths a second way.
        @{ Rx = 'tied_pathcount\(\)\s*[!=]=\s*[^;]*(\d\s*\+|\+\s*\d|\.size\(\)|count\b)'
           What = 'the tied-path count worked out a second way'; Why = 'Path::recount_tied_paths owns the count; pin it' },
        # No row: this is a shape of test, not one rule's question, so no
        # single owner fits a scan row.
        @{ Rx = $assertRx + '[^;]*==\s*[^;]*([\w)\]]\s*\([^()]*\)\s*(?:\+|-(?!>)|\*|/)\s*[A-Za-z_(]|[\w)\]]\s*(?:\+|-(?!>)|\*|/)\s*[\w:<>]+\s*\()'
           What = 'an expected value computed from other calls'; Why = 'a test pins literals from one run; it never computes the expected value' },
        # No row: the same, a shape of test with no single owner.
        @{ Rx = '\b(want|wanted|expect|expected)\w*\s*=\s*[^;]*[!=]=[^;]*(\|\||&&)'
           What = 'the expected answer built from the predicate''s own comparisons'; Why = 'pin a literal list of inputs and answers instead of restating the rule' }
    )
    # No row can own these: the scan reads only .cpp and .h files.
    $py = @(
        @{ Rx = '[<>]=?\s*[A-Za-z_]\w*\.[A-Z][A-Z0-9_]{2,}\b'
           What = 'a fixture decides with the module''s own constant'; Why = 'the fixture makes the comparison production makes; pin the inputs as literals' },
        @{ Rx = $assertRx + '\s*[A-Za-z_]\w*\.[A-Z][A-Z0-9_]{2,}\s*,[^)]*[A-Za-z_]\w*\.[A-Z][A-Z0-9_]{2,}'
           What = 'a module constant checked against a formula of other constants'; Why = 'the test restates the module''s formula; pin the value' }
    )
    foreach ($f in $added.Keys) {
        if (Test-SkippedFile $f) { continue }
        $isCppTest = (Test-TestFile $f) -and (Test-CppPath $f)
        $isPyTest = (Test-TestFile $f) -and (Test-PyPath $f)
        if (-not ($isCppTest -or $isPyTest)) { continue }
        $v = Get-Views $f
        $rules = if ($isPyTest) { $py } else { $cpp }
        $reported = [System.Collections.Generic.HashSet[string]]::new()  # rule|line already shown
        foreach ($ln in (Get-AddedLines $f $v.Code.Count)) {
            $code = $v.Code[$ln - 1]
            # A C++ statement split over lines is read whole (up to 4 lines,
            # decision D49).
            $shown = $v.NoComments[$ln - 1].Trim()
            $lastLn = $ln
            if (-not $isPyTest -and $code.Trim() -and $code -notmatch '[;{}]\s*$') {
                for ($k = $ln; $k -lt [Math]::Min($v.Code.Count, $ln + 3); $k++) {
                    $code += ' ' + $v.Code[$k].Trim()
                    $shown += ' ' + $v.NoComments[$k].Trim()
                    $lastLn = $k + 1
                    if ($v.Code[$k] -match '[;{}]\s*$') { break }
                }
            }
            foreach ($r in $rules) {
                if ($reported.Contains("$($r.What)|$ln")) { continue }
                if ($code -cnotmatch $r.Rx) { continue }
                if ($r.Ctx) {
                    # Context within 2 lines either side. D49 records this
                    # reach for the typed-500 rule, which is now a scan row;
                    # the 2.0 hit-window rule kept the same reach.
                    $from = [Math]::Max(1, $ln - 2)
                    $to = [Math]::Min($v.Raw.Count, $lastLn + 2)
                    if ((($v.Raw[($from - 1)..($to - 1)]) -join "`n") -notmatch $r.Ctx) { continue }
                }
                for ($k = $ln; $k -le $lastLn; $k++) { [void]$reported.Add("$($r.What)|$k") }
                Add-Item 'B' $f $ln "$($r.What): $shown" $r.Why
            }
        }
    }
}

# ------------------------------------------- check 3: numbers with no decision

$numDocRx = [regex]::new('(?:(?<=[x×±~≈^])|(?<![\w.]))(?:0[xX][0-9a-fA-F]+|\d{1,3}(?:[,'' ]\d{3})+(?:\.\d+)?|\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)(?!\d)', 'Compiled')
function Get-Canon([double]$v) {
    $v = [Math]::Abs($v)
    if ($v -eq [Math]::Floor($v) -and $v -lt 1e15) { return ([long]$v).ToString($inv) }
    $v.ToString('G10', $inv)
}
function ConvertTo-Number([string]$t) {
    $t = $t -replace "[,' _]", ''
    if ($t -match '^0[xX]([0-9a-fA-F]+)') { return [double][Convert]::ToInt64($Matches[1], 16) }
    $t = $t -replace '[uUlLfF]+$', ''
    $d = 0.0
    if ([double]::TryParse($t, [System.Globalization.NumberStyles]::Float, $inv, [ref]$d)) { return $d }
    $null
}
function Get-DocNumbers([string]$Text) {
    $set = [System.Collections.Generic.HashSet[string]]::new()
    foreach ($m in $numDocRx.Matches($Text)) {
        $n = ConvertTo-Number $m.Value
        if ($null -ne $n) { [void]$set.Add((Get-Canon $n)) }
    }
    foreach ($m in [regex]::Matches($Text, '\b2\s*\^\s*(\d{1,2})\b')) { [void]$set.Add((Get-Canon ([Math]::Pow(2, [int]$m.Groups[1].Value)))) }
    ,$set
}
function New-Para([string]$Record, [string]$Text) {
    $words = [System.Collections.Generic.HashSet[string]]::new()
    foreach ($m in [regex]::Matches($Text.ToLowerInvariant(), '[a-z][a-z0-9]*')) { [void]$words.Add($m.Value) }
    [pscustomobject]@{ Record = $Record; Words = $words; Numbers = (Get-DocNumbers $Text) }
}
$limitWords = '(?i)depth|cap|limit|filter|band|toleran|threshold|max|min|floor|ceil|seed|worker|frame|timeout|deadline|window|leeway|slop|budget|retr|repeat|probe|sample|settle|wait|sleep'
$loopWords = '(?i)seed|trial|repeat|iteration|attempt|sample|probe|frame|retry|round|run\b'
# A doc's paragraphs: split at blank lines and, unless -WholeLists, before
# each list item, so a number and a word meet only inside one item. The
# D-records keep their lists whole: a ruling's list items share its words.
function Split-Paras([string]$Text, [switch]$WholeLists) {
    $rx = if ($WholeLists) { '\n\s*\n' } else { '\n\s*\n|\n(?=\s*(?:\d+\.|[-*])\s)' }
    $Text -split $rx | Where-Object { $_.Trim() }
}
# The name a decision record goes by ("D49", "ADR 14"), so a citation in code
# and a record read from the docs meet under one key.
function Get-RecordName([string]$Kind, [string]$Number) {
    if ($Kind -eq 'ADR') { "ADR $([int]$Number)" } else { "D$([int]$Number)" }
}

function Get-DecisionParas {
    $paras = [System.Collections.Generic.List[object]]::new()
    foreach ($rev in (@($tip, $left) | Where-Object { $_ } | Select-Object -Unique)) {
    $docs = Invoke-Git @('ls-tree', '-r', '--name-only', $rev, '--', 'docs/adr', 'CONTEXT.md', 'docs/superpowers/plans', 'docs/audit')
    foreach ($d in $docs) {
        if ($d -notmatch '\.md$') { continue }
        if ($d -like 'docs/adr/*' -or $d -eq 'CONTEXT.md') {
            # An ADR is named by its number; CONTEXT.md by its own name.
            $rec = if ($d -match '/(\d{4})[^/]*$') { Get-RecordName 'ADR' $Matches[1] } else { $d }
            foreach ($p in (Split-Paras (Get-TipText $d $rev))) { $paras.Add((New-Para $rec $p)) }
        } elseif ($d -match '^docs/superpowers/plans/[^/]+\.md$') {
            $in = $false; $buf = [System.Collections.Generic.List[string]]::new(); $level = 0
            $flush = { foreach ($p in (Split-Paras ($buf -join "`n"))) { $paras.Add((New-Para "plan $d" $p)) }; $buf.Clear() }
            foreach ($l in (Get-TipText $d $rev).Split("`n")) {
                if ($l -match '^\*\*User decisions[^*]*\*\*') { $in = $true; $level = 99; $buf.Add($l); continue }
                if ($l -match '^(#+)\s+Test limits') { $in = $true; $level = $Matches[1].Length; $buf.Add($l); continue }
                if ($in) {
                    $endBold = $level -eq 99 -and ($l -match '^\*\*(?!User decisions)[^*]+\*\*' -or $l -match '^#')
                    $endHead = $level -ne 99 -and $l -match '^(#+)\s' -and $Matches[1].Length -le $level
                    if ($endBold -or $endHead) { $in = $false; & $flush }
                    else { $buf.Add($l) }
                }
            }
            if ($buf.Count) { & $flush }
        } elseif ($d -match '^docs/audit/[^/]*decisions[^/]*\.md$') {
            $rec = $null
            foreach ($p in (Split-Paras (Get-TipText $d $rev) -WholeLists)) {
                if ($p -match '^\s*\*\*D(\d+)\b') { $rec = Get-RecordName 'D' $Matches[1] }
                elseif ($p -match '^\s*#') { $rec = $null }
                if ($rec) { $paras.Add((New-Para $rec $p)) }
            }
        }
    }
    }
    $paras
}

$stopWords = [System.Collections.Generic.HashSet[string]]::new([string[]]@(
    'const', 'constexpr', 'static', 'inline', 'auto', 'int', 'int64', 'int32', 'uint8', 'uint16', 'uint32', 'uint64',
    'double', 'float', 'bool', 'char', 'void', 'size', 'std', 'hydra', 'string', 'vector', 'return', 'for', 'while',
    'check', 'require', 'cast', 'static_cast', 'true', 'false', 'the', 'and', 'not', 'nullptr', 'value', 'values',
    'test', 'tests', 'cfg', 'opts', 'options', 'settings', 'self', 'assert', 'equal', 'almost', 'less', 'greater',
    'make', 'get', 'set', 'push', 'back', 'emplace', 'begin', 'end', 'data', 'out', 'auto', 'long', 'unsigned',
    'max', 'min', 'abs', 'fabs', 'approx', 'doctest', 'message', 'info', 'else', 'elif', 'def', 'len', 'range',
    'int64_t', 'size_t', 'uint8_t', 'new', 'ptr', 'ref', 'tmp', 'res', 'ret', 'val', 'num', 'idx', 'count'))
function Get-ContextWords([string]$Code) {
    $words = [System.Collections.Generic.HashSet[string]]::new()
    foreach ($m in [regex]::Matches($Code, '[A-Za-z_][A-Za-z0-9_]*')) {
        $id = $m.Value -replace '^k(?=[A-Z])', ''
        foreach ($w in ([regex]::Replace($id, '([a-z0-9])([A-Z])', '$1_$2') -split '_')) {
            $w = $w.ToLowerInvariant()
            if ($w.Length -ge 3 -and -not $stopWords.Contains($w) -and $w -notmatch '^\d') { [void]$words.Add($w) }
        }
    }
    ,$words
}

function Invoke-Check3 {
    if ($WholeTree) { Write-Host 'check 3 skipped: -WholeTree has no "new" numbers'; return }
    $paras = Get-DecisionParas
    $byRecord = @{}
    foreach ($p in $paras) { if (-not $byRecord.ContainsKey($p.Record)) { $byRecord[$p.Record] = [System.Collections.Generic.List[object]]::new() }; $byRecord[$p.Record].Add($p) }
    $litRx = [regex]::new('(?<![\w.''])(?:0[xX][0-9a-fA-F'']+|\d[\d'']*(?:\.\d*)?(?:[eE][-+]?\d+)?|\.\d+(?:[eE][-+]?\d+)?)(?:[uUlLfF]+)?(?![\w.''])', 'Compiled')
    # 0, 1 and 2 are counts, first and second, and off-by-one, not
    # thresholds (decision D49).
    $skip = [System.Collections.Generic.HashSet[string]]::new([string[]]@('0', '1', '2'))
    # 1000, 1024 and 1000000 next to * or / are unit factors, ms to s and
    # KiB, not limits (decision D49).
    $unitFactors = [System.Collections.Generic.HashSet[string]]::new([string[]]@('1000', '1024', '1000000'))
    # Is this number a duration? It is the first argument of a time call
    # (seconds(, sleep_for(, a timeout or deadline...); in a test, a unit
    # after it (5ms, 2s) says so too.
    $durationBeforeRx = '\b(seconds|milliseconds|minutes|microseconds|sleep_for|timeout|deadline)\w*\s*\(\s*$'
    foreach ($f in $added.Keys) {
        if (-not ((Test-CppPath $f) -or (Test-PyPath $f))) { continue }
        if (Test-SkippedFile $f) { continue }
        $isTest = Test-TestFile $f
        $v = Get-Views $f
        # Sizes of arrays this file declares: a loop or a bound that matches one is the container's size.
        $containerSizes = [System.Collections.Generic.HashSet[string]]::new()
        foreach ($cm in [regex]::Matches($v.CodeText, '\b[A-Za-z_]\w*\s+[A-Za-z_]\w*\s*\[\s*(\d+)\s*\]|std::array\s*<[^,<>]+,\s*(\d+)\s*>')) {
            [void]$containerSizes.Add("$($cm.Groups[1].Value)$($cm.Groups[2].Value)")
        }
        foreach ($ln in (Get-AddedLines $f $v.Code.Count)) {
            $code = $v.Code[$ln - 1]
            $t = $code.Trim()
            if (-not $t) { continue }
            if ($t -match '^#\s*(include|pragma|error|if|ifdef|ifndef|endif|else|elif)\b|^\s*static_assert\b|^(import|from)\s') { continue }
            $isCheck = $t -match $assertRx
            $isLoop = $t -match '^\s*(for|while)\s*\('
            foreach ($m in $litRx.Matches($code)) {
                $n = ConvertTo-Number $m.Value
                if ($null -eq $n) { continue }
                $canon = Get-Canon $n
                if ($skip.Contains($canon)) { continue }
                $before = $code.Substring(0, $m.Index).TrimEnd()
                $after = $code.Substring($m.Index + $m.Length).TrimStart()
                if ($before.EndsWith('[') -and $after.StartsWith(']')) { continue }
                if ($containerSizes.Contains($canon)) { continue }
                # A unit factor (ms to s, bytes to KiB) multiplies or divides.
                if ($unitFactors.Contains($canon) -and ($before -match '[*/]=?$' -or $after -match '^[*/]')) { continue }
                $inCompare = ($before -match '(?<![<>-])(<=?|>=?)$' -and $before -notmatch '(<<|>>|->)$') -or
                             ($after -match '^(<=?|>=?)(?![<>=])')
                # A shift amount counts as compared when its line compares.
                if (-not $inCompare -and $before -match '(<<|>>)$') {
                    $plain = $code -replace '<<|>>|->', ' ' -replace '<[\w:\s,*&]*>', ' '
                    $inCompare = $plain -match '[<>]'
                }
                if (-not $isTest) {
                    # Production: thresholds, limits, tolerances, times and
                    # fallbacks show up as a comparison, a named value (the
                    # right side of a plain "="), a returned value, a
                    # min/max/clamp bound or a duration. A number inside other
                    # arithmetic or passed to a call computes a layout or a
                    # conversion, so it is left to the reviewer.
                    $isNamed = $before -match '(?<![=!<>+\-*/%&|^])=\s*[-+(]*$|^\s*(constexpr|const|static)?[\w:<>\s]*\b[A-Z][A-Z0-9_]+\s*=\s*$'
                    $isReturn = $before -match '\breturn\s*[-+(]*$' -and $after -match '^[;)]'
                    $isBound = $before -match '\bstd::(min|max|clamp)\s*(<[^>]*>)?\s*\(([^()]*,)?\s*$'
                    $isDuration = $before -match $durationBeforeRx
                    if (-not ($inCompare -or $isNamed -or $isReturn -or $isBound -or $isDuration)) { continue }
                }
                if ($isTest) {
                    # A test's own limits: a field or constant whose name says
                    # limit, a loop over seeds or repeats, a duration, or a
                    # comparison. Ticks, notes and offsets set on a fixture are
                    # fixture data.
                    $assigned = if ($before -match '(?:(?:\.|->)(\w+)|\b(\w+))\s*=$') { "$($Matches[1])$($Matches[2])" } else { '' }
                    $isField = $assigned -and $assigned -match $limitWords
                    $isLoopLimit = $isLoop -and $t -match $loopWords -and $t -notmatch ':\s*\{'
                    $isDuration = $before -match $durationBeforeRx -or $after -match '^(ms|s|min)\b'
                    if ($isCheck -and -not $inCompare) { continue }
                    if ($isLoop -and -not $isLoopLimit -and -not $inCompare) { continue }
                    if ($isLoop -and $inCompare -and -not $isLoopLimit) { continue }
                    $isSample = $before -match '(begin\(\)|\.data\(\))\s*\+$|\bstd::(min|max)\s*(<[^>]*>)?\s*\([^()]*,$'
                    if (-not ($inCompare -or $isLoopLimit -or $isField -or $isDuration -or $isSample)) { continue }
                }
                # Cited decision on this line or the three above (D49)?
                $from = [Math]::Max(1, $ln - 3)
                $near = ($v.Raw[($from - 1)..($ln - 1)]) -join "`n"
                $cites = [System.Collections.Generic.List[string]]::new()
                foreach ($c in [regex]::Matches($near, '\bD(\d{1,3})\b')) { $cites.Add((Get-RecordName 'D' $c.Groups[1].Value)) }
                foreach ($c in [regex]::Matches($near, '\bADR\s*0*(\d{1,4})\b')) { $cites.Add((Get-RecordName 'ADR' $c.Groups[1].Value)) }
                if ($cites.Count) {
                    $found = $false
                    foreach ($r in $cites) { if ($byRecord.ContainsKey($r)) { foreach ($p in $byRecord[$r]) { if ($p.Numbers.Contains($canon)) { $found = $true } } } }
                    if (-not $found) {
                        Add-Item 'D' $f $ln "$($m.Value) in: $($v.NoComments[$ln - 1].Trim())" "cites $(($cites | Select-Object -Unique) -join ', '), which does not contain $($m.Value)"
                    }
                    continue
                }
                # The words that say what the number is: the line's own names,
                # or, when it has none, the three lines above it.
                $words = Get-ContextWords $code
                if ($words.Count -eq 0) { $words = Get-ContextWords $near }
                $found = $false
                foreach ($p in $paras) {
                    if (-not $p.Numbers.Contains($canon)) { continue }
                    foreach ($w in $words) {
                        if ($p.Words.Contains($w) -or $p.Words.Contains("${w}s") -or ($w.EndsWith('s') -and $p.Words.Contains($w.Substring(0, $w.Length - 1)))) { $found = $true; break }
                    }
                    if ($found) { break }
                }
                if (-not $found) {
                    $about = if ($words.Count) { " beside any of: $(($words | Select-Object -First 4) -join ', ')" } else { '' }
                    Add-Item 'D' $f $ln "$($m.Value) in: $($v.NoComments[$ln - 1].Trim())" "no ADR, CONTEXT.md, plan decision or D-record names $($m.Value)$about"
                }
            }
        }
    }
}

# ------------------------------------------- check 4: scans outside the scan file

function Invoke-Check4 {
    # A test reading the source tree through HYDRA_SOURCE_DIR, or walking it,
    # is a scan row ("Which test reads the source tree?", "Which test walks
    # the source tree?").
    Add-RowItems @('E')
    $rowLines = [System.Collections.Generic.HashSet[string]]::new()
    foreach ($h in (Get-RowHits)) { if ($h.Kind -eq 'E') { [void]$rowLines.Add("$($h.File):$($h.Line)") } }
    foreach ($f in $added.Keys) {
        # Every code folder but src: a scan is a test's or a tool's to make.
        if ((Get-TopFolder $f) -eq 'src') { continue }
        if (Test-SkippedFile $f) { continue }
        $py = Test-PyPath $f
        if (-not ((Test-CppPath $f) -or $py)) { continue }
        $v = Get-Views $f
        foreach ($ln in (Get-AddedLines $f $v.NoComments.Count)) {
            if ($rowLines.Contains("${f}:$ln")) { continue }
            $t = $v.NoComments[$ln - 1]
            $what = $null
            if ($py) {
                # No row can own this: the scan reads only .cpp and .h files.
                if ($t -match '(\bos\.walk|\bos\.listdir|\.r?glob)\s*\([^)]*src' -or
                    $t -match ('\bopen\s*\([^)]*' + $cppExt + '[''"]')) { $what = 'reads the source tree' }
            } else {
                # No row: "Which test reads the source tree?" matches only
                # HYDRA_SOURCE_DIR, so a test that opens a source file by a
                # path built another way (sourcetree::root() / "src" / ...)
                # passes the scan. When that row widens, this entry goes.
                if ($t -match ('\b(ifstream|fopen|_wfopen|open_file\w*)\b[^;]*(' + $cppExt + '"|"[^"]*/?src/)')) { $what = 'opens a source file' }
            }
            if ($what) {
                Add-Item 'E' $f $ln "$($what): $($t.Trim())" "the single-owner scan is the one scan of the source tree; make this a row in $rulesDir"
            }
        }
    }
    # New rule rows in the rule files: the rows read above, with Added set
    # when the range wrote any of their lines (only when the rows are read
    # from the range's own last commit).
    foreach ($r in ($rows | Where-Object Added)) {
        $lists = [ordered]@{ 'must-match' = $r.MustMatch; 'must-not-match' = $r.MustNotMatch }
        foreach ($name in $lists.Keys) {
            if ($lists[$name].Count -eq 0) {
                Add-Item 'E' $r.File $r.Line "scan row ""$($r.Question)"" has no $name examples" 'every scan row ships with lines it must match and must not match, so the self-test proves its pattern'
            }
        }
        if ($r.Pattern -match '\(\?<?!\(?[A-Za-z_]\w*(\|[A-Za-z_]\w*)*\)?\\s\*\(?\\?\.') {
            Add-Item 'E' $r.File $r.Line "scan row ""$($r.Question)"" exempts variable names inside its pattern ($($Matches[0])...)" 'an exemption in the pattern applies in every file; list the allowed lines as owner lines with their reason'
        }
    }
}

# --------------------------------------- check 5: second homes in src/ (F)

# How big a repeat must be to count. These are the user's decision D113
# item 8 in docs/audit/2026-10-03-fix-decisions.md, starting values the
# first real runs may tune. This is the one place they live.
$minRepeatedString = 12  # characters in a string literal, as written between its quotes
$minRecordFields = 3     # fields a struct or class needs before it is compared
$minSharedFields = 4     # field names two records share before they count as one idea

# A struct, class, enum or union definition: the keyword, an attribute, the
# name, then "final" or a base clause and the opening brace. A base clause
# never starts "::", so "struct Engine::Impl {" (a member type defined out of
# line) is not read as a struct named Engine.
$typeDefRx = [regex]::new('\b(?<kind>enum\s+class|enum\s+struct|enum|struct|class|union)\s+(?:\[\[[^\]]*\]\]\s*)?(?<name>[A-Za-z_]\w*)\s*(?:final\s*)?(?::(?!:)[^;{]*)?\{', 'Compiled')

# The data members of the struct or class whose body is the "{" at $Open:
# name -> line. Every block nested in the body (a method body, a brace
# initialiser, a nested type) is blanked first, so a nested type's members
# and its access labels never count as the outer type's. A statement ends
# at ";", or at a nested block's "}" when no "," ";" "=" or ")" follows it
# (a method body ends a statement; a brace initialiser does not). A statement
# with a "(" is a method, and one starting with a declaration keyword
# (using, static, a nested type...) declares no member. One statement can
# declare several members ("int64_t base, combo, sp;"), so its declarators
# are split at commas outside <...>, (...), [...] and {...}.
function Get-RecordFields([object]$V, [int]$Open, [int]$Close) {
    $start = $Open + 1
    $chars = $V.CodeText.Substring($start, $Close - $start).ToCharArray()
    $closes = (Get-Braces $V).Close
    foreach ($kid in (Get-BraceChildren $V $Open)) {
        $kc = $closes[$kid]
        if ($null -eq $kc) { continue }
        for ($i = $kid + 1; $i -lt $kc; $i++) { if ($chars[$i - $start] -ne "`n") { $chars[$i - $start] = ' ' } }
    }
    $body = [string]::new([char[]]$chars)
    $fields = [ordered]@{}
    $from = 0
    $ends = @(@([regex]::Matches($body, ';|\}(?=\s*[^\s,;=)])') | ForEach-Object { $_.Index }) + $body.Length)
    foreach ($e in $ends) {
        $stmt = $body.Substring($from, $e - $from)
        $stmtAt = $from
        $from = $e + 1
        $stmt = [regex]::Replace($stmt, '(?m)^\s*(?:public|private|protected)\s*:', { param($m) Blank $m.Value })
        $t = $stmt.Trim()
        if (-not $t -or $t.Contains('(') -or $t -match '^(?:using|friend|static|typedef|template|enum|struct|class|union)\b') { continue }
        $depth = 0; $partAt = 0; $partNo = 0
        for ($i = 0; $i -le $stmt.Length; $i++) {
            if ($i -lt $stmt.Length) {
                $c = $stmt[$i]
                if ('<([{'.IndexOf($c) -ge 0) { $depth++; continue }
                if ('>)]}'.IndexOf($c) -ge 0) { $depth--; continue }
                if ($c -ne ',' -or $depth -ne 0) { continue }
            }
            $part = $stmt.Substring($partAt, $i - $partAt)
            $eq = $part.IndexOf('=')
            if ($eq -ge 0) { $part = $part.Substring(0, $eq) }
            $m = [regex]::Match($part, '([A-Za-z_]\w*)\s*(?:\{\s*\}\s*|\[[^\]]*\]\s*)*$')
            # The first declarator carries the type, so its name follows one.
            $typed = $partNo -gt 0 -or ($m.Success -and $part.Substring(0, $m.Index).Trim())
            if ($m.Success -and $typed -and -not $cppKeywords.Contains($m.Groups[1].Value) -and -not $fields.Contains($m.Groups[1].Value)) {
                $fields[$m.Groups[1].Value] = Get-LineOf $V ($start + $stmtAt + $partAt + $m.Groups[1].Index)
            }
            $partAt = $i + 1; $partNo++
        }
    }
    $fields
}

# Every other home of one thing: "file:line, file:line", one per file.
function Format-OtherHomes([object[]]$Homes, [string]$File) {
    (@($Homes | Where-Object { $_.File -ne $File } | Group-Object File | ForEach-Object { "$($_.Name):$($_.Group[0].Line)" }) -join ', ')
}

function Invoke-Check5 {
    # The audit that asked for this check covers src/ only (D113 item 1), so
    # both sides of every comparison are src/ C++ files at the range's tip.
    $srcFiles = @($allFiles | Where-Object { (Get-TopFolder $_) -eq 'src' -and (Test-CppPath $_) })
    if (-not @($srcFiles | Where-Object { $added.ContainsKey($_) }).Count) { return }
    $types = [System.Collections.Generic.List[object]]::new()
    $records = [System.Collections.Generic.List[object]]::new()
    $funcs = [System.Collections.Generic.List[object]]::new()
    $texts = [System.Collections.Generic.List[object]]::new()
    foreach ($f in $srcFiles) {
        $v = Get-Views $f
        foreach ($m in $typeDefRx.Matches($v.CodeText)) {
            $kind = $m.Groups['kind'].Value -replace '\s+', ' '
            $type = [pscustomobject]@{ File = $f; Line = (Get-LineOf $v $m.Groups['name'].Index); Kind = $kind; Name = $m.Groups['name'].Value }
            $types.Add($type)
            if ($kind -ne 'struct' -and $kind -ne 'class') { continue }
            $open = $m.Index + $m.Length - 1
            $close = Get-CloseBrace $v $open $v.CodeText.Length
            if ($close -lt 0) { continue }
            $fields = Get-RecordFields $v $open $close
            if ($fields.Count -ge $minRecordFields) { $records.Add([pscustomobject]@{ File = $f; Line = $type.Line; Kind = $kind; Name = $type.Name; Fields = $fields }) }
        }
        if (Test-CppSourcePath $f) {
            foreach ($d in (Get-Definitions $f -NamesOnly)) { if (-not $d.Lambda) { $funcs.Add($d) } }
        }
        # String literals as the lexer reads them, so a quote inside a
        # comment is never one; raw strings and #include paths are left out.
        foreach ($m in $cppLex.Matches((Get-TipText $f))) {
            $g = $m.Groups['s']
            if (-not $g.Success -or $g.Value[0] -ne '"') { continue }
            $s = $g.Value.Substring(1, $g.Value.Length - 2)
            if ($s.Length -lt $minRepeatedString) { continue }
            $line = Get-LineOf $v $g.Index
            if ($v.Code[$line - 1] -match '^\s*#\s*include\b') { continue }
            $texts.Add([pscustomobject]@{ File = $f; Line = $line; Text = $s })
        }
    }

    $why = 'a second home for an idea that already has one; name the concept it extends, or reuse it (derive-once review, step 8)'
    foreach ($g in ($types | Group-Object Name -CaseSensitive)) {
        if (@($g.Group | Select-Object -ExpandProperty File -Unique).Count -lt 2) { continue }
        foreach ($t in ($g.Group | Where-Object { Test-Added $_.File $_.Line })) {
            $others = Format-OtherHomes $g.Group $t.File
            if ($others) { Add-Item 'F' $t.File $t.Line "$($t.Kind) $($t.Name) is also defined at $others" $why }
        }
    }
    foreach ($r in $records) {
        foreach ($o in $records) {
            if ($o.File -eq $r.File) { continue }
            $shared = @($r.Fields.Keys | Where-Object { $o.Fields.Contains($_) } | Sort-Object)
            if ($shared.Count -lt $minSharedFields) { continue }
            # The record counts when the range added its name or one of the shared fields.
            $at = @(@($r.Line) + @($shared | ForEach-Object { $r.Fields[$_] }) | Where-Object { Test-Added $r.File $_ })
            if (-not $at.Count) { continue }
            Add-Item 'F' $r.File $at[0] "$($r.Kind) $($r.Name) shares $($shared.Count) field names with $($o.Kind) $($o.Name) at $($o.File):$($o.Line): $($shared -join ', ')" $why
        }
    }
    foreach ($g in ($funcs | Group-Object Name -CaseSensitive)) {
        if (@($g.Group | Select-Object -ExpandProperty File -Unique).Count -lt 2) { continue }
        foreach ($d in ($g.Group | Where-Object { Test-Added $_.File $_.Line })) {
            $others = Format-OtherHomes $g.Group $d.File
            if ($others) { Add-Item 'F' $d.File $d.Line "free function $($d.Name) is also defined at $others" $why }
        }
    }
    foreach ($g in ($texts | Group-Object Text -CaseSensitive)) {
        if (@($g.Group | Select-Object -ExpandProperty File -Unique).Count -lt 2) { continue }
        # One line per file: the first place this range wrote the text there.
        foreach ($fg in ($g.Group | Where-Object { Test-Added $_.File $_.Line } | Group-Object File)) {
            $first = $fg.Group[0]
            Add-Item 'F' $first.File $first.Line """$($first.Text)"" is also written at $(Format-OtherHomes $g.Group $first.File)" $why
        }
    }
}

# ---------------------------------------------------------------- range size

# Is this range too big to review in one read? A reviewer reads the whole
# range in one pass, and big ranges hide copies. The user's decision is D82
# item 2 in docs/audit/2026-10-03-fix-decisions.md, a starting value to be
# tuned later. This is the one place it lives.
$largeRangeLines = 400
# Which top folders do not count toward the size: vendored code and test
# data, which the reviewer does not read line by line.
$sizeExcluded = @('third_party', 'testdata')
# How the two size lines start. The self-test loads these from this file.
$sizePrefix = 'range size: '
$largeRangePrefix = 'large range: '
# How many lines does the range change? Added plus deleted, over every file
# the range touches outside $sizeExcluded. A binary file counts no lines
# (git gives it no line counts). This is the one place that rule lives.
function Get-RangeSize {
    $add = 0; $del = 0
    foreach ($l in (Invoke-Git @('diff', '--numstat', '--no-color', '--no-ext-diff', '--no-renames', $base, $tip))) {
        if ($l -notmatch '^(\d+|-)\t(\d+|-)\t(.+)$') { continue }
        if ($Matches[1] -eq '-') { continue }  # a binary file
        if ($sizeExcluded -contains (Get-TopFolder $Matches[3].TrimStart('"'))) { continue }
        $add += [int]$Matches[1]; $del += [int]$Matches[2]
    }
    [pscustomobject]@{ Added = $add; Deleted = $del; Total = $add + $del }
}

# ---------------------------------------------------------------- run

for ($checkNo = 1; $checkNo -le $checks.Count; $checkNo++) {
    if ($Disable -notcontains $checkNo) { & $checks[$checkNo - 1] }
}

foreach ($w in $rowWarnings) { Write-Host "$warnPrefix$w (this row could not be read as expected: std::regex and .NET may read its pattern differently; trust the scan)" }
if (-not $ruleFiles.Count) { Write-Host "$warnPrefix$rulesDir has no rule files at $($rulesRev.Substring(0, 7)), so no scan rows were applied" }

$order = @{ C = 0; B = 1; A = 2; D = 3; E = 4; F = 5 }
$sorted = $items | Sort-Object @{ e = { $order[$_.Kind] } }, File, Line, What -Unique
foreach ($i in $sorted) {
    "{0} {1}:{2}  {3}  -- {4}" -f $i.Kind, $i.File, $i.Line, $i.What, $i.Why
}
if (-not $WholeTree) {
    $size = Get-RangeSize
    Write-Host ("{0}{1} changed lines ({2} added, {3} deleted), not counting {4}" -f $sizePrefix, $size.Total, $size.Added, $size.Deleted, (($sizeExcluded | ForEach-Object { "$_/" }) -join ' or '))
    if ($size.Total -gt $largeRangeLines) {
        Write-Host ("{0}{1} changed lines is over the {2}-line threshold. Split this range into smaller merges before review. This is a warning, not a gate." -f $largeRangePrefix, $size.Total, $largeRangeLines)
    }
}
$counts = ($sorted | Group-Object Kind | ForEach-Object { "$($_.Name) $($_.Count)" }) -join ', '
Write-Host ("derive_once_precheck: {0} items ({1}) for {2} at {3}" -f @($sorted).Count, $(if ($counts) { $counts } else { 'none' }), $(if ($WholeTree) { 'the whole tree' } else { $Range }), $tip.Substring(0, 7))
