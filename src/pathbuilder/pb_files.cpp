#include "pathbuilder/pb_files.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "replay_json.h"

#include "app/report_files.h"  // write_file_replacing
#include "app/user_messages.h"
#include "core/winstr.h"  // os_path
#include "search/pather.h"
#include "search/pathstring_read.h"

namespace hydra::pathbuilder {

namespace {

Refusal refusal_for(const std::exception& e) {
    return Refusal{app::plain_error(e), app::plain_error_detail(e)};
}

}  // namespace

std::string copy_text(const HydraRecord& record, const Path& path) {
    return path.pathstring_verbose(record.multsqueezes);
}

std::optional<Refusal> paste_path(PathHistory& history, const OpenChart& chart,
                                  const std::string& text) {
    try {
        // Both steps run before the edit, so a refusal leaves the history alone.
        PlayerPath next = player_copy_of(paths_from_pathstring(chart.song, chart.settings, text));
        history.edit(std::move(next));
    } catch (const std::exception& e) {
        return refusal_for(e);
    }
    return std::nullopt;
}

nlohmann::json path_file_json(const Path& path, const Song& song) {
    return nlohmann::json{{"paths", paths_json({&path}, song.timing())}};
}

void save_path_file(const std::string& file, const Path& path, const Song& song) {
    if (!app::write_file_replacing(std::filesystem::u8path(file), path_file_json(path, song).dump(2) + "\n",
                                   app::LineEnds::AsGiven))
        throw std::runtime_error("cannot write the path file: " + file);
}

std::vector<Path> read_path_file(const std::string& file, const OpenChart& chart) {
    std::ifstream in(os_path(file), std::ios::binary);
    if (!in) throw std::runtime_error("cannot read the path file: " + file);
    nlohmann::json doc;
    try {
        in >> doc;
    } catch (const std::exception&) {
        throw std::runtime_error("the path file " + file + " is not valid JSON");
    }
    if (!doc.is_object() || !doc.contains("paths") || !doc["paths"].is_array() ||
        doc["paths"].empty())
        throw std::runtime_error("the path file " + file + " has no \"paths\" array with a path in it");

    std::vector<Path> out;
    for (size_t i = 0; i < doc["paths"].size(); ++i) {
        const std::vector<PinnedWindow> pins =
            pinned_windows(chart.song, windows_from_json(doc["paths"][i]));
        const TargetResult found = search_target(*chart.graph, pins);
        if (found.paths.empty())
            throw std::invalid_argument("path " + std::to_string(i + 1) + " of " + file +
                                        " does not fit this chart: its window at tick " +
                                        std::to_string(found.failed_tick.value_or(-1)) +
                                        " broke (" + found.failed_reason + ")");
        out.push_back(found.paths.front());
    }
    return out;
}

std::optional<Refusal> open_path_file(PathHistory& history, std::vector<Path>& branches,
                                      const OpenChart& chart, const std::string& file) {
    try {
        std::vector<Path> read = read_path_file(file, chart);
        // Built before anything changes, so a refusal leaves both alone.
        PlayerPath next = player_copy_of(read.front());
        history.edit(std::move(next));
        branches.insert(branches.end(), read.begin(), read.end());
    } catch (const std::exception& e) {
        return refusal_for(e);
    }
    return std::nullopt;
}

}  // namespace hydra::pathbuilder
