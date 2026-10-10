#include "app/allocator.h"

#ifdef HYDRA_MIMALLOC
#include <mimalloc.h>
#else
// The build without mimalloc (HYDRA_MIMALLOC in CMakeLists.txt; only the
// AddressSanitizer build) runs on the Windows heap.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#endif

namespace hydra::app {

#ifdef HYDRA_MIMALLOC

void return_freed_memory() {
    // true: purge every arena now, not only what has waited its delay.
    mi_collect(true);
}

size_t committed_bytes() {
    size_t current_commit = 0;
    mi_process_info(nullptr, nullptr, nullptr, nullptr, nullptr, &current_commit, nullptr,
                    nullptr);
    return current_commit;
}

bool malloc_redirected() { return mi_is_redirected(); }

#else

// The Windows heap has no purge delay to cut short.
void return_freed_memory() {}

size_t committed_bytes() {
    PROCESS_MEMORY_COUNTERS counters{};
    counters.cb = sizeof(counters);
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters))) return 0;
    return counters.PagefileUsage;  // the process's private committed bytes
}

bool malloc_redirected() { return false; }

#endif

}  // namespace hydra::app
