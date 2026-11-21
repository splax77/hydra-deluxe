"""Tests for kinds.py: each line of the sorting rule, with a partner case that
must come out differently, on a small made-up copy shaped like the real lines
the plan cites. Run: py -I tools/blind_copy/test_kinds.py"""

import contextlib
import io
import os
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import find_seams as fs  # noqa: E402
import kinds as k  # noqa: E402
import match_records as mr  # noqa: E402

SRC = {
    "src/net/http.h": """\
namespace hydra::net {
using Fetcher = std::function<std::string(const std::string& url, const std::atomic<bool>* cancel)>;
using Downloader = std::function<void(const std::string& url, const std::string& path)>;
void set_fetcher(Fetcher fetcher);
void set_downloader(Downloader downloader);
}
""",
    "src/app/analysis.h": """\
namespace hydra::app {
using ChartAnalyzer = std::function<AnalysisResult(const std::string& path, const AnalysisSettings& settings)>;
void set_installed_for_test(std::optional<bool> installed);
}
""",
    "src/ui/jobs.h": """\
namespace hydra::ui {
using StoreOpenGate = std::function<void()>;
void set_view_analyzer_for_test(app::ChartAnalyzer analyzer);
void set_app_batch_analyzer_for_test(app::ChartAnalyzer analyzer, int workers);
void set_scan_hold_for_test(std::function<void()> hold);
void set_store_open_gate_for_test(StoreOpenGate gate);
void set_update_folder_for_test(std::string folder);
void set_audio_device_factory(AudioDeviceFactory factory);
struct Job {
    void set_analyzer_for_test(app::ChartAnalyzer analyzer, int workers);
};
using AudioDeviceFactory = std::function<std::unique_ptr<Device>(int channels)>;
}
""",
    "src/store/store.h": """\
class RecordStore {
    int wal_autocheckpoint_for_test() const;
    void set_wal_autocheckpoint_for_test(int pages);
    void fail_chart_list_reader_open_for_test(int nth);
};
""",
}

