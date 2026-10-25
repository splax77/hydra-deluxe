#include "decisions.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "code_lexer.h"
#include "core/strutil.h"
#include "text.h"

namespace precheck {

using hydra::ends_with_ci;
using hydra::equals_ci;
using hydra::is_ascii_space;
using hydra::lower_ascii;
using hydra::starts_with_ci;

namespace {

// The bound below which number_canon writes a whole number as an integer.
// It is the bound the PowerShell precheck's number key used, kept so a key
// spelled then and now is the same text.
constexpr double kWholeNumberLimit = 1e15;

// The most significant hex digits a 64-bit value holds (four bits each). A
// longer 0x number spells no value, as the PowerShell precheck's
// [Convert]::ToInt64 refused it.
constexpr size_t kMaxHexDigits = 16;

// The level plan_decision_paras gives a "User decisions" bold block: deeper
// than any heading, so only a bold line or a heading ends it. The PowerShell
// precheck used the same marker level.
constexpr size_t kBoldLevel = 99;

bool digit(char c) { return c >= '0' && c <= '9'; }
bool hex_digit(char c) { return digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
char at(const std::string& s, size_t i) { return i < s.size() ? s[i] : '\0'; }
size_t skip_digits(const std::string& s, size_t i) {
    while (i < s.size() && digit(s[i])) ++i;
    return i;
}

// Where a doc number that starts at `i` (a digit) ends: hex, a number with
// thousands groups, or a plain decimal with an optional exponent, tried in
// that order, the first that fits winning.
size_t doc_number_end(const std::string& s, size_t i) {
    if (s[i] == '0' && (at(s, i + 1) == 'x' || at(s, i + 1) == 'X') && hex_digit(at(s, i + 2))) {
        size_t j = i + 2;
        while (j < s.size() && hex_digit(s[j])) ++j;
        return j;
    }
    // Thousands groups: one to three digits, then groups of a separator and
    // three digits; a decimal part may follow; no digit may follow.
    const size_t lead = skip_digits(s, i);
    if (lead - i <= 3) {
        std::vector<size_t> group_ends;
        size_t p = lead;
        for (;;) {
            const char sep = at(s, p);
            if (sep != ',' && sep != '\'' && sep != ' ') break;
            if (!(digit(at(s, p + 1)) && digit(at(s, p + 2)) && digit(at(s, p + 3)))) break;
            p += 4;
            group_ends.push_back(p);
        }
        if (!group_ends.empty()) {
            const size_t last = group_ends.back();
            if (at(s, last) == '.' && digit(at(s, last + 1))) return skip_digits(s, last + 1);
            if (!digit(at(s, last))) return last;
            // The last group ran into a fourth digit; one group fewer ends at
            // a separator, which no digit is.
            if (group_ends.size() >= 2) return group_ends[group_ends.size() - 2];
        }
    }
    size_t j = lead;
    if (at(s, j) == '.' && digit(at(s, j + 1))) j = skip_digits(s, j + 1);
    if (at(s, j) == 'e' || at(s, j) == 'E') {
        size_t k = j + 1;
        if (at(s, k) == '-' || at(s, k) == '+') ++k;
        if (digit(at(s, k))) j = skip_digits(s, k);
    }
    return j;
}

}  // namespace

bool word_byte(char c) { return codelex::detail::ident_char(c) && static_cast<unsigned char>(c) < 0x80; }

bool word_byte_before(const std::string& s, size_t pos) { return pos > 0 && word_byte(s[pos - 1]); }

std::string number_canon(double v) {
    v = std::fabs(v);
    if (v == std::floor(v) && v < kWholeNumberLimit) return std::to_string(static_cast<long long>(v));
    char buf[64];
    std::snprintf(buf, sizeof buf, "%.10G", v);
    return buf;
}

std::optional<double> number_of(const std::string& text) {
    std::string t;
    for (const char c : text)
        if (c != ',' && c != '\'' && c != ' ' && c != '_') t += c;
    if (t.size() > 2 && t[0] == '0' && (t[1] == 'x' || t[1] == 'X') && hex_digit(t[2])) {
        size_t i = 2;
        while (i < t.size() && t[i] == '0') ++i;
        size_t j = i;
        while (j < t.size() && hex_digit(t[j])) ++j;
        if (j - i > kMaxHexDigits) return std::nullopt;
        uint64_t u = 0;
        for (size_t k = i; k < j; ++k) {
            const char c = t[k];
            u = u * 16 + static_cast<uint64_t>(digit(c) ? c - '0' : lower_ascii(c) - 'a' + 10);
        }
        return static_cast<double>(static_cast<int64_t>(u));
    }
    while (!t.empty() && std::string_view("uUlLfF").find(t.back()) != std::string_view::npos) t.pop_back();
    return hydra::parse_finite_number(t);
}

std::set<std::string> doc_numbers(const std::string& text) {
    std::set<std::string> out;
    size_t i = 0;
    while (i < text.size()) {
        // A number may follow an "x" (2x40) or any byte that is not a word
        // byte or a dot.
        const bool starts = digit(text[i]) && (i == 0 || text[i - 1] == 'x' ||
                                               !(word_byte(text[i - 1]) || text[i - 1] == '.'));
        if (!starts) {
            ++i;
            continue;
        }
        const size_t end = doc_number_end(text, i);
        if (const auto n = number_of(text.substr(i, end - i))) out.insert(number_canon(*n));
        i = end;
    }
    // "2^N" names the power.
    for (size_t i2 = 0; i2 < text.size(); ++i2) {
        if (text[i2] != '2' || word_byte_before(text, i2)) continue;
        size_t p = skip_space(text, i2 + 1);
        if (at(text, p) != '^') continue;
        p = skip_space(text, p + 1);
        const size_t e = skip_digits(text, p);
        if (e == p || e - p > 2 || word_byte(at(text, e))) continue;
        out.insert(number_canon(std::pow(2.0, std::stoi(text.substr(p, e - p)))));
        i2 = e - 1;
    }
    return out;
}

std::vector<std::string> split_paras(const std::string& text, bool whole_lists) {
    std::vector<std::string> pieces;
    size_t from = 0;
    size_t i = 0;
    while (i < text.size()) {
        if (text[i] != '\n') {
            ++i;
            continue;
        }
        // A blank line: this break, any whitespace, and the last break in it.
        const size_t run_end = skip_space(text, i + 1);
        size_t last_break = std::string::npos;
        for (size_t k = i + 1; k < run_end; ++k)
            if (text[k] == '\n') last_break = k;
        size_t sep_end = std::string::npos;
        if (last_break != std::string::npos) {
            sep_end = last_break + 1;
        } else if (!whole_lists) {
            // A list item next: whitespace, then "12." or "-" or "*", then
            // whitespace. Only the break itself is cut.
            size_t p = run_end;
            const size_t d = skip_digits(text, p);
            bool marker = false;
            if (d > p && at(text, d) == '.') {
                marker = true;
                p = d + 1;
            } else if (at(text, p) == '-' || at(text, p) == '*') {
                marker = true;
                p = p + 1;
            }
            if (marker && p < text.size() && is_ascii_space(text[p])) sep_end = i + 1;
        }
        if (sep_end == std::string::npos) {
            ++i;
            continue;
        }
        pieces.push_back(text.substr(from, i - from));
        from = sep_end;
        i = sep_end;
    }
    pieces.push_back(text.substr(from));
    std::vector<std::string> out;
    for (std::string& p : pieces)
        if (!hydra::trim_view(p).empty()) out.push_back(std::move(p));
    return out;
}

std::string record_name(bool adr, const std::string& digits) {
    size_t i = 0;
    while (i + 1 < digits.size() && digits[i] == '0') ++i;
    const std::string n = digits.substr(i);
    return adr ? "ADR " + n : "D" + n;
}

DecisionPara make_para(const std::string& record, const std::string& text) {
    DecisionPara p;
    p.record = record;
    const std::string low = hydra::to_lower_ascii(text);
    size_t i = 0;
    while (i < low.size()) {
        if (low[i] < 'a' || low[i] > 'z') {
            ++i;
            continue;
        }
        size_t j = i + 1;
        while (j < low.size() && ((low[j] >= 'a' && low[j] <= 'z') || digit(low[j]))) ++j;
        p.words.insert(low.substr(i, j - i));
        i = j;
    }
    p.numbers = doc_numbers(text);
    return p;
}

namespace {

// The run of '#' a line starts with, and whether whitespace follows it.
size_t heading_level(const std::string& l, bool& space_after) {
    size_t n = 0;
    while (n < l.size() && l[n] == '#') ++n;
    space_after = n > 0 && n < l.size() && is_ascii_space(l[n]);
    return n;
}

// Does the line start "**<text>**" with no "*" inside, at least one byte of
// text, and (when `not_user`) text that does not start "User decisions"?
bool bold_line(const std::string& l, bool not_user) {
    if (!starts_with_ci(l, "**")) return false;
    if (not_user && starts_with_ci(std::string_view(l).substr(2), "User decisions")) return false;
    const size_t star = l.find('*', 2);
    return star != std::string::npos && star > 2 && at(l, star + 1) == '*';
}

bool user_decisions_line(const std::string& l) {
    if (!starts_with_ci(l, "**User decisions")) return false;
    const size_t star = l.find('*', 16);
    return star != std::string::npos && at(l, star + 1) == '*';
}

bool test_limits_line(const std::string& l, size_t& level) {
    bool space = false;
    level = heading_level(l, space);
    if (!space) return false;
    return starts_with_ci(std::string_view(l).substr(skip_space(l, level)), "Test limits");
}

// The D-record a paragraph opens ("**D49 ..."), or nothing.
std::optional<std::string> opens_record(const std::string& p) {
    const size_t s = skip_space(p, 0);
    if (!starts_with_ci(std::string_view(p).substr(s), "**D")) return std::nullopt;
    const size_t d = s + 3;
    const size_t e = skip_digits(p, d);
    if (e == d || word_byte(at(p, e))) return std::nullopt;
    return record_name(false, p.substr(d, e - d));
}

}  // namespace

std::vector<std::string> plan_decision_paras(const std::string& text) {
    std::vector<std::string> out;
    std::vector<std::string> buf;
    bool in = false;
    size_t level = 0;
    auto flush = [&] {
        std::string joined;
        for (size_t k = 0; k < buf.size(); ++k) joined += (k ? "\n" : "") + buf[k];
        for (std::string& p : split_paras(joined, false)) out.push_back(std::move(p));
        buf.clear();
    };
    size_t from = 0;
    for (;;) {
        const size_t nl = text.find('\n', from);
        const std::string l = text.substr(from, nl == std::string::npos ? std::string::npos : nl - from);
        size_t heading = 0;
        if (user_decisions_line(l)) {
            in = true;
            level = kBoldLevel;
            buf.push_back(l);
        } else if (test_limits_line(l, heading)) {
            in = true;
            level = heading;
            buf.push_back(l);
        } else if (in) {
            bool space = false;
            const size_t n = heading_level(l, space);
            const bool end_bold = level == kBoldLevel && (bold_line(l, true) || n > 0);
            const bool end_head = level != kBoldLevel && space && n <= level;
            if (end_bold || end_head) {
                in = false;
                flush();
            } else {
                buf.push_back(l);
            }
        }
        if (nl == std::string::npos) break;
        from = nl + 1;
    }
    if (!buf.empty()) flush();
    return out;
}

std::vector<DecisionPara> decision_paras(Repo& repo, const std::vector<std::string>& revs) {
    std::vector<DecisionPara> paras;
    std::vector<std::string> seen;
    for (const std::string& rev : revs) {
        if (rev.empty()) continue;
        bool repeat = false;
        for (const std::string& s : seen) repeat = repeat || s == rev;
        if (repeat) continue;
        seen.push_back(rev);
        const std::vector<std::string> docs = repo.git(
            {"ls-tree", "-r", "--name-only", rev, "--", "docs/adr", "CONTEXT.md", "docs/superpowers/plans", "docs/audit"});
        for (const std::string& d : docs) {
            if (!ends_with_ci(d, ".md")) continue;
            if (starts_with_ci(d, "docs/adr/") || equals_ci(d, "CONTEXT.md")) {
                // An ADR is named by its number; CONTEXT.md by its own name.
                std::string rec = d;
                const size_t slash = d.rfind('/');
                if (slash != std::string::npos && slash + 4 < d.size() && digit(d[slash + 1]) &&
                    digit(d[slash + 2]) && digit(d[slash + 3]) && digit(d[slash + 4]))
                    rec = record_name(true, d.substr(slash + 1, 4));
                for (const std::string& p : split_paras(repo.files().text(rev, d), false))
                    paras.push_back(make_para(rec, p));
                continue;
            }
            const std::string_view plans = "docs/superpowers/plans/";
            if (starts_with_ci(d, plans) && d.find('/', plans.size()) == std::string::npos &&
                d.size() - plans.size() >= 4) {
                for (const std::string& p : plan_decision_paras(repo.files().text(rev, d)))
                    paras.push_back(make_para("plan " + d, p));
                continue;
            }
            const std::string_view audit = "docs/audit/";
            if (starts_with_ci(d, audit) && d.find('/', audit.size()) == std::string::npos) {
                const std::string stem = hydra::to_lower_ascii(d.substr(audit.size(), d.size() - audit.size() - 3));
                if (stem.find("decisions") == std::string::npos) continue;
                std::optional<std::string> rec;
                for (const std::string& p : split_paras(repo.files().text(rev, d), true)) {
                    if (const auto opened = opens_record(p)) {
                        rec = opened;
                    } else {
                        const size_t s = skip_space(p, 0);
                        if (at(p, s) == '#') rec.reset();
                    }
                    if (rec) paras.push_back(make_para(*rec, p));
                }
            }
        }
    }
    return paras;
}

}  // namespace precheck
