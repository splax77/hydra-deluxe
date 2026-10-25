// Check 1: test helpers defined twice (kind C). A helper defined in two test
// files under one name, or with one body under two names, and the fixture
// spellings that belong in a shared test header.
#include <algorithm>
#include <cctype>
#include <map>
#include <regex>

#include "checks.h"
#include "definitions.h"
#include "single_owner/rows.h"  // KnownCopy
#include "views.h"

namespace precheck {

namespace {

std::string trim(const std::string& s) {
    const size_t b = s.find_first_not_of(" \t\r\n\f\v");
    if (b == std::string::npos) return {};
    const size_t e = s.find_last_not_of(" \t\r\n\f\v");
    return s.substr(b, e - b + 1);
}

// Group keys compare with case ignored, as PowerShell's Group-Object did.
std::string fold(const std::string& s) {
    std::string out = s;
    for (char& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

// The baseline's known copies at the rules revision.
// PCX-2 MERGE: check_rows reads tests/single_owner/baseline.json at
// range.rules_rev for the row hits' notes. Return its known copies here so
// the baseline has one reader; until then check 1 sees none.
std::vector<single_owner::KnownCopy> baseline_copies(Context&) { return {}; }

// The fix of the first known copy that one of `lines` is, trimmed, in
// `file`; empty when none is.
std::string known_copy_fix(const std::vector<single_owner::KnownCopy>& copies, const std::string& file,
                           const std::vector<std::string>& lines) {
    for (const std::string& l : lines) {
        const std::string t = trim(l);
        for (const single_owner::KnownCopy& k : copies)
            if (k.file == file && t == k.line_text) return k.removed_by;
    }
    return {};
}

// PCX-2 MERGE: the row hits' notes spell "known copy: <fix>" too (the
// script's Format-KnownCopy); use check_rows' one spelling here.
std::string known_copy_note(const std::string& fix) { return " (known copy: " + fix + ")"; }

const char* const kHelperWhy = "a test helper written in two files; keep one in a shared header";

// Fixture spellings no scan row asks about yet, outside the shared header
// that owns them. The MIDI row ("Which test helper writes MThd/MTrk
// chunks?") matches only the chunk tags, so a variable-length delta typed as
// bytes has no row; when a row for it lands, this entry goes.
struct Spelling {
    std::regex rx;
    const char* owner;
    const char* what;
    const char* why;
};
const std::vector<Spelling>& spellings() {
    static const std::vector<Spelling> list = {
        {std::regex(R"(&\s*0x7[Ff]\b[^;]*\|\s*0x80|0x80\s*\|\s*\(|\{\s*0x8[1-9A-Fa-f],\s*0x[0-7][0-9A-Fa-f]\s*\})"),
         "tests/midi_util.h", "a MIDI variable-length delta encoded by hand",
         "the varlen writer belongs in tests/midi_util.h"},
    };
    return list;
}

struct Group {
    bool by_name;
    std::vector<const Definition*> defs;
};

size_t unique_count(const std::vector<const Definition*>& defs, std::string Definition::*field) {
    std::vector<std::string> seen;
    for (const Definition* d : defs)
        if (std::find(seen.begin(), seen.end(), d->*field) == seen.end()) seen.push_back(d->*field);
    return seen.size();
}

}  // namespace

void run_check_1(Context& ctx) {
    ViewCache views(ctx.repo);
    const std::string& tip = ctx.range.tip;
    const std::vector<single_owner::KnownCopy> copies = baseline_copies(ctx);

    std::vector<Definition> defs;
    for (const std::string& f : ctx.all_files) {
        if (!test_file(f) || !is_cpp(f)) continue;
        for (Definition& d : definitions(views.get(tip, f), f)) defs.push_back(std::move(d));
    }

    // Groups in order of each key's first definition.
    std::vector<Group> groups;
    {
        // By name: free functions only; lambdas are local names.
        std::vector<std::string> order;
        std::map<std::string, std::vector<const Definition*>> by;
        for (const Definition& d : defs) {
            if (d.lambda) continue;
            const std::string k = fold(d.name);
            if (!by.count(k)) order.push_back(k);
            by[k].push_back(&d);
        }
        for (const std::string& k : order)
            if (unique_count(by[k], &Definition::file) >= 2) groups.push_back({true, by[k]});
    }
    {
        // By body, names normalised; bodies under kBodyTokenFloor tokens
        // match by name only.
        std::vector<std::string> order;
        std::map<std::string, std::vector<const Definition*>> by;
        for (const Definition& d : defs) {
            if (d.tokens < kBodyTokenFloor) continue;
            const std::string k = fold(d.key);
            if (!by.count(k)) order.push_back(k);
            by[k].push_back(&d);
        }
        for (const std::string& k : order) {
            const std::vector<const Definition*>& g = by[k];
            if (unique_count(g, &Definition::file) < 2) continue;
            if (unique_count(g, &Definition::name) == 1 && !g[0]->lambda) continue;  // already a name group
            groups.push_back({false, g});
        }
    }

    for (const Group& g : groups) {
        const Definition* anchor = nullptr;
        for (const Definition* d : g.defs)
            if (ctx.added.added(d->file, d->line)) {
                anchor = d;
                break;
            }
        if (!anchor) continue;
        std::string others;
        for (const Definition* d : g.defs) {
            if (d == anchor) continue;
            if (!others.empty()) others += ", ";
            others += d->name + " at " + d->file + ":" + std::to_string(d->line);
        }
        std::string fix;
        for (const Definition* d : g.defs) {
            const View& v = views.get(tip, d->file);
            std::vector<std::string> lines;
            for (int n = d->line; n <= d->last_line; ++n) lines.push_back(v.line(v.raw, n));
            const std::string f = known_copy_fix(copies, d->file, lines);
            if (!f.empty()) fix = f;
        }
        std::string what;
        if (g.by_name) {
            const bool same = unique_count(g.defs, &Definition::key) == 1;
            what = "helper " + anchor->name + " is also defined as " + others +
                   (same ? " (same body)" : " (bodies differ)");
        } else {
            what = std::string(anchor->lambda ? "lambda" : "helper") + " " + anchor->name +
                   " has the same body, names changed, as " + others;
        }
        std::string why = kHelperWhy;
        if (!fix.empty()) why += known_copy_note(fix);
        ctx.items.add("C", anchor->file, anchor->line, what, why);
    }

    // Copied fixtures a scan row names (the MIDI chunk tags, the .srb
    // deflate, the .chart header, the per-difficulty table, the tied-variant
    // walks and the SqIn step check are rows in tests/single_owner/rules/).
    // PCX-2 MERGE: add the row hits of kind C here, the script's
    // Add-RowItems @('C'), through check_rows' row_items("C").

    for (const std::string& f : ctx.added.files()) {
        if (!test_file(f) || !is_cpp(f) || skipped_file(f)) continue;
        const View& v = views.get(tip, f);
        for (int ln : ctx.added.lines(f, v.line_count())) {
            const std::string text = v.line(v.no_comments, ln);
            for (const Spelling& s : spellings()) {
                if (f == s.owner) continue;
                if (!std::regex_search(text, s.rx)) continue;
                std::string why = s.why;
                const std::string fix = known_copy_fix(copies, f, {v.line(v.raw, ln)});
                if (!fix.empty()) why += known_copy_note(fix);
                ctx.items.add("C", f, ln, std::string(s.what) + ": " + trim(text), why);
            }
        }
    }
}

}  // namespace precheck
