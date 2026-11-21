"""Tests for tools/agent_cost.py, the transcript accounting.

Each test runs the script the way a session does and reads its printed lines, its CSV and
its exit code. Most tests build a small transcript folder by hand. The tests in RealRecords
read real records cut from this project's transcripts (testdata/ch_runs/agent-cost-transcripts)
and work their expected dollars out from the script's own price table.

Run with: py -I tools/test_agent_cost.py
"""

import csv
import importlib.util
import json
import os
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
SCRIPT = os.path.join(HERE, "agent_cost.py")

FABLE = "claude-fable-5-1"
OPUS = "claude-opus-5-5"
SONNET = "claude-sonnet-5-5"
HAIKU = "claude-haiku-5-5"

S1 = "11111111-aaaa"
S2 = "22222222-bbbb"


def usage(fresh=0, output=0, read=0, five=0, hour=0):
    return {"input_tokens": fresh, "output_tokens": output, "cache_read_input_tokens": read,
            "cache_creation_input_tokens": five + hour,
            "cache_creation": {"ephemeral_5m_input_tokens": five, "ephemeral_1h_input_tokens": hour}}


# 100,000 in, 200,000 out, 1,000,000 read, 300,000 written for 5 minutes.
BIG = usage(100_000, 200_000, 1_000_000, 300_000)


def assistant(request, model=OPUS, used=None, effort="high", tools=0, at="2026-10-20T10:00:00.000Z"):
    content = [{"type": "tool_use", "id": "t", "name": "Read", "input": {}}] * tools
    content.append({"type": "text", "text": "x"})
    record = {"type": "assistant", "timestamp": at, "message": {"model": model, "content": content,
              "usage": used if used is not None else usage(output=1)}}
    if request is not None:
        record["requestId"] = request
    if effort is not None:
        record["effort"] = effort
    return record


class Base(unittest.TestCase):
    def setUp(self):
        folder = tempfile.TemporaryDirectory()
        self.addCleanup(folder.cleanup)
        self.root = folder.name
        self.csv = os.path.join(self.root, "rows.csv")

    def write(self, relative, records, tail=""):
        path = os.path.join(self.root, relative)
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", encoding="utf-8") as f:
            for record in records:
                f.write(json.dumps(record) + "\n")
            f.write(tail)

    def agent(self, session, agent_id, records, kind="executor", nested="", tail=""):
        folder = "%s/subagents/%s" % (session, nested)
        self.write("%sagent-%s.jsonl" % (folder, agent_id), records, tail)
        with open(os.path.join(self.root, folder, "agent-%s.meta.json" % agent_id), "w") as f:
            json.dump({"agentType": kind}, f)

    def main_session(self, session, records):
        self.write(session + ".jsonl", records)

    def run_script(self, *args):
        r = subprocess.run([sys.executable, "-I", SCRIPT, "--root", self.root, "--out", self.csv, *args],
                           capture_output=True, text=True)
        return r.returncode, r.stdout, r.stderr

    def rows(self, *args):
        code, out, err = self.run_script(*args)
        self.assertEqual(code, 0, out + err)
        with open(self.csv, newline="", encoding="utf-8") as f:
            return {row["agent_id"]: row for row in csv.DictReader(f)}

    def line(self, out, start):
        found = [text for text in out.splitlines() if text.startswith(start)]
        self.assertEqual(len(found), 1, out)
        return found[0]


