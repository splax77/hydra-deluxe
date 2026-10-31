// What GitHub's "latest release" reply says about an update, read once with
// the refusal rules in docs/development.md, "The version rule" (D259). A
// reply that lacks a
// piece the update needs is refused here, before anything is downloaded.
//
// It lives in hydra_dm with the other GUI-only network code, so the
// command-line tools never link it.

#ifndef HYDRA_APP_UPDATE_RELEASE_H
#define HYDRA_APP_UPDATE_RELEASE_H

#include <cstdint>
#include <string>

#include "core/app_version.h"

namespace hydra::app {

struct UpdateRelease {
    core::AppVersion version;  // the release's tag, read by core::parse_version
    std::string notes;         // the release's notes as GitHub holds them; empty when none
    std::string installer_url;
    uint64_t installer_size = 0;  // the installer asset's stated size in bytes
    std::string digest;           // the installer's SHA-256 as hex, without the "sha256:" prefix
    std::string sig_url;          // the installer's signature asset
};

// The release in GitHub's releases/latest JSON. Throws
// KindedError(ErrorKind::UpdateUnverified) when the reply can't be read or
// lacks a piece the update needs; the error's text names the piece.
UpdateRelease parse_latest_release_json(const std::string& body);

}  // namespace hydra::app

#endif  // HYDRA_APP_UPDATE_RELEASE_H
