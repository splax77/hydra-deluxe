// Where a report stands, in the words both AppState (ui/app_state.h) and the
// report windows (ui/report_window.h) use. One enum per idea, so nothing maps
// one copy onto another.

#ifndef HYDRA_UI_REPORT_STATE_H
#define HYDRA_UI_REPORT_STATE_H

namespace hydra::ui {

// Where a report's latest build stands. AppState::path_report_build and
// dm_report_build answer it: Building while the report's job runs, otherwise
// how the last build ended (ReportSlot::last), or None when the report was
// never built. A window draws each one in report_frame's state_body
// (ui/report_window.cpp). A Ready report with no rows shows its
// nothing-to-report text instead of a table.
enum class ReportBuild { None, Building, Ready, Cancelled, Failed };

// Why the rows on screen may be older than the library or the settings: a
// batch finished after the report was built (Library), or a setting it reads
// changed (Settings). The later of the two wins (D103). The window's
// out-of-date strip reads it.
enum class ReportOutOfDate { None, Library, Settings };

}  // namespace hydra::ui

#endif  // HYDRA_UI_REPORT_STATE_H
