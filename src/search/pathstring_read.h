// Reading Hydra's path text back into a path. Path::pathstring writes the
// text; this finds the path it names on one chart. The text carries no ticks,
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
// with no windows reads back as that path. Throws std::invalid_argument for
// empty text, and for a token no fill can place, naming its position.
Path paths_from_pathstring(const Song& song, const SearchSettings& settings,
                           const std::string& text);

}  // namespace hydra

#endif  // HYDRA_SEARCH_PATHSTRING_READ_H
