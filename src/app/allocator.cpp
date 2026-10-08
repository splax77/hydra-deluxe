#include "app/allocator.h"

#include <mimalloc.h>

namespace hydra::app {

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

}  // namespace hydra::app
