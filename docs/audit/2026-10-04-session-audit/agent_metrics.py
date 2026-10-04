"""Per-agent timing metrics from Claude Code transcripts.

Usage: python agent_metrics.py <out_dir> <session_id> [<session_id> ...]
Writes agents.csv (one row per transcript), commands.csv (every shell/tool call
with its duration), gaps.csv (silences > 3 min inside an agent).
"""
import csv, json, os, re, sys, glob
from datetime import datetime

ROOT = r"C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test"


def ts(s):
    return datetime.fromisoformat(s.replace("Z", "+00:00")) if s else None


def classify(tool, inp):
    if tool not in ("Bash", "PowerShell"):
        return tool
    c = (inp.get("command") or "").lower()
    if "status_append" in c:
        # Agents often chain real work onto the status write; classify the rest.
        rest = " ".join(seg for seg in re.split(r";|&&|\n", c) if "status_append" not in seg).strip()
        if len(rest) < 8:
            return "status"
        c = rest
    if re.search(r"cmake\s+--build|msbuild|ninja|build_cpp|cmake\s+-s|cmake --preset", c):
        return "build"
    if re.search(r"hydra_uitest", c):
        return "uitest"
    if re.search(r"hydra_tests|ctest|test_h\w*\.ps1|doctest|-tc=|-sf=", c):
        return "test"
    if re.search(r"selfcheck|hydra_cli|hydra_replay|s2-scores|scores\.ps1|library|corpus|compare", c):
        return "run/compare"
    if re.search(r"^\s*(git|& ?\"?c:\\program files\\git)", c) or " git " in c[:40]:
        return "git"
    if re.search(r"start-sleep|sleep |wait-process|get-process|tasklist", c):
        return "wait/poll"
    if re.search(r"select-string|grep|rg |get-content|cat |sed -n|head |tail ", c):
        return "read/search"
    return "shell-other"


def files_for(session):
    base = os.path.join(ROOT, session)
    out = [(os.path.join(ROOT, session + ".jsonl"), "MAIN")]
    for p in glob.glob(os.path.join(base, "**", "*.jsonl"), recursive=True):
        if os.path.basename(p) == "journal.jsonl":
            continue
        out.append((p, os.path.relpath(p, base)))
    return out


def analyse(path, rel, session, agents_w, cmds_w, gaps_w):
    meta = {}
    mp = path[:-6] + ".meta.json"
    if os.path.exists(mp):
        try:
            meta = json.load(open(mp, encoding="utf-8"))
        except Exception:
            pass
    pending = {}  # tool_use_id -> (start, tool, input)
    first = last = None
    models = {}
    tool_time = {}
    tool_count = {}
    usage = {"in": 0, "out": 0, "cache_read": 0, "cache_create": 0}
    last_ts = None
    denials = 0
    first_prompt = ""
    model_time = 0.0
    prev_result_ts = None
    for line in open(path, encoding="utf-8", errors="replace"):
        try:
            o = json.loads(line)
        except Exception:
            continue
        t = ts(o.get("timestamp"))
        if not t:
            continue
        first = first or t
        last = t
        if last_ts and (t - last_ts).total_seconds() > 180 and rel != "MAIN":
            gaps_w.writerow([session, rel, meta.get("description", ""), last_ts.isoformat(),
                             round((t - last_ts).total_seconds() / 60, 1), o.get("type")])
        last_ts = t
        m = o.get("message") or {}
        c = m.get("content")
        if o.get("type") == "user" and not first_prompt:
            if isinstance(c, str):
                first_prompt = c
            elif isinstance(c, list):
                first_prompt = " ".join(x.get("text", "") for x in c if isinstance(x, dict))
        if o.get("type") == "assistant":
            if m.get("model"):
                models[m["model"]] = models.get(m["model"], 0) + 1
            u = m.get("usage") or {}
            usage["in"] += u.get("input_tokens", 0) or 0
            usage["out"] += u.get("output_tokens", 0) or 0
            usage["cache_read"] += u.get("cache_read_input_tokens", 0) or 0
            usage["cache_create"] += u.get("cache_creation_input_tokens", 0) or 0
            if prev_result_ts:
                model_time += max(0.0, (t - prev_result_ts).total_seconds())
                prev_result_ts = None
            if isinstance(c, list):
                for x in c:
                    if isinstance(x, dict) and x.get("type") == "tool_use":
                        pending[x.get("id")] = (t, x.get("name"), x.get("input") or {})
        elif o.get("type") == "user" and isinstance(c, list):
            for x in c:
                if isinstance(x, dict) and x.get("type") == "tool_result":
                    prev_result_ts = t
                    st = pending.pop(x.get("tool_use_id"), None)
                    txt = x.get("content")
                    if isinstance(txt, list):
                        txt = " ".join(y.get("text", "") for y in txt if isinstance(y, dict))
                    txt = str(txt or "")
                    if re.search(r"denied|hook .*block|PreToolUse.*(deny|block)", txt[:400], re.I):
                        denials += 1
                    if not st:
                        continue
                    t0, name, inp = st
                    dur = (t - t0).total_seconds()
                    cat = classify(name, inp)
                    tool_time[cat] = tool_time.get(cat, 0) + dur
                    tool_count[cat] = tool_count.get(cat, 0) + 1
                    desc = inp.get("command") or inp.get("file_path") or inp.get("pattern") or inp.get("description") or ""
                    cmds_w.writerow([session, rel, meta.get("description", ""), t0.isoformat(), name, cat,
                                     round(dur, 1), re.sub(r"\s+", " ", str(desc))[:300],
                                     re.sub(r"\s+", " ", txt[:160])])
    if not first:
        return
    wall = (last - first).total_seconds() / 60
    agents_w.writerow([
        session, rel, meta.get("agentType", ""), meta.get("description", ""), meta.get("workflowPhase", ""),
        ";".join(f"{k}:{v}" for k, v in models.items()), first.isoformat(), last.isoformat(), round(wall, 1),
        round(model_time / 60, 1), sum(tool_count.values()), denials,
        ";".join(f"{k}={tool_count[k]}/{round(tool_time[k]/60,1)}m" for k in sorted(tool_time, key=lambda k: -tool_time[k])),
        usage["in"], usage["out"], usage["cache_read"], usage["cache_create"],
        re.sub(r"\s+", " ", first_prompt)[:400],
    ])


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, "agents.csv"), "w", newline="", encoding="utf-8") as a, \
         open(os.path.join(out, "commands.csv"), "w", newline="", encoding="utf-8") as c, \
         open(os.path.join(out, "gaps.csv"), "w", newline="", encoding="utf-8") as g:
        aw, cw, gw = csv.writer(a), csv.writer(c), csv.writer(g)
        aw.writerow(["session", "file", "agent_type", "description", "phase", "models", "start", "end", "wall_min",
                     "model_min", "tool_calls", "denials", "tool_time_by_category", "tok_in", "tok_out",
                     "tok_cache_read", "tok_cache_create", "first_prompt"])
        cw.writerow(["session", "file", "description", "start", "tool", "category", "seconds", "input", "result_head"])
        gw.writerow(["session", "file", "description", "gap_start", "gap_min", "next_record_type"])
        for s in sys.argv[2:]:
            for p, rel in files_for(s):
                analyse(p, rel, s, aw, cw, gw)


main()
