#include "parse/text_decode.h"

#include "core/strutil.h"  // starts_with

namespace hydra {

namespace {

// Byte-order marks.
constexpr std::string_view kUtf8Bom = "\xEF\xBB\xBF";
constexpr std::string_view kUtf16LeBom = "\xFF\xFE";
constexpr std::string_view kUtf16BeBom = "\xFE\xFF";

// What a broken UTF-16 unit (a lone surrogate or a trailing odd byte) reads as.
constexpr uint32_t kReplacementChar = 0xFFFD;

// The code points Windows-1252 gives bytes 0x80..0x9F. The five bytes it
// leaves undefined (0x81, 0x8D, 0x8F, 0x90, 0x9D) keep their C1 code point,
// as the WHATWG table does. Every other byte is its own code point.
constexpr uint16_t kCp1252High[32] = {
    0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
    0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178,
};

void append_utf8(std::string& out, uint32_t cp) {
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

std::string from_cp1252(std::string_view bytes) {
    std::string out;
    out.reserve(bytes.size() + bytes.size() / 2);
    for (char ch : bytes) {
        const auto b = static_cast<unsigned char>(ch);
        if (b >= 0x80 && b <= 0x9F) append_utf8(out, kCp1252High[b - 0x80]);
        else append_utf8(out, b);
    }
    return out;
}

// UTF-16 (after its mark) to UTF-8. A surrogate without its partner and an
// odd last byte each read as U+FFFD, so the result is always valid.
std::string from_utf16(std::string_view bytes, bool big_endian) {
    const auto unit_at = [&](size_t i) -> uint32_t {
        const auto a = static_cast<unsigned char>(bytes[i]);
        const auto b = static_cast<unsigned char>(bytes[i + 1]);
        return big_endian ? (uint32_t{a} << 8) | b : (uint32_t{b} << 8) | a;
    };
    std::string out;
    out.reserve(bytes.size());
    size_t i = 0;
    while (i + 1 < bytes.size()) {
        const uint32_t u = unit_at(i);
        i += 2;
        if (u >= 0xD800 && u <= 0xDBFF && i + 1 < bytes.size()) {
            const uint32_t low = unit_at(i);
            if (low >= 0xDC00 && low <= 0xDFFF) {
                i += 2;
                append_utf8(out, 0x10000 + ((u - 0xD800) << 10) + (low - 0xDC00));
                continue;
            }
        }
        append_utf8(out, (u >= 0xD800 && u <= 0xDFFF) ? kReplacementChar : u);
    }
    if (i < bytes.size()) append_utf8(out, kReplacementChar);
    return out;
}

}  // namespace

bool is_valid_utf8(std::string_view bytes) {
    const size_t n = bytes.size();
    size_t i = 0;
    const auto at = [&](size_t k) { return static_cast<unsigned char>(bytes[k]); };
    const auto cont = [&](size_t k) { return k < n && (at(k) & 0xC0) == 0x80; };
    while (i < n) {
        const unsigned char b = at(i);
        if (b < 0x80) {
            ++i;
            continue;
        }
        // The lead byte fixes the length and the allowed range of the second
        // byte, which is what rules out overlongs, surrogates and anything
        // past U+10FFFF (the Unicode standard's table of well-formed bytes).
        size_t len = 0;
        unsigned char lo = 0x80, hi = 0xBF;
        if (b >= 0xC2 && b <= 0xDF) len = 2;
        else if (b == 0xE0) { len = 3; lo = 0xA0; }
        else if (b >= 0xE1 && b <= 0xEC) len = 3;
        else if (b == 0xED) { len = 3; hi = 0x9F; }
        else if (b >= 0xEE && b <= 0xEF) len = 3;
        else if (b == 0xF0) { len = 4; lo = 0x90; }
        else if (b >= 0xF1 && b <= 0xF3) len = 4;
        else if (b == 0xF4) { len = 4; hi = 0x8F; }
        else return false;
        if (i + 1 >= n || at(i + 1) < lo || at(i + 1) > hi) return false;
        for (size_t k = 2; k < len; ++k)
            if (!cont(i + k)) return false;
        i += len;
    }
    return true;
}

std::string_view decode_chart_text_view(std::string_view bytes, std::string& storage) {
    if (starts_with(bytes, kUtf8Bom)) {
        // The mark claims UTF-8; bytes that break that claim still read as
        // Windows-1252, so the result stays valid.
        const std::string_view rest = bytes.substr(kUtf8Bom.size());
        if (is_valid_utf8(rest)) return rest;
        storage = from_cp1252(rest);
        return storage;
    }
    if (starts_with(bytes, kUtf16LeBom)) {
        storage = from_utf16(bytes.substr(kUtf16LeBom.size()), false);
        return storage;
    }
    if (starts_with(bytes, kUtf16BeBom)) {
        storage = from_utf16(bytes.substr(kUtf16BeBom.size()), true);
        return storage;
    }
    if (is_valid_utf8(bytes)) return bytes;
    storage = from_cp1252(bytes);
    return storage;
}

std::string decode_chart_text(std::string_view bytes) {
    std::string storage;
    const std::string_view text = decode_chart_text_view(bytes, storage);
    if (text.data() == storage.data()) return std::move(storage);
    return std::string(text);
}

}  // namespace hydra
