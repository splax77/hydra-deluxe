#include "search/pathstring_read.h"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>
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

bool is_space(char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; }

// `s` without the whitespace at either end.
std::string_view trimmed(std::string_view s) {
    while (!s.empty() && is_space(s.front())) s.remove_prefix(1);
    while (!s.empty() && is_space(s.back())) s.remove_suffix(1);
    return s;
}

// The text cut at every section separator; one piece when it has none.
std::vector<std::string_view> sections_of(std::string_view text) {
    std::vector<std::string_view> out;
    for (size_t at; (at = text.find(kPathSectionSeparator)) != std::string_view::npos;) {
        out.push_back(text.substr(0, at));
        text.remove_prefix(at + kPathSectionSeparator.size());
    }
    out.push_back(text);
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
// window `prints` accepts; empty when none does.
template <class Accepts>
std::optional<Path> window_printing(const std::vector<Path>& paths, const Accepts& prints) {
    for (const Path* p : flatten_paths(paths)) {
        const ActivationWalk acts = p->walk_activations();
        if (!acts.empty() && prints(acts.back())) return *p;
    }
    return std::nullopt;
}

// The windows after `placed` once one more is placed: the song's fills from
// `next_fill` on are tried in chart order as its activation, and the first
// whose window `prints` accepts is taken, every window then pinned in full.
// `next_fill` moves past the fills tried. Empty when no fill's window is
// accepted.
template <class Accepts>
std::optional<std::vector<PinnedWindow>> place_next(const Song& song,
                                                    const SearchSettings& settings,
                                                    const std::vector<int64_t>& fills,
                                                    size_t& next_fill,
                                                    const std::vector<PinnedWindow>& placed,
                                                    const Accepts& prints) {
    for (; next_fill < fills.size(); ++next_fill) {
        std::vector<PinnedWindow> windows = placed;
        windows.push_back(PinnedWindow{fills[next_fill]});
        const std::optional<Path> found =
            window_printing(search_target(song, settings, windows).paths, prints);
        if (found) {
            ++next_fill;
            return pinned_windows(*found);
        }
    }
    return std::nullopt;
}

// The windows Path::pathstring's text names, each token compared with
// Activation::notationstr.
std::vector<PinnedWindow> read_short(const Song& song, const SearchSettings& settings,
                                     const std::string& text) {
    const std::vector<std::string> tokens = tokens_of(text);
    std::vector<PinnedWindow> placed;
    // Path::pathstring's text for a path with no windows (asked of the writer)
    // places none.
    if (tokens == tokens_of(Path().pathstring())) return placed;

    const std::vector<int64_t> fills = fill_ticks(song);
    size_t next_fill = 0;
    for (size_t k = 0; k < tokens.size(); ++k) {
        std::optional<std::vector<PinnedWindow>> next = place_next(
            song, settings, fills, next_fill, placed,
            [&](const Activation& act) { return act.notationstr() == tokens[k]; });
        if (!next)
            throw std::invalid_argument("window " + std::to_string(k + 1) + " (\"" + tokens[k] +
                                        "\") matches no fill on this chart");
        placed = std::move(*next);
    }
    return placed;
}

// The windows section of Path::pathstring_verbose's text. Its place among the
// sections, and how many sections there are, are read from the writer's text
// for a path with no windows: the windows section is the one that matches
// Path::pathstring's text for no windows.
std::string_view windows_section(std::string_view text) {
    const std::string none_long = Path().pathstring_verbose({});
    const std::vector<std::string_view> shape = sections_of(none_long);
    const std::string none = Path().pathstring();
    size_t windows_at = shape.size();
    for (size_t i = 0; i < shape.size(); ++i)
        if (shape[i] == none) windows_at = i;
    if (windows_at == shape.size())
        throw std::logic_error("Path::pathstring_verbose's text for a path with no windows has no "
                               "section that reads \"" + none + "\"");

    const std::vector<std::string_view> sections = sections_of(text);
    if (sections.size() != shape.size())
        throw std::invalid_argument("the path text has " + std::to_string(sections.size()) +
                                    " sections where Hydra's long Copy path text has " +
                                    std::to_string(shape.size()));
    return trimmed(sections[windows_at]);
}

// The windows Path::pathstring_verbose's windows section names. The tokens
// hold spaces of their own, so the section is not split: each window is the
// one whose Activation::notationstr_verbose is the text at the front of what
// is left, followed by whitespace or the end.
std::vector<PinnedWindow> read_long(const Song& song, const SearchSettings& settings,
                                    std::string_view windows) {
    std::vector<PinnedWindow> placed;
    if (windows == Path().pathstring()) return placed;
    if (windows.empty()) throw std::invalid_argument("the path text's windows section is empty");

    const std::vector<int64_t> fills = fill_ticks(song);
    size_t next_fill = 0;
    std::string_view rest = windows;
    for (size_t k = 1; !rest.empty(); ++k) {
        size_t printed_size = 0;
        std::optional<std::vector<PinnedWindow>> next =
            place_next(song, settings, fills, next_fill, placed, [&](const Activation& act) {
                const std::string printed = act.notationstr_verbose();
                const bool fits = rest.substr(0, printed.size()) == printed &&
                                  (rest.size() == printed.size() || is_space(rest[printed.size()]));
                if (fits) printed_size = printed.size();
                return fits;
            });
        if (!next)
            throw std::invalid_argument("window " + std::to_string(k) + " (at \"" +
                                        std::string(rest) + "\") matches no fill on this chart");
        placed = std::move(*next);
        rest = trimmed(rest.substr(printed_size));
    }
    return placed;
}

}  // namespace

Path paths_from_pathstring(const Song& song, const SearchSettings& settings,
                           const std::string& text) {
    if (tokens_of(text).empty()) throw std::invalid_argument("the path text is empty");

    const std::vector<PinnedWindow> placed =
        sections_of(text).size() == 1 ? read_short(song, settings, text)
                                       : read_long(song, settings, windows_section(text));

    const TargetResult whole = search_target(song, settings, placed);
    if (whole.paths.empty())
        throw std::logic_error("the windows read from the path text did not come back pinned in "
                               "full: the window at tick " +
                               std::to_string(whole.failed_tick.value_or(-1)) + " broke (" +
                               whole.failed_reason + ")");
    return whole.paths.front();
}

}  // namespace hydra
