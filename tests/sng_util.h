// The one .sng fixture writer the tests share, like srb_util.h for .srb.
//
// Tests fabricate a .sng the way parse/sng.h reads one. Audit finding 117:
// every test that builds a .sng builds it through here.
//
// Note for task J2-5: test_preview_source.cpp still has its own make_sng,
// which writes 20 junk bytes in place of the metadata block when it gets no
// pairs. This one always writes a real metadata block, an empty one when
// there are no pairs.

#ifndef HYDRA_TESTS_SNG_UTIL_H
#define HYDRA_TESTS_SNG_UTIL_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "parse/sng.h"

namespace testsng {

inline void push_u32(std::vector<uint8_t>& out, uint64_t v) {
    for (int i = 0; i < 4; ++i) out.push_back(static_cast<uint8_t>(v >> (8 * i)));
}
inline void push_u64(std::vector<uint8_t>& out, uint64_t v) {
    for (int i = 0; i < 8; ++i) out.push_back(static_cast<uint8_t>(v >> (8 * i)));
}

// A .sng: the prefix bytes, the XOR mask, the metadata block (pair count,
// then u32-length key and value strings), the file section (its length, the
// file count, then name/length/absolute-offset entries), then each file's
// bytes XOR-encoded from its own index 0.
inline std::vector<uint8_t> make_sng(
    const std::vector<std::pair<std::string, std::string>>& meta,
    const std::vector<std::pair<std::string, std::vector<uint8_t>>>& files) {
    std::vector<uint8_t> out(hydra::kSngXorMaskOffset, 0x53);
    uint8_t mask[hydra::kSngXorMaskSize];
    for (size_t i = 0; i < hydra::kSngXorMaskSize; ++i)
        mask[i] = static_cast<uint8_t>(0x30 + i * 7);
    out.insert(out.end(), mask, mask + hydra::kSngXorMaskSize);

    std::vector<uint8_t> md;
    push_u64(md, meta.size());
    for (const auto& [k, v] : meta) {
        push_u32(md, k.size());
        md.insert(md.end(), k.begin(), k.end());
        push_u32(md, v.size());
        md.insert(md.end(), v.begin(), v.end());
    }
    push_u64(out, md.size());
    out.insert(out.end(), md.begin(), md.end());

    size_t entries = 0;
    for (const auto& f : files) entries += 1 + f.first.size() + 16;
    uint64_t offset = out.size() + 16 + entries;
    push_u64(out, 8 + entries);
    push_u64(out, files.size());
    for (const auto& f : files) {
        out.push_back(static_cast<uint8_t>(f.first.size()));
        out.insert(out.end(), f.first.begin(), f.first.end());
        push_u64(out, f.second.size());
        push_u64(out, offset);
        offset += f.second.size();
    }
    for (const auto& f : files)
        for (size_t i = 0; i < f.second.size(); ++i)
            out.push_back(static_cast<uint8_t>(f.second[i] ^ mask[i % hydra::kSngXorMaskSize] ^
                                               (i & 0xff)));
    return out;
}

}  // namespace testsng

#endif  // HYDRA_TESTS_SNG_UTIL_H
