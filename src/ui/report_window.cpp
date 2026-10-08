#include "ui/report_window.h"

#include <algorithm>
#include <cfloat>
#include <ctime>

#include "imgui_internal.h"  // TableSetColumnSortDirection, the window's item rects
#include "ui/app_shell.h"    // report_window_class, place_report_window
#include "ui/fonts.h"        // g_mono_font, px
#include "ui/theme.h"
#include "ui/widgets.h"

namespace hydra::ui::report_frame {

namespace {

using app::report::ChipToken;
using app::report_view::SortDir;
using app::report_view::Tone;

// "13:42", in local time: "Built 13:42" and "(a batch finished at 14:05)"
// both write their time here.
std::string clock_text(std::chrono::system_clock::time_point at) {
    const std::time_t t = std::chrono::system_clock::to_time_t(at);
    std::tm local{};
    localtime_s(&local, &t);
    char hhmm[8];
    std::strftime(hhmm, sizeof(hhmm), "%H:%M", &local);
    return hhmm;
}

// Puts the next `width` of items at the right of the current line when they
// fit beside what is already on it, else on a line of their own.
void right_align(float width) {
    ImGui::SameLine();
    const float avail = ImGui::GetContentRegionAvail().x;
    if (width <= avail)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - width);
    else
        ImGui::NewLine();
}

float button_w(const char* label) { return button_slot_width(label); }

// The width of buttons laid out on one line.
float buttons_w(const std::vector<const char*>& labels) {
    float w = 0.0f;
    for (const char* l : labels) w += button_w(l) + ImGui::GetStyle().ItemSpacing.x;
    return labels.empty() ? 0.0f : w - ImGui::GetStyle().ItemSpacing.x;
}

void call(const std::function<void()>& f) {
    if (f) f();
}

// "Hydra <heading>", the subtitle, "Built HH:MM" and the header's buttons.
void header(const Frame& f) {
    const ReportCallbacks& cb = *f.callbacks;
    ImGui::TextUnformatted("Hydra");
    ImGui::SameLine();
    ImGui::TextColored(kAccentColor, "%s", f.heading);

    if (f.state == ReportState::Ready && f.built) {
        const std::string built = "Built " + clock_text(*f.built);
        std::vector<const char*> buttons;
        if (f.compare_another) buttons.push_back("Compare another player...");
        buttons.push_back("Refresh");
        const float spacing = ImGui::GetStyle().ItemSpacing.x;
        right_align(ImGui::CalcTextSize(built.c_str()).x + spacing + buttons_w(buttons));
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(kDimTextColor, "%s", built.c_str());
        if (f.compare_another) {
            ImGui::SameLine();
            if (ImGui::Button("Compare another player...")) call(cb.compare_another);
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh")) call(cb.refresh);
    }
    const char* subtitle = f.state == ReportState::Ready      ? f.subtitle.c_str()
                           : f.state == ReportState::Building ? f.building_subtitle
                                                              : "";
    if (*subtitle) {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextColored(kDimTextColor, "%s", subtitle);
        ImGui::PopTextWrapPos();
    }
}

// The out-of-date strip: why the rows may be old, and Refresh. The library
// line names the batch's finish time when there is one; the settings line
// has no time (D103 item 21).
void out_of_date_strip(const Frame& f) {
    if (f.state != ReportState::Ready || f.out_of_date == ReportOutOfDate::None) return;
    const std::string why =
        f.out_of_date == ReportOutOfDate::Settings
            ? "The settings changed since this report was built."
        : f.batch_finished ? "Your library changed since this report was built (a batch finished at " +
                                 clock_text(*f.batch_finished) + ")."
                           : "Your library changed since this report was built.";
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(why.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Refresh##outofdate")) call(f.callbacks->refresh);
    ImGui::Separator();
}

// The notice strip: the report's sentence and the files under Show files.
void notice_strip(const Frame& f, Memory& m) {
    if (f.state != ReportState::Ready || f.notice.empty()) return;
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(kWarningColor, "%s", f.notice.c_str());
    if (!f.notice_files.empty()) {
        ImGui::SameLine();
        if (ImGui::Button("Show files")) m.show_files = !m.show_files;
        if (m.show_files) {
            ImGui::PushFont(g_mono_font, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, kDimTextColor);
            for (const std::string& file : f.notice_files) ImGui::BulletText("%s", file.c_str());
            ImGui::PopStyleColor();
            ImGui::PopFont();
        }
    }
}

// The rows a building report will fill, drawn as grey bars (mock board 2a).
void placeholder_rows() {
    if (!ImGui::BeginTable("##placeholder", 4, ImGuiTableFlags_SizingStretchSame)) return;
    const ImU32 bar = ImGui::GetColorU32(ImGuiCol_FrameBg);
    for (int r = 0; r < 3; ++r) {
        ImGui::TableNextRow();
        for (int c = 0; c < 4; ++c) {
            ImGui::TableSetColumnIndex(c);
            const ImVec2 p = ImGui::GetCursorScreenPos();
            const ImVec2 size(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight());
            ImGui::GetWindowDrawList()->AddRectFilled(
                p, ImVec2(p.x + size.x, p.y + size.y * 0.5f), bar,
                ImGui::GetStyle().FrameRounding);
            ImGui::Dummy(size);
        }
    }
    ImGui::EndTable();
}

// Every state but Ready with rows. True when nothing more is drawn.
bool state_body(const Frame& f) {
    const ReportCallbacks& cb = *f.callbacks;
    switch (f.state) {
        case ReportState::Building: {
            // The first line says what is happening; any after it are dim
            // notes, as in the picker box.
            for (size_t i = 0; i < f.building_lines.size(); ++i) {
                if (i == 0)
                    ImGui::TextUnformatted(f.building_lines[i]);
                else
                    ImGui::TextColored(kDimTextColor, "%s", f.building_lines[i]);
            }
            if (f.progress) {
                const auto [done, total] = *f.progress;
                const std::string overlay = "Analyzing " + group_thousands(done) + " of " +
                                            counted(total, "chart", "charts");
                ImGui::ProgressBar(progress_fraction(done, total), ImVec2(-1.0f, 0.0f),
                                   overlay.c_str());
            } else {
                // No count to show: the bar only moves (ImGui's indeterminate bar).
                ImGui::ProgressBar(-1.0f * static_cast<float>(ImGui::GetTime()),
                                   ImVec2(-1.0f, 0.0f), "");
            }
            right_align(button_w("Cancel"));
            if (ImGui::Button("Cancel")) call(cb.cancel);
            if (f.progress) placeholder_rows();
            return true;
        }
        case ReportState::Cancelled:
            ImGui::TextUnformatted("Report cancelled.");
            if (ImGui::Button("Try again")) call(cb.try_again);
            return true;
        case ReportState::Failed:
            ImGui::TextColored(kWarningColor, "%s", f.failure_sentence);
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextWrapped("%s", f.failure_message.c_str());
            ImGui::TextColored(kDimTextColor, "%s", f.failure_error.c_str());
            ImGui::PopTextWrapPos();
            ImGui::Spacing();
            if (ImGui::Button("Try again")) call(cb.try_again);
            if (f.compare_another) {
                ImGui::SameLine();
                if (ImGui::Button("Compare another player...")) call(cb.compare_another);
            }
            return true;
        case ReportState::Ready:
            if (f.empty_text.empty()) return false;
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextWrapped("%s", f.empty_text.c_str());
            ImGui::PopTextWrapPos();
            return true;
    }
    return true;
}

}  // namespace

bool begin(bool* open, const Frame& f, Memory& m) {
    place_report_window(f.window_name.c_str());
    const ImGuiWindowClass window_class = report_window_class();
    ImGui::SetNextWindowClass(&window_class);
    // The smallest size where the controls row and the first five columns
    // still fit, measured from last frame's content (D103 item 14).
    const ImGuiStyle& style = ImGui::GetStyle();
    const float min_w = (std::max)(m.controls_w, m.columns_w);
    if (min_w > 0.0f) {
        const float row_h = ImGui::GetFrameHeightWithSpacing();
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(min_w + style.WindowPadding.x * 2.0f + style.ScrollbarSize,
                   m.table_top + row_h * 2.0f + style.WindowPadding.y),
            ImVec2(FLT_MAX, FLT_MAX));
    }
    // The OS window has Windows' own title bar, so ImGui draws none. No
    // collapse: the OS title bar minimizes it.
    m.visible = ImGui::Begin(f.window_name.c_str(), open,
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);
    m.focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    const bool popup_open =
        ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
    // Esc (once the search box has let go of it) and Ctrl+W close it.
    if (m.focused && !popup_open &&
        ((ImGui::IsKeyPressed(ImGuiKey_Escape, false) && !ImGui::GetIO().WantTextInput) ||
         ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_W)))
        *open = false;
    if (!m.visible) return false;

