// hydra_precheck: the derive-once precheck. It reads a git range and lists
// what a reviewer should look at; see docs/agents/derive-once-review.md.
//
//   hydra_precheck [--repo <folder>] [--range <A...B | A..B | commit>]
//                  [--whole-tree] [--rules-at <commit>] [--disable <N[,N...]>]
//   hydra_precheck --list-checks
//   hydra_precheck --code-mask <file>
//
// --range defaults to main...HEAD and --repo to the current folder. A single
// bare argument is read as the range. It never writes to the repository.

#include <fcntl.h>
#include <io.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "core/winstr.h"
#include "precheck/check_rows.h"
#include "precheck/checks.h"
#include "precheck/code_mask.h"
#include "precheck/git_text.h"
#include "precheck/items.h"
#include "precheck/range.h"

namespace {

struct Options {
    std::string repo = ".";
    std::string range = "main...HEAD";
    bool range_given = false;
    bool whole_tree = false;
    std::string rules_at;
    std::vector<std::string> disable;
    std::string code_mask_file;
    bool list_checks = false;
    bool help = false;
};

const char* const kUsage =
    "usage: hydra_precheck [--repo <folder>] [--range <A...B | A..B | commit>] [--whole-tree]\n"
    "                      [--rules-at <commit>] [--disable <N[,N...]>]\n"
    "       hydra_precheck --list-checks\n"
    "       hydra_precheck --code-mask <file>\n";

Options parse_options(const std::vector<std::string>& args) {
    Options o;
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        auto value = [&]() -> std::string {
            if (i + 1 >= args.size()) throw std::invalid_argument(a + " needs a value");
            return args[++i];
        };
        if (a == "--repo") o.repo = value();
        else if (a == "--range") {
            o.range = value();
            o.range_given = true;
        } else if (a == "--whole-tree") o.whole_tree = true;
        else if (a == "--rules-at") o.rules_at = value();
        else if (a == "--disable") o.disable.push_back(value());
        else if (a == "--code-mask") o.code_mask_file = value();
        else if (a == "--list-checks") o.list_checks = true;
        else if (a == "--help" || a == "-h") o.help = true;
        else if (!a.empty() && a[0] == '-') throw std::invalid_argument("unknown option " + a);
        else if (!o.range_given) {
            o.range = a;
            o.range_given = true;
        } else throw std::invalid_argument("one range only, not also '" + a + "'");
    }
    return o;
}

int print_code_mask(const std::string& file) {
    std::ifstream in(hydra::os_path(file), std::ios::binary);
    if (!in) throw std::runtime_error("cannot read " + file);
    const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    const std::optional<std::string> masked = precheck::code_mask(text, file);
    if (!masked) throw std::runtime_error("--code-mask reads C++ and Python files only, not " + file);
    std::cout << *masked;
    return 0;
}

int run(const Options& o) {
    using namespace precheck;
    if (o.help) {
        std::cout << kUsage;
        return 0;
    }
    if (o.list_checks) {
        for (size_t i = 0; i < kChecks.size(); ++i) std::cout << (i + 1) << " " << kChecks[i].name << "\n";
        return 0;
    }
    if (!o.code_mask_file.empty()) return print_code_mask(o.code_mask_file);

    const std::set<int> disabled = parse_disable(o.disable);
    Repo repo(o.repo);
    const Range range = resolve_range(repo, o.range, o.whole_tree, o.rules_at);
    const std::vector<std::string> files = all_files(repo, range.tip);
    const AddedLines added =
        range.whole_tree ? AddedLines::whole(files) : AddedLines::from_diff(repo, range.base, range.tip);

    Items items;
    Context ctx{repo, range, added, files, items, std::cout};
    for (size_t n = 1; n <= kChecks.size(); ++n)
        if (!disabled.count(static_cast<int>(n))) kChecks[n - 1].run(ctx);
    print_rule_warnings(ctx);

    const std::vector<Item> sorted = items.sorted();
    for (const Item& i : sorted) std::cout << item_line(i) << "\n";
    if (!range.whole_tree) {
        const RangeSize size = range_size(repo, range.base, range.tip);
        std::cout << size_line(size) << "\n";
        if (const std::optional<std::string> big = large_range_line(size)) std::cout << *big << "\n";
    }
    std::cout << summary_line(sorted, range) << "\n";
    return 0;
}

}  // namespace

int wmain(int argc, wchar_t** argv) {
    // Bytes out as they are: UTF-8 text and "\n" line breaks.
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) args.push_back(hydra::wide_to_utf8(argv[i]));
    try {
        const int code = run(parse_options(args));
        std::cout.flush();
        return code;
    } catch (const std::exception& e) {
        std::cout.flush();
        std::cerr << e.what() << "\n";
        return 1;
    }
}
