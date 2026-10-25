// The precheck's row hits; check_rows.h says what each part answers. Every
// "does this line answer this row" comes from single_owner::scan_file.
#include "check_rows.h"

#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <tuple>

#include "core/strutil.h"
#include "json.hpp"
#include "single_owner/engine.h"

namespace precheck {

namespace so = single_owner;

namespace {

// The rules folder's parent: select_rule_files names each file from the
// rules folder's own name down ("rules/x.json").
std::string rules_parent() {
    const std::string dir = so::kRulesDir;
    return dir.substr(0, dir.rfind('/') + 1);
}

// The rows and known copies at one revision, each rules file with its text.
struct Tables {
    std::vector<std::string> rule_files;  // repo-relative, in select_rule_files order
    std::vector<std::string> rule_texts;
    std::vector<so::OwnerRule> rules;
    std::vector<so::KnownCopy> copies;
};

// The rules files at `rev`, each named from the rules folder down.
std::vector<std::string> rule_file_names(Repo& repo, const std::string& rev) {
    const std::string dir = so::kRulesDir;
    std::vector<std::string> rel;
    for (const std::string& p : repo.git({"ls-tree", "-r", "--name-only", rev, "--", dir + "/"}))
        if (p.size() > dir.size() + 1) rel.push_back(hydra::relative_slash_path(p, dir));
    return so::select_rule_files(rel);
}

Tables read_tables(Repo& repo, const std::string& rev) {
    Tables t;
    for (const std::string& name : rule_file_names(repo, rev)) {
        const std::string path = rules_parent() + name;
        std::string text = repo.files().text(rev, path);
        for (so::OwnerRule& r : so::parse_rules_file(name, text)) t.rules.push_back(std::move(r));
        t.rule_files.push_back(path);
        t.rule_texts.push_back(std::move(text));
    }
    if (const std::optional<std::string> raw = repo.files().raw(rev, so::kBaselineFile))
        t.copies = so::parse_baseline(tip_text_form(*raw)).copies;
    return t;
}

// Which entries of the tip's lists the range added. An entry is added when
// the base holds fewer entries with its key than the tip holds up to and
// including it, so a second entry with the same key counts as new.
class AddedKeys {
public:
    // Nothing counts as added.
    AddedKeys() = default;
    // Everything counts as added.
    static AddedKeys all() {
        AddedKeys k;
        k.all_ = true;
        k.on_ = true;
        return k;
    }
    // What is at `base`.
    static AddedKeys against(const Tables& base) {
        AddedKeys k;
        k.on_ = true;
        for (const so::OwnerRule& r : base.rules) {
            ++k.base_[{"row", r.question, "", ""}];
            for (const so::OwnerLine& o : r.owner_lines)
                ++k.base_[{"owner", r.question, o.file, hydra::trim(o.line_text)}];
        }
        for (const so::KnownCopy& c : base.copies)
            ++k.base_[{"copy", c.question, c.file, hydra::trim(c.line_text)}];
        return k;
    }
    // Asked once per tip entry, in the tip's order.
    bool next(const std::string& what, const std::string& question, const std::string& file = "",
              const std::string& text = "") {
        if (!on_) return false;
        if (all_) return true;
        const Key key{what, question, file, text};
        const int nth = ++seen_[key];
        const auto it = base_.find(key);
        return nth > (it == base_.end() ? 0 : it->second);
    }

private:
    using Key = std::tuple<std::string, std::string, std::string, std::string>;
    bool on_ = false;
    bool all_ = false;
    std::map<Key, int> base_;
    std::map<Key, int> seen_;
};

// The line of the opening brace of the `nth` (0-based) row asking `question`
// in a rules file's text, or 0 when it is not found.
int row_line(const std::string& text, const std::string& question, int nth) {
    std::vector<std::string> lines;
    std::istringstream in(text);
    for (std::string l; std::getline(in, l);) lines.push_back(l);
    int seen = 0;
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string t = hydra::trim(lines[i]);
        if (!hydra::starts_with(t, "\"question\"")) continue;
        std::string value = hydra::trim(t.substr(t.find(':') == std::string::npos ? t.size() : t.find(':') + 1));
        if (!value.empty() && value.back() == ',') value.pop_back();
        const nlohmann::json v = nlohmann::json::parse(value, nullptr, false);
        if (!v.is_string() || v.get<std::string>() != question) continue;
        if (seen++ != nth) continue;
        for (size_t j = i + 1; j-- > 0;) {
            const std::string b = hydra::trim(lines[j]);
            if (!b.empty() && b.back() == '{') return static_cast<int>(j) + 1;
        }
        return 0;
    }
    return 0;
}

// Format-RowWhy, Format-KnownCopy and Format-OwnerLineNote of the script
// this replaces, word for word.
std::string row_why(const std::string& rules_file, const std::string& owner) {
    return "the scan row in " + rules_file + " gives this question to " + owner;
}
std::string owner_line_note(const std::string& why) {
    return "this range lists it as an owner line: \"" + why + "\"; check that reason";
}
const char* const kAddedEntrySuffix = "; this range added that entry";
const char* const kSecondCopyNote = "a second copy of a listed line; each entry covers one line";

// Everything one range's row hits need, worked out once.
struct RowRun {
    std::string key;
    std::vector<RowItem> items;
    std::vector<AddedRow> added_rows;
    std::vector<so::KnownCopy> copies;
};

std::string run_key(Context& ctx) {
    const Range& r = ctx.range;
    return ctx.repo.root() + "\n" + r.tip + "\n" + r.base + "\n" + r.rules_rev + "\n" +
           (r.whole_tree ? "w" : "r");
}

std::unique_ptr<RowRun> compute(Context& ctx) {
    auto run = std::make_unique<RowRun>();
    run->key = run_key(ctx);
    const Range& range = ctx.range;
    const Tables tip = read_tables(ctx.repo, range.rules_rev);
    run->copies = tip.copies;

    // Added means written by this range, so only rows read at the tip can be.
    AddedKeys added;
    if (range.rules_rev == range.tip)
        added = (range.whole_tree || range.base.empty()) ? AddedKeys::all()
                                                         : AddedKeys::against(read_tables(ctx.repo, range.base));

    // The rows the range added, and its added listed entries in listed_lines
    // order: every row's owner lines, then every known copy.
    std::map<std::string, int> row_seen;  // rows so far per rules file and question
    for (const so::OwnerRule& r : tip.rules) {
        const std::string file = rules_parent() + r.rules_file;
        const int nth = row_seen[file + "\n" + r.question]++;
        if (!added.next("row", r.question)) continue;
        int line = 0;
        for (size_t i = 0; i < tip.rule_files.size(); ++i)
            if (tip.rule_files[i] == file) line = row_line(tip.rule_texts[i], r.question, nth);
        run->added_rows.push_back({r, file, line});
    }
    const std::vector<so::ListedLine> listed = so::listed_lines(tip.rules, tip.copies);
    size_t owner_count = 0;
    for (const so::OwnerRule& r : tip.rules) owner_count += r.owner_lines.size();
    std::vector<bool> listed_added;
    std::vector<std::string> listed_note;  // the owner line's why, or the copy's fix
    for (const so::OwnerRule& r : tip.rules)
        for (const so::OwnerLine& o : r.owner_lines) {
            listed_added.push_back(added.next("owner", r.question, o.file, hydra::trim(o.line_text)));
            listed_note.push_back(o.why);
        }
    for (const so::KnownCopy& c : tip.copies) {
        listed_added.push_back(added.next("copy", c.question, c.file, hydra::trim(c.line_text)));
        listed_note.push_back(c.removed_by);
    }

    std::vector<so::CompiledRule> compiled;
    compiled.reserve(tip.rules.size());
    for (const so::OwnerRule& r : tip.rules) compiled.push_back(so::compile_rule(r));
    so::ScanState state = so::start_scan(compiled, listed);

    for (const std::string& f : ctx.added.files()) {
        if (skipped_file(f)) continue;
        const bool py = is_py(f);
        if (!is_cpp(f) && !py) continue;
        const std::optional<std::string> raw = ctx.repo.files().raw(range.tip, f);
        if (!raw) continue;  // deleted by the range
        std::istringstream in(tip_text_form(*raw));
        so::scan_file(state, f, py, in, [&](const so::RowHit& h) {
            if (!ctx.added.added(f, h.line)) return;
            const so::OwnerRule& r = *compiled[h.rule_index].rule;
            std::string note;
            if (h.take == so::Take::taken) {
                const size_t i = *h.listed_index;
                if (i < owner_count) {
                    // An owner line passes unless this range wrote it; then
                    // the reviewer judges its reason.
                    if (!listed_added[i]) return;
                    note = owner_line_note(listed_note[i]);
                } else {
                    note = known_copy_note(listed_note[i]) + (listed_added[i] ? kAddedEntrySuffix : "");
                }
            } else if (h.take == so::Take::used_up) {
                note = kSecondCopyNote;
            }
            std::string why = row_why(rules_parent() + r.rules_file, r.owner);
            if (!note.empty()) why += " (" + note + ")";
            run->items.push_back(
                {row_kind(f, r.owner), f, h.line, "answers \"" + r.question + "\": " + h.text, why});
        });
    }
    return run;
}

RowRun& row_run(Context& ctx) {
    static std::unique_ptr<RowRun> cached;
    if (!cached || cached->key != run_key(ctx)) cached = compute(ctx);
    return *cached;
}

bool is_word_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

}  // namespace

