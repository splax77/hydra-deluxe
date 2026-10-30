// The Path Builder's saved path files and its copy and paste. A path file is
// the JSON shape hydra_replay writes and reads (paths_json and
// windows_from_json, tools/replay_json.h). Copy writes Hydra's own Copy path
// text (Path::pathstring_verbose) and paste reads it back through
// paths_from_pathstring (search/pathstring_read.h), which takes the long and
// the short form. No ImGui and no window; pb_window.cpp acts on the menu
// through these.

#ifndef HYDRA_PATHBUILDER_PB_FILES_H
#define HYDRA_PATHBUILDER_PB_FILES_H

#include <optional>
#include <string>
#include <vector>

#include "json.hpp"

#include "core/model.h"
#include "core/song.h"
#include "pathbuilder/pb_state.h"

namespace hydra::pathbuilder {

// The text Copy puts on the clipboard for `path` on the chart `record` was
// made for: the text Hydra's Copy path button writes (copy_selected_path in
// src/ui/paths_tab.cpp), from the same writer.
std::string copy_text(const HydraRecord& record, const Path& path);

// Why a paste or an open changed nothing: app::plain_error's sentence and
// app::plain_error_detail's line for the error that stopped it.
struct Refusal {
    std::string message;
    std::string detail;
};

// Reads `text` (the long or the short form) on the open chart and makes the
// path it names the player's, as one edit of `history` (player_copy_of, as
// "Edit a copy" makes it), so undo puts the old path back. When the text does
// not fit the chart, `history` is left as it was and the refusal says why:
// the reader's refusal reaches plain_error as ErrorKind::PathTextRefused.
std::optional<Refusal> paste_path(PathHistory& history, const OpenChart& chart,
                                  const std::string& text);

// What Save writes for `path`: an object whose "paths" array is
// paths_json's, the shape `hydra_replay score --path` reads.
nlohmann::json path_file_json(const Path& path, const Song& song);

// Writes path_file_json to `file`, replacing it. Throws
// KindedError(ErrorKind::PathFileWrite) when the file cannot be written.
void save_path_file(const std::string& file, const Path& path, const Song& song);

// The paths a path file holds, on the open chart: each entry of its "paths"
// array read back by windows_from_json, pinned in full (pinned_windows) and
// handed to search_target, whose path it is. Throws
// KindedError(ErrorKind::PathFileUnreadable) when the file cannot be opened,
// and KindedError(ErrorKind::PathFileRefused) when it is not that shape or an
// entry's windows do not fit the chart; the text names the file and the entry.
std::vector<Path> read_path_file(const std::string& file, const OpenChart& chart);

// Opens a path file: every path read_path_file finds is added to `branches`
// (the left list's saved branches) unless the left list (path_rows) already
// shows it, judged by path_identity, so each path shows once (the user's
// ruling, "Show each path once"). The first path becomes the player's path
// either way, as one edit of `history`, so undo puts the old path back. When
// the file cannot be read or does not fit the chart, nothing changes and the
// refusal says why.
std::optional<Refusal> open_path_file(PathHistory& history, std::vector<Path>& branches,
                                      const OpenChart& chart, const std::string& file);

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_FILES_H
