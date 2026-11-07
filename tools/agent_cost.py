"""Report what agents and main sessions cost, read from Claude Code's transcripts.

Transcripts are only read. Each request is priced by the model that served it, so one
agent's cost is the sum of its requests, and each request counts once however many
records repeat it.

    py -I tools/agent_cost.py [--root <project folder>] [--session <id> ...]
                              [--agent <id>] [--since <date or date-time>] [--out <csv>]

The report has one row per agent and a summary per agent type and effort. With --session
it also gives each session's own main-transcript cost and the total spend, which counts a
request once even when a resumed session copied it into a second transcript.

--since keeps an agent by its first timestamp, because a row is a whole agent. A main
session runs for days, so its cost counts only the requests from that moment on. A time
with no offset is UTC, as the transcripts are.

Exit codes: 0 when every request was priced, 1 when a model has no price (the totals are
then too low), 2 on a bad argument or a session with no transcript.
"""

import argparse
import collections
import csv
import datetime
import json
import pathlib
import statistics
import sys

DEFAULT_ROOT = pathlib.Path.home() / ".claude" / "projects" / "C--Users-Patrick-Downloads-Hydra-hydra-test"

# Every price lives here. Source: the `claude-api` skill. Dollars per million tokens, as
# (input, output, cache read). Haiku's cache read rate is the skill's general 0.1 times
# input, not a listed figure.
PRICES = {
    "claude-fable-5-1": (10.0, 50.0, 0.25),
    "claude-opus-5-5": (4.0, 20.0, 0.20),
    "claude-sonnet-5-5": (2.0, 10.0, 0.20),
    "claude-haiku-5-5": (0.10, 0.50, 0.01),
}
# A model listed here changes price above this many tokens of context.
LONG_CONTEXT_PRICES = {
    "claude-haiku-5-5": (100_000, (0.50, 2.50, 0.05)),
}
CACHE_WRITE_5M_TIMES_INPUT = 1.25
CACHE_WRITE_1H_TIMES_INPUT = 2.0

# The model name on a message Claude Code wrote itself. No API request lies behind it.
NOT_A_REQUEST = "<synthetic>"
NO_EFFORT = "none"
CSV_COLUMNS = ["agent_id", "session", "type", "model", "effort", "cost", "calls",
               "output_tokens", "first_timestamp"]


def parse_time(text):
    moment = datetime.datetime.fromisoformat(text)
    return moment if moment.tzinfo else moment.replace(tzinfo=datetime.timezone.utc)


def request_cost(model, usage):
    """Dollars for one request. The model must be in PRICES."""
    fresh = usage.get("input_tokens", 0)
    read = usage.get("cache_read_input_tokens", 0)
    written = usage.get("cache_creation_input_tokens", 0)
    rates = PRICES[model]
    if model in LONG_CONTEXT_PRICES:
        limit, long_rates = LONG_CONTEXT_PRICES[model]
        if fresh + read + written > limit:
            rates = long_rates
    input_rate, output_rate, read_rate = rates

    split = usage.get("cache_creation")
    if split is None:
        write_units = written * CACHE_WRITE_5M_TIMES_INPUT
    else:
        write_units = (split.get("ephemeral_5m_input_tokens", 0) * CACHE_WRITE_5M_TIMES_INPUT
                       + split.get("ephemeral_1h_input_tokens", 0) * CACHE_WRITE_1H_TIMES_INPUT)
    return (fresh * input_rate + usage.get("output_tokens", 0) * output_rate
            + read * read_rate + write_units * input_rate) / 1_000_000


class Transcript:
    """What one .jsonl file holds. requests maps a requestId to its (model, last usage)."""

    def __init__(self):
        self.requests = {}
        self.efforts = set()
        self.calls = 0
        self.first_timestamp = None
        self.bad_lines = 0
        self.no_request_id = 0

    def models(self):
        return sorted({model for model, _ in self.requests.values()})

    def cost(self):
        return sum(request_cost(model, usage) for model, usage in self.requests.values()
                   if model in PRICES)

    def output_tokens(self):
        return sum(usage.get("output_tokens", 0) for _, usage in self.requests.values())

    def effort_label(self):
        return "+".join(sorted(self.efforts)) or NO_EFFORT


def read_transcript(path, since=None):
    """Read one transcript, leaving out every record older than since."""
    found = Transcript()
    with open(path, encoding="utf-8", errors="replace") as lines:
        for line in lines:
            try:
                record = json.loads(line)
            except json.JSONDecodeError:
                found.bad_lines += 1
                continue
            stamp = record.get("timestamp")
            if since is not None and (stamp is None or parse_time(stamp) < since):
                continue
            if found.first_timestamp is None:
                found.first_timestamp = stamp
            if record.get("type") == "assistant":
                read_assistant_record(record, found)
    return found


def read_assistant_record(record, found):
    message = record.get("message") or {}
    model = message.get("model")
    if model == NOT_A_REQUEST:
        return
    found.efforts.add(record.get("effort") or NO_EFFORT)
    content = message.get("content")
    if isinstance(content, list):
        found.calls += sum(1 for block in content
                           if isinstance(block, dict) and block.get("type") == "tool_use")
    usage = message.get("usage")
    if not usage:
        return
    request_id = record.get("requestId")
    if request_id:
        found.requests[request_id] = (model, usage)
    else:
        found.no_request_id += 1


def agent_type(transcript_path):
    meta = transcript_path.with_name(transcript_path.name[:-len(".jsonl")] + ".meta.json")
    try:
        return json.loads(meta.read_text(encoding="utf-8")).get("agentType") or "unknown"
    except (OSError, ValueError):
        return "unknown"