TESTS = {
    "tests/test_a.cpp": """\
hydra::app::ChartAnalyzer real_analyzer() {
    return [](const std::string& path, const AnalysisSettings& settings) -> AnalysisResult {
        return hydra::app::analyze_chart_file(path, settings);
    };
}

hydra::app::ChartAnalyzer always(AnalysisResult real) {
    return [real](const std::string&, const AnalysisSettings&) { return real; };
}

AudioDeviceFactory counting_factory(int& calls) {
    auto fail = no_device_factory();
    return [&calls, fail](int channels) {
        ++calls;
        return fail(channels);
    };
}

void start_redo(hydra::app::ChartAnalyzer analyzer) {
    const ScopedHook seam(hydra::ui::set_app_batch_analyzer_for_test, std::move(analyzer), 1);  // L:param
}

ScopedHook failing_view_analyzer(int error) {  // L:header
    return ScopedHook(hydra::ui::set_view_analyzer_for_test,  // L:returned
                      [error](const std::string&, const AnalysisSettings&) -> AnalysisResult { throw error; });
}

struct World {
    ScopedHook installed{hydra::app::set_installed_for_test, std::optional<bool>(true)};  // L:w_installed
    ScopedHook fetcher;  // L:w_member
    World()
        : fetcher(hydra::net::set_fetcher,  // L:w_fetcher
                  [this](const std::string& url, const std::atomic<bool>*) -> std::string {
                      if (fail) throw std::runtime_error("down");
                      return reply;
                  }) {}
};

struct Holder {
    std::optional<ScopedHook> seam_;  // L:opt_member
    void hold() {
        seam_.emplace(hydra::ui::set_scan_hold_for_test, [] { wait_until([] { return go; }, "go"); });  // L:emplace
    }
};

TEST_CASE("cases") {
    hydra::net::set_fetcher([](const std::string& url, const std::atomic<bool>*) -> std::string {  // L:throw_fetch
        throw std::runtime_error("offline");
    });
    hydra::net::set_fetcher([&](const std::string& url, const std::atomic<bool>*) { return canned.reply; });  // L:reply_fetch
    job.set_analyzer_for_test([](const std::string& path, const AnalysisSettings& settings) {  // L:wait_real
        wait_until([] { return open; }, "open");
        return hydra::app::analyze_chart_file(path, settings);
    }, 1);
    job.set_analyzer_for_test([&](const std::string& path, const AnalysisSettings& settings) {  // L:wait_canned
        wait_until([] { return open; }, "open");
        return canned;
    }, 1);
    job.set_analyzer_for_test(real_analyzer(), 1);  // L:real_only
    job.set_analyzer_for_test(always(real), 1);  // L:always
    hydra::app::set_installed_for_test(std::optional<bool>(true));  // L:plain
    hydra::app::set_installed_for_test({});  // L:cleared
    CHECK(store.wal_autocheckpoint_for_test() == 5);  // L:getter
    store.set_wal_autocheckpoint_for_test(1);  // L:setter
    store.fail_chart_list_reader_open_for_test(2);  // L:fail_int
    const ScopedHook a(hydra::ui::set_scan_hold_for_test, [&] { wait_until([&] { return go; }, "go"); });  // L:void_wait
    const ScopedHook b(hydra::ui::set_scan_hold_for_test, [&] { ++count; });  // L:void_plain
    hydra::net::set_downloader([](const std::string& url, const std::string& path) { throw std::runtime_error("x"); });  // L:dl_throw
    hydra::net::set_downloader([](const std::string& url, const std::string& path) { write_file(path, "x"); });  // L:dl_write
    const ScopedHook c(hydra::ui::set_store_open_gate_for_test, [&] { ++opened; });  // L:alias
    hydra::ui::set_update_folder_for_test(folder);  // L:plain_type
    const ScopedHook d(  // L:stmt_head
        hydra::ui::set_scan_hold_for_test,  // L:stmt_hook
        [&] { throw broke; });
    const ScopedHook e = failing_view_analyzer(3);  // L:from_helper
    World w;  // L:wrapper
    start_redo([](const std::string&, const AnalysisSettings&) -> AnalysisResult { throw 1; });  // L:caller_throw
    start_redo(real_analyzer());  // L:caller_real
    pc.set_audio_device_factory(counting_factory(calls));  // L:counting
    act_at_seam(hydra::ui::set_scan_hold_for_test, app);  // L:passed
    const ScopedHook f(hydra::ui::set_scan_hold_for_test, [&] {  // L:wait_throw
        wait_until([&] { return go; }, "go");
        throw broke;
    });
    const ScopedHook g(hydra::ui::set_scan_hold_for_test, [&] {  // L:wait_call
        wait_until([&] { return go; }, "go");
        take_reading(app);
    });
}
""",
    "tests/ui/harness.h": """\
struct ViewGate {
    ViewGate();
    ScopedHook seam_;  // L:cpp_member
};
""",
    "tests/ui/harness.cpp": """\
#include "harness.h"
ViewGate::ViewGate()
    : seam_(hydra::ui::set_view_analyzer_for_test,  // L:cpp_init
            [](const std::string& path, const hydra::app::AnalysisSettings& settings) {
                testwait::wait_until([&] { return open; }, "open");
                return hydra::app::analyze_chart_file(path, settings);
            }) {}
""",
}


def build(root, files):
    for path, body in files.items():
        real = os.path.join(root, "code", *path.split("/"))
        os.makedirs(os.path.dirname(real), exist_ok=True)
        with open(real, "w", encoding="utf-8", newline="\n") as f:
            f.write(body)
    os.makedirs(os.path.join(root, "code", "tools"), exist_ok=True)


