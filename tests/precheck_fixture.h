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

}  // namespace precheck_fixture
