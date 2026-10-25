#include "definitions.h"

#include "core/strutil.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <unordered_map>
#include <unordered_set>

namespace precheck {

namespace {

// The keywords that start a statement or an expression and never a
// declaration. A definition's return-type words hold none of them, so
// "return f(x) {" is not a definition of f; "static int f(x) {" is, though
// static and int are keywords.
const char* const kStatementKeywords[] = {"return", "else", "new", "delete", "throw",
                                          "case", "do", "goto", "co_return"};

const std::unordered_set<std::string>& keywords() {
    static const std::unordered_set<std::string> set = [] {
        std::unordered_set<std::string> s = {
            "alignas", "alignof", "auto", "bool", "break", "catch", "char", "class", "const", "constexpr",
            "const_cast", "continue", "decltype", "default", "double", "dynamic_cast", "enum",
            "explicit", "extern", "false", "float", "for", "friend", "if", "inline", "int", "long", "mutable",
            "namespace", "noexcept", "nullptr", "operator", "private", "protected", "public", "reinterpret_cast",
            "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch",
            "template", "this", "true", "try", "typedef", "typename", "union", "unsigned", "using", "virtual",
            "void", "volatile", "while", "size_t", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t",
            "uint32_t", "uint64_t"};
        for (const char* k : kStatementKeywords) s.insert(k);
        return s;
    }();
    return set;
}

// Names that are not keywords but are still not helpers: the doctest test
// macros and main (and the doctest assertions, kDoctestAssertPattern).
bool not_function_name(const std::string& name) {
    static const std::unordered_set<std::string> set = {"TEST_CASE", "SUBCASE", "TEST_SUITE", "main"};
    if (set.count(name)) return true;
    static const std::regex asserts(kDoctestAssertPattern);
    return std::regex_match(name, asserts);
}

}  // namespace

bool word_char(char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }
bool ident_start(char c) { return std::isalpha(static_cast<unsigned char>(c)) || c == '_'; }

namespace {

bool space_char(char c) { return hydra::is_ascii_space(c); }

// "name::" qualifiers (with any spaces around the "::") dropped, every one,
// left to right.
std::string drop_qualifiers(const std::string& t) {
    std::string out;
    out.reserve(t.size());
    size_t i = 0;
    while (i < t.size()) {
        if (!word_char(t[i]) || (i > 0 && word_char(t[i - 1]))) {
            out += t[i++];
            continue;
        }
        size_t j = i;
        while (j < t.size() && word_char(t[j])) ++j;
        if (ident_start(t[i])) {
            size_t k = j;
            while (k < t.size() && space_char(t[k])) ++k;
            if (k + 1 < t.size() && t[k] == ':' && t[k + 1] == ':') {
                k += 2;
                while (k < t.size() && space_char(t[k])) ++k;
                i = k;
                continue;
            }
        }
        out.append(t, i, j - i);
        i = j;
    }
    return out;
}

// The body tokens: a word, a number (with its digit separators and any
// suffix), an empty-looking string or char literal (the code view blanks
// their insides), "->", "::", or any other one character.
std::vector<std::string> tokens_of(const std::string& t) {
    std::vector<std::string> out;
    size_t i = 0;
    while (i < t.size()) {
        const char c = t[i];
        if (space_char(c)) {
            ++i;
            continue;
        }
        size_t j = i + 1;
        if (ident_start(c)) {
            while (j < t.size() && word_char(t[j])) ++j;
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            while (j < t.size() && (word_char(t[j]) || t[j] == '.' || t[j] == '\'')) ++j;
        } else if (c == '"' || c == '\'') {
            size_t k = i + 1;
            while (k < t.size() && space_char(t[k])) ++k;
            if (k < t.size() && t[k] == c) j = k + 1;
        } else if (i + 1 < t.size() && ((c == '-' && t[i + 1] == '>') || (c == ':' && t[i + 1] == ':'))) {
            j = i + 2;
        } else if (static_cast<unsigned char>(c) >= 0x80) {
            // One character, however many UTF-8 bytes it takes.
            while (j < t.size() && (static_cast<unsigned char>(t[j]) & 0xC0) == 0x80) ++j;
        }
        out.push_back(t.substr(i, j - i));
        i = j;
    }
    return out;
}

// What the finder matches at a line start, read from that line start to the
// first ";", "{" or "}" after it. None of its parts can hold one of those
// three, so the match always ends at that first brace.
const std::regex& function_head() {
    static const std::regex rx(
        R"([ \t]*(?:template\s*<[^;{}]*>\s*)?((?:[A-Za-z_][\w:]*(?:<[^;{}()]*>)?[\s\*&]+)+?)([A-Za-z_]\w*)\s*\(([^;{}]*?)\)\s*(?:const\s*)?(?:noexcept\s*)?(?:->\s*[^;{}()]+?)?\{)");
    return rx;
}

// A named lambda, read from "auto" or "const" to the first ";", "{" or "}"
// after its capture list.
const std::regex& lambda_head() {
    static const std::regex rx(
        R"((?:auto|const\s+auto)\s*&?\s*([A-Za-z_]\w*)\s*=\s*\[[^\]]*\]\s*(?:\(([^;{}]*?)\))?\s*(?:mutable\s*)?(?:->\s*[^;{}()]+?)?\{)");
    return rx;
}

const std::regex& statement_word() {
    static const std::regex rx = [] {
        std::string alt;
        for (const char* k : kStatementKeywords) alt += (alt.empty() ? "" : "|") + std::string(k);
        return std::regex("\\b(" + alt + ")\\b", std::regex::icase);
    }();
    return rx;
}

const std::regex& namespace_head() {
    static const std::regex rx(R"(\bnamespace\b[^;{}]*$|\bextern\s*"[^"]*"\s*$)", std::regex::icase);
    return rx;
}

size_t first_of_braces(const std::string& code, size_t from) { return code.find_first_of(";{}", from); }

}  // namespace

const char* const kDoctestAssertPattern =
    R"((?:FAST_)?(?:CHECK|REQUIRE|WARN)(?:_(?:EQ|NE|GT|GE|LT|LE|UNARY(?:_FALSE)?|FALSE(?:_MESSAGE)?|MESSAGE|NOTHROW(?:_MESSAGE)?|THROWS\w*))?)";

const std::string& assert_call_pattern() {
    static const std::string p = "\\b(" + std::string(kDoctestAssertPattern) + "|assert\\w*|self\\.assert\\w*)\\s*\\(";
    return p;
}

bool is_cpp_keyword(const std::string& word) { return keywords().count(word) != 0; }

Normalised normalise(const std::string& text) {
    Normalised n;
    std::unordered_map<std::string, std::string> names;
    std::string prev;
    for (const std::string& raw : tokens_of(drop_qualifiers(text))) {
        std::string tok = raw;
        if (tok[0] == '"') {
            tok = "S";
        } else if (tok[0] == '\'') {
            tok = "C";
        } else if (ident_start(tok[0]) && !is_cpp_keyword(tok) && prev != "." && prev != "->") {
            auto it = names.find(tok);
            if (it == names.end()) it = names.emplace(tok, "v" + std::to_string(names.size())).first;
            tok = it->second;
        }
        n.key += tok;
        n.key += ' ';
        prev = raw;
        ++n.tokens;
    }
    return n;
}

std::vector<Definition> definitions(const View& view, const Braces& braces, const std::string& file,
                                    bool names_only) {
    const std::string& code = view.code;
    std::unordered_map<size_t, bool> ns_memo;
    auto is_ns = [&](size_t o) {
        auto it = ns_memo.find(o);
        if (it != ns_memo.end()) return it->second;
        const size_t from = o > kNamespaceLookBack ? o - kNamespaceLookBack : 0;
        const std::string before = code.substr(from, o - from);
        const size_t cut = before.find_last_of(";{}");
        const std::string head = cut == std::string::npos ? before : before.substr(cut + 1);
        const bool ns = std::regex_search(head, namespace_head());
        ns_memo[o] = ns;
        return ns;
    };
    auto at_top = [&](size_t o) {
        for (size_t p = braces.parent(o); p != Braces::kTop; p = braces.parent(p))
            if (!is_ns(p)) return false;
        return true;
    };

    std::vector<Definition> defs;
    // `start` is where the match begins, `open` its "{".
    auto add = [&](size_t start, size_t open, const std::string& pre, const std::string& name,
                   const std::string& params, bool lambda) {
        const std::optional<size_t> end = braces.close(open, code.size());
        if (!end) return;
        if (!lambda && !at_top(open)) return;
        if (is_cpp_keyword(name) || not_function_name(name)) return;
        if (!lambda && std::regex_search(pre, statement_word())) return;
        Definition d;
        d.file = file;
        d.name = name;
        d.lambda = lambda;
        if (!names_only) {
            const Normalised n = normalise(params + " " + code.substr(open, *end - open + 1));
            d.key = n.key;
            d.tokens = n.tokens;
        }
        size_t first = start;
        while (first < open && space_char(code[first])) ++first;
        d.line = view.line_of(first);
        d.last_line = view.line_of(*end);
        defs.push_back(std::move(d));
    };

    // Functions: a match starts at a line start and ends at the first brace
    // after it; the next search starts after the match.
    size_t next = 0;
    for (size_t p : view.line_starts) {
        if (p < next || p >= code.size()) continue;
        size_t s = p;
        while (s < code.size() && (code[s] == ' ' || code[s] == '\t')) ++s;
        if (s >= code.size() || !ident_start(code[s])) continue;
        const size_t q = first_of_braces(code, p);
        if (q == std::string::npos || code[q] != '{') continue;
        std::smatch m;
        const auto b = code.begin() + static_cast<std::ptrdiff_t>(p);
        const auto e = code.begin() + static_cast<std::ptrdiff_t>(q + 1);
        if (!std::regex_match(b, e, m, function_head())) continue;
        add(p, q, m[1].str(), m[2].str(), m[3].str(), false);
        next = q + 1;
    }

    // Named lambdas: a match starts at a word "auto" or "const" anywhere.
    next = 0;
    for (size_t i = 0; i < code.size(); ++i) {
        if (i < next || (code[i] != 'a' && code[i] != 'c')) continue;
        if (i > 0 && word_char(code[i - 1])) continue;
        if (code.compare(i, 4, "auto") != 0 && code.compare(i, 5, "const") != 0) continue;
        const size_t lb = code.find_first_of("[;{}", i);
        if (lb == std::string::npos || code[lb] != '[') continue;
        const size_t rb = code.find(']', lb + 1);
        if (rb == std::string::npos) continue;
        const size_t q = first_of_braces(code, rb + 1);
        if (q == std::string::npos || code[q] != '{') continue;
        std::smatch m;
        const auto b = code.begin() + static_cast<std::ptrdiff_t>(i);
        const auto e = code.begin() + static_cast<std::ptrdiff_t>(q + 1);
        if (!std::regex_match(b, e, m, lambda_head())) continue;
        add(i, q, "", m[1].str(), m[2].str(), true);
        next = q + 1;
    }
    return defs;
}

std::vector<Definition> definitions(const View& view, const std::string& file, bool names_only) {
    return definitions(view, Braces(view.code), file, names_only);
}

}  // namespace precheck
