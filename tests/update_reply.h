// The updater's fixtures that need no test framework, so the doctest tests
// and the GUI tests (hydra_uitest) read testdata/update/ the same way:
// where it is, the canned release reply, the committed test key, and the
// files saved from GitHub unedited.
// update_fixtures.h adds what only doctest tests use.

#ifndef HYDRA_TESTS_UPDATE_REPLY_H
#define HYDRA_TESTS_UPDATE_REPLY_H

#include <array>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
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

// The v2.10.2 reply in latest.json with its tag set to `tag`, and its
// installer asset describing sample.bin (size and digest), so a download of
// sample.bin passes the digest check. latest.json is not GitHub's reply byte
// for byte: its signature asset was typed in by hand. GitHub's own replies
// are under real_dir().
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

// testdata/update/real: files saved from GitHub unedited. Its README.md gives
// each file's command, date and status code.
inline std::string real_dir() { return update_dir() + "/real"; }

// GitHub's releases/latest reply, as saved.
inline std::string real_reply_text() { return hydra::read_file_text(real_dir() + "/latest.json"); }

// The body GitHub sends, with status 404, for a release that does not exist.
inline std::string real_not_found_text() {
    return hydra::read_file_text(real_dir() + "/not-found.json");
}

// The .sig of an earlier release: a real signature, of a different file.
inline std::string other_release_sig_path() { return real_dir() + "/other-release.sig"; }

// The saved file of the asset whose download address in the saved reply is
// `url`, found under the asset's own name.
inline std::string real_asset_path(const std::string& url) {
    const nlohmann::json reply = nlohmann::json::parse(real_reply_text());
    for (const nlohmann::json& asset : reply.at("assets"))
        if (asset.value("browser_download_url", std::string()) == url)
            return real_dir() + "/" + asset.at("name").get<std::string>();
    throw std::runtime_error("the saved reply " + real_dir() + "/latest.json has no asset at " + url);
}

// The saved reply with its tag_name value set to `tag`, every other byte as
// GitHub sent it. Throws when the reply does not hold "tag_name" exactly once
// as a key with a string value.
inline std::string real_reply_at(const std::string& tag) {
    std::string text = real_reply_text();
    const std::string old_value =
        nlohmann::json(nlohmann::json::parse(text).at("tag_name").get<std::string>()).dump();
    const std::string key = "\"tag_name\"";
    const size_t key_at = text.find(key);
    if (key_at == std::string::npos || text.find(key, key_at + 1) != std::string::npos)
        throw std::runtime_error("the saved reply does not hold \"tag_name\" exactly once");
    // Between the key and its value only white space and one colon may sit.
    size_t at = key_at + key.size();
    bool colon = false;
    for (; at < text.size(); ++at) {
        const char c = text[at];
        if (c == ':' && !colon) {
            colon = true;
            continue;
        }
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') break;
    }
    if (!colon || text.compare(at, old_value.size(), old_value) != 0)
        throw std::runtime_error("the saved reply's \"tag_name\" is not followed by its value");
    text.replace(at, old_value.size(), nlohmann::json(tag).dump());
    return text;
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
