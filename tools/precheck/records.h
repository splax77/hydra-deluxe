// The types a C++ file defines (struct, class, enum, enum class, union) and
// the data members of each struct or class. Check 5 compares them across
// src/ files. The one type-definition finder and record-field reader in
// tools/precheck/. Both read a view's code text (View::code), so a brace or a
// keyword inside a comment or a string never counts.
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "braces.h"

namespace precheck {

struct TypeDefinition {
    std::string kind;    // "struct", "class", "enum", "enum class", "enum struct" or "union"
    std::string name;
    size_t name_at = 0;  // the offset of the name
    size_t open = 0;     // the offset of the body's "{"
};

// Every type definition in `code`, in file order, nested ones included. A
// base clause never starts "::", so "struct Engine::Impl {" (a member type
// defined out of line) is not read as a struct named Engine.
std::vector<TypeDefinition> type_definitions(const std::string& code);

struct RecordField {
    std::string name;
    size_t at = 0;  // the offset of the name
};

// The data members of the struct or class whose body runs from the "{" at
// `open` to the "}" at `close`, in the order they are declared. Blocks nested
// in the body are blanked first, so a nested type's members never count; a
// statement with a "(" is a method; one statement may declare several
// members. Names compare with case ignored, so a second spelling of a name
// is not a second field.
std::vector<RecordField> record_fields(const std::string& code, const Braces& braces, size_t open, size_t close);

}  // namespace precheck
