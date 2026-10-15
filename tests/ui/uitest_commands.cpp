// The command interpreter (uitest_commands.h). Verb table:
// docs/agents/ui-testing.md.

// The harness first: its Windows headers define a Yield() macro that the
// test engine's header, included after them, undefines.
#include "uitest_harness.h"
#include "uitest_commands.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>
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

// Split a line at its first " | " (blank, bar, blank): head is what comes
// before it, tail everything after, kept exactly. A line ending in " |" is a
// bar with an empty tail, so a client that trims trailing blanks can still
// clear a field. False when there is no bar. The one split type and the text
// and dump filters share.
bool split_bar(const std::string& line, std::string& head, std::string& tail) {
    size_t bar = line.find(" | ");
    if (bar == std::string::npos) {
        if (line.size() < 2 || line.compare(line.size() - 2, 2, " |") != 0) return false;
        bar = line.size() - 2;
    }
    head = line.substr(0, bar);
    tail = bar + 3 <= line.size() ? line.substr(bar + 3) : std::string();
    return true;
}

// The seconds a timeout or wait argument gives: the number
// hydra::parse_finite_number reads, 0 or more. The one parse both verbs share.
bool parse_seconds(const std::string& text, double& seconds) {
    const std::optional<double> v = hydra::parse_finite_number(text);
    if (!v || *v < 0) return false;
    seconds = *v == 0 ? 0.0 : *v;  // "-0" reads as 0
    return true;
}

// printf into a string, for the interpreter's messages.
std::string message(const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return buf;
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
    // The interpreter's own refusal, which stops the command there. A script
    // fails the test with the reason in its log, naming the line; live mode
    // hands the reason back and the session goes on.
    auto refuse = [&](const std::string& why) -> CommandResult {
        if (session.live) return {false, why};
        IM_ERRORF("line %d: %s", lineno, why.c_str());
        return {false, ""};
    };

    if (verb == "window") {
        ctx->SetRef(rest.c_str());
    } else if (verb == "click") {
        ctx->ItemClick(rest.c_str());
    } else if (verb == "check" || verb == "uncheck") {
        if (verb == "check") ctx->ItemCheck(rest.c_str());
        else ctx->ItemUncheck(rest.c_str());
    } else if (verb == "type") {
        // type <ref> | <text>: the text is everything after the bar, as given.
        std::string head, text, ignored, ref;
        if (!split_bar(line, head, text)) return refuse("type needs \"<ref> | <text>\"");
        split_verb(head, ignored, ref);
        ctx->ItemInputValue(ref.c_str(), text.c_str());
    } else if (verb == "key") {
        ImGuiKeyChord chord = 0;
        if (!parse_chord(rest, chord)) return refuse(message("unknown key \"%s\"", rest.c_str()));
        ctx->KeyPress(chord);
    } else if (verb == "wait" || verb == "timeout") {
        double seconds = 0;
        if (!parse_seconds(rest, seconds))
            return refuse(message("%s needs a number of seconds, 0 or more, and nothing else; "
                                  "got \"%s\"",
                                  verb.c_str(), rest.c_str()));
        if (verb == "timeout") {
            session.timeout = seconds;
        } else {
            // In live mode the timeout is the longest any command may take.
            if (session.live && seconds > timeout)
                return refuse(message("wait %s is longer than the %g s timeout in force; raise "
                                      "the timeout first",
                                      rest.c_str(), timeout));
            ctx->SleepNoSkip((float)seconds, 1.0f / 60.0f);
        }
    } else if (verb == "wait-idle") {
        if (!wait_until(ctx, [&] { return !jobs_busy(h); }, timeout))
            return refuse(message("jobs still running after %.0fs", timeout));
    } else if (verb == "wait-text") {
        if (!wait_until(ctx, [&] { return visible_text(h).find(rest) != std::string::npos; },
                        timeout))
            return refuse(message("\"%s\" did not appear within %.0fs", rest.c_str(), timeout));
    } else if (verb == "expect-text") {
        ctx->Yield();
        if (visible_text(h).find(rest) == std::string::npos)
            return refuse(message("\"%s\" is not on screen", rest.c_str()));
    } else if (verb == "expect-not-text") {
        ctx->Yield();
        if (visible_text(h).find(rest) != std::string::npos)
            return refuse(message("\"%s\" is on screen", rest.c_str()));
    } else if (verb == "text" || verb == "dump") {
        // text [| <part>] and dump [<window>] [| <part>]: what comes after the
        // bar filters the output. No bar filters on nothing. Both forms of
        // text go through lines_holding, so they end their lines alike.
        std::string arg = rest, part, head, ignored;
        if (split_bar(line, head, part)) split_verb(head, ignored, arg);
        if (verb == "text" && !arg.empty())
            return refuse("text takes nothing, or \"| <substring>\"");
        ctx->Yield();
        if (verb == "dump") r.text = widget_dump(ctx, arg, part);
        else r.text = lines_holding(visible_text(h), part);
    } else if (verb == "rect") {
        ctx->Yield();
        const ImGuiTestItemInfo it = ctx->ItemInfo(rest.c_str(), ImGuiTestOpFlags_NoError);
        if (it.ID == 0) return refuse(message("rect found no item \"%s\"", rest.c_str()));
        r.text = item_line(it, 0);
    } else if (verb == "state") {
        r.text = state_text(h);
    } else if (verb == "screenshot") {
        std::string saved_to;
        if (!screenshot(ctx, rest, &saved_to)) return refuse("screenshot failed");
        r.text = "screenshot: " + saved_to + "\n";
    } else if (verb == "reset") {
        start_session(ctx);
    } else if (verb == "quit") {
        session.quit = true;
    } else {
        return refuse(message("unknown verb \"%s\"", verb.c_str()));
    }
    if (ctx->IsError()) {
        // An engine action failed; its reason is in the log.
        if (session.live) return {false, "failed: " + line};
        char buf[64];
        std::snprintf(buf, sizeof(buf), "!! failed at line %d: ", lineno);
        return {false, buf + line + "\n"};
    }
    return r;
}

}  // namespace uitest
