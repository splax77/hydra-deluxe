// Which file is a chart, decided once for the library scan and the parser.
//
// Clone Hero reads a loose folder's notes.mid / notes.chart / song.ini in any
// letter case, and so does Hydra: the scan (app/analysis.cpp discover_charts)
// and the loaders (parse/song.cpp) both ask these functions, so the two can
// never disagree about which files count.

#ifndef HYDRA_PARSE_CHART_FILES_H
#define HYDRA_PARSE_CHART_FILES_H

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "core/winstr.h"  // DirEntry

namespace hydra {

enum class ChartFormat { None, Mid, Chart, Sng, Srb };

// A path's chart format from its extension, any case: ".mid", ".chart",
// ".sng", ".srb". Anything else is None.
ChartFormat chart_format_of(std::string_view path);

// A notes file by its exact name, any case, wherever it sits: loose in a song
// folder, or as an entry inside a .sng or .srb. "notes.mid" is Mid,
// "notes.chart" is Chart, anything else is None. The one name rule for both.
ChartFormat notes_file_format(std::string_view filename);

// The notes file a listing picked: its position in the listing and its format
// (Mid or Chart).
struct NotesFilePick {
    size_t index = 0;
    ChartFormat format = ChartFormat::None;
};

// Which name in a listing (a song folder's entries, or a .sng's entries) is
// the notes file, when a song has both: the first notes.mid wins; with none,
// the last notes.chart does. Empty when the listing holds neither. The one
// rule for the scan and the .sng loader.
std::optional<NotesFilePick> pick_notes_file(const std::vector<std::string>& names);

// "song.ini", any case.
bool is_song_ini(std::string_view filename);

// The folder's song.ini in a listing of it: the first file (not a folder)
// named song.ini in any case, or nullptr when there is none. A Windows folder
// holds at most one, so "first" only matters on paper.
const DirEntry* find_song_ini(const std::vector<DirEntry>& listing);

// The same pick over the folder itself (list_dir): the song.ini's full path,
// or "" when there is none.
std::string find_song_ini(const std::string& folder);

}  // namespace hydra

#endif  // HYDRA_PARSE_CHART_FILES_H
