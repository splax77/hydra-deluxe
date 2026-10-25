// Which "}" closes each "{", which "{" encloses it, and which "{"s sit
// directly inside it. The one brace matcher in tools/precheck/: one pass with
// a stack over a view's code text (View::code, so a brace inside a comment or
// a string never counts).
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace precheck {

class Braces {
public:
    // Stands for "no enclosing brace": the parent of a top-level "{", and the
    // key whose children() are the top-level "{"s.
    static constexpr size_t kTop = static_cast<size_t>(-1);

    explicit Braces(const std::string& code);

    // The offset of the "}" that closes the "{" at `open`, or nothing when it
    // never closes or closes at or after `limit`.
    std::optional<size_t> close(size_t open, size_t limit) const;
    // The "{" that encloses the "{" at `open`, or kTop. A "{" that never
    // closes stays the enclosing block of everything after it.
    size_t parent(size_t open) const;
    // The innermost "{" whose block holds offset `p` (its own "}" included),
    // or kTop. The one answer to "which block is this offset in".
    size_t innermost(size_t p) const;
    // Whether `open` is the offset of a "{" at all.
    bool is_open(size_t open) const { return parent_.count(open) != 0; }
    // The "{" offsets directly inside the "{" at `open` (kTop for the top
    // level), in file order.
    const std::vector<size_t>& children(size_t open) const;

private:
    std::unordered_map<size_t, size_t> close_;
    std::unordered_map<size_t, size_t> parent_;
    std::unordered_map<size_t, std::vector<size_t>> children_;
};

}  // namespace precheck
