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
// vendored code the repo never ships, so no test reads it. Each caller picks
// one of two lists (Listing below). The single-owner and clone scans read
// committed and new files, so they catch a copied rule in a file before its
// first commit. The docs check reads committed and staged files only, so a
// draft nobody has added to git never fails it.
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

// Which files a listing below holds.
enum class Listing {
    // The files git tracks (committed or staged) and the new files it would
    // add. The single-owner and clone scans read this list.
    CommittedAndNew,
    // The files git tracks (committed or staged) and nothing else. The docs
    // check in tests/test_docs_match_code.cpp reads this list.
    CommittedOnly,
};

// The repo-relative paths (forward slashes) in the listing `which` that match
// the git pathspec `pathspec`. A file git ignores is in neither listing. In
// byte order, each once. Throws std::runtime_error naming `root` when git
// fails there.
inline std::vector<std::string> git_listed(const std::filesystem::path& root, const std::string& pathspec,
                                           Listing which = Listing::CommittedAndNew) {
    const std::string where = root.u8string();
    std::vector<std::string> lines;
    try {
        // -z: names end in a NUL and come unquoted, so a space, a quote or a
        // non-ASCII byte in a name arrives as written.
        std::vector<std::string> args = {"ls-files", "-z", "--cached"};
        if (which == Listing::CommittedAndNew) {
            args.push_back("--others");
            args.push_back("--exclude-standard");
        }
        args.push_back("--");
        args.push_back(pathspec);
        lines = precheck::run_git(where, args);
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

// Every markdown file in the repo in the listing `which`, in byte order.
// Throws std::runtime_error naming `root` when git fails there or lists
// nothing, so a scan can never pass by reading no files.
inline std::vector<std::string> markdown_files(const std::filesystem::path& root,
                                               Listing which = Listing::CommittedAndNew) {
    std::vector<std::string> out = git_listed(root, "*.md", which);
    if (out.empty())
        throw std::runtime_error("the source walk found no markdown files git knows in " + root.u8string());
    return out;
}

// The paths in the listing `which` under the walked folders. Folder by folder
// in kWalkedFolders order, each in byte order. Throws std::runtime_error
// naming `root` when git fails there or lists nothing, so a scan can never
// pass by reading no files.
inline std::vector<std::string> source_files(const std::filesystem::path& root,
                                             Listing which = Listing::CommittedAndNew) {
    const std::string where = root.u8string();
    std::vector<std::string> out;
    for (const char* sub : kWalkedFolders)
        for (std::string& rel : git_listed(root, sub, which))
            if (walk_visits(rel)) out.push_back(std::move(rel));
    if (out.empty())
        throw std::runtime_error("the source walk found no files git knows under the walked folders in " +
                                 where);
    return out;
}

// Calls visit(path, rel) for every file source_files lists under `root` in
// the listing `which` that is a regular file on disk (a tracked file deleted
// in the work tree is skipped), in that order. rel is the file's
// repo-relative path with forward slashes, such as "src/core/winstr.cpp"; its
// first part is the top folder.
template <typename Visit>
void for_each_source_file(const std::filesystem::path& root, Listing which, Visit&& visit) {
    namespace fs = std::filesystem;
    for (const std::string& rel : source_files(root, which)) {
        const fs::path path = root / fs::u8path(rel);
        if (!fs::is_regular_file(path)) continue;
        visit(path, rel);
    }
}

// The same walk of the committed and new files.
template <typename Visit>
void for_each_source_file(const std::filesystem::path& root, Visit&& visit) {
    for_each_source_file(root, Listing::CommittedAndNew, std::forward<Visit>(visit));
}

// The same walk of the repo at root().
template <typename Visit>
void for_each_source_file(Visit&& visit) {
    for_each_source_file(sourcetree::root(), std::forward<Visit>(visit));
}

}  // namespace sourcetree
