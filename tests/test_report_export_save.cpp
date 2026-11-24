// AppState's save of a report window's Excel export (save_report_export,
// ui/app_state.h), given its file directly, so no save dialog opens: what
// the window's export line then says, and the folder Settings keeps.

#include "doctest.h"

#include <filesystem>
#include <string>

#include "app/user_messages.h"  // plain_error
#include "app/xlsx.h"
#include "core/error_kind.h"
#include "core/winstr.h"  // os_path
#include "scratch_paths.h"
#include "temp_util.h"
#include "ui/app_state.h"

using hydra::ui::AppState;
using hydra::ui::ExportNote;

namespace {

// A sheet of one text cell, which the writer takes.
hydra::app::xlsx::Sheet one_cell_sheet() {
    hydra::app::xlsx::Sheet sheet;
    sheet.name = "Path Index";
    sheet.widths = {10};
    sheet.rows = {{hydra::app::xlsx::Cell::of_text("Song", {})}};
    return sheet;
}

}  // namespace

TEST_CASE("report export save: a good write says where it saved and keeps the folder") {
    ScratchPaths paths("report_export_save_good");
    AppState app;
    app.wait_store_open();
    const testtemp::ScopedFile file("report_export_save", ".xlsx");
    ExportNote note;
    app.save_report_export(one_cell_sheet(), file.path, note);
    // The line's own words, then the file exactly as picked.
    const std::string prefix = hydra::ui::kExportSavedPrefix;
    REQUIRE(note.text.size() > prefix.size());
    CHECK(note.text.compare(0, prefix.size(), prefix) == 0);
    CHECK(note.text.substr(prefix.size()) == file.path);
    CHECK_FALSE(note.problem);
    CHECK(std::filesystem::exists(hydra::os_path(std::filesystem::u8path(file.path))));
    CHECK(app.settings.export_folder ==
          std::filesystem::u8path(file.path).parent_path().u8string());
}

TEST_CASE("report export save: a write that fails says Hydra's ReportWrite sentence and keeps no folder") {
    ScratchPaths paths("report_export_save_bad");
    AppState app;
    app.wait_store_open();
    const std::string dir = testtemp::temp_path("report_export_save_missing", "");
    testtemp::remove_dir(dir);
    ExportNote note;
    app.save_report_export(one_cell_sheet(), dir + "\\Path report.xlsx", note);
    CHECK(note.text ==
          hydra::app::plain_error(hydra::KindedError(hydra::ErrorKind::ReportWrite, "")));
    CHECK(note.problem);
    CHECK(app.settings.export_folder.empty());
}