    out_of_date_strip(f);
    header(f);
    notice_strip(f, m);
    ImGui::Spacing();
    return !state_body(f);
}

void end(bool* open, const Frame& f, Memory& m) {
    if (m.visible && f.state == ReportState::Ready && !f.footer.empty()) {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextColored(kDimTextColor, "%s", f.footer.c_str());
        ImGui::PopTextWrapPos();
    }
    ImGui::End();
    if (m.was_open && !*open && f.callbacks->close) f.callbacks->close();
    m.was_open = *open;
}

void tiles(const std::vector<app::report::Tile>& tiles) {
    const ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kPanelBg);
    for (size_t i = 0; i < tiles.size(); ++i) {
        const app::report::Tile& t = tiles[i];
        const float w = (std::max)(ImGui::CalcTextSize(t.label.c_str()).x,
                                   ImGui::CalcTextSize(t.value.c_str()).x) +
                        style.WindowPadding.x * 2.0f;
        if (i > 0 && fits_on_line(w, style.ItemSpacing.x)) ImGui::SameLine();
        ImGui::PushID(static_cast<int>(i));
        ImGui::BeginChild("##tile", ImVec2(w, 0.0f),
                          ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY,
                          ImGuiWindowFlags_NoScrollbar);
        ImGui::TextColored(kDimTextColor, "%s", t.label.c_str());
        ImGui::TextUnformatted(t.value.c_str());
        ImGui::EndChild();
        ImGui::PopID();
    }
    ImGui::PopStyleColor();
}

