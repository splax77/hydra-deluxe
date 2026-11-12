"""Count methods, and methods with a scrambled name, per assembly in dump.cs.

A type belongs to the image whose start TypeDefIndex (the "// Image N:
name - start" lines at the top of dump.cs) is the largest one at or below
its own TypeDefIndex. A method is a line inside a type's "// Methods" block
that ends in "{ }" and holds "(", the shape Il2CppDumper writes. Its name is
the word just before the first "(" (after any generic "<...>" part is cut);
a name with any non-ASCII letter counts as scrambled, the same test IN-46's
type count used. Nested types count in the image of their own TypeDefIndex.

    py -I docs/audit/2026-11-08-clone-hero-unknowns/W10b/count_methods.py <dump.cs> [image ...]
"""
import bisect
import re
import sys

IMAGE = re.compile(r"^// Image \d+: (\S+) - (\d+)$")
TYPE = re.compile(r"// TypeDefIndex: (\d+)\s*$")


def method_name(line):
    head = line.split("(", 1)[0].rstrip()
    head = re.sub(r"<[^<>]*>$", "", head)
    return head.split()[-1] if head.split() else ""


def main(path, wanted):
    starts, names = [], []
    totals = {}
    image = None
    in_methods = False
    last_rva = ""
    with open(path, encoding="utf-8") as f:
        for line in f:
            m = IMAGE.match(line)
            if m:
                names.append(m.group(1))
                starts.append(int(m.group(2)))
                continue
            t = TYPE.search(line)
            if t and not line.startswith("\t"):
                i = bisect.bisect_right(starts, int(t.group(1))) - 1
                image = names[i]
                in_methods = False
                continue
            s = line.strip()
            if s == "// Methods":
                in_methods = True
                continue
            if s.startswith("// ") and s in ("// Fields", "// Properties"):
                in_methods = False
                continue
            if s == "}":
                in_methods = False
                continue
            if s.startswith("// RVA: "):
                last_rva = s
                continue
            if in_methods and image and s.endswith("{ }") and "(" in s:
                name = method_name(s)
                tot = totals.setdefault(image, [0, 0, 0, 0])
                scrambled = any(ord(c) > 127 for c in name)
                tot[0] += 1
                tot[1] += scrambled
                if not last_rva.startswith("// RVA: -1"):
                    tot[2] += 1
                    tot[3] += scrambled
                last_rva = ""
    for img in wanted or sorted(totals):
        tot, scr, tot_rva, scr_rva = totals.get(img, [0, 0, 0, 0])
        print(f"{img}\t{scr} of {tot} methods scrambled;"
              f" {scr_rva} of {tot_rva} among those with an address (RVA line not -1)")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2:])
