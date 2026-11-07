// The names a set of changed lines defines or sits inside: the functions,
// types and constants a test could reach the change through. The test
// picker (docs/development.md, "The merge test picker", step 3)
// greps the tests for these. The one owner of "which names did these lines
// change" in tools/precheck/. It reads only masked code (View::code), so a
// name inside a comment or a string never comes out.
#pragma once

#include <set>
#include <string>
#include <vector>

#include "braces.h"
#include "views.h"

namespace precheck {

// How a changed line reached a name. Lower is stronger: a name reached two
// ways keeps the stronger kind.
enum class NameKind {
    Body,      // the line sits inside the name's block
    Declared,  // the line is part of the name's own head or declaration
    Value,     // the line declares the name as a constant or an enum value
    Comment,   // the line is a comment (or blank) above the name's declaration
};

// The word the picker prints for a kind ("body", "declared", ...).
const char* kind_word(NameKind kind);

struct ChangedName {
    std::string name;  // a plain name, or Enum::Value for an enumerator
    NameKind kind = NameKind::Body;
    // A function whose body sits directly inside a type's block (an inline
    // member). Only set for a body-reached name; the type is the key then.
    bool member = false;
};

// The names lines `lines` (1-based, in view's numbering) of a C++ file
// change, each once, in order of first appearance. `braces` must be built
// from view.code. The caller decides which side of a diff the view is;
// a deleted line is read against the base text the same way.
std::vector<ChangedName> changed_names(const View& view, const Braces& braces, const std::set<int>& lines);

}  // namespace precheck
