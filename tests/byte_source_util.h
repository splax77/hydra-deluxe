// The test wrappers around a ByteSource: one counts what a load reads (the
// container tests prove a .sng or .srb load reads only its header and notes),
// one makes the first read come back short.

#ifndef HYDRA_TESTS_BYTE_SOURCE_UTIL_H
#define HYDRA_TESTS_BYTE_SOURCE_UTIL_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "core/winstr.h"

namespace testbytes {

// `inner`, adding every byte it hands back to `bytes_read`, which must outlive
// the returned source.
inline hydra::ByteSource counting(hydra::ByteSource inner, uint64_t& bytes_read) {
    hydra::ByteSource out;
    out.size = inner.size;
    out.read = [inner, &bytes_read](uint64_t offset, size_t length) {
        std::vector<uint8_t> b = inner.read(offset, length);
        bytes_read += b.size();
        return b;
    };
    return out;
}

// `inner`, except that its first read hands back at most `keep` bytes: a file
// that looked shorter for a moment. A reader that follows the short-read rule
// stops there; one that reads on finds the later bytes.
inline hydra::ByteSource short_first_read(hydra::ByteSource inner, size_t keep) {
    hydra::ByteSource out;
    out.size = inner.size;
    auto first = std::make_shared<bool>(true);
    out.read = [inner, keep, first](uint64_t offset, size_t length) {
        std::vector<uint8_t> b = inner.read(offset, length);
        if (*first && b.size() > keep) b.resize(keep);
        *first = false;
        return b;
    };
    return out;
}

}  // namespace testbytes

#endif  // HYDRA_TESTS_BYTE_SOURCE_UTIL_H
