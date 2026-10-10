// The settings every GUI test runs under, and the ones the Burnout reference
// results in the unit tests were made with (audit finding 280). The GUI
// harness writes its scratch hydra_settings.ini from this struct, and the unit
// tests read it through the app's own Settings::to_analysis_settings, so the
// two can never drift apart.
//
// Everything is the app's default except three things: the chart folder is
// the checked-in corpus, a finished report does not open a browser, and the
// depth is 2 scores to keep analyses short.

#ifndef HYDRA_TESTS_SCRATCH_SETTINGS_H
#define HYDRA_TESTS_SCRATCH_SETTINGS_H

#include "app/config.h"
#include "testdata_dirs.h"  // root

inline hydra::app::Settings scratch_settings() {
    hydra::app::Settings s;
    s.chartfolders = {corpus::root()};
    s.auto_open_report = false;
    s.depth_value = 2;
    return s;
}

#endif  // HYDRA_TESTS_SCRATCH_SETTINGS_H
