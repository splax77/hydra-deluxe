// The functions a C++ file defines at namespace level, and its named
// lambdas, each with its body normalised so two copies with renamed
// variables compare equal. Check 1 compares helpers across test files with
// it; check 5 takes the free-function names. The one function-definition
// finder in tools/precheck/.
#pragma once

#include <string>
#include <vector>

#include "braces.h"
#include "views.h"

namespace precheck {

// A body with fewer tokens than this matches another by name only, never by
// body: a short body (a getter, a one-line wrapper) is the same in two files
// by accident. The user's decision is D49 in
// docs/audit/2026-10-03-fix-decisions.md.
constexpr int kBodyTokenFloor = 30;

// How far back from a "{" the finder looks for the "namespace" or extern "C"
// head that makes the block a namespace. D49 names it: a parse buffer long
// enough for any namespace head.
constexpr size_t kNamespaceLookBack = 200;

// Is this the text before a "{" (what follows the last ";", "{" or "}") a
// namespace head: "namespace", "inline namespace x" or extern "C"? The one
// reader of that question in tools/precheck/.
bool is_namespace_head(const std::string& head);

// Is this word a C++ keyword (or a fixed-width integer name, which reads as
// one)? The normaliser keeps these instead of renaming them, and a
// definition is never named by one. Case matters.
bool is_cpp_keyword(const std::string& word);

// Is this byte a letter, digit or underscore (word_char), or a letter or
// underscore (ident_start)? ASCII only, never the locale. The one spelling
// of "part of a C++ word" in tools/precheck/.
bool word_char(char c);
bool ident_start(char c);

// The doctest assertion macros (CHECK, REQUIRE or WARN, with or without
// FAST_, with any of doctest.h's suffixes), as an ECMAScript pattern with no
// anchors and no capture groups. Case matters. The one spelling of that
// family in tools/precheck/; checks that look for an assertion call build
// their pattern from it.
extern const char* const kDoctestAssertPattern;

// An assertion call: one of the doctest macros above, or assert*( or
// self.assert*( for C asserts and Python's unittest, up to and including its
// "(". Case matters: CHECK( is one, a helper named check( is not. The one
// pattern checks 2 and 3 read.
const std::string& assert_call_pattern();

// A body with its identifiers renamed in order of first use (v0, v1, ...),
// keywords, members (after "." or "->") and numbers kept, strings read as S
// and char literals as C, and any "name::" qualifier dropped.
struct Normalised {
    std::string key;  // the tokens, each followed by one space
    int tokens = 0;
};
Normalised normalise(const std::string& text);

struct Definition {
    std::string file;
    std::string name;
    int line = 0;       // the line its definition starts on
    int last_line = 0;  // the line of its closing "}"
    std::string key;    // normalise(params + " " + body).key; empty with names_only
    int tokens = 0;
    bool lambda = false;
};

// The free functions (every enclosing block a namespace or extern "C") and
// named lambdas `view` defines, functions first, each list in file order.
// Statement keywords before a name ("return f(x) {"), keywords, main and the
// doctest test and assertion macros are not definitions. names_only skips
// the normalising, which check 5 does not read.
std::vector<Definition> definitions(const View& view, const Braces& braces, const std::string& file,
                                    bool names_only = false);
std::vector<Definition> definitions(const View& view, const std::string& file, bool names_only = false);

}  // namespace precheck
