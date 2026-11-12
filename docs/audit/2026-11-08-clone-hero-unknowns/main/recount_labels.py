import glob, os, re, sys
root = sys.argv[1]
labels = ["Proven in the game's code", "Proven in a live run", "Hydra assumes it, never checked", "Unknown"]
tot = [0] * 6
for p in sorted(glob.glob(os.path.join(root, "docs/clone-hero/0*.md"))):
    text = open(p, encoding="utf-8").read().splitlines()
    heads = sum(1 for l in text if l.startswith("### "))
    labs = [l[len("Label: "):].strip() for l in text if l.startswith("Label: ")]
    counts = [labs.count(x) for x in labels]
    other = len(labs) - sum(counts)
    row = [heads, len(labs)] + counts
    print(os.path.basename(p), "headings", heads, "labels", len(labs), "|", " | ".join(map(str, counts)), "| other", other)
    tot = [a + b for a, b in zip(tot, row)]
print("total headings", tot[0], "labels", tot[1], "|", " | ".join(map(str, tot[2:])))
