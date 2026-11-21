// A helper that holds the record store's lock from another thread, for the
// tests that check a batch or a tick does not wait on it.
#ifndef HYDRA_TESTS_STORE_LOCK_HOLDER_H
#define HYDRA_TESTS_STORE_LOCK_HOLDER_H

#include "store/record_store.h"
#include "store_file_fixtures.h"  // HeldSaveGroup

namespace hydra::test {

// Holds the store's lock from a thread of its own, the way a batch's open
// save group does, until release() or kWaitCap, whichever comes first, so a
// broken build fails its checks instead of hanging. The holding is
// HeldSaveGroup's, the one helper that opens a save group on another thread
// and catches that thread's errors; this class only names the two questions
// the no-wait tests ask of it.
class StoreLockHolder {
public:
    explicit StoreLockHolder(store::RecordStore& store) : group_(store) {}
    StoreLockHolder(const StoreLockHolder&) = delete;
    StoreLockHolder& operator=(const StoreLockHolder&) = delete;

    // Lets the lock go and waits for the helper to commit its group.
    void release() { group_.let_go(); }
    // True once the helper has let the lock go (or is about to).
    bool let_go() const { return !group_.held(); }

private:
    HeldSaveGroup group_;
};

// Runs call() while a save group holds the store's lock, then lets the lock
// go. True when call() came back before the holder let the lock go, that is,
// when it did not wait for the lock.
template <typename Call>
bool returns_while_held(store::RecordStore& store, Call&& call) {
    StoreLockHolder holder(store);
    call();
    const bool returned_while_held = !holder.let_go();
    holder.release();
    return returned_while_held;
}

}  // namespace hydra::test

#endif  // HYDRA_TESTS_STORE_LOCK_HOLDER_H
