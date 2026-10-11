# Phase 1 lead probes for the concept duplication audit (plan:
# docs/superpowers/plans/2026-10-11-concept-duplication-audit.md, phase 1).
# A record of the audit's one run, not a tool. Reads src/ only and prints
# markdown to stdout. Thresholds are D113 item 8.
#
# Run from the repo root:  pwsh -File docs/audit/2026-10-11-concept-audit/leads.ps1

param(
    [int]$MinString = 12,   # probe 1: shortest string literal kept
    [int]$MinFields = 3,    # probe 3: smallest struct considered
    [int]$MinShared = 4,    # probe 3: shared field names that make a pair
    [switch]$Types          # print the enum and struct list (scout C2's input) instead
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path 'src').Path
$files = Get-ChildItem $root -Recurse -Include *.cpp, *.h | Sort-Object FullName

function Rel($p) { $p.Substring($root.Length + 1).Replace('\', '/') }
function TopDir($rel) { $rel.Split('/')[0] }

# Strip // and /* */ comments but keep string literals, so a comment that
# quotes a string does not count as a use.
function Strip-Comments([string]$text) {
    $sb = [System.Text.StringBuilder]::new()
    $i = 0; $n = $text.Length
    while ($i -lt $n) {
        $c = $text[$i]
        if ($c -eq '"' -or $c -eq "'") {
            $q = $c; [void]$sb.Append($c); $i++
            while ($i -lt $n -and $text[$i] -ne $q) {
                if ($text[$i] -eq '\' -and $i + 1 -lt $n) { [void]$sb.Append($text[$i]); $i++ }
                [void]$sb.Append($text[$i]); $i++
            }
            if ($i -lt $n) { [void]$sb.Append($text[$i]); $i++ }
        } elseif ($c -eq '/' -and $i + 1 -lt $n -and $text[$i + 1] -eq '/') {
            while ($i -lt $n -and $text[$i] -ne "`n") { $i++ }
        } elseif ($c -eq '/' -and $i + 1 -lt $n -and $text[$i + 1] -eq '*') {
            $i += 2
            while ($i + 1 -lt $n -and -not ($text[$i] -eq '*' -and $text[$i + 1] -eq '/')) {
                if ($text[$i] -eq "`n") { [void]$sb.Append("`n") }
                $i++
            }
            $i += 2
        } else { [void]$sb.Append($c); $i++ }
    }
    $sb.ToString()
}

$src = @{}
foreach ($f in $files) { $src[(Rel $f.FullName)] = Strip-Comments ([IO.File]::ReadAllText($f.FullName)) }

# ---------- probe 1: repeated string literals ----------
$strFiles = @{}
foreach ($k in $src.Keys) {
    $lines = $src[$k] -split "`n" | Where-Object { $_ -notmatch '^\s*#\s*include' }
    $body = $lines -join "`n"
    foreach ($m in [regex]::Matches($body, '(?<![A-Za-z0-9_])(?:u8|L)?"((?:[^"\\\n]|\\.)*)"')) {
        $s = $m.Groups[1].Value
        if ($s.Length -lt $MinString) { continue }
        if (-not $strFiles.ContainsKey($s)) { $strFiles[$s] = [System.Collections.Generic.HashSet[string]]::new() }
        [void]$strFiles[$s].Add($k)
    }
}
# The same probe without stripping comments, so a comment that quotes a
# string counts as a use. Only its counts are printed, to compare with the
# planner's one-off run.
$rawFiles = @{}
foreach ($f in $files) {
    $k = Rel $f.FullName
    foreach ($m in [regex]::Matches([IO.File]::ReadAllText($f.FullName), '"([^"\n]{' + $MinString + ',}?)"')) {
        $s = $m.Groups[1].Value
        if ($s -match '\.(h|hpp)$') { continue }
        if (-not $rawFiles.ContainsKey($s)) { $rawFiles[$s] = [System.Collections.Generic.HashSet[string]]::new() }
        [void]$rawFiles[$s].Add($k)
    }
}
$rawMulti = @($rawFiles.GetEnumerator() | Where-Object { $_.Value.Count -ge 2 })
$rawCross = @($rawMulti | Where-Object { @($_.Value | ForEach-Object { TopDir $_ } | Sort-Object -Unique).Count -ge 2 }).Count

$p1 = $strFiles.GetEnumerator() | Where-Object { $_.Value.Count -ge 2 } | ForEach-Object {
    $dirs = @($_.Value | ForEach-Object { TopDir $_ } | Sort-Object -Unique)
    [pscustomobject]@{ Text = $_.Key; Files = @($_.Value | Sort-Object); Cross = ($dirs.Count -ge 2) }
} | Sort-Object @{ e = { -$_.Files.Count } }, Text

# ---------- probe 2: free function names defined in two or more .cpp files ----------
# A definition is a line at column 0 that ends in an opening parameter list
# and is followed (same or a later line) by '{' before any ';'. Member
# definitions (Name::f) and keywords are left out.
$skip = 'if','for','while','switch','return','sizeof','catch','defined','TEST_CASE','SUBCASE','main'
$fnFiles = @{}
foreach ($k in ($src.Keys | Where-Object { $_ -like '*.cpp' })) {
    $text = $src[$k]
    foreach ($m in [regex]::Matches($text, '(?m)^(?:static\s+|inline\s+|constexpr\s+|\[\[nodiscard\]\]\s+)*[A-Za-z_][\w:<>,\s\*&]*?[\s\*&]([A-Za-z_]\w*)\s*\(([^;{}]*?)\)\s*(?:const\s*)?(?:noexcept\s*)?(?:->\s*[\w:<>,\s\*&]+)?\s*\{')) {
        $name = $m.Groups[1].Value
        $before = $m.Value.Substring(0, $m.Value.IndexOf($name))
        if ($skip -contains $name) { continue }
        if ($before -match '::\s*$') { continue }
        if ($before -match '^\s*(return|else|case|typedef|using|struct|class|enum)\b') { continue }
        if (-not $fnFiles.ContainsKey($name)) { $fnFiles[$name] = [System.Collections.Generic.HashSet[string]]::new() }
        [void]$fnFiles[$name].Add($k)
    }
}
$p2 = $fnFiles.GetEnumerator() | Where-Object { $_.Value.Count -ge 2 } | ForEach-Object {
    [pscustomobject]@{ Name = $_.Key; Files = @($_.Value | Sort-Object) }
} | Sort-Object @{ e = { -$_.Files.Count } }, Name

# ---------- structs and enums (probes 3 and 5) ----------
$structs = [System.Collections.Generic.List[object]]::new()
$typeDefs = @{}
$enumCount = 0
foreach ($k in $src.Keys) {
    $text = $src[$k]
    foreach ($m in [regex]::Matches($text, '(?m)\b(struct|class|enum\s+class|enum\s+struct|enum)\s+(?:\[\[[^\]]*\]\]\s*)?([A-Za-z_]\w*)\s*(?:final\s*)?(:(?!:)[^;{]*)?([;{])')) {
        $kind = ($m.Groups[1].Value -replace '\s+', ' ')
        $name = $m.Groups[2].Value
        $isDef = $m.Groups[4].Value -eq '{'
        if ($kind -like 'enum*' -and $isDef) { $enumCount++ }
        if (-not $typeDefs.ContainsKey($name)) { $typeDefs[$name] = [System.Collections.Generic.List[object]]::new() }
        $typeDefs[$name].Add([pscustomobject]@{ File = $k; Kind = $kind; Def = $isDef })
        if (-not $isDef -or $kind -like 'enum*') { continue }
        # Walk the body at depth 1 and collect data member names.
        $start = $m.Index + $m.Length
        $depth = 1; $i = $start
        while ($i -lt $text.Length -and $depth -gt 0) {
            if ($text[$i] -eq '{') { $depth++ } elseif ($text[$i] -eq '}') { $depth-- }
            $i++
        }
        $body = $text.Substring($start, [Math]::Max(0, $i - 1 - $start))
        # Flatten nested braces so only depth-1 statements remain.
        $flat = [System.Text.StringBuilder]::new(); $d = 0
        foreach ($ch in $body.ToCharArray()) {
            if ($ch -eq '{') { $d++; if ($d -eq 1) { [void]$flat.Append('{') }; continue }
            if ($ch -eq '}') { $d--; if ($d -eq 0) { [void]$flat.Append('}') }; continue }
            if ($d -eq 0) { [void]$flat.Append($ch) }
        }
        $fields = [System.Collections.Generic.HashSet[string]]::new()
        foreach ($stmt in ($flat.ToString() -split ';')) {
            $s = ($stmt -replace '(?m)^\s*(public|private|protected)\s*:', '').Trim()
            if ($s -eq '' -or $s -match '^(using|friend|static|typedef|template|enum|struct|class)\b') { continue }
            if ($s -match '\)\s*(const)?\s*(override|noexcept|=\s*(default|delete|0))?\s*(\{\})?\s*$' -or $s -match '\(') {
                # a method, or a field with a call initialiser such as std::atomic<int> n(0)
                if ($s -notmatch '^[\w:<>,\s\*&]+\s+([A-Za-z_]\w*)\s*\{\}\s*$') { continue }
            }
            $s = ($s -split '=')[0].Trim()
            $s = $s -replace '\{\}$', '' -replace '\[[^\]]*\]$', ''
            if ($s -match '\s' -and $s -match '([A-Za-z_]\w*)\s*$') { [void]$fields.Add($Matches[1]) }
        }
        if ($fields.Count -ge $MinFields) {
            $structs.Add([pscustomobject]@{ Name = $name; File = $k; Fields = $fields })
        }
    }
}

if ($Types) {
    "# Types in src/ (input for inventory scout C2)"
    ""
    "Generated by ``leads.ps1 -Types``. Enum definitions, then structs and classes with $MinFields+ data members."
    ""
    "## Enums"
    ""
    foreach ($e in ($typeDefs.GetEnumerator() | Sort-Object Key)) {
        foreach ($d in ($e.Value | Where-Object { $_.Def -and $_.Kind -like 'enum*' })) { "- ``$($e.Key)``: $($d.File)" }
    }
    ""
    "## Structs and classes"
    ""
    foreach ($s in ($structs | Sort-Object File, Name)) { "- ``$($s.Name)`` ($($s.File)): $(($s.Fields | Sort-Object) -join ', ')" }
    return
}

# ---------- probe 3: struct pairs sharing field names ----------
$p3 = [System.Collections.Generic.List[object]]::new()
for ($a = 0; $a -lt $structs.Count; $a++) {
    for ($b = $a + 1; $b -lt $structs.Count; $b++) {
        $x = $structs[$a]; $y = $structs[$b]
        if ($x.File -eq $y.File) { continue }
        $shared = @($x.Fields | Where-Object { $y.Fields.Contains($_) })
        if ($shared.Count -ge $MinShared) {
            $p3.Add([pscustomobject]@{ A = "$($x.Name) ($($x.File))"; B = "$($y.Name) ($($y.File))"; Count = $shared.Count; Shared = ($shared | Sort-Object) -join ', ' })
        }
    }
}
$p3 = $p3 | Sort-Object @{ e = { -$_.Count } }, A

# ---------- probe 4: RecordStore methods called from two or more files ----------
$hdr = $src['store/record_store.h']
$cls = [regex]::Match($hdr, '(?s)class RecordStore\s*\{(.*?)\n\};')
# Split at the class's own private: (column 0), not a nested class's.
$methods = @([regex]::Matches(($cls.Groups[1].Value -split '(?m)^private:')[0],'(?m)^\s+(?:static\s+|\[\[nodiscard\]\]\s+)*[\w:<>,\s\*&]+?\s+([a-z_]\w*)\s*\(') |
    ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique)
$p4 = foreach ($meth in $methods) {
    $callers = @($src.Keys | Where-Object { $_ -notlike 'store/record_store*' -and $src[$_] -match "\bstore\w*(\(\))?(\.|->)$meth\s*\(" } | Sort-Object)
    if ($callers.Count -ge 2) { [pscustomobject]@{ Method = $meth; Files = $callers } }
}

# ---------- probe 5: a struct, class or enum name defined twice ----------
$p5 = $typeDefs.GetEnumerator() | ForEach-Object {
    $defs = @($_.Value | Where-Object { $_.Def })
    $all = @($_.Value | Select-Object -ExpandProperty File -Unique)
    # Two definitions, or a definition plus an enum declaration elsewhere
    # (an opaque enum re-declared in another header).
    $enumDecl = @($_.Value | Where-Object { -not $_.Def -and $_.Kind -like 'enum*' }).Count
    $defFiles = @($defs | Select-Object -ExpandProperty File -Unique)
    if ($defFiles.Count -ge 2 -or ($defs.Count -ge 1-and $all.Count -ge 2 -and $enumDecl -ge 1)) {
        [pscustomobject]@{ Name = $_.Key; Where = (($_.Value | ForEach-Object { "$($_.File) ($($_.Kind)$(if (-not $_.Def) { ', declaration' }))" } | Sort-Object -Unique) -join '; ') }
    }
} | Sort-Object Name

# ---------- output ----------
$crossCount = @($p1 | Where-Object Cross).Count
"# Phase 1 leads"
""
"Generated by ``leads.ps1`` over ``src/`` ($($files.Count) files). Thresholds: strings of $MinString+ characters, structs of $MinFields+ fields, $MinShared+ shared field names."
""
"| Probe | Count |"
"|---|---|"
"| 1. Repeated string literals, comments stripped | $(@($p1).Count) ($crossCount cross a top-level folder) |"
"| 1b. The same, comments counted (count only) | $($rawMulti.Count) ($rawCross cross a top-level folder) |"
"| 2. Free function names defined in 2+ .cpp files | $(@($p2).Count) |"
"| 3. Struct pairs sharing $MinShared+ field names | $(@($p3).Count) (of $($structs.Count) structs with $MinFields+ fields) |"
"| 4. RecordStore methods called from 2+ files | $(@($p4).Count) |"
"| 5. Type names defined twice | $(@($p5).Count) ($enumCount enum definitions in src/) |"
""
"## Probe 1: repeated strings"
""
foreach ($r in $p1) { "- $(if ($r.Cross) { '**cross** ' })``$($r.Text)``: $($r.Files -join ', ')" }
""
"## Probe 1b: strings repeated only once comments count"
""
"A comment quoting a displayed string is a second place that names it."
""
$inP1 = @{}; foreach ($r in $p1) { $inP1[$r.Text] = $true }
foreach ($r in ($rawMulti | Where-Object { -not $inP1.ContainsKey($_.Key) } | Sort-Object @{ e = { -$_.Value.Count } }, Key)) {
    $fl = @($r.Value | Sort-Object)
    $cross = @($fl | ForEach-Object { TopDir $_ } | Sort-Object -Unique).Count -ge 2
    "- $(if ($cross) { '**cross** ' })``$($r.Key)``: $($fl -join ', ')"
}
""
"## Probe 2: function names"
""
foreach ($r in $p2) { "- ``$($r.Name)``: $($r.Files -join ', ')" }
""
"## Probe 3: struct pairs"
""
foreach ($r in $p3) { "- $($r.Count) shared: $($r.A) and $($r.B): $($r.Shared)" }
""
"## Probe 4: store methods"
""
foreach ($r in $p4) { "- ``$($r.Method)``: $($r.Files -join ', ')" }
""
"## Probe 5: type names"
""
foreach ($r in $p5) { "- ``$($r.Name)``: $($r.Where)" }
