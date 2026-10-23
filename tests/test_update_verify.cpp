// Tests for app/update_verify.{h,cpp}: the digest and signature check a
// downloaded installer must pass, against the committed test key pair in
// testdata/update/ (made by tools/release_keygen.ps1 -TestKey, with
// sample.bin.sig made by tools/release_sign.ps1). Also pins that the shipped
// key in app/update_key.h is no longer the test key.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

#include "app/update_key.h"
#include "app/update_verify.h"
#include "core/error_kind.h"
#include "core/winstr.h"
#include "temp_util.h"
#include "update_fixtures.h"

using hydra::ErrorKind;
using hydra::KindedError;
using update_fixtures::sample_path;
using update_fixtures::test_public_key;
using update_fixtures::update_dir;

namespace {

// The kind every refusal carries.
constexpr ErrorKind kRefusalKind = ErrorKind::UpdateUnverified;

// sha256_file of testdata/update/sample.bin, from its first run (2026-10-22);
// release_sign.ps1 printed the same digest when it signed the file.
const std::string kSampleSha256 = "339332f7bafa1294c90bfa5259652039caa1d02b5f32b2f266dc2c22849e0966";

std::vector<uint8_t> sample_sig() { return hydra::read_file_bytes(update_dir() + "/sample.bin.sig"); }

// Runs verify_update and returns the kind it refused with, or nothing when it
// passed.
std::optional<ErrorKind> refusal(const std::string& path, const std::string& digest,
                                 const std::vector<uint8_t>& sig,
                                 const std::array<uint8_t, 64>& key) {
    try {
        hydra::app::verify_update(path, digest, sig, key);
        return std::nullopt;
    } catch (const KindedError& e) {
        return e.kind();
    }
}

std::array<uint8_t, 32> hex_to_hash(const std::string& hex) {
    std::array<uint8_t, 32> out{};
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<uint8_t>(std::stoul(hex.substr(2 * i, 2), nullptr, 16));
    return out;
}

// A second, freshly made P-256 key: its public half and its signature of
// `hash`, made with bcrypt so the test needs no second committed key.
struct OtherKey {
    std::array<uint8_t, 64> public_key{};
    std::vector<uint8_t> sig;
};

OtherKey sign_with_new_key(std::array<uint8_t, 32> hash) {
    OtherKey out;
    BCRYPT_ALG_HANDLE alg = nullptr;
    REQUIRE(BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&alg, BCRYPT_ECDSA_P256_ALGORITHM, nullptr, 0)));
    BCRYPT_KEY_HANDLE key = nullptr;
    REQUIRE(BCRYPT_SUCCESS(BCryptGenerateKeyPair(alg, &key, 256, 0)));
    REQUIRE(BCRYPT_SUCCESS(BCryptFinalizeKeyPair(key, 0)));

    std::vector<uint8_t> blob(sizeof(BCRYPT_ECCKEY_BLOB) + 64);
    ULONG got = 0;
    REQUIRE(BCRYPT_SUCCESS(BCryptExportKey(key, nullptr, BCRYPT_ECCPUBLIC_BLOB, blob.data(),
                                           static_cast<ULONG>(blob.size()), &got, 0)));
    REQUIRE(got == blob.size());
    std::memcpy(out.public_key.data(), blob.data() + sizeof(BCRYPT_ECCKEY_BLOB), 64);

    ULONG sig_len = 0;
    REQUIRE(BCRYPT_SUCCESS(BCryptSignHash(key, nullptr, hash.data(), static_cast<ULONG>(hash.size()),
                                          nullptr, 0, &sig_len, 0)));
    out.sig.resize(sig_len);
    REQUIRE(BCRYPT_SUCCESS(BCryptSignHash(key, nullptr, hash.data(), static_cast<ULONG>(hash.size()),
                                          out.sig.data(), sig_len, &sig_len, 0)));
    out.sig.resize(sig_len);

    BCryptDestroyKey(key);
    BCryptCloseAlgorithmProvider(alg, 0);
    return out;
}

}  // namespace

TEST_CASE("sha256_file gives the sample's digest") {
    CHECK(hydra::app::sha256_file(sample_path()) == kSampleSha256);
}

TEST_CASE("verify_update passes the signed sample with the test key") {
    CHECK_FALSE(refusal(sample_path(), kSampleSha256, sample_sig(), test_public_key()).has_value());
}

TEST_CASE("verify_update refuses a file with any one byte changed") {
    const std::vector<uint8_t> original = hydra::read_file_bytes(sample_path());
    REQUIRE(original.size() > 2);
    testtemp::ScopedFile copy("update_verify_flip", ".bin");
    // The first byte, one in the middle and the last.
    for (size_t at : {size_t{0}, original.size() / 2, original.size() - 1}) {
        CAPTURE(at);
        std::vector<uint8_t> bytes = original;
        bytes[at] ^= 0x01;
        testtemp::write_bytes(copy.path, bytes);
        CHECK(refusal(copy.path, kSampleSha256, sample_sig(), test_public_key()) == kRefusalKind);
        // The digest of the changed file, so the signature check is the one that refuses.
        CHECK(refusal(copy.path, hydra::app::sha256_file(copy.path), sample_sig(), test_public_key()) ==
              kRefusalKind);
    }
}

TEST_CASE("verify_update refuses a signature with any one byte changed") {
    const std::vector<uint8_t> original = sample_sig();
    REQUIRE(original.size() == 64);
    for (size_t at = 0; at < original.size(); ++at) {
        CAPTURE(at);
        std::vector<uint8_t> sig = original;
        sig[at] ^= 0x01;
        CHECK(refusal(sample_path(), kSampleSha256, sig, test_public_key()) == kRefusalKind);
    }
}

TEST_CASE("verify_update refuses a digest with any one digit changed") {
    for (size_t at = 0; at < kSampleSha256.size(); ++at) {
        CAPTURE(at);
        std::string digest = kSampleSha256;
        digest[at] = digest[at] == '0' ? '1' : '0';
        CHECK(refusal(sample_path(), digest, sample_sig(), test_public_key()) == kRefusalKind);
    }
    CHECK(refusal(sample_path(), "", sample_sig(), test_public_key()) == kRefusalKind);
}

TEST_CASE("verify_update refuses a 63-byte signature") {
    std::vector<uint8_t> sig = sample_sig();
    sig.pop_back();
    CHECK(refusal(sample_path(), kSampleSha256, sig, test_public_key()) == kRefusalKind);
}

TEST_CASE("verify_update refuses a signature from a second key") {
    const OtherKey other = sign_with_new_key(hex_to_hash(kSampleSha256));
    // The second key's own signature checks out with its own public half...
    CHECK_FALSE(refusal(sample_path(), kSampleSha256, other.sig, other.public_key).has_value());
    // ...and is refused with the test key, as the test key's is with the second key.
    CHECK(refusal(sample_path(), kSampleSha256, other.sig, test_public_key()) == kRefusalKind);
    CHECK(refusal(sample_path(), kSampleSha256, sample_sig(), other.public_key) == kRefusalKind);
}

TEST_CASE("verify_update refuses a missing file") {
    testtemp::ScopedFile gone("update_verify_gone", ".bin");
    CHECK(refusal(gone.path, kSampleSha256, sample_sig(), test_public_key()) == kRefusalKind);
}

TEST_CASE("the shipped update key is not the test key") {
    // A build must never ship the test key; app/update_key.h holds the
    // production key (docs/development.md, "Releasing").
    CHECK_MESSAGE(hydra::app::kUpdatePublicKey != test_public_key(),
                  "replace the test key with the production key before release");
}
