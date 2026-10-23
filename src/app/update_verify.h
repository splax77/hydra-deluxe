#pragma once

// The check a downloaded installer has to pass before Hydra runs it. The
// signature scheme (what is signed, with which curve, and in which byte form)
// is written up in docs/superpowers/plans/2026-10-22-auto-updater.md under
// "The signature scheme"; tools/release_sign.ps1 is the side that signs.

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace hydra::app {

// The file's SHA-256 as 64 lowercase hex digits, read in chunks so a large
// installer is never held in memory whole. Throws KindedError(HashFailed)
// when the file can't be read or the hash can't be made.
std::string sha256_file(const std::string& path);

// Returns when the file at `path` is the one the release key signed, and
// throws a KindedError otherwise, its raw text naming the check that failed.
// Two checks run, digest first:
//   `digest_hex` is the release's stated SHA-256 (64 lowercase hex digits, as
//   GitHub's asset `digest` field gives it once its "sha256:" is removed);
//   `sig` is the .sig asset's bytes, an ECDSA signature of the file's SHA-256
//   checked against `public_key` (the production one is kUpdatePublicKey in
//   app/update_key.h; tests pass the test key).
void verify_update(const std::string& path, const std::string& digest_hex,
                   const std::vector<uint8_t>& sig, const std::array<uint8_t, 64>& public_key);

}  // namespace hydra::app
