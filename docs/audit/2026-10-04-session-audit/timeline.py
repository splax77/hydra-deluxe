"""Split every agent's wall time into non-overlapping states.

Each interval between two consecutive transcript records is charged once:
- tool:        at least one tool call is running
- model-wait:  last record was a tool result / prompt; model hasn't produced anything yet
- model-gen:   last record was an assistant block and no tool is running (streaming more blocks)
- other:       anything else (attachments, hook/system records)
Usage: python timeline.py <session> [...]
"""
import json, os, glob, sys, collections
from datetime import datetime

ROOT = r"C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test"
tot = collections.Counter()
other_kinds = collections.Counter()
big = []
for s in sys.argv[1:]:
    for p in glob.glob(os.path.join(ROOT, s, "**", "agent-*.jsonl"), recursive=True):
        pending = set()
        prev_t = None
        state = "model-wait"
        prev_kind = ""
        for line in open(p, encoding="utf-8", errors="replace"):
            try:
                o = json.loads(line)
            except Exception:
                continue
            t = o.get("timestamp")
            if not t:
                continue
            t = datetime.fromisoformat(t.replace("Z", "+00:00"))
            if prev_t:
                dt = (t - prev_t).total_seconds()
                st = "tool" if pending else state
                if st == "other" and prev_kind.startswith("attachment:total_tokens"):
                    st = "model-wait"
                if st == "model-gen" and o.get("type") == "user":
                    cc = (o.get("message") or {}).get("content")
                    if not (isinstance(cc, list) and any(isinstance(x, dict) and x.get("type") == "tool_result" for x in cc)):
                        st = "idle-resumed"
                tot[st] += dt
                if st == "other":
                    other_kinds[prev_kind] += dt
                if dt > 300:
                    big.append((dt / 60, st, prev_kind, o.get("type"), os.path.basename(p)))
            prev_t = t
            typ = o.get("type")
            m = o.get("message") or {}
            c = m.get("content")
            if typ == "assistant":
                state = "model-gen"
                if isinstance(c, list):
                    for x in c:
                        if isinstance(x, dict) and x.get("type") == "tool_use":
                            pending.add(x.get("id"))
                prev_kind = "assistant"
            elif typ == "user":
                if isinstance(c, list):
                    for x in c:
                        if isinstance(x, dict) and x.get("type") == "tool_result":
                            pending.discard(x.get("tool_use_id"))
                state = "model-wait"
                prev_kind = "user"
            else:
                sub = o.get("subtype") or (o.get("attachment") or {}).get("type") or ""
                prev_kind = f"{typ}:{sub}"
                if not pending:
                    state = "other"
h = sum(tot.values()) / 3600
print(f"total agent wall {h:.1f} h")
for k, v in tot.most_common():
    print(f"  {k:11s} {v/3600:5.1f} h  {100*v/3600/h:4.1f}%")
print("other, by preceding record kind:")
for k, v in other_kinds.most_common(8):
    print(f"  {k:40s} {v/3600:5.2f} h")
big.sort(reverse=True)
print("intervals over 5 min, by state:", collections.Counter(b[1] for b in big))
for b in big[:12]:
    print(f"  {b[0]:6.1f} min {b[1]:10s} after {b[2]:25s} -> {b[3]} {b[4]}")
