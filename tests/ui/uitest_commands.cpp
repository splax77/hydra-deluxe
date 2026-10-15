// The command interpreter (uitest_commands.h). Verb table:
// docs/agents/ui-testing.md.

// The harness first: its Windows headers define a Yield() macro that the
// test engine's header, included after them, undefines.
#include "uitest_harness.h"
#include "uitest_commands.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "core/strutil.h"

namespace uitest {

namespace {

using hydra::trim;

// Split "verb rest-of-line" at the first blank.
void split_verb(const std::string& line, std::string& verb, std::string& rest) {
    size_t sp = line.find_first_of(" \t");
    if (sp == std::string::npos) {
        verb = line;
        rest.clear();
    } else {
        verb = line.substr(0, sp);
        rest = trim(line.substr(sp + 1));
    }
}

// Key names are ASCII and match ignoring case.
bool same_name(const std::string& a, const char* b) { return _stricmp(a.c_str(), b) == 0; }

// One key name, ignoring case: a name from the table below, a letter A to Z,
// a digit 0 to 9, or F1 to F12. ImGuiKey_None when it is none of those.
ImGuiKey key_named(const std::string& name) {
    static const struct {
        const char* name;
        ImGuiKey key;
    } kKeys[] = {
        {"Escape", ImGuiKey_Escape},   {"Enter", ImGuiKey_Enter},
        {"Tab", ImGuiKey_Tab},         {"Space", ImGuiKey_Space},
        {"Backspace", ImGuiKey_Backspace}, {"Delete", ImGuiKey_Delete},
        {"Insert", ImGuiKey_Insert},   {"Up", ImGuiKey_UpArrow},
        {"Down", ImGuiKey_DownArrow},  {"Left", ImGuiKey_LeftArrow},
        {"Right", ImGuiKey_RightArrow}, {"Home", ImGuiKey_Home},
        {"End", ImGuiKey_End},         {"PageUp", ImGuiKey_PageUp},
        {"PageDown", ImGuiKey_PageDown},
    };
    for (const auto& k : kKeys)
        if (same_name(name, k.name)) return k.key;
    if (name.size() == 1) {
        const char c = name[0];
        if (c >= 'a' && c <= 'z') return static_cast<ImGuiKey>(ImGuiKey_A + (c - 'a'));
        if (c >= 'A' && c <= 'Z') return static_cast<ImGuiKey>(ImGuiKey_A + (c - 'A'));
        if (c >= '0' && c <= '9') return static_cast<ImGuiKey>(ImGuiKey_0 + (c - '0'));
    }
    if (name.size() >= 2 && (name[0] == 'F' || name[0] == 'f')) {
        const int n = std::atoi(name.c_str() + 1);
        if (n >= 1 && n <= 12 && std::to_string(n) == name.substr(1))
            return static_cast<ImGuiKey>(ImGuiKey_F1 + (n - 1));
    }
    return ImGuiKey_None;
}

// A key or chord written as names joined by '+': "Escape", "Ctrl+W",
// "Ctrl+Shift+Tab". The modifiers are Ctrl, Shift and Alt; the last name is
// the key (key_named). False when a name is unknown or no key is given.
bool parse_chord(const std::string& text, ImGuiKeyChord& chord) {
    chord = 0;
    ImGuiKey key = ImGuiKey_None;
    size_t start = 0;
    while (start <= text.size()) {
        size_t plus = text.find('+', start);
        if (plus == std::string::npos) plus = text.size();
        const std::string name = trim(text.substr(start, plus - start));
        if (key != ImGuiKey_None) return false;  // a name after the key
        if (same_name(name, "Ctrl")) chord |= ImGuiMod_Ctrl;
        else if (same_name(name, "Shift")) chord |= ImGuiMod_Shift;
        else if (same_name(name, "Alt")) chord |= ImGuiMod_Alt;
        else if ((key = key_named(name)) == ImGuiKey_None) return false;
        start = plus + 1;
    }
    if (key == ImGuiKey_None) return false;
    chord |= key;
    return true;
}

}  // namespace

void start_session(ImGuiTestContext* ctx) {
    reset_app(harness(ctx));
    ctx->SetRef("//Hydra");
}

CommandResult run_command(ImGuiTestContext* ctx, CommandSession& session, const std::string& line) {
    Harness& h = harness(ctx);
    const int lineno = session.line;
    const double timeout = session.timeout;
    std::string verb, rest;
    split_verb(line, verb, rest);
    CommandResult r;
    // The interpreter's own errors: the log names the line, and the command
    // stops there.
    const CommandResult failed{false, ""};

    if (verb == "window") {
        ctx->SetRef(rest.c_str());
    } else if (verb == "click") {
        ctx->ItemClick(rest.c_str());
    } else if (verb == "check" || verb == "uncheck") {
        if (verb == "check") ctx->ItemCheck(rest.c_str());
        else ctx->ItemUncheck(rest.c_str());
    } else if (verb == "type") {
        // type <ref> | <text>
        size_t bar = rest.find('|');
        if (bar == std::string::npos) {
            IM_ERRORF("line %d: type needs \"<ref> | <text>\"", lineno);
            return failed;
        }
        std::string ref = trim(rest.substr(0, bar));
        std::string text = trim(rest.substr(bar + 1));
        ctx->ItemInputValue(ref.c_str(), text.c_str());
    } else if (verb == "key") {
        ImGuiKeyChord chord = 0;
        if (!parse_chord(rest, chord)) {
            IM_ERRORF("line %d: unknown key \"%s\"", lineno, rest.c_str());
            return failed;
        }
        ctx->KeyPress(chord);
    } else if (verb == "wait") {
        ctx->SleepNoSkip((float)std::atof(rest.c_str()), 1.0f / 60.0f);
    } else if (verb == "timeout") {
        session.timeout = std::atof(rest.c_str());
    } else if (verb == "wait-idle") {
        if (!wait_until(ctx, [&] { return !jobs_busy(h); }, timeout)) {
            IM_ERRORF("line %d: jobs still running after %.0fs", lineno, timeout);
            return failed;
        }
    } else if (verb == "wait-text") {
        if (!wait_until(ctx, [&] { return visible_text(h).find(rest) != std::string::npos; },
                        timeout)) {
            IM_ERRORF("line %d: \"%s\" did not appear within %.0fs", lineno, rest.c_str(),
                      timeout);
            return failed;
        }
    } else if (verb == "expect-text") {
        ctx->Yield();
        if (visible_text(h).find(rest) == std::string::npos) {
            IM_ERRORF("line %d: \"%s\" is not on screen", lineno, rest.c_str());
            return failed;
        }
    } else if (verb == "expect-not-text") {
        ctx->Yield();
        if (visible_text(h).find(rest) != std::string::npos) {
            IM_ERRORF("line %d: \"%s\" is on screen", lineno, rest.c_str());
            return failed;
        }
    } else if (verb == "text") {
        ctx->Yield();
        r.text = visible_text(h) + "\n";
    } else if (verb == "dump") {
        ctx->Yield();
        r.text = widget_dump(ctx, rest);
    } else if (verb == "rect") {
        ctx->Yield();
        const ImGuiTestItemInfo it = ctx->ItemInfo(rest.c_str());
        if (it.ID != 0) r.text = item_line(it, 0);
    } else if (verb == "state") {
        r.text = state_text(h);
    } else if (verb == "screenshot") {
        std::string saved_to;
        if (!screenshot(ctx, rest, &saved_to)) {
            IM_ERRORF("line %d: screenshot failed", lineno);
            return failed;
        }
        r.text = "screenshot: " + saved_to + "\n";
    } else if (verb == "reset") {
        start_session(ctx);
    } else if (verb == "quit") {
        session.quit = true;
    } else {
        IM_ERRORF("line %d: unknown verb \"%s\"", lineno, verb.c_str());
        return failed;
    }
    if (ctx->IsError()) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "!! failed at line %d: ", lineno);
        return {false, buf + line + "\n"};
    }
    return r;
}

}  // namespace uitest
