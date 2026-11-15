// A library file in the temp folder and a save group held on another thread,
// for the store tests that open a real file and check a read does not wait on
// the store's lock. test_store_records.cpp and test_store_library.cpp both use
// them, so they live here once.
#ifndef HYDRA_TESTS_STORE_FILE_FIXTURES_H
#define HYDRA_TESTS_STORE_FILE_FIXTURES_H

#include <atomic>
#include <cstdio>
#include <string>
#include <thread>

#include "store/record_store.h"
#include "temp_util.h"
#include "wait_util.h"  // wait_until, WaitTimeout, kWaitCap

namespace hydra::test {

// A library file in the temp folder, with its -wal and -shm files, removed
// when it is made (so a leftover from a crashed run does not confuse the
// test) and again when the case ends.
struct StoreFile {
    std::string path;
    explicit StoreFile(const char* name) : path(testtemp::temp_path(name, ".db")) {
        remove_all();
    }
    ~StoreFile() { remove_all(); }
    StoreFile(const StoreFile&) = delete;
    StoreFile& operator=(const StoreFile&) = delete;
    void remove_all() const {
        for (const char* ext : {"", "-wal", "-shm"}) std::remove((path + ext).c_str());
    }
};

// A save group a helper thread opens on `store` and holds, so the store's
// lock is taken on another thread, until the test lets go or kWaitCap passes.
// The constructor returns once the group is open.
class HeldSaveGroup {
public:
    explicit HeldSaveGroup(store::RecordStore& store)
        : helper_([this, &store] {
              try {
                  store.begin_save_group();
              } catch (...) {
                  return;  // never held: the constructor's wait gives up
              }
              held_ = true;
              try {
                  testwait::wait_until([this] { return let_go_.load(); },
                                       "the test to let go of the save group");
              } catch (const testwait::WaitTimeout&) {
              }
              held_ = false;
              try {
                  store.commit_save_group();
              } catch (...) {
              }
          }) {
        try {
            testwait::wait_until([this] { return held_.load(); }, "the helper's save group");
        } catch (...) {
            let_go();
            throw;
        }
    }
    ~HeldSaveGroup() { let_go(); }
    HeldSaveGroup(const HeldSaveGroup&) = delete;
    HeldSaveGroup& operator=(const HeldSaveGroup&) = delete;

    // Whether the helper still holds the group.
    bool held() const { return held_.load(); }
    // Lets the group go and waits for the helper to commit it.
    void let_go() {
        let_go_ = true;
        if (helper_.joinable()) helper_.join();
    }

private:
    std::atomic<bool> held_{false};
    std::atomic<bool> let_go_{false};
    std::thread helper_;
};

}  // namespace hydra::test

#endif  // HYDRA_TESTS_STORE_FILE_FIXTURES_H
