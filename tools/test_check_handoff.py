"""Tests for tools/check_handoff.py, the handoff checker.

Each test builds small git repos in a temp folder, writes a handoff file that names
things in them, runs the script the way a session does and reads its printed lines
and exit code. Run with: py -I tools/test_check_handoff.py
"""

import datetime
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
SCRIPT = os.path.join(HERE, "check_handoff.py")

OLD = "2026-10-01T12:00:00"


def git(repo, *args, date=OLD):
    env = {k: v for k, v in os.environ.items() if not k.startswith("GIT_")}
    env.update(GIT_AUTHOR_DATE=date, GIT_COMMITTER_DATE=date)
    r = subprocess.run(["git", "-C", repo, "-c", "user.name=t", "-c", "user.email=t@t", *args],
                       capture_output=True, text=True, env=env)
    if r.returncode != 0:
        raise RuntimeError("git %s failed: %s" % (" ".join(args), r.stderr))
    return r.stdout.strip()


def commit(repo, files, subject, date=OLD):
    for name, text in files.items():
        path = os.path.join(repo, name)
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", newline="\n") as f:
            f.write(text)
        git(repo, "add", name)
    git(repo, "commit", "-q", "-m", subject, date=date)
    return git(repo, "rev-parse", "HEAD")


def run(*args):
    r = subprocess.run([sys.executable, "-I", SCRIPT, *args], capture_output=True, text=True)
    return r.returncode, r.stdout, r.stderr


def section(out, name):
    """The lines of one scrape-mode section, without its heading or its "(none)" line."""
    lines, inside = [], False
    for line in out.splitlines():
        if not line.startswith(" "):
            inside = line.startswith(name)
            continue
        if inside and line.strip() != "(none)":
            lines.append(line.strip())
    return lines


class Base(unittest.TestCase):
    """A repo with main pushed to a bare origin, one side branch and one unpushed commit."""

    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="check-handoff-")
        self.repo = os.path.join(self.tmp, "repo")
        os.makedirs(self.repo)
        git(self.repo, "init", "-q", "-b", "main")
        self.c_main = commit(self.repo, {"src/thing.cpp": "int compute_total(int x)\n{\n    return x;\n}\n",
                                         "README.md": "readme\n"}, "add thing")
        git(self.repo, "checkout", "-q", "-b", "claude/side")
        self.c_side = commit(self.repo, {"side.txt": "side\n"}, "side work", date="2026-10-02T12:00:00")
        git(self.repo, "checkout", "-q", "main")
        self.origin = os.path.join(self.tmp, "origin.git")
        git(self.tmp, "init", "-q", "--bare", self.origin)
        git(self.repo, "remote", "add", "origin", self.origin)
        git(self.repo, "push", "-q", "origin", "main")
        self.c_unpushed = commit(self.repo, {"later.txt": "later\n"}, "not pushed yet",
                                 date="2026-10-03T12:00:00")

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def handoff(self, text, name="2026-10-15-test-handoff.md", folder=None):
        path = os.path.join(folder or self.tmp, name)
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
        return path

    def scrape(self, text, **kw):
        return run(self.handoff(text, **kw), "--repo", self.repo)


