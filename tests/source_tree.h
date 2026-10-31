// The repo's own source tree, as the tests that read it see it. This is the
// one place a test reads HYDRA_SOURCE_DIR (the repo root CMakeLists.txt passes
// in) and the one place a test lists the repo's files: the code under the
// walked folders (source_files) and the markdown files (markdown_files).
// The scans in tests/single_owner/ and the docs check in
// tests/test_docs_match_code.cpp both come here, so a third test that reads
// the tree calls these instead of growing its own walker.
//
// Both lists come from git (git_listed), not from the disk. A folder git
// ignores, such as a node_modules an npm install put under tools/, holds
// vendored code the repo never ships; a disk walk read it and failed the
// scans on lines nobody here wrote.
#pragma once

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "precheck/git_text.h"

#ifndef HYDRA_SOURCE_DIR
#error "HYDRA_SOURCE_DIR must be defined (see CMakeLists.txt)"
#endif

namespace sourcetree {

// The repo root.
inline std::filesystem::path root() { return std::filesystem::u8path(HYDRA_SOURCE_DIR); }

// The top folders the walk below reads, in the order it reads them.
inline constexpr const char* kWalkedFolders[] = {"src", "tools", "tests"};

// Whether the walk below visits the repo-relative path `rel` (forward
// slashes), judged by its top folder.
inline bool walk_visits(const std::string& rel) {
    const std::string top = rel.substr(0, rel.find('/'));
    if (top.size() == rel.size()) return false;  // a file at the repo root
    for (const char* sub : kWalkedFolders)
        if (top == sub) return true;
    return false;
}

// The repo-relative paths (forward slashes) that git tracks or would add and
// that match the git pathspec `pathspec`. A new file nobody has added yet is
// in the list, so a scan sees it before its commit; a file git ignores never
// is. In byte order, each once. Throws std::runtime_error naming `root` when
// git fails there.
inline std::vector<std::string> git_listed(const std::filesystem::path& root, const std::string& pathspec) {
    const std::string where = root.u8string();
    std::vector<std::string> lines;
    try {
        // -z: names end in a NUL and come unquoted, so a space, a quote or a
        // non-ASCII byte in a name arrives as written.
        lines = precheck::run_git(
            where, {"ls-files", "-z", "--cached", "--others", "--exclude-standard", "--", pathspec});
    } catch (const precheck::GitError& e) {
        throw std::runtime_error("the source walk could not ask git for the files in " + where + ": " +
                                 e.what());
    }
    // run_git splits on line breaks; a name may hold one, so join them back.
    // (It also drops a "\r" before each break, so a name holding "\r\n"
    // comes back altered, names no file, and is skipped as not a file.)
    std::string joined;
    for (size_t i = 0; i < lines.size(); ++i) joined += (i ? "\n" : "") + lines[i];
    std::vector<std::string> out;
    for (size_t at = 0; at < joined.size();) {
        size_t end = joined.find('\0', at);
        if (end == std::string::npos) end = joined.size();
        out.push_back(joined.substr(at, end - at));
        at = end + 1;
    }
    // A conflicted file is listed once per side; it is one file.
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

// Every markdown file in the repo git_listed lists, in byte order. Throws
// std::runtime_error naming `root` when git fails there or lists nothing, so
// a scan can never pass by reading no files.
inline std::vector<std::string> markdown_files(const std::filesystem::path& root) {
    std::vector<std::string> out = git_listed(root, "*.md");
    if (out.empty())
        throw std::runtime_error("the source walk found no markdown files git knows in " + root.u8string());
    return out;
}

// The paths git_listed lists under the walked folders. Folder by folder in
// kWalkedFolders order, each in byte order. Throws std::runtime_error naming
// `root` when git fails there or lists nothing, so a scan can never pass by
// reading no files.
inline std::vector<std::string> source_files(const std::filesystem::path& root) {
    const std::string where = root.u8string();
    std::vector<std::string> out;
    for (const char* sub : kWalkedFolders)
        for (std::string& rel : git_listed(root, sub))
            if (walk_visits(rel)) out.push_back(std::move(rel));
    if (out.empty())
        throw std::runtime_error("the source walk found no files git knows under the walked folders in " +
                                 where);
    return out;
}

// Calls visit(path, rel) for every file source_files lists under `root` that
// is a regular file on disk (a tracked file deleted in the work tree is
// skipped), in that order. rel is the file's repo-relative path with forward
// slashes, such as "src/core/winstr.cpp"; its first part is the top folder.
template <typename Visit>
void for_each_source_file(const std::filesystem::path& root, Visit&& visit) {
    namespace fs = std::filesystem;
    for (const std::string& rel : source_files(root)) {
        const fs::path path = root / fs::u8path(rel);
        if (!fs::is_regular_file(path)) continue;
        visit(path, rel);
    }
}

// The same walk of the repo at root().
template <typename Visit>
void for_each_source_file(Visit&& visit) {
    for_each_source_file(sourcetree::root(), std::forward<Visit>(visit));
}

}  // namespace sourcetree
