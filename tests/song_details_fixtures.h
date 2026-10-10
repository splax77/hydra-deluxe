// The Song Details fixtures that test_path_list.cpp and
// test_activation_rows.cpp share: the corpus chart they draw from, Burnout
// analyzed the way the GUI tests analyze it, and the separator the labels use.
// The functions are inline in a named namespace, so each chart is analyzed
// once per test run, not once per file.

#ifndef HYDRA_TESTS_SONG_DETAILS_FIXTURES_H
#define HYDRA_TESTS_SONG_DETAILS_FIXTURES_H

#include <stdexcept>
#include <string>

#include "analyzed_chart.h"
#include "app/analysis.h"
#include "corpus_util.h"
#include "scratch_settings.h"

namespace song_details {

using namespace hydra;
using namespace hydra::app;

// One analyzed corpus chart (see analyzed_chart.h), shared across cases.
inline const AnalysisResult& analyzed() { return corpus::analyzed_by_ten_scores(); }

// Burnout (Green Day, charter Hoph2o), analyzed the way the GUI tests
// analyze it: Expert, Pro Drums, 2x Bass, SP cap 4, 2 scores, 10 ms limit.
inline const AnalysisResult& burnout() {
    static const AnalysisResult result = [] {
        const AnalysisSettings settings = scratch_settings().to_analysis_settings();
        for (const std::string& path : corpus::chart_paths())
            if (path.find("Green Day - Burnout") != std::string::npos)
                return analyze_chart_file(path, settings);
        throw std::runtime_error("Burnout is not in testdata/input");
    }();
    return result;
}

// The separator the new labels use: a middle dot, U+00B7, in UTF-8.
inline const std::string kDot = " \xC2\xB7 ";

}  // namespace song_details

#endif