std::string row_kind(const std::string& file, const std::string& owner) {
    if (!test_file(file)) return "A";
    // The script matched the owner without regard to case; so does this.
    std::string o = owner;
    for (char& c : o)
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    if (o.find("tests/source_tree.h") != std::string::npos) return "E";
    for (size_t at = o.find("tests/"); at != std::string::npos; at = o.find("tests/", at + 1))
        if (at == 0 || !is_word_char(o[at - 1])) return "C";
    return "B";
}

std::vector<RowItem> row_items(Context& ctx, const std::string& kinds) {
    std::vector<RowItem> out;
    for (const RowItem& i : row_run(ctx).items)
        if (kinds.find(i.kind) != std::string::npos) out.push_back(i);
    return out;
}

void add_row_items(Context& ctx, const std::string& kinds) {
    for (const RowItem& i : row_items(ctx, kinds)) ctx.items.add(i.kind, i.file, i.line, i.what, i.why);
}

void add_row_items_for_check(int check, Context& ctx) {
    if (check == 1) add_row_items(ctx, "C");
    else if (check == 2) add_row_items(ctx, "BA");
    else if (check == 4) add_row_items(ctx, "E");
}

std::vector<AddedRow> added_rows(Context& ctx) { return row_run(ctx).added_rows; }

const std::vector<so::KnownCopy>& known_copies(Context& ctx) { return row_run(ctx).copies; }

std::string known_copy_note(const std::string& fix) { return "known copy: " + fix; }

void print_rule_warnings(Context& ctx) {
    if (!rule_file_names(ctx.repo, ctx.range.rules_rev).empty()) return;
    ctx.out << "precheck: " << so::kRulesDir << " has no rule files at " << ctx.range.rules_rev.substr(0, 7)
            << ", so no scan rows were applied\n";
}

}  // namespace precheck
