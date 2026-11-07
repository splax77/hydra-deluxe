#include "parse/chart_files.h"

#include <string>

#include "core/strutil.h"

namespace hydra {

const std::vector<ChartExtension>& chart_extensions() {
    static const std::vector<ChartExtension> list = {
        {".mid", ChartFormat::Mid},
        {".chart", ChartFormat::Chart},
        {".sng", ChartFormat::Sng},
        {".srb", ChartFormat::Srb},
    };
    return list;
}

ChartFormat chart_format_of(std::string_view path) {
    for (const ChartExtension& e : chart_extensions())
        if (ends_with_ci(path, e.extension)) return e.format;
    return ChartFormat::None;
}

bool is_container(ChartFormat format) {
    return format == ChartFormat::Sng || format == ChartFormat::Srb;
}

ChartFormat notes_file_format(std::string_view filename) {
    const std::string low = to_lower_ascii(filename);
    if (low == "notes.mid") return ChartFormat::Mid;
    if (low == "notes.chart") return ChartFormat::Chart;
    return ChartFormat::None;
}

std::optional<NotesFilePick> pick_notes_file(const std::vector<std::string>& names) {
    std::optional<NotesFilePick> pick;
    for (size_t i = 0; i < names.size(); ++i) {
        const ChartFormat f = notes_file_format(names[i]);
        if (f == ChartFormat::Mid) return NotesFilePick{i, f};
        if (f == ChartFormat::Chart) pick = NotesFilePick{i, f};
    }
    return pick;
}

bool is_song_ini(std::string_view filename) { return to_lower_ascii(filename) == "song.ini"; }

const DirEntry* find_song_ini(const std::vector<DirEntry>& listing) {
    for (const DirEntry& e : listing)
        if (!e.is_dir && is_song_ini(e.name)) return &e;
    return nullptr;
}

std::string find_song_ini(const std::string& folder) {
    const std::vector<DirEntry> listing = list_dir(folder);
    const DirEntry* ini = find_song_ini(listing);
    return ini ? join_folder(folder, ini->name) : std::string();
}

}  // namespace hydra