class Pricing(Base):
    def cost_of(self, model, used):
        self.agent(S1, "a", [assistant("r0", model, usage(output=0)), assistant("r1", model, used)])
        return self.rows()["a"]["cost"]

    def test_fable(self):
        self.assertEqual(self.cost_of(FABLE, BIG), "15.0000")

    def test_opus(self):
        self.assertEqual(self.cost_of(OPUS, BIG), "6.1000")

    def test_sonnet(self):
        self.assertEqual(self.cost_of(SONNET, BIG), "3.1500")

    def test_haiku_one_token_over_the_context_limit_takes_the_long_price(self):
        self.assertEqual(self.cost_of(HAIKU, usage(30_001, 200_000, 50_000, 20_000)), "0.5300")

    def test_output_tokens_do_not_count_toward_haiku_context(self):
        self.assertEqual(self.cost_of(HAIKU, usage(100_000, 1_000_000)), "0.5100")

    def test_five_minute_cache_write_is_1_25_times_input(self):
        self.assertEqual(self.cost_of(OPUS, usage(five=1_000)), "0.0050")

    def test_one_hour_cache_write_is_2_times_input(self):
        self.assertEqual(self.cost_of(OPUS, usage(hour=1_000)), "0.0080")

    def test_both_cache_write_lengths_in_one_record(self):
        self.assertEqual(self.cost_of(OPUS, usage(five=1_000, hour=1_000)), "0.0130")

    def test_a_clean_run_still_says_how_many_records_had_no_split(self):
        self.agent(S1, "a", [assistant("r1")])
        self.assertIn("0 records had no cache_creation object", self.run_script()[1])

    def test_one_agent_on_two_models_prices_each_request_by_its_own_model(self):
        self.agent(S1, "a", [assistant("r1", OPUS, BIG), assistant("r2", SONNET, BIG)])
        row = self.rows()["a"]
        self.assertEqual(row["cost"], "9.2500")
        self.assertEqual(row["model"], OPUS + "+" + SONNET)


class Reading(Base):
    def test_last_usage_per_request_wins(self):
        self.agent(S1, "a", [assistant("r1", OPUS, usage(output=10)),
                             assistant("r1", OPUS, usage(output=50_000))])
        row = self.rows()["a"]
        self.assertEqual(row["cost"], "1.0000")
        self.assertEqual(row["output_tokens"], "50000")

    def test_effort_values_are_listed(self):
        self.agent(S1, "a", [assistant("r1", effort="high"), assistant("r2", effort="medium")])
        self.assertEqual(self.rows()["a"]["effort"], "high+medium")

    def test_an_assistant_record_with_no_effort_shows_in_the_list(self):
        self.agent(S1, "a", [assistant("r1", effort="low"), assistant("r2", effort=None)])
        self.assertEqual(self.rows()["a"]["effort"], "low+none")

    def test_tool_use_blocks_are_counted_on_every_record(self):
        self.agent(S1, "a", [assistant("r1", tools=2), assistant("r1", tools=1), assistant("r2")])
        self.assertEqual(self.rows()["a"]["calls"], "3")

    def test_row_carries_type_and_first_timestamp(self):
        self.agent(S1, "a", [assistant("r1", at="2026-10-20T10:00:00.000Z"),
                             assistant("r2", at="2026-10-21T10:00:00.000Z")], kind="reviewer")
        row = self.rows()["a"]
        self.assertEqual(row["type"], "reviewer")
        self.assertEqual(row["first_timestamp"], "2026-10-20T10:00:00.000Z")
        self.assertEqual(row["session"], S1)

    def test_agents_nested_under_workflows_are_found(self):
        self.agent(S1, "w", [assistant("r1")], nested="workflows/run-1/")
        self.assertIn("w", self.rows())

    def test_a_half_written_last_line_is_skipped_and_counted(self):
        self.agent(S1, "a", [assistant("r1", OPUS, BIG)], tail='{"type": "assi')
        code, out, _ = self.run_script()
        self.assertEqual(code, 0)
        self.assertIn("1 transcript lines could not be read", out)

    def test_a_message_claude_code_wrote_itself_is_no_request(self):
        self.agent(S1, "a", [assistant("r1", effort="high"),
                             assistant(None, "<synthetic>", usage(), effort=None)])
        code, out, err = self.run_script()
        self.assertEqual(code, 0, err)
        self.assertNotIn("no requestId", out)
        with open(self.csv, newline="") as f:
            self.assertEqual(next(csv.DictReader(f))["effort"], "high")


