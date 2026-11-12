"""IN-50: is the 1.1 drum engine name absent from 1.0, and how many GameManager
field names do the two builds share?

Usage: python -I in50_compare.py <ch-dumps folder>

Reads 6142's dump.cs (the GameManager class's field lines, and the drum engine
class named in 6142's facts.json signatures) and 4045's facts.json (every
class, field and method name, and the raw file text). Prints to stdout as UTF-8.
"""
import io
import json
import os
import re
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8")
root = sys.argv[1]
dump = open(os.path.join(root, "6142", "dump.cs"), encoding="utf-8").read().splitlines()
f6 = json.load(open(os.path.join(root, "6142", "facts.json"), encoding="utf-8"))
raw4 = open(os.path.join(root, "4045", "facts.json"), encoding="utf-8").read()
f4 = json.loads(raw4)

# 1. The drum engine's 1.1 name.
eng = f6["signatures"]["drums_engine"]
name = eng["name"]
print(f"6142 drums_engine signature: name {name!r}, keys {sorted(eng)}")
names4 = set()
for c in f4["classes"]:
    names4.add(c["name"])
    for f in c.get("fields", []):
        names4.add(f["name"] if isinstance(f, dict) else str(f))
    for m in c.get("methods", []):
        names4.add(m["name"] if isinstance(m, dict) else str(m))
print(f"4045 facts.json: {len(f4['classes'])} classes, {len(names4)} distinct class/field/method names")
print(f"engine name as a 4045 class, field or method name: {name in names4}")
print(f"engine name anywhere in 4045 facts.json text: {name in raw4}")
# The JSON may hold the name with \\u escapes; check that spelling too.
esc = json.dumps(name)[1:-1]
print(f"engine name in its JSON-escaped spelling anywhere in 4045 facts.json: {esc in raw4}")

# 2. GameManager fields. 6142 from dump.cs, between the class line and "// Methods".
start = next(i for i, l in enumerate(dump) if l.startswith("public class GameManager : MonoBehaviour"))
fields6 = []
i = start + 1
while not dump[i].strip().startswith("// Methods"):
    m = re.match(r"\s*(?:\[.*\]\s*)?(?:public|private|protected|internal)[^;=(]*?\s([^\s;=]+)\s*(?:=[^;]*)?;\s*// 0x", dump[i])
    if m:
        fields6.append(m.group(1))
    i += 1
print(f"6142 GameManager: dump.cs line {start + 1}, {len(fields6)} fields")
gm4 = [c for c in f4["classes"] if c["name"] == "GameManager"]
print(f"4045 GameManager classes: {len(gm4)}")
fields4 = [f["name"] if isinstance(f, dict) else str(f) for f in gm4[0]["fields"]]
print(f"4045 GameManager: {len(fields4)} fields")
shared = [f for f in fields6 if f in set(fields4)]
print(f"field names in both: {len(shared)}")
for f in shared:
    print(f"  {f}")
