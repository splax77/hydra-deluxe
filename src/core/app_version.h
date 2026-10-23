// An app version as three whole numbers, and the one place two versions are
// compared. The updater asks "is this release newer than the running app"
// through parse_version and operator<, and the running app's own version
// (HYDRA_VERSION) is read by the same parse, so both sides follow one rule
// (docs/superpowers/plans/2026-10-22-auto-updater.md, "The version rule").

#ifndef HYDRA_CORE_APP_VERSION_H
#define HYDRA_CORE_APP_VERSION_H

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>

#include "core/version.h"  // HYDRA_VERSION

namespace hydra::core {

struct AppVersion {
    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t patch = 0;

    friend bool operator==(const AppVersion& a, const AppVersion& b) {
        return std::tie(a.major, a.minor, a.patch) == std::tie(b.major, b.minor, b.patch);
    }
    friend bool operator!=(const AppVersion& a, const AppVersion& b) { return !(a == b); }
    // Older first: the numbers are compared in order, as numbers, so 2.9.1
    // is older than 2.10.2.
    friend bool operator<(const AppVersion& a, const AppVersion& b) {
        return std::tie(a.major, a.minor, a.patch) < std::tie(b.major, b.minor, b.patch);
    }
};

// "2.10.2" or "v2.10.2" as an AppVersion. Empty for anything else: fewer or
// more than three parts, an empty part, a sign, a letter, or a number too big
// to hold.
inline std::optional<AppVersion> parse_version(std::string_view text) {
    if (!text.empty() && text.front() == 'v') text.remove_prefix(1);
    uint32_t parts[3] = {0, 0, 0};
    size_t pos = 0;
    for (int i = 0; i < 3; ++i) {
        if (i > 0) {
            if (pos >= text.size() || text[pos] != '.') return std::nullopt;
            ++pos;
        }
        const size_t start = pos;
        uint64_t value = 0;
        while (pos < text.size() && text[pos] >= '0' && text[pos] <= '9') {
            value = value * 10 + static_cast<uint64_t>(text[pos] - '0');
            if (value > UINT32_MAX) return std::nullopt;
            ++pos;
        }
        if (pos == start) return std::nullopt;
        parts[i] = static_cast<uint32_t>(value);
    }
    if (pos != text.size()) return std::nullopt;
    return AppVersion{parts[0], parts[1], parts[2]};
}

// Whether `candidate` is strictly newer than `current`: an equal or older
// version is never newer.
inline bool is_newer_version(const AppVersion& candidate, const AppVersion& current) {
    return current < candidate;
}

// The version as the user reads it, "2.10.2" (no leading v).
inline std::string version_text(const AppVersion& v) {
    return std::to_string(v.major) + "." + std::to_string(v.minor) + "." +
           std::to_string(v.patch);
}

// The running app's version, read from HYDRA_VERSION by parse_version. The
// build sets HYDRA_VERSION from CMakeLists.txt's project version; one that
// does not parse is a build mistake, so this throws rather than guess.
inline AppVersion running_version() {
    const std::optional<AppVersion> v = parse_version(HYDRA_VERSION);
    if (!v) throw std::logic_error("HYDRA_VERSION is not a version: " HYDRA_VERSION);
    return *v;
}

}  // namespace hydra::core

#endif  // HYDRA_CORE_APP_VERSION_H
