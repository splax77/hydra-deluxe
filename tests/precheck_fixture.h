// The fixture repos of the hydra_precheck tests (tests/test_precheck_*.cpp):
// a tiny git repo in a temp folder, written and committed file by file. This
// is the one home of those helpers.
#pragma once

#include "doctest.h"

#include <cctype>
#include <cstdint>
#include <filesystem>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "core/winstr.h"
#include "precheck/git_text.h"
#include "temp_util.h"

namespace precheck_fixture {

// Writes `text` to `rel` under `dir`, making its folders.
inline void write_fixture_file(const std::string& dir, const std::string& rel, const std::string& text) {
    namespace fs = std::filesystem;
    const fs::path path = fs::path(hydra::os_path(dir)) / fs::u8path(rel);
    fs::create_directories(path.parent_path());
    testtemp::write_bytes(path.u8string(), std::vector<uint8_t>(text.begin(), text.end()));
}

// Runs git in `dir` with a fixed author and no line-ending conversion.
inline void fixture_git(const std::string& dir, std::vector<std::string> args) {
    std::vector<std::string> full = {"-c", "user.name=precheck", "-c", "user.email=precheck@example.invalid",
                                     "-c", "core.autocrlf=false"};
    full.insert(full.end(), args.begin(), args.end());
    precheck::run_git(dir, full);
}

// Commits everything in `dir` on the current branch.
inline void commit_all(const std::string& dir, const std::string& message) {
    fixture_git(dir, {"add", "-A"});
    fixture_git(dir, {"commit", "-q", "-m", message});
}

// Writes each file under `dir`, then commits everything.
inline void commit_texts(const std::string& dir, const std::map<std::string, std::string>& texts,
                         const std::string& message) {
    for (const auto& [rel, text] : texts) write_fixture_file(dir, rel, text);
    commit_all(dir, message);
}

// The item lines (kind letter, a space, the rest) hydra_precheck prints for
// main...feature in `repo` with the checks named in `disable` ("2,3,4,5")
// turned off. Fails the test when the exe exits non-zero.
inline std::vector<std::string> exe_item_lines(const std::string& repo, const std::string& disable) {
    precheck::ChildProcess p({HYDRA_PRECHECK_EXE, "--repo", repo, "--range", "main...feature", "--disable", disable});
    const std::string out = p.read_all();
    std::string err;
    const int code = p.wait(err);
    INFO(err);
    CHECK(code == 0);
    std::vector<std::string> lines;
    std::istringstream in(out);
    for (std::string l; std::getline(in, l);) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        if (l.size() > 2 && l[1] == ' ' && std::isupper(static_cast<unsigned char>(l[0]))) lines.push_back(l);
    }
    return lines;
}

// A fixture repo whose main branch holds `base`; commit_tip() commits the tip
// on top, so the range "HEAD" is that one commit.
struct FixtureRepo {
    std::string dir;
    FixtureRepo(const std::string& name, const std::map<std::string, std::string>& base)
        : dir(testtemp::temp_dir(name)) {
        namespace fs = std::filesystem;
        testtemp::remove_dir(dir);
        fs::create_directories(hydra::os_path(dir));
        fixture_git(dir, {"init", "-q", "-b", "main"});
        commit_texts(dir, base, "base");
    }
    ~FixtureRepo() { testtemp::remove_dir(dir); }
    void commit_tip(const std::map<std::string, std::string>& tip) const { commit_texts(dir, tip, "tip"); }
    // Deletes `rel` and commits that alone.
    void commit_delete(const std::string& rel) const {
        namespace fs = std::filesystem;
        fs::remove(fs::path(hydra::os_path(dir)) / fs::u8path(rel));
        commit_all(dir, "delete");
    }
};

// What one run of the hydra_precheck exe printed, as raw text: its stdout,
// its stderr and its exit code.
struct ExeOutput {
    int exit_code = -1;
    std::string out;
    std::string err;
};

// Runs the exe with `args` after its path.
inline ExeOutput run_exe(const std::vector<std::string>& args) {
    std::vector<std::string> full = {HYDRA_PRECHECK_EXE};
    full.insert(full.end(), args.begin(), args.end());
    precheck::ChildProcess p(full);
    ExeOutput r;
    r.out = p.read_all();
    r.exit_code = p.wait(r.err);
    return r;
}

// What one run of the hydra_precheck exe printed: its stdout lines, its
// stderr and its exit code.
struct ExeRun {
    std::vector<std::string> lines;
    std::string err;
    int code = -1;
};

// Runs the exe as `--repo <repo> <mode> <range>` (mode is "--tests" or
// "--changed-tests").
inline ExeRun run_exe_mode(const std::string& repo, const std::string& mode, const std::string& range) {
    const ExeOutput o = run_exe({"--repo", repo, mode, range});
    ExeRun r;
    std::istringstream text(o.out);
    for (std::string l; std::getline(text, l);) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        r.lines.push_back(l);
    }
    r.err = o.err;
    r.code = o.exit_code;
    return r;
}

}  // namespace precheck_fixture
