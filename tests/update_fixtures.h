// The updater's checked-in fixtures in testdata/update/, read the same way by
// every updater test (test_update_verify.cpp, test_update_state.cpp). The
// parts that need no doctest live in update_reply.h, which hydra_uitest also
// reads.

#ifndef HYDRA_TESTS_UPDATE_FIXTURES_H
#define HYDRA_TESTS_UPDATE_FIXTURES_H

#include "doctest.h"

#include <array>
#include <cstdint>

#include "update_reply.h"  // update_dir, sample_path, release_reply_at, read_test_public_key

namespace update_fixtures {

// test-public-key.txt as bytes; the test fails when the file is malformed.
inline std::array<uint8_t, 64> test_public_key() {
    std::array<uint8_t, 64> key{};
    REQUIRE(read_test_public_key(key));
    return key;
}

}  // namespace update_fixtures

#endif  // HYDRA_TESTS_UPDATE_FIXTURES_H