bool search_box(Memory& m, const char* hint_text) {
    // As wide as its hint, framed like a button's label.
    ImGui::SetNextItemWidth(button_slot_width(hint_text));
    // The first Escape empties the box, as in the library's search.
    return ImGui::InputTextWithHint("##search", hint_text, m.search, sizeof(m.search),
                                    ImGuiInputTextFlags_EscapeClearsAll);
}

bool dropdown(const char* id, const std::vector<std::string>& labels, int& chosen) {
    float widest = 0.0f;
    for (const std::string& l : labels) widest = (std::max)(widest, ImGui::CalcTextSize(l.c_str()).x);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(widest + ImGui::GetFrameHeight() +
                            ImGui::GetStyle().FramePadding.x * 2.0f);
    bool changed = false;
    if (ImGui::BeginCombo(id, labels[static_cast<size_t>(chosen)].c_str())) {
        for (int i = 0; i < static_cast<int>(labels.size()); ++i) {
            if (ImGui::Selectable(labels[static_cast<size_t>(i)].c_str(), i == chosen) &&
                i != chosen) {
                chosen = i;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

void count_line(Memory& m, const std::string& text) {
    const ImGuiStyle& style = ImGui::GetStyle();
    // The controls packed to the left end here; the count line follows them.
    const float controls_end = ImGui::GetItemRectMax().x - ImGui::GetWindowPos().x;
    const float w = ImGui::CalcTextSize(text.c_str()).x;
    m.controls_w = controls_end - style.WindowPadding.x + style.ItemSpacing.x + w;
    right_align(w);
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(kDimTextColor, "%s", text.c_str());
}

bool nothing_matches() {
    ImGui::Spacing();
    ImGui::TextUnformatted("Nothing matches those filters.");
    return ImGui::Button("Clear filters");
}

float footer_height(const Frame& f) {
    if (f.footer.empty()) return 0.0f;
    const float wrap = ImGui::GetContentRegionAvail().x;
    return ImGui::CalcTextSize(f.footer.c_str(), nullptr, false, wrap).y +
           ImGui::GetStyle().ItemSpacing.y;
}

void cell(const std::string& text, const app::report_view::CellLook& look, Tone tone,
          bool numeric) {
    const bool dim = tone == Tone::Dim || (tone == Tone::Normal && look.dim);
    // An alert reads in the third tier's red, as the pages' .neg did.
    const ImVec4 color = tone == Tone::Alert ? chip_color(ChipToken::t3)
                         : dim               ? kDimTextColor
                                             : ImGui::GetStyleColorVec4(ImGuiCol_Text);
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    if (look.mono) ImGui::PushFont(g_mono_font, 0.0f);
    if (numeric) {
        const float w = ImGui::CalcTextSize(text.c_str()).x;
        const float avail = ImGui::GetContentRegionAvail().x;
        if (w < avail) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - w);
    }
    text_ellipsized(text.c_str());
    if (look.mono) ImGui::PopFont();
    ImGui::PopStyleColor();
}

void chip(const std::string& text, ChipToken token) {
    const ImVec4 color = chip_color(token);
    const float pad = ImGui::GetStyle().FramePadding.x;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float avail = ImGui::GetContentRegionAvail().x;
    const float w = (std::min)(ImGui::CalcTextSize(text.c_str()).x + pad * 2.0f, avail);
    const float h = ImGui::GetTextLineHeight();
    // The pages drew the no-squeeze and not-in-library chips (tn) without an
    // outline.
    if (token != ChipToken::tn)
        ImGui::GetWindowDrawList()->AddRect(pos, ImVec2(pos.x + w, pos.y + h),
                                            ImGui::GetColorU32(color), h * 0.5f);
    ImGui::SetCursorScreenPos(ImVec2(pos.x + pad, pos.y));
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    text_ellipsized(text.c_str(), (std::max)(0.0f, w - pad * 2.0f));
    ImGui::PopStyleColor();
}

ImGuiTableFlags table_flags() {
    // The library table's flags (library_table.cpp's render_table), with
    // SortMulti for the Shift+click second sort (D103 item 1).
    return ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable |
           ImGuiTableFlags_SortMulti | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
           ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_SizingStretchProp;
}

float row_number_width(size_t rows) {
    return ImGui::CalcTextSize(group_thousands(static_cast<int64_t>(rows)).c_str()).x;
}

void best_bar() {
    // The pages' gold bar, 3 px wide (html_page.cpp's tr.best), at the row's
    // left edge.
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    ImGui::GetWindowDrawList()->AddRectFilled(min, ImVec2(min.x + px(3.0f), max.y),
                                              ImGui::GetColorU32(kBestPathColor));
}

bool table_settled() { return !ImGui::GetCurrentTable()->IsInitializing; }

void push_sort(const HeaderSort& sort) {
    for (size_t i = 0; i < sort.size(); ++i)
        ImGui::TableSetColumnSortDirection(sort[i].first,
                                           sort[i].second == SortDir::Descending
                                               ? ImGuiSortDirection_Descending
                                               : ImGuiSortDirection_Ascending,
                                           i > 0);
}

std::optional<HeaderSort> header_sort() {
    ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs();
    if (!specs || !specs->SpecsDirty) return std::nullopt;
    specs->SpecsDirty = false;
    HeaderSort sort;
    for (int i = 0; i < specs->SpecsCount && i < 2; ++i)
        sort.push_back({specs->Specs[i].ColumnIndex,
                        specs->Specs[i].SortDirection == ImGuiSortDirection_Descending
                            ? SortDir::Descending
                            : SortDir::Ascending});
    if (specs->SpecsCount > 2) push_sort(sort);
    return sort;
}

void measure_columns(Memory& m, const std::vector<std::string>& titles) {
    const ImGuiStyle& style = ImGui::GetStyle();
    float w = 0.0f;
    for (size_t c = 0; c < titles.size() && c < 5; ++c)
        w += ImGui::CalcTextSize(titles[c].c_str()).x + style.CellPadding.x * 2.0f;
    m.columns_w = w;
    const ImGuiTable* table = ImGui::GetCurrentTable();
    m.table_top = table->OuterRect.Min.y - table->OuterWindow->Pos.y;
}

}  // namespace hydra::ui::report_frame
