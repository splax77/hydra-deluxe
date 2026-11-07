// The GUI's default analysis settings and the corpus song read under them, for
// the tests that price or word a corpus chart the way a fresh Hydra would.

#ifndef HYDRA_TESTS_CORPUS_DEFAULT_CFG_H
#define HYDRA_TESTS_CORPUS_DEFAULT_CFG_H

#include <string>

#include "app/config.h"
#include "corpus_util.h"
#include "doctest.h"
#include "parse/song.h"

namespace corpus {

// The settings a fresh Hydra analyzes with (its Path limit is 10 ms).
inline const hydra::app::AnalysisSettings& default_cfg() {
    static const hydra::app::AnalysisSettings cfg =
        hydra::app::Settings().to_analysis_settings();
    return cfg;
}

// The corpus chart whose path holds `name`, parsed under default_cfg's reading
// and rules.
inline const hydra::Song& song_named(const std::string& name) {
    const std::string chart = chart_named(name);
    REQUIRE_FALSE(chart.empty());
    return song(chart, default_cfg().reading(), default_cfg().rules);
}

}  // namespace corpus

#endif  // HYDRA_TESTS_CORPUS_DEFAULT_CFG_H