def marks(files):
    out = {}
    for path, body in files.items():
        for n, line in enumerate(body.split("\n"), 1):
            if "// L:" in line:
                out[line.split("// L:")[1].strip()] = (path, n)
    return out


class Copy(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.copy = cls.tmp.name
        build(cls.copy, {**SRC, **TESTS})
        cls.at = marks(TESTS)
        cls.found = fs.search(cls.copy)
        cov = {c["name"]: c["declared_at"] for c in cls.found["coverage"]}
        # Every declared hook word, so a hit the search's patterns skip is
        # still sorted by the rule.
        cls.sorter = k.Sorter(cls.copy, cov, set(cls.at.values()))

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def rows(self, mark):
        path, n = self.at[mark]
        return self.sorter.sort(path, n)

    def one(self, mark):
        got = self.rows(mark)
        self.assertEqual(len(got), 1, [(r.outcome, r.note) for r in got])
        return got[0]

    def outcome(self, mark):
        return self.one(mark).outcome


class TypeSettles(Copy):
    def test_getter_against_setter(self):
        self.assertEqual(self.outcome("getter"), k.NOTHING)
        self.assertEqual(self.outcome("setter"), k.FAKE)

    def test_fail_switch_with_int_against_plain_setter(self):
        self.assertEqual(self.outcome("fail_int"), k.FAIL)
        self.assertEqual(self.outcome("setter"), k.FAKE)

    def test_plain_value_against_cleared(self):
        self.assertEqual(self.outcome("plain"), k.FAKE)
        self.assertEqual(self.outcome("cleared"), k.NOTHING)

    def test_alias_against_plain_type(self):
        alias = self.one("alias")
        self.assertEqual(alias.outcome, k.MIDSTEP)
        self.assertIn("StoreOpenGate = std::function<void()>", alias.decl)
        self.assertEqual(self.outcome("plain_type"), k.FAKE)


class BodySorts(Copy):
    def test_throwing_fetcher_against_one_that_returns(self):
        self.assertEqual(self.outcome("throw_fetch"), k.FAIL)
        self.assertEqual(self.outcome("reply_fetch"), k.FAKE)

    def test_wait_before_real_against_wait_before_canned(self):
        real = self.one("wait_real")
        self.assertEqual((real.outcome, real.mark), (k.WAIT, ""))
        canned = self.one("wait_canned")
        self.assertEqual(canned.outcome, k.FAKE)
        self.assertTrue(canned.mark)

    def test_real_alone_against_return_real(self):
        self.assertEqual(self.outcome("real_only"), k.NOTHING)
        self.assertEqual(self.outcome("always"), k.FAKE)

    def test_void_with_wait_against_without(self):
        self.assertEqual(self.outcome("void_wait"), k.WAIT)
        self.assertEqual(self.outcome("void_plain"), k.MIDSTEP)

    def test_downloader_that_throws_against_one_that_writes(self):
        self.assertEqual(self.outcome("dl_throw"), k.FAIL)
        self.assertEqual(self.outcome("dl_write"), k.FAKE)

    def test_helper_followed_to_its_lambda(self):
        r = self.one("real_only")
        self.assertTrue(r.body.startswith("tests/test_a.cpp:2-"), r.body)

    def test_wait_then_throw_is_marked(self):
        r = self.one("wait_throw")
        self.assertEqual(r.outcome, k.WAIT)
        self.assertIn("throws", r.mark)

    def test_wait_and_another_call_is_marked(self):
        r = self.one("wait_call")
        self.assertEqual(r.outcome, k.WAIT)
        self.assertIn("take_reading", r.mark)

    def test_counting_factory_is_unsorted(self):
        self.assertEqual(self.outcome("counting"), k.UNSORTED)

    def test_hook_passed_as_argument_is_unsorted(self):
        self.assertEqual(self.outcome("passed"), k.UNSORTED)

    def test_parameter_sorted_per_caller(self):
        got = {r.via: r.outcome for r in self.rows("param")}
        path = "tests/test_a.cpp"
        self.assertEqual(got, {f"{path}:{self.at['caller_throw'][1]}": k.FAIL,
                               f"{path}:{self.at['caller_real'][1]}": k.NOTHING})


class LinesWithNoHook(Copy):
    def test_member_matched_to_its_initialiser(self):
        r = self.one("w_member")
        self.assertEqual(r.via, f"tests/test_a.cpp:{self.at['w_fetcher'][1]}")
        self.assertEqual(r.outcome, self.outcome("w_fetcher"))
        self.assertEqual(r.outcome, k.FAKE)
        self.assertIn("throw", r.mark)

    def test_member_matched_to_initialiser_in_cpp(self):
        r = self.one("cpp_member")
        self.assertEqual(r.via, f"tests/ui/harness.cpp:{self.at['cpp_init'][1]}")
        self.assertEqual(r.outcome, k.WAIT)

    def test_member_matched_to_emplace(self):
        r = self.one("opt_member")
        self.assertEqual(r.via, f"tests/test_a.cpp:{self.at['emplace'][1]}")
        self.assertEqual(r.outcome, k.WAIT)

    def test_header_whose_helper_returns_scoped_hook(self):
        for mark in ("header", "from_helper"):
            r = self.one(mark)
            self.assertEqual(r.via, f"tests/test_a.cpp:{self.at['returned'][1]}")
            self.assertEqual(r.outcome, k.FAIL)

    def test_statement_head_takes_its_hook(self):
        self.assertEqual(self.outcome("stmt_head"), k.FAIL)

    def test_wrapper_takes_its_inner_kinds(self):
        got = sorted(r.outcome for r in self.rows("wrapper"))
        self.assertEqual(got, sorted([k.FAKE, k.FAKE]))
        vias = {r.via for r in self.rows("wrapper")}
        self.assertEqual(vias, {f"tests/test_a.cpp:{self.at['w_installed'][1]}",
                                f"tests/test_a.cpp:{self.at['w_fetcher'][1]}"})


def write(path, header, rows):
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\t".join(header) + "\n")
        for r in rows:
            f.write("\t".join(r) + "\n")


def seam_row(test, test_file, hit_file, hit_line):
    return [test, test_file, "1", "yes", hit_file, str(hit_line), "for_test name", "x"]


def matched_row(record, name, file, state, newest="yes"):
    row = {c: "" for c in mr.COLUMNS}
    row.update({"record": record, "test name": name, "test file": file, "match": mr.SAME_PLACE,
                "state": state, "newest copy": newest, "test state": state,
                "line 9": f"{record}: 9 Stand-in: None.", "line 11": f"{record}: 11 Real: Yes."})
    return [row[c] for c in mr.COLUMNS]


class Run(Copy):
    def run_kinds(self, seams, matched, answers=None, repo=None, at=None, base=k.BASE):
        out = tempfile.mkdtemp(dir=self.copy)
        sp, cp, mp = (os.path.join(out, n) for n in ("seams.tsv", "coverage.tsv", "matched.tsv"))
        write(sp, fs.SEAM_COLUMNS, seams)
        write(cp, fs.COVERAGE_COLUMNS, [[c[x] for x in fs.COVERAGE_COLUMNS] for c in self.found["coverage"]])
        write(mp, mr.COLUMNS, matched)
        ap = None
        if answers is not None:
            ap = os.path.join(out, "answers.tsv")
            write(ap, ("kind", "answer", "exceptions"), answers)
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            code = k.main([self.copy, "--seams", sp, "--coverage", cp, "--matched", mp, "--out", out]
                          + (["--answers", ap] if ap else [])
                          + (["--repo", repo, "--at", at, "--base", base] if repo else []))
        joined = k.read_tsv(os.path.join(out, "joined.tsv"))
        return code, buf.getvalue(), out, joined

    def test_many_unsorted_prints_six_counts_and_exits_0(self):
        path, n = self.at["passed"]
        cpath, cn = self.at["counting"]
        seams = [seam_row("t1", "tests/test_a.cpp", path, n), seam_row("t2", "tests/test_a.cpp", cpath, cn)]
        code, printed, out, _ = self.run_kinds(seams, [])
        self.assertEqual(code, 0)
        for o in k.OUTCOMES:
            self.assertIn(f"outcome: {o} | rows: ", printed)
        self.assertIn(f"outcome: {k.UNSORTED} | rows: 2 |", printed)
        with open(os.path.join(out, "kinds.md"), encoding="utf-8") as f:
            md = f.read()
        for o in k.OUTCOMES:
            self.assertIn(f"\n## {o}\n", md)
        self.assertIn(f"- hit: {path}:{n}", md)

    def test_joined_states_and_answers(self):
        tp, tn = self.at["throw_fetch"]
        rp, rn = self.at["reply_fetch"]
        gp, gn = self.at["getter"]
        seams = [seam_row("harness like", "tests/test_a.cpp", tp, tn),
                 seam_row("world like", "tests/test_a.cpp", rp, rn),
                 seam_row("nothing only", "tests/test_a.cpp", gp, gn),
                 seam_row("no record", "tests/test_a.cpp", rp, rn)]
        matched = [matched_row("A.md:1", "harness like", "tests/test_a.cpp", mr.FLAGGED),
                   matched_row("A.md:2", "world like", "tests/test_a.cpp", mr.FLAGGED),
                   matched_row("A.md:3", "nothing only", "tests/test_a.cpp", mr.FLAGGED)]
        _, _, _, joined = self.run_kinds(seams, matched, [[k.FAKE, k.CONTRADICTED, ""]])
        res = {j["test"]: j["result"] for j in joined}
        self.assertEqual(res["harness like"], mr.FLAGGED)
        self.assertEqual(res["world like"], k.CONTRADICTED)
        self.assertEqual(res["nothing only"], mr.FLAGGED)
        self.assertEqual({j["test"]: j["state"] for j in joined}["no record"], k.NO_RECORD)
        _, _, _, joined = self.run_kinds(seams, matched, [[k.FAKE, k.CONTRADICTED, "world like"],
                                                          [k.FAIL, k.STANDS, ""]])
        res = {j["test"]: j["result"] for j in joined}
        self.assertEqual(res["world like"], k.RECORD_STANDS)
        self.assertEqual(res["harness like"], k.RECORD_STANDS)
        self.assertEqual(res["nothing only"], mr.FLAGGED)

    def test_changed_file_column(self):
        repo = os.path.join(self.copy, "repo")
        os.makedirs(os.path.join(repo, "tests"))

        def git(*a):
            return subprocess.run(["git", "-C", repo, "-c", "user.name=t", "-c", "user.email=t@t", *a],
                                  capture_output=True, text=True, check=True).stdout.strip()
        def put(name, body):
            with open(os.path.join(repo, "tests", name), "w", encoding="utf-8") as f:
                f.write(body)
        git("init", "-q")
        for name in ("test_a.cpp", "test_b.cpp"):
            put(name, "one\n")
        git("add", "tests/test_a.cpp", "tests/test_b.cpp")
        git("commit", "-q", "-m", "base")
        base = git("rev-parse", "HEAD")
        put("test_a.cpp", "two\n")
        git("commit", "-q", "-am", "change")
        path, n = self.at["getter"]
        seams = [seam_row("a", "tests/test_a.cpp", path, n), seam_row("b", "tests/test_b.cpp", path, n)]
        _, _, _, joined = self.run_kinds(seams, [], repo=repo, at="HEAD", base=base)
        col = "file changed since " + k.BASE
        self.assertEqual({j["test"]: j[col] for j in joined}, {"a": "yes", "b": "no"})


if __name__ == "__main__":
    unittest.main()
