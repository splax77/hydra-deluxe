// The installer's AppId and the ID Hydra looks for in the registry are typed
// in two places: installer/hydra.iss and kInstallerAppId (app/config.h). This
// test holds them together, so a change to either one alone fails here.
#include "doctest.h"

#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/config.h"
#include "core/strutil.h"
#include "core/winstr.h"
#include "source_tree.h"

namespace {

struct AppIdLine {
    std::string section;  // the section header it sits under, such as "[Setup]"
    std::string value;    // the text after "AppId=", as the script types it
};

// Every AppId= line in the script, with the section it sits under.
std::vector<AppIdLine> app_id_lines(const std::string& script) {
    std::vector<AppIdLine> out;
    std::istringstream in(script);
    std::string line;
    std::string section;
    const std::string key = "AppId=";
    while (std::getline(in, line)) {
        const std::string t = hydra::trim(line);
        if (!t.empty() && t.front() == '[') {
            section = t;
            continue;
        }
        if (t.compare(0, key.size(), key) == 0) out.push_back({section, hydra::trim(t.substr(key.size()))});
    }
    return out;
}

// The text Inno Setup makes of a typed value whose only braces are literal
// ones. Inno Setup's help page on constants
// (https://jrsoftware.org/ishelp/topic_consts.htm) says a literal opening
// brace is typed as two consecutive { characters, and a closing one is not
// doubled.
std::string literal_braces(const std::string& typed) {
    std::string out;
    for (size_t i = 0; i < typed.size(); ++i) {
        out += typed[i];
        if (typed[i] == '{' && i + 1 < typed.size() && typed[i + 1] == '{') ++i;
    }
    return out;
}

}  // namespace

TEST_CASE("installer: the script's AppId is the ID Hydra opens the uninstall key with") {
    const std::string path = (sourcetree::root() / "installer" / "hydra.iss").u8string();
    std::string script;
    try {
        script = hydra::read_file_text(path);
    } catch (const std::runtime_error& e) {
        FAIL("could not read the installer script " << path << ": " << e.what());
    }

    const std::vector<AppIdLine> lines = app_id_lines(script);
    INFO("the installer script " << path << " holds " << lines.size() << " AppId= lines");
    REQUIRE(lines.size() == 1);
    CHECK(lines[0].section == "[Setup]");

    const std::string expected = "{" + hydra::wide_to_utf8(hydra::app::kInstallerAppId) + "}";
    INFO("AppId as typed: " << lines[0].value);
    CHECK(literal_braces(lines[0].value) == expected);
}
