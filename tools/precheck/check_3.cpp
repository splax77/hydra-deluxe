// Check 3: new numbers with no decision (kind D). Each skip and test below
// says why beside it; decisions.h owns the docs the numbers are compared
// against.
#include <algorithm>
#include <map>
#include <optional>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "checks.h"
#include "core/strutil.h"
#include "decisions.h"
#include "definitions.h"
#include "text.h"         // skip_space
#include "views.h"

namespace precheck {

namespace {

using hydra::is_ascii_space;

bool digit(char c) { return c >= '0' && c <= '9'; }
bool hex_or_quote(char c) {
    return digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || c == '\'';
}
char at(const std::string& s, size_t i) { return i < s.size() ? s[i] : '\0'; }
bool has(const char* set, char c) { return c != '\0' && std::string_view(set).find(c) != std::string_view::npos; }

std::string trim_end(const std::string& s) {
    size_t e = s.size();
    while (e > 0 && is_ascii_space(s[e - 1])) --e;
    return s.substr(0, e);
}
std::string trim_start(const std::string& s) { return s.substr(precheck::skip_space(s, 0)); }
using hydra::ends_with;

// Words that say nothing about what a number is.
const std::set<std::string> kStopWords = {
    "const", "constexpr", "static", "inline", "auto", "int", "int64", "int32", "uint8", "uint16", "uint32", "uint64",
    "double", "float", "bool", "char", "void", "size", "std", "hydra", "string", "vector", "return", "for", "while",
    "check", "require", "cast", "static_cast", "true", "false", "the", "and", "not", "nullptr", "value", "values",
    "test", "tests", "cfg", "opts", "options", "settings", "self", "assert", "equal", "almost", "less", "greater",
    "make", "get", "set", "push", "back", "emplace", "begin", "end", "data", "out", "auto", "long", "unsigned",
    "max", "min", "abs", "fabs", "approx", "doctest", "message", "info", "else", "elif", "def", "len", "range",
    "int64_t", "size_t", "uint8_t", "new", "ptr", "ref", "tmp", "res", "ret", "val", "num", "idx", "count"};

const auto kIcase = std::regex::ECMAScript | std::regex::icase;

// A test field or constant whose name says it is a limit.
const std::regex& limit_words() {
    static const std::regex rx(
        "depth|cap|limit|filter|band|toleran|threshold|max|min|floor|ceil|seed|worker|frame|timeout|deadline|window|"
        "leeway|slop|budget|retr|repeat|probe|sample|settle|wait|sleep",
        kIcase);
    return rx;
}
// A test loop over seeds, trials, repeats...
const std::regex& loop_words() {
    static const std::regex rx("seed|trial|repeat|iteration|attempt|sample|probe|frame|retry|round|run\\b", kIcase);
    return rx;
}
// An assertion call (assert_call_pattern).
const std::regex& assertion() {
    static const std::regex rx(assert_call_pattern());
    return rx;
}
// Lines with no number to check: preprocessor lines, static_assert, Python
// imports.
const std::regex& skipped_line() {
    static const std::regex rx(
        "^#\\s*(include|pragma|error|if|ifdef|ifndef|endif|else|elif)\\b|^\\s*static_assert\\b|^(import|from)\\s",
        kIcase);
    return rx;
}
const std::regex& loop_line() {
    static const std::regex rx("^\\s*(for|while)\\s*\\(", kIcase);
    return rx;
}
const std::regex& braced_loop() {
    static const std::regex rx(":\\s*\\{", kIcase);
    return rx;
}
// Is this number a duration? It is the first argument of a time call
// (seconds(, sleep_for(, a timeout or deadline...); in a test, a unit after
// it (5ms, 2s) says so too.
const std::regex& duration_before() {
    static const std::regex rx(
        "\\b(seconds|milliseconds|minutes|microseconds|sleep_for|timeout|deadline)\\w*\\s*\\(\\s*$", kIcase);
    return rx;
}
const std::regex& unit_after() {
    static const std::regex rx("^(ms|s|min)\\b", kIcase);
    return rx;
}
const std::regex& returned() {
    static const std::regex rx("\\breturn\\s*[-+(]*$", kIcase);
    return rx;
}
const std::regex& bound() {
    static const std::regex rx("\\bstd::(min|max|clamp)\\s*(<[^>]*>)?\\s*\\(([^()]*,)?\\s*$", kIcase);
    return rx;
}
const std::regex& sample() {
    static const std::regex rx(
        "(begin\\(\\)|\\.data\\(\\))\\s*\\+$|\\bstd::(min|max)\\s*(<[^>]*>)?\\s*\\([^()]*,$", kIcase);
    return rx;
}
const std::regex& cite_d() {
    static const std::regex rx("\\bD(\\d{1,3})\\b");
    return rx;
}
const std::regex& cite_adr() {
    static const std::regex rx("\\bADR\\s*0*(\\d{1,4})\\b");
    return rx;
}

bool search(const std::string& s, const std::regex& rx) { return std::regex_search(s, rx); }

// ---------------------------------------------------------- the literals

// Where a literal's C++ suffix and the end check put it, from `p`: the
// suffix letters, then no word byte, dot or quote may follow.
size_t literal_close(const std::string& s, size_t p) {
    while (has("uUlLfF", at(s, p))) ++p;
    const char c = at(s, p);
    return (word_byte(c) || c == '.' || c == '\'') ? std::string::npos : p;
}
size_t digits_end(const std::string& s, size_t i) {
    while (digit(at(s, i))) ++i;
    return i;
}
// The ends an optional exponent can give from `p`, longest first, then `p`
// itself (no exponent).
std::vector<size_t> exponent_ends(const std::string& s, size_t p) {
    std::vector<size_t> out;
    if (at(s, p) == 'e' || at(s, p) == 'E') {
        const size_t q = p + 1;
        if (at(s, q) == '-' || at(s, q) == '+')
            for (size_t e = digits_end(s, q + 1); e > q + 1; --e) out.push_back(e);
        for (size_t e = digits_end(s, q); e > q; --e) out.push_back(e);
    }
    out.push_back(p);
    return out;
}
size_t close_after_exponent(const std::string& s, size_t p) {
    for (const size_t e : exponent_ends(s, p))
        if (const size_t end = literal_close(s, e); end != std::string::npos) return end;
    return std::string::npos;
}

// The end of a numeric literal starting at `i`, or npos: hex (digit
// separators allowed), a decimal with an optional fraction and exponent, or
// a fraction alone; each with an optional suffix. Every choice is tried
// longest first, in that order, the first that ends cleanly winning.
size_t literal_end(const std::string& s, size_t i) {
    if (s[i] == '0' && (at(s, i + 1) == 'x' || at(s, i + 1) == 'X')) {
        size_t run = i + 2;
        while (hex_or_quote(at(s, run))) ++run;
        for (size_t e = run; e > i + 2; --e)
            if (const size_t end = literal_close(s, e); end != std::string::npos) return end;
    }
    if (digit(s[i])) {
        size_t run = i + 1;
        while (digit(at(s, run)) || at(s, run) == '\'') ++run;
        for (size_t r = run; r >= i + 1; --r) {
            if (at(s, r) == '.') {
                for (size_t d = digits_end(s, r + 1); d >= r + 1; --d)
                    if (const size_t end = close_after_exponent(s, d); end != std::string::npos) return end;
            }
            if (const size_t end = close_after_exponent(s, r); end != std::string::npos) return end;
        }
        return std::string::npos;
    }
    if (s[i] == '.' && digit(at(s, i + 1))) {
        for (size_t d = digits_end(s, i + 1); d > i + 1; --d)
            if (const size_t end = close_after_exponent(s, d); end != std::string::npos) return end;
    }
    return std::string::npos;
}

struct Literal {
    size_t pos;
    std::string text;
};

// Every numeric literal on a code line, left to right. A literal never
// follows a word byte, a dot or a quote.
std::vector<Literal> literals(const std::string& line) {
    std::vector<Literal> out;
    size_t i = 0;
    while (i < line.size()) {
        const char c = line[i];
        const char prev = i ? line[i - 1] : '\0';
        if ((digit(c) || c == '.') && !(word_byte(prev) || prev == '.' || prev == '\'')) {
            if (const size_t end = literal_end(line, i); end != std::string::npos) {
                out.push_back({i, line.substr(i, end - i)});
                i = end;
                continue;
            }
        }
        ++i;
    }
    return out;
}

// The sizes of the arrays a file declares ("int key[256]", "std::array<T,
// 4>"), as written: a loop or bound that matches one is the container's size.
std::set<std::string> container_sizes(const std::string& code) {
    std::set<std::string> out;
    const auto ident_end = [&](size_t p) {
        while (word_byte(at(code, p))) ++p;
        return p;
    };
    const auto spaces_end = [&](size_t p) {
        while (is_ascii_space(at(code, p))) ++p;
        return p;
    };
    const auto ident_start = [&](size_t p) {
        const char c = at(code, p);
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
    };
    const std::string kArray = "std::array";
    size_t i = 0;
    while (i < code.size()) {
        // "<type> <name> [ N ]", the type at a word start.
        if (ident_start(i) && !word_byte_before(code, i)) {
            size_t p = ident_end(i);
            const size_t gap = spaces_end(p);
            if (gap > p && ident_start(gap)) {
                p = spaces_end(ident_end(gap));
                if (at(code, p) == '[') {
                    const size_t n = spaces_end(p + 1);
                    const size_t ne = digits_end(code, n);
                    const size_t close = spaces_end(ne);
                    if (ne > n && at(code, close) == ']') {
                        out.insert(code.substr(n, ne - n));
                        i = close + 1;
                        continue;
                    }
                }
            }
        }
        // "std::array < T , N >".
        if (code.compare(i, kArray.size(), kArray) == 0) {
            size_t p = spaces_end(i + kArray.size());
            if (at(code, p) == '<') {
                size_t q = p + 1;
                while (q < code.size() && !has(",<>", code[q])) ++q;
                if (q > p + 1 && at(code, q) == ',') {
                    const size_t n = spaces_end(q + 1);
                    const size_t ne = digits_end(code, n);
                    const size_t close = spaces_end(ne);
                    if (ne > n && at(code, close) == '>') {
                        out.insert(code.substr(n, ne - n));
                        i = close + 1;
                        continue;
                    }
                }
            }
        }
        ++i;
    }
    return out;
}

// ---------------------------------------------------- the context words

// Words in the order first seen, each once.
struct OrderedWords {
    std::vector<std::string> list;
    std::set<std::string> seen;
    void add(const std::string& w) {
        if (seen.insert(w).second) list.push_back(w);
    }
};

// The words that say what a number is: each identifier's parts (a leading
// "k" before a letter dropped, split at camel humps and underscores),
// lowercase, three letters or more, not a stop word, not starting with a
// digit.
OrderedWords context_words(const std::string& code) {
    OrderedWords words;
    size_t i = 0;
    while (i < code.size()) {
        const char c = code[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')) {
            ++i;
            continue;
        }
        size_t j = i + 1;
        while (word_byte(at(code, j))) ++j;
        std::string id = code.substr(i, j - i);
        i = j;
        // The script dropped the k with PowerShell's -replace, which ignores
        // case, so "kind" loses its k as "kMaxDepth" does.
        const char second = at(id, 1);
        if ((id[0] == 'k' || id[0] == 'K') && ((second >= 'A' && second <= 'Z') || (second >= 'a' && second <= 'z')))
            id.erase(0, 1);
        std::string split;
        for (size_t k = 0; k < id.size(); ++k) {
            const char a = id[k];
            const char b = at(id, k + 1);
            split += a;
            if (((a >= 'a' && a <= 'z') || digit(a)) && b >= 'A' && b <= 'Z') {
                split += '_';
                split += b;
                ++k;
            }
        }
        size_t from = 0;
        for (;;) {
            const size_t us = split.find('_', from);
            const std::string w = hydra::to_lower_ascii(split.substr(from, us == std::string::npos ? std::string::npos : us - from));
            if (w.size() >= 3 && !kStopWords.count(w) && !digit(w[0])) words.add(w);
            if (us == std::string::npos) break;
            from = us + 1;
        }
    }
    return words;
}

// ------------------------------------------------- what a number is doing

// Does `before` end in a comparison operator (<, <=, >, >=) that is not
// part of <<, >> or ->? Or does `after` start with one that is not followed
// by <, > or =?
bool in_compare(const std::string& before, const std::string& after) {
    bool ends_op = false;
    if (!before.empty()) {
        size_t op = std::string::npos;
        const char last = before.back();
        if (last == '<' || last == '>') {
            op = before.size() - 1;
        } else if (last == '=' && before.size() >= 2 && has("<>", before[before.size() - 2])) {
            op = before.size() - 2;
        }
        ends_op = op != std::string::npos && !(op > 0 && has("<>-", before[op - 1])) &&
                  !(ends_with(before, "<<") || ends_with(before, ">>") || ends_with(before, "->"));
    }
    bool starts_op = false;
    if (!after.empty() && has("<>", after[0])) {
        const size_t next = at(after, 1) == '=' ? 2 : 1;
        starts_op = !has("<>=", at(after, next));
    }
    return ends_op || starts_op;
}

// Does the line compare, once shifts, arrows and template brackets are
// taken out? A shift amount counts as compared when its line does.
bool line_compares(const std::string& code) {
    std::string plain;
    for (size_t i = 0; i < code.size(); ++i) {
        const char a = code[i];
        const char b = at(code, i + 1);
        if ((a == '<' && b == '<') || (a == '>' && b == '>') || (a == '-' && b == '>')) {
            plain += ' ';
            ++i;
        } else {
            plain += a;
        }
    }
    std::string out;
    for (size_t i = 0; i < plain.size(); ++i) {
        if (plain[i] == '<') {
            size_t j = i + 1;
            while (j < plain.size() && (word_byte(plain[j]) || is_ascii_space(plain[j]) || has(":,*&", plain[j]))) ++j;
            if (at(plain, j) == '>') {
                out += ' ';
                i = j;
                continue;
            }
        }
        out += plain[i];
    }
    return out.find_first_of("<>") != std::string::npos;
}

// Is the number the right side of a plain "=" (not ==, !=, <=, += ...),
// with only signs and brackets between? The script's second spelling (a
// NAME = at the end) only matches where this one does.
bool named_value(const std::string& before) {
    for (size_t p = 0; p < before.size(); ++p) {
        if (before[p] != '=') continue;
        if (p > 0 && has("=!<>+-*/%&|^", before[p - 1])) continue;
        size_t q = precheck::skip_space(before, p + 1);
        while (q < before.size() && has("-+(", before[q])) ++q;
        if (q == before.size()) return true;
    }
    return false;
}

// The name a test assigns the number to ("x.limit =", "depth ="), or empty.
std::string assigned_name(const std::string& before) {
    if (before.empty() || before.back() != '=') return "";
    size_t e = before.size() - 1;
    while (e > 0 && is_ascii_space(before[e - 1])) --e;
    size_t b = e;
    while (b > 0 && word_byte(before[b - 1])) --b;
    return before.substr(b, e - b);
}

bool word_matches(const std::set<std::string>& para_words, const std::string& w) {
    return para_words.count(w) || para_words.count(w + "s") ||
           (w.size() > 1 && w.back() == 's' && para_words.count(w.substr(0, w.size() - 1)));
}

std::string join(const std::vector<std::string>& parts, size_t limit) {
    std::string out;
    for (size_t i = 0; i < parts.size() && i < limit; ++i) out += (i ? ", " : "") + parts[i];
    return out;
}

}  // namespace

void run_check_3(Context& ctx) {
    if (ctx.range.whole_tree) {
        ctx.out << "check 3 skipped: -WholeTree has no \"new\" numbers\n";
        return;
    }
    const std::vector<DecisionPara> paras = decision_paras(ctx.repo, {ctx.range.tip, ctx.range.left});
    std::map<std::string, std::vector<const DecisionPara*>> by_record;
    for (const DecisionPara& p : paras) by_record[p.record].push_back(&p);
    // 0, 1 and 2 are counts, first and second, and off-by-one, not
    // thresholds (decision D49).
    const std::set<std::string> skip = {"0", "1", "2"};
    // 1000, 1024 and 1000000 next to * or / are unit factors, ms to s and
    // KiB, not limits (decision D49).
    const std::set<std::string> unit_factors = {"1000", "1024", "1000000"};
    ViewCache views(ctx.repo);
    for (const std::string& f : ctx.added.files()) {
        if (!(is_cpp(f) || is_py(f))) continue;
        if (skipped_file(f)) continue;
        const bool is_test = test_file(f);
        const View& v = views.get(ctx.range.tip, f);
        const std::set<std::string> sizes = container_sizes(v.code);
        for (const int ln : ctx.added.lines(f, v.line_count())) {
            const std::string code = v.line(v.code, ln);
            const std::string t = hydra::trim(code);
            if (t.empty()) continue;
            if (search(t, skipped_line())) continue;
            const bool is_check = search(t, assertion());
            const bool is_loop = search(t, loop_line());
            for (const Literal& m : literals(code)) {
                const std::optional<double> n = number_of(m.text);
                if (!n) continue;
                const std::string canon = number_canon(*n);
                if (skip.count(canon)) continue;
                const std::string before = trim_end(code.substr(0, m.pos));
                const std::string after = trim_start(code.substr(m.pos + m.text.size()));
                if (!before.empty() && before.back() == '[' && !after.empty() && after[0] == ']') continue;
                if (sizes.count(canon)) continue;
                // A unit factor (ms to s, bytes to KiB) multiplies or divides.
                const bool factor_before = ends_with(before, "*") || ends_with(before, "/") ||
                                           ends_with(before, "*=") || ends_with(before, "/=");
                if (unit_factors.count(canon) && (factor_before || (!after.empty() && has("*/", after[0]))))
                    continue;
                bool compared = in_compare(before, after);
                if (!compared && (ends_with(before, "<<") || ends_with(before, ">>"))) compared = line_compares(code);
                if (!is_test) {
                    // Production: thresholds, limits, tolerances, times and
                    // fallbacks show up as a comparison, a named value, a
                    // returned value, a min/max/clamp bound or a duration. A
                    // number inside other arithmetic or passed to a call
                    // computes a layout or a conversion, so it is left to the
                    // reviewer.
                    const bool is_named = named_value(before);
                    const bool is_return = search(before, returned()) && !after.empty() && has(";)", after[0]);
                    const bool is_bound = search(before, bound());
                    const bool is_duration = search(before, duration_before());
                    if (!(compared || is_named || is_return || is_bound || is_duration)) continue;
                } else {
                    // A test's own limits: a field or constant whose name
                    // says limit, a loop over seeds or repeats, a duration,
                    // or a comparison. Ticks, notes and offsets set on a
                    // fixture are fixture data.
                    const std::string assigned = assigned_name(before);
                    const bool is_field = !assigned.empty() && search(assigned, limit_words());
                    const bool is_loop_limit = is_loop && search(t, loop_words()) && !search(t, braced_loop());
                    const bool is_duration = search(before, duration_before()) || search(after, unit_after());
                    if (is_check && !compared) continue;
                    if (is_loop && !is_loop_limit && !compared) continue;
                    if (is_loop && compared && !is_loop_limit) continue;
                    const bool is_sample = search(before, sample());
                    if (!(compared || is_loop_limit || is_field || is_duration || is_sample)) continue;
                }
                // Cited decision on this line or the three above (D49)?
                std::string near;
                const int first = std::max(1, ln - 3);
                for (int k = first; k <= ln; ++k) near += (k > first ? "\n" : "") + v.line(v.raw, k);
                std::vector<std::string> cites;
                for (std::sregex_iterator it(near.begin(), near.end(), cite_d()), end; it != end; ++it)
                    cites.push_back(record_name(false, (*it)[1].str()));
                for (std::sregex_iterator it(near.begin(), near.end(), cite_adr()), end; it != end; ++it)
                    cites.push_back(record_name(true, (*it)[1].str()));
                const std::string shown = m.text + " in: " + hydra::trim(v.line(v.no_comments, ln));
                if (!cites.empty()) {
                    bool found = false;
                    for (const std::string& r : cites) {
                        const auto rec = by_record.find(r);
                        if (rec == by_record.end()) continue;
                        for (const DecisionPara* p : rec->second) found = found || p->numbers.count(canon);
                    }
                    if (!found) {
                        OrderedWords unique;
                        for (const std::string& c : cites) unique.add(c);
                        ctx.items.add("D", f, ln, shown,
                                      "cites " + join(unique.list, unique.list.size()) + ", which does not contain " +
                                          m.text);
                    }
                    continue;
                }
                // The words that say what the number is: the line's own
                // names, or, when it has none, the three lines above it.
                OrderedWords words = context_words(code);
                if (words.list.empty()) words = context_words(near);
                bool found = false;
                for (const DecisionPara& p : paras) {
                    if (!p.numbers.count(canon)) continue;
                    for (const std::string& w : words.list) {
                        if (word_matches(p.words, w)) {
                            found = true;
                            break;
                        }
                    }
                    if (found) break;
                }
                if (!found) {
                    const std::string about = words.list.empty() ? "" : " beside any of: " + join(words.list, 4);
                    ctx.items.add("D", f, ln, shown,
                                  "no ADR, CONTEXT.md, plan decision or D-record names " + m.text + about);
                }
            }
        }
    }
}

}  // namespace precheck
