// The updater's fixtures that need no test framework, so the doctest tests
// and the GUI tests (hydra_uitest) read testdata/update/ the same way:
// where it is, the canned release reply, and the committed test key.
// update_fixtures.h adds what only doctest tests use.

#ifndef HYDRA_TESTS_UPDATE_REPLY_H
#define HYDRA_TESTS_UPDATE_REPLY_H

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>

#include "app/update_verify.h"  // sha256_file
#include "core/winstr.h"        // read_file_text, os_path
#include "json.hpp"
#include "testdata_dirs.h"

namespace update_fixtures {

// testdata/update, the folder that holds the fixtures below.
inline std::string update_dir() { return corpus::testdata_root() + "/update"; }

// sample.bin, the file the committed test signature sample.bin.sig signs.
inline std::string sample_path() { return update_dir() + "/sample.bin"; }

// The real v2.10.2 reply (latest.json) with its tag set to `tag`, and its
// installer asset describing sample.bin (size and digest), so a download of
// sample.bin passes the digest check.
inline std::string release_reply_at(const std::string& tag) {
    nlohmann::json reply = nlohmann::json::parse(hydra::read_file_text(update_dir() + "/latest.json"));
    reply["tag_name"] = tag;
    for (nlohmann::json& asset : reply["assets"]) {
        const std::string name = asset["name"];
        if (name.size() > 4 && name.substr(name.size() - 4) == ".exe") {
            asset["size"] = std::filesystem::file_size(hydra::os_path(sample_path()));
            asset["digest"] = "sha256:" + hydra::app::sha256_file(sample_path());
        }
    }
    return reply.dump();
}

// test-public-key.txt: the 64 key bytes as hex, as release_keygen.ps1 wrote
// them. False (key untouched) when the file is not 128 hex digits.
inline bool read_test_public_key(std::array<uint8_t, 64>& key) {
    std::string text = hydra::read_file_text(update_dir() + "/test-public-key.txt");
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) text.pop_back();
    if (text.size() != 2 * key.size()) return false;
    for (size_t i = 0; i < key.size(); ++i)
        key[i] = static_cast<uint8_t>(std::stoul(text.substr(2 * i, 2), nullptr, 16));
    return true;
}

}  // namespace update_fixtures

#endif  // HYDRA_TESTS_UPDATE_REPLY_H