class ScrapeMode(Base):
    def test_existing_claims_listed(self):
        code, out, _ = self.scrape("Landed `%s` on `claude/side`, see `src/thing.cpp` and `compute_total()`.\n"
                                   % self.c_main)
        exists = section(out, "EXISTS")
        self.assertTrue(any(self.c_main in l and "add thing" in l and "on main" in l for l in exists), out)
        self.assertTrue(any(l.startswith("claude/side") and self.c_side[:10] in l for l in exists), out)
        self.assertTrue(any(l.startswith("src/thing.cpp") for l in exists), out)
        self.assertTrue(any(l.startswith("compute_total()") and "src/thing.cpp:1" in l for l in exists), out)
        self.assertEqual(section(out, "MISSING"), [])
        self.assertEqual(code, 0)

    def test_missing_claims_fail(self):
        code, out, _ = self.scrape("Gone: `deadbeef1234567`, `claude/nope`, `src/nope.cpp`, `nope_function()`.\n")
        missing = section(out, "MISSING")
        for name in ("deadbeef1234567", "claude/nope", "src/nope.cpp", "nope_function()"):
            self.assertTrue(any(l.startswith(name) for l in missing), name + "\n" + out)
        self.assertEqual(section(out, "EXISTS"), [])
        self.assertEqual(code, 1)
        self.assertIn("checked 4 claims: 0 exist, 4 missing", out)

    def test_changed_since_date(self):
        commit(self.repo, {"src/thing.cpp": "int compute_total(int x) { return x + 1; }\n"},
               "change thing again", date="2026-10-20T12:00:00")
        _, out, _ = self.scrape("See `src/thing.cpp`.\n")
        self.assertIn("CHANGED SINCE 2026-10-15", out)
        changed = section(out, "CHANGED SINCE")
        self.assertEqual(len(changed), 1, out)
        self.assertTrue(changed[0].startswith("src/thing.cpp: 1 commit"), out)
        self.assertIn("change thing again", changed[0])
        self.assertIn("1 changed since 2026-10-15", out)

    def test_unchanged_is_not_listed(self):
        commit(self.repo, {"src/thing.cpp": "int compute_total(int x) { return x + 1; }\n"},
               "change thing again", date="2026-10-20T12:00:00")
        _, out, _ = self.scrape("See `README.md`.\n")
        self.assertEqual(section(out, "CHANGED SINCE"), [], out)
        self.assertTrue(any(l.startswith("README.md") for l in section(out, "EXISTS")), out)

    def test_prose_is_ignored(self):
        code, out, _ = self.scrape("The user said `yes` and `--repo` is a flag.\n")
        self.assertNotIn("yes", out)
        self.assertIn("checked 0 claims", out)
        self.assertEqual(code, 0)

    def test_other_repo_root_on_line(self):
        hooks = os.path.join(self.tmp, "hooks")
        os.makedirs(hooks)
        git(hooks, "init", "-q", "-b", "main")
        h = commit(hooks, {"gate.py": "x = 1\n"}, "hook commit")
        code, out, _ = self.scrape("In `%s` the hook landed as `%s`.\n" % (hooks, h))
        self.assertTrue(any(l.startswith(h) and "hook commit" in l for l in section(out, "EXISTS")), out)
        self.assertEqual(code, 0)
        code, out, _ = self.scrape("The hook landed as `%s`.\n" % h, name="2026-10-15-other-handoff.md")
        self.assertTrue(any(l.startswith(h) for l in section(out, "MISSING")), out)
        self.assertEqual(code, 1)

    def test_date_sources(self):
        _, out, _ = self.scrape("See `README.md`.\n")
        self.assertIn("CHANGED SINCE 2026-10-15", out)
        tracked = self.handoff("See `README.md`.\n", name="undated-handoff.md", folder=self.repo)
        git(self.repo, "add", "undated-handoff.md")
        git(self.repo, "commit", "-q", "-m", "handoff", date="2026-10-05T12:00:00")
        _, out, err = run(tracked, "--repo", self.repo)
        self.assertIn("CHANGED SINCE 2026-10-05", out)
        self.assertEqual(err, "")
        _, out, err = self.scrape("See `README.md`.\n", name="loose-handoff.md")
        self.assertIn("CHANGED SINCE %s" % datetime.date.today().isoformat(), out)
        self.assertIn("warning", err)
        _, out, _ = run(tracked, "--repo", self.repo, "--since", "2026-09-30")
        self.assertIn("CHANGED SINCE 2026-09-30", out)

    def test_usage(self):
        code, out, err = run()
        self.assertEqual(code, 2)
        self.assertIn("usage", err)
        code, _, err = run(os.path.join(self.tmp, "no-such-handoff.md"), "--repo", self.repo)
        self.assertEqual(code, 2)
        self.assertIn("usage", err)
        code, _, err = run(self.handoff("x\n"), "--repo", self.repo, "--since", "last week")
        self.assertEqual(code, 2)

    def test_doc_mention_is_not_a_function(self):
        commit(self.repo, {"docs/notes.md": "Old::gone_function was removed.\n"}, "notes")
        code, out, _ = self.scrape("See `Old::gone_function`.\n")
        missing = section(out, "MISSING")
        self.assertTrue(any(l.startswith("Old::gone_function") and "named only in docs, docs/notes.md:1" in l
                            for l in missing), out)
        self.assertEqual(code, 1)

    def test_qualified_function_found_where_written(self):
        commit(self.repo, {"aaa/first.cpp": "void f() { read_frames(); }\n",
                           "docs/durations.txt": "Playhead::read_frames 3 ms\n",
                           "src/player.cpp": "// player\nint Playhead::read_frames() { return 0; }\n"}, "player")
        _, out, _ = self.scrape("See `Playhead::read_frames`.\n")
        self.assertTrue(any(l.startswith("Playhead::read_frames") and "src/player.cpp:2" in l
                            for l in section(out, "EXISTS")), out)

    def test_path_matching_only_by_its_tail_is_missing(self):
        code, out, _ = self.scrape("In the source folder see `thing.cpp`, and `app/thing.cpp` is wrong.\n")
        self.assertEqual(section(out, "EXISTS"), [], out)
        missing = section(out, "MISSING")
        self.assertTrue(any(l.startswith("thing.cpp") and "src/thing.cpp" in l for l in missing), out)
        self.assertTrue(any(l.startswith("app/thing.cpp") for l in missing), out)
        self.assertEqual(code, 1)
        commit(self.repo, {"deep/src/other.cpp": "x\n"}, "another file")
        _, out, _ = self.scrape("See `src/other.cpp`.\n")
        self.assertTrue(any(l.startswith("src/other.cpp") and "a different file ends the same way" in l
                            for l in section(out, "MISSING")), out)

    def test_path_outside_the_repo_or_in_the_wrong_case_is_missing(self):
        with open(os.path.join(self.tmp, "outside.txt"), "w") as f:
            f.write("x\n")
        _, out, _ = self.scrape("See `SRC/Thing.cpp`, `../outside.txt` and `/README.md`.\n")
        self.assertEqual(section(out, "EXISTS"), [], out)
        self.assertIn("checked 3 claims: 0 exist, 3 missing", out)

    def test_qualified_method_needs_its_class_in_the_file(self):
        commit(self.repo, {"src/other.cpp": "int compute_total(int);\nint Widget::draw() { return 0; }\n",
                           "src/widget.h": "struct Gadget { int draw() { return 1; } };\n"}, "more")
        code, out, _ = self.scrape("`Gadget::draw` and `Widget::run` and `Gadget::compute_total()`.\n")
        exists = section(out, "EXISTS")
        self.assertTrue(any(l.startswith("Gadget::draw") and "src/widget.h:1" in l for l in exists), out)
        missing = section(out, "MISSING")
        self.assertTrue(any(l.startswith("Widget::run") for l in missing), out)
        self.assertTrue(any(l.startswith("Gadget::compute_total()") for l in missing), out)
        self.assertEqual(code, 1)

    def test_session_id_is_not_a_commit(self):
        _, out, _ = self.scrape("Written in session e3dd3907, id 6777b546-e751-41ab-bb22-ba066e7b1412.\n")
        self.assertIn("checked 0 claims", out)


