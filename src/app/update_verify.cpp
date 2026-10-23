#include "app/update_verify.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>

#include <cstdio>
#include <cstring>
#include <memory>

#include "core/error_kind.h"
#include "core/strutil.h"
#include "core/winstr.h"

namespace hydra::app {

namespace {

// Every refusal verify_update makes goes through here, so the kind is named
// once.
// UpdateUnverified: swap NetBadReply for ErrorKind::UpdateUnverified once
// that kind exists (task UPD-T2a adds it to core/error_kind.h).
[[noreturn]] void refuse(const std::string& why) {
    throw KindedError(ErrorKind::NetBadReply, why);
}

// One bcrypt algorithm provider, closed when it goes out of scope.
class Provider {
public:
    explicit Provider(LPCWSTR algorithm) {
        if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&alg_, algorithm, nullptr, 0)))
            alg_ = nullptr;
    }
    ~Provider() {
        if (alg_) BCryptCloseAlgorithmProvider(alg_, 0);
    }
    Provider(const Provider&) = delete;
    Provider& operator=(const Provider&) = delete;

    BCRYPT_ALG_HANDLE handle() const { return alg_; }

private:
    BCRYPT_ALG_HANDLE alg_ = nullptr;
};

using Sha256 = std::array<uint8_t, 32>;

// An ECDSA P-256 signature in the IEEE P1363 form (r then s) is two 32-byte
// numbers.
constexpr size_t kSignatureBytes = 64;

Sha256 sha256_bytes(const std::string& path) {
    FILE* f = fopen_utf8(path, L"rb");
    if (f == nullptr) throw KindedError(ErrorKind::HashFailed, "cannot open file: " + path);
    std::unique_ptr<FILE, int (*)(FILE*)> closer(f, &std::fclose);

    Provider alg(BCRYPT_SHA256_ALGORITHM);
    if (alg.handle() == nullptr)
        throw KindedError(ErrorKind::HashFailed, "BCryptOpenAlgorithmProvider(SHA256) failed");
    BCRYPT_HASH_HANDLE hash = nullptr;
    if (!BCRYPT_SUCCESS(BCryptCreateHash(alg.handle(), &hash, nullptr, 0, nullptr, 0, 0)))
        throw KindedError(ErrorKind::HashFailed, "SHA-256 hashing failed");
    std::unique_ptr<void, NTSTATUS (*)(BCRYPT_HASH_HANDLE)> hash_closer(hash, &BCryptDestroyHash);

    std::vector<uint8_t> buf(1 << 20);
    size_t got;
    while ((got = std::fread(buf.data(), 1, buf.size(), f)) > 0) {
        if (!BCRYPT_SUCCESS(BCryptHashData(hash, buf.data(), static_cast<ULONG>(got), 0)))
            throw KindedError(ErrorKind::HashFailed, "SHA-256 hashing failed");
    }
    if (std::ferror(f)) throw KindedError(ErrorKind::HashFailed, "cannot read file: " + path);

    Sha256 digest{};
    if (!BCRYPT_SUCCESS(BCryptFinishHash(hash, digest.data(), static_cast<ULONG>(digest.size()), 0)))
        throw KindedError(ErrorKind::HashFailed, "SHA-256 hashing failed");
    return digest;
}

std::string to_hex(const Sha256& bytes) { return hex_lower(bytes.data(), bytes.size()); }

// Whether `sig` is the release key's signature of `hash`. The key goes to
// bcrypt as a BCRYPT_ECCKEY_BLOB header followed by X then Y, the 64 bytes
// tools/release_keygen.ps1 prints.
bool signature_matches(const Sha256& hash, const std::vector<uint8_t>& sig,
                       const std::array<uint8_t, 64>& public_key) {
    Provider alg(BCRYPT_ECDSA_P256_ALGORITHM);
    if (alg.handle() == nullptr) refuse("BCryptOpenAlgorithmProvider(ECDSA_P256) failed");

    std::vector<uint8_t> blob(sizeof(BCRYPT_ECCKEY_BLOB) + public_key.size());
    BCRYPT_ECCKEY_BLOB header{};
    header.dwMagic = BCRYPT_ECDSA_PUBLIC_P256_MAGIC;
    header.cbKey = static_cast<ULONG>(public_key.size() / 2);
    std::memcpy(blob.data(), &header, sizeof(header));
    std::memcpy(blob.data() + sizeof(header), public_key.data(), public_key.size());

    BCRYPT_KEY_HANDLE key = nullptr;
    if (!BCRYPT_SUCCESS(BCryptImportKeyPair(alg.handle(), nullptr, BCRYPT_ECCPUBLIC_BLOB, &key,
                                            blob.data(), static_cast<ULONG>(blob.size()), 0)))
        refuse("the release key isn't a valid P-256 public key");
    std::unique_ptr<void, NTSTATUS (*)(BCRYPT_KEY_HANDLE)> key_closer(key, &BCryptDestroyKey);

    // bcrypt takes non-const buffers but does not write to either.
    Sha256 h = hash;
    std::vector<uint8_t> s = sig;
    return BCRYPT_SUCCESS(BCryptVerifySignature(key, nullptr, h.data(), static_cast<ULONG>(h.size()),
                                                s.data(), static_cast<ULONG>(s.size()), 0));
}

}  // namespace

std::string sha256_file(const std::string& path) { return to_hex(sha256_bytes(path)); }

void verify_update(const std::string& path, const std::string& digest_hex,
                   const std::vector<uint8_t>& sig, const std::array<uint8_t, 64>& public_key) {
    Sha256 hash{};
    try {
        hash = sha256_bytes(path);
    } catch (const KindedError&) {
        refuse("the download couldn't be read and hashed: " + path);
    }
    if (digest_hex != to_hex(hash))
        refuse("the download's SHA-256 is " + to_hex(hash) + ", but the release states " +
               (digest_hex.empty() ? std::string("none") : digest_hex));
    if (sig.size() != kSignatureBytes)
        refuse("the signature is " + std::to_string(sig.size()) + " bytes, not " +
               std::to_string(kSignatureBytes));
    if (!signature_matches(hash, sig, public_key))
        refuse("the signature doesn't match the release key");
}

}  // namespace hydra::app
