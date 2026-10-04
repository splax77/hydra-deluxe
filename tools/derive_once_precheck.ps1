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
The kind letters are the merge-gate audit's (2026-10-04, section 4): C a
copied test helper or fixture, B a test that recomputes what production
computes, D a number with no decision, E a source scan outside the one scan
file. A line ending "(known copy: ...)" is already in known_copies() of
tests/test_single_owner.cpp with the fix that removes it.

THE FOUR CHECKS

1. Helpers defined twice (kind C). Every function defined at namespace level
   in a .cpp or .h under tests/, and every named lambda (auto f = [..](..) {),
   is matched against the rest of tests/ two ways: by name, and by body. For
   the body match, comments and the text inside string literals are dropped,
   namespace prefixes (std::, hydra::) are dropped, and every name that is not
   a C++ keyword or a member after "." or "->" is renamed in order of first
   use, so a copy with its function and variables renamed still matches.
   Bodies under 30 tokens are skipped by body (too many tiny helpers look
   alike); they still match by name. A group is printed when one of its
   definitions starts on a line the range adds. Skipped: member functions
   inside a struct or class, copies inside one file, and the doctest macros
   (TEST_CASE, SUBCASE). Same-named helpers with different bodies are still
   printed, marked "bodies differ": two helpers with one name in two test
   files is how most copies start.
   The same check also looks for the fixture spellings the audit found copied
   under other names, outside the shared header that owns each one: a MIDI
   MThd/MTrk chunk or a hand-encoded variable-length delta (tests/midi_util.h),
   a raw deflate for a .srb (tests/srb_util.h), a minimal .chart "[Song]\n"
   or "[SyncTrack]\n" header in a string (tests/chart_text.h), a
   per-difficulty literal table (tests/difficulty_literals.h), a loop over a
   path's tied variants (tests/record_fixtures.h), and an SqIn step test
   typed by hand (tests/bank_check.h, or is_sqin_kind in src/core/model.h).
   tests/test_single_owner.cpp is skipped: its strings are pattern examples.

2. Recompute spellings in tests (kind B). Added lines under tests/ (C++ and
   Python) and in Python test files under tools/ are matched against the
   spellings merge-gate.md section 4 lists for kind B: an SP-end offset
   rebuilt from two times (".ms() - x.ms()" or "x_ms - y.ms()"); a typed 500
   (the squeeze window) within two lines of a squeeze, window, SqIn/SqOut,
   leeway, deact or fabs( word; a typed 2.0 beside a hit window; "== 1.0" or
   "!= 1.0" on an early, late or transfer scale, where is_scaled exists; the
   plain SP end rebuilt with plusmeasure(..sp_bars_to_measures(..)); a
   tied-path count checked against a second count; a test setting
   target_act_ticks itself (re-running the targeted search's filter); a
   CHECK whose expected side is arithmetic over other calls; an expected
   answer built from the predicate's own comparisons ("want = a == x || ...");
   and, in Python tests, a fixture that decides with the module's own
   constants or asserts one module constant equals a formula of others.
   A C++ statement split over up to four lines is read as one. Skipped:
   comments, the text inside strings, and tests/test_single_owner.cpp.

3. New numbers with no decision (kind D). Every numeric literal on an added
   code line under src/, tools/ and tests/ (C++ and Python; not comments,
   strings, #include, #pragma, #error or static_assert lines, and not
   tests/test_single_owner.cpp, whose literals are pattern examples). The
   decision text is read at both ends of the range, the branch's last commit
   and the left side (main, for main...HEAD), because decisions are often
   recorded on main while the branch is open: docs/adr/*.md, CONTEXT.md, the
   "User decisions" block and any "Test limits" section of each plan in
   docs/superpowers/plans/, and the D-records in docs/audit/*decisions*.md
   (the user's numbered rulings, which reviewers count as decisions; the
   brief names only the first three places). If the line or the three lines
   above it cite a
   decision (D43, ADR 0014), the number must appear in that decision.
   Otherwise it must appear in one decision paragraph together with one of
   the line's own words (depth, window, floor...), so "40" in an unrelated
   sentence does not decide a depth of 40. Numbers are compared by value, so
   1'000'000, 1,000,000 and 1000000 are one number, 0xFFFF is 65,535, 500.0 is
   500, and "2^46" in a doc decides a 46.
   Skipped, and why:
   - 0, 1, 2 and their negatives: counts, first/second, off-by-one and
     halving are arithmetic, not thresholds.
   - A number directly inside [ ]: an array size or an index. A number equal
     to an array size the file declares (key[256], std::array<T, 4>): a loop
     bound or limit that matches its container.
   - 1000, 1024 and 1000000 next to * or /: unit factors (ms to s, KiB).
   - In src/ and tools/, a number inside other arithmetic or passed to a
     call: that computes a layout or a conversion (pos += 8, fits(buf, 16)).
     Production numbers are checked where a threshold, limit, tolerance,
     time or fallback shows up: a comparison (a shift amount counts when
     its line compares, as in "x >= (1 << 30)"), the right side of a plain
     "=", a returned value, a std::min/max/clamp bound, or a duration.
     Table rows in braces are data and are skipped.
   - In tests, a number on a CHECK, REQUIRE or assert line that is not in a
     < > <= >= comparison: that is a pinned result from a run, which is what
     a test should hold, not a threshold.
   - In tests, fixture data: tick lists, notes, offsets and other values set
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

4. Scans outside the scan file (kind E). Any added line in a C++ or Python
   file under tests/ or tools/, other than tests/test_single_owner.cpp, that
   reads the source tree: HYDRA_SOURCE_DIR, a directory iterator in a file
   that names "src", an ifstream or open() of a .cpp or .h path or of a path
   under src/, or os.walk/glob/rglob over src. This script and its self-test
   are skipped: they read source text because they are the pre-review tool,
   not a test. Then every rule row the range adds to rules() in
   tests/test_single_owner.cpp is checked for an empty or missing must-match
   or must-not-match list, and for a negative lookahead that names variables
   inside its pattern (an exemption no owner line records, which applies in
   every file).

What it cannot see: a production rule written twice (kind A), a copy whose
shape it does not know, a decision that exists only as a reviewer's
judgement, and code the range does not add.
#>
[CmdletBinding()]
param(
    [string]$Range = 'main...HEAD',
    [string]$Repo = '.',
    [switch]$WholeTree,
    [string[]]$Disable = @()
)

$ErrorActionPreference = 'Stop'
# -Disable takes check numbers, as a list or comma-separated (-Disable 2 or -Disable 1,3).
$Disable = @($Disable | ForEach-Object { "$_" -split ',' } | Where-Object { $_ } | ForEach-Object {
    if ($_ -notmatch '^[1-4]$') { throw "-Disable takes check numbers 1 to 4, not '$_'" }; [int]$_ })
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

$script:Root = (& git -C $Repo rev-parse --show-toplevel 2>$null)
if ($LASTEXITCODE -ne 0 -or -not $script:Root) { throw "not a git repository: $Repo" }
$script:Root = "$($script:Root)".Trim()

$left = $null  # the range's left side, whose decisions count too (main, for main...HEAD)
if ($Range -match '^(.*?)\.\.\.(.*)$') {
    $a = if ($Matches[1]) { $Matches[1] } else { 'HEAD' }
    $b = if ($Matches[2]) { $Matches[2] } else { 'HEAD' }
    $tip = @(Invoke-Git @('rev-parse', '--verify', "$b^{commit}"))[0]
    $base = @(Invoke-Git @('merge-base', $a, $b))[0]
    $left = @(Invoke-Git @('rev-parse', '--verify', "$a^{commit}"))[0]
} elseif ($Range -match '^(.*?)\.\.(.*)$') {
    $a = if ($Matches[1]) { $Matches[1] } else { 'HEAD' }
    $b = if ($Matches[2]) { $Matches[2] } else { 'HEAD' }
    $tip = @(Invoke-Git @('rev-parse', '--verify', "$b^{commit}"))[0]
    $base = @(Invoke-Git @('rev-parse', '--verify', "$a^{commit}"))[0]
    $left = $base
} else {
    $tip = @(Invoke-Git @('rev-parse', '--verify', "$Range^{commit}"))[0]
    $base = if ($WholeTree) { $null } else { @(Invoke-Git @('rev-parse', '--verify', "$Range^1"))[0] }
}

$script:FileCache = @{}
function Get-TipText([string]$Path, [string]$Rev = $tip) {
    $key = "${Rev}:$Path"
    if (-not $script:FileCache.ContainsKey($key)) {
        $lines = Invoke-Git @('show', $key)
        $script:FileCache[$key] = (($lines | ForEach-Object { $_.TrimEnd("`r") }) -join "`n")
    }
    $script:FileCache[$key]
}

$allFiles = @(Invoke-Git @('ls-tree', '-r', '--name-only', $tip, '--', 'src', 'tests', 'tools'))

# Added lines per file: path -> HashSet[int] (1-based, in the tip's numbering).
$added = @{}
if ($WholeTree) {
    foreach ($f in $allFiles) { $added[$f] = $null }  # $null means every line
} else {
    $cur = $null
    foreach ($l in (Invoke-Git @('diff', '--no-color', '--no-ext-diff', '--no-renames', '-U0', $base, $tip, '--', 'src', 'tests', 'tools'))) {
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
$script:LexCache = @{}
function Get-Views([string]$Path) {
    if ($script:LexCache.ContainsKey($Path)) { return $script:LexCache[$Path] }
    $text = Get-TipText $Path
    $rx = if ($Path -match '\.py$') { $pyLex } else { $cppLex }
    $noComments = $rx.Replace($text, { param($m) if ($m.Groups['c'].Success) { Blank $m.Value } else { $m.Value } })
    $code = $rx.Replace($text, { param($m) if ($m.Groups['c'].Success) { Blank $m.Value } else { Blank-Inner $m.Value } })
    $v = [pscustomobject]@{
        Raw        = $text.Split("`n")
        NoComments = $noComments.Split("`n")
        Code       = $code.Split("`n")
        CodeText   = $code
        NcText     = $noComments
    }
    $script:LexCache[$Path] = $v
    $v
}

# ------------------------------------------------------------------- output

$items = [System.Collections.Generic.List[object]]::new()
function Add-Item([string]$Kind, [string]$File, [int]$Line, [string]$What, [string]$Why) {
    $items.Add([pscustomobject]@{ Kind = $Kind; File = $File; Line = $Line; What = $What; Why = $Why })
}

# known_copies() of tests/test_single_owner.cpp: file -> list of (line text, fix)
$known = @{}
$scanFile = 'tests/test_single_owner.cpp'

# Splits a braced initializer list into its top-level elements, each as
# (start offset, end offset) in the text, using the string-blanked view so
# braces and commas inside strings do not count.
function Get-TopElements([string]$Code, [int]$Open, [int]$Close) {
    $out = [System.Collections.Generic.List[object]]::new()
    $depth = 0; $start = -1
    for ($i = $Open + 1; $i -lt $Close; $i++) {
        $ch = $Code[$i]
        if ($ch -eq '{' -or $ch -eq '(') { if ($depth -eq 0 -and $ch -eq '{') { $start = $i }; $depth++ }
        elseif ($ch -eq '}' -or $ch -eq ')') { $depth--; if ($depth -eq 0 -and $ch -eq '}' -and $start -ge 0) { $out.Add(@($start, $i)); $start = -1 } }
    }
    ,$out
}
function Split-Fields([string]$Code, [int]$Open, [int]$Close) {
    $out = [System.Collections.Generic.List[object]]::new()
    $depth = 0; $start = $Open + 1
    for ($i = $Open + 1; $i -lt $Close; $i++) {
        $ch = $Code[$i]
        if ('{(['.Contains($ch)) { $depth++ }
        elseif ('})]'.Contains($ch)) { $depth-- }
        elseif ($ch -eq ',' -and $depth -eq 0) { $out.Add(@($start, $i)); $start = $i + 1 }
    }
    if ($Close -gt $start -and $Code.Substring($start, $Close - $start).Trim()) { $out.Add(@($start, $Close)) }
    ,$out
}
$strRx = [regex]'R"(?<d>[^(\s"\\]{0,16})\((?<r>[\s\S]*?)\)\k<d>"|"(?<s>(?:\\.|[^"\\\n])*)"'
function Get-Strings([string]$Text) {
    $out = [System.Collections.Generic.List[string]]::new()
    foreach ($m in $strRx.Matches($Text)) {
        if ($m.Groups['r'].Success) { $out.Add($m.Groups['r'].Value) }
        else { $out.Add(($m.Groups['s'].Value -replace '\\(["\\''])', '$1' -replace '\\n', "`n")) }
    }
    ,$out
}
# The body of a function named $Name in a file: offsets of its opening and
# closing brace in the string-blanked text.
function Find-FunctionBody([string]$Code, [string]$Name) {
    $m = [regex]::Match($Code, "\b$([regex]::Escape($Name))\s*\(\s*\)\s*\{")
    if (-not $m.Success) { return $null }
    $open = $m.Index + $m.Length - 1
    $depth = 0
    for ($i = $open; $i -lt $Code.Length; $i++) {
        if ($Code[$i] -eq '{') { $depth++ } elseif ($Code[$i] -eq '}') { $depth--; if ($depth -eq 0) { return @($open, $i) } }
    }
    $null
}
function Get-InitList([string]$Code, [int]$BodyOpen, [int]$BodyClose) {
    # the first "= {" inside the body is the static table's initializer
    $m = [regex]::new('=\s*\{').Match($Code, $BodyOpen, $BodyClose - $BodyOpen)
    if (-not $m.Success) { return $null }
    $open = $m.Index + $m.Length - 1
    $depth = 0
    for ($i = $open; $i -lt $BodyClose; $i++) {
        if ($Code[$i] -eq '{') { $depth++ } elseif ($Code[$i] -eq '}') { $depth--; if ($depth -eq 0) { return @($open, $i) } }
    }
    $null
}

if ($allFiles -contains $scanFile) {
    $sv = Get-Views $scanFile
    $body = Find-FunctionBody $sv.CodeText 'known_copies'
    if ($body) {
        $init = Get-InitList $sv.CodeText $body[0] $body[1]
        if ($init) {
            foreach ($el in (Get-TopElements $sv.CodeText $init[0] $init[1])) {
                $fields = Split-Fields $sv.CodeText $el[0] $el[1]
                $vals = foreach ($f in $fields) { (Get-Strings $sv.NcText.Substring($f[0], $f[1] - $f[0])) -join '' }
                if (@($vals).Count -ge 4) {
                    if (-not $known.ContainsKey($vals[1])) { $known[$vals[1]] = [System.Collections.Generic.List[object]]::new() }
                    $known[$vals[1]].Add([pscustomobject]@{ Text = $vals[2].Trim(); Fix = $vals[3] })
                }
            }
        }
    }
}
function Get-KnownCopy([string]$File, [string[]]$LineTexts) {
    if (-not $known.ContainsKey($File)) { return $null }
    foreach ($t in $LineTexts) {
        foreach ($k in $known[$File]) { if ($t.Trim() -eq $k.Text) { return $k.Fix } }
    }
    $null
}

function Test-CppPath([string]$p) { $p -match '\.(cpp|h|hpp|cc)$' }
function Test-PyPath([string]$p) { $p -match '\.py$' }
$selfFiles = @('tools/derive_once_precheck.ps1', 'tools/test_derive_once_precheck.ps1')

# ------------------------------------------- check 1: helpers defined twice

$cppKeywords = [System.Collections.Generic.HashSet[string]]::new([string[]]@(
    'alignas', 'alignof', 'auto', 'bool', 'break', 'case', 'catch', 'char', 'class', 'const', 'constexpr',
    'const_cast', 'continue', 'decltype', 'default', 'delete', 'do', 'double', 'dynamic_cast', 'else', 'enum',
    'explicit', 'extern', 'false', 'float', 'for', 'friend', 'goto', 'if', 'inline', 'int', 'long', 'mutable',
    'namespace', 'new', 'noexcept', 'nullptr', 'operator', 'private', 'protected', 'public', 'reinterpret_cast',
    'return', 'short', 'signed', 'sizeof', 'static', 'static_assert', 'static_cast', 'struct', 'switch',
    'template', 'this', 'throw', 'true', 'try', 'typedef', 'typename', 'union', 'unsigned', 'using', 'virtual',
    'void', 'volatile', 'while', 'size_t', 'int8_t', 'int16_t', 'int32_t', 'int64_t', 'uint8_t', 'uint16_t',
    'uint32_t', 'uint64_t'))
$notFunctionNames = [System.Collections.Generic.HashSet[string]]::new([string[]]@(
    'if', 'for', 'while', 'switch', 'catch', 'return', 'sizeof', 'decltype', 'alignof', 'static_assert',
    'TEST_CASE', 'SUBCASE', 'TEST_SUITE', 'CHECK', 'REQUIRE', 'main'))
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

function Get-Definitions([string]$Path) {
    $v = Get-Views $Path
    $code = $v.CodeText
    # Brace structure: for each '{', its closing offset and whether every
    # enclosing block is a namespace (so a function there is a free function).
    $close = @{}; $top = @{}
    $stack = [System.Collections.Generic.List[object]]::new()
    foreach ($m in [regex]::Matches($code, '[{}]')) {
        if ($m.Value -eq '{') {
            $from = [Math]::Max(0, $m.Index - 200)
            $before = $code.Substring($from, $m.Index - $from)
            $cut = $before.LastIndexOfAny([char[]]@(';', '{', '}'))
            $head = $before.Substring($cut + 1)
            $isNs = $head -match '\bnamespace\b[^;{}]*$|\bextern\s*"[^"]*"\s*$'
            $allNs = $true
            foreach ($s in $stack) { if (-not $s.Ns) { $allNs = $false; break } }
            $top[$m.Index] = $allNs
            $stack.Add([pscustomobject]@{ Pos = $m.Index; Ns = $isNs })
        } elseif ($stack.Count -gt 0) {
            $close[$stack[$stack.Count - 1].Pos] = $m.Index
            $stack.RemoveAt($stack.Count - 1)
        }
    }
    $lineStarts = [System.Collections.Generic.List[int]]::new(); $lineStarts.Add(0)
    for ($i = 0; $i -lt $code.Length; $i++) { if ($code[$i] -eq "`n") { $lineStarts.Add($i + 1) } }
    $lineOf = { param($off) $lo = 0; $hi = $lineStarts.Count - 1
        while ($lo -lt $hi) { $mid = [int][Math]::Floor(($lo + $hi + 1) / 2); if ($lineStarts[$mid] -le $off) { $lo = $mid } else { $hi = $mid - 1 } }
        $lo + 1 }
    $defs = [System.Collections.Generic.List[object]]::new()
    $add = {
        param($m, $isLambda)
        $open = $m.Index + $m.Length - 1
        if (-not $close.ContainsKey($open)) { return }
        if (-not $isLambda -and -not $top[$open]) { return }
        $name = $m.Groups['name'].Value
        if ($notFunctionNames.Contains($name)) { return }
        if (-not $isLambda -and ($m.Groups['pre'].Value -match '\b(return|else|new|delete|throw|case|do|goto|co_return)\b')) { return }
        $end = $close[$open]
        $norm = Get-Normalised ($m.Groups['params'].Value + ' ' + $code.Substring($open, $end - $open + 1))
        $first = & $lineOf ($m.Index + ($m.Value.Length - $m.Value.TrimStart().Length))
        $last = & $lineOf $end
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
    $testFiles = @($allFiles | Where-Object { $_ -like 'tests/*' -and (Test-CppPath $_) })
    $defs = [System.Collections.Generic.List[object]]::new()
    foreach ($f in $testFiles) { foreach ($d in (Get-Definitions $f)) { $defs.Add($d) } }

    $groups = [System.Collections.Generic.List[object]]::new()
    # by name (free functions only; lambdas are local names)
    foreach ($g in ($defs | Where-Object { -not $_.Lambda } | Group-Object Name)) {
        $files = @($g.Group | Select-Object -ExpandProperty File -Unique)
        if ($files.Count -ge 2) { $groups.Add([pscustomobject]@{ By = 'name'; Defs = @($g.Group) }) }
    }
    # by body, names normalised
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
        if ($knownFix) { $why += " (known copy: $knownFix)" }
        Add-Item 'C' $anchor.File $anchor.Line $what $why
    }

    # Fixture spellings outside the shared header that owns them.
    $spellings = @(
        @{ Rx = '''M'',\s*''T'',\s*''(h'',\s*''d|r'',\s*''k)''|"MT(hd|rk)"'; Owner = 'tests/midi_util.h'
           What = 'a MIDI MThd/MTrk chunk written by hand'; Why = 'MIDI file writers belong in tests/midi_util.h' },
        @{ Rx = '&\s*0x7[Ff]\b[^;]*\|\s*0x80|0x80\s*\|\s*\(|\{\s*0x8[1-9A-Fa-f],\s*0x[0-7][0-9A-Fa-f]\s*\}'; Owner = 'tests/midi_util.h'
           What = 'a MIDI variable-length delta encoded by hand'; Why = 'the varlen writer belongs in tests/midi_util.h' },
        @{ Rx = '\btdefl_compress_mem_to_heap\s*\('; Owner = 'tests/srb_util.h'
           What = 'a raw deflate for a .srb fixture'; Why = '.srb fixture writers belong in tests/srb_util.h' },
        @{ Rx = '\[SyncTrack\]\\n|"\[Song\]\\n'; Owner = 'tests/chart_text.h'
           What = 'a minimal .chart header typed by hand'; Why = 'the minimal .chart writer belongs in tests/chart_text.h' },
        @{ Rx = '\{\s*(hydra::)?Difficulty::(Expert|Hard|Medium|Easy)\s*,\s*-?\d'; Owner = 'tests/difficulty_literals.h'
           What = 'a per-difficulty literal table typed in a test'; Why = 'per-difficulty values belong in one pinned table (tests/difficulty_literals.h)' },
        @{ Rx = 'for\s*\(\s*const\s+(hydra::)?Path\s*[&*]\s*\w+\s*:\s*[^)]*\)\s*collect_\w+\s*\(|:\s*[\w.>()-]*\bvariants\s*\)'; Owner = 'tests/record_fixtures.h'
           What = 'a hand-written walk over tied variants'; Why = 'walking tied variants belongs to test::collect_tied / test::all_tied (tests/record_fixtures.h)' },
        @{ Rx = '\bkind\s*[!=]=\s*(hydra::)?SpEndKind::SqIn\b|count_if\([^;]*SqIn'; Owner = 'tests/bank_check.h'
           What = 'an SqIn step test typed by hand'; Why = 'is_sqin_kind (src/core/model.h) and check_one_step_per_sqin (tests/bank_check.h) own it' }
    )
    foreach ($f in $added.Keys) {
        if ($f -notlike 'tests/*' -or -not (Test-CppPath $f) -or $f -eq $scanFile) { continue }
        $v = Get-Views $f
        foreach ($ln in (Get-AddedLines $f $v.NoComments.Count)) {
            $text = $v.NoComments[$ln - 1]
            foreach ($s in $spellings) {
                if ($f -eq $s.Owner) { continue }
                if ($text -cmatch $s.Rx) {
                    $why = $s.Why
                    $fix = Get-KnownCopy $f @($v.Raw[$ln - 1])
                    if ($fix) { $why += " (known copy: $fix)" }
                    Add-Item 'C' $f $ln "$($s.What): $($text.Trim())" $why
                }
            }
        }
    }
}

# --------------------------------------- check 2: recompute spellings (B)

function Invoke-Check2 {
    $cpp = @(
        @{ Rx = '\.ms\(\)\s*-\s*[^;,]*\.ms\(\)|\.ms\(\)\s*-\s*[\w.>]*_ms\b|\b\w*_ms\b\s*-\s*[\w.>()-]*\.ms\(\)|\.ms\s*-\s*[\w.>]+\.\w*_ms\b'
           What = 'an offset worked out from two times'; Why = 'offset_from_sp_end owns how far a note is from an SP end; call it or pin the literal' },
        @{ Rx = '(?<![\w.''])500(\.0*)?(?![\w.''])'; Ctx = '(?i)window|squeez|sq_?in|sq_?out|leeway|sp_end|deact|fabs\('
           What = 'a typed 500 beside a squeeze check'; Why = 'the squeeze window is kSqueezeWindowMs / within_squeeze_window' },
        @{ Rx = '(?<![\w.''])2\.0*(?![\w.''])'; Ctx = '(?i)hit.?window'
           What = 'a typed 2.0 beside a hit-window check'; Why = 'the hit-window rule has one owner; call it or pin the literal' },
        @{ Rx = '[!=]=\s*1\.0*(?![\d.])(?!\s*[-+*/])|(?<![\w.])1\.0*\s*[!=]='; Ctx = '(?i)\b(early|late)\b|transfer'; Same = $true
           What = 'x1.00 decided with exact equality'; Why = 'is_scaled (src/core/squeeze_rating.h) owns "is this scale x1.00"' },
        @{ Rx = 'plusmeasure\s*\([^;]*sp_bars_to_measures\s*\('
           What = 'the plain SP end rebuilt by hand'; Why = 'the graph owns the plain SP end; read nominal_end() or pin the tick' },
        @{ Rx = 'tied_pathcount\(\)\s*[!=]=\s*[^;]*(\d\s*\+|\+\s*\d|\.size\(\)|count\b)'
           What = 'the tied-path count worked out a second way'; Why = 'Path::recount_tied_paths owns the count; pin it' },
        @{ Rx = '\.target_act_ticks\s*=(?!=)'
           What = 'a test runs its own targeted search'; Why = 'search_target owns the targeted search and its filter; call it and pin its answer' },
        @{ Rx = '\b(CHECK|REQUIRE|CHECK_EQ|REQUIRE_EQ|CHECK_FALSE)\s*\([^;]*==\s*[^;]*([\w)\]]\s*\([^()]*\)\s*(?:\+|-(?!>)|\*|/)\s*[A-Za-z_(]|[\w)\]]\s*(?:\+|-(?!>)|\*|/)\s*[\w:<>]+\s*\()'
           What = 'an expected value computed from other calls'; Why = 'a test pins literals from one run; it never computes the expected value' },
        @{ Rx = '\b(want|wanted|expect|expected)\w*\s*=\s*[^;]*[!=]=[^;]*(\|\||&&)'
           What = 'the expected answer built from the predicate''s own comparisons'; Why = 'pin a literal list of inputs and answers instead of restating the rule' }
    )
    $py = @(
        @{ Rx = '[<>]=?\s*[A-Za-z_]\w*\.[A-Z][A-Z0-9_]{2,}\b'
           What = 'a fixture decides with the module''s own constant'; Why = 'the fixture makes the comparison production makes; pin the inputs as literals' },
        @{ Rx = 'assert\w*\(\s*[A-Za-z_]\w*\.[A-Z][A-Z0-9_]{2,}\s*,[^)]*[A-Za-z_]\w*\.[A-Z][A-Z0-9_]{2,}'
           What = 'a module constant checked against a formula of other constants'; Why = 'the test restates the module''s formula; pin the value' }
    )
    foreach ($f in $added.Keys) {
        $isCppTest = $f -like 'tests/*' -and (Test-CppPath $f) -and $f -ne $scanFile
        $isPyTest = (Test-PyPath $f) -and ($f -like 'tests/*' -or $f -match '^tools/.*(/tests/|/test_[^/]*\.py$)')
        if (-not ($isCppTest -or $isPyTest)) { continue }
        $v = Get-Views $f
        $rules = if ($isPyTest) { $py } else { $cpp }
        $reported = [System.Collections.Generic.HashSet[string]]::new()  # rule|line already shown
        foreach ($ln in (Get-AddedLines $f $v.Code.Count)) {
            $code = $v.Code[$ln - 1]
            # A C++ statement split over lines is read whole (up to 4 lines).
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
                    $from = if ($r.Same) { $ln } else { [Math]::Max(1, $ln - 2) }
                    $to = if ($r.Same) { $lastLn } else { [Math]::Min($v.Raw.Count, $lastLn + 2) }
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
function Split-Paras([string]$Text) {
    $Text -split '\n\s*\n|\n(?=\s*(?:\d+\.|[-*])\s)' | Where-Object { $_.Trim() }
}

function Get-DecisionParas {
    $paras = [System.Collections.Generic.List[object]]::new()
    foreach ($rev in (@($tip, $left) | Where-Object { $_ } | Select-Object -Unique)) {
    $docs = Invoke-Git @('ls-tree', '-r', '--name-only', $rev, '--', 'docs/adr', 'CONTEXT.md', 'docs/superpowers/plans', 'docs/audit')
    foreach ($d in $docs) {
        if ($d -notmatch '\.md$') { continue }
        if ($d -like 'docs/adr/*') {
            $rec = if ($d -match '/(\d{4})[^/]*$') { "ADR $([int]$Matches[1])" } else { $d }
            foreach ($p in (Split-Paras (Get-TipText $d $rev))) { $paras.Add((New-Para $rec $p)) }
        } elseif ($d -eq 'CONTEXT.md') {
            foreach ($p in (Split-Paras (Get-TipText $d $rev))) { $paras.Add((New-Para 'CONTEXT.md' $p)) }
        } elseif ($d -match '^docs/superpowers/plans/[^/]+\.md$') {
            $lines = (Get-TipText $d $rev).Split("`n")
            $in = $false; $buf = [System.Collections.Generic.List[string]]::new(); $level = 0
            foreach ($l in $lines) {
                if ($l -match '^\*\*User decisions[^*]*\*\*') { $in = $true; $level = 99; $buf.Add($l); continue }
                if ($l -match '^(#+)\s+Test limits') { $in = $true; $level = $Matches[1].Length; $buf.Add($l); continue }
                if ($in) {
                    $endBold = $level -eq 99 -and ($l -match '^\*\*(?!User decisions)[^*]+\*\*' -or $l -match '^#')
                    $endHead = $level -ne 99 -and $l -match '^(#+)\s' -and $Matches[1].Length -le $level
                    if ($endBold -or $endHead) { $in = $false; foreach ($p in (Split-Paras ($buf -join "`n"))) { $paras.Add((New-Para "plan $d" $p)) }; $buf.Clear() }
                    else { $buf.Add($l) }
                }
            }
            if ($buf.Count) { foreach ($p in (Split-Paras ($buf -join "`n"))) { $paras.Add((New-Para "plan $d" $p)) } }
        } elseif ($d -match '^docs/audit/[^/]*decisions[^/]*\.md$') {
            $rec = $null
            foreach ($p in ((Get-TipText $d $rev) -split '\n\s*\n')) {
                if ($p -match '^\s*\*\*D(\d+)\b') { $rec = "D$([int]$Matches[1])" }
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
    $skip = [System.Collections.Generic.HashSet[string]]::new([string[]]@('0', '1', '2'))
    $unitFactors = [System.Collections.Generic.HashSet[string]]::new([string[]]@('1000', '1024', '1000000'))
    foreach ($f in $added.Keys) {
        if (-not ((Test-CppPath $f) -or (Test-PyPath $f))) { continue }
        if ($f -eq $scanFile -or $selfFiles -contains $f) { continue }
        $isTest = $f -like 'tests/*' -or $f -match '^tools/.*(/tests/|/test_[^/]*\.py$)'
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
            $isCheck = $t -match '\b(CHECK|REQUIRE|CHECK_EQ|REQUIRE_EQ|CHECK_FALSE|CHECK_THROWS\w*|WARN|assert\w*|self\.assert\w*)\s*\('
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
                    $isDuration = $before -match '\b(seconds|milliseconds|minutes|microseconds|sleep_for|timeout|deadline)\w*\s*\(\s*$'
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
                    $isDuration = $before -match '\b(seconds|milliseconds|minutes|microseconds|sleep_for|timeout|deadline)\w*\s*\(\s*$' -or $after -match '^(ms|s|min)\b'
                    if ($isCheck -and -not $inCompare) { continue }
                    if ($isLoop -and -not $isLoopLimit -and -not $inCompare) { continue }
                    if ($isLoop -and $inCompare -and -not $isLoopLimit) { continue }
                    $isSample = $before -match '(begin\(\)|\.data\(\))\s*\+$|\bstd::(min|max)\s*(<[^>]*>)?\s*\([^()]*,$'
                    if (-not ($inCompare -or $isLoopLimit -or $isField -or $isDuration -or $isSample)) { continue }
                }
                # Cited decision on this line or the three above?
                $from = [Math]::Max(1, $ln - 3)
                $near = ($v.Raw[($from - 1)..($ln - 1)]) -join "`n"
                $cites = [System.Collections.Generic.List[string]]::new()
                foreach ($c in [regex]::Matches($near, '\bD(\d{1,3})\b')) { $cites.Add("D$([int]$c.Groups[1].Value)") }
                foreach ($c in [regex]::Matches($near, '\bADR\s*0*(\d{1,4})\b')) { $cites.Add("ADR $([int]$c.Groups[1].Value)") }
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
    foreach ($f in $added.Keys) {
        if (-not ($f -like 'tests/*' -or $f -like 'tools/*')) { continue }
        if ($f -eq $scanFile -or $selfFiles -contains $f) { continue }
        $py = Test-PyPath $f
        if (-not ((Test-CppPath $f) -or $py)) { continue }
        $v = Get-Views $f
        $namesSrc = $v.NcText -match '"src"|''src''|/src\b|\bHYDRA_SOURCE_DIR\b'
        foreach ($ln in (Get-AddedLines $f $v.NoComments.Count)) {
            $t = $v.NoComments[$ln - 1]
            $what = $null
            if ($py) {
                if ($t -match '\b(os\.walk|glob\.glob|\.rglob|\.glob|os\.listdir)\s*\([^)]*src' -or
                    $t -match '\bopen\s*\([^)]*\.(cpp|h)[''"]') { $what = 'reads the source tree' }
            } else {
                if ($t -match '\bHYDRA_SOURCE_DIR\b') { $what = 'uses HYDRA_SOURCE_DIR' }
                elseif ($t -match '\b(recursive_)?directory_iterator\b' -and $namesSrc) { $what = 'walks a directory in a file that names src' }
                elseif ($t -match '\b(ifstream|fopen|_wfopen|open_file\w*)\b[^;]*(\.(cpp|h)"|"[^"]*/?src/)') { $what = 'opens a source file' }
            }
            if ($what) {
                Add-Item 'E' $f $ln "$($what): $($t.Trim())" 'tests/test_single_owner.cpp is the one scan of the source tree; make this a row there'
            }
        }
    }
    # New rule rows in the scan file.
    if (-not $added.ContainsKey($scanFile)) { return }
    $sv = Get-Views $scanFile
    $body = Find-FunctionBody $sv.CodeText 'rules'
    if (-not $body) { return }
    $init = Get-InitList $sv.CodeText $body[0] $body[1]
    if (-not $init) { return }
    foreach ($el in (Get-TopElements $sv.CodeText $init[0] $init[1])) {
        $line = ($sv.CodeText.Substring(0, $el[0]) -split "`n").Count
        $last = ($sv.CodeText.Substring(0, $el[1]) -split "`n").Count
        $isNew = $false
        for ($i = $line; $i -le $last; $i++) { if (Test-Added $scanFile $i) { $isNew = $true; break } }
        if (-not $isNew) { continue }
        $fields = Split-Fields $sv.CodeText $el[0] $el[1]
        $q = if ($fields.Count) { (Get-Strings $sv.NcText.Substring($fields[0][0], $fields[0][1] - $fields[0][0])) -join '' } else { '?' }
        $lists = @{ 7 = 'must-match'; 8 = 'must-not-match' }
        foreach ($k in 7, 8) {
            $n = if ($fields.Count -gt $k) { @(Get-Strings $sv.NcText.Substring($fields[$k][0], $fields[$k][1] - $fields[$k][0])).Count } else { 0 }
            if ($n -eq 0) {
                Add-Item 'E' $scanFile $line "scan row ""$q"" has no $($lists[$k]) examples" 'every scan row ships with lines it must match and must not match, so the self-test proves its pattern'
            }
        }
        if ($fields.Count -gt 2) {
            $pat = (Get-Strings $sv.NcText.Substring($fields[2][0], $fields[2][1] - $fields[2][0])) -join ''
            if ($pat -match '\(\?<?!\(?[A-Za-z_]\w*(\|[A-Za-z_]\w*)*\)?\\s\*\(?\\?\.') {
                Add-Item 'E' $scanFile $line "scan row ""$q"" exempts variable names inside its pattern ($($Matches[0])...)" 'an exemption in the pattern applies in every file; list the allowed lines as owner lines with their reason'
            }
        }
    }
}

# ---------------------------------------------------------------- run

if ($Disable -notcontains 1) { Invoke-Check1 }
if ($Disable -notcontains 2) { Invoke-Check2 }
if ($Disable -notcontains 3) { Invoke-Check3 }
if ($Disable -notcontains 4) { Invoke-Check4 }

$order = @{ C = 0; B = 1; D = 2; E = 3 }
$sorted = $items | Sort-Object @{ e = { $order[$_.Kind] } }, File, Line, What -Unique
foreach ($i in $sorted) {
    "{0} {1}:{2}  {3}  -- {4}" -f $i.Kind, $i.File, $i.Line, $i.What, $i.Why
}
$counts = ($sorted | Group-Object Kind | ForEach-Object { "$($_.Name) $($_.Count)" }) -join ', '
Write-Host ("derive_once_precheck: {0} items ({1}) for {2} at {3}" -f @($sorted).Count, $(if ($counts) { $counts } else { 'none' }), $(if ($WholeTree) { 'the whole tree' } else { $Range }), $tip.Substring(0, 7))
