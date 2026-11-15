// A helper that holds the record store's lock from another thread, for the
// tests that check a batch or a tick does not wait on it.
#ifndef HYDRA_TESTS_STORE_LOCK_HOLDER_H
#define HYDRA_TESTS_STORE_LOCK_HOLDER_H

#include <atomic>
#include <chrono>
#include <thread>

#include "store/record_store.h"
#include "wait_util.h"

namespace hydra::test {

// Holds the store's lock from a thread of its own, the way a batch's open
// save group does, until release() or kWaitCap, whichever comes first, so a
// broken build fails its checks instead of hanging.
class StoreLockHolder {
public:
    explicit StoreLockHolder(store::RecordStore& store)
        : thread_([this, &store] {
              store.begin_save_group();
              holding_ = true;
              const auto until = std::chrono::steady_clock::now() + testwait::kWaitCap;
              while (!release_.load() && std::chrono::steady_clock::now() < until)
                  std::this_thread::sleep_for(std::chrono::milliseconds(1));
              let_go_ = true;
              store.commit_save_group();
          }) {
        try {
            testwait::wait_until([this] { return holding_.load(); },
                                 "the helper to take the store lock");
        } catch (...) {
            release();
            thread_.join();
            throw;
        }
    }
    ~StoreLockHolder() {
        release();
        thread_.join();
    }
    StoreLockHolder(const StoreLockHolder&) = delete;
    StoreLockHolder& operator=(const StoreLockHolder&) = delete;

    void release() { release_ = true; }
    // True once the helper has let the lock go (or is about to).
    bool let_go() const { return let_go_.load(); }

private:
    std::atomic<bool> holding_{false};
    std::atomic<bool> release_{false};
    std::atomic<bool> let_go_{false};
    std::thread thread_;  // last, so the flags exist before it starts
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
