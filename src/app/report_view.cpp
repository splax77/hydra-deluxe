#include "app/report_view.h"

#include "app/library_query.h"  // fold_for_search
#include "core/model.h"         // group_thousands

namespace hydra::app::report_view {

std::vector<std::string> search_words(std::string_view typed) {
    // fold_for_search turns every run of whitespace into one space, so a
    // space is the only separator left to split on.
    const std::string folded = fold_for_search(typed);
    std::vector<std::string> words;
    size_t start = 0;
    while (start <= folded.size()) {
        size_t end = folded.find(' ', start);
        if (end == std::string::npos) end = folded.size();
        if (end > start) words.push_back(folded.substr(start, end - start));
        start = end + 1;
    }
    return words;
}

bool search_matches(const std::vector<std::string>& words, std::string_view search_text) {
    for (const std::string& w : words)
        if (search_text.find(w) == std::string_view::npos) return false;
    return true;
}

SortKey folded_key(SortKey key) {
    // The library table orders its text columns by their folded form too
    // (LibraryModel::resort).
    if (std::string* text = std::get_if<std::string>(&key)) *text = fold_for_search(*text);
    return key;
}

int compare_keys(const SortKey& a, const SortKey& b) {
    // A column's keys are all numbers or all text; a mix orders by kind so
    // the sort stays a strict order.
    if (a.index() != b.index()) return a.index() < b.index() ? -1 : 1;
    if (const double* x = std::get_if<double>(&a)) {
        const double y = std::get<double>(b);
        return *x < y ? -1 : (*x > y ? 1 : 0);
    }
    if (const std::string* x = std::get_if<std::string>(&a)) {
        const int c = x->compare(std::get<std::string>(b));
        return c < 0 ? -1 : (c > 0 ? 1 : 0);
    }
    return 0;
}

std::string count_line(size_t shown, size_t total, std::string_view noun) {
    return group_thousands(static_cast<int64_t>(shown)) + " of " +
           group_thousands(static_cast<int64_t>(total)) + " " + std::string(noun);
}

SortDir first_direction(bool numeric) {
    return numeric ? SortDir::Descending : SortDir::Ascending;
}

}  // namespace hydra::app::report_view
