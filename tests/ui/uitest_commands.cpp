// The command interpreter (uitest_commands.h). Verb table:
// docs/agents/ui-testing.md.

// The engine first: its Windows headers define a Yield() macro that the
// test engine's header, included after them, undefines.
#include "uitest_engine.h"
#include "uitest_commands.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

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

// printf into a string, for the interpreter's messages, at whatever length
// the message has: a first pass measures it, a second writes it.
std::string message(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_list measure;
    va_copy(measure, args);
    const int size = std::vsnprintf(nullptr, 0, fmt, measure);
    va_end(measure);
    std::string out;
    if (size > 0) {
        out.resize(static_cast<size_t>(size));
        std::vsnprintf(out.data(), out.size() + 1, fmt, args);
    }
    va_end(args);
    return out;
}

// The last label of a ref, as the engine reads it: what follows the last '/'
// that no '\' escapes, with the escapes undone ("**/AC\/DC" gives "AC/DC").
// `segments` gets how many labels the ref holds.
std::string last_label(const std::string& ref, size_t& segments) {
    std::string label;
    bool escaped = false;
    segments = 1;
    for (char c : ref) {
        if (escaped) {
            label += c;
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '/') {
            label.clear();
            ++segments;
        } else {
            label += c;
        }
    }
    return label;
}

// The drawn widgets, named one per line, for a refusal's text.
std::string named(const std::vector<ImGuiTestItemInfo>& items) {
    std::string out;
    for (const ImGuiTestItemInfo& it : items) {
        std::string line = item_line(it, 0);
        if (!line.empty() && line.back() == '\n') line.pop_back();
        out += (out.empty() ? "" : "; ") + line + " in window \"" + it.Window->Name + "\"";
    }
    return out;
}

// A wildcard lookup of a library row's title scrolls the whole library when
// the row is not drawn, which with a full library takes minutes. So such a
// lookup is refused while the search box is empty; the program says which
// labels those are (EngineHooks::is_library_row_title). A widget of that
// label drawn outside the library (a button that shares a song's title) is
// found without any scrolling, so it goes ahead; `searches_here` says the ref
// searches under the current window, where drawn_items_labelled looks.
// Empty when the lookup may go ahead.
std::string library_scroll_refusal(ImGuiTestContext* ctx, Engine& e, const std::string& label,
                                   bool searches_here) {
    if (!e.hooks.is_library_row_title || !e.hooks.is_library_row_title(label)) return "";
    if (searches_here)
        for (const ImGuiTestItemInfo& it : drawn_items_labelled(ctx, label))
            if (std::strstr(it.Window->Name, "##library") == nullptr) return "";
    return message("refused: \"%s\" is a library row's title and the search box is "
                   "empty, so finding it would scroll the whole library; use "
                   "\"open %s\", or type into **/##search first",
                   label.c_str(), label.c_str());
}

// The ref an item verb (click, check, uncheck, type) acts on. A ref that
// neither starts with '/' nor holds "**/" and finds no item relative to the
// current window is tried once more as "**/<ref>", and that is used when it
// finds one; retried says so. The retry is refused when two or more drawn
// widgets under the current window carry the label. A ref that would go
// through a wildcard lookup passes library_scroll_refusal first. refusal holds
// the reason of either.
struct ItemRef {
    std::string ref;
    std::string refusal;
    bool retried = false;
};

ItemRef item_ref(ImGuiTestContext* ctx, Engine& h, const std::string& ref) {
    const bool wildcard = ref.find("**/") != std::string::npos;
    if (!wildcard && !ref.empty() && ref[0] == '/') return {ref};
    if (!wildcard) {
        if (ctx->ItemInfo(ref.c_str(), ImGuiTestOpFlags_NoError).ID != 0) return {ref};
    }
    // Only a ref with no prefix (bare, or starting "**/") searches under the
    // current window, which is where drawn_items_labelled looks.
    size_t segments = 1;
    const std::string label = last_label(ref, segments);
    const bool searches_here = !wildcard || ref.compare(0, 3, "**/") == 0;
    if (std::string why = library_scroll_refusal(ctx, h, label, searches_here); !why.empty())
        return {"", why};
    if (wildcard) return {ref};
    // The engine keeps the last item a wildcard lookup matched and counts no
    // others, so the drawn widgets are counted here. A ref of several labels
    // is matched on its whole path by the engine, which this count does not do.
    if (segments == 1) {
        const std::vector<ImGuiTestItemInfo> drawn = drawn_items_labelled(ctx, label);
        if (drawn.size() >= 2)
            return {"", message("refused: \"%s\" matches %zu widgets, and **/%s would take only "
                                "one of them; name one with a longer ref: %s",
                                label.c_str(), drawn.size(), ref.c_str(), named(drawn).c_str())};
    }
    const std::string wide = "**/" + ref;
    if (ctx->ItemInfo(wide.c_str(), ImGuiTestOpFlags_NoError).ID != 0) return {wide, "", true};
    return {ref};  // the engine's own lookup fails it, with its own message
}

}  // namespace

