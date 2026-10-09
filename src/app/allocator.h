// The calls Hydra makes into its memory allocator, mimalloc (D88), by name.
// Everything else about mimalloc (which exes run on it, its options) is
// decided in CMakeLists.txt.

#ifndef HYDRA_APP_ALLOCATOR_H
#define HYDRA_APP_ALLOCATOR_H

#include <cstddef>

namespace hydra::app {

// Hands the memory mimalloc keeps after it is freed back to Windows at once,
// instead of after mimalloc's purge delay (D95 call 1), for a job that has
// just freed a lot at once. It takes some ms, so it does not belong in a loop.
void return_freed_memory();

// Committed memory in bytes as mimalloc's mi_process_info reports it, for
// tests and measurements.
size_t committed_bytes();

// Whether this process's malloc and free really go to mimalloc. False when
// mimalloc.dll loads after another DLL that allocates (the exe's link order,
// CMakeLists.txt), or under the Debug runtime, which mimalloc-redirect.dll
// doesn't patch. For tests.
bool malloc_redirected();

}  // namespace hydra::app

#endif  // HYDRA_APP_ALLOCATOR_H
