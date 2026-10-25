// The fixture repos of the hydra_precheck tests (tests/test_precheck_*.cpp):
// a tiny git repo in a temp folder, written and committed file by file. This
// is the one home of those helpers.
#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
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

}  // namespace precheck_fixture
