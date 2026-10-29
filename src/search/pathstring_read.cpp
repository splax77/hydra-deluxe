#include "search/pathstring_read.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace hydra {

namespace {

// The text split at whitespace: one token per window.
std::vector<std::string> tokens_of(const std::string& text) {
    std::istringstream in(text);
    std::vector<std::string> out;
    for (std::string token; in >> token;) out.push_back(token);
    return out;
}

// The ticks of the song's fills, in chart order: the chords a fill rides on
// (SongTimestamp::has_activation), which are the ticks search_target pins.
std::vector<int64_t> fill_ticks(const Song& song) {
    std::vector<int64_t> out;
    for (const SongTimestamp& ts : song.sequence)
        if (ts.has_activation()) out.push_back(ts.timecode.ticks());
    return out;
}

// The first path search_target returns, tied variants included, whose last
// window prints `token`; empty when none does.
std::optional<Path> window_printing(const std::vector<Path>& paths, const std::string& token) {
    for (const Path* p : flatten_paths(paths)) {
        const ActivationWalk acts = p->walk_activations();
        if (!acts.empty() && acts.back().notationstr() == token) return *p;
    }
    return std::nullopt;
}

}  // namespace

Path paths_from_pathstring(const Song& song, const SearchSettings& settings,
                           const std::string& text) {
    const std::vector<std::string> tokens = tokens_of(text);
    if (tokens.empty()) throw std::invalid_argument("the path text is empty");

    // Path::pathstring's text for a path with no windows (asked of the writer)
    // places none.
    std::vector<PinnedWindow> placed;
    if (tokens != tokens_of(Path().pathstring())) {
        const std::vector<int64_t> fills = fill_ticks(song);
        size_t next_fill = 0;
        for (size_t k = 0; k < tokens.size(); ++k) {
            std::optional<Path> found;
            for (; next_fill < fills.size() && !found; ++next_fill) {
                std::vector<PinnedWindow> windows = placed;
                windows.push_back(PinnedWindow{fills[next_fill]});
                found = window_printing(search_target(song, settings, windows).paths, tokens[k]);
            }
            if (!found)
                throw std::invalid_argument("window " + std::to_string(k + 1) + " (\"" +
                                            tokens[k] + "\") matches no fill on this chart");
            placed = pinned_windows(*found);
        }
    }

    const TargetResult whole = search_target(song, settings, placed);
    if (whole.paths.empty())
        throw std::logic_error("the windows read from the path text did not come back pinned in "
                               "full: the window at tick " +
                               std::to_string(whole.failed_tick.value_or(-1)) + " broke (" +
                               whole.failed_reason + ")");
    return whole.paths.front();
}

}  // namespace hydra
