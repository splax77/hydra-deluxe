#!/bin/sh
# Lists every method RVA in dump.cs lines 72905 to 73100 (PlayerSelectionMenu ends at 73053)
# with chdis, then prints each write or compare of the page field +0x120 and the argument
# set before each call of the two page setters 0x330820 and 0x330350.
# Run from the worktree root: sh docs/audit/2026-11-08-clone-hero-unknowns/W9b/psm_page_values.sh <out dir>
DUMP=C:/Users/Patrick/Downloads/Hydra/hydra-data/ch-dumps/6142/dump.cs
OUT=${1:?give an output folder}
mkdir -p "$OUT"
awk 'NR>=72905 && NR<=73100 && /RVA: 0x3[0-9A-F]+ /{print $3}' "$DUMP" | sort -u > "$OUT/rvas.txt"
for r in $(cat "$OUT/rvas.txt"); do
  h=${r#0x}
  C:/Python314/python.exe -m tools.ch_mcp.chdis "$h" > "$OUT/$h.asm" 2>&1
done
echo "== writes and compares of +0x120"
grep -n "+ 0x120\], [0-9a-z]" "$OUT"/*.asm
echo "== arguments before the page setters"
grep -n -B5 "0x180330820\|0x180330350" "$OUT"/*.asm | grep "edx\|dl,\| 0x180330820\| 0x180330350"
