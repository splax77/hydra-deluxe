// The two calls Hydra makes into its memory allocator, mimalloc (D88), by
// name. Everything else about mimalloc (which exes run on it, its options) is
// decided in CMakeLists.txt.

#ifndef HYDRA_APP_ALLOCATOR_H
#define HYDRA_APP_ALLOCATOR_H

#include <cstddef>

namespace hydra::app {

// Hands the memory mimalloc keeps after it is freed back to Windows at once,
// instead of after mimalloc's purge delay (D95 call 1). Library batches call
// it once when they end. It takes some ms, so it does not belong in a loop.
void return_freed_memory();

// Committed memory in bytes as mimalloc's mi_process_info reports it, for
// tests and measurements.
size_t committed_bytes();

}  // namespace hydra::app

#endif  // HYDRA_APP_ALLOCATOR_H
