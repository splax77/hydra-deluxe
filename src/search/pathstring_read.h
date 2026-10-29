// Reading Hydra's path text back into a path. Path::pathstring and
// Path::pathstring_verbose write the text; this finds the path it names on one chart. The text carries no ticks,
// so the engine places each window: see paths_from_pathstring below.

#ifndef HYDRA_SEARCH_PATHSTRING_READ_H
#define HYDRA_SEARCH_PATHSTRING_READ_H

#include <string>

#include "core/model.h"
#include "core/song.h"
#include "search/pather.h"

namespace hydra {

// The path on `song` whose Path::pathstring is `text`. Each token is one
// window, as Activation::notationstr writes it. For each token in turn, the
// song's fills after the previous window's activation are tried in chart
// order as the next activation: search_target runs on the windows already
// placed, each pinned in full (pinned_windows), plus that fill pinned by its
// activation alone. The first fill whose window prints the token is taken,
// with the end the returned path gives it. The answer is search_target's
// path for every window pinned in full. Path::pathstring's text for a path
// with no windows reads back as that path.
//
// It also reads the long text Path::pathstring_verbose writes (Hydra's Copy
// path). Text holding the writer's section separator is the long form; only
// its windows section is read, and the multiplier-squeeze and score sections
// are ignored. There each window is matched the same way, comparing the
// candidate's Activation::notationstr_verbose with the text where the
// previous window ended. Throws std::invalid_argument for empty text, for long
// text whose section count is not the writer's, and for a window no fill can
// place, naming its position.
Path paths_from_pathstring(const Song& song, const SearchSettings& settings,
                           const std::string& text);

}  // namespace hydra

#endif  // HYDRA_SEARCH_PATHSTRING_READ_H