void start_session(ImGuiTestContext* ctx) {
    Engine& e = engine_of(ctx);
    if (e.hooks.reset) e.hooks.reset();
    ctx->SetRef(e.home_ref.c_str());
}

CommandResult run_command(ImGuiTestContext* ctx, CommandSession& session, const std::string& line) {
    Engine& h = engine_of(ctx);
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

    // The item verbs' ref, through item_ref. A retried ref is named in the
    // reply, so the caller learns the path that works.
    auto resolve = [&](const std::string& ref, ItemRef& out) {
        out = item_ref(ctx, h, ref);
        if (out.retried) r.text = "used ref " + out.ref + "\n";
        return out.refusal.empty();
    };
    ItemRef target;

    if (verb == "window") {
        ctx->SetRef(rest.c_str());
    } else if (verb == "click") {
        if (!resolve(rest, target)) return refuse(target.refusal);
        ctx->ItemClick(target.ref.c_str());
    } else if (verb == "check" || verb == "uncheck") {
        if (!resolve(rest, target)) return refuse(target.refusal);
        if (verb == "check") ctx->ItemCheck(target.ref.c_str());
        else ctx->ItemUncheck(target.ref.c_str());
    } else if (verb == "type") {
        // type <ref> | <text>: the text is everything after the bar, as given.
        std::string head, text, ignored, ref;
        if (!split_bar(line, head, text)) return refuse("type needs \"<ref> | <text>\"");
        split_verb(head, ignored, ref);
        if (!resolve(ref, target)) return refuse(target.refusal);
        ctx->ItemInputValue(target.ref.c_str(), text.c_str());
    } else if (verb == "open") {
        // open <title>: the program's open (EngineHooks::open_song; Hydra's
        // searches for the title). The ref goes back to home_ref, where a
        // session starts, so later refs read as they did before the open.
        // The wait for the song to settle is capped by the timeout in force,
        // like wait-idle, so a live open never outlasts the longest a command
        // may take.
        if (rest.empty()) return refuse("open needs a song title");
        if (!h.hooks.open_song) return refuse("open has no songs to open in this program");
        const std::string why = h.hooks.open_song(ctx, rest, timeout);
        ctx->SetRef(h.home_ref.c_str());
        if (!why.empty()) return refuse(why);
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
        if (!wait_until(ctx, [&] { return !h.hooks.jobs_busy || !h.hooks.jobs_busy(); }, timeout))
            return refuse(message("jobs still running after %g s", timeout));
        // Jobs are done, but an open dialog still blocks the next command.
        if (const std::string note = open_modal_note(ctx); !note.empty()) r.text = note + "\n";
    } else if (verb == "wait-text") {
        if (!wait_until(ctx, [&] { return visible_text(h).find(rest) != std::string::npos; },
                        timeout))
            return refuse(message("\"%s\" did not appear within %g s", rest.c_str(), timeout));
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
        if (h.hooks.state_text) r.text = h.hooks.state_text();
    } else if (verb == "screenshot") {
        // screenshot <file> [| <ref>]: after a bar, only that window's or
        // item's rectangle is saved. A window name is tried first, then an item.
        std::string file = rest, ref, head, ignored;
        if (split_bar(line, head, ref)) split_verb(head, ignored, file);
        ref = trim(ref);
        ImRect crop;
        bool cropped = false;
        if (!ref.empty()) {
            ctx->Yield();
            if (ImGuiWindow* w = ctx->WindowInfo(ref.c_str(), ImGuiTestOpFlags_NoError).Window) {
                crop = w->Rect();
            } else {
                const ImGuiTestItemInfo it = ctx->ItemInfo(ref.c_str(), ImGuiTestOpFlags_NoError);
                if (it.ID == 0)
                    return refuse(message("screenshot found no window or item \"%s\"", ref.c_str()));
                crop = it.RectFull;
            }
            cropped = true;
        }
        std::string saved_to;
        if (!screenshot(ctx, file, &saved_to, cropped ? &crop : nullptr))
            return refuse("screenshot failed");
        r.text = "screenshot: " + saved_to + "\n";
    } else if (verb == "reset") {
        start_session(ctx);
    } else if (verb == "quit") {
        session.quit = true;
    } else {
        return refuse(message("unknown verb \"%s\"", verb.c_str()));
    }
    if (ctx->IsError()) {
        // An engine action failed; its reason is in the log. An open dialog
        // is the usual cause the log does not name, so the reply names it.
        const std::string note = open_modal_note(ctx);
        if (session.live) return {false, "failed: " + line + (note.empty() ? "" : "\n" + note)};
        char buf[64];
        std::snprintf(buf, sizeof(buf), "!! failed at line %d: ", lineno);
        return {false, buf + line + "\n" + (note.empty() ? "" : note + "\n")};
    }
    return r;
}

}  // namespace uitest
