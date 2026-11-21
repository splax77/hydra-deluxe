// Tests for parse/text_decode.{h,cpp}: how chart text bytes become UTF-8.

#include "doctest.h"

#include <ostream>  // doctest prints a std::string_view through operator<<
#include <string>
#include <string_view>

#include "core/winstr.h"  // wide_to_utf8
#include "parse/text_decode.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>  // MultiByteToWideChar, Windows' code page 1252 decoder

using hydra::decode_chart_text;
using hydra::decode_chart_text_view;
using hydra::is_valid_utf8;

namespace {

// The bytes as a string, so a test can write any byte, NULs included.
std::string bytes(std::initializer_list<unsigned char> b) {
    return std::string(b.begin(), b.end());
}

}  // namespace

TEST_CASE("text_decode: a Windows-1252 name reads as UTF-8") {
    // "Szamár Madár" from a real song.ini, á = 0xE1.
    const std::string raw =
        bytes({0x53, 0x7A, 0x61, 0x6D, 0xE1, 0x72, 0x20, 0x4D, 0x61, 0x64, 0xE1, 0x72});
    CHECK(decode_chart_text(raw) == "Szam\xC3\xA1r Mad\xC3\xA1r");
}

TEST_CASE("text_decode: valid UTF-8 is kept as it is") {
    const std::string text = "caf\xC3\xA9 \xC3\xB8 \xE6\x97\xA5\xE6\x9C\xAC \xF0\x9F\x8E\xB8";
    CHECK(decode_chart_text(text) == text);
    std::string storage;
    const std::string_view view = decode_chart_text_view(text, storage);
    CHECK(view == text);
    CHECK(view.data() == text.data());  // no copy
}

TEST_CASE("text_decode: a UTF-8 byte-order mark is dropped") {
    CHECK(decode_chart_text("\xEF\xBB\xBF[song]") == "[song]");
    CHECK(decode_chart_text("\xEF\xBB\xBF") == "");
}

TEST_CASE("text_decode: UTF-16 with a byte-order mark becomes UTF-8") {
    // "é日" then U+1F3B8 as a surrogate pair.
    const std::string le = bytes({0xFF, 0xFE, 0xE9, 0x00, 0xE5, 0x65, 0x3C, 0xD8, 0xB8, 0xDF});
    const std::string be = bytes({0xFE, 0xFF, 0x00, 0xE9, 0x65, 0xE5, 0xD8, 0x3C, 0xDF, 0xB8});
    const std::string want = "\xC3\xA9\xE6\x97\xA5\xF0\x9F\x8E\xB8";
    CHECK(decode_chart_text(le) == want);
    CHECK(decode_chart_text(be) == want);
}

TEST_CASE("text_decode: broken UTF-16 still gives valid UTF-8") {
    // A lone high surrogate, then an odd trailing byte.
    const std::string le = bytes({0xFF, 0xFE, 0x3C, 0xD8, 0x41, 0x00, 0x42});
    const std::string out = decode_chart_text(le);
    CHECK(out == "\xEF\xBF\xBD" "A\xEF\xBF\xBD");
    CHECK(is_valid_utf8(out));
}

TEST_CASE("text_decode: every single byte reads as Windows' own code page 1252 reads it") {
    for (int b = 0x00; b <= 0xFF; ++b) {
        CAPTURE(b);
        const char in = static_cast<char>(b);
        // Windows' decoder for code page 1252. A byte the code page leaves
        // undefined is refused, and then keeps its own code point.
        wchar_t wide[4] = {};
        const int n = MultiByteToWideChar(1252, MB_ERR_INVALID_CHARS, &in, 1, wide, 4);
        const std::wstring want_wide = n > 0 ? std::wstring(wide, static_cast<size_t>(n))
                                             : std::wstring(1, static_cast<wchar_t>(b));
        CHECK(decode_chart_text(std::string(1, in)) == hydra::wide_to_utf8(want_wide));
    }
}

TEST_CASE("text_decode: one invalid byte sends the whole text to Windows-1252") {
    // A valid UTF-8 é (C3 A9) and a lone Windows-1252 é (E9): every high byte
    // is then read as Windows-1252, the valid pair too.
    const std::string raw = bytes({0xC3, 0xA9, 0x20, 0xE9});
    CHECK(decode_chart_text(raw) == "\xC3\x83\xC2\xA9 \xC3\xA9");
}

TEST_CASE("text_decode: empty input stays empty") {
    CHECK(decode_chart_text(std::string_view()) == "");
    CHECK(is_valid_utf8(""));
}

TEST_CASE("text_decode: is_valid_utf8 refuses overlongs, surrogates and past U+10FFFF") {
    CHECK_FALSE(is_valid_utf8(bytes({0xC0, 0xAF})));              // overlong '/'
    CHECK_FALSE(is_valid_utf8(bytes({0xE0, 0x80, 0xAF})));        // overlong
    CHECK_FALSE(is_valid_utf8(bytes({0xED, 0xA0, 0x80})));        // U+D800
    CHECK_FALSE(is_valid_utf8(bytes({0xF4, 0x90, 0x80, 0x80})));  // U+110000
    CHECK_FALSE(is_valid_utf8(bytes({0xE6, 0x97})));              // cut short
    CHECK(is_valid_utf8(bytes({0xF4, 0x8F, 0xBF, 0xBF})));        // U+10FFFF
    CHECK(is_valid_utf8(bytes({0xED, 0x9F, 0xBF})));              // U+D7FF
}
