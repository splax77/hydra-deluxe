// A leak check for one doctest case, from the memory audit
// (docs/handoffs/2026-10-07-memory-audit.md, recommendation 8).
//
// Wrap a TEST_CASE's body: hydra::test::leak_checked([&] { ... });
// In a Debug build the body runs twice. The first run is a warm-up, so
// function-local statics and a library's one-time setup (the corpus path
// list, SQLite's start-up) are made before anything is counted. The second
// run is measured on the CRT debug heap, and the case fails if more blocks
// are alive after it than before, printing each block it left behind. In a
// Release build the body runs once and nothing is checked.
//
// Run it with mimalloc's redirect off (MIMALLOC_DISABLE_REDIRECT=1). With the
// redirect on, mimalloc serves every allocation and the CRT debug heap sees
// none of them, so the check refuses to run rather than pass blind.

#ifndef HYDRA_TESTS_LEAK_CHECK_H
#define HYDRA_TESTS_LEAK_CHECK_H

#include "doctest.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

namespace hydra::test {

#if defined(_MSC_VER) && defined(_DEBUG)

// Can the CRT debug heap see this process's allocations at all?
inline bool debug_heap_sees_allocations() {
    _CrtMemState before, after, diff;
    _CrtMemCheckpoint(&before);
    char* probe = new char(0);
    _CrtMemCheckpoint(&after);
    const bool seen = _CrtMemDifference(&diff, &before, &after) != 0;
    delete probe;
    return seen;
}

template <typename Body>
void leak_checked(Body&& body) {
    if (!debug_heap_sees_allocations()) {
        FAIL_CHECK("the CRT debug heap sees no allocations; "
                   "run with MIMALLOC_DISABLE_REDIRECT=1");
        body();
        return;
    }
    body();  // warm-up
    _CrtMemState before, after, diff;
    _CrtMemCheckpoint(&before);
    body();
    _CrtMemCheckpoint(&after);
    if (!_CrtMemDifference(&diff, &before, &after)) return;
    // Blocks the body freed that were made before it are not leaks; only
    // more live blocks than before are.
    const long left = static_cast<long>(diff.lCounts[_NORMAL_BLOCK] + diff.lCounts[_CLIENT_BLOCK]);
    if (left <= 0) return;
    const int old_mode = _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    const _HFILE old_file = _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtMemDumpAllObjectsSince(&before);
    _CrtSetReportFile(_CRT_WARN, old_file);
    _CrtSetReportMode(_CRT_WARN, old_mode);
    FAIL_CHECK("the test left " << left << " heap block(s) alive (listed above)");
}

#else

template <typename Body>
void leak_checked(Body&& body) {
    body();
}

#endif

}  // namespace hydra::test

#endif  // HYDRA_TESTS_LEAK_CHECK_H
