// The chart loaders as the tests call them. parse/song.h's loaders take the
// rules and the fill rule with no default, so production code that forgets
// the fill rule fails to compile (D115 TJ-3: the Preview read 1.1 fills under
// the "1.0 fills" setting that way). Tests mostly read charts the way the
// app's default settings do, so this header names their fill rule once,
// kTestFillRule, and gives each loader the shorter forms the tests used
// before. A test about another game version's fills passes its rule to the
// full form.

#ifndef HYDRA_TESTS_LOAD_UTIL_H
#define HYDRA_TESTS_LOAD_UTIL_H

#include <cstdint>
#include <string>
#include <vector>

#include "core/rules.h"
#include "parse/chart_reading.h"
#include "parse/song.h"

namespace hydra {

// The fill rule every test load uses unless it names its own: Clone Hero
// 1.1's generator, the app's default ("1.0 fills" off).
inline constexpr FillDeadlineRule kTestFillRule = FillDeadlineRule::Ch11;

#define HYDRA_TEST_LOADER(name, Source)                                                        \
    inline Song name(Source src, const ChartReading& reading, const core::Rules& rules) {     \
        return name(src, reading, rules, kTestFillRule);                                     \
    }                                                                                         \
    inline Song name(Source src, const ChartReading& reading) {                              \
        return name(src, reading, core::default_rules(), kTestFillRule);                     \
    }

HYDRA_TEST_LOADER(load_songpath, const std::string&)
HYDRA_TEST_LOADER(load_songpath_with_notes, const std::string&)
HYDRA_TEST_LOADER(load_songpath_mid, const std::string&)
HYDRA_TEST_LOADER(load_songpath_chart, const std::string&)
HYDRA_TEST_LOADER(load_songpath_sng, const std::string&)
HYDRA_TEST_LOADER(load_songpath_srb, const std::string&)
HYDRA_TEST_LOADER(load_songbytes_mid, const std::vector<uint8_t>&)
HYDRA_TEST_LOADER(load_songbytes_chart, const std::vector<uint8_t>&)
HYDRA_TEST_LOADER(load_songbytes_sng, const std::vector<uint8_t>&)
HYDRA_TEST_LOADER(load_songbytes_srb, const std::vector<uint8_t>&)

#undef HYDRA_TEST_LOADER

inline Song load_songpath_from_bytes(const std::string& path, const std::vector<uint8_t>& bytes,
                                     const ChartReading& reading, const core::Rules& rules) {
    return load_songpath_from_bytes(path, bytes, reading, rules, kTestFillRule);
}
inline Song load_songpath_from_bytes(const std::string& path, const std::vector<uint8_t>& bytes,
                                     const ChartReading& reading) {
    return load_songpath_from_bytes(path, bytes, reading, core::default_rules(), kTestFillRule);
}

inline Song load_songpath_reading(const ByteSource& src, const std::string& path,
                                  const ChartReading& reading, const core::Rules& rules) {
    return load_songpath_reading(src, path, reading, rules, kTestFillRule);
}
inline Song load_songpath_reading(const ByteSource& src, const std::string& path,
                                  const ChartReading& reading) {
    return load_songpath_reading(src, path, reading, core::default_rules(), kTestFillRule);
}

}  // namespace hydra

#endif  // HYDRA_TESTS_LOAD_UTIL_H
