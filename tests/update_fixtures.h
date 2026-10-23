// The updater's checked-in fixtures in testdata/update/, read the same way by
// every updater test (test_update_verify.cpp, test_update_state.cpp).

#ifndef HYDRA_TESTS_UPDATE_FIXTURES_H
#define HYDRA_TESTS_UPDATE_FIXTURES_H

#include "doctest.h"

#include <array>
#include <cstdint>
#include <string>

#include "core/winstr.h"  // read_file_text
#include "testdata_dirs.h"

namespace update_fixtures {

// testdata/update, the folder that holds the fixtures below.
inline std::string update_dir() { return corpus::testdata_root() + "/update"; }

// sample.bin, the file the committed test signature sample.bin.sig signs.
inline std::string sample_path() { return update_dir() + "/sample.bin"; }

// test-public-key.txt: the 64 key bytes as hex, as release_keygen.ps1 wrote them.
inline std::array<uint8_t, 64> test_public_key() {
    std::string text = hydra::read_file_text(update_dir() + "/test-public-key.txt");
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) text.pop_back();
    REQUIRE(text.size() == 128);
    std::array<uint8_t, 64> key{};
    for (size_t i = 0; i < key.size(); ++i)
        key[i] = static_cast<uint8_t>(std::stoul(text.substr(2 * i, 2), nullptr, 16));
    return key;
}

}  // namespace update_fixtures

#endif  // HYDRA_TESTS_UPDATE_FIXTURES_H
