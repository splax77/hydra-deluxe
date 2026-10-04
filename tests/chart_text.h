// The one writer of a minimal .chart in the tests.
//
// A unit case that pins one .chart rule needs a few lines of chart text, not a
// corpus file. chart_text() writes the [Song] and [SyncTrack] header once, so
// each case states only the lines its rule is about.

#ifndef HYDRA_TESTS_CHART_TEXT_H
#define HYDRA_TESTS_CHART_TEXT_H

#include <cstdint>
#include <string>
#include <vector>

namespace testchart {

// The usual [SyncTrack] body: 4/4 from tick 0, at 120 BPM.
inline const std::string kSync44At120 = "  0 = TS 4\n  0 = B 120000\n";

// One section: its [name], then `lines` between braces.
inline std::string section(const std::string& name, const std::string& lines) {
    return "[" + name + "]\n{\n" + lines + "}\n";
}

// "  <tick> = <what>\n", one line of a section.
inline std::string line(int64_t tick, const std::string& what) {
    return "  " + std::to_string(tick) + " = " + what + "\n";
}

// A whole .chart: [Song] with the resolution and any `song_extra` lines, then
// [SyncTrack] with `sync`, then `sections` (each written by section()).
inline std::string chart_text(const std::string& sections, int64_t resolution = 192,
                              const std::string& song_extra = "",
                              const std::string& sync = kSync44At120) {
    return section("Song", "  Resolution = " + std::to_string(resolution) + "\n" + song_extra) +
           section("SyncTrack", sync) + sections;
}

// chart_text() as the bytes a loader reads.
inline std::vector<uint8_t> chart_bytes(const std::string& sections, int64_t resolution = 192,
                                        const std::string& song_extra = "",
                                        const std::string& sync = kSync44At120) {
    const std::string s = chart_text(sections, resolution, song_extra, sync);
    return std::vector<uint8_t>(s.begin(), s.end());
}

}  // namespace testchart

#endif  // HYDRA_TESTS_CHART_TEXT_H
