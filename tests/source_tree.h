// The repo's own source tree, as the tests that read it see it. This is the
// one place a test reads HYDRA_SOURCE_DIR (the repo root CMakeLists.txt passes
// in) and the one place a test walks src/, tools/ and tests/. The scan in
// tests/test_single_owner.cpp and the docs check in
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

// Calls visit(path, rel) for every regular file under src/, tools/ and tests/,
// in that order. rel is the file's repo-relative path with forward slashes,
// such as "src/core/winstr.cpp"; its first part is the top folder.
template <typename Visit>
void for_each_source_file(Visit&& visit) {
    namespace fs = std::filesystem;
    const fs::path root = sourcetree::root();
    for (const std::string sub : {"src", "tools", "tests"}) {
        for (const fs::directory_entry& e : fs::recursive_directory_iterator(root / sub)) {
            if (!e.is_regular_file()) continue;
            const std::string rel = fs::relative(e.path(), root).generic_u8string();
            visit(e.path(), rel);
        }
    }
}

}  // namespace sourcetree
