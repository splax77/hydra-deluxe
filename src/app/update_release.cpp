#include "app/update_release.h"

#include <optional>
#include <string_view>
#include <vector>

#include "core/error_kind.h"
#include "core/strutil.h"  // starts_with, ends_with
#include "json.hpp"

namespace hydra::app {

namespace {

using json = nlohmann::json;

constexpr std::string_view kInstallerSuffix = "-setup.exe";
constexpr std::string_view kSignatureSuffix = "-setup.exe.sig";
constexpr std::string_view kDigestPrefix = "sha256:";

[[noreturn]] void refuse(const std::string& what) {
    throw KindedError(ErrorKind::UpdateUnverified, "the release on GitHub " + what);
}

bool is_hex(std::string_view s) {
    for (char c : s) {
        const bool digit = c >= '0' && c <= '9';
        const bool lower = c >= 'a' && c <= 'f';
        const bool upper = c >= 'A' && c <= 'F';
        if (!digit && !lower && !upper) return false;
    }
    return true;
}

// The string field `key` of `obj`, empty when it is missing or not a string.
std::string string_field(const json& obj, const char* key) {
    const auto it = obj.find(key);
    return it != obj.end() && it->is_string() ? it->get<std::string>() : std::string();
}

// The one asset whose name ends in `suffix`; refuses none or more than one.
const json& one_asset(const json& assets, std::string_view suffix) {
    const json* found = nullptr;
    int count = 0;
    for (const json& asset : assets) {
        if (!asset.is_object()) continue;
        if (!ends_with(string_field(asset, "name"), suffix)) continue;
        found = &asset;
        ++count;
    }
    if (count == 0) refuse("has no asset named *" + std::string(suffix));
    if (count > 1) refuse("has " + std::to_string(count) + " assets named *" + std::string(suffix));
    return *found;
}

UpdateRelease read_release(const json& root) {
    if (!root.is_object()) refuse("sent a reply that is not a JSON object");
    UpdateRelease out;

    const std::string tag = string_field(root, "tag_name");
    const std::optional<core::AppVersion> version = core::parse_version(tag);
    if (!version) refuse("has a tag that is not a version: \"" + tag + "\"");
    out.version = *version;
    out.notes = string_field(root, "body");

    const auto assets = root.find("assets");
    if (assets == root.end() || !assets->is_array()) refuse("has no asset list");

    const json& installer = one_asset(*assets, kInstallerSuffix);
    const std::string name = string_field(installer, "name");
    out.installer_url = string_field(installer, "browser_download_url");
    if (out.installer_url.empty()) refuse("has no download address for " + name);
    const auto size = installer.find("size");
    if (size == installer.end() || !size->is_number_unsigned())
        refuse("has no size for " + name);
    out.installer_size = size->get<uint64_t>();

    const auto digest_field = installer.find("digest");
    if (digest_field == installer.end() || !digest_field->is_string())
        refuse("has no digest for " + name);
    const std::string digest = digest_field->get<std::string>();
    if (!starts_with(digest, kDigestPrefix))
        refuse("has a digest for " + name + " that is not SHA-256: \"" + digest + "\"");
    out.digest = digest.substr(kDigestPrefix.size());
    if (out.digest.size() != 64 || !is_hex(out.digest))
        refuse("has a SHA-256 digest for " + name + " that is not 64 hex digits: \"" + digest +
               "\"");

    const json& signature = one_asset(*assets, kSignatureSuffix);
    out.sig_url = string_field(signature, "browser_download_url");
    if (out.sig_url.empty())
        refuse("has no download address for " + string_field(signature, "name"));
    return out;
}

}  // namespace

UpdateRelease parse_latest_release_json(const std::string& body) {
    json root;
    try {
        root = json::parse(body);
    } catch (const json::exception&) {
        refuse("sent a reply that is not JSON");
    }
    return read_release(root);
}

}  // namespace hydra::app
