// The fuzzed fast-tempo charts and the analysis wrapper that reports a throw
// as a failed check. Shared by test_fast_tempo.cpp and test_search_target.cpp,
// whose search_target cases pin answers on these charts.

#ifndef HYDRA_TESTS_FAST_TEMPO_CHARTS_H
#define HYDRA_TESTS_FAST_TEMPO_CHARTS_H

#include <algorithm>
#include <cstdint>
#include <exception>
#include <sstream>
#include <string>
#include <vector>

#include "app/config.h"
#include "chart_text.h"
#include "core/model.h"
#include "doctest.h"
#include "parse/song.h"
#include "search/pather.h"

namespace hydra::test {

// The analysis, or a failed check naming the throw (so one chart's throw
// does not hide the others').
inline bool analyzes(const Song& song, const app::AnalysisSettings& cfg, HydraRecord& rec) {
    try {
        rec = analyze_chart(song, cfg);
        return true;
    } catch (const std::exception& e) {
        FAIL_CHECK("analyze_chart threw: " << std::string(e.what()));
        return false;
    }
}

// The reviewer's fuzz chart shape (review-sqout fuzz.cpp), on a fixed
// generator so the charts are the same on every build: four measures at
// 120 BPM with two or three phrases, then 2,000 or 4,000 BPM from a random
// tick, with 12 to 30 random notes that are phrases, fills or plain.
struct FuzzChart {
    std::string text;
    int cap = 2;
};
inline FuzzChart fuzz_chart(uint64_t seed) {
    uint64_t s = seed * 0x9E3779B97F4A7C15ull + 1;
    auto next = [&s]() {
        uint64_t z = (s += 0x9E3779B97F4A7C15ull);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        return z ^ (z >> 31);
    };
    auto U = [&next](int a, int b) { return a + (int)(next() % (uint64_t)(b - a + 1)); };
    const int fast_at = 3072 + 96 * U(0, 60);
    const std::string sync =
        testchart::kSync44At120 +
        testchart::line(fast_at, "B " + std::to_string(U(0, 1) ? 4000000 : 2000000));
    std::ostringstream c;  // the [ExpertDrums] lines
    std::vector<int> ticks;
    for (int t = 0; t < 3072; t += 96) ticks.push_back(t);
    const int n_rand = U(12, 30);
    for (int i = 0; i < n_rand; ++i) ticks.push_back(3072 + 96 * U(0, 220));
    std::sort(ticks.begin(), ticks.end());
    ticks.erase(std::unique(ticks.begin(), ticks.end()), ticks.end());
    for (int t : ticks) {
        bool phrase = t == 768 || t == 2304;
        bool fill = false;
        if (t == 1536) phrase = U(0, 1) == 1;
        if (t >= 3072) {
            const int r = U(0, 99);
            phrase = r < 28;
            fill = !phrase && r < 52;
        }
        if (fill) c << "  " << (t - 384) << " = S 64 384\n";
        if (phrase) c << "  " << t << " = S 2 10\n";
        c << "  " << t << " = N " << (fill ? 1 : U(0, 1) ? 1 : 2) << " 0\n";
    }
    return {testchart::chart_text(
                testchart::section("Events", "") + testchart::section("ExpertDrums", c.str()),
                192, "  Offset = 0\n", sync),
            U(2, 4)};
}

}  // namespace hydra::test

#endif  // HYDRA_TESTS_FAST_TEMPO_CHARTS_H