TABLE_HEAD = "# A handoff\n\nProse.\n\n## Claims\n\n| Kind | Value | Claim | How to check |\n|---|---|---|---|\n"


def row(kind, value, claim="a claim"):
    return "| %s | `%s` | %s | check it |\n" % (kind, value, claim)


class TableMode(Base):
    def test_claims_table_statuses(self):
        text = TABLE_HEAD + "".join([
            row("commit", self.c_main, "on main and pushed"),
            row("commit", self.c_side, "on a side branch"),
            row("commit", self.c_unpushed, "on main only"),
            row("commit", "0123456789abcdef0123456789abcdef01234567", "made up"),
            row("commit", self.c_main[:12], "abbreviated"),
            row("path", "src/thing.cpp", "real path"),
            row("path", "src/nope.cpp", "made-up path"),
            row("branch", "claude/side", "real branch"),
            row("branch", "claude/nope", "made-up branch"),
            row("function", "compute_total()", "real function"),
            row("function", "nope_function()", "made-up function"),
            row("test", "py -I tools/test_x.py", "Ran 3 tests OK"),
            row("ruling", "6777b546", '"yes plan all of this"'),
            row("state", "the worktree x exists", "it exists"),
        ]) + "\n## Checker output\n\nnothing yet\n"
        code, out, _ = self.scrape(text)
        lines = out.splitlines()
        want = ["ok", "not-on-main", "not-pushed", "missing", "missing", "ok", "missing",
                "ok", "missing", "ok", "missing", "manual", "manual", "manual"]
        self.assertEqual([l.split(" | ")[0] for l in lines[:-1]], want, out)
        self.assertEqual(lines[0], "ok | commit | %s | on main and pushed" % self.c_main)
        self.assertEqual(lines[-1], "checked 14 rows: 4 ok, 5 missing, 1 not-on-main, 1 not-pushed, 3 manual")
        self.assertEqual(code, 1)

    def test_claims_table_only_rows(self):
        text = ("Earlier prose names `deadbeef1234567` and `src/nope.cpp`.\n\n" + TABLE_HEAD
                + row("path", "README.md") + row("ruling", "abc", '"ok"'))
        code, out, _ = self.scrape(text)
        self.assertEqual(out.splitlines(), ["ok | path | README.md | a claim", 'manual | ruling | abc | "ok"',
                                            "checked 2 rows: 1 ok, 0 missing, 0 not-on-main, 0 not-pushed, 1 manual"])
        self.assertEqual(code, 0)

    def test_claims_exit_code(self):
        code, _, _ = self.scrape(TABLE_HEAD + row("commit", self.c_main) + row("path", "README.md"))
        self.assertEqual(code, 0)
        code, _, _ = self.scrape(TABLE_HEAD + row("commit", self.c_main) + row("path", "src/nope.cpp"))
        self.assertEqual(code, 1)
        code, _, _ = self.scrape(TABLE_HEAD + row("commit", self.c_side))
        self.assertEqual(code, 1)
        code, _, _ = self.scrape(TABLE_HEAD + row("branch", "claude/nope"))
        self.assertEqual(code, 1)

    def test_claims_rows_that_must_not_pass(self):
        text = TABLE_HEAD + "".join([
            "| path | | empty value | x |\n",
            row("path", "."),
            row("path", "../outside.txt", "a real file outside the repo"),
            row("path", "thing.cpp", "only its tail"),
            row("path", "SRC/thing.cpp", "wrong case"),
            row("function", "not a function", "prose"),
            row("function", "compute_total", "bare name"),
            row("function", "Gadget::compute_total()", "wrong class"),
            row("fn", "compute_total()", "kind the guide does not have"),
        ])
        with open(os.path.join(self.tmp, "outside.txt"), "w") as f:
            f.write("x\n")
        code, out, _ = self.scrape(text)
        got = [l.split(" | ")[0] for l in out.splitlines()[:-1]]
        self.assertEqual(got, ["missing"] * 6 + ["ok", "missing", "unknown"], out)
        self.assertEqual(code, 1)

    def test_claims_table_with_no_readable_row_fails(self):
        code, out, err = self.scrape("## Claims\n\nnothing here\n")
        self.assertEqual(code, 1)
        self.assertIn("error", err)
        self.assertIn("checked 0 rows", out)

    def test_row_without_outer_pipes_is_read(self):
        code, out, _ = self.scrape(TABLE_HEAD + "path | `src/nope.cpp` | no pipes | x\n")
        self.assertTrue(out.startswith("missing | path | src/nope.cpp"), out)
        self.assertEqual(code, 1)


if __name__ == "__main__":
    unittest.main()