def load_agent_cost():
    """The script as a module, for its price table. py -I puts no folder on sys.path."""
    spec = importlib.util.spec_from_file_location("agent_cost", SCRIPT)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


AGENT_COST = load_agent_cost()

# Real assistant records, one per case folder, each cut from an agent transcript in this
# project's transcript folder (agent_cost.DEFAULT_ROOT, <session>/subagents/agent-*.jsonl).
# A record's own timestamp finds its source line again:
#   haiku-under-limit    a card-writer agent's Haiku request
#   model-with-no-price  a planner agent's claude-fable-5-2 request
#   no-meta-file         an Opus agent's request, from a transcript that had no meta file beside it
# Each record keeps only what agent_cost.py reads: type, timestamp, effort and the message's
# model and usage, as written. The requestId is replaced by "request-1", the session and
# agent folders by neutral names, and a meta file keeps only its agentType.
REAL_RECORDS = os.path.join(HERE, "..", "testdata", "ch_runs", "agent-cost-transcripts")


def real_record(case):
    """The one record of a case folder, and that folder's transcript root."""
    root = os.path.join(REAL_RECORDS, case)
    subagents = os.path.join(root, "session", "subagents")
    (name,) = [n for n in os.listdir(subagents) if n.endswith(".jsonl")]
    with open(os.path.join(subagents, name), encoding="utf-8") as f:
        (line,) = f.read().splitlines()
    return root, name[len("agent-"):-len(".jsonl")], json.loads(line)


def price_at(rates, used):
    """Dollars for one usage at one (input, output, cache read) rate triple from the table."""
    input_rate, output_rate, read_rate = rates
    split = used["cache_creation"]
    write_units = (split["ephemeral_5m_input_tokens"] * AGENT_COST.CACHE_WRITE_5M_TIMES_INPUT
                   + split["ephemeral_1h_input_tokens"] * AGENT_COST.CACHE_WRITE_1H_TIMES_INPUT)
    return (used["input_tokens"] * input_rate + used["output_tokens"] * output_rate
            + used["cache_read_input_tokens"] * read_rate + write_units * input_rate) / 1_000_000


class RealRecords(Base):
    def run_on(self, case):
        root, agent_id, record = real_record(case)
        self.root = root
        return agent_id, record

    def test_haiku_under_the_context_limit_keeps_the_short_price(self):
        agent_id, record = self.run_on("haiku-under-limit")
        model, used = record["message"]["model"], record["message"]["usage"]
        limit, long_rates = AGENT_COST.LONG_CONTEXT_PRICES[model]
        context = (used["input_tokens"] + used["cache_read_input_tokens"]
                   + used["cache_creation_input_tokens"])
        self.assertLessEqual(context, limit)
        short = "%.4f" % price_at(AGENT_COST.PRICES[model], used)
        self.assertNotEqual(short, "%.4f" % price_at(long_rates, used))
        self.assertEqual(self.rows()[agent_id]["cost"], short)

    def test_agent_without_meta_file_is_unknown_type(self):
        agent_id, _ = self.run_on("no-meta-file")
        subagents = os.path.join(self.root, "session", "subagents")
        self.assertFalse(os.path.exists(os.path.join(subagents, "agent-%s.meta.json" % agent_id)))
        self.assertEqual(self.rows()[agent_id]["type"], "unknown")

    def test_a_model_with_no_price_fails_loudly(self):
        _, record = self.run_on("model-with-no-price")
        model = record["message"]["model"]
        self.assertNotIn(model, AGENT_COST.PRICES)
        code, _, err = self.run_script()
        self.assertEqual(code, 1)
        self.assertIn(model, err)


