// The repo's own source tree, as the tests that read it see it. This is the
// one place a test reads HYDRA_SOURCE_DIR (the repo root CMakeLists.txt passes
// in) and the one place a test walks src/, tools/ and tests/. The scans in
// tests/single_owner/ and the docs check in
// tests/test_docs_match_code.cpp both come here, so a third test that reads
// the tree calls these instead of growing its own walker.
#pragma once

#include <filesystem>
#include <string>

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

// Calls visit(path, rel) for every regular file under the walked folders, in
// order. rel is the file's repo-relative path with forward slashes, such as
// "src/core/winstr.cpp"; its first part is the top folder.
template <typename Visit>
void for_each_source_file(Visit&& visit) {
    namespace fs = std::filesystem;
    const fs::path root = sourcetree::root();
    for (const std::string sub : kWalkedFolders) {
        for (const fs::directory_entry& e : fs::recursive_directory_iterator(root / sub)) {
            if (!e.is_regular_file()) continue;
            const std::string rel = fs::relative(e.path(), root).generic_u8string();
            visit(e.path(), rel);
        }
    }
}

}  // namespace sourcetree
