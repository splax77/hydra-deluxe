// One analyzed corpus chart, shared by the view-model tests that need a real
// result to draw from. The Song Details tests (test_path_view.cpp) and the
// Preview tests (test_preview_view.cpp) both read it.

#ifndef HYDRA_TESTS_ANALYZED_CHART_H
#define HYDRA_TESTS_ANALYZED_CHART_H

#include "app/analysis.h"
#include "corpus_util.h"

namespace corpus {

// The first corpus chart that yields paths, analyzed by score depth 10 with a
// 10 ms limit. It is analyzed once per test run: analysis is the slow part.
inline const hydra::app::AnalysisResult& analyzed_by_ten_scores() {
    static const hydra::app::AnalysisResult result = [] {
        hydra::app::AnalysisSettings settings;
        settings.depth_mode = hydra::DepthMode::Scores;
        settings.depth_value = 10;
        settings.ms_filter = 10.0;
        return first_analyzed_with_paths(settings);
    }();
    return result;
}

}  // namespace corpus

#endif