class Filters(Base):
    def setUp(self):
        super().setUp()
        self.agent(S1, "old", [assistant("r1", OPUS, BIG, at="2026-10-18T23:59:59.000Z"),
                               assistant("r2", OPUS, BIG, at="2026-10-19T00:00:01.000Z")])
        self.agent(S1, "new", [assistant("r3", OPUS, BIG, at="2026-10-19T00:00:00.000Z")])
        self.agent(S2, "other", [assistant("r4", SONNET, BIG, at="2026-10-25T00:00:00.000Z")], kind="scout")
        self.main_session(S1, [assistant("m1", OPUS, BIG, at="2026-10-18T00:00:00.000Z"),
                               assistant("m2", FABLE, BIG, at="2026-10-20T00:00:00.000Z", tools=2)])
        self.main_session(S2, [assistant("m3", OPUS, usage(hour=1_000_000))])

    def test_no_filter_reads_the_whole_project(self):
        self.assertEqual(set(self.rows()), {"old", "new", "other"})

    def test_since_keeps_an_agent_by_its_first_timestamp_inclusive(self):
        self.assertEqual(set(self.rows("--since", "2026-10-19")), {"new", "other"})

    def test_since_takes_a_date_time(self):
        self.assertEqual(set(self.rows("--since", "2026-10-19T00:00:01")), {"other"})

    def test_one_session(self):
        self.assertEqual(set(self.rows("--session", S2)), {"other"})

    def test_several_sessions_after_one_flag(self):
        self.assertEqual(set(self.rows("--session", S1, S2)), {"old", "new", "other"})

    def test_several_sessions_by_repeating_the_flag(self):
        self.assertEqual(set(self.rows("--session", S1, "--session", S2)), {"old", "new", "other"})

    def test_one_agent(self):
        self.assertEqual(set(self.rows("--agent", "new")), {"new"})

    def test_main_session_cost_is_printed_for_each_session(self):
        _, out, _ = self.run_script("--session", S1, S2)
        self.assertIn("$21.10, 2 calls", self.line(out, "Main session " + S1))
        self.assertIn("$8.00", self.line(out, "Main session " + S2))

    def test_total_spend_sums_agents_and_main_sessions_over_every_session(self):
        # Agents 12.20 + 6.10 + 3.15, main sessions 21.10 + 8.00.
        _, out, _ = self.run_script("--session", S1, S2)
        self.assertIn("$50.55", self.line(out, "Total spend"))

    def test_since_counts_only_the_later_part_of_a_main_session(self):
        _, out, _ = self.run_script("--session", S1, "--since", "2026-10-19")
        self.assertIn("$15.00", self.line(out, "Main session " + S1))

    def test_a_request_copied_into_a_second_session_counts_once_in_the_total(self):
        self.main_session(S2, [assistant("m1", OPUS, BIG, at="2026-10-18T00:00:00.000Z")])
        _, out, _ = self.run_script("--session", S1, S2)
        # As the total above, without S2's 8.00 main session, whose one request S1 also holds.
        self.assertIn("$42.55", self.line(out, "Total spend"))

    def test_no_main_session_line_without_session(self):
        self.assertNotIn("Main session", self.run_script()[1])

    def test_a_session_with_no_transcript_is_an_error_not_zero_dollars(self):
        code, _, err = self.run_script("--session", "no-such-id")
        self.assertEqual(code, 2)
        self.assertIn("no-such-id", err)

    def summary_fields(self, out, kind, effort):
        """The fields of the one summary line for this type and effort."""
        found = [text.split() for text in out.splitlines()
                 if text.split()[:2] == [kind, effort]]
        self.assertEqual(len(found), 1, out)
        return found[0]

    def test_summary_groups_by_type_and_effort(self):
        # The two high-effort executors' costs, as the script's own rows give them.
        costs = [float(row["cost"]) for row in self.rows().values()
                 if (row["type"], row["effort"]) == ("executor", "high")]
        self.assertEqual(len(costs), 2, costs)
        _, out, _ = self.run_script()
        self.assertEqual(self.summary_fields(out, "executor", "high")[2:6],
                         ["2", "%.2f" % ((costs[0] + costs[1]) / 2),
                          "%.2f" % ((costs[0] + costs[1]) / 2), "%.2f" % max(costs)])
        self.assertEqual(self.summary_fields(out, "scout", "high")[2], "1")


if __name__ == "__main__":
    unittest.main()
