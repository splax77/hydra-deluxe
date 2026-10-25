#include "records.h"

#include <cctype>
#include <optional>
#include <set>

#include "definitions.h"  // is_cpp_keyword

namespace precheck {

namespace {

bool word_char(char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }
bool word_start(char c) { return std::isalpha(static_cast<unsigned char>(c)) || c == '_'; }
bool space(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v'; }

size_t skip_space(const std::string& s, size_t i) {
    while (i < s.size() && space(s[i])) ++i;
    return i;
}

bool at(const std::string& s, size_t i, const std::string& lit) { return s.compare(i, lit.size(), lit) == 0; }

bool at_icase(const std::string& s, size_t i, const std::string& lit) {
    if (i + lit.size() > s.size()) return false;
    for (size_t k = 0; k < lit.size(); ++k)
        if (std::tolower(static_cast<unsigned char>(s[i + k])) != lit[k]) return false;
    return true;
}

std::string fold(const std::string& s) {
    std::string out = s;
    for (char& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

std::string trim(const std::string& s) {
    const size_t b = s.find_first_not_of(" \t\r\n\f\v");
    if (b == std::string::npos) return {};
    const size_t e = s.find_last_not_of(" \t\r\n\f\v");
    return s.substr(b, e - b + 1);
}

struct Tail {
    size_t name_at, name_end, open;
};

// What follows the kind keyword: white space, an optional [[attribute]], the
// name, an optional "final", an optional base clause (":" but not "::", up
// to the first ";" or "{"), then the "{".
std::optional<Tail> definition_tail(const std::string& s, size_t i) {
    size_t j = skip_space(s, i);
    if (j == i) return std::nullopt;
    if (at(s, j, "[[")) {
        const size_t k = s.find(']', j + 2);
        if (k != std::string::npos && k + 1 < s.size() && s[k + 1] == ']') j = skip_space(s, k + 2);
    }
    if (j >= s.size() || !word_start(s[j])) return std::nullopt;
    const size_t name_at = j;
    while (j < s.size() && word_char(s[j])) ++j;
    const size_t name_end = j;
    j = skip_space(s, j);
    for (const bool with_final : {true, false}) {
        size_t k = j;
        if (with_final) {
            if (!at(s, k, "final")) continue;
            k = skip_space(s, k + 5);
        }
        if (k < s.size() && s[k] == ':' && !(k + 1 < s.size() && s[k + 1] == ':')) {
            const size_t m = s.find_first_of(";{", k + 1);
            if (m != std::string::npos && s[m] == '{') return Tail{name_at, name_end, m};
        }
        if (k < s.size() && s[k] == '{') return Tail{name_at, name_end, k};
    }
    return std::nullopt;
}

// The kind keywords, tried in this order at each place.
struct KindSpelling {
    const char* first;
    const char* second;  // nullptr for a one-word kind
};
const KindSpelling kKinds[] = {{"enum", "class"}, {"enum", "struct"}, {"enum", nullptr},
                               {"struct", nullptr}, {"class", nullptr}, {"union", nullptr}};

// The trailing name of one declarator: a name followed only by empty braces
// ("x{}", what a blanked brace initialiser leaves), array bounds and white
// space. Gives [begin, end) of the leftmost such name.
std::optional<std::pair<size_t, size_t>> declarator_name(const std::string& part) {
    const size_t n = part.size();
    size_t p = 0;
    while (p < n) {
        if (!word_char(part[p])) {
            ++p;
            continue;
        }
        size_t q = p;
        while (q < n && word_char(part[q])) ++q;
        size_t f = p;
        while (f < q && !word_start(part[f])) ++f;
        if (f < q) {
            size_t k = skip_space(part, q);
            while (k < n) {
                if (part[k] == '{') {
                    const size_t m = skip_space(part, k + 1);
                    if (m < n && part[m] == '}') {
                        k = skip_space(part, m + 1);
                        continue;
                    }
                } else if (part[k] == '[') {
                    const size_t m = part.find(']', k + 1);
                    if (m != std::string::npos) {
                        k = skip_space(part, m + 1);
                        continue;
                    }
                }
                break;
            }
            if (k == n) return std::make_pair(f, q);
        }
        p = q;
    }
    return std::nullopt;
}

// Blanks each access label ("public:") that starts a line of `stmt`.
void blank_access_labels(std::string& stmt) {
    static const char* const kLabels[] = {"public", "private", "protected"};
    size_t p = 0;
    while (p < stmt.size()) {
        if (p != 0 && stmt[p - 1] != '\n') {
            ++p;
            continue;
        }
        size_t k = skip_space(stmt, p);
        size_t end = std::string::npos;
        for (const char* label : kLabels) {
            const std::string l = label;
            if (!at(stmt, k, l)) continue;
            const size_t c = skip_space(stmt, k + l.size());
            if (c < stmt.size() && stmt[c] == ':') end = c + 1;
            break;
        }
        if (end == std::string::npos) {
            ++p;
            continue;
        }
        for (size_t i = p; i < end; ++i)
            if (stmt[i] != '\n') stmt[i] = ' ';
        p = end;
    }
}

// Does the trimmed statement start with a keyword that declares no member
// (a using, a friend, a static, a nested type...)? Case is ignored.
bool declares_no_member(const std::string& t) {
    static const char* const kWords[] = {"using", "friend", "static", "typedef", "template",
                                         "enum",  "struct", "class",  "union"};
    for (const char* w : kWords) {
        const std::string s = w;
        if (at_icase(t, 0, s) && (t.size() == s.size() || !word_char(t[s.size()]))) return true;
    }
    return false;
}

}  // namespace

std::vector<TypeDefinition> type_definitions(const std::string& code) {
    std::vector<TypeDefinition> out;
    size_t i = 0;
    while (i < code.size()) {
        if (i > 0 && word_char(code[i - 1])) {
            ++i;
            continue;
        }
        std::optional<TypeDefinition> found;
        for (const KindSpelling& k : kKinds) {
            if (!at(code, i, k.first)) continue;
            size_t kind_end = i + std::string(k.first).size();
            std::string kind = k.first;
            if (k.second) {
                const size_t j = skip_space(code, kind_end);
                if (j == kind_end || !at(code, j, k.second)) continue;
                kind_end = j + std::string(k.second).size();
                kind += std::string(" ") + k.second;
            }
            const std::optional<Tail> tail = definition_tail(code, kind_end);
            if (!tail) continue;
            found = TypeDefinition{kind, code.substr(tail->name_at, tail->name_end - tail->name_at), tail->name_at,
                                   tail->open};
            break;
        }
        if (!found) {
            ++i;
            continue;
        }
        i = found->open + 1;
        out.push_back(std::move(*found));
    }
    return out;
}

std::vector<RecordField> record_fields(const std::string& code, const Braces& braces, size_t open, size_t close) {
    const size_t start = open + 1;
    std::string body = code.substr(start, close - start);
    for (size_t kid : braces.children(open)) {
        const std::optional<size_t> kc = braces.close(kid, code.size());
        if (!kc) continue;
        for (size_t i = kid + 1; i < *kc; ++i)
            if (body[i - start] != '\n') body[i - start] = ' ';
    }

    // Where statements end: each ";", and each "}" that no "," ";" "=" or ")"
    // follows (a method body ends a statement; a brace initialiser does not).
    std::vector<size_t> ends;
    for (size_t p = 0; p < body.size(); ++p) {
        if (body[p] == ';') {
            ends.push_back(p);
        } else if (body[p] == '}') {
            const size_t q = skip_space(body, p + 1);
            if (q < body.size() && std::string(",;=)").find(body[q]) == std::string::npos) ends.push_back(p);
        }
    }
    ends.push_back(body.size());

    std::vector<RecordField> fields;
    std::set<std::string> seen;  // folded names
    size_t from = 0;
    for (size_t e : ends) {
        std::string stmt = body.substr(from, e - from);
        const size_t stmt_at = from;
        from = e + 1;
        blank_access_labels(stmt);
        const std::string t = trim(stmt);
        if (t.empty() || t.find('(') != std::string::npos || declares_no_member(t)) continue;
        // One statement can declare several members ("int64_t base, combo,
        // sp;"): split at commas outside <...>, (...), [...] and {...}.
        int depth = 0;
        size_t part_at = 0;
        int part_no = 0;
        for (size_t i = 0; i <= stmt.size(); ++i) {
            if (i < stmt.size()) {
                const char c = stmt[i];
                if (c == '<' || c == '(' || c == '[' || c == '{') {
                    ++depth;
                    continue;
                }
                if (c == '>' || c == ')' || c == ']' || c == '}') {
                    --depth;
                    continue;
                }
                if (c != ',' || depth != 0) continue;
            }
            std::string part = stmt.substr(part_at, i - part_at);
            const size_t eq = part.find('=');
            if (eq != std::string::npos) part = part.substr(0, eq);
            const auto name = declarator_name(part);
            // The first declarator carries the type, so its name follows one.
            const bool typed = part_no > 0 || (name && !trim(part.substr(0, name->first)).empty());
            if (name && typed) {
                const std::string n = part.substr(name->first, name->second - name->first);
                if (!is_cpp_keyword(n) && seen.insert(fold(n)).second)
                    fields.push_back({n, start + stmt_at + part_at + name->first});
            }
            part_at = i + 1;
            ++part_no;
        }
    }
    return fields;
}

}  // namespace precheck
