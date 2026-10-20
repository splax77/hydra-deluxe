// Chart text as UTF-8: the one rule for how the bytes of a song.ini, a .chart,
// a MIDI text event or a container's metadata string become the text Hydra
// keeps and shows.
//
// A byte-order mark decides first (UTF-8, UTF-16LE or UTF-16BE, the mark
// itself dropped). Without one, bytes that are already valid UTF-8 are kept as
// they are, and anything else is read as Windows-1252. This is the order
// YARG.Core's YARGTextReader follows, with Windows-1252 in place of its
// ISO-8859-1, as the WHATWG Encoding standard reads "latin1". The result is
// always valid UTF-8, so the UI never meets a byte it cannot draw.

#ifndef HYDRA_PARSE_TEXT_DECODE_H
#define HYDRA_PARSE_TEXT_DECODE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace hydra {

// Whether `bytes` is well-formed UTF-8: no overlong forms, no surrogate code
// points and nothing above U+10FFFF.
bool is_valid_utf8(std::string_view bytes);

// `bytes` decoded as chart text (the rule at the top of this file). Decide
// per unit the file format treats as one text: a whole song.ini or .chart,
// one MIDI text event, one metadata string.
std::string decode_chart_text(std::string_view bytes);

inline std::string decode_chart_text(const uint8_t* bytes, size_t len) {
    return decode_chart_text(std::string_view(reinterpret_cast<const char*>(bytes), len));
}

// The same text as decode_chart_text, without a copy when none is needed:
// valid UTF-8 comes back as a view into `bytes` (past any UTF-8 mark), and
// anything else is decoded into `storage` and viewed there. The view lives as
// long as both `bytes` and `storage` do.
std::string_view decode_chart_text_view(std::string_view bytes, std::string& storage);

}  // namespace hydra

#endif  // HYDRA_PARSE_TEXT_DECODE_H