def agent_paths(root, sessions):
    """Every agent transcript of these sessions, or of the whole project when none are named."""
    folders = [root / session for session in sessions] if sessions else sorted(root.iterdir())
    for folder in folders:
        yield from sorted((folder / "subagents").rglob("agent-*.jsonl"))


def agent_row(path, root, found):
    return {
        "agent_id": path.stem[len("agent-"):],
        "session": path.relative_to(root).parts[0],
        "type": agent_type(path),
        "model": "+".join(found.models()),
        "effort": found.effort_label(),
        "cost": found.cost(),
        "calls": found.calls,
        "output_tokens": found.output_tokens(),
        "first_timestamp": found.first_timestamp or "",
    }


def read_agents(root, sessions, agent, since):
    """The agents the filters keep, as (row, transcript) pairs."""
    kept = []
    for path in agent_paths(root, sessions):
        if agent and path.stem != "agent-" + agent:
            continue
        found = read_transcript(path)
        if since is not None and (found.first_timestamp is None
                                  or parse_time(found.first_timestamp) < since):
            continue
        kept.append((agent_row(path, root, found), found))
    return kept


def print_agent_rows(rows):
    print("%-20s %-16s %-30s %-12s %9s %6s %8s  %s"
          % ("agent", "type", "model", "effort", "cost", "calls", "output", "first timestamp"))
    for row in rows:
        print("%-20s %-16s %-30s %-12s %9.2f %6d %8d  %s"
              % (row["agent_id"], row["type"], row["model"], row["effort"], row["cost"],
                 row["calls"], row["output_tokens"], row["first_timestamp"]))


def print_summary(rows):
    groups = collections.defaultdict(list)
    for row in rows:
        groups[(row["type"], row["effort"])].append(row)
    print()
    print("%-16s %-12s %6s %9s %9s %9s %9s %10s"
          % ("type", "effort", "agents", "median", "mean", "max", "med calls", "total"))
    for (kind, effort), group in sorted(groups.items()):
        costs = [row["cost"] for row in group]
        print("%-16s %-12s %6d %9.2f %9.2f %9.2f %9.1f %10.2f"
              % (kind, effort, len(group), statistics.median(costs), statistics.mean(costs),
                 max(costs), statistics.median(row["calls"] for row in group), sum(costs)))


def write_csv(path, rows):
    with open(path, "w", newline="", encoding="utf-8") as out:
        writer = csv.DictWriter(out, fieldnames=CSV_COLUMNS)
        writer.writeheader()
        for row in rows:
            writer.writerow(dict(row, cost="%.4f" % row["cost"]))


def print_main_sessions(mains):
    print()
    for session, found in mains:
        print("Main session %s: $%.2f, %d calls, effort %s, model %s"
              % (session, found.cost(), found.calls, found.effort_label(),
                 "+".join(found.models()) or "none"))


def parse_args(argv):
    parser = argparse.ArgumentParser(description="Report agent and main-session cost from transcripts.")
    parser.add_argument("--root", type=pathlib.Path, default=DEFAULT_ROOT,
                        help="the project's transcript folder")
    parser.add_argument("--session", nargs="+", action="extend", default=[], metavar="ID",
                        help="report only these sessions, and add each one's main-session cost")
    parser.add_argument("--agent", metavar="ID", help="report only this agent")
    parser.add_argument("--since", type=parse_time, metavar="WHEN",
                        help="a date or date-time; the module docstring says how it applies")
    parser.add_argument("--out", type=pathlib.Path, metavar="CSV", help="also write the agent rows here")
    args = parser.parse_args(argv)
    if not args.root.is_dir():
        parser.error("no transcript folder at %s" % args.root)
    for session in args.session:
        if not (args.root / session).is_dir() and not (args.root / (session + ".jsonl")).is_file():
            parser.error("no transcript for session %s under %s" % (session, args.root))
    return args


def main(argv=None):
    args = parse_args(argv)
    kept = read_agents(args.root, args.session, args.agent, args.since)
    rows = [row for row, _ in kept]
    transcripts = [found for _, found in kept]

    mains = []
    if args.session and not args.agent:
        for session in dict.fromkeys(args.session):
            path = args.root / (session + ".jsonl")
            mains.append((session, read_transcript(path, args.since) if path.is_file() else Transcript()))
        transcripts += [found for _, found in mains]

    print_agent_rows(rows)
    print_summary(rows)
    if args.out:
        write_csv(args.out, rows)

    every_request = {}
    for found in transcripts:
        every_request.update(found.requests)
    if mains:
        print_main_sessions(mains)
        spend = sum(request_cost(model, usage) for model, usage in every_request.values()
                    if model in PRICES)
        print("Total spend, agents and main sessions: $%.2f" % spend)

    no_split = sum(1 for _, usage in every_request.values() if usage.get("cache_creation") is None)
    print()
    print("%d records had no cache_creation object; their cache writes are priced at the 5-minute rate."
          % no_split)
    bad_lines = sum(found.bad_lines for found in transcripts)
    if bad_lines:
        print("%d transcript lines could not be read as JSON and were skipped." % bad_lines)
    no_id = sum(found.no_request_id for found in transcripts)
    if no_id:
        print("%d records with usage had no requestId and are not priced." % no_id)
    unpriced = collections.Counter(model for model, _ in every_request.values() if model not in PRICES)
    if unpriced:
        print("ERROR: no price for %s; the totals are too low." % dict(unpriced), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
